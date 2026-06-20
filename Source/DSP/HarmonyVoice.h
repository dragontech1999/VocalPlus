#pragma once

#include "ResamplePitchShifter.h"

namespace vocalplus
{

struct HarmonyVoiceSettings
{
    bool enabled = false;
    int intervalSemitones = 4;
    float level = 0.65f;
    float pan = 0.0f;
    float formantShift = 1.0f;
    float delayMs = 0.0f;
    float vibratoDepth = 0.0f;
    float vibratoRate = 5.0f;
};

class HarmonyVoice
{
public:
    void prepare (double sampleRate, int maxBlockSize);
    void reset();

    void setSettings (const HarmonyVoiceSettings& newSettings);
    void setBasePitchRatio (float ratio);

    void process (const float* input, float* leftOut, float* rightOut, int numSamples);

private:
    HarmonyVoiceSettings settings;
    ResamplePitchShifter shifter;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine { 48000 };
    float basePitchRatio = 1.0f;
    float vibratoPhase = 0.0f;
    double sampleRate = 44100.0;
    std::vector<float> monoScratch;
};

} // namespace vocalplus
