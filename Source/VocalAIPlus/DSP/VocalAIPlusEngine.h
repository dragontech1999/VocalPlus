#pragma once

#include <JuceHeader.h>
#include "../../TunePlus/DSP/VocalCompressor.h"

namespace vocalaiplus
{

struct ChainSettings
{
    bool noiseEnabled = true;
    float noiseAmount = 0.55f;

    bool eqEnabled = true;
    float eqLowShelfDb = 0.0f;
    float eqLowMidDb = 0.0f;
    float eqPresenceDb = 0.0f;
    float eqHighShelfDb = 0.0f;

    bool compEnabled = true;
    float compThresholdDb = -18.0f;
    float compRatio = 3.5f;
    float compAttackMs = 8.0f;
    float compReleaseMs = 120.0f;
    float compMakeupDb = 4.0f;

    bool deEssEnabled = true;
    float deEssFreqHz = 6500.0f;
    float deEssReductionDb = 4.0f;
    float deEssSensitivity = 0.5f;

    bool exciterEnabled = false;
    float exciterAmount = 0.15f;
    float exciterMix = 0.25f;

    bool reverbEnabled = false;
    float reverbWet = 0.12f;
    float reverbRoomSize = 0.35f;

    bool limiterEnabled = true;
    float limiterCeilingDbTP = -1.0f;
    float limiterInputGainDb = 0.0f;
    float limiterTargetLUFS = -11.0f;

    float outputGainDb = 0.0f;
    float mix = 1.0f;
};

class VocalAIPlusEngine
{
public:
    void prepare (double sampleRate, int maxBlockSize, int numChannels);
    void reset();
    void setSettings (const ChainSettings& newSettings);
    void process (juce::AudioBuffer<float>& buffer);

    float getInputLevelDb() const noexcept { return inputLevelDb; }
    float getOutputLevelDb() const noexcept { return outputLevelDb; }
    float getGainReductionDb() const noexcept { return compressor.getGainReductionDb(); }

private:
    void updateFilters();
    void processExciter (juce::AudioBuffer<float>& buffer);
    void processDeEsser (juce::AudioBuffer<float>& buffer);

    ChainSettings settings;
    tuneplus::VocalCompressor compressor;
    tuneplus::VocalCompressor limiter;

    juce::dsp::IIR::Filter<float> noiseHpfL, noiseHpfR;
    juce::dsp::IIR::Filter<float> eqLowShelfL, eqLowShelfR;
    juce::dsp::IIR::Filter<float> eqLowMidL, eqLowMidR;
    juce::dsp::IIR::Filter<float> eqPresenceL, eqPresenceR;
    juce::dsp::IIR::Filter<float> eqHighShelfL, eqHighShelfR;
    juce::dsp::IIR::Filter<float> deEssL, deEssR;
    juce::dsp::Reverb reverb;
    juce::dsp::Reverb::Parameters reverbParams;

    juce::AudioBuffer<float> dryBuffer;
    juce::AudioBuffer<float> exciterScratch;
    juce::dsp::ProcessSpec spec {};
    float inputLevelDb = -60.0f;
    float outputLevelDb = -60.0f;
    float deEssEnvelope = 0.0f;
    double sampleRateHz = 44100.0;
    bool prepared = false;
};

} // namespace vocalaiplus
