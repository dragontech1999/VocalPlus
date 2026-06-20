#pragma once

#include <JuceHeader.h>
#include "../../TunePlus/DSP/VocalCompressor.h"
#include "../../DSP/AutoTuneEngine.h"
#include "../../DSP/PhaseVocoderPitchShifter.h"

namespace voxplus
{

struct FxModuleSettings
{
    bool enabled = true;
    int mode = 0;
    float amount = 0.0f;
    float tone = 0.5f;
};

struct VoxPlusSettings
{
    float inputGainDb = 0.0f;
    float outputGainDb = 0.0f;
    float globalMix = 1.0f;
    float autoLevel = 0.0f;
    float lowCutHz = 80.0f;
    float highCutHz = 16000.0f;
    float doubling = 0.0f;
    float focusEQ = 0.5f;
    float superGlue = 0.0f;

    bool voiceBypass = false;
    float voiceMix = 1.0f;
    float pitchShiftSemitones = 0.0f;
    float formant = 1.0f;
    float unison = 0.0f;
    float correction = 0.0f;
    int rootNote = 0;
    int tuneScale = 1;

    std::array<FxModuleSettings, 6> fx {};
};

class VoxPlusEngine
{
public:
    void prepare (double sampleRate, int maxBlockSize, int numChannels);
    void reset();
    void setSettings (const VoxPlusSettings& newSettings);
    void process (juce::AudioBuffer<float>& buffer);

    float getInputLevelDb() const noexcept { return inputLevelDb; }
    float getOutputLevelDb() const noexcept { return outputLevelDb; }
    float getGainReductionDb() const noexcept { return compressor.getGainReductionDb(); }
    float getDetectedPitchHz() const noexcept { return autoTune.getCurrentPitchHz(); }

private:
    void updateFilters();
    void processVoice (juce::AudioBuffer<float>& buffer);
    void processFxModule (int index, juce::AudioBuffer<float>& buffer);
    void applyDynamics (juce::AudioBuffer<float>& buffer, const FxModuleSettings& fx);
    void applyCharacter (juce::AudioBuffer<float>& buffer, const FxModuleSettings& fx);
    void applyFilter (juce::AudioBuffer<float>& buffer, const FxModuleSettings& fx);
    void applyDelay (juce::AudioBuffer<float>& buffer, const FxModuleSettings& fx);
    void applyReverb (juce::AudioBuffer<float>& buffer, const FxModuleSettings& fx);
    void applyChop (juce::AudioBuffer<float>& buffer, const FxModuleSettings& fx);
    tuneplus::CompressorSettings dynamicsToCompressor (const FxModuleSettings& fx) const;

    VoxPlusSettings settings;
    tuneplus::VocalCompressor compressor;

    vocalplus::AutoTuneEngine autoTune;
    vocalplus::PhaseVocoderPitchShifter pitchShifter;
    vocalplus::PhaseVocoderPitchShifter unisonShifterL;
    vocalplus::PhaseVocoderPitchShifter unisonShifterR;

    juce::dsp::IIR::Filter<float> lowCutFilter;
    juce::dsp::IIR::Filter<float> highCutFilter;
    juce::dsp::IIR::Filter<float> focusLow;
    juce::dsp::IIR::Filter<float> focusHigh;
    juce::dsp::IIR::Filter<float> fxFilterL;
    juce::dsp::IIR::Filter<float> fxFilterR;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLineL { 96000 };
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLineR { 96000 };
    juce::dsp::Reverb reverb;
    juce::dsp::Reverb::Parameters reverbParams;

    juce::AudioBuffer<float> voiceDry;
    juce::AudioBuffer<float> globalDry;
    juce::AudioBuffer<float> monoScratch;
    juce::AudioBuffer<float> monoScratch2;
    std::vector<float> monoIn;
    std::vector<float> monoOut;

    juce::dsp::ProcessSpec spec {};
    float inputLevelDb = -60.0f;
    float outputLevelDb = -60.0f;
    float chopPhase = 0.0f;
    double sampleRateHz = 44100.0;
    bool prepared = false;
};

} // namespace voxplus
