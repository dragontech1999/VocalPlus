#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/VocalChangePlusLookAndFeel.h"
#include "../UI/VocalPlusEditorShell.h"

namespace vocalchangeplus
{

class VocalChangePlusAudioProcessorEditor : public juce::AudioProcessorEditor,
                                            private juce::Timer
{
public:
    static constexpr int kContentWidth  = 960;
    static constexpr int kContentHeight = 620;

    explicit VocalChangePlusAudioProcessorEditor (VocalChangePlusAudioProcessor&);
    ~VocalChangePlusAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    class PresetButton;
    class ScrollableContent;

    void timerCallback() override;
    void loadPreset (int index);
    void layoutKnob (juce::Rectangle<int> area, juce::Slider& slider, juce::Label& label);
    void paintContent (juce::Graphics& g);
    void layoutContent();
    void drawNeonPanel (juce::Graphics& g, juce::Rectangle<int> r, const juce::String& title) const;
    void rebuildPresetButtons();

    VocalChangePlusAudioProcessor& processor;
    VocalChangePlusLookAndFeel lookAndFeel;
    vocalplus::ui::VocalPlusEditorShell editorShell;
    std::unique_ptr<ScrollableContent> scrollContent;

    juce::Label titleLabel { {}, "VocalChange+" };
    juce::Label subtitleLabel { {}, "Real-time voice transformation by Neeberman" };
    juce::Label presetDescLabel;
    juce::TextButton randomizeButton { "Randomize" };

    juce::Label inputMeterLabel, outputMeterLabel, pitchLabel;

    juce::Slider pitchSlider, formantSlider, speedSlider, mixSlider;
    juce::Slider distortionSlider, reverbSlider, robotSlider, chorusSlider;
    juce::Slider correctionSlider, inputSlider, outputSlider, aiMorphSlider;
    juce::ComboBox filterBox;

    juce::Label pitchLabelCtrl { {}, "Pitch" }, formantLabel { {}, "Formant" };
    juce::Label speedLabel { {}, "Speed" }, mixLabelCtrl { {}, "Mix" };
    juce::Label distortionLabel { {}, "Distort" }, reverbLabel { {}, "Reverb" };
    juce::Label robotLabel { {}, "Robot" }, chorusLabel { {}, "Chorus" };
    juce::Label correctionLabel { {}, "Tune" }, filterLabel { {}, "Filter" };
    juce::Label inputLabel { {}, "In" }, outputLabel { {}, "Out" }, aiMorphLabel { {}, "AI Morph" };

    juce::OwnedArray<PresetButton> presetButtons;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<Attachment> pitchAttachment, formantAttachment, speedAttachment, mixAttachment;
    std::unique_ptr<Attachment> distortionAttachment, reverbAttachment, robotAttachment, chorusAttachment;
    std::unique_ptr<Attachment> correctionAttachment, inputAttachment, outputAttachment, aiMorphAttachment;
    std::unique_ptr<ComboAttachment> filterAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VocalChangePlusAudioProcessorEditor)
};

} // namespace vocalchangeplus
