#include "PluginEditor.h"
#include "ParameterIDs.h"

namespace voxplus
{

using namespace ParamIDs;

class VoxPlusAudioProcessorEditor::ScrollableContent : public juce::Component
{
public:
    explicit ScrollableContent (VoxPlusAudioProcessorEditor& ownerIn) : owner (ownerIn) {}

    void paint (juce::Graphics& g) override { owner.paintContent (g); }
    void resized() override { owner.layoutContent(); }

private:
    VoxPlusAudioProcessorEditor& owner;
};

namespace
{
    void styleLabel (juce::Label& label, bool secondary = false)
    {
        label.setJustificationType (juce::Justification::centred);
        label.setColour (juce::Label::textColourId,
                         secondary ? VoxPlusLookAndFeel::kTextSecondary
                                   : VoxPlusLookAndFeel::kTextPrimary);
        label.setFont (juce::FontOptions (secondary ? 10.0f : 11.0f));
    }

    void styleSlider (juce::Slider& slider, bool large = false)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, large ? 56 : 48, 16);
        slider.setColour (juce::Slider::textBoxTextColourId, VoxPlusLookAndFeel::kTextSecondary);
        slider.setScrollWheelEnabled (true);
    }
}

void VoxPlusAudioProcessorEditor::setupFxModule (FxModuleUI& fx, int index,
                                                 juce::AudioProcessorValueTreeState& apvts,
                                                 const std::function<void (juce::Component&)>& addToContent)
{
    fx.bypass.setButtonText ("On");
    addToContent (fx.bypass);
    addToContent (fx.modeBox);
    addToContent (fx.amount);
    addToContent (fx.tone);
    addToContent (fx.amountLabel);
    addToContent (fx.toneLabel);

    for (int m = 0; m < kFxModules[static_cast<size_t> (index)].modes.size(); ++m)
        fx.modeBox.addItem (kFxModules[static_cast<size_t> (index)].modes[m], m + 1);

    styleSlider (fx.amount, true);
    styleSlider (fx.tone, false);
    styleLabel (fx.amountLabel, true);
    styleLabel (fx.toneLabel, true);

    fx.bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, fxEnabled (index), fx.bypass);
    fx.modeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        apvts, fxMode (index), fx.modeBox);
    fx.amountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, fxAmount (index), fx.amount);
    fx.toneAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, fxTone (index), fx.tone);
}

VoxPlusAudioProcessorEditor::VoxPlusAudioProcessorEditor (VoxPlusAudioProcessor& p)
    : AudioProcessorEditor (&p),
      processor (p),
      editorShell (*this, VoxPlusLookAndFeel::kNeonGreen)
{
    setLookAndFeel (&lookAndFeel);

    scrollContent = std::make_unique<ScrollableContent> (*this);
    addAndMakeVisible (editorShell);
    editorShell.setScrollContent (scrollContent.get(), kContentWidth, kContentHeight);

    auto addToContent = [this] (juce::Component& c) { scrollContent->addAndMakeVisible (c); };

    for (int i = 0; i < processor.getPresetManager().getNumPresets(); ++i)
        presetBox.addItem (processor.getPresetManager().getPreset (i).name, i + 1);

    if (processor.getCurrentPresetIndex() >= 0)
        presetBox.setSelectedId (processor.getCurrentPresetIndex() + 1, juce::dontSendNotification);

    presetBox.onChange = [this] { loadPreset (presetBox.getSelectedItemIndex()); };

    randomizeButton.onClick = [this]
    {
        processor.randomizeSettings();
        presetBox.setSelectedId (0, juce::dontSendNotification);
        presetDescLabel.setText ("Randomized settings", juce::dontSendNotification);
    };

    styleLabel (presetLabel, true);
    styleLabel (presetDescLabel, true);
    styleLabel (inputMeterLabel);
    styleLabel (grMeterLabel, true);
    styleLabel (outputMeterLabel);
    styleLabel (pitchLabel, true);

    presetDescLabel.setText (processor.getPresetManager().getPreset (0).description,
                             juce::dontSendNotification);

    addToContent (presetBox);
    addToContent (presetLabel);
    addToContent (presetDescLabel);
    addToContent (randomizeButton);
    addToContent (inputMeterLabel);
    addToContent (grMeterLabel);
    addToContent (outputMeterLabel);
    addToContent (pitchLabel);

    auto& apvts = processor.getAPVTS();

    addToContent (voiceBypass);
    voiceBypassAttachment = std::make_unique<ButtonAttachment> (apvts, ParamIDs::voiceBypass, voiceBypass);

    juce::Slider* voiceSliders[] { &voiceMixSlider, &pitchSlider, &formantSlider, &unisonSlider, &correctionSlider };
    juce::Label* voiceLabels[] { &voiceMixLabel, &pitchLabelCtrl, &formantLabel, &unisonLabel, &correctionLabel };

    for (int i = 0; i < 5; ++i)
    {
        styleSlider (*voiceSliders[i], i > 0);
        styleLabel (*voiceLabels[i], true);
        addToContent (*voiceSliders[i]);
        addToContent (*voiceLabels[i]);
    }

    voiceMixAttachment = std::make_unique<Attachment> (apvts, ParamIDs::voiceMix, voiceMixSlider);
    pitchAttachment = std::make_unique<Attachment> (apvts, ParamIDs::pitchShift, pitchSlider);
    formantAttachment = std::make_unique<Attachment> (apvts, ParamIDs::formant, formantSlider);
    unisonAttachment = std::make_unique<Attachment> (apvts, ParamIDs::unison, unisonSlider);
    correctionAttachment = std::make_unique<Attachment> (apvts, ParamIDs::correction, correctionSlider);

    for (int i = 0; i < getNoteNames().size(); ++i)
        keyBox.addItem (getNoteNames()[i], i + 1);
    for (int i = 0; i < getTuneScaleNames().size(); ++i)
        scaleBox.addItem (getTuneScaleNames()[i], i + 1);

    addToContent (keyBox);
    addToContent (scaleBox);
    addToContent (keyLabel);
    addToContent (scaleLabel);
    styleLabel (keyLabel, true);
    styleLabel (scaleLabel, true);

    keyAttachment = std::make_unique<ComboAttachment> (apvts, ParamIDs::rootNote, keyBox);
    scaleAttachment = std::make_unique<ComboAttachment> (apvts, ParamIDs::tuneScale, scaleBox);

    for (int i = 0; i < 6; ++i)
        setupFxModule (fxModules[static_cast<size_t> (i)], i, apvts, addToContent);

    juce::Slider* bottomSliders[] { &autoLevelSlider, &lowCutSlider, &highCutSlider, &globalMixSlider,
                                    &doublingSlider, &focusSlider, &glueSlider, &inputSlider, &outputSlider };
    juce::Label* bottomLabels[] { &autoLevelLabel, &lowCutLabel, &highCutLabel, &globalMixLabel,
                                    &doublingLabel, &focusLabel, &glueLabel, &inputLabel, &outputLabel };

    for (int i = 0; i < 9; ++i)
    {
        styleSlider (*bottomSliders[i], false);
        styleLabel (*bottomLabels[i], true);
        addToContent (*bottomSliders[i]);
        addToContent (*bottomLabels[i]);
    }

    autoLevelAttachment = std::make_unique<Attachment> (apvts, ParamIDs::autoLevel, autoLevelSlider);
    lowCutAttachment = std::make_unique<Attachment> (apvts, ParamIDs::lowCut, lowCutSlider);
    highCutAttachment = std::make_unique<Attachment> (apvts, ParamIDs::highCut, highCutSlider);
    globalMixAttachment = std::make_unique<Attachment> (apvts, ParamIDs::globalMix, globalMixSlider);
    doublingAttachment = std::make_unique<Attachment> (apvts, ParamIDs::doubling, doublingSlider);
    focusAttachment = std::make_unique<Attachment> (apvts, ParamIDs::focusEQ, focusSlider);
    glueAttachment = std::make_unique<Attachment> (apvts, ParamIDs::superGlue, glueSlider);
    inputAttachment = std::make_unique<Attachment> (apvts, ParamIDs::inputGain, inputSlider);
    outputAttachment = std::make_unique<Attachment> (apvts, ParamIDs::outputGain, outputSlider);

    layoutContent();

    vocalplus::ui::VocalPlusEditorShell::configureEditorWindow (*this, kContentWidth, kContentHeight);
    startTimerHz (15);
}

VoxPlusAudioProcessorEditor::~VoxPlusAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void VoxPlusAudioProcessorEditor::drawNeonPanel (juce::Graphics& g, juce::Rectangle<int> r,
                                                 const juce::String& title) const
{
    g.setColour (VoxPlusLookAndFeel::kPanel);
    g.fillRoundedRectangle (r.toFloat(), 8.0f);
    g.setColour (VoxPlusLookAndFeel::kNeonGlow);
    g.drawRoundedRectangle (r.toFloat().expanded (0.5f), 8.0f, 2.0f);
    g.setColour (VoxPlusLookAndFeel::kNeonGreen);
    g.drawRoundedRectangle (r.toFloat(), 8.0f, 1.0f);

    auto titleArea = r.reduced (12, 8).removeFromTop (16);
    g.setFont (juce::FontOptions (10.5f).withStyle ("Medium"));
    g.drawText (title.toUpperCase(), titleArea, juce::Justification::centredLeft);
}

void VoxPlusAudioProcessorEditor::layoutKnob (juce::Rectangle<int> area, juce::Slider& slider,
                                              juce::Label& label, bool large)
{
    juce::ignoreUnused (large);
    auto block = area.reduced (3);
    label.setBounds (block.removeFromTop (13));
    slider.setBounds (block);
}

void VoxPlusAudioProcessorEditor::layoutFxModule (juce::Rectangle<int> area, FxModuleUI& fx)
{
    auto content = area.reduced (8, 28);
    fx.bypass.setBounds (content.removeFromTop (22));
    content.removeFromTop (4);
    fx.modeBox.setBounds (content.removeFromTop (22));
    content.removeFromTop (6);

    auto toneRow = content.removeFromBottom (content.getHeight() / 3);
    auto amountArea = content;

    layoutKnob (amountArea, fx.amount, fx.amountLabel, true);
    layoutKnob (toneRow, fx.tone, fx.toneLabel, false);
}

void VoxPlusAudioProcessorEditor::loadPreset (int index)
{
    if (index < 0)
        return;

    processor.applyPreset (index);
    presetDescLabel.setText (processor.getPresetManager().getPreset (index).description,
                             juce::dontSendNotification);
}

void VoxPlusAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (VoxPlusLookAndFeel::kBackground);
}

void VoxPlusAudioProcessorEditor::paintContent (juce::Graphics& g)
{
    g.fillAll (VoxPlusLookAndFeel::kBackground);

    g.setColour (VoxPlusLookAndFeel::kPanel);
    g.fillRect (0, 0, kContentWidth, 72);
    g.setColour (VoxPlusLookAndFeel::kNeonGreen);
    g.drawHorizontalLine (72, 0.0f, static_cast<float> (kContentWidth));

    g.setFont (juce::FontOptions (26.0f).withStyle ("Bold"));
    g.drawText ("VOX+", 18, 10, 90, 30, juce::Justification::centredLeft);

    g.setColour (VoxPlusLookAndFeel::kTextSecondary);
    g.setFont (juce::FontOptions (11.0f));
    g.drawText ("Vocal Multi-FX · Voice · Dynamics · Character · Filter · Delay · Reverb · Chop",
                18, 42, 620, 16, juce::Justification::centredLeft);

    drawNeonPanel (g, { 16, 84, 1208, 168 }, "Voice");
    drawNeonPanel (g, { 16, 264, 1208, 300 }, "Multi-FX");

    const int colW = 1208 / 6;
    for (int i = 0; i < 6; ++i)
    {
        const auto col = juce::Rectangle<int> (16 + i * colW, 264, colW, 300).reduced (4, 0);
        drawNeonPanel (g, col, kFxModules[static_cast<size_t> (i)].title);
    }

    drawNeonPanel (g, { 16, 576, 1208, 128 }, "Global");
}

void VoxPlusAudioProcessorEditor::resized()
{
    editorShell.setBounds (getLocalBounds());
    layoutContent();
}

void VoxPlusAudioProcessorEditor::layoutContent()
{
    presetLabel.setBounds (130, 18, 50, 16);
    presetBox.setBounds (130, 34, 260, 24);
    presetDescLabel.setBounds (130, 58, 560, 14);
    randomizeButton.setBounds (400, 32, 100, 26);

    inputMeterLabel.setBounds (kContentWidth - 380, 18, 90, 16);
    grMeterLabel.setBounds (kContentWidth - 280, 18, 90, 16);
    outputMeterLabel.setBounds (kContentWidth - 180, 18, 90, 16);
    pitchLabel.setBounds (kContentWidth - 90, 18, 80, 16);

    const juce::Rectangle<int> voicePanel (16, 84, 1208, 168);
    auto voiceContent = voicePanel.reduced (16, 32);

    auto voiceLeft = voiceContent.removeFromLeft (100);
    voiceBypass.setBounds (voiceLeft.removeFromTop (24));
    voiceLeft.removeFromTop (8);
    layoutKnob (voiceLeft, voiceMixSlider, voiceMixLabel);

    auto voiceKnobs = voiceContent.removeFromTop (voiceContent.getHeight() - 30);
    const int vk = voiceKnobs.getWidth() / 5;
    layoutKnob (voiceKnobs.removeFromLeft (vk), pitchSlider, pitchLabelCtrl, true);
    layoutKnob (voiceKnobs.removeFromLeft (vk), formantSlider, formantLabel, true);
    layoutKnob (voiceKnobs.removeFromLeft (vk), unisonSlider, unisonLabel, true);
    layoutKnob (voiceKnobs.removeFromLeft (vk), correctionSlider, correctionLabel, true);

    auto keyRow = voiceContent;
    keyLabel.setBounds (keyRow.removeFromLeft (28));
    keyBox.setBounds (keyRow.removeFromLeft (56).reduced (0, 2));
    keyRow.removeFromLeft (16);
    scaleLabel.setBounds (keyRow.removeFromLeft (38));
    scaleBox.setBounds (keyRow.removeFromLeft (140).reduced (0, 2));

    const juce::Rectangle<int> fxPanel (16, 264, 1208, 300);
    const int fxColW = fxPanel.getWidth() / 6;

    for (int i = 0; i < 6; ++i)
    {
        auto col = fxPanel.reduced (0, 0);
        col = col.withX (fxPanel.getX() + i * fxColW).withWidth (fxColW).reduced (6, 0);
        layoutFxModule (col, fxModules[static_cast<size_t> (i)]);
    }

    const juce::Rectangle<int> globalPanel (16, 576, 1208, 128);
    auto globalContent = globalPanel.reduced (16, 32);
    auto globalRow = globalContent;
    const int gk = globalRow.getWidth() / 9;

    layoutKnob (globalRow.removeFromLeft (gk), autoLevelSlider, autoLevelLabel);
    layoutKnob (globalRow.removeFromLeft (gk), lowCutSlider, lowCutLabel);
    layoutKnob (globalRow.removeFromLeft (gk), highCutSlider, highCutLabel);
    layoutKnob (globalRow.removeFromLeft (gk), globalMixSlider, globalMixLabel);
    layoutKnob (globalRow.removeFromLeft (gk), doublingSlider, doublingLabel);
    layoutKnob (globalRow.removeFromLeft (gk), focusSlider, focusLabel);
    layoutKnob (globalRow.removeFromLeft (gk), glueSlider, glueLabel);
    layoutKnob (globalRow.removeFromLeft (gk), inputSlider, inputLabel);
    layoutKnob (globalRow, outputSlider, outputLabel);
}

void VoxPlusAudioProcessorEditor::timerCallback()
{
    inputMeterLabel.setText ("IN " + juce::String (processor.getInputLevelDb(), 1) + " dB",
                             juce::dontSendNotification);
    grMeterLabel.setText ("GR " + juce::String (processor.getGainReductionDb(), 1) + " dB",
                          juce::dontSendNotification);
    outputMeterLabel.setText ("OUT " + juce::String (processor.getOutputLevelDb(), 1) + " dB",
                              juce::dontSendNotification);

    const float hz = processor.getDetectedPitchHz();
    if (hz > 0.0f)
    {
        const int midi = static_cast<int> (std::round (69.0f + 12.0f * std::log2 (hz / 440.0f)));
        const juce::String note = getNoteNames()[juce::jlimit (0, 11, midi % 12)];
        pitchLabel.setText (note + juce::String (midi / 12 - 1), juce::dontSendNotification);
    }
    else
    {
        pitchLabel.setText ("—", juce::dontSendNotification);
    }
}

} // namespace voxplus
