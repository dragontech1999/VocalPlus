#include "LocalMasteringAI.h"

namespace vocalaiplus
{

float LocalMasteringAI::targetLUFSForGenre (VocalGenre genre)
{
    switch (genre)
    {
        case VocalGenre::Pop:
        case VocalGenre::Rnb:
        case VocalGenre::Soul: return -11.0f;
        case VocalGenre::HipHop:
        case VocalGenre::Trap: return -9.0f;
        case VocalGenre::Rock:
        case VocalGenre::Country: return -10.0f;
        case VocalGenre::Edm: return -8.0f;
        case VocalGenre::Jazz:
        case VocalGenre::Acoustic: return -14.0f;
        default: return -11.0f;
    }
}

juce::String LocalMasteringAI::genreName (VocalGenre genre)
{
    static const juce::StringArray names { "Pop", "Hip-Hop", "R&B", "Rock", "Country",
                                           "EDM", "Jazz", "Acoustic", "Trap", "Soul" };
    return names[juce::jlimit (0, names.size() - 1, static_cast<int> (genre))];
}

juce::String LocalMasteringAI::genreHint (VocalGenre genre)
{
    switch (genre)
    {
        case VocalGenre::Pop: return "Bright, upfront, controlled sibilance.";
        case VocalGenre::HipHop: return "Punchy mids, forward presence.";
        case VocalGenre::Rnb: return "Warm body, silky top.";
        case VocalGenre::Rock: return "Gritty presence, wider dynamics.";
        case VocalGenre::Country: return "Natural air, vocal-forward.";
        case VocalGenre::Edm: return "Hyper-present, bright air.";
        case VocalGenre::Jazz: return "Natural dynamics, airy room.";
        case VocalGenre::Acoustic: return "Transparent, soft top.";
        case VocalGenre::Trap: return "Dry, hyped highs.";
        case VocalGenre::Soul: return "Thick low-mids, vintage warmth.";
        default: return "Genre-aware vocal mastering.";
    }
}

float LocalMasteringAI::genreLowShelf (VocalGenre genre)
{
    switch (genre)
    {
        case VocalGenre::HipHop:
        case VocalGenre::Trap:
        case VocalGenre::Rnb:
        case VocalGenre::Soul: return 1.8f;
        case VocalGenre::Rock: return 0.8f;
        case VocalGenre::Jazz:
        case VocalGenre::Acoustic: return -0.5f;
        default: return 0.4f;
    }
}

float LocalMasteringAI::genreAirBoost (VocalGenre genre)
{
    switch (genre)
    {
        case VocalGenre::Pop:
        case VocalGenre::Edm: return 2.2f;
        case VocalGenre::Country:
        case VocalGenre::Acoustic: return 1.2f;
        case VocalGenre::Jazz: return 0.6f;
        default: return 1.5f;
    }
}

float LocalMasteringAI::genreCompressionRatio (VocalGenre genre)
{
    switch (genre)
    {
        case VocalGenre::Edm:
        case VocalGenre::Trap:
        case VocalGenre::HipHop: return 5.5f;
        case VocalGenre::Rock:
        case VocalGenre::Pop: return 4.0f;
        case VocalGenre::Jazz:
        case VocalGenre::Acoustic: return 2.2f;
        default: return 3.2f;
    }
}

float LocalMasteringAI::genrePresenceBoost (VocalGenre genre)
{
    switch (genre)
    {
        case VocalGenre::Pop:
        case VocalGenre::Edm:
        case VocalGenre::Trap: return 2.8f;
        case VocalGenre::Rock: return 2.2f;
        case VocalGenre::Rnb:
        case VocalGenre::Soul: return 1.6f;
        case VocalGenre::Jazz:
        case VocalGenre::Acoustic: return 0.8f;
        default: return 1.4f;
    }
}

ChainSettings LocalMasteringAI::synthesis (const VocalAnalysisSnapshot& analysis, VocalGenre genre)
{
    ChainSettings chain;

    chain.noiseEnabled = true;
    chain.noiseAmount = analysis.sibilanceIndex > 0.42f ? 0.7f : 0.45f;

    chain.eqEnabled = true;
    chain.eqLowShelfDb = genreLowShelf (genre) - analysis.lowMidMudIndex * 4.0f;
    chain.eqLowMidDb = analysis.lowMidMudIndex > 0.38f ? -2.5f : 0.5f;
    chain.eqPresenceDb = genrePresenceBoost (genre) - analysis.presenceIndex * 3.0f;
    chain.eqHighShelfDb = genreAirBoost (genre)
        + (analysis.spectralCentroidHz < 2400.0f ? 1.5f : 0.0f);

    chain.compEnabled = true;
    chain.compThresholdDb = -22.0f + analysis.crestFactorDB * 0.35f;
    chain.compRatio = genreCompressionRatio (genre);
    chain.compAttackMs = (genre == VocalGenre::Edm || genre == VocalGenre::Trap) ? 3.0f : 8.0f;
    chain.compReleaseMs = (genre == VocalGenre::Jazz || genre == VocalGenre::Acoustic) ? 180.0f : 110.0f;
    chain.compMakeupDb = juce::jmax (0.0f, (targetLUFSForGenre (genre) - analysis.integratedLUFS) * 0.35f);

    chain.deEssEnabled = analysis.sibilanceIndex > 0.32f;
    chain.deEssFreqHz = analysis.spectralCentroidHz > 2800.0f ? 7200.0f : 6200.0f;
    chain.deEssReductionDb = juce::jmin (9.0f, 3.0f + analysis.sibilanceIndex * 8.0f);
    chain.deEssSensitivity = 0.35f + analysis.sibilanceIndex * 0.5f;

    chain.exciterEnabled = genre == VocalGenre::Rock || genre == VocalGenre::Edm || genre == VocalGenre::Trap;
    chain.exciterAmount = genre == VocalGenre::Edm ? 0.35f : 0.18f;
    chain.exciterMix = 0.22f;

    chain.reverbEnabled = genre == VocalGenre::Jazz || genre == VocalGenre::Acoustic || genre == VocalGenre::Soul;
    chain.reverbWet = genre == VocalGenre::Jazz ? 0.18f : 0.1f;
    chain.reverbRoomSize = genre == VocalGenre::Acoustic ? 0.28f : 0.42f;

    chain.limiterEnabled = true;
    chain.limiterTargetLUFS = targetLUFSForGenre (genre);
    chain.limiterCeilingDbTP = -1.0f;
    const float loudnessDelta = targetLUFSForGenre (genre) - analysis.integratedLUFS;
    chain.limiterInputGainDb = juce::jlimit (-2.0f, 8.0f, loudnessDelta * 0.65f);
    chain.limiterInputGainDb = (genre == VocalGenre::HipHop || genre == VocalGenre::Trap) ? chain.limiterInputGainDb : chain.limiterInputGainDb;

    chain.outputGainDb = 0.0f;
    chain.mix = 1.0f;
    return chain;
}

juce::String LocalMasteringAI::buildRationale (const VocalAnalysisSnapshot& analysis,
                                               VocalGenre genre, const ChainSettings& chain)
{
    return juce::String::formatted (
        "Genre %s (%s). Detected %.1f LUFS, %.1f dBTP peak, crest %.1f dB. "
        "Applied %s, %s, target %.0f LUFS.",
        genreName (genre).toRawUTF8(),
        genreHint (genre).toRawUTF8(),
        analysis.integratedLUFS,
        analysis.truePeakDBTP,
        analysis.crestFactorDB,
        chain.compEnabled ? "vocal compressor" : "minimal dynamics",
        chain.deEssEnabled ? juce::String ("de-esser at " + juce::String (static_cast<int> (chain.deEssFreqHz)) + " Hz").toRawUTF8()
                           : "light sibilance control",
        chain.limiterTargetLUFS);
}

AIMasteringDecision LocalMasteringAI::master (const VocalAnalysisSnapshot& analysis, VocalGenre genre)
{
    AIMasteringDecision decision;
    decision.chain = synthesis (analysis, genre);
    decision.rationale = buildRationale (analysis, genre, decision.chain);
    decision.confidence = analysis.valid ? 0.82f : 0.65f;
    return decision;
}

} // namespace vocalaiplus
