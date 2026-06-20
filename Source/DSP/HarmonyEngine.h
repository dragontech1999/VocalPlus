#pragma once

#include "AutoTuneEngine.h"
#include "HarmonyVoice.h"
#include <array>

namespace vocalplus
{

struct HarmonyEngineSettings
{
    AutoTuneSettings autoTune;
    std::array<HarmonyVoiceSettings, 3> voices;
    float dryLevel = 0.35f;
    float wetLevel = 1.0f;
    float stereoWidth = 1.0f;
    bool blendToMono = true;
};

class HarmonyEngine
{
public:
    static constexpr int numHarmonyVoices = 3;
    static constexpr int pitchLatencySamples = 512;

    void prepare (double sampleRate, int maxBlockSize, int numChannels);
    void reset();

    void setSettings (const HarmonyEngineSettings& newSettings);
    void process (juce::AudioBuffer<float>& buffer);

    const AutoTuneEngine& getAutoTuneEngine() const noexcept { return autoTune; }

    void getStemOutputs (juce::AudioBuffer<float>& dry,
                         juce::AudioBuffer<float>& corrected,
                         std::array<juce::AudioBuffer<float>, numHarmonyVoices>& harmonies,
                         juce::AudioBuffer<float>& blend) const;

private:
    float readLatencyAlignedDry (float sample) noexcept;

    HarmonyEngineSettings settings;
    AutoTuneEngine autoTune;
    std::array<HarmonyVoice, numHarmonyVoices> voices;

    juce::AudioBuffer<float> dryBuffer;
    juce::AudioBuffer<float> correctedBuffer;
    std::array<juce::AudioBuffer<float>, numHarmonyVoices> harmonyBuffers;
    juce::AudioBuffer<float> leftScratch;
    juce::AudioBuffer<float> rightScratch;

    std::vector<float> dryDelayLine;
    int dryDelayWritePos = 0;

    int maxBlockSize = 512;
    int numChannels = 2;
};

} // namespace vocalplus
