#include "VoxPlusEngine.h"
#include "../../DSP/ScaleQuantizer.h"
#include "../../DSP/AudioSafety.h"

namespace voxplus
{

namespace
{
    float magnitudeToDb (float mag)
    {
        return mag > 1.0e-8f ? 20.0f * std::log10 (mag) : -60.0f;
    }

    vocalplus::ScaleType scaleFromIndex (int index)
    {
        switch (index)
        {
            case 0: return vocalplus::ScaleType::chromatic;
            case 2: return vocalplus::ScaleType::naturalMinor;
            case 3: return vocalplus::ScaleType::pentatonicMajor;
            case 4: return vocalplus::ScaleType::pentatonicMinor;
            default: return vocalplus::ScaleType::major;
        }
    }
}

void VoxPlusEngine::prepare (double sampleRate, int maxBlockSize, int numChannels)
{
    sampleRateHz = sampleRate;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (maxBlockSize);
    spec.numChannels = static_cast<juce::uint32> (numChannels);

    lowCutFilter.prepare (spec);
    highCutFilter.prepare (spec);
    focusLow.prepare (spec);
    focusHigh.prepare (spec);
    fxFilterL.prepare (spec);
    fxFilterR.prepare (spec);

    const int maxDelaySamples = static_cast<int> (sampleRate * 2.0) + 1;
    delayLineL.setMaximumDelayInSamples (maxDelaySamples);
    delayLineR.setMaximumDelayInSamples (maxDelaySamples);

    juce::dsp::ProcessSpec delaySpec;
    delaySpec.sampleRate = sampleRate;
    delaySpec.maximumBlockSize = static_cast<juce::uint32> (maxBlockSize);
    delaySpec.numChannels = 1;
    delayLineL.prepare (delaySpec);
    delayLineR.prepare (delaySpec);
    delayLineL.setDelay (1.0f);
    delayLineR.setDelay (1.0f);

    reverb.prepare (spec);
    reverb.reset();

    autoTune.prepare (sampleRate, maxBlockSize);
    pitchShifter.prepare (sampleRate, maxBlockSize);
    unisonShifterL.prepare (sampleRate, maxBlockSize);
    unisonShifterR.prepare (sampleRate, maxBlockSize);

    compressor.prepare (sampleRate, maxBlockSize, numChannels);

    monoIn.resize (static_cast<size_t> (maxBlockSize));
    monoOut.resize (static_cast<size_t> (maxBlockSize));

    reset();
    prepared = true;
    setSettings (settings);
}

void VoxPlusEngine::reset()
{
    lowCutFilter.reset();
    highCutFilter.reset();
    focusLow.reset();
    focusHigh.reset();
    delayLineL.reset();
    delayLineR.reset();
    reverb.reset();
    autoTune.reset();
    pitchShifter.reset();
    unisonShifterL.reset();
    unisonShifterR.reset();
    compressor.reset();
    chopPhase = 0.0f;
    inputLevelDb = -60.0f;
    outputLevelDb = -60.0f;
}

void VoxPlusEngine::setSettings (const VoxPlusSettings& newSettings)
{
    settings = newSettings;
    updateFilters();
}

void VoxPlusEngine::updateFilters()
{
    if (spec.sampleRate <= 0.0)
        return;

    const double sr = spec.sampleRate;

    lowCutFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass (
        sr, juce::jmax (20.0f, settings.lowCutHz));

    highCutFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass (
        sr, juce::jlimit (2000.0f, 20000.0f, settings.highCutHz));

    const float focus = (settings.focusEQ - 0.5f) * 12.0f;
    focusLow.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf (
        sr, 250.0, 0.707f, juce::Decibels::decibelsToGain (-focus));
    focusHigh.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf (
        sr, 4000.0, 0.707f, juce::Decibels::decibelsToGain (focus));
}

tuneplus::CompressorSettings VoxPlusEngine::dynamicsToCompressor (const FxModuleSettings& fx) const
{
    tuneplus::CompressorSettings c;
    c.enabled = true;
    c.mix = 1.0f;
    c.highPassHz = settings.lowCutHz;

    const float a = fx.amount;
    const float t = fx.tone;

    switch (fx.mode)
    {
        case 0: c.thresholdDb = -20.0f - a * 10.0f; c.ratio = 2.0f + a * 4.0f; c.attackMs = 8.0f; c.releaseMs = 100.0f + t * 150.0f; break;
        case 1: c.thresholdDb = -22.0f - a * 12.0f; c.ratio = 3.0f + a * 5.0f; c.attackMs = 3.0f; c.releaseMs = 80.0f; break;
        case 2: c.thresholdDb = -14.0f - a * 8.0f; c.ratio = 2.0f + a * 2.0f; c.attackMs = 25.0f + t * 20.0f; c.releaseMs = 220.0f; break;
        case 3: c.thresholdDb = -26.0f - a * 10.0f; c.ratio = 4.0f + a * 8.0f; c.attackMs = 1.5f; c.releaseMs = 60.0f; break;
        case 4: c.thresholdDb = -12.0f - a * 6.0f; c.ratio = 1.5f + a * 2.0f; c.attackMs = 20.0f; c.releaseMs = 280.0f; c.mix = 0.5f + a * 0.5f; break;
        default: c.thresholdDb = -28.0f - a * 8.0f; c.ratio = 6.0f + a * 10.0f; c.attackMs = 1.0f; c.releaseMs = 50.0f; break;
    }

    c.makeupDb = a * (4.0f + t * 6.0f);
    return c;
}

void VoxPlusEngine::processVoice (juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int channels = buffer.getNumChannels();

    voiceDry.makeCopyOf (buffer, true);
    monoScratch.setSize (1, numSamples, false, false, true);
    monoScratch2.setSize (1, numSamples, false, false, true);

    auto* mono = monoScratch.getWritePointer (0);
    auto* processed = monoScratch2.getWritePointer (0);

    if (channels == 1)
        std::copy (buffer.getReadPointer (0), buffer.getReadPointer (0) + numSamples, mono);
    else
        for (int i = 0; i < numSamples; ++i)
            mono[i] = 0.5f * (buffer.getSample (0, i) + buffer.getSample (1, i));

    vocalplus::AutoTuneSettings at;
    at.enabled = settings.correction > 0.01f;
    at.retuneSpeed = 0.3f + settings.correction * 0.7f;
    at.toleranceCents = juce::jmap (settings.correction, 0.0f, 1.0f, 100.0f, 0.0f);
    at.formantPreserve = 0.5f + (settings.formant - 1.0f) * 0.5f;
    at.rootNote = settings.rootNote;
    at.scale = scaleFromIndex (settings.tuneScale);
    at.detuneCents = 0.0f;
    autoTune.setSettings (at);
    autoTune.process (mono, processed, numSamples);

    const float pitchRatio = std::pow (2.0f, (settings.pitchShiftSemitones / 12.0f));
    pitchShifter.setPitchRatio (pitchRatio);
    pitchShifter.setFormantRatio (settings.formant);
    pitchShifter.processBlock (processed, mono, numSamples);
    std::copy (mono, mono + numSamples, processed);

    if (settings.unison > 0.01f)
    {
        auto* uniL = monoIn.data();
        auto* uniR = monoOut.data();
        unisonShifterL.setPitchRatio (pitchRatio * std::pow (2.0f, 0.08f * settings.unison));
        unisonShifterR.setPitchRatio (pitchRatio * std::pow (2.0f, -0.08f * settings.unison));
        unisonShifterL.setFormantRatio (settings.formant);
        unisonShifterR.setFormantRatio (settings.formant);
        unisonShifterL.processBlock (processed, uniL, numSamples);
        unisonShifterR.processBlock (processed, uniR, numSamples);

        for (int i = 0; i < numSamples; ++i)
            processed[i] = processed[i] * (1.0f - settings.unison * 0.5f)
                         + (uniL[i] + uniR[i]) * 0.5f * settings.unison * 0.35f;
    }

    const float mix = settings.voiceBypass ? 0.0f : settings.voiceMix;

    for (int ch = 0; ch < channels; ++ch)
    {
        auto* out = buffer.getWritePointer (ch);
        const auto* dry = voiceDry.getReadPointer (ch);

        for (int i = 0; i < numSamples; ++i)
            out[i] = dry[i] + (processed[i] - dry[i]) * mix;
    }
}

void VoxPlusEngine::applyDynamics (juce::AudioBuffer<float>& buffer, const FxModuleSettings& fx)
{
    compressor.setSettings (dynamicsToCompressor (fx));
    compressor.process (buffer);
}

void VoxPlusEngine::applyCharacter (juce::AudioBuffer<float>& buffer, const FxModuleSettings& fx)
{
    if (fx.amount <= 0.001f)
        return;

    const float drive = 1.0f + fx.amount * (4.0f + static_cast<float> (fx.mode) * 0.8f);
    const float mix = juce::jlimit (0.0f, 1.0f, fx.amount * (0.75f + fx.tone * 0.25f));

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getWritePointer (ch);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const float dry = data[i];
            float wet = std::tanh (dry * drive);

            switch (fx.mode)
            {
                case 2: wet = wet * 0.9f + wet * wet * wet * 0.15f; break;
                case 3: wet = wet + fx.tone * 0.1f * wet * wet; break;
                case 4: wet = juce::jlimit (-1.0f, 1.0f, wet * (1.0f + fx.tone)); break;
                case 5: wet = wet * (1.0f - fx.amount * 0.2f) + std::sin (wet * 3.0f) * 0.05f; break;
                default: break;
            }

            data[i] = dry + (wet - dry) * mix;
        }
    }
}

void VoxPlusEngine::applyFilter (juce::AudioBuffer<float>& buffer, const FxModuleSettings& fx)
{
    if (fx.amount <= 0.001f || spec.sampleRate <= 0.0)
        return;

    const float a = juce::jmin (1.0f, fx.amount * 1.25f);
    const float t = fx.tone;

    auto setCoeffs = [&] (juce::dsp::IIR::Filter<float>& filter)
    {
        switch (fx.mode)
        {
            case 0: filter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter (
                spec.sampleRate, 2800.0 + t * 2000.0, 1.0f, juce::Decibels::decibelsToGain (a * 12.0f)); break;
            case 1: filter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass (
                spec.sampleRate, juce::jmap (a, 9000.0f, 1800.0f)); break;
            case 2: filter.coefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass (
                spec.sampleRate, 800.0 + t * 1600.0, 0.8f + a * 0.8f); break;
            case 3: filter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf (
                spec.sampleRate, 8000.0, 0.707f, juce::Decibels::decibelsToGain (a * 14.0f)); break;
            case 4: filter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter (
                spec.sampleRate, 1500.0 + t * 1500.0, 1.0f, juce::Decibels::decibelsToGain (a * 10.0f)); break;
            default: filter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter (
                spec.sampleRate, 4500.0, 1.5f, juce::Decibels::decibelsToGain (-a * 12.0f)); break;
        }
    };

    setCoeffs (fxFilterL);
    setCoeffs (fxFilterR);

    juce::dsp::AudioBlock<float> block (buffer);
    const int channels = buffer.getNumChannels();

    if (channels >= 1)
    {
        auto channelBlock = block.getSingleChannelBlock (0);
        juce::dsp::ProcessContextReplacing<float> ctx (channelBlock);
        fxFilterL.process (ctx);
    }

    if (channels >= 2)
    {
        auto channelBlock = block.getSingleChannelBlock (1);
        juce::dsp::ProcessContextReplacing<float> ctx (channelBlock);
        fxFilterR.process (ctx);
    }
}

void VoxPlusEngine::applyDelay (juce::AudioBuffer<float>& buffer, const FxModuleSettings& fx)
{
    if (fx.amount <= 0.001f)
        return;

    const int numSamples = buffer.getNumSamples();
    const float baseMs = 80.0f + fx.tone * 320.0f;
    const float delayMs = baseMs * (0.5f + fx.amount);
    const int delaySamples = juce::jmax (1, static_cast<int> (delayMs * 0.001 * sampleRateHz));
    const float feedback = juce::jlimit (0.0f, 0.75f, fx.amount * 0.45f);
    const float mix = fx.amount * 0.85f;

    delayLineL.setDelay (static_cast<float> (delaySamples));
    delayLineR.setDelay (static_cast<float> (delaySamples + (fx.mode == 4 ? 20 : 0)));

    auto* left = buffer.getWritePointer (0);
    auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : left;

    for (int i = 0; i < numSamples; ++i)
    {
        const float inL = left[i];
        const float inR = right[i];

        const float dl = delayLineL.popSample (0);
        const float dr = delayLineR.popSample (0);

        delayLineL.pushSample (0, inL + dl * feedback);
        delayLineR.pushSample (0, (fx.mode == 4 ? inR : inL) + dr * feedback);

        left[i] = inL + dl * mix;
        right[i] = inR + dr * mix;
    }
}

void VoxPlusEngine::applyReverb (juce::AudioBuffer<float>& buffer, const FxModuleSettings& fx)
{
    if (fx.amount <= 0.001f)
        return;

    reverbParams.roomSize = 0.2f + fx.amount * 0.75f;
    reverbParams.damping = 0.3f + fx.tone * 0.6f;
    reverbParams.wetLevel = fx.amount * 0.75f;
    reverbParams.dryLevel = 1.0f - fx.amount * 0.45f;
    reverbParams.width = 0.5f + fx.tone * 0.5f;
    reverbParams.freezeMode = fx.mode == 3 ? 1.0f : 0.0f;
    reverb.setParameters (reverbParams);

    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> ctx (block);
    reverb.process (ctx);
}

void VoxPlusEngine::applyChop (juce::AudioBuffer<float>& buffer, const FxModuleSettings& fx)
{
    if (fx.amount <= 0.001f)
        return;

    const int numSamples = buffer.getNumSamples();
    const float rateHz = 2.0f + fx.amount * (8.0f + static_cast<float> (fx.mode) * 2.0f);
    const float depth = fx.amount * (0.5f + fx.tone * 0.5f);

    for (int i = 0; i < numSamples; ++i)
    {
        chopPhase += rateHz / static_cast<float> (sampleRateHz);
        if (chopPhase >= 1.0f)
            chopPhase -= 1.0f;

        float gate = 1.0f;
        switch (fx.mode)
        {
            case 0: gate = chopPhase < 0.25f ? 1.0f : 0.0f; break;
            case 1: gate = chopPhase > 0.5f ? 1.0f : 0.05f; break;
            case 2: gate = std::sin (chopPhase * juce::MathConstants<float>::twoPi) * 0.5f + 0.5f; break;
            case 3: gate = (static_cast<int> (chopPhase * 8.0f) % 2 == 0) ? 1.0f : 0.2f; break;
            case 4: gate = 0.5f + 0.5f * std::sin (chopPhase * juce::MathConstants<float>::twoPi * 3.0f); break;
            default: gate = chopPhase < 0.35f ? 1.0f : 0.15f; break;
        }

        const float g = 1.0f - depth + depth * gate;

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            buffer.getWritePointer (ch)[i] *= g;
    }
}

void VoxPlusEngine::processFxModule (int index, juce::AudioBuffer<float>& buffer)
{
    const auto& fx = settings.fx[static_cast<size_t> (index)];
    if (! fx.enabled)
        return;

    switch (index)
    {
        case 0: applyDynamics (buffer, fx); break;
        case 1: applyCharacter (buffer, fx); break;
        case 2: applyFilter (buffer, fx); break;
        case 3: applyDelay (buffer, fx); break;
        case 4: applyReverb (buffer, fx); break;
        case 5: applyChop (buffer, fx); break;
        default: break;
    }
}

void VoxPlusEngine::process (juce::AudioBuffer<float>& buffer)
{
    if (! prepared || buffer.getNumSamples() <= 0)
        return;

    const int numSamples = buffer.getNumSamples();
    const int channels = buffer.getNumChannels();

    inputLevelDb = magnitudeToDb (buffer.getMagnitude (0, numSamples));

    buffer.applyGain (juce::Decibels::decibelsToGain (settings.inputGainDb));
    globalDry.makeCopyOf (buffer, true);

    processVoice (buffer);

    for (int i = 0; i < 6; ++i)
        processFxModule (i, buffer);

    juce::dsp::AudioBlock<float> block (buffer);
    for (int ch = 0; ch < channels; ++ch)
    {
        auto channelBlock = block.getSingleChannelBlock (static_cast<size_t> (ch));
        juce::dsp::ProcessContextReplacing<float> ctx (channelBlock);
        lowCutFilter.process (ctx);
        highCutFilter.process (ctx);
        focusLow.process (ctx);
        focusHigh.process (ctx);
    }

    if (settings.doubling > 0.01f && channels >= 2)
    {
        auto* left = buffer.getWritePointer (0);
        auto* right = buffer.getWritePointer (1);
        const int offset = static_cast<int> (0.015 * sampleRateHz);

        for (int i = numSamples - 1; i >= offset; --i)
        {
            const float d = left[i - offset] * settings.doubling * 0.5f;
            left[i] += d;
            right[i] += d * 0.95f;
        }
    }

    if (settings.superGlue > 0.01f)
    {
        tuneplus::CompressorSettings glue;
        glue.enabled = true;
        glue.thresholdDb = -30.0f - settings.superGlue * 15.0f;
        glue.ratio = 4.0f + settings.superGlue * 12.0f;
        glue.attackMs = 0.5f;
        glue.releaseMs = 40.0f;
        glue.makeupDb = settings.superGlue * 8.0f;
        glue.mix = settings.superGlue;
        glue.highPassHz = settings.lowCutHz;
        compressor.setSettings (glue);
        compressor.process (buffer);
    }

    if (settings.autoLevel > 0.01f)
    {
        const float peak = buffer.getMagnitude (0, numSamples);
        if (peak > 1.0e-6f)
        {
            const float target = juce::jmap (settings.autoLevel, 0.0f, 1.0f, 1.0f, 0.5f);
            buffer.applyGain (vocalplus::safeAutoLevelGain (peak, target));
        }
    }

    buffer.applyGain (juce::Decibels::decibelsToGain (settings.outputGainDb));

    if (settings.globalMix < 0.999f)
    {
        for (int ch = 0; ch < channels; ++ch)
        {
            auto* out = buffer.getWritePointer (ch);
            const auto* in = globalDry.getReadPointer (ch);

            for (int i = 0; i < numSamples; ++i)
                out[i] = in[i] + (out[i] - in[i]) * settings.globalMix;
        }
    }

    outputLevelDb = magnitudeToDb (buffer.getMagnitude (0, numSamples));
}

} // namespace voxplus
