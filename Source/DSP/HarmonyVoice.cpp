#include "HarmonyVoice.h"

namespace vocalplus
{

void HarmonyVoice::prepare (double sr, int maxBlockSize)
{
    sampleRate = sr;
    shifter.prepare (sr, maxBlockSize);

    delayLine.setMaximumDelayInSamples (static_cast<int> (sr * 0.05) + 1);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sr;
    spec.maximumBlockSize = static_cast<juce::uint32> (maxBlockSize);
    spec.numChannels = 1;
    delayLine.prepare (spec);
    delayLine.setDelay (static_cast<float> (settings.delayMs * 0.001 * sampleRate));

    monoScratch.assign (static_cast<size_t> (maxBlockSize), 0.0f);
    reset();
}

void HarmonyVoice::reset()
{
    shifter.reset();
    delayLine.reset();
    vibratoPhase = 0.0f;
}

void HarmonyVoice::setSettings (const HarmonyVoiceSettings& newSettings)
{
    settings = newSettings;

    if (sampleRate > 0.0)
        delayLine.setDelay (static_cast<float> (settings.delayMs * 0.001 * sampleRate));
}

void HarmonyVoice::setBasePitchRatio (float ratio)
{
    basePitchRatio = ratio;
}

void HarmonyVoice::process (const float* input, float* leftOut, float* rightOut, int numSamples)
{
    if (! settings.enabled || numSamples <= 0 || input == nullptr || leftOut == nullptr || rightOut == nullptr)
        return;

    if (static_cast<int> (monoScratch.size()) < numSamples)
        monoScratch.resize (static_cast<size_t> (numSamples));

    const float intervalRatio = std::pow (2.0f, static_cast<float> (settings.intervalSemitones) / 12.0f);
    const float vibrato = 1.0f + settings.vibratoDepth * std::sin (juce::MathConstants<float>::twoPi * vibratoPhase);
    vibratoPhase += settings.vibratoRate * static_cast<float> (numSamples) / static_cast<float> (sampleRate);
    if (vibratoPhase >= 1.0f)
        vibratoPhase -= std::floor (vibratoPhase);

    const float pitchRatio = basePitchRatio * intervalRatio * vibrato;
    const bool useDelay = settings.delayMs > 0.5f;

    shifter.setPitchRatio (basePitchRatio * intervalRatio * vibrato);
    shifter.processBlock (input, monoScratch.data(), numSamples);

    const float panNorm = (settings.pan + 1.0f) * 0.5f;
    const float leftGain = settings.level * std::cos (panNorm * juce::MathConstants<float>::halfPi);
    const float rightGain = settings.level * std::sin (panNorm * juce::MathConstants<float>::halfPi);

    for (int i = 0; i < numSamples; ++i)
    {
        const float shifted = monoScratch[static_cast<size_t> (i)];

        if (useDelay)
        {
            delayLine.pushSample (0, shifted);
            const float sample = delayLine.popSample (0);
            leftOut[i] += sample * leftGain;
            rightOut[i] += sample * rightGain;
        }
        else
        {
            leftOut[i] += shifted * leftGain;
            rightOut[i] += shifted * rightGain;
        }
    }
}

} // namespace vocalplus
