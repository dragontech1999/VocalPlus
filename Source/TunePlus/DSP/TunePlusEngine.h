#pragma once

#include "../../DSP/HarmonyEngine.h"
#include "VocalCompressor.h"

namespace tuneplus
{

struct TunePlusSettings
{
    vocalplus::HarmonyEngineSettings harmony;
    CompressorSettings compressor;
};

class TunePlusEngine
{
public:
    static constexpr int numHarmonyVoices = vocalplus::HarmonyEngine::numHarmonyVoices;

    void prepare (double sampleRate, int maxBlockSize, int numChannels);
    void reset();

    void setSettings (const TunePlusSettings& newSettings);
    void process (juce::AudioBuffer<float>& buffer);

    const vocalplus::AutoTuneEngine& getAutoTuneEngine() const noexcept { return harmony.getAutoTuneEngine(); }
    float getCompressorGainReductionDb() const noexcept { return compressor.getGainReductionDb(); }

    void getStemOutputs (juce::AudioBuffer<float>& dry,
                         juce::AudioBuffer<float>& corrected,
                         std::array<juce::AudioBuffer<float>, numHarmonyVoices>& harmonies,
                         juce::AudioBuffer<float>& blend) const;

private:
    vocalplus::HarmonyEngine harmony;
    VocalCompressor compressor;
    TunePlusSettings settings;
    bool prepared = false;
};

} // namespace tuneplus
