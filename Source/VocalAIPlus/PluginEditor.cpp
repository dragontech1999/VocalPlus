#include "PluginEditor.h"
#include "ParameterIDs.h"

namespace vocalaiplus
{

class VocalAIPlusAudioProcessorEditor::PresetButton : public juce::TextButton
{
public:
    PresetButton (VocalAIPlusAudioProcessorEditor& ed, int idx, juce::String name)
        : editor (ed), presetIndex (idx)
    {
        setButtonText (std::move (name));
        onClick = [this] { editor.loadPreset (presetIndex); };
    }

private:
    VocalAIPlusAudioProcessorEditor& editor;
    int presetIndex;
};

class VocalAIPlusAudioProcessorEditor::ScrollableContent : public juce::Component
{
public:
    explicit ScrollableContent (VocalAIPlusAudioProcessorEditor& ed) : editor (ed) {}

    void paint (juce::Graphics& g) override { editor.paintContent (g); }
    void resized() override { editor.layoutContent(); }

private:
    VocalAIPlusAudioProcessorEditor& editor;
};

namespace
{
void setupKnob (juce::Slider& s)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 56, 16);
}
} // namespace

VocalAIPlusAudioProcessorEditor::VocalAIPlusAudioProcessorEditor (VocalAIPlusAudioProcessor& p)
    : AudioProcessorEditor (&p),
      processor (p),
      editorShell (*this, VocalAIPlusLookAndFeel::kNeonGreen)
{
    setLookAndFeel (&lookAndFeel);

    titleLabel.setFont (juce::Font (22.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, VocalAIPlusLookAndFeel::kNeonGreen);
    subtitleLabel.setColour (juce::Label::textColourId, VocalAIPlusLookAndFeel::kTextSecondary);
    presetDescLabel.setColour (juce::Label::textColourId, VocalAIPlusLookAndFeel::kTextSecondary);
    rationaleLabel.setColour (juce::Label::textColourId, VocalAIPlusLookAndFeel::kTextSecondary);
    analysisLabel.setColour (juce::Label::textColourId, VocalAIPlusLookAndFeel::kTextPrimary);
    meterLabel.setColour (juce::Label::textColourId, VocalAIPlusLookAndFeel::kTextPrimary);

    for (auto* s : { &noiseAmt, &eqLow, &eqLowMid, &eqPres, &eqAir,
                     &compThr, &compRatioKnob, &compAtk, &compRel, &compMakeup,
                     &deEssFreq, &deEssCut, &deEssSens, &excAmt, &excMix,
                     &revWet, &revRoom, &limCeil, &limGain, &limTarget,
                     &outputGain, &mixSlider })
        setupKnob (*s);

    for (int g = 0; g < static_cast<int> (VocalGenre::numGenres); ++g)
        genreBox.addItem (LocalMasteringAI::genreName (static_cast<VocalGenre> (g)), g + 1);

    aiMasterButton.onClick = [this] { processor.runAIMaster(); };

    scrollContent = std::make_unique<ScrollableContent> (*this);
    addAndMakeVisible (editorShell);
    editorShell.setScrollContent (scrollContent.get(), kContentWidth, kContentHeight);

    auto addToContent = [this] (juce::Component& c) { scrollContent->addAndMakeVisible (c); };

    for (auto* c : { static_cast<juce::Component*> (&titleLabel),
                     static_cast<juce::Component*> (&subtitleLabel),
                     static_cast<juce::Component*> (&presetDescLabel),
                     static_cast<juce::Component*> (&rationaleLabel),
                     static_cast<juce::Component*> (&analysisLabel),
                     static_cast<juce::Component*> (&meterLabel),
                     static_cast<juce::Component*> (&aiMasterButton),
                     static_cast<juce::Component*> (&genreBox),
                     static_cast<juce::Component*> (&genreLbl) })
        addToContent (*c);

    for (auto* s : { &noiseAmt, &eqLow, &eqLowMid, &eqPres, &eqAir,
                     &compThr, &compRatioKnob, &compAtk, &compRel, &compMakeup,
                     &deEssFreq, &deEssCut, &deEssSens, &excAmt, &excMix,
                     &revWet, &revRoom, &limCeil, &limGain, &limTarget,
                     &outputGain, &mixSlider })
        addToContent (*s);

    for (auto* l : { &noiseLbl, &eqLowLbl, &eqLowMidLbl, &eqPresLbl, &eqAirLbl,
                     &compThrLbl, &compRatioLbl, &compAtkLbl, &compRelLbl, &compMakeupLbl,
                     &deEssFreqLbl, &deEssCutLbl, &deEssSensLbl, &excAmtLbl, &excMixLbl,
                     &revWetLbl, &revRoomLbl, &limCeilLbl, &limGainLbl, &limTargetLbl,
                     &outputLbl, &mixLbl })
        addToContent (*l);

    rebuildPresetButtons();

    auto& apvts = processor.getAPVTS();
    using namespace ParamIDs;

    noiseAmtAtt = std::make_unique<Attachment> (apvts, noiseAmount, noiseAmt);
    eqLowAtt = std::make_unique<Attachment> (apvts, eqLowShelfDb, eqLow);
    eqLowMidAtt = std::make_unique<Attachment> (apvts, eqLowMidDb, eqLowMid);
    eqPresAtt = std::make_unique<Attachment> (apvts, eqPresenceDb, eqPres);
    eqAirAtt = std::make_unique<Attachment> (apvts, eqHighShelfDb, eqAir);
    compThrAtt = std::make_unique<Attachment> (apvts, compThresholdDb, compThr);
    compRatioAtt = std::make_unique<Attachment> (apvts, ParamIDs::compRatio, compRatioKnob);
    compAtkAtt = std::make_unique<Attachment> (apvts, compAttackMs, compAtk);
    compRelAtt = std::make_unique<Attachment> (apvts, compReleaseMs, compRel);
    compMakeupAtt = std::make_unique<Attachment> (apvts, compMakeupDb, compMakeup);
    deEssFreqAtt = std::make_unique<Attachment> (apvts, deEssFreqHz, deEssFreq);
    deEssCutAtt = std::make_unique<Attachment> (apvts, deEssReductionDb, deEssCut);
    deEssSensAtt = std::make_unique<Attachment> (apvts, deEssSensitivity, deEssSens);
    excAmtAtt = std::make_unique<Attachment> (apvts, exciterAmount, excAmt);
    excMixAtt = std::make_unique<Attachment> (apvts, exciterMix, excMix);
    revWetAtt = std::make_unique<Attachment> (apvts, reverbWet, revWet);
    revRoomAtt = std::make_unique<Attachment> (apvts, reverbRoomSize, revRoom);
    limCeilAtt = std::make_unique<Attachment> (apvts, limiterCeilingDbTP, limCeil);
    limGainAtt = std::make_unique<Attachment> (apvts, limiterInputGainDb, limGain);
    limTargetAtt = std::make_unique<Attachment> (apvts, limiterTargetLUFS, limTarget);
    outputAtt = std::make_unique<Attachment> (apvts, outputGainDb, outputGain);
    mixAtt = std::make_unique<Attachment> (apvts, mix, mixSlider);
    genreAtt = std::make_unique<ComboAttachment> (apvts, genre, genreBox);

    loadPreset (processor.getCurrentPresetIndex());
    vocalplus::ui::VocalPlusEditorShell::configureEditorWindow (*this, kContentWidth, kContentHeight);
    startTimerHz (12);
}

VocalAIPlusAudioProcessorEditor::~VocalAIPlusAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void VocalAIPlusAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (VocalAIPlusLookAndFeel::kBackground);
}

void VocalAIPlusAudioProcessorEditor::resized()
{
    editorShell.setBounds (getLocalBounds());
}

void VocalAIPlusAudioProcessorEditor::drawNeonPanel (juce::Graphics& g, juce::Rectangle<int> r,
                                                     const juce::String& title) const
{
    g.setColour (VocalAIPlusLookAndFeel::kPanel);
    g.fillRoundedRectangle (r.toFloat(), 6.0f);
    g.setColour (VocalAIPlusLookAndFeel::kPanelBorder.withAlpha (0.55f));
    g.drawRoundedRectangle (r.toFloat().reduced (0.5f), 6.0f, 1.0f);
    g.setColour (VocalAIPlusLookAndFeel::kNeonGreen);
    g.setFont (juce::Font (12.0f, juce::Font::bold));
    g.drawText (title, r.removeFromTop (22).reduced (8, 0), juce::Justification::centredLeft);
}

void VocalAIPlusAudioProcessorEditor::paintContent (juce::Graphics& g)
{
    drawNeonPanel (g, { 16, 52, kContentWidth - 32, 100 }, "Analysis");
    drawNeonPanel (g, { 16, 162, kContentWidth - 32, 130 }, "Presets");
    drawNeonPanel (g, { 16, 302, 280, 500 }, "Noise / EQ");
    drawNeonPanel (g, { 306, 302, 280, 500 }, "Dynamics");
    drawNeonPanel (g, { 596, 302, 280, 500 }, "Color / Space");
    drawNeonPanel (g, { 886, 302, 278, 500 }, "Limiter / Output");
}

void VocalAIPlusAudioProcessorEditor::layoutModuleRow (juce::Rectangle<int>& area,
                                                        juce::Slider& a, juce::Slider& b,
                                                        juce::Label& la, juce::Label& lb)
{
    auto row = area.removeFromTop (88);
    auto left = row.removeFromLeft (row.getWidth() / 2).reduced (4);
    auto right = row.reduced (4);
    la.setBounds (left.removeFromTop (16));
    a.setBounds (left);
    lb.setBounds (right.removeFromTop (16));
    b.setBounds (right);
}

void VocalAIPlusAudioProcessorEditor::layoutContent()
{
    titleLabel.setBounds (20, 8, 300, 28);
    subtitleLabel.setBounds (20, 34, 500, 18);
    genreLbl.setBounds (kContentWidth - 280, 12, 50, 20);
    genreBox.setBounds (kContentWidth - 220, 8, 120, 24);
    aiMasterButton.setBounds (kContentWidth - 90, 8, 74, 24);

    analysisLabel.setBounds (28, 78, kContentWidth - 56, 36);
    rationaleLabel.setBounds (28, 114, kContentWidth - 56, 32);
    meterLabel.setBounds (28, 148, kContentWidth - 56, 18);

    auto presetArea = juce::Rectangle<int> (24, 188, kContentWidth - 48, 96);
    int x = presetArea.getX();
    const int btnW = 108;
    const int btnH = 26;
    int rowY = presetArea.getY();
    for (int i = 0; i < presetButtons.size(); ++i)
    {
        if (x + btnW > presetArea.getRight())
        {
            x = presetArea.getX();
            rowY += btnH + 4;
        }
        presetButtons[i]->setBounds (x, rowY, btnW, btnH);
        x += btnW + 4;
    }
    presetDescLabel.setBounds (24, 268, kContentWidth - 48, 18);

    auto eqArea = juce::Rectangle<int> (24, 328, 264, 460).reduced (0, 24);
    layoutModuleRow (eqArea, noiseAmt, eqLow, noiseLbl, eqLowLbl);
    layoutModuleRow (eqArea, eqLowMid, eqPres, eqLowMidLbl, eqPresLbl);
    eqAirLbl.setBounds (eqArea.removeFromTop (16));
    eqAir.setBounds (eqArea.removeFromTop (80).reduced (40, 0));

    auto dynArea = juce::Rectangle<int> (314, 328, 264, 460).reduced (0, 24);
    layoutModuleRow (dynArea, compThr, compRatioKnob, compThrLbl, compRatioLbl);
    layoutModuleRow (dynArea, compAtk, compRel, compAtkLbl, compRelLbl);
    compMakeupLbl.setBounds (dynArea.removeFromTop (16));
    compMakeup.setBounds (dynArea.removeFromTop (80).reduced (40, 0));

    auto colArea = juce::Rectangle<int> (604, 328, 264, 460).reduced (0, 24);
    layoutModuleRow (colArea, deEssFreq, deEssCut, deEssFreqLbl, deEssCutLbl);
    layoutModuleRow (colArea, deEssSens, excAmt, deEssSensLbl, excAmtLbl);
    layoutModuleRow (colArea, excMix, revWet, excMixLbl, revWetLbl);
    revRoomLbl.setBounds (colArea.removeFromTop (16));
    revRoom.setBounds (colArea.removeFromTop (80).reduced (40, 0));

    auto outArea = juce::Rectangle<int> (894, 328, 262, 460).reduced (0, 24);
    layoutModuleRow (outArea, limCeil, limGain, limCeilLbl, limGainLbl);
    layoutModuleRow (outArea, limTarget, outputGain, limTargetLbl, outputLbl);
    mixLbl.setBounds (outArea.removeFromTop (16));
    mixSlider.setBounds (outArea.removeFromTop (80).reduced (40, 0));
}

void VocalAIPlusAudioProcessorEditor::rebuildPresetButtons()
{
    presetButtons.clear();
    for (int i = 0; i < processor.getPresetManager().getNumPresets(); ++i)
    {
        auto* btn = presetButtons.add (new PresetButton (*this, i,
            processor.getPresetManager().getPreset (i).name));
        scrollContent->addAndMakeVisible (btn);
    }
}

void VocalAIPlusAudioProcessorEditor::loadPreset (int index)
{
    processor.applyPreset (index);
    const auto& preset = processor.getPresetManager().getPreset (index);
    presetDescLabel.setText (preset.description, juce::dontSendNotification);

    for (int i = 0; i < presetButtons.size(); ++i)
        presetButtons[i]->setToggleState (i == index, juce::dontSendNotification);
}

void VocalAIPlusAudioProcessorEditor::timerCallback()
{
    const auto snap = processor.getAnalysisSnapshot();
    analysisLabel.setText (
        juce::String::formatted ("LUFS %.1f  |  Peak %.1f dBTP  |  Crest %.1f dB  |  Centroid %.0f Hz",
                                 snap.integratedLUFS, snap.truePeakDBTP,
                                 snap.crestFactorDB, snap.spectralCentroidHz),
        juce::dontSendNotification);

    rationaleLabel.setText (processor.getLastRationale(), juce::dontSendNotification);

    meterLabel.setText (
        juce::String::formatted ("In %.1f dB  |  GR %.1f dB  |  Out %.1f dB",
                                 processor.getInputLevelDb(),
                                 processor.getGainReductionDb(),
                                 processor.getOutputLevelDb()),
        juce::dontSendNotification);
}

} // namespace vocalaiplus
