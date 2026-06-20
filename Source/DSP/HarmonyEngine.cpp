#include "HarmonyEngine.h"

namespace vocalplus
{

void HarmonyEngine::prepare (double sampleRate, int blockSize, int channels)
{
    maxBlockSize = blockSize;
    numChannels = channels;

    autoTune.prepare (sampleRate, blockSize);

    for (auto& voice : voices)
        voice.prepare (sampleRate, blockSize);

    dryBuffer.setSize (1, blockSize);
    correctedBuffer.setSize (1, blockSize);

    for (auto& harmony : harmonyBuffers)
        harmony.setSize (2, blockSize);

    leftScratch.setSize (1, blockSize);
    rightScratch.setSize (1, blockSize);

    dryDelayLine.assign (static_cast<size_t> (pitchLatencySamples + blockSize + 64), 0.0f);
    dryDelayWritePos = 0;

    reset();
}

void HarmonyEngine::reset()
{
    autoTune.reset();

    for (auto& voice : voices)
        voice.reset();

    std::fill (dryDelayLine.begin(), dryDelayLine.end(), 0.0f);
    dryDelayWritePos = 0;
}

void HarmonyEngine::setSettings (const HarmonyEngineSettings& newSettings)
{
    settings = newSettings;
    autoTune.setSettings (settings.autoTune);

    for (size_t i = 0; i < voices.size(); ++i)
        voices[i].setSettings (settings.voices[i]);
}

float HarmonyEngine::readLatencyAlignedDry (float sample) noexcept
{
    if (dryDelayLine.empty())
        return sample;

    dryDelayLine[static_cast<size_t> (dryDelayWritePos)] = sample;
    dryDelayWritePos = (dryDelayWritePos + 1) % static_cast<int> (dryDelayLine.size());

    if (! settings.autoTune.enabled)
        return sample;

    int readPos = dryDelayWritePos - pitchLatencySamples;
    while (readPos < 0)
        readPos += static_cast<int> (dryDelayLine.size());

    return dryDelayLine[static_cast<size_t> (readPos)];
}

void HarmonyEngine::process (juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int channels = buffer.getNumChannels();

    if (numSamples <= 0 || channels <= 0)
        return;

    dryBuffer.setSize (1, numSamples, false, false, true);
    correctedBuffer.setSize (1, numSamples, false, false, true);
    leftScratch.setSize (1, numSamples, false, false, true);
    rightScratch.setSize (1, numSamples, false, false, true);

    for (auto& harmony : harmonyBuffers)
        harmony.setSize (2, numSamples, false, false, true);

    float* dry = dryBuffer.getWritePointer (0);
    float* corrected = correctedBuffer.getWritePointer (0);
    float* left = leftScratch.getWritePointer (0);
    float* right = rightScratch.getWritePointer (0);

    if (channels == 1)
    {
        const float* input = buffer.getReadPointer (0);
        std::copy (input, input + numSamples, dry);
    }
    else
    {
        const float* leftIn = buffer.getReadPointer (0);
        const float* rightIn = buffer.getReadPointer (1);

        for (int i = 0; i < numSamples; ++i)
            dry[i] = 0.5f * (leftIn[i] + rightIn[i]);
    }

    autoTune.process (dry, corrected, numSamples);

    int activeHarmonyVoices = 0;
    for (const auto& v : settings.voices)
        if (v.enabled && std::abs (v.intervalSemitones) > 0)
            ++activeHarmonyVoices;

    for (size_t v = 0; v < voices.size(); ++v)
    {
        if (! settings.voices[v].enabled)
            continue;

        auto& harmony = harmonyBuffers[v];
        harmony.clear();
        voices[v].setBasePitchRatio (1.0f);
        voices[v].process (dry,
                           harmony.getWritePointer (0),
                           harmony.getWritePointer (1),
                           numSamples);
    }

    const float wet = settings.wetLevel;
    const float dryMix = settings.dryLevel;
    const float leadLevel = activeHarmonyVoices > 0 ? wet * 0.72f : wet;
    const float harmonyGain = activeHarmonyVoices > 0 ? 2.2f : 1.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        left[i] = corrected[i] * leadLevel;
        right[i] = corrected[i] * leadLevel;

        for (size_t v = 0; v < voices.size(); ++v)
        {
            if (! settings.voices[v].enabled)
                continue;

            left[i] += harmonyBuffers[v].getSample (0, i) * harmonyGain;
            right[i] += harmonyBuffers[v].getSample (1, i) * harmonyGain;
        }

        const float drySample = readLatencyAlignedDry (dry[i]);
        left[i] += drySample * dryMix;
        right[i] += drySample * dryMix;
    }

    const float width = settings.stereoWidth;
    for (int i = 0; i < numSamples; ++i)
    {
        const float mid = 0.5f * (left[i] + right[i]);
        const float side = 0.5f * (left[i] - right[i]) * width;
        left[i] = mid + side;
        right[i] = mid - side;
    }

    if (channels == 1)
    {
        float* out = buffer.getWritePointer (0);

        if (settings.blendToMono)
        {
            for (int i = 0; i < numSamples; ++i)
                out[i] = 0.5f * (left[i] + right[i]);
        }
        else
        {
            for (int i = 0; i < numSamples; ++i)
                out[i] = left[i];
        }
    }
    else
    {
        float* outL = buffer.getWritePointer (0);
        float* outR = buffer.getWritePointer (1);

        for (int i = 0; i < numSamples; ++i)
        {
            outL[i] = left[i];
            outR[i] = right[i];
        }
    }
}

void HarmonyEngine::getStemOutputs (juce::AudioBuffer<float>& dry,
                                    juce::AudioBuffer<float>& corrected,
                                    std::array<juce::AudioBuffer<float>, numHarmonyVoices>& harmonies,
                                    juce::AudioBuffer<float>& blend) const
{
    dry.makeCopyOf (dryBuffer, true);
    corrected.makeCopyOf (correctedBuffer, true);
    blend.setSize (2, dryBuffer.getNumSamples(), false, false, true);

    for (size_t i = 0; i < harmonies.size(); ++i)
        harmonies[i].makeCopyOf (harmonyBuffers[i], true);

    for (int i = 0; i < blend.getNumSamples(); ++i)
    {
        float l = correctedBuffer.getSample (0, i) * settings.wetLevel + dryBuffer.getSample (0, i) * settings.dryLevel;
        float r = l;

        for (const auto& harmony : harmonyBuffers)
        {
            l += harmony.getSample (0, i);
            r += harmony.getSample (1, i);
        }

        blend.setSample (0, i, l);
        blend.setSample (1, i, r);
    }
}

} // namespace vocalplus
