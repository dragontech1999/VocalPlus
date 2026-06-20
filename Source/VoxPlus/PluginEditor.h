#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/VoxPlusLookAndFeel.h"
#include "DSP/VoxPlusFxModes.h"
#include "../UI/VocalPlusEditorShell.h"

namespace voxplus
{

class VoxPlusAudioProcessorEditor : public juce::AudioProcessorEditor,
                                    private juce::Timer
{
public:
    static constexpr int kContentWidth  = 1240;
    static constexpr int kContentHeight = 880;

    explicit VoxPlusAudioProcessorEditor (VoxPlusAudioProcessor&);
    ~VoxPlusAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    class ScrollableContent;

    struct FxModuleUI
    {
        juce::ToggleButton bypass;
        juce::ComboBox modeBox;
        juce::Slider amount, tone;
        juce::Label amountLabel { {}, "Amount" }, toneLabel { {}, "Tone" };

        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> amountAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> toneAttachment;
    };

    void timerCallback() override;
    void loadPreset (int index);
    void layoutKnob (juce::Rectangle<int> area, juce::Slider& slider, juce::Label& label, bool large = false);
    void layoutFxModule (juce::Rectangle<int> area, FxModuleUI& fx);
    void paintContent (juce::Graphics& g);
    void layoutContent();
    void drawNeonPanel (juce::Graphics& g, juce::Rectangle<int> r, const juce::String& title) const;
    void setupFxModule (FxModuleUI& fx, int index, juce::AudioProcessorValueTreeState& apvts,
                        const std::function<void (juce::Component&)>& addToContent);

    VoxPlusAudioProcessor& processor;
    VoxPlusLookAndFeel lookAndFeel;

    vocalplus::ui::VocalPlusEditorShell editorShell;
    std::unique_ptr<ScrollableContent> scrollContent;

    juce::ComboBox presetBox;
    juce::Label presetLabel { {}, "Preset" };
    juce::Label presetDescLabel;
    juce::TextButton randomizeButton { "Randomize" };
    juce::Label inputMeterLabel, grMeterLabel, outputMeterLabel, pitchLabel;

    juce::ToggleButton voiceBypass { "Bypass" };
    juce::Slider voiceMixSlider, pitchSlider, formantSlider, unisonSlider, correctionSlider;
    juce::Label voiceMixLabel { {}, "Mix" }, pitchLabelCtrl { {}, "Pitch" };
    juce::Label formantLabel { {}, "Formant" }, unisonLabel { {}, "Unison" }, correctionLabel { {}, "Tune" };
    juce::ComboBox keyBox, scaleBox;
    juce::Label keyLabel { {}, "Key" }, scaleLabel { {}, "Scale" };

    std::array<FxModuleUI, 6> fxModules;

    juce::Slider autoLevelSlider, lowCutSlider, highCutSlider, globalMixSlider;
    juce::Slider doublingSlider, focusSlider, glueSlider, inputSlider, outputSlider;
    juce::Label autoLevelLabel { {}, "Auto" }, lowCutLabel { {}, "Low Cut" };
    juce::Label highCutLabel { {}, "High Cut" }, globalMixLabel { {}, "Mix" };
    juce::Label doublingLabel { {}, "Double" }, focusLabel { {}, "Focus" };
    juce::Label glueLabel { {}, "Glue" }, inputLabel { {}, "In" }, outputLabel { {}, "Out" };

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<ButtonAttachment> voiceBypassAttachment;
    std::unique_ptr<Attachment> voiceMixAttachment, pitchAttachment, formantAttachment;
    std::unique_ptr<Attachment> unisonAttachment, correctionAttachment;
    std::unique_ptr<ComboAttachment> keyAttachment, scaleAttachment;
    std::unique_ptr<Attachment> autoLevelAttachment, lowCutAttachment, highCutAttachment;
    std::unique_ptr<Attachment> globalMixAttachment, doublingAttachment, focusAttachment;
    std::unique_ptr<Attachment> glueAttachment, inputAttachment, outputAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoxPlusAudioProcessorEditor)
};

} // namespace voxplus
