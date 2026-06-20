#include "StemExporter.h"

namespace vocalplus
{

StemExporter::StemExporter() : juce::Thread ("VocalPlus Stem Export")
{
    harmonyCapture.resize (3);
}

StemExporter::~StemExporter()
{
    stopThread (5000);
}

void StemExporter::prepare (double sr, int numChannels)
{
    sampleRate = sr;
    channels = numChannels;
    reset();
}

void StemExporter::reset()
{
    const juce::ScopedLock lock (bufferLock);
    dryCapture.clear();
    correctedCapture.clear();
    blendCapture.clear();

    for (auto& h : harmonyCapture)
        h.clear();

    capturing = false;
    exportReady = false;
}

void StemExporter::beginCapture()
{
    const juce::ScopedLock lock (bufferLock);
    dryCapture.clear();
    correctedCapture.clear();
    blendCapture.clear();

    for (auto& h : harmonyCapture)
        h.clear();

    capturing = true;
    exportReady = false;
}

void StemExporter::captureBlock (const juce::AudioBuffer<float>& dry,
                                 const juce::AudioBuffer<float>& corrected,
                                 const std::vector<juce::AudioBuffer<float>>& harmonies,
                                 const juce::AudioBuffer<float>& blend)
{
    if (! capturing.load())
        return;

    const juce::ScopedLock lock (bufferLock);

    auto appendBuffer = [] (juce::AudioBuffer<float>& dest, const juce::AudioBuffer<float>& src)
    {
        const int newLength = dest.getNumSamples() + src.getNumSamples();
        dest.setSize (juce::jmax (dest.getNumChannels(), src.getNumChannels()), newLength, true, true, true);

        for (int ch = 0; ch < src.getNumChannels(); ++ch)
            dest.copyFrom (ch, dest.getNumSamples() - src.getNumSamples(), src, ch, 0, src.getNumSamples());
    };

    appendBuffer (dryCapture, dry);
    appendBuffer (correctedCapture, corrected);
    appendBuffer (blendCapture, blend);

    for (size_t i = 0; i < harmonyCapture.size() && i < harmonies.size(); ++i)
        appendBuffer (harmonyCapture[i], harmonies[i]);
}

void StemExporter::exportStemsAsync (std::function<void (bool, juce::String)> callback)
{
    completionCallback = std::move (callback);
    exportRequested = true;
    capturing = false;
    exportReady = true;

    if (! isThreadRunning())
        startThread();
    else
        notify();
}

void StemExporter::run()
{
    while (! threadShouldExit())
    {
        if (! exportRequested.exchange (false))
        {
            wait (500);
            continue;
        }

        juce::AudioBuffer<float> dry, corrected, blend;
        std::vector<juce::AudioBuffer<float>> harmonies (3);

        {
            const juce::ScopedLock lock (bufferLock);
            dry.makeCopyOf (dryCapture);
            corrected.makeCopyOf (correctedCapture);
            blend.makeCopyOf (blendCapture);

            for (size_t i = 0; i < harmonies.size(); ++i)
                harmonies[i].makeCopyOf (harmonyCapture[i]);
        }

        const auto timestamp = juce::Time::getCurrentTime().formatted ("%Y-%m-%d_%H-%M-%S");
        const auto folder = juce::File::getSpecialLocation (juce::File::userMusicDirectory)
                                .getChildFile ("VocalPlus Exports")
                                .getChildFile ("Session_" + timestamp);

        const bool created = folder.createDirectory();

        bool success = created;
        success = success && writeStem (folder, "01_Dry", dry);
        success = success && writeStem (folder, "02_Corrected", corrected);
        success = success && writeStem (folder, "03_Harmony_1", harmonies[0]);
        success = success && writeStem (folder, "04_Harmony_2", harmonies[1]);
        success = success && writeStem (folder, "05_Harmony_3", harmonies[2]);
        success = success && writeStem (folder, "06_Blend", blend);

        lastExportFolder = folder;

        if (completionCallback)
        {
            juce::MessageManager::callAsync ([success, folder, cb = completionCallback]
            {
                cb (success, success ? folder.getFullPathName() : juce::String());
            });
        }
    }
}

bool StemExporter::writeStem (const juce::File& folder, const juce::String& name, const juce::AudioBuffer<float>& buffer)
{
    if (buffer.getNumSamples() == 0)
        return true;

    const auto file = folder.getChildFile (name + ".wav");
    std::unique_ptr<juce::FileOutputStream> stream (file.createOutputStream());

    if (stream == nullptr)
        return false;

    juce::WavAudioFormat wav;
    std::unique_ptr<juce::AudioFormatWriter> writer (
        wav.createWriterFor (stream.get(), sampleRate,
                             static_cast<unsigned int> (juce::jmax (1, buffer.getNumChannels())),
                             24, {}, 0));

    if (writer == nullptr)
        return false;

    stream.release();
    return writer->writeFromAudioSampleBuffer (buffer, 0, buffer.getNumSamples());
}

} // namespace vocalplus
