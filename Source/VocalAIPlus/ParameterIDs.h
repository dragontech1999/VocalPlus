#pragma once

#include <JuceHeader.h>

namespace vocalaiplus
{
namespace ParamIDs
{
    inline constexpr auto genre = "genre";

    inline constexpr auto noiseEnabled = "noiseEnabled";
    inline constexpr auto noiseAmount = "noiseAmount";

    inline constexpr auto eqEnabled = "eqEnabled";
    inline constexpr auto eqLowShelfDb = "eqLowShelfDb";
    inline constexpr auto eqLowMidDb = "eqLowMidDb";
    inline constexpr auto eqPresenceDb = "eqPresenceDb";
    inline constexpr auto eqHighShelfDb = "eqHighShelfDb";

    inline constexpr auto compEnabled = "compEnabled";
    inline constexpr auto compThresholdDb = "compThresholdDb";
    inline constexpr auto compRatio = "compRatio";
    inline constexpr auto compAttackMs = "compAttackMs";
    inline constexpr auto compReleaseMs = "compReleaseMs";
    inline constexpr auto compMakeupDb = "compMakeupDb";

    inline constexpr auto deEssEnabled = "deEssEnabled";
    inline constexpr auto deEssFreqHz = "deEssFreqHz";
    inline constexpr auto deEssReductionDb = "deEssReductionDb";
    inline constexpr auto deEssSensitivity = "deEssSensitivity";

    inline constexpr auto exciterEnabled = "exciterEnabled";
    inline constexpr auto exciterAmount = "exciterAmount";
    inline constexpr auto exciterMix = "exciterMix";

    inline constexpr auto reverbEnabled = "reverbEnabled";
    inline constexpr auto reverbWet = "reverbWet";
    inline constexpr auto reverbRoomSize = "reverbRoomSize";

    inline constexpr auto limiterEnabled = "limiterEnabled";
    inline constexpr auto limiterCeilingDbTP = "limiterCeilingDbTP";
    inline constexpr auto limiterInputGainDb = "limiterInputGainDb";
    inline constexpr auto limiterTargetLUFS = "limiterTargetLUFS";

    inline constexpr auto outputGainDb = "outputGainDb";
    inline constexpr auto mix = "mix";
} // namespace ParamIDs
} // namespace vocalaiplus
