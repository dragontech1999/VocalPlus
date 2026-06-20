#pragma once

#include <JuceHeader.h>

namespace eqplus
{

class DynamicEQEnvelope
{
public:
    void prepare (double sampleRate)
    {
        sr = sampleRate;
        reset();
    }

    void reset() noexcept
    {
        envelope = 0.0f;
    }

    void setAttackMs (float ms) noexcept  { attackCoeff = coeffFromMs (ms); }
    void setReleaseMs (float ms) noexcept { releaseCoeff = coeffFromMs (ms); }

    float process (float inputLevelDb, float thresholdDb, float ratio) noexcept
    {
        const float over = inputLevelDb - thresholdDb;
        const float target = over > 0.0f ? -over * (1.0f - 1.0f / juce::jmax (1.0f, ratio)) : 0.0f;
        const float coeff = target > envelope ? attackCoeff : releaseCoeff;
        envelope = target + coeff * (envelope - target);
        return envelope;
    }

private:
    float coeffFromMs (float ms) const noexcept
    {
        if (ms <= 0.0f || sr <= 0.0)
            return 0.0f;
        return std::exp (-1.0f / static_cast<float> (ms * 0.001 * sr));
    }

    double sr = 44100.0;
    float envelope = 0.0f;
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;
};

} // namespace eqplus
