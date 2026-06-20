#pragma once

#include <JuceHeader.h>
#include "DSP/EQEngine.h"
#include "DSP/SmartEQAssistant.h"
#include "DSP/EQMatcher.h"

namespace eqplus
{

class EQPlusAudioProcessor : public juce::AudioProcessor
{
public:
    EQPlusAudioProcessor();
    ~EQPlusAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "EQ+"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 8; }
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() noexcept { return apvts; }
    EQEngine& getEngine() noexcept { return engine; }

    void applyAISuggestions();
    void applyEQMatch();
    void captureReferenceSpectrum();
    void sketchEQCurve (const std::vector<std::pair<float, float>>& points);
    void copyBandsToClipboard();
    void pasteBandsFromClipboard();
    void swapABState();

    void applyDeEss();
    void applySpectrumGrab (int bandIndex, float freq, float gainDb, bool dynamic);

    juce::String getStatusMessage() const noexcept { return statusMessage; }

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    void setChoiceParam (const juce::String& id, int index);
    void setBand (int i, const BandSettings& band);
    void clearAllBands();
    std::array<BandSettings, kMaxBands> readBandsFromParameters() const;
    EQGlobalSettings readGlobalSettings() const;
    void syncEngineFromParameters();
    void loadFactoryPreset (int index);

    juce::AudioProcessorValueTreeState apvts;
    EQEngine engine;
    SmartEQAssistant aiAssistant;
    EQMatcher eqMatcher;

    int currentProgram = 0;
    juce::ValueTree stateA;
    juce::ValueTree stateB;
    bool showingA = true;
    juce::String statusMessage;
    std::array<float, kMaxBands> bandSlopes {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EQPlusAudioProcessor)
};

} // namespace eqplus
