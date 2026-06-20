#pragma once

#include <JuceHeader.h>
#include "../../DSP/AutoTuneEngine.h"
#include "../../DSP/ResamplePitchShifter.h"

namespace vocalchangeplus
{

struct VocalChangeSettings
{
    float inputGainDb = 0.0f;
    float outputGainDb = 0.0f;
    float mix = 1.0f;
    float pitchShiftSemitones = 0.0f;
    float formant = 1.0f;
    float speed = 1.0f;
    float distortion = 0.0f;
    float reverb = 0.0f;
    float robot = 0.0f;
    float chorus = 0.0f;
    int filterMode = 0;
    float correction = 0.0f;
    float aiMorph = 0.0f;

    bool operator== (const VocalChangeSettings& other) const noexcept
    {
        return inputGainDb == other.inputGainDb
            && outputGainDb == other.outputGainDb
            && mix == other.mix
            && pitchShiftSemitones == other.pitchShiftSemitones
            && formant == other.formant
            && speed == other.speed
            && distortion == other.distortion
            && reverb == other.reverb
            && robot == other.robot
            && chorus == other.chorus
            && filterMode == other.filterMode
            && correction == other.correction
            && aiMorph == other.aiMorph;
    }

    bool operator!= (const VocalChangeSettings& other) const noexcept
    {
        return ! (*this == other);
    }
};

class VocalChangeEngine
{
public:
    void prepare (double sampleRate, int maxBlockSize, int numChannels);
    void reset();
    void setSettings (const VocalChangeSettings& newSettings);
    void process (juce::AudioBuffer<float>& buffer);

    float getInputLevelDb() const noexcept { return inputLevelDb; }
    float getOutputLevelDb() const noexcept { return outputLevelDb; }
    float getDetectedPitchHz() const noexcept { return autoTune.getCurrentPitchHz(); }

private:
    void updateFilters();
    void processMonoVoice (const float* monoIn, float* monoOut, int numSamples);
    void applySpeed (float* data, int numSamples);
    void applyDistortion (float* data, int numSamples, float amount);
    void applyRobot (float* data, int numSamples, float amount);
    void applyChorus (float* data, int numSamples, float amount);
    void applyAiMorph (float* data, int numSamples, float amount);
    void applyFilter (float* data, int numSamples);
    void applyFormantEQ (float* data, int numSamples);
    void applyReverb (juce::AudioBuffer<float>& buffer);
    void updateFormantFilters();
    bool isVoiceTransformBypassed() const noexcept;

    VocalChangeSettings settings;
    VocalChangeSettings appliedSettings;
    vocalplus::AutoTuneEngine autoTune;
    vocalplus::ResamplePitchShifter pitchResampler;

    juce::dsp::IIR::Filter<float> filterBand;
    juce::dsp::IIR::Filter<float> formantLowShelf;
    juce::dsp::IIR::Filter<float> formantHighShelf;
    juce::dsp::IIR::Filter<float> formantPresence;
    juce::dsp::Reverb reverb;
    juce::dsp::Reverb::Parameters reverbParams;

    juce::AudioBuffer<float> dryBuffer;
    juce::AudioBuffer<float> monoInBuffer;
    juce::AudioBuffer<float> monoOutBuffer;
    juce::AudioBuffer<float> reverbBuffer;

    std::vector<float> speedBuffer;
    std::vector<float> voiceScratch;
    std::vector<float> chorusDelay;
    int chorusWritePos = 0;
    double speedWritePos = 0.0;
    double speedReadPos = 0.0;
    float robotPhase = 0.0f;
    float chorusPhase = 0.0f;
    float aiPhase = 0.0f;

    juce::dsp::ProcessSpec spec {};
    float inputLevelDb = -60.0f;
    float outputLevelDb = -60.0f;
    double sampleRateHz = 44100.0;
    int maxBlockSize = 512;
    bool prepared = false;
};

} // namespace vocalchangeplus
