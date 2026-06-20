#pragma once

#include <JuceHeader.h>
#include "DSP/HarmonyEngine.h"
#include "DSP/PresetManager.h"
#include "DSP/StemExporter.h"

namespace vocalplus
{

class VocalPlusAudioProcessor : public juce::AudioProcessor
{
public:
    VocalPlusAudioProcessor();
    ~VocalPlusAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Vocal+"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.05; }

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() noexcept { return apvts; }
    PresetManager& getPresetManager() noexcept { return presetManager; }

    void applyPreset (int index);
    void beginStemCapture();
    void exportStems (std::function<void (bool, juce::String)> callback);

    float getDetectedPitchHz() const noexcept { return engine.getAutoTuneEngine().getCurrentPitchHz(); }
    float getCorrectionAmount() const noexcept { return engine.getAutoTuneEngine().getCorrectionAmount(); }
    bool isStemCapturing() const noexcept { return stemExporter.isCapturing(); }

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    HarmonyEngineSettings readSettingsFromParameters() const;

    juce::AudioProcessorValueTreeState apvts;
    PresetManager presetManager;
    HarmonyEngine engine;
    StemExporter stemExporter;

    juce::AudioBuffer<float> stemDry;
    juce::AudioBuffer<float> stemCorrected;
    std::array<juce::AudioBuffer<float>, HarmonyEngine::numHarmonyVoices> stemHarmonies;
    juce::AudioBuffer<float> stemBlend;

    int currentProgram = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VocalPlusAudioProcessor)
};

} // namespace vocalplus
