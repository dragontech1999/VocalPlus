#pragma once

#include <JuceHeader.h>
#include "DSP/VocalChangeEngine.h"
#include "DSP/VocalChangePresetManager.h"

namespace vocalchangeplus
{

class VocalChangePlusAudioProcessor : public juce::AudioProcessor
{
public:
    VocalChangePlusAudioProcessor();
    ~VocalChangePlusAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "VocalChange+"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.2; }

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() noexcept { return apvts; }
    VocalChangePresetManager& getPresetManager() noexcept { return presetManager; }

    void applyPreset (int index);
    void randomizeSettings();

    float getInputLevelDb() const noexcept { return engine.getInputLevelDb(); }
    float getOutputLevelDb() const noexcept { return engine.getOutputLevelDb(); }
    float getDetectedPitchHz() const noexcept { return engine.getDetectedPitchHz(); }
    int getCurrentPresetIndex() const noexcept { return currentPreset; }

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    VocalChangeSettings readSettingsFromParameters() const;
    void applySettingsToParameters (const VocalChangeSettings& s);

    juce::AudioProcessorValueTreeState apvts;
    VocalChangePresetManager presetManager;
    VocalChangeEngine engine;
    juce::Random random;
    int currentPreset = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VocalChangePlusAudioProcessor)
};

} // namespace vocalchangeplus
