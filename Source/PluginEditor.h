#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/VocalPlusLookAndFeel.h"
#include "UI/VocalPlusEditorShell.h"

namespace vocalplus
{

class VocalPlusAudioProcessorEditor : public juce::AudioProcessorEditor,
                                      private juce::Timer
{
public:
    static constexpr int kContentWidth  = 1020;
    static constexpr int kContentHeight = 760;

    explicit VocalPlusAudioProcessorEditor (VocalPlusAudioProcessor&);
    ~VocalPlusAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    class ScrollableContent;

    void timerCallback() override;
    void loadPreset (int index);
    void layoutKnob (juce::Rectangle<int> area, juce::Slider& slider, juce::Label& label);
    void paintContent (juce::Graphics& g);
    void layoutContent();

    struct VoiceControls
    {
        juce::ToggleButton enabled { "Enable" };
        juce::Slider interval, level, pan, formant, delay, vibrato;
        juce::Label title;
        juce::Label intervalLabel, levelLabel, panLabel, formantLabel, delayLabel, vibratoLabel;
    };

    void layoutVoiceKnobs (juce::Rectangle<int> area, VoiceControls& voice);

    VocalPlusAudioProcessor& processor;
    VocalPlusLookAndFeel lookAndFeel;

    ui::VocalPlusEditorShell editorShell;
    std::unique_ptr<ScrollableContent> scrollContent;

    juce::ComboBox presetBox;
    juce::Label presetLabel { {}, "Preset" };
    juce::TextButton captureButton { "Record Stems" };
    juce::TextButton exportButton { "Export WAV" };
    juce::Label statusLabel;
    juce::Label pitchLabel;
    juce::Label correctionLabel;

    juce::ToggleButton autoTuneToggle { "Auto-Tune" };
    juce::Slider retuneSlider, toleranceSlider, formantSlider, detuneSlider;
    juce::Label retuneLabel, toleranceLabel, formantLabel, detuneLabel;
    juce::ComboBox keyBox, scaleBox;
    juce::Label keyLabel { {}, "Key" }, scaleLabel { {}, "Scale" };

    juce::Slider drySlider, wetSlider, widthSlider;
    juce::Label dryLabel, wetLabel, widthLabel;
    juce::ToggleButton monoBlendToggle { "Mono Output" };

    std::array<VoiceControls, 3> voices;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<ButtonAttachment> autoTuneAttachment, monoBlendAttachment;
    std::unique_ptr<Attachment> retuneAttachment, toleranceAttachment, formantAttachment, detuneAttachment;
    std::unique_ptr<Attachment> dryAttachment, wetAttachment, widthAttachment;
    std::unique_ptr<ComboAttachment> keyAttachment, scaleAttachment;

    struct VoiceAttachments
    {
        std::unique_ptr<ButtonAttachment> enabled;
        std::unique_ptr<Attachment> interval, level, pan, formant, delay, vibrato;
    };
    std::array<VoiceAttachments, 3> voiceAttachments;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VocalPlusAudioProcessorEditor)
};

} // namespace vocalplus
