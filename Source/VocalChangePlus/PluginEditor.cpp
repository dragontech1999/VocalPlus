#include "PluginEditor.h"
#include "ParameterIDs.h"

namespace vocalchangeplus
{

using namespace ParamIDs;

class VocalChangePlusAudioProcessorEditor::ScrollableContent : public juce::Component
{
public:
    explicit ScrollableContent (VocalChangePlusAudioProcessorEditor& ownerIn) : owner (ownerIn) {}

    void paint (juce::Graphics& g) override { owner.paintContent (g); }
    void resized() override { owner.layoutContent(); }

private:
    VocalChangePlusAudioProcessorEditor& owner;
};

class VocalChangePlusAudioProcessorEditor::PresetButton : public juce::TextButton
{
public:
    PresetButton (VocalChangePlusAudioProcessorEditor& ownerIn, int indexIn)
        : owner (ownerIn), presetIndex (indexIn)
    {
        setClickingTogglesState (true);
        setRadioGroupId (9001);
    }

    void setPresetInfo (const juce::String& emoji, const juce::String& name)
    {
        emojiText = emoji;
        voiceName = name;
        setButtonText (emoji + " " + name);
    }

    void paintButton (juce::Graphics& g, bool highlighted, bool down) override
    {
        auto bounds = getLocalBounds().toFloat().reduced (1.0f);
        const bool active = getToggleState();

        g.setColour (active ? VocalChangePlusLookAndFeel::kNeonRed.withAlpha (0.35f)
                            : (highlighted ? VocalChangePlusLookAndFeel::kPresetHover
                                           : VocalChangePlusLookAndFeel::kPresetOff));
        g.fillRoundedRectangle (bounds, 5.0f);

        g.setColour (active ? VocalChangePlusLookAndFeel::kNeonRed
                            : VocalChangePlusLookAndFeel::kPanelBorder);
        g.drawRoundedRectangle (bounds, 5.0f, active ? 1.5f : 1.0f);

        if (active)
        {
            g.setColour (VocalChangePlusLookAndFeel::kNeonGlow);
            g.drawRoundedRectangle (bounds.expanded (1.0f), 6.0f, 2.0f);
        }

        g.setColour (VocalChangePlusLookAndFeel::kTextPrimary);
        g.setFont (juce::FontOptions (10.5f));
        g.drawText (getButtonText(), bounds.reduced (4.0f), juce::Justification::centred, true);
        juce::ignoreUnused (down);
    }

    int presetIndex = 0;

private:
    VocalChangePlusAudioProcessorEditor& owner;
    juce::String emojiText, voiceName;
};

namespace
{
    void styleLabel (juce::Label& label, bool secondary = false)
    {
        label.setJustificationType (juce::Justification::centred);
        label.setColour (juce::Label::textColourId,
                         secondary ? VocalChangePlusLookAndFeel::kTextSecondary
                                   : VocalChangePlusLookAndFeel::kTextPrimary);
        label.setFont (juce::FontOptions (secondary ? 10.0f : 11.0f));
    }

    void styleSlider (juce::Slider& slider)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 52, 16);
        slider.setColour (juce::Slider::textBoxTextColourId, VocalChangePlusLookAndFeel::kTextSecondary);
        slider.setScrollWheelEnabled (true);
    }
}

VocalChangePlusAudioProcessorEditor::VocalChangePlusAudioProcessorEditor (VocalChangePlusAudioProcessor& p)
    : AudioProcessorEditor (&p),
      processor (p),
      editorShell (*this, VocalChangePlusLookAndFeel::kNeonRed)
{
    setLookAndFeel (&lookAndFeel);

    scrollContent = std::make_unique<ScrollableContent> (*this);
    addAndMakeVisible (editorShell);
    editorShell.setScrollContent (scrollContent.get(), kContentWidth, kContentHeight);

    auto addToContent = [this] (juce::Component& c) { scrollContent->addAndMakeVisible (c); };

    titleLabel.setFont (juce::FontOptions (26.0f).withStyle ("Bold"));
    titleLabel.setColour (juce::Label::textColourId, VocalChangePlusLookAndFeel::kNeonRed);
    titleLabel.setJustificationType (juce::Justification::centredLeft);
    addToContent (titleLabel);

    subtitleLabel.setFont (juce::FontOptions (11.0f));
    subtitleLabel.setColour (juce::Label::textColourId, VocalChangePlusLookAndFeel::kTextSecondary);
    subtitleLabel.setJustificationType (juce::Justification::centredLeft);
    addToContent (subtitleLabel);

    presetDescLabel.setFont (juce::FontOptions (11.0f));
    presetDescLabel.setColour (juce::Label::textColourId, VocalChangePlusLookAndFeel::kTextSecondary);
    presetDescLabel.setJustificationType (juce::Justification::centredLeft);
    presetDescLabel.setText (processor.getPresetManager().getPreset (0).description, juce::dontSendNotification);
    addToContent (presetDescLabel);

    randomizeButton.onClick = [this]
    {
        processor.randomizeSettings();
        for (auto* btn : presetButtons)
            btn->setToggleState (false, juce::dontSendNotification);
        presetDescLabel.setText ("Randomized voice settings", juce::dontSendNotification);
    };
    addToContent (randomizeButton);

    styleLabel (inputMeterLabel);
    styleLabel (outputMeterLabel);
    styleLabel (pitchLabel, true);
    addToContent (inputMeterLabel);
    addToContent (outputMeterLabel);
    addToContent (pitchLabel);

    for (auto* slider : { &pitchSlider, &formantSlider, &speedSlider, &mixSlider,
                          &distortionSlider, &reverbSlider, &robotSlider, &chorusSlider,
                          &correctionSlider, &inputSlider, &outputSlider, &aiMorphSlider })
    {
        styleSlider (*slider);
        addToContent (*slider);
    }

    for (auto* label : { &pitchLabelCtrl, &formantLabel, &speedLabel, &mixLabelCtrl,
                         &distortionLabel, &reverbLabel, &robotLabel, &chorusLabel,
                         &correctionLabel, &filterLabel, &inputLabel, &outputLabel, &aiMorphLabel })
    {
        styleLabel (*label, label == &filterLabel);
        addToContent (*label);
    }

    filterBox.addItemList ({ "Off", "Phone", "Radio", "Space", "Cave" }, 1);
    addToContent (filterBox);

    auto& apvts = processor.getAPVTS();

    pitchAttachment = std::make_unique<Attachment> (apvts, pitchShift, pitchSlider);
    formantAttachment = std::make_unique<Attachment> (apvts, formant, formantSlider);
    speedAttachment = std::make_unique<Attachment> (apvts, speed, speedSlider);
    mixAttachment = std::make_unique<Attachment> (apvts, mix, mixSlider);
    distortionAttachment = std::make_unique<Attachment> (apvts, distortion, distortionSlider);
    reverbAttachment = std::make_unique<Attachment> (apvts, reverb, reverbSlider);
    robotAttachment = std::make_unique<Attachment> (apvts, robot, robotSlider);
    chorusAttachment = std::make_unique<Attachment> (apvts, chorus, chorusSlider);
    correctionAttachment = std::make_unique<Attachment> (apvts, correction, correctionSlider);
    inputAttachment = std::make_unique<Attachment> (apvts, inputGain, inputSlider);
    outputAttachment = std::make_unique<Attachment> (apvts, outputGain, outputSlider);
    aiMorphAttachment = std::make_unique<Attachment> (apvts, aiMorph, aiMorphSlider);
    filterAttachment = std::make_unique<ComboAttachment> (apvts, filterMode, filterBox);

    rebuildPresetButtons();

    layoutContent();
    vocalplus::ui::VocalPlusEditorShell::configureEditorWindow (*this, kContentWidth, kContentHeight);
    startTimerHz (15);
}

VocalChangePlusAudioProcessorEditor::~VocalChangePlusAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void VocalChangePlusAudioProcessorEditor::rebuildPresetButtons()
{
    presetButtons.clear();

    for (int i = 0; i < processor.getPresetManager().getNumPresets(); ++i)
    {
        const auto& preset = processor.getPresetManager().getPreset (i);
        auto* btn = presetButtons.add (new PresetButton (*this, i));
        btn->setPresetInfo (preset.emoji, preset.name);
        btn->setToggleState (i == processor.getCurrentPresetIndex(), juce::dontSendNotification);
        btn->onClick = [this, i, btn]
        {
            loadPreset (i);
            for (auto* other : presetButtons)
                other->setToggleState (other == btn, juce::dontSendNotification);
        };
        scrollContent->addAndMakeVisible (btn);
    }
}

void VocalChangePlusAudioProcessorEditor::loadPreset (int index)
{
    processor.applyPreset (index);
    presetDescLabel.setText (processor.getPresetManager().getPreset (index).description,
                             juce::dontSendNotification);
}

void VocalChangePlusAudioProcessorEditor::timerCallback()
{
    const auto inDb = processor.getInputLevelDb();
    const auto outDb = processor.getOutputLevelDb();
    const auto pitchHz = processor.getDetectedPitchHz();

    inputMeterLabel.setText ("In " + juce::String (inDb, 1) + " dB", juce::dontSendNotification);
    outputMeterLabel.setText ("Out " + juce::String (outDb, 1) + " dB", juce::dontSendNotification);
    pitchLabel.setText (pitchHz > 20.0f ? juce::String (pitchHz, 1) + " Hz" : "— Hz",
                        juce::dontSendNotification);
}

void VocalChangePlusAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (VocalChangePlusLookAndFeel::kBackground);
}

void VocalChangePlusAudioProcessorEditor::paintContent (juce::Graphics& g)
{
    g.fillAll (VocalChangePlusLookAndFeel::kBackground);

    g.setColour (VocalChangePlusLookAndFeel::kNeonGlow);
    g.fillRect (16, 55, kContentWidth - 32, 1);

    drawNeonPanel (g, { 16, 64, kContentWidth - 32, 220 }, "Voice Presets");
    drawNeonPanel (g, { 16, 292, kContentWidth - 32, 200 }, "Voice Lab");
    drawNeonPanel (g, { 16, 500, kContentWidth - 32, 110 }, "Effects");
}

void VocalChangePlusAudioProcessorEditor::drawNeonPanel (juce::Graphics& g, juce::Rectangle<int> r,
                                                         const juce::String& title) const
{
    g.setColour (VocalChangePlusLookAndFeel::kPanel);
    g.fillRoundedRectangle (r.toFloat(), 8.0f);
    g.setColour (VocalChangePlusLookAndFeel::kPanelBorder);
    g.drawRoundedRectangle (r.toFloat(), 8.0f, 1.0f);

    auto titleBar = r.removeFromTop (24).reduced (10, 0);
    g.setColour (VocalChangePlusLookAndFeel::kNeonRed);
    g.setFont (juce::FontOptions (11.0f).withStyle ("Bold"));
    g.drawText (title, titleBar, juce::Justification::centredLeft, true);
}

void VocalChangePlusAudioProcessorEditor::layoutKnob (juce::Rectangle<int> area,
                                                      juce::Slider& slider, juce::Label& label)
{
    label.setBounds (area.removeFromTop (16));
    slider.setBounds (area);
}

void VocalChangePlusAudioProcessorEditor::resized()
{
    editorShell.setBounds (getLocalBounds());
    layoutContent();
}

void VocalChangePlusAudioProcessorEditor::layoutContent()
{
    auto bounds = juce::Rectangle<int> (0, 0, kContentWidth, kContentHeight).reduced (16, 8);

    auto header = bounds.removeFromTop (48);
    titleLabel.setBounds (header.removeFromLeft (220));
    subtitleLabel.setBounds (header.removeFromLeft (260));
    randomizeButton.setBounds (header.removeFromRight (100).reduced (0, 10));
    inputMeterLabel.setBounds (header.removeFromRight (90));
    outputMeterLabel.setBounds (header.removeFromRight (90));
    pitchLabel.setBounds (header.removeFromRight (70));

    bounds.removeFromTop (8);
    auto presetPanel = bounds.removeFromTop (212);
    presetDescLabel.setBounds (presetPanel.removeFromBottom (20).reduced (12, 0));

    auto grid = presetPanel.reduced (12, 28);
    const int cols = 5;
    const int rows = 4;
    const int cellW = grid.getWidth() / cols;
    const int cellH = grid.getHeight() / rows;

    for (int i = 0; i < presetButtons.size(); ++i)
    {
        const int col = i % cols;
        const int row = i / cols;
        presetButtons[i]->setBounds (grid.getX() + col * cellW + 2,
                                     grid.getY() + row * cellH + 2,
                                     cellW - 4, cellH - 4);
    }

    bounds.removeFromTop (8);
    auto voicePanel = bounds.removeFromTop (192);
    auto voiceGrid = voicePanel.reduced (12, 28);
    const int knobW = voiceGrid.getWidth() / 6;

    auto row1 = voiceGrid.removeFromTop (voiceGrid.getHeight() / 2);
    layoutKnob (row1.removeFromLeft (knobW), pitchSlider, pitchLabelCtrl);
    layoutKnob (row1.removeFromLeft (knobW), formantSlider, formantLabel);
    layoutKnob (row1.removeFromLeft (knobW), speedSlider, speedLabel);
    layoutKnob (row1.removeFromLeft (knobW), mixSlider, mixLabelCtrl);
    layoutKnob (row1.removeFromLeft (knobW), correctionSlider, correctionLabel);
    auto filterArea = row1.removeFromLeft (knobW);
    filterLabel.setBounds (filterArea.removeFromTop (16));
    filterBox.setBounds (filterArea.withSizeKeepingCentre (filterArea.getWidth() - 8, 28));

    auto row2 = voiceGrid;
    layoutKnob (row2.removeFromLeft (knobW), inputSlider, inputLabel);
    layoutKnob (row2.removeFromLeft (knobW), outputSlider, outputLabel);
    layoutKnob (row2.removeFromLeft (knobW), aiMorphSlider, aiMorphLabel);

    bounds.removeFromTop (8);
    auto fxPanel = bounds.removeFromTop (122);
    auto fxGrid = fxPanel.reduced (12, 28);
    const int fxKnobW = fxGrid.getWidth() / 4;

    layoutKnob (fxGrid.removeFromLeft (fxKnobW), distortionSlider, distortionLabel);
    layoutKnob (fxGrid.removeFromLeft (fxKnobW), reverbSlider, reverbLabel);
    layoutKnob (fxGrid.removeFromLeft (fxKnobW), robotSlider, robotLabel);
    layoutKnob (fxGrid.removeFromLeft (fxKnobW), chorusSlider, chorusLabel);
}

} // namespace vocalchangeplus
