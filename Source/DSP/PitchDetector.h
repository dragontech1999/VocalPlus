#pragma once

#include <JuceHeader.h>
#include <cmath>
#include <vector>

namespace vocalplus
{

class PitchDetector
{
public:
    void prepare (double sampleRate, int maxBlockSize);
    void reset();

    /** Returns detected frequency in Hz, or 0 if unvoiced. */
    float process (const float* samples, int numSamples);

    float getConfidence() const noexcept { return confidence; }
    float getDetectedMidiNote() const noexcept { return detectedMidiNote; }

private:
    float yinDifference (const float* samples, int numSamples, int tau) const;
    float parabolicInterpolation (const std::vector<float>& yinBuffer, int tau) const;

    double sampleRate = 44100.0;
    std::vector<float> yinBuffer;
    float confidence = 0.0f;
    float detectedMidiNote = 0.0f;
    static constexpr float kMinFreq = 65.0f;
    static constexpr float kMaxFreq = 1200.0f;
};

} // namespace vocalplus
