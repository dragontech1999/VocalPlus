#pragma once

#include <JuceHeader.h>

namespace eqplus
{

inline constexpr int kMaxBands = 24;
inline constexpr float kMinFreq = 10.0f;
inline constexpr float kMaxFreq = 30000.0f;
inline constexpr float kMinGain = -30.0f;
inline constexpr float kMaxGain = 30.0f;
inline constexpr float kMinQ = 0.025f;
inline constexpr float kMaxQ = 40.0f;

enum class FilterType
{
    Bell = 0,
    LowShelf,
    HighShelf,
    LowCut,
    HighCut,
    Notch,
    BandPass,
    TiltShelf,
    AllPass
};

enum class ChannelMode
{
    Stereo = 0,
    Mid,
    Side,
    Left,
    Right
};

enum class ProcessingMode
{
    ZeroLatency = 0,
    NaturalPhase,
    LinearPhase
};

enum class DisplayRange
{
    Range3dB = 0,
    Range6dB,
    Range12dB,
    Range30dB
};

inline juce::StringArray getFilterTypeNames()
{
    return { "Bell", "Low Shelf", "High Shelf", "Low Cut", "High Cut",
             "Notch", "Band Pass", "Tilt Shelf", "All Pass" };
}

inline juce::StringArray getChannelModeNames()
{
    return { "Stereo", "Mid", "Side", "Left", "Right" };
}

inline juce::StringArray getProcessingModeNames()
{
    return { "Zero Latency", "Natural Phase", "Linear Phase" };
}

inline juce::StringArray getDisplayRangeNames()
{
    return { "±3 dB", "±6 dB", "±12 dB", "±30 dB" };
}

inline float displayRangeToDb (DisplayRange range)
{
    switch (range)
    {
        case DisplayRange::Range3dB:  return 3.0f;
        case DisplayRange::Range6dB:  return 6.0f;
        case DisplayRange::Range12dB: return 12.0f;
        case DisplayRange::Range30dB: return 30.0f;
    }
    return 12.0f;
}

struct BandSettings
{
    bool enabled = false;
    bool solo = false;
    FilterType type = FilterType::Bell;
    ChannelMode channel = ChannelMode::Stereo;
    float frequency = 1000.0f;
    float gainDb = 0.0f;
    float q = 1.0f;
    float slope = 12.0f;

    bool dynamicEnabled = false;
    float dynThresholdDb = -20.0f;
    float dynRatio = 2.0f;
    float dynAttackMs = 10.0f;
    float dynReleaseMs = 100.0f;
};

struct EQGlobalSettings
{
    float outputGainDb = 0.0f;
    bool autoGain = false;
    bool phaseInvert = false;
    ProcessingMode processingMode = ProcessingMode::ZeroLatency;
    DisplayRange displayRange = DisplayRange::Range12dB;
    float spectrumSpeed = 0.7f;
    bool showPreSpectrum = true;
    bool showPostSpectrum = true;
};

} // namespace eqplus
