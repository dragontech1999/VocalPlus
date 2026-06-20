#pragma once

#include <JuceHeader.h>
#include <array>
#include <cmath>

namespace vocalplus
{

enum class ScaleType
{
    chromatic = 0,
    major,
    naturalMinor,
    harmonicMinor,
    pentatonicMajor,
    pentatonicMinor,
    dorian,
    mixolydian
};

class ScaleQuantizer
{
public:
    static float quantizeMidi (float midiNote, int rootNote, ScaleType scale, float toleranceCents)
    {
        if (scale == ScaleType::chromatic || toleranceCents >= 100.0f)
            return midiNote;

        const auto scaleDegrees = getScaleDegrees (scale);
        const float noteInOctave = std::fmod (midiNote, 12.0f);
        const float octave = std::floor (midiNote / 12.0f);
        const float relative = std::fmod (noteInOctave - static_cast<float> (rootNote) + 12.0f, 12.0f);

        float bestDistance = 100.0f;
        float bestDegree = relative;

        for (const auto degree : scaleDegrees)
        {
            const float dist = std::abs (relative - static_cast<float> (degree));
            const float wrapped = std::min (dist, 12.0f - dist);

            if (wrapped < bestDistance)
            {
                bestDistance = wrapped;
                bestDegree = static_cast<float> (degree);
            }
        }

        const float centsOff = std::abs (relative - bestDegree) * 100.0f;
        if (centsOff <= toleranceCents)
            return midiNote;

        const float quantized = octave * 12.0f + static_cast<float> (rootNote) + bestDegree;
        return quantized;
    }

    static juce::String scaleName (ScaleType scale)
    {
        switch (scale)
        {
            case ScaleType::chromatic: return "Chromatic";
            case ScaleType::major: return "Major";
            case ScaleType::naturalMinor: return "Natural Minor";
            case ScaleType::harmonicMinor: return "Harmonic Minor";
            case ScaleType::pentatonicMajor: return "Pentatonic Major";
            case ScaleType::pentatonicMinor: return "Pentatonic Minor";
            case ScaleType::dorian: return "Dorian";
            case ScaleType::mixolydian: return "Mixolydian";
            default: return "Major";
        }
    }

private:
    static std::array<int, 12> getScaleDegrees (ScaleType scale)
    {
        switch (scale)
        {
            case ScaleType::major: return { 0, 2, 4, 5, 7, 9, 11, 0, 0, 0, 0, 0 };
            case ScaleType::naturalMinor: return { 0, 2, 3, 5, 7, 8, 10, 0, 0, 0, 0, 0 };
            case ScaleType::harmonicMinor: return { 0, 2, 3, 5, 7, 8, 11, 0, 0, 0, 0, 0 };
            case ScaleType::pentatonicMajor: return { 0, 2, 4, 7, 9, 0, 0, 0, 0, 0, 0, 0 };
            case ScaleType::pentatonicMinor: return { 0, 3, 5, 7, 10, 0, 0, 0, 0, 0, 0, 0 };
            case ScaleType::dorian: return { 0, 2, 3, 5, 7, 9, 10, 0, 0, 0, 0, 0 };
            case ScaleType::mixolydian: return { 0, 2, 4, 5, 7, 9, 10, 0, 0, 0, 0, 0 };
            case ScaleType::chromatic:
            default: return { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
        }
    }
};

} // namespace vocalplus
