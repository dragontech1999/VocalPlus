#pragma once

#include <JuceHeader.h>
#include "DSP/VocalAIPlusEngine.h"
#include "DSP/VocalAnalysisEngine.h"
#include "DSP/VocalAIPlusPresetManager.h"
#include "DSP/LocalMasteringAI.h"

namespace vocalaiplus
{

class VocalAIPlusAudioProcessor : public juce::AudioProcessor
{
public:
    VocalAIPlusAudioProcessor();
    ~VocalAIPlusAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "VocalAI+"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.3; }

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() noexcept { return apvts; }
    VocalAIPlusPresetManager& getPresetManager() noexcept { return presetManager; }

    void applyPreset (int index);
    void runAIMaster();
    VocalAnalysisSnapshot getAnalysisSnapshot() const;
    juce::String getLastRationale() const { return lastRationale; }

    float getInputLevelDb() const noexcept { return engine.getInputLevelDb(); }
    float getOutputLevelDb() const noexcept { return engine.getOutputLevelDb(); }
    float getGainReductionDb() const noexcept { return engine.getGainReductionDb(); }
    int getCurrentPresetIndex() const noexcept { return currentPreset; }

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    ChainSettings readSettingsFromParameters() const;
    void applySettingsToParameters (const ChainSettings& s);

    juce::AudioProcessorValueTreeState apvts;
    VocalAIPlusPresetManager presetManager;
    VocalAIPlusEngine engine;
    VocalAnalysisEngine analyzer;
    juce::String lastRationale;
    int currentPreset = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VocalAIPlusAudioProcessor)
};

} // namespace vocalaiplus
