#pragma once

#include <JuceHeader.h>

namespace tuneplus
{

struct CompressorSettings
{
    bool enabled = true;
    float thresholdDb = -18.0f;
    float ratio = 3.0f;
    float attackMs = 12.0f;
    float releaseMs = 120.0f;
    float makeupDb = 4.0f;
    float kneeDb = 6.0f;
    float highPassHz = 80.0f;
    float mix = 1.0f;
};

class VocalCompressor
{
public:
    void prepare (double sampleRate, int maxBlockSize, int numChannels);
    void reset();

    void setSettings (const CompressorSettings& newSettings);
    void process (juce::AudioBuffer<float>& buffer);

    float getGainReductionDb() const noexcept { return gainReductionDb; }

private:
    CompressorSettings settings;
    juce::dsp::Compressor<float> compressor;
    juce::dsp::IIR::Filter<float> highPass;
    juce::dsp::ProcessSpec spec {};
    float gainReductionDb = 0.0f;
    bool prepared = false;
};

} // namespace tuneplus
