#pragma once

#include <JuceHeader.h>
#include "DSP/EQTypes.h"

namespace eqplus::ParamIDs
{
    inline constexpr auto outputGain = "outputGain";
    inline constexpr auto autoGain = "autoGain";
    inline constexpr auto phaseInvert = "phaseInvert";
    inline constexpr auto processingMode = "processingMode";
    inline constexpr auto displayRange = "displayRange";
    inline constexpr auto spectrumSpeed = "spectrumSpeed";
    inline constexpr auto showPreSpectrum = "showPreSpectrum";
    inline constexpr auto showPostSpectrum = "showPostSpectrum";
    inline constexpr auto selectedBand = "selectedBand";
    inline constexpr auto abState = "abState";

    inline juce::String bandEnabled (int i)     { return "band" + juce::String (i) + "Enabled"; }
    inline juce::String bandSolo (int i)        { return "band" + juce::String (i) + "Solo"; }
    inline juce::String bandType (int i)        { return "band" + juce::String (i) + "Type"; }
    inline juce::String bandChannel (int i)     { return "band" + juce::String (i) + "Channel"; }
    inline juce::String bandFreq (int i)        { return "band" + juce::String (i) + "Freq"; }
    inline juce::String bandGain (int i)        { return "band" + juce::String (i) + "Gain"; }
    inline juce::String bandQ (int i)           { return "band" + juce::String (i) + "Q"; }
    inline juce::String bandDynEnabled (int i)  { return "band" + juce::String (i) + "DynEnabled"; }
    inline juce::String bandDynThreshold (int i){ return "band" + juce::String (i) + "DynThreshold"; }
    inline juce::String bandDynRatio (int i)    { return "band" + juce::String (i) + "DynRatio"; }
    inline juce::String bandDynAttack (int i)   { return "band" + juce::String (i) + "DynAttack"; }
    inline juce::String bandDynRelease (int i)  { return "band" + juce::String (i) + "DynRelease"; }
}
