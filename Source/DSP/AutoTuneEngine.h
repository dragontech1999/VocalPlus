#pragma once

#include "PitchDetector.h"
#include "ResamplePitchShifter.h"
#include "ScaleQuantizer.h"

namespace vocalplus
{

struct AutoTuneSettings
{
    bool enabled = true;
    float retuneSpeed = 0.85f;
    float toleranceCents = 25.0f;
    float formantPreserve = 0.75f;
    int rootNote = 0;
    ScaleType scale = ScaleType::major;
    float detuneCents = 0.0f;
};

class AutoTuneEngine
{
public:
    void prepare (double sampleRate, int maxBlockSize);
    void reset();

    void setSettings (const AutoTuneSettings& newSettings);
    void process (const float* input, float* output, int numSamples);

    float getCurrentPitchHz() const noexcept { return currentPitchHz; }
    float getTargetMidiNote() const noexcept { return targetMidiNote; }
    float getCorrectionAmount() const noexcept { return correctionAmount; }

private:
    AutoTuneSettings settings;
    PitchDetector detector;
    ResamplePitchShifter shifter;
    std::vector<float> analysisBuffer;
    std::vector<float> orderedAnalysis;
    std::vector<float> scratch;

    float smoothedTargetMidi = 69.0f;
    float currentPitchHz = 0.0f;
    float targetMidiNote = 0.0f;
    float correctionAmount = 0.0f;
    float currentPitchRatio = 1.0f;
    int analysisWritePos = 0;
};

} // namespace vocalplus
