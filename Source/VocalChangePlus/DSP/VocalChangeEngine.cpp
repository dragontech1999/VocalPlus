#include "VocalChangeEngine.h"

namespace vocalchangeplus
{

namespace
{
    float magnitudeToDb (float mag)
    {
        return mag > 1.0e-8f ? 20.0f * std::log10 (mag) : -60.0f;
    }

    float softClip (float x, float drive)
    {
        return std::tanh (x * drive) / std::tanh (drive);
    }

    int wrapIndex (int index, int length)
    {
        if (length <= 0)
            return 0;

        index %= length;
        if (index < 0)
            index += length;

        return index;
    }
}

void VocalChangeEngine::prepare (double sampleRate, int blockSize, int numChannels)
{
    sampleRateHz = sampleRate;
    maxBlockSize = blockSize;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (blockSize);
    spec.numChannels = static_cast<juce::uint32> (numChannels);

    autoTune.prepare (sampleRate, blockSize);
    pitchResampler.prepare (sampleRate, blockSize);
    filterBand.prepare (spec);
    formantLowShelf.prepare (spec);
    formantHighShelf.prepare (spec);
    formantPresence.prepare (spec);
    reverb.prepare (spec);
    reverb.reset();

    speedBuffer.assign (static_cast<size_t> (sampleRate * 4.0), 0.0f);
    chorusDelay.assign (static_cast<size_t> (sampleRate * 0.05) + 1, 0.0f);

    reset();
    prepared = true;
    appliedSettings = {};
    setSettings (settings);
}

void VocalChangeEngine::reset()
{
    autoTune.reset();
    pitchResampler.reset();
    filterBand.reset();
    formantLowShelf.reset();
    formantHighShelf.reset();
    formantPresence.reset();
    reverb.reset();
    std::fill (speedBuffer.begin(), speedBuffer.end(), 0.0f);
    std::fill (chorusDelay.begin(), chorusDelay.end(), 0.0f);
    chorusWritePos = 0;
    speedWritePos = 0.0;
    speedReadPos = static_cast<double> (speedBuffer.size()) / 2.0;
    robotPhase = 0.0f;
    chorusPhase = 0.0f;
    aiPhase = 0.0f;
    inputLevelDb = -60.0f;
    outputLevelDb = -60.0f;
}

void VocalChangeEngine::setSettings (const VocalChangeSettings& newSettings)
{
    settings = newSettings;

    if (settings != appliedSettings)
    {
        updateFilters();
        updateFormantFilters();
        appliedSettings = settings;
    }
}

void VocalChangeEngine::updateFormantFilters()
{
    if (spec.sampleRate <= 0.0)
        return;

    const double sr = spec.sampleRate;
    const float f = settings.formant;
    const float lowGainDb = juce::jmap (f, 0.5f, 1.5f, 10.0f, -10.0f);
    const float highGainDb = juce::jmap (f, 0.5f, 1.5f, -12.0f, 12.0f);
    const float presenceDb = juce::jmap (f, 0.5f, 1.5f, -6.0f, 9.0f);

    formantLowShelf.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf (
        sr, 160.0, 0.75, juce::Decibels::decibelsToGain (lowGainDb));
    formantHighShelf.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf (
        sr, 4200.0, 0.65, juce::Decibels::decibelsToGain (highGainDb));
    formantPresence.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter (
        sr, 2600.0, 1.1, juce::Decibels::decibelsToGain (presenceDb));
}

void VocalChangeEngine::updateFilters()
{
    if (spec.sampleRate <= 0.0)
        return;

    const double sr = spec.sampleRate;

    switch (settings.filterMode)
    {
        case 1:
            filterBand.coefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass (sr, 1200.0, 0.8);
            break;
        case 2:
            filterBand.coefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass (sr, 800.0, 0.6);
            break;
        case 3:
            filterBand.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass (sr, 200.0);
            break;
        case 4:
            filterBand.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass (sr, 2500.0);
            break;
        default:
            filterBand.coefficients = juce::dsp::IIR::Coefficients<float>::makeAllPass (sr, 1000.0);
            break;
    }

    reverbParams.roomSize = 0.3f + settings.reverb * 0.65f;
    reverbParams.damping = 0.4f + (1.0f - settings.reverb) * 0.4f;
    reverbParams.wetLevel = settings.reverb * 0.55f;
    reverbParams.dryLevel = 1.0f - settings.reverb * 0.3f;
    reverbParams.width = 0.8f + settings.reverb * 0.2f;
    reverbParams.freezeMode = 0.0f;
    reverb.setParameters (reverbParams);
}

bool VocalChangeEngine::isVoiceTransformBypassed() const noexcept
{
    return settings.correction <= 0.01f
        && std::abs (settings.pitchShiftSemitones) < 0.01f
        && std::abs (settings.formant - 1.0f) < 0.02f
        && std::abs (settings.speed - 1.0f) < 0.01f
        && settings.distortion <= 0.001f
        && settings.reverb <= 0.001f
        && settings.robot <= 0.001f
        && settings.chorus <= 0.001f
        && settings.aiMorph <= 0.001f
        && settings.filterMode <= 0;
}

void VocalChangeEngine::applySpeed (float* data, int numSamples)
{
    if (speedBuffer.empty() || std::abs (settings.speed - 1.0f) <= 0.01f)
        return;

    const int bufLen = static_cast<int> (speedBuffer.size());
    const double readIncrement = 1.0 / static_cast<double> (settings.speed);

    for (int i = 0; i < numSamples; ++i)
    {
        speedBuffer[static_cast<size_t> (wrapIndex (static_cast<int> (speedWritePos), bufLen))] = data[i];
        speedWritePos += 1.0;
        if (speedWritePos >= bufLen)
            speedWritePos -= bufLen;

        const int idx0 = wrapIndex (static_cast<int> (speedReadPos), bufLen);
        const int idx1 = wrapIndex (idx0 + 1, bufLen);
        const float frac = static_cast<float> (speedReadPos - std::floor (speedReadPos));
        data[i] = speedBuffer[static_cast<size_t> (idx0)] * (1.0f - frac)
                + speedBuffer[static_cast<size_t> (idx1)] * frac;
        speedReadPos += readIncrement;
    }

    while (speedReadPos >= bufLen)
        speedReadPos -= static_cast<double> (bufLen);
}

void VocalChangeEngine::processMonoVoice (const float* monoIn, float* monoOut, int numSamples)
{
    if (isVoiceTransformBypassed())
    {
        std::copy (monoIn, monoIn + numSamples, monoOut);
        return;
    }

    if (static_cast<int> (voiceScratch.size()) < numSamples)
        voiceScratch.assign (static_cast<size_t> (numSamples), 0.0f);

    if (settings.correction > 0.05f)
    {
        vocalplus::AutoTuneSettings at;
        at.enabled = true;
        at.retuneSpeed = 0.4f + settings.correction * 0.6f;
        at.toleranceCents = juce::jmap (settings.correction, 0.0f, 1.0f, 80.0f, 0.0f);
        at.formantPreserve = juce::jlimit (0.0f, 1.0f, 1.0f - std::abs (settings.formant - 1.0f) * 0.5f);
        at.rootNote = 0;
        at.scale = vocalplus::ScaleType::chromatic;
        at.detuneCents = 0.0f;
        autoTune.setSettings (at);
        autoTune.process (monoIn, voiceScratch.data(), numSamples);
    }
    else
    {
        std::copy (monoIn, monoIn + numSamples, voiceScratch.data());
    }

    pitchResampler.setPitchSemitones (settings.pitchShiftSemitones);
    pitchResampler.processBlock (voiceScratch.data(), monoOut, numSamples);

    applyFormantEQ (monoOut, numSamples);
    applySpeed (monoOut, numSamples);
    applyAiMorph (monoOut, numSamples, juce::jmax (settings.aiMorph, settings.robot * 0.55f));
    applyDistortion (monoOut, numSamples, settings.distortion);
    applyRobot (monoOut, numSamples, settings.robot);
    applyChorus (monoOut, numSamples, settings.chorus);
    applyFilter (monoOut, numSamples);

    const float makeup = juce::Decibels::decibelsToGain (6.0f);
    for (int i = 0; i < numSamples; ++i)
        monoOut[i] *= makeup;
}

void VocalChangeEngine::applyFormantEQ (float* data, int numSamples)
{
    if (std::abs (settings.formant - 1.0f) < 0.02f || numSamples <= 0)
        return;

    juce::dsp::AudioBlock<float> block (&data, 1, static_cast<size_t> (numSamples));
    juce::dsp::ProcessContextReplacing<float> ctx (block);
    formantLowShelf.process (ctx);
    formantHighShelf.process (ctx);
    formantPresence.process (ctx);
}

void VocalChangeEngine::applyAiMorph (float* data, int numSamples, float amount)
{
    if (amount <= 0.001f)
        return;

    const float carrierHz = 80.0f + amount * 220.0f;
    const float phaseInc = juce::MathConstants<float>::twoPi * carrierHz
                           / static_cast<float> (sampleRateHz);

    for (int i = 0; i < numSamples; ++i)
    {
        const float carrier = std::sin (aiPhase);
        const float envelope = 0.65f + 0.35f * std::sin (aiPhase * 0.37f);
        float wet = data[i] * (0.55f + 0.45f * carrier * envelope);
        wet += 0.12f * amount * std::tanh (data[i] * (2.0f + amount * 6.0f));
        wet = juce::jlimit (-1.5f, 1.5f, wet);
        data[i] = data[i] * (1.0f - amount) + wet * amount;

        aiPhase += phaseInc;
        if (aiPhase > juce::MathConstants<float>::twoPi)
            aiPhase -= juce::MathConstants<float>::twoPi;
    }
}

void VocalChangeEngine::applyDistortion (float* data, int numSamples, float amount)
{
    if (amount <= 0.001f)
        return;

    const float drive = 1.0f + amount * 10.0f;

    for (int i = 0; i < numSamples; ++i)
        data[i] = softClip (data[i], drive) * (1.0f - amount * 0.1f);
}

void VocalChangeEngine::applyRobot (float* data, int numSamples, float amount)
{
    if (amount <= 0.001f)
        return;

    const float carrierHz = 30.0f + amount * 90.0f;
    const float phaseInc = juce::MathConstants<float>::twoPi * carrierHz
                           / static_cast<float> (sampleRateHz);

    for (int i = 0; i < numSamples; ++i)
    {
        const float mod = 0.5f + 0.5f * std::sin (robotPhase);
        data[i] = data[i] * (1.0f - amount) + data[i] * mod * amount;

        const int bits = juce::jmax (4, static_cast<int> (14.0f - amount * 8.0f));
        const float step = 1.0f / static_cast<float> (1 << bits);
        data[i] = std::floor (data[i] / step + 0.5f) * step;

        robotPhase += phaseInc;
        if (robotPhase > juce::MathConstants<float>::twoPi)
            robotPhase -= juce::MathConstants<float>::twoPi;
    }
}

void VocalChangeEngine::applyChorus (float* data, int numSamples, float amount)
{
    if (amount <= 0.001f || chorusDelay.empty())
        return;

    const float rateHz = 0.4f + amount * 3.0f;
    const float depthSamples = amount * static_cast<float> (chorusDelay.size()) * 0.35f;
    const float phaseInc = juce::MathConstants<float>::twoPi * rateHz
                           / static_cast<float> (sampleRateHz);
    const int delayLen = static_cast<int> (chorusDelay.size());
    const float baseDelay = juce::jmin (40.0f, static_cast<float> (delayLen - 2));

    for (int i = 0; i < numSamples; ++i)
    {
        const float mod = std::sin (chorusPhase) * depthSamples;
        const float readPos = baseDelay + mod;
        const int idx0 = wrapIndex (static_cast<int> (readPos), delayLen);
        const int idx1 = wrapIndex (idx0 + 1, delayLen);
        const float frac = readPos - std::floor (readPos);
        const float delayed = chorusDelay[static_cast<size_t> (idx0)] * (1.0f - frac)
                            + chorusDelay[static_cast<size_t> (idx1)] * frac;

        chorusDelay[static_cast<size_t> (chorusWritePos)] = data[i];
        chorusWritePos = wrapIndex (chorusWritePos + 1, delayLen);

        data[i] = data[i] * (1.0f - amount) + delayed * amount;

        chorusPhase += phaseInc;
        if (chorusPhase > juce::MathConstants<float>::twoPi)
            chorusPhase -= juce::MathConstants<float>::twoPi;
    }
}

void VocalChangeEngine::applyFilter (float* data, int numSamples)
{
    if (settings.filterMode <= 0 || filterBand.coefficients == nullptr)
        return;

    juce::dsp::AudioBlock<float> block (&data, 1, static_cast<size_t> (numSamples));
    juce::dsp::ProcessContextReplacing<float> ctx (block);
    filterBand.process (ctx);
}

void VocalChangeEngine::applyReverb (juce::AudioBuffer<float>& buffer)
{
    if (settings.reverb <= 0.001f)
        return;

    reverbBuffer.makeCopyOf (buffer, true);
    juce::dsp::AudioBlock<float> block (reverbBuffer);
    juce::dsp::ProcessContextReplacing<float> ctx (block);
    reverb.process (ctx);

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        buffer.copyFrom (ch, 0, reverbBuffer, ch, 0, buffer.getNumSamples());
}

void VocalChangeEngine::process (juce::AudioBuffer<float>& buffer)
{
    if (! prepared || buffer.getNumSamples() <= 0)
        return;

    const int numSamples = buffer.getNumSamples();
    const int channels = buffer.getNumChannels();

    buffer.applyGain (juce::Decibels::decibelsToGain (juce::jlimit (-12.0f, 12.0f, settings.inputGainDb)));
    dryBuffer.makeCopyOf (buffer, true);

    float peakIn = 0.0f;
    for (int ch = 0; ch < channels; ++ch)
        peakIn = juce::jmax (peakIn, buffer.getMagnitude (ch, 0, numSamples));
    inputLevelDb = magnitudeToDb (peakIn);

    monoInBuffer.setSize (1, numSamples, false, false, true);
    monoOutBuffer.setSize (1, numSamples, false, false, true);
    auto* monoIn = monoInBuffer.getWritePointer (0);
    auto* monoOut = monoOutBuffer.getWritePointer (0);

    if (channels == 1)
        std::copy (buffer.getReadPointer (0), buffer.getReadPointer (0) + numSamples, monoIn);
    else
        for (int i = 0; i < numSamples; ++i)
            monoIn[i] = 0.5f * (buffer.getSample (0, i) + buffer.getSample (1, i));

    processMonoVoice (monoIn, monoOut, numSamples);

    for (int ch = 0; ch < channels; ++ch)
        buffer.copyFrom (ch, 0, monoOut, 0, numSamples);

    applyReverb (buffer);

    const float mix = juce::jlimit (0.0f, 1.0f, settings.mix);

    for (int ch = 0; ch < channels; ++ch)
    {
        auto* out = buffer.getWritePointer (ch);
        const auto* dry = dryBuffer.getReadPointer (ch);

        for (int i = 0; i < numSamples; ++i)
            out[i] = dry[i] + (out[i] - dry[i]) * mix;
    }

    buffer.applyGain (juce::Decibels::decibelsToGain (juce::jlimit (-12.0f, 12.0f, settings.outputGainDb)));

    float peakOut = 0.0f;
    for (int ch = 0; ch < channels; ++ch)
        peakOut = juce::jmax (peakOut, buffer.getMagnitude (ch, 0, numSamples));
    outputLevelDb = magnitudeToDb (peakOut);
}

} // namespace vocalchangeplus
