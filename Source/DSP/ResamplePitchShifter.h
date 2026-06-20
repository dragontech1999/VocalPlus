#pragma once

#include <JuceHeader.h>
#include <vector>

namespace vocalplus
{

class ResamplePitchShifter
{
public:
    void prepare (double sampleRate, int maxBlockSize);
    void reset();
    void setPitchRatio (float ratio);
    void setPitchSemitones (float semitones);

    void processBlock (const float* input, float* output, int numSamples);

private:
    float readInterpolated (double pos) const;

    std::vector<float> ringBuffer;
    double readPos = 0.0;
    int writeIndex = 0;
    float pitchRatio = 1.0f;
    double sampleRate = 44100.0;
};

} // namespace vocalplus
