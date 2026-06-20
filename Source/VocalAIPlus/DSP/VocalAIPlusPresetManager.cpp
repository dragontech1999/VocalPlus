#include "VocalAIPlusPresetManager.h"

namespace vocalaiplus
{

namespace
{
ChainSettings makeChain (bool noiseOn, float noiseAmt,
                       float low, float lowMid, float pres, float air,
                       float thr, float ratio, float atk, float rel, float makeup,
                       bool deEss, float deFreq, float deAmt,
                       bool exc, float excAmt,
                       bool rev, float revWet,
                       float limGain, float targetLufs)
{
    ChainSettings s;
    s.noiseEnabled = noiseOn;
    s.noiseAmount = noiseAmt;
    s.eqEnabled = true;
    s.eqLowShelfDb = low;
    s.eqLowMidDb = lowMid;
    s.eqPresenceDb = pres;
    s.eqHighShelfDb = air;
    s.compEnabled = true;
    s.compThresholdDb = thr;
    s.compRatio = ratio;
    s.compAttackMs = atk;
    s.compReleaseMs = rel;
    s.compMakeupDb = makeup;
    s.deEssEnabled = deEss;
    s.deEssFreqHz = deFreq;
    s.deEssReductionDb = deAmt;
    s.deEssSensitivity = 0.5f;
    s.exciterEnabled = exc;
    s.exciterAmount = excAmt;
    s.exciterMix = 0.22f;
    s.reverbEnabled = rev;
    s.reverbWet = revWet;
    s.reverbRoomSize = 0.35f;
    s.limiterEnabled = true;
    s.limiterCeilingDbTP = -1.0f;
    s.limiterInputGainDb = limGain;
    s.limiterTargetLUFS = targetLufs;
    s.outputGainDb = 0.0f;
    s.mix = 1.0f;
    return s;
}
} // namespace

VocalAIPlusPresetManager::VocalAIPlusPresetManager()
{
    VocalAnalysisSnapshot neutral;
    neutral.integratedLUFS = -16.0f;
    neutral.truePeakDBTP = -4.0f;
    neutral.crestFactorDB = 12.0f;
    neutral.spectralCentroidHz = 2600.0f;
    neutral.sibilanceIndex = 0.28f;
    neutral.lowMidMudIndex = 0.30f;
    neutral.presenceIndex = 0.35f;
    neutral.valid = true;

    for (int g = 0; g < static_cast<int> (VocalGenre::numGenres); ++g)
    {
        const auto genre = static_cast<VocalGenre> (g);
        addPreset ({ LocalMasteringAI::genreName (genre) + " Master",
                     LocalMasteringAI::genreHint (genre),
                     genre,
                     LocalMasteringAI::synthesis (neutral, genre) });
    }

    addPreset ({ "Podcast Voice", "Clear speech, gentle dynamics", VocalGenre::Acoustic,
                 makeChain (true, 0.5f, -0.5f, -1.5f, 1.0f, 0.5f,
                            -24.0f, 2.5f, 12.0f, 150.0f, 2.0f,
                            true, 7000.0f, 5.0f, false, 0.0f, false, 0.0f, 1.0f, -14.0f) });

    addPreset ({ "Lo-Fi Warmth", "Dusty top, soft limiter", VocalGenre::Soul,
                 makeChain (true, 0.35f, 1.2f, 0.5f, 0.0f, -1.5f,
                            -20.0f, 3.0f, 10.0f, 140.0f, 3.0f,
                            false, 6500.0f, 3.0f, true, 0.12f, false, 0.0f, 0.5f, -12.0f) });
}

const MasterPreset& VocalAIPlusPresetManager::getPreset (int index) const
{
    return presets[static_cast<size_t> (juce::jlimit (0, getNumPresets() - 1, index))];
}

void VocalAIPlusPresetManager::addPreset (MasterPreset preset)
{
    presets.push_back (std::move (preset));
}

} // namespace vocalaiplus
