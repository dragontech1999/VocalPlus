#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ParameterIDs.h"
#include "../DSP/AudioSafety.h"
#include "../DSP/ParameterHelpers.h"

namespace eqplus
{

using namespace ParamIDs;
using namespace vocalplus;

EQPlusAudioProcessor::EQPlusAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "EQPlus", createParameterLayout())
{
    setCurrentProgram (1);
}

EQPlusAudioProcessor::~EQPlusAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout EQPlusAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        outputGain, "Output Gain", juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool> (autoGain, "Auto Gain", false));
    params.push_back (std::make_unique<juce::AudioParameterBool> (phaseInvert, "Phase Invert", false));
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        processingMode, "Processing", getProcessingModeNames(), 0));
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        displayRange, "Display Range", getDisplayRangeNames(), 2));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        spectrumSpeed, "Spectrum Speed", juce::NormalisableRange<float> (0.1f, 0.95f, 0.01f), 0.7f));
    params.push_back (std::make_unique<juce::AudioParameterBool> (showPreSpectrum, "Pre Spectrum", true));
    params.push_back (std::make_unique<juce::AudioParameterBool> (showPostSpectrum, "Post Spectrum", true));
    params.push_back (std::make_unique<juce::AudioParameterInt> (
        selectedBand, "Selected Band", 0, kMaxBands - 1, 0));
    params.push_back (std::make_unique<juce::AudioParameterBool> (abState, "A/B", true));

    for (int i = 0; i < kMaxBands; ++i)
    {
        const auto suffix = juce::String (i);
        params.push_back (std::make_unique<juce::AudioParameterBool> (
            bandEnabled (i), "Band " + suffix + " On", false));
        params.push_back (std::make_unique<juce::AudioParameterBool> (
            bandSolo (i), "Band " + suffix + " Solo", false));
        params.push_back (std::make_unique<juce::AudioParameterChoice> (
            bandType (i), "Band " + suffix + " Type", getFilterTypeNames(), 0));
        params.push_back (std::make_unique<juce::AudioParameterChoice> (
            bandChannel (i), "Band " + suffix + " Channel", getChannelModeNames(), 0));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            bandFreq (i), "Band " + suffix + " Freq",
            juce::NormalisableRange<float> (kMinFreq, kMaxFreq, 0.1f, 0.25f), 1000.0f * std::pow (2.0f, static_cast<float> (i) * 0.25f)));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            bandGain (i), "Band " + suffix + " Gain",
            juce::NormalisableRange<float> (kMinGain, kMaxGain, 0.1f), 0.0f));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            bandQ (i), "Band " + suffix + " Q",
            juce::NormalisableRange<float> (kMinQ, kMaxQ, 0.01f, 0.3f), 1.0f));
        params.push_back (std::make_unique<juce::AudioParameterBool> (
            bandDynEnabled (i), "Band " + suffix + " Dyn", false));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            bandDynThreshold (i), "Band " + suffix + " Threshold",
            juce::NormalisableRange<float> (-60.0f, 0.0f, 0.1f), -20.0f));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            bandDynRatio (i), "Band " + suffix + " Ratio",
            juce::NormalisableRange<float> (1.0f, 20.0f, 0.1f), 2.0f));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            bandDynAttack (i), "Band " + suffix + " Attack",
            juce::NormalisableRange<float> (0.1f, 500.0f, 0.1f, 0.4f), 10.0f));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            bandDynRelease (i), "Band " + suffix + " Release",
            juce::NormalisableRange<float> (1.0f, 2000.0f, 1.0f, 0.4f), 100.0f));
    }

    return { params.begin(), params.end() };
}

void EQPlusAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.prepare (sampleRate, samplesPerBlock, getTotalNumOutputChannels());
    syncEngineFromParameters();
}

void EQPlusAudioProcessor::releaseResources() {}

bool EQPlusAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
    return true;
}

std::array<BandSettings, kMaxBands> EQPlusAudioProcessor::readBandsFromParameters() const
{
    std::array<BandSettings, kMaxBands> result;
    for (int i = 0; i < kMaxBands; ++i)
    {
        auto& b = result[static_cast<size_t> (i)];
        b.enabled = readBoolParam (apvts, bandEnabled (i));
        b.solo = readBoolParam (apvts, bandSolo (i));
        b.type = static_cast<FilterType> (readChoiceParam (apvts, bandType (i)));
        b.channel = static_cast<ChannelMode> (readChoiceParam (apvts, bandChannel (i)));
        b.frequency = readFloatParam (apvts, bandFreq (i));
        b.gainDb = readFloatParam (apvts, bandGain (i));
        b.q = readFloatParam (apvts, bandQ (i));
        b.dynamicEnabled = readBoolParam (apvts, bandDynEnabled (i));
        b.dynThresholdDb = readFloatParam (apvts, bandDynThreshold (i));
        b.dynRatio = readFloatParam (apvts, bandDynRatio (i));
        b.dynAttackMs = readFloatParam (apvts, bandDynAttack (i));
        b.dynReleaseMs = readFloatParam (apvts, bandDynRelease (i));
        b.slope = bandSlopes[static_cast<size_t> (i)];
    }
    return result;
}

EQGlobalSettings EQPlusAudioProcessor::readGlobalSettings() const
{
    EQGlobalSettings g;
    g.outputGainDb = readFloatParam (apvts, outputGain);
    g.autoGain = readBoolParam (apvts, autoGain);
    g.phaseInvert = readBoolParam (apvts, phaseInvert);
    g.processingMode = static_cast<ProcessingMode> (readChoiceParam (apvts, processingMode));
    g.displayRange = static_cast<DisplayRange> (readChoiceParam (apvts, displayRange));
    g.spectrumSpeed = readFloatParam (apvts, spectrumSpeed);
    g.showPreSpectrum = readBoolParam (apvts, showPreSpectrum);
    g.showPostSpectrum = readBoolParam (apvts, showPostSpectrum);
    return g;
}

void EQPlusAudioProcessor::syncEngineFromParameters()
{
    engine.setBands (readBandsFromParameters());
    engine.setGlobalSettings (readGlobalSettings());
}

void EQPlusAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ignoreUnused (midi);
    juce::ScopedNoDenormals noDenormals;

    syncEngineFromParameters();
    engine.processBlock (buffer);
    vocalplus::sanitizeBuffer (buffer);
}

juce::AudioProcessorEditor* EQPlusAudioProcessor::createEditor()
{
    return new EQPlusAudioProcessorEditor (*this);
}

void EQPlusAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("currentProgram", currentProgram, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void EQPlusAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        auto vt = juce::ValueTree::fromXml (*xml);
        apvts.replaceState (vt);
        currentProgram = vt.getProperty ("currentProgram", 0);
        syncEngineFromParameters();
    }
}

void EQPlusAudioProcessor::setCurrentProgram (int index)
{
    currentProgram = juce::jlimit (0, getNumPrograms() - 1, index);
    loadFactoryPreset (currentProgram);
}

const juce::String EQPlusAudioProcessor::getProgramName (int index)
{
    static const juce::StringArray names = {
        "Flat", "Vocal Presence", "Mix Bus Glue", "Mastering Tilt",
        "Bass Boost", "De-Harsh", "Air & Shine", "Podcast Voice"
    };
    return names[juce::jlimit (0, names.size() - 1, index)];
}

void EQPlusAudioProcessor::setChoiceParam (const juce::String& id, int index)
{
    if (auto* p = apvts.getParameter (id))
        p->setValueNotifyingHost (p->convertTo0to1 (static_cast<float> (index)));
}

void EQPlusAudioProcessor::setBand (int i, const BandSettings& band)
{
    if (auto* p = apvts.getParameter (bandEnabled (i))) p->setValueNotifyingHost (band.enabled ? 1.0f : 0.0f);
    setChoiceParam (bandType (i), static_cast<int> (band.type));
    setChoiceParam (bandChannel (i), static_cast<int> (band.channel));
    if (auto* p = apvts.getParameter (bandFreq (i)))
        p->setValueNotifyingHost (p->getNormalisableRange().convertTo0to1 (band.frequency));
    if (auto* p = apvts.getParameter (bandGain (i)))
        p->setValueNotifyingHost (p->getNormalisableRange().convertTo0to1 (band.gainDb));
    if (auto* p = apvts.getParameter (bandQ (i)))
        p->setValueNotifyingHost (p->getNormalisableRange().convertTo0to1 (band.q));
    if (auto* p = apvts.getParameter (bandDynEnabled (i))) p->setValueNotifyingHost (band.dynamicEnabled ? 1.0f : 0.0f);
}

void EQPlusAudioProcessor::clearAllBands()
{
    for (int i = 0; i < kMaxBands; ++i)
    {
        if (auto* p = apvts.getParameter (bandEnabled (i))) p->setValueNotifyingHost (0.0f);
        if (auto* p = apvts.getParameter (bandDynEnabled (i))) p->setValueNotifyingHost (0.0f);
    }
}

void EQPlusAudioProcessor::loadFactoryPreset (int index)
{
    clearAllBands();

    auto enable = [&] (int i, FilterType type, float freq, float gain, float q,
                       bool dyn = false, float slope = 12.0f)
    {
        BandSettings b;
        b.enabled = true;
        b.type = type;
        b.frequency = freq;
        b.gainDb = gain;
        b.q = q;
        b.slope = slope;
        b.slope = slope;
        b.dynamicEnabled = dyn;
        bandSlopes[static_cast<size_t> (i)] = slope;
        setBand (i, b);
    };

    switch (index)
    {
        case 1: // Vocal Presence (Ozone-style forward vocal EQ)
            enable (0, FilterType::LowCut, 70.0f, 0.0f, 0.707f, false, 24.0f);
            enable (1, FilterType::Bell, 180.0f, -3.5f, 1.3f);
            enable (2, FilterType::Bell, 1100.0f, 2.5f, 0.85f);
            enable (3, FilterType::Bell, 3600.0f, 5.0f, 1.0f);
            enable (4, FilterType::HighShelf, 9500.0f, 3.5f, 0.65f);
            enable (5, FilterType::Bell, 6200.0f, -2.5f, 2.2f, true);
            break;
        case 2: // Mix Bus Glue
            enable (0, FilterType::LowCut, 28.0f, 0.0f, 0.707f, false, 24.0f);
            enable (1, FilterType::Bell, 280.0f, -1.8f, 1.2f);
            enable (2, FilterType::Bell, 900.0f, -0.8f, 1.5f);
            enable (3, FilterType::Bell, 3200.0f, 1.2f, 0.7f);
            enable (4, FilterType::HighShelf, 12000.0f, 1.5f, 0.7f);
            enable (5, FilterType::HighCut, 18000.0f, 0.0f, 0.707f, false, 12.0f);
            break;
        case 3: // Mastering Tilt
            enable (0, FilterType::LowCut, 22.0f, 0.0f, 0.707f, false, 24.0f);
            enable (1, FilterType::TiltShelf, 800.0f, -1.2f, 0.707f);
            enable (2, FilterType::Bell, 350.0f, -0.8f, 1.8f);
            enable (3, FilterType::Bell, 5500.0f, 0.8f, 1.0f);
            enable (4, FilterType::HighCut, 20000.0f, 0.0f, 0.707f, false, 12.0f);
            break;
        case 4: // Bass Boost
            enable (0, FilterType::LowShelf, 70.0f, 5.0f, 0.65f);
            enable (1, FilterType::Bell, 110.0f, 2.5f, 1.2f);
            enable (2, FilterType::Bell, 200.0f, -1.0f, 1.5f);
            enable (3, FilterType::LowCut, 25.0f, 0.0f, 0.707f, false, 18.0f);
            break;
        case 5: // De-Harsh
            enable (0, FilterType::Bell, 3100.0f, -3.5f, 2.8f, true);
            enable (1, FilterType::Bell, 5200.0f, -2.5f, 3.5f, true);
            enable (2, FilterType::Bell, 7800.0f, -1.8f, 2.0f);
            enable (3, FilterType::Bell, 850.0f, -1.2f, 1.4f);
            enable (4, FilterType::HighShelf, 14000.0f, -1.0f, 0.7f);
            break;
        case 6: // Air & Shine
            enable (0, FilterType::HighShelf, 7500.0f, 3.5f, 0.65f);
            enable (1, FilterType::Bell, 11500.0f, 2.5f, 1.8f);
            enable (2, FilterType::Bell, 4000.0f, 1.0f, 0.8f);
            enable (3, FilterType::LowCut, 35.0f, 0.0f, 0.707f, false, 18.0f);
            break;
        case 7: // Podcast Voice
            enable (0, FilterType::LowCut, 85.0f, 0.0f, 0.707f, false, 24.0f);
            enable (1, FilterType::Bell, 160.0f, -3.5f, 1.6f);
            enable (2, FilterType::Bell, 4200.0f, 3.0f, 1.0f);
            enable (3, FilterType::Bell, 7000.0f, -2.5f, 2.2f);
            enable (4, FilterType::Bell, 300.0f, -1.5f, 1.2f);
            enable (5, FilterType::HighShelf, 9000.0f, 1.5f, 0.7f);
            break;
        default:
            statusMessage = "Flat — all bands bypassed";
            break;
    }

    if (index > 0)
        statusMessage = "Loaded preset: " + getProgramName (index);

    syncEngineFromParameters();
}

void EQPlusAudioProcessor::applyAISuggestions()
{
    const auto suggestions = aiAssistant.analyze (engine.getSpectrumAnalyzer(), getSampleRate());
    clearAllBands();

    juce::StringArray reasons;
    int bandIndex = 0;

    for (const auto& s : suggestions)
    {
        if (bandIndex >= kMaxBands)
            break;

        BandSettings b;
        b.enabled = true;
        b.type = s.type;
        b.frequency = s.frequency;
        b.gainDb = s.gainDb;
        b.q = s.q;
        b.slope = (s.type == FilterType::LowCut || s.type == FilterType::HighCut) ? 24.0f : 12.0f;
        bandSlopes[static_cast<size_t> (bandIndex)] = b.slope;
        setBand (bandIndex, b);
        reasons.add (s.reason);
        ++bandIndex;
    }

    syncEngineFromParameters();

    if (bandIndex > 0)
        statusMessage = "AI applied " + juce::String (bandIndex) + " bands: " + reasons.joinIntoString (", ");
    else
        statusMessage = "AI: no adjustments needed";
}

void EQPlusAudioProcessor::applyEQMatch()
{
    if (! engine.getSpectrumAnalyzer().hasReference())
    {
        statusMessage = "EQ Match: capture a reference spectrum first";
        return;
    }

    const auto matchBands = eqMatcher.computeMatchBands (engine.getSpectrumAnalyzer());
    clearAllBands();

    int bandIndex = 0;
    for (const auto& b : matchBands)
    {
        if (bandIndex >= kMaxBands)
            break;
        setBand (bandIndex, b);
        ++bandIndex;
    }

    syncEngineFromParameters();
    statusMessage = "EQ Match applied " + juce::String (bandIndex) + " compensating bands";
}

void EQPlusAudioProcessor::captureReferenceSpectrum()
{
    engine.getSpectrumAnalyzer().captureReference();
}

void EQPlusAudioProcessor::sketchEQCurve (const std::vector<std::pair<float, float>>& points)
{
    engine.sketchCurve (points);
    const auto& bands = engine.getBands();
    for (int i = 0; i < kMaxBands; ++i)
    {
        const auto& b = bands[static_cast<size_t> (i)];
        if (auto* p = apvts.getParameter (bandEnabled (i))) p->setValueNotifyingHost (b.enabled ? 1.0f : 0.0f);
        if (auto* p = apvts.getParameter (bandFreq (i))) p->setValueNotifyingHost (p->getNormalisableRange().convertTo0to1 (b.frequency));
        if (auto* p = apvts.getParameter (bandGain (i))) p->setValueNotifyingHost (p->getNormalisableRange().convertTo0to1 (b.gainDb));
        if (auto* p = apvts.getParameter (bandQ (i))) p->setValueNotifyingHost (p->getNormalisableRange().convertTo0to1 (b.q));
    }
}

void EQPlusAudioProcessor::copyBandsToClipboard()
{
    juce::String text;
    const auto bands = readBandsFromParameters();
    for (int i = 0; i < kMaxBands; ++i)
    {
        const auto& b = bands[static_cast<size_t> (i)];
        if (! b.enabled) continue;
        text += juce::String (i) + "," + juce::String (static_cast<int> (b.type)) + ","
             + juce::String (b.frequency) + "," + juce::String (b.gainDb) + "," + juce::String (b.q) + "\n";
    }
    juce::SystemClipboard::copyTextToClipboard (text);
}

void EQPlusAudioProcessor::pasteBandsFromClipboard()
{
    const auto text = juce::SystemClipboard::getTextFromClipboard();
    if (text.isEmpty()) return;

    for (const auto& line : juce::StringArray::fromLines (text))
    {
        auto tokens = juce::StringArray::fromTokens (line, ",", "");
        if (tokens.size() < 5) continue;
        const int i = tokens[0].getIntValue();
        if (i < 0 || i >= kMaxBands) continue;
        if (auto* p = apvts.getParameter (bandEnabled (i))) p->setValueNotifyingHost (1.0f);
        if (auto* p = apvts.getParameter (bandType (i))) p->setValueNotifyingHost (tokens[1].getFloatValue() / static_cast<float> (getFilterTypeNames().size() - 1));
        if (auto* p = apvts.getParameter (bandFreq (i))) p->setValueNotifyingHost (p->getNormalisableRange().convertTo0to1 (tokens[2].getFloatValue()));
        if (auto* p = apvts.getParameter (bandGain (i))) p->setValueNotifyingHost (p->getNormalisableRange().convertTo0to1 (tokens[3].getFloatValue()));
        if (auto* p = apvts.getParameter (bandQ (i))) p->setValueNotifyingHost (p->getNormalisableRange().convertTo0to1 (tokens[4].getFloatValue()));
    }
    syncEngineFromParameters();
}

void EQPlusAudioProcessor::applySpectrumGrab (int bandIndex, float freq, float gainDb, bool dynamic)
{
    if (bandIndex < 0 || bandIndex >= kMaxBands)
        return;

    BandSettings b;
    b.enabled = true;
    b.type = FilterType::Bell;
    b.frequency = freq;
    b.gainDb = gainDb;
    b.q = freq > 4000.0f ? 3.5f : 2.0f;
    b.dynamicEnabled = dynamic;
    b.dynThresholdDb = -24.0f;
    b.dynRatio = 3.0f;
    b.dynAttackMs = 5.0f;
    b.dynReleaseMs = 80.0f;
    setBand (bandIndex, b);
    syncEngineFromParameters();
    statusMessage = "Spectrum grab: " + juce::String (freq, 0) + " Hz"
                  + (dynamic ? " (dynamic)" : "");
}

void EQPlusAudioProcessor::applyDeEss()
{
    const auto& spectrum = engine.getSpectrumAnalyzer().getAccumulatedSpectrum();
    const double sr = getSampleRate() > 0.0 ? getSampleRate() : 44100.0;

    int bestBin = -1;
    float bestDb = -100.0f;

    for (int i = 2; i < SpectrumAnalyzer::fftSize / 2; ++i)
    {
        const float freq = static_cast<float> (i) * static_cast<float> (sr) / static_cast<float> (SpectrumAnalyzer::fftSize);
        if (freq < 4000.0f || freq > 9000.0f)
            continue;

        const float db = spectrum[static_cast<size_t> (i)];
        if (db > bestDb)
        {
            bestDb = db;
            bestBin = i;
        }
    }

    const int band = 0;
    BandSettings b;
    b.enabled = true;
    b.type = FilterType::Bell;
    b.frequency = bestBin >= 0
                ? static_cast<float> (bestBin) * static_cast<float> (sr) / static_cast<float> (SpectrumAnalyzer::fftSize)
                : 6500.0f;
    b.gainDb = -4.0f;
    b.q = 4.0f;
    b.dynamicEnabled = true;
    b.dynThresholdDb = -22.0f;
    b.dynRatio = 4.0f;
    b.dynAttackMs = 3.0f;
    b.dynReleaseMs = 60.0f;
    setBand (band, b);
    syncEngineFromParameters();
    statusMessage = "De-ess band at " + juce::String (b.frequency, 0) + " Hz (dynamic)";
}

void EQPlusAudioProcessor::swapABState()
{
    if (showingA)
    {
        stateB = apvts.copyState();
        if (stateA.isValid())
            apvts.replaceState (stateA);
        showingA = false;
    }
    else
    {
        stateA = apvts.copyState();
        if (stateB.isValid())
            apvts.replaceState (stateB);
        showingA = true;
    }
    if (auto* p = apvts.getParameter (abState))
        p->setValueNotifyingHost (showingA ? 1.0f : 0.0f);
    syncEngineFromParameters();
}

} // namespace eqplus

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new eqplus::EQPlusAudioProcessor();
}
