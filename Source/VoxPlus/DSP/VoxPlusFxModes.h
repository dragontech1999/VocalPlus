#pragma once

#include <JuceHeader.h>

namespace voxplus
{

struct FxModuleNames
{
    juce::String title;
    juce::StringArray modes;
};

inline const std::array<FxModuleNames, 6> kFxModules {{
    { "Dynamics", { "Broadcast", "Punch", "Smooth", "Aggressive", "Glue", "Pump" } },
    { "Character", { "Warm", "Grit", "Tape", "Tube", "Bright", "Lo-Fi" } },
    { "Filter", { "Presence", "Dark", "Phone", "Air", "Mid Boost", "De-Harsh" } },
    { "Delay", { "Slap", "Echo", "Wide", "Dotted", "Ping-Pong", "Wash" } },
    { "Reverb", { "Room", "Plate", "Hall", "Shimmer", "Spring", "Ambient" } },
    { "Chop", { "Stutter", "Gate", "Rhythmic", "Granular", "Wobble", "Off-Beat" } }
}};

inline juce::StringArray getTuneScaleNames()
{
    return { "Chromatic", "Major", "Minor", "Pentatonic Maj", "Pentatonic Min" };
}

inline juce::StringArray getNoteNames()
{
    return { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
}

} // namespace voxplus
