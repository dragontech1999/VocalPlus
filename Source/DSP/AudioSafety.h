#pragma once

#include <JuceHeader.h>
#include <cmath>

namespace vocalplus
{

/** Sanitize and soft-limit audio — prevents NaN/Inf/static when stacking multiple plugins. */
inline void sanitizeBuffer (juce::AudioBuffer<float>& buffer, float hardLimit = 4.0f) noexcept
{
    const int channels = buffer.getNumChannels();
    const int samples = buffer.getNumSamples();

    for (int ch = 0; ch < channels; ++ch)
    {
        auto* data = buffer.getWritePointer (ch);

        for (int i = 0; i < samples; ++i)
        {
            float x = data[i];

            if (! std::isfinite (x))
                x = 0.0f;

            if (x > hardLimit)
                x = hardLimit;
            else if (x < -hardLimit)
                x = -hardLimit;

            data[i] = x;
        }
    }
}

inline float safeGain (float linearGain, float maxGain = 4.0f) noexcept
{
    if (! std::isfinite (linearGain))
        return 1.0f;

    return juce::jlimit (0.0f, maxGain, linearGain);
}

inline float safeAutoLevelGain (float peak, float target, float maxBoost = 4.0f) noexcept
{
    if (! std::isfinite (peak) || peak <= 1.0e-6f)
        return 1.0f;

    return juce::jlimit (0.0f, maxBoost, target / peak);
}

} // namespace vocalplus
