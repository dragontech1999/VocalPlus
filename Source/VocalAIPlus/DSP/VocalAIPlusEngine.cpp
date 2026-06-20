#include "VocalAIPlusEngine.h"

namespace vocalaiplus
{

void VocalAIPlusEngine::prepare (double sampleRate, int maxBlockSize, int numChannels)
{
    sampleRateHz = sampleRate;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (maxBlockSize);
    spec.numChannels = static_cast<juce::uint32> (numChannels);

    compressor.prepare (sampleRate, maxBlockSize, numChannels);
    limiter.prepare (sampleRate, maxBlockSize, numChannels);

    for (auto* f : { &noiseHpfL, &noiseHpfR, &eqLowShelfL, &eqLowShelfR,
                     &eqLowMidL, &eqLowMidR, &eqPresenceL, &eqPresenceR,
                     &eqHighShelfL, &eqHighShelfR, &deEssL, &deEssR })
    {
        f->prepare (spec);
    }

    reverb.prepare (spec);
    dryBuffer.setSize (numChannels, maxBlockSize);
    exciterScratch.setSize (numChannels, maxBlockSize);
    reset();
    prepared = true;
    setSettings (settings);
}

void VocalAIPlusEngine::reset()
{
    compressor.reset();
    limiter.reset();
    reverb.reset();

    for (auto* f : { &noiseHpfL, &noiseHpfR, &eqLowShelfL, &eqLowShelfR,
                     &eqLowMidL, &eqLowMidR, &eqPresenceL, &eqPresenceR,
                     &eqHighShelfL, &eqHighShelfR, &deEssL, &deEssR })
        f->reset();

    deEssEnvelope = 0.0f;
    inputLevelDb = -60.0f;
    outputLevelDb = -60.0f;
}

void VocalAIPlusEngine::updateFilters()
{
    if (sampleRateHz <= 0.0)
        return;

    const float hpfHz = 70.0f + settings.noiseAmount * 40.0f;
    auto hpf = juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRateHz, hpfHz);
    noiseHpfL.coefficients = hpf;
    noiseHpfR.coefficients = hpf;

    auto lowShelf = juce::dsp::IIR::Coefficients<float>::makeLowShelf (
        sampleRateHz, 120.0f, 0.707f, juce::Decibels::decibelsToGain (settings.eqLowShelfDb));
    auto lowMid = juce::dsp::IIR::Coefficients<float>::makePeakFilter (
        sampleRateHz, 320.0f, 1.0f, juce::Decibels::decibelsToGain (settings.eqLowMidDb));
    auto presence = juce::dsp::IIR::Coefficients<float>::makePeakFilter (
        sampleRateHz, 4200.0f, 1.0f, juce::Decibels::decibelsToGain (settings.eqPresenceDb));
    auto highShelf = juce::dsp::IIR::Coefficients<float>::makeHighShelf (
        sampleRateHz, 10000.0f, 0.707f, juce::Decibels::decibelsToGain (settings.eqHighShelfDb));

    eqLowShelfL.coefficients = lowShelf;
    eqLowShelfR.coefficients = lowShelf;
    eqLowMidL.coefficients = lowMid;
    eqLowMidR.coefficients = lowMid;
    eqPresenceL.coefficients = presence;
    eqPresenceR.coefficients = presence;
    eqHighShelfL.coefficients = highShelf;
    eqHighShelfR.coefficients = highShelf;

    const float bw = 0.35f + settings.deEssSensitivity * 0.25f;
    const float q = 1.0f / juce::jmax (0.15f, bw);
    auto deEssCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter (
        sampleRateHz, settings.deEssFreqHz, q, 1.0f);
    deEssL.coefficients = deEssCoeffs;
    deEssR.coefficients = deEssCoeffs;

    reverbParams.roomSize = juce::jlimit (0.0f, 1.0f, settings.reverbRoomSize);
    reverbParams.damping = 0.5f;
    reverbParams.wetLevel = settings.reverbWet;
    reverbParams.dryLevel = 1.0f - settings.reverbWet;
    reverbParams.width = 1.0f;
    reverbParams.freezeMode = 0.0f;
    reverb.setParameters (reverbParams);
}

void VocalAIPlusEngine::setSettings (const ChainSettings& newSettings)
{
    settings = newSettings;
    updateFilters();

    tuneplus::CompressorSettings comp;
    comp.enabled = settings.compEnabled;
    comp.thresholdDb = settings.compThresholdDb;
    comp.ratio = settings.compRatio;
    comp.attackMs = settings.compAttackMs;
    comp.releaseMs = settings.compReleaseMs;
    comp.makeupDb = settings.compMakeupDb;
    comp.highPassHz = 80.0f;
    comp.mix = 1.0f;
    compressor.setSettings (comp);

    tuneplus::CompressorSettings lim;
    lim.enabled = settings.limiterEnabled;
    lim.thresholdDb = settings.limiterCeilingDbTP - 3.0f + settings.limiterInputGainDb;
    lim.ratio = 20.0f;
    lim.attackMs = 1.0f;
    lim.releaseMs = 55.0f;
    lim.makeupDb = 0.0f;
    lim.highPassHz = 20.0f;
    lim.mix = 1.0f;
    limiter.setSettings (lim);
}

void VocalAIPlusEngine::processDeEsser (juce::AudioBuffer<float>& buffer)
{
    if (! settings.deEssEnabled)
        return;

    const int numSamples = buffer.getNumSamples();
    const float attack = std::exp (-1.0f / (0.003f * static_cast<float> (sampleRateHz)));
    const float release = std::exp (-1.0f / (0.050f * static_cast<float> (sampleRateHz)));
    const float threshold = 0.02f + (1.0f - settings.deEssSensitivity) * 0.08f;
    const float maxCut = juce::Decibels::decibelsToGain (-settings.deEssReductionDb);

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto& deEss = (ch == 0) ? deEssL : deEssR;
        auto channelBlock = juce::dsp::AudioBlock<float> (buffer).getSingleChannelBlock (
            static_cast<size_t> (ch));
        juce::dsp::ProcessContextReplacing<float> ctx (channelBlock);
        deEss.process (ctx);

        auto* data = buffer.getWritePointer (ch);
        for (int i = 0; i < numSamples; ++i)
        {
            const float level = std::abs (data[i]);
            deEssEnvelope = level > deEssEnvelope
                ? attack * deEssEnvelope + (1.0f - attack) * level
                : release * deEssEnvelope + (1.0f - release) * level;

            if (deEssEnvelope > threshold)
            {
                const float gr = juce::jmap (deEssEnvelope, threshold, threshold * 4.0f, 1.0f, maxCut);
                data[i] *= juce::jlimit (maxCut, 1.0f, gr);
            }
        }
    }
}

void VocalAIPlusEngine::processExciter (juce::AudioBuffer<float>& buffer)
{
    if (! settings.exciterEnabled || settings.exciterMix <= 0.001f)
        return;

    exciterScratch.makeCopyOf (buffer, true);
    const float drive = 1.0f + settings.exciterAmount * 4.0f;
    const float wet = settings.exciterMix * settings.exciterAmount;

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* out = buffer.getWritePointer (ch);
        const auto* dry = exciterScratch.getReadPointer (ch);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const float x = dry[i] * drive;
            const float shaped = std::tanh (x);
            out[i] = dry[i] + (shaped - dry[i]) * wet;
        }
    }
}

void VocalAIPlusEngine::process (juce::AudioBuffer<float>& buffer)
{
    if (! prepared || buffer.getNumSamples() <= 0)
        return;

    const float inMag = buffer.getMagnitude (0, buffer.getNumSamples());
    inputLevelDb = inMag > 1.0e-8f ? juce::Decibels::gainToDecibels (inMag) : -60.0f;

    if (settings.mix < 0.999f)
        dryBuffer.makeCopyOf (buffer, true);

    if (settings.noiseEnabled)
    {
        const float nrGain = juce::Decibels::decibelsToGain (-2.0f * settings.noiseAmount);
        auto block = juce::dsp::AudioBlock<float> (buffer);

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto& hpf = (ch == 0) ? noiseHpfL : noiseHpfR;
            auto channelBlock = block.getSingleChannelBlock (static_cast<size_t> (ch));
            juce::dsp::ProcessContextReplacing<float> ctx (channelBlock);
            hpf.process (ctx);
        }

        buffer.applyGain (nrGain);
    }

    if (settings.eqEnabled)
    {
        auto block = juce::dsp::AudioBlock<float> (buffer);

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto channelBlock = block.getSingleChannelBlock (static_cast<size_t> (ch));
            juce::dsp::ProcessContextReplacing<float> ctx (channelBlock);

            if (ch == 0)
            {
                eqLowShelfL.process (ctx);
                eqLowMidL.process (ctx);
                eqPresenceL.process (ctx);
                eqHighShelfL.process (ctx);
            }
            else
            {
                eqLowShelfR.process (ctx);
                eqLowMidR.process (ctx);
                eqPresenceR.process (ctx);
                eqHighShelfR.process (ctx);
            }
        }
    }

    compressor.process (buffer);
    processDeEsser (buffer);
    processExciter (buffer);

    if (settings.reverbEnabled && settings.reverbWet > 0.001f)
    {
        juce::dsp::AudioBlock<float> block (buffer);
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        reverb.process (ctx);
    }

    limiter.process (buffer);
    buffer.applyGain (juce::Decibels::decibelsToGain (settings.outputGainDb));

    if (settings.mix < 0.999f)
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* out = buffer.getWritePointer (ch);
            const auto* dry = dryBuffer.getReadPointer (ch);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
                out[i] = dry[i] + (out[i] - dry[i]) * settings.mix;
        }
    }

    const float outMag = buffer.getMagnitude (0, buffer.getNumSamples());
    outputLevelDb = outMag > 1.0e-8f ? juce::Decibels::gainToDecibels (outMag) : -60.0f;
}

} // namespace vocalaiplus
