#pragma once

#include <JuceHeader.h>

namespace voxplus::ParamIDs
{
    inline constexpr auto inputGain  = "inputGain";
    inline constexpr auto outputGain = "outputGain";
    inline constexpr auto globalMix  = "globalMix";
    inline constexpr auto autoLevel  = "autoLevel";
    inline constexpr auto lowCut     = "lowCut";
    inline constexpr auto highCut    = "highCut";
    inline constexpr auto doubling   = "doubling";
    inline constexpr auto focusEQ    = "focusEQ";
    inline constexpr auto superGlue  = "superGlue";

    inline constexpr auto voiceBypass   = "voiceBypass";
    inline constexpr auto voiceMix      = "voiceMix";
    inline constexpr auto pitchShift    = "pitchShift";
    inline constexpr auto formant       = "formant";
    inline constexpr auto unison        = "unison";
    inline constexpr auto correction    = "correction";
    inline constexpr auto rootNote      = "rootNote";
    inline constexpr auto tuneScale     = "tuneScale";

    inline juce::String fxEnabled (int i) { return "fx" + juce::String (i) + "Enabled"; }
    inline juce::String fxMode    (int i) { return "fx" + juce::String (i) + "Mode"; }
    inline juce::String fxAmount  (int i) { return "fx" + juce::String (i) + "Amount"; }
    inline juce::String fxTone    (int i) { return "fx" + juce::String (i) + "Tone"; }

    inline constexpr int numFxModules = 6;
}
