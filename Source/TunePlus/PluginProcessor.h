#pragma once

#include <JuceHeader.h>
#include "DSP/TunePlusEngine.h"
#include "DSP/TunePlusPresetManager.h"
#include "../DSP/StemExporter.h"

namespace tuneplus
{

class TunePlusAudioProcessor : public juce::AudioProcessor
{
public:
    TunePlusAudioProcessor();
    ~TunePlusAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Tune+"; }
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
    TunePlusPresetManager& getPresetManager() noexcept { return presetManager; }

    void applyGenrePreset (int index);
    void applyCompressorPreset (int index);

    void beginStemCapture();
    void exportStems (std::function<void (bool, juce::String)> callback);

    float getDetectedPitchHz() const noexcept { return engine.getAutoTuneEngine().getCurrentPitchHz(); }
    float getCorrectionAmount() const noexcept { return engine.getAutoTuneEngine().getCorrectionAmount(); }
    float getCompressorGainReductionDb() const noexcept { return engine.getCompressorGainReductionDb(); }
    bool isStemCapturing() const noexcept { return stemExporter.isCapturing(); }

    int getCurrentGenreProgram() const noexcept { return currentGenreProgram; }
    int getCurrentCompressorProgram() const noexcept { return currentCompressorProgram; }

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    TunePlusSettings readSettingsFromParameters() const;
    void applyCompressorSettings (const CompressorSettings& c);

    juce::AudioProcessorValueTreeState apvts;
    TunePlusPresetManager presetManager;
    TunePlusEngine engine;
    vocalplus::StemExporter stemExporter;

    juce::AudioBuffer<float> stemDry;
    juce::AudioBuffer<float> stemCorrected;
    std::array<juce::AudioBuffer<float>, TunePlusEngine::numHarmonyVoices> stemHarmonies;
    juce::AudioBuffer<float> stemBlend;

    int currentGenreProgram = 0;
    int currentCompressorProgram = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TunePlusAudioProcessor)
};

} // namespace tuneplus
