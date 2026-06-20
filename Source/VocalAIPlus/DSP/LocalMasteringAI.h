#pragma once

#include "VocalAnalysisEngine.h"
#include "VocalAIPlusEngine.h"
#include <JuceHeader.h>

namespace vocalaiplus
{

enum class VocalGenre
{
    Pop = 0,
    HipHop,
    Rnb,
    Rock,
    Country,
    Edm,
    Jazz,
    Acoustic,
    Trap,
    Soul,
    numGenres
};

struct AIMasteringDecision
{
    ChainSettings chain;
    juce::String rationale;
    float confidence = 0.82f;
};

class LocalMasteringAI
{
public:
    static float targetLUFSForGenre (VocalGenre genre);
    static juce::String genreName (VocalGenre genre);
    static juce::String genreHint (VocalGenre genre);

    static AIMasteringDecision master (const VocalAnalysisSnapshot& analysis, VocalGenre genre);
    static ChainSettings synthesis (const VocalAnalysisSnapshot& analysis, VocalGenre genre);

private:
    static float genreLowShelf (VocalGenre genre);
    static float genreAirBoost (VocalGenre genre);
    static float genreCompressionRatio (VocalGenre genre);
    static float genrePresenceBoost (VocalGenre genre);
    static juce::String buildRationale (const VocalAnalysisSnapshot& analysis,
                                        VocalGenre genre, const ChainSettings& chain);
};

} // namespace vocalaiplus
