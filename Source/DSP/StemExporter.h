#pragma once

#include <JuceHeader.h>
#include <functional>

namespace vocalplus
{

class StemExporter : public juce::Thread
{
public:
    StemExporter();
    ~StemExporter() override;

    void prepare (double sampleRate, int numChannels);
    void reset();

    void beginCapture();
    void captureBlock (const juce::AudioBuffer<float>& dry,
                       const juce::AudioBuffer<float>& corrected,
                       const std::vector<juce::AudioBuffer<float>>& harmonies,
                       const juce::AudioBuffer<float>& blend);

    bool isCapturing() const noexcept { return capturing; }
    bool isExportReady() const noexcept { return exportReady; }

    /** Exports captured stems to WAV files in ~/Music/VocalPlus Exports/<timestamp>/ */
    void exportStemsAsync (std::function<void (bool success, juce::String message)> callback);

    juce::File getLastExportFolder() const noexcept { return lastExportFolder; }

private:
    void run() override;
    bool writeStem (const juce::File& folder, const juce::String& name, const juce::AudioBuffer<float>& buffer);

    double sampleRate = 44100.0;
    int channels = 2;
    std::atomic<bool> capturing { false };
    std::atomic<bool> exportReady { false };
    std::atomic<bool> exportRequested { false };

    juce::CriticalSection bufferLock;
    juce::AudioBuffer<float> dryCapture;
    juce::AudioBuffer<float> correctedCapture;
    std::vector<juce::AudioBuffer<float>> harmonyCapture;
    juce::AudioBuffer<float> blendCapture;

    juce::File lastExportFolder;
    std::function<void (bool, juce::String)> completionCallback;
};

} // namespace vocalplus
