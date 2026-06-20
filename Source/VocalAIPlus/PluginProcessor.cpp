#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ParameterIDs.h"
#include "../DSP/ParameterHelpers.h"
#include "../DSP/AudioSafety.h"

namespace vocalaiplus
{

using namespace ParamIDs;
using namespace vocalplus;

VocalAIPlusAudioProcessor::VocalAIPlusAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "VocalAIPlus", createParameterLayout())
{
    applyPreset (0);
}

VocalAIPlusAudioProcessor::~VocalAIPlusAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout VocalAIPlusAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    juce::StringArray genres;
    for (int g = 0; g < static_cast<int> (VocalGenre::numGenres); ++g)
        genres.add (LocalMasteringAI::genreName (static_cast<VocalGenre> (g)));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (genre, "Genre", genres, 0));

    params.push_back (std::make_unique<juce::AudioParameterBool> (noiseEnabled, "NR On", true));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (noiseAmount, "NR",
        linearRange (0.0f, 1.0f, 0.01f), 0.55f));

    params.push_back (std::make_unique<juce::AudioParameterBool> (eqEnabled, "EQ On", true));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (eqLowShelfDb, "Low",
        rangedRange (-12.0f, 12.0f, 0.1f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (eqLowMidDb, "Low Mid",
        rangedRange (-12.0f, 12.0f, 0.1f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (eqPresenceDb, "Presence",
        rangedRange (-12.0f, 12.0f, 0.1f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (eqHighShelfDb, "Air",
        rangedRange (-12.0f, 12.0f, 0.1f), 0.0f));

    params.push_back (std::make_unique<juce::AudioParameterBool> (compEnabled, "Comp On", true));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (compThresholdDb, "Threshold",
        rangedRange (-40.0f, 0.0f, 0.1f), -18.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (compRatio, "Ratio",
        rangedRange (1.0f, 12.0f, 0.1f), 3.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (compAttackMs, "Attack",
        rangedRange (1.0f, 80.0f, 0.1f), 8.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (compReleaseMs, "Release",
        rangedRange (40.0f, 400.0f, 1.0f), 120.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (compMakeupDb, "Makeup",
        rangedRange (0.0f, 18.0f, 0.1f), 4.0f));

    params.push_back (std::make_unique<juce::AudioParameterBool> (deEssEnabled, "DeEss On", true));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (deEssFreqHz, "DeEss Freq",
        rangedRange (4000.0f, 10000.0f, 10.0f), 6500.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (deEssReductionDb, "DeEss Cut",
        rangedRange (0.0f, 12.0f, 0.1f), 4.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (deEssSensitivity, "DeEss Sens",
        linearRange (0.0f, 1.0f, 0.01f), 0.5f));

    params.push_back (std::make_unique<juce::AudioParameterBool> (exciterEnabled, "Exciter On", false));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (exciterAmount, "Excite",
        linearRange (0.0f, 1.0f, 0.01f), 0.15f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (exciterMix, "Excite Mix",
        linearRange (0.0f, 1.0f, 0.01f), 0.25f));

    params.push_back (std::make_unique<juce::AudioParameterBool> (reverbEnabled, "Reverb On", false));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (reverbWet, "Reverb",
        linearRange (0.0f, 0.5f, 0.01f), 0.12f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (reverbRoomSize, "Room",
        linearRange (0.0f, 1.0f, 0.01f), 0.35f));

    params.push_back (std::make_unique<juce::AudioParameterBool> (limiterEnabled, "Lim On", true));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (limiterCeilingDbTP, "Ceiling",
        rangedRange (-3.0f, 0.0f, 0.1f), -1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (limiterInputGainDb, "Lim Gain",
        rangedRange (-6.0f, 12.0f, 0.1f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (limiterTargetLUFS, "Target LUFS",
        rangedRange (-16.0f, -7.0f, 0.1f), -11.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (outputGainDb, "Output",
        rangedRange (-12.0f, 12.0f, 0.1f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (mix, "Mix",
        linearRange (0.0f, 1.0f, 0.01f), 1.0f));

    return { params.begin(), params.end() };
}

void VocalAIPlusAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.prepare (sampleRate, samplesPerBlock, getTotalNumOutputChannels());
    analyzer.prepare (sampleRate);
    engine.setSettings (readSettingsFromParameters());
}

void VocalAIPlusAudioProcessor::releaseResources() {}

bool VocalAIPlusAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return layouts.getMainOutputChannelSet() == layouts.getMainInputChannelSet();
}

ChainSettings VocalAIPlusAudioProcessor::readSettingsFromParameters() const
{
    ChainSettings s;
    s.noiseEnabled = readBoolParam (apvts, noiseEnabled);
    s.noiseAmount = readFloatParam (apvts, noiseAmount);
    s.eqEnabled = readBoolParam (apvts, eqEnabled);
    s.eqLowShelfDb = readFloatParam (apvts, eqLowShelfDb);
    s.eqLowMidDb = readFloatParam (apvts, eqLowMidDb);
    s.eqPresenceDb = readFloatParam (apvts, eqPresenceDb);
    s.eqHighShelfDb = readFloatParam (apvts, eqHighShelfDb);
    s.compEnabled = readBoolParam (apvts, compEnabled);
    s.compThresholdDb = readFloatParam (apvts, compThresholdDb);
    s.compRatio = readFloatParam (apvts, compRatio);
    s.compAttackMs = readFloatParam (apvts, compAttackMs);
    s.compReleaseMs = readFloatParam (apvts, compReleaseMs);
    s.compMakeupDb = readFloatParam (apvts, compMakeupDb);
    s.deEssEnabled = readBoolParam (apvts, deEssEnabled);
    s.deEssFreqHz = readFloatParam (apvts, deEssFreqHz);
    s.deEssReductionDb = readFloatParam (apvts, deEssReductionDb);
    s.deEssSensitivity = readFloatParam (apvts, deEssSensitivity);
    s.exciterEnabled = readBoolParam (apvts, exciterEnabled);
    s.exciterAmount = readFloatParam (apvts, exciterAmount);
    s.exciterMix = readFloatParam (apvts, exciterMix);
    s.reverbEnabled = readBoolParam (apvts, reverbEnabled);
    s.reverbWet = readFloatParam (apvts, reverbWet);
    s.reverbRoomSize = readFloatParam (apvts, reverbRoomSize);
    s.limiterEnabled = readBoolParam (apvts, limiterEnabled);
    s.limiterCeilingDbTP = readFloatParam (apvts, limiterCeilingDbTP);
    s.limiterInputGainDb = readFloatParam (apvts, limiterInputGainDb);
    s.limiterTargetLUFS = readFloatParam (apvts, limiterTargetLUFS);
    s.outputGainDb = readFloatParam (apvts, outputGainDb);
    s.mix = readFloatParam (apvts, mix);
    return s;
}

void VocalAIPlusAudioProcessor::applySettingsToParameters (const ChainSettings& s)
{
    auto setFloat = [this] (const juce::String& id, float value)
    {
        if (auto* p = apvts.getParameter (id))
            p->setValueNotifyingHost (apvts.getParameterRange (id).convertTo0to1 (value));
    };

    auto setBool = [this] (const juce::String& id, bool value)
    {
        if (auto* p = apvts.getParameter (id))
            p->setValueNotifyingHost (value ? 1.0f : 0.0f);
    };

    setBool (noiseEnabled, s.noiseEnabled);
    setFloat (noiseAmount, s.noiseAmount);
    setBool (eqEnabled, s.eqEnabled);
    setFloat (eqLowShelfDb, s.eqLowShelfDb);
    setFloat (eqLowMidDb, s.eqLowMidDb);
    setFloat (eqPresenceDb, s.eqPresenceDb);
    setFloat (eqHighShelfDb, s.eqHighShelfDb);
    setBool (compEnabled, s.compEnabled);
    setFloat (compThresholdDb, s.compThresholdDb);
    setFloat (compRatio, s.compRatio);
    setFloat (compAttackMs, s.compAttackMs);
    setFloat (compReleaseMs, s.compReleaseMs);
    setFloat (compMakeupDb, s.compMakeupDb);
    setBool (deEssEnabled, s.deEssEnabled);
    setFloat (deEssFreqHz, s.deEssFreqHz);
    setFloat (deEssReductionDb, s.deEssReductionDb);
    setFloat (deEssSensitivity, s.deEssSensitivity);
    setBool (exciterEnabled, s.exciterEnabled);
    setFloat (exciterAmount, s.exciterAmount);
    setFloat (exciterMix, s.exciterMix);
    setBool (reverbEnabled, s.reverbEnabled);
    setFloat (reverbWet, s.reverbWet);
    setFloat (reverbRoomSize, s.reverbRoomSize);
    setBool (limiterEnabled, s.limiterEnabled);
    setFloat (limiterCeilingDbTP, s.limiterCeilingDbTP);
    setFloat (limiterInputGainDb, s.limiterInputGainDb);
    setFloat (limiterTargetLUFS, s.limiterTargetLUFS);
    setFloat (outputGainDb, s.outputGainDb);
    setFloat (mix, s.mix);
}

void VocalAIPlusAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    if (getSampleRate() <= 0.0 || buffer.getNumSamples() <= 0)
        return;

    analyzer.pushSamples (buffer);
    engine.setSettings (readSettingsFromParameters());
    engine.process (buffer);
    vocalplus::sanitizeBuffer (buffer);
}

juce::AudioProcessorEditor* VocalAIPlusAudioProcessor::createEditor()
{
    return new VocalAIPlusAudioProcessorEditor (*this);
}

int VocalAIPlusAudioProcessor::getNumPrograms()
{
    return presetManager.getNumPresets();
}

int VocalAIPlusAudioProcessor::getCurrentProgram()
{
    return currentPreset;
}

void VocalAIPlusAudioProcessor::setCurrentProgram (int index)
{
    applyPreset (index);
}

const juce::String VocalAIPlusAudioProcessor::getProgramName (int index)
{
    return presetManager.getPreset (index).name;
}

void VocalAIPlusAudioProcessor::applyPreset (int index)
{
    currentPreset = juce::jlimit (0, presetManager.getNumPresets() - 1, index);
    const auto& preset = presetManager.getPreset (currentPreset);
    applySettingsToParameters (preset.settings);

    if (auto* genreParam = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (genre)))
        genreParam->setValueNotifyingHost (static_cast<float> (static_cast<int> (preset.genre))
                                           / static_cast<float> (static_cast<int> (VocalGenre::numGenres) - 1));
}

void VocalAIPlusAudioProcessor::runAIMaster()
{
    const auto genreIdx = readChoiceParam (apvts, genre);
    const auto genreEnum = static_cast<VocalGenre> (juce::jlimit (0, static_cast<int> (VocalGenre::numGenres) - 1, genreIdx));
    const auto decision = LocalMasteringAI::master (analyzer.analyze(), genreEnum);
    applySettingsToParameters (decision.chain);
    lastRationale = decision.rationale;
}

VocalAnalysisSnapshot VocalAIPlusAudioProcessor::getAnalysisSnapshot() const
{
    return analyzer.analyze();
}

void VocalAIPlusAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream (destData, true);
    stream.writeInt (currentPreset);
    apvts.state.writeToStream (stream);
}

void VocalAIPlusAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::MemoryInputStream stream (data, static_cast<size_t> (sizeInBytes), false);
    currentPreset = stream.readInt();
    apvts.replaceState (juce::ValueTree::readFromStream (stream));
}

} // namespace vocalaiplus

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new vocalaiplus::VocalAIPlusAudioProcessor();
}
