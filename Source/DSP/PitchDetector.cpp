#include "PitchDetector.h"

namespace vocalplus
{

void PitchDetector::prepare (double sr, int /*maxBlockSize*/)
{
    sampleRate = sr;
    const int maxLag = static_cast<int> (sampleRate / kMinFreq) + 2;
    yinBuffer.assign (static_cast<size_t> (maxLag), 0.0f);
    reset();
}

void PitchDetector::reset()
{
    confidence = 0.0f;
    detectedMidiNote = 0.0f;
    std::fill (yinBuffer.begin(), yinBuffer.end(), 0.0f);
}

float PitchDetector::yinDifference (const float* samples, int numSamples, int tau) const
{
    if (tau <= 0 || tau >= numSamples)
        return 1.0f;

    float sum = 0.0f;
    const int limit = numSamples - tau;

    for (int i = 0; i < limit; ++i)
    {
        const float delta = samples[i] - samples[i + tau];
        sum += delta * delta;
    }

    return sum;
}

float PitchDetector::parabolicInterpolation (const std::vector<float>& buffer, int tau) const
{
    if (tau <= 1 || tau >= static_cast<int> (buffer.size()) - 1)
        return static_cast<float> (tau);

    const float s0 = buffer[static_cast<size_t> (tau - 1)];
    const float s1 = buffer[static_cast<size_t> (tau)];
    const float s2 = buffer[static_cast<size_t> (tau + 1)];
    const float denom = 2.0f * (2.0f * s1 - s2 - s0);

    if (std::abs (denom) < 1.0e-6f)
        return static_cast<float> (tau);

    return static_cast<float> (tau) + (s2 - s0) / denom;
}

float PitchDetector::process (const float* samples, int numSamples)
{
    if (numSamples < 64 || samples == nullptr)
        return 0.0f;

    const int minLag = juce::jmax (2, static_cast<int> (sampleRate / kMaxFreq));
    const int maxLag = juce::jmin (static_cast<int> (yinBuffer.size()) - 1,
                                   static_cast<int> (sampleRate / kMinFreq));

    if (maxLag <= minLag)
        return 0.0f;

    float runningSum = 0.0f;

    for (int tau = 1; tau <= maxLag; ++tau)
    {
        const float diff = yinDifference (samples, numSamples, tau);
        runningSum += diff;
        yinBuffer[static_cast<size_t> (tau)] = runningSum > 0.0f ? diff * static_cast<float> (tau) / runningSum : 1.0f;
    }

    constexpr float threshold = 0.15f;
    int bestTau = -1;

    for (int tau = minLag; tau <= maxLag; ++tau)
    {
        if (yinBuffer[static_cast<size_t> (tau)] < threshold)
        {
            while (tau + 1 <= maxLag && yinBuffer[static_cast<size_t> (tau + 1)] < yinBuffer[static_cast<size_t> (tau)])
                ++tau;

            bestTau = tau;
            break;
        }
    }

    if (bestTau < 0)
    {
        confidence = 0.0f;
        detectedMidiNote = 0.0f;
        return 0.0f;
    }

    const float refinedTau = parabolicInterpolation (yinBuffer, bestTau);
    const float frequency = static_cast<float> (sampleRate / refinedTau);
    confidence = juce::jlimit (0.0f, 1.0f, 1.0f - yinBuffer[static_cast<size_t> (bestTau)]);
    detectedMidiNote = 69.0f + 12.0f * std::log2 (frequency / 440.0f);

    return frequency;
}

} // namespace vocalplus
