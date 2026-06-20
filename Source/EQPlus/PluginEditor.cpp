#include "PluginEditor.h"
#include "ParameterIDs.h"

namespace eqplus
{

class EQPlusAudioProcessorEditor::ScrollableContent : public juce::Component
{
public:
    explicit ScrollableContent (EQPlusAudioProcessorEditor& ownerIn) : owner (ownerIn) {}

    void paint (juce::Graphics& g) override { owner.paintContent (g); }
    void resized() override { owner.layoutContent(); }

private:
    EQPlusAudioProcessorEditor& owner;
};

EQPlusAudioProcessorEditor::EQPlusAudioProcessorEditor (EQPlusAudioProcessor& p)
    : AudioProcessorEditor (p),
      processor (p),
      editorShell (*this, EQPlusLookAndFeel::kNeonCyan)
{
    setLookAndFeel (&lookAndFeel);

    scrollContent = std::make_unique<ScrollableContent> (*this);
    addAndMakeVisible (editorShell);
    editorShell.setScrollContent (scrollContent.get(), kContentWidth, kContentHeight);

    auto addToContent = [this] (juce::Component& c) { scrollContent->addAndMakeVisible (c); };

    curveDisplay = std::make_unique<EQCurveDisplay> (processor.getEngine(), processor.getAPVTS());
    addToContent (*curveDisplay);

    curveDisplay->onBandSelected = [this] (int band)
    {
        currentBand = band;
        updateSelectedBandControls();
    };

    curveDisplay->onSketchComplete = [this] (const auto& points) { processor.sketchEQCurve (points); };

    curveDisplay->onSpectrumGrab = [this] (int band, float freq, float gain, bool dynamic)
    {
        processor.applySpectrumGrab (band, freq, gain, dynamic);
        currentBand = band;
        updateSelectedBandControls();
        statusLabel.setText (processor.getStatusMessage(), juce::dontSendNotification);
    };

    for (int i = 0; i < processor.getNumPrograms(); ++i)
        presetBox.addItem (processor.getProgramName (i), i + 1);
    presetBox.setSelectedId (processor.getCurrentProgram() + 1);
    presetBox.onChange = [this]
    {
        processor.setCurrentProgram (presetBox.getSelectedId() - 1);
        statusLabel.setText (processor.getStatusMessage(), juce::dontSendNotification);
    };
    addToContent (presetBox);

    for (const auto& name : getFilterTypeNames()) typeBox.addItem (name, typeBox.getNumItems() + 1);
    for (const auto& name : getChannelModeNames()) channelBox.addItem (name, channelBox.getNumItems() + 1);
    for (const auto& name : getProcessingModeNames()) processingBox.addItem (name, processingBox.getNumItems() + 1);
    for (const auto& name : getDisplayRangeNames()) displayRangeBox.addItem (name, displayRangeBox.getNumItems() + 1);

    for (auto* c : { &presetBox, &typeBox, &channelBox, &processingBox, &displayRangeBox })
        addToContent (*c);

    for (auto* l : { &presetLabel, &typeLabel, &channelLabel, &processingLabel, &displayRangeLabel,
                     &bandTitle, &peakLabel, &autoGainLabel, &statusLabel,
                     &freqLabel, &gainLabel, &qLabel,
                     &dynThresholdLabel, &dynRatioLabel, &dynAttackLabel, &dynReleaseLabel,
                     &outputLabel, &spectrumSpeedLabel })
    {
        l->setJustificationType (juce::Justification::centred);
        addToContent (*l);
    }

    bandTitle.setJustificationType (juce::Justification::centredLeft);
    bandTitle.setFont (juce::FontOptions (13.0f).withStyle ("Bold"));
    statusLabel.setJustificationType (juce::Justification::centredLeft);
    statusLabel.setColour (juce::Label::textColourId, EQPlusLookAndFeel::kNeonCyan.withAlpha (0.85f));

    for (auto* s : { &freqSlider, &gainSlider, &qSlider,
                     &dynThresholdSlider, &dynRatioSlider, &dynAttackSlider, &dynReleaseSlider,
                     &outputSlider, &spectrumSpeedSlider })
    {
        s->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 56, 14);
        addToContent (*s);
    }

    freqSlider.setSkewFactorFromMidPoint (1000.0);
    qSlider.setSkewFactorFromMidPoint (1.0);
    dynAttackSlider.setSkewFactorFromMidPoint (50.0);
    dynReleaseSlider.setSkewFactorFromMidPoint (200.0);

    for (auto* t : { &bandEnableToggle, &bandSoloToggle, &dynToggle,
                     &autoGainToggle, &phaseInvertToggle, &preSpectrumToggle, &postSpectrumToggle })
        addToContent (*t);

    for (auto* b : { &aiSuggestButton, &eqMatchButton, &captureRefButton, &deEssButton,
                     &abButton, &copyButton, &pasteButton, &freezeButton })
        addToContent (*b);

    aiSuggestButton.onClick = [this]
    {
        processor.applyAISuggestions();
        statusLabel.setText (processor.getStatusMessage(), juce::dontSendNotification);
    };
    eqMatchButton.onClick = [this]
    {
        processor.applyEQMatch();
        statusLabel.setText (processor.getStatusMessage(), juce::dontSendNotification);
    };
    captureRefButton.onClick = [this]
    {
        processor.captureReferenceSpectrum();
        statusLabel.setText ("Reference spectrum captured", juce::dontSendNotification);
    };
    deEssButton.onClick = [this]
    {
        processor.applyDeEss();
        currentBand = 0;
        updateSelectedBandControls();
        statusLabel.setText (processor.getStatusMessage(), juce::dontSendNotification);
    };
    abButton.onClick = [this] { processor.swapABState(); };
    copyButton.onClick = [this] { processor.copyBandsToClipboard(); };
    pasteButton.onClick = [this] { processor.pasteBandsFromClipboard(); };
    freezeButton.onClick = [this]
    {
        auto& analyzer = processor.getEngine().getSpectrumAnalyzer();
        analyzer.freeze (! analyzer.isFrozen());
        freezeButton.setToggleState (analyzer.isFrozen(), juce::dontSendNotification);
    };
    freezeButton.setClickingTogglesState (true);

    auto& apvts = processor.getAPVTS();
    bandEnableAttachment = std::make_unique<ButtonAttachment> (apvts, ParamIDs::bandEnabled (0), bandEnableToggle);
    bandSoloAttachment = std::make_unique<ButtonAttachment> (apvts, ParamIDs::bandSolo (0), bandSoloToggle);
    dynAttachment = std::make_unique<ButtonAttachment> (apvts, ParamIDs::bandDynEnabled (0), dynToggle);
    freqAttachment = std::make_unique<Attachment> (apvts, ParamIDs::bandFreq (0), freqSlider);
    gainAttachment = std::make_unique<Attachment> (apvts, ParamIDs::bandGain (0), gainSlider);
    qAttachment = std::make_unique<Attachment> (apvts, ParamIDs::bandQ (0), qSlider);
    dynThresholdAttachment = std::make_unique<Attachment> (apvts, ParamIDs::bandDynThreshold (0), dynThresholdSlider);
    dynRatioAttachment = std::make_unique<Attachment> (apvts, ParamIDs::bandDynRatio (0), dynRatioSlider);
    dynAttackAttachment = std::make_unique<Attachment> (apvts, ParamIDs::bandDynAttack (0), dynAttackSlider);
    dynReleaseAttachment = std::make_unique<Attachment> (apvts, ParamIDs::bandDynRelease (0), dynReleaseSlider);
    typeAttachment = std::make_unique<ComboAttachment> (apvts, ParamIDs::bandType (0), typeBox);
    channelAttachment = std::make_unique<ComboAttachment> (apvts, ParamIDs::bandChannel (0), channelBox);
    autoGainAttachment = std::make_unique<ButtonAttachment> (apvts, ParamIDs::autoGain, autoGainToggle);
    phaseInvertAttachment = std::make_unique<ButtonAttachment> (apvts, ParamIDs::phaseInvert, phaseInvertToggle);
    preSpectrumAttachment = std::make_unique<ButtonAttachment> (apvts, ParamIDs::showPreSpectrum, preSpectrumToggle);
    postSpectrumAttachment = std::make_unique<ButtonAttachment> (apvts, ParamIDs::showPostSpectrum, postSpectrumToggle);
    outputAttachment = std::make_unique<Attachment> (apvts, ParamIDs::outputGain, outputSlider);
    spectrumSpeedAttachment = std::make_unique<Attachment> (apvts, ParamIDs::spectrumSpeed, spectrumSpeedSlider);
    processingAttachment = std::make_unique<ComboAttachment> (apvts, ParamIDs::processingMode, processingBox);
    displayRangeAttachment = std::make_unique<ComboAttachment> (apvts, ParamIDs::displayRange, displayRangeBox);

    displayRangeBox.onChange = [this]
    {
        curveDisplay->setDisplayRange (static_cast<DisplayRange> (displayRangeBox.getSelectedItemIndex()));
    };

    updateSelectedBandControls();
    layoutContent();
    vocalplus::ui::VocalPlusEditorShell::configureEditorWindow (*this, kContentWidth, kContentHeight);
    startTimerHz (8);
}

EQPlusAudioProcessorEditor::~EQPlusAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void EQPlusAudioProcessorEditor::updateSelectedBandControls()
{
    auto& apvts = processor.getAPVTS();
    bandTitle.setText ("Band " + juce::String (currentBand + 1), juce::dontSendNotification);
    bandEnableAttachment = std::make_unique<ButtonAttachment> (apvts, ParamIDs::bandEnabled (currentBand), bandEnableToggle);
    bandSoloAttachment = std::make_unique<ButtonAttachment> (apvts, ParamIDs::bandSolo (currentBand), bandSoloToggle);
    dynAttachment = std::make_unique<ButtonAttachment> (apvts, ParamIDs::bandDynEnabled (currentBand), dynToggle);
    freqAttachment = std::make_unique<Attachment> (apvts, ParamIDs::bandFreq (currentBand), freqSlider);
    gainAttachment = std::make_unique<Attachment> (apvts, ParamIDs::bandGain (currentBand), gainSlider);
    qAttachment = std::make_unique<Attachment> (apvts, ParamIDs::bandQ (currentBand), qSlider);
    dynThresholdAttachment = std::make_unique<Attachment> (apvts, ParamIDs::bandDynThreshold (currentBand), dynThresholdSlider);
    dynRatioAttachment = std::make_unique<Attachment> (apvts, ParamIDs::bandDynRatio (currentBand), dynRatioSlider);
    dynAttackAttachment = std::make_unique<Attachment> (apvts, ParamIDs::bandDynAttack (currentBand), dynAttackSlider);
    dynReleaseAttachment = std::make_unique<Attachment> (apvts, ParamIDs::bandDynRelease (currentBand), dynReleaseSlider);
    typeAttachment = std::make_unique<ComboAttachment> (apvts, ParamIDs::bandType (currentBand), typeBox);
    channelAttachment = std::make_unique<ComboAttachment> (apvts, ParamIDs::bandChannel (currentBand), channelBox);
}

void EQPlusAudioProcessorEditor::timerCallback()
{
    const float peak = processor.getEngine().getOutputPeakDb();
    const float autoGain = processor.getEngine().getAutoGainCompensationDb();
    if (std::abs (peak - lastPeakDisplay) > 0.4f)
    {
        lastPeakDisplay = peak;
        peakLabel.setText ("Peak " + juce::String (peak, 1) + " dB", juce::dontSendNotification);
    }
    if (std::abs (autoGain - lastAutoGainDisplay) > 0.25f)
    {
        lastAutoGainDisplay = autoGain;
        autoGainLabel.setText ("Auto " + juce::String (autoGain, 1) + " dB", juce::dontSendNotification);
    }
}

void EQPlusAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (EQPlusLookAndFeel::kBackground);
}

void EQPlusAudioProcessorEditor::paintContent (juce::Graphics& g)
{
    g.fillAll (EQPlusLookAndFeel::kBackground);

    g.setColour (EQPlusLookAndFeel::kTextPrimary);
    g.setFont (juce::FontOptions (18.0f).withStyle ("Bold"));
    g.drawText ("EQ+", 16, 12, 48, 24, juce::Justification::centredLeft, false);

    g.setColour (EQPlusLookAndFeel::kTextSecondary);
    g.setFont (juce::FontOptions (11.0f));
    g.drawText ("Dynamic Equalizer", 64, 16, 160, 18, juce::Justification::centredLeft, false);
}

void EQPlusAudioProcessorEditor::layoutKnobRow (juce::Rectangle<int>& row, int numKnobs,
                                                juce::Slider* sliders[], juce::Label* labels[])
{
    const int w = row.getWidth() / numKnobs;
    for (int i = 0; i < numKnobs; ++i)
    {
        auto cell = row.removeFromLeft (w).reduced (2);
        labels[i]->setBounds (cell.removeFromTop (14));
        sliders[i]->setBounds (cell);
    }
}

void EQPlusAudioProcessorEditor::resized()
{
    editorShell.setBounds (getLocalBounds());
    layoutContent();
}

void EQPlusAudioProcessorEditor::layoutContent()
{
    auto bounds = juce::Rectangle<int> (0, 0, kContentWidth, kContentHeight).reduced (12);
    auto header = bounds.removeFromTop (36);

    presetLabel.setBounds (header.removeFromLeft (44).reduced (0, 8));
    presetBox.setBounds (header.removeFromLeft (140).reduced (0, 6));
    header.removeFromLeft (8);
    aiSuggestButton.setBounds (header.removeFromLeft (72).reduced (0, 6));
    eqMatchButton.setBounds (header.removeFromLeft (68).reduced (0, 6));
    deEssButton.setBounds (header.removeFromLeft (56).reduced (0, 6));
    freezeButton.setBounds (header.removeFromRight (56).reduced (0, 6));
    abButton.setBounds (header.removeFromRight (36).reduced (0, 6));

    auto footer = bounds.removeFromBottom (96);
    auto sidebar = bounds.removeFromRight (240).reduced (8, 0);

    if (curveDisplay != nullptr)
        curveDisplay->setBounds (bounds.reduced (0, 4));

    bandTitle.setBounds (sidebar.removeFromTop (20));
    auto bandToggles = sidebar.removeFromTop (24);
    bandEnableToggle.setBounds (bandToggles.removeFromLeft (48));
    bandSoloToggle.setBounds (bandToggles.removeFromLeft (48));
    dynToggle.setBounds (bandToggles.removeFromLeft (68));

    auto typeRow = sidebar.removeFromTop (22);
    typeLabel.setBounds (typeRow.removeFromLeft (32));
    typeBox.setBounds (typeRow);

    auto chRow = sidebar.removeFromTop (22);
    channelLabel.setBounds (chRow.removeFromLeft (32));
    channelBox.setBounds (chRow);
    sidebar.removeFromTop (6);

    juce::Slider* eqKnobs[] = { &freqSlider, &gainSlider, &qSlider };
    juce::Label* eqLabels[] = { &freqLabel, &gainLabel, &qLabel };
    auto eqRow = sidebar.removeFromTop (82);
    layoutKnobRow (eqRow, 3, eqKnobs, eqLabels);

    juce::Slider* dynKnobs[] = { &dynThresholdSlider, &dynRatioSlider, &dynAttackSlider, &dynReleaseSlider };
    juce::Label* dynLabels[] = { &dynThresholdLabel, &dynRatioLabel, &dynAttackLabel, &dynReleaseLabel };
    auto dynRow = sidebar.removeFromTop (82);
    layoutKnobRow (dynRow, 4, dynKnobs, dynLabels);

    auto globalRow1 = footer.removeFromTop (22);
    processingBox.setBounds (globalRow1.removeFromLeft (100));
    globalRow1.removeFromLeft (8);
    displayRangeBox.setBounds (globalRow1.removeFromLeft (72));
    globalRow1.removeFromLeft (8);
    autoGainToggle.setBounds (globalRow1.removeFromLeft (72));
    preSpectrumToggle.setBounds (globalRow1.removeFromLeft (40));
    postSpectrumToggle.setBounds (globalRow1.removeFromLeft (44));
    peakLabel.setBounds (globalRow1.removeFromRight (100));
    autoGainLabel.setBounds (globalRow1.removeFromRight (100));

    juce::Slider* globKnobs[] = { &outputSlider, &spectrumSpeedSlider };
    juce::Label* globLabels[] = { &outputLabel, &spectrumSpeedLabel };
    auto globRow = footer.removeFromTop (72);
    layoutKnobRow (globRow, 2, globKnobs, globLabels);
}

} // namespace eqplus
