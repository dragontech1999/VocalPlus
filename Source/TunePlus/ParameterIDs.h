#pragma once

#include <JuceHeader.h>

namespace tuneplus::ParamIDs
{
    inline constexpr auto genrePreset = "genrePreset";
    inline constexpr auto compressorPreset = "compressorPreset";

    inline constexpr auto autoTuneEnabled = "autoTuneEnabled";
    inline constexpr auto retuneSpeed = "retuneSpeed";
    inline constexpr auto tolerance = "tolerance";
    inline constexpr auto formantPreserve = "formantPreserve";
    inline constexpr auto rootNote = "rootNote";
    inline constexpr auto scaleType = "scaleType";
    inline constexpr auto detune = "detune";
    inline constexpr auto dryLevel = "dryLevel";
    inline constexpr auto wetLevel = "wetLevel";
    inline constexpr auto stereoWidth = "stereoWidth";
    inline constexpr auto blendToMono = "blendToMono";

    inline constexpr auto compEnabled = "compEnabled";
    inline constexpr auto compThreshold = "compThreshold";
    inline constexpr auto compRatio = "compRatio";
    inline constexpr auto compAttack = "compAttack";
    inline constexpr auto compRelease = "compRelease";
    inline constexpr auto compMakeup = "compMakeup";
    inline constexpr auto compHighPass = "compHighPass";
    inline constexpr auto compMix = "compMix";

    inline juce::String voiceEnabled (int i) { return "voice" + juce::String (i) + "Enabled"; }
    inline juce::String voiceInterval (int i) { return "voice" + juce::String (i) + "Interval"; }
    inline juce::String voiceLevel (int i) { return "voice" + juce::String (i) + "Level"; }
    inline juce::String voicePan (int i) { return "voice" + juce::String (i) + "Pan"; }
    inline juce::String voiceFormant (int i) { return "voice" + juce::String (i) + "Formant"; }
    inline juce::String voiceDelay (int i) { return "voice" + juce::String (i) + "Delay"; }
    inline juce::String voiceVibrato (int i) { return "voice" + juce::String (i) + "Vibrato"; }
}
