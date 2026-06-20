#include "SmartEQAssistant.h"

namespace eqplus
{

std::vector<EQSuggestion> SmartEQAssistant::analyze (const SpectrumAnalyzer& analyzer, double sampleRate)
{
    std::vector<EQSuggestion> suggestions;
    const auto& spectrum = analyzer.hasSignal() ? analyzer.getAccumulatedSpectrum()
                                                : analyzer.getSpectrum (false);

    float peakDb = -100.0f;
    float totalEnergy = 0.0f;
    int count = 0;

    for (int i = 2; i < SpectrumAnalyzer::fftSize / 2; ++i)
    {
        const float db = spectrum[static_cast<size_t> (i)];
        peakDb = juce::jmax (peakDb, db);
        if (analyzer.getBinFrequency (i) >= 60.0f && analyzer.getBinFrequency (i) <= 18000.0f)
        {
            totalEnergy += db;
            ++count;
        }
    }

    const float avgEnergy = count > 0 ? totalEnergy / static_cast<float> (count) : -70.0f;
    const float threshold = juce::jmax (avgEnergy + 4.0f, peakDb - 18.0f);

    const auto peaks = findPeaks (spectrum, threshold);
    for (const auto peakBin : peaks)
    {
        const float freq = analyzer.getBinFrequency (peakBin);
        if (freq < 80.0f || freq > 16000.0f)
            continue;

        const float prominence = spectrum[static_cast<size_t> (peakBin)] - avgEnergy;
        if (prominence < 3.0f)
            continue;

        EQSuggestion s;
        s.type = FilterType::Bell;
        s.frequency = freq;
        s.gainDb = -juce::jlimit (1.5f, 9.0f, prominence * 0.65f);
        s.q = freq > 2500.0f ? juce::jmap (prominence, 3.0f, 12.0f, 2.5f, 6.0f) : 1.8f;

        if (freq > 2800.0f && freq < 7500.0f)
            s.reason = "Harsh resonance";
        else if (freq < 350.0f)
            s.reason = "Low-mid buildup";
        else if (freq > 8000.0f)
            s.reason = "Sibilant peak";
        else
            s.reason = "Resonant peak";

        suggestions.push_back (s);
        if (suggestions.size() >= 5)
            break;
    }

    const float lowBuildup = estimateLowEndBuildup (spectrum, sampleRate);
    if (lowBuildup > 4.0f)
    {
        EQSuggestion hp;
        hp.type = FilterType::LowCut;
        hp.frequency = lowBuildup > 8.0f ? 45.0f : 30.0f;
        hp.gainDb = 0.0f;
        hp.q = 0.707f;
        hp.reason = "Sub-bass cleanup";
        suggestions.insert (suggestions.begin(), hp);
    }

    float lowMidEnergy = 0.0f, presenceEnergy = 0.0f, airEnergy = 0.0f;
    int lowMidCount = 0, presenceCount = 0, airCount = 0;

    for (int i = 2; i < SpectrumAnalyzer::fftSize / 2; ++i)
    {
        const float freq = analyzer.getBinFrequency (i);
        const float e = spectrum[static_cast<size_t> (i)];
        if (freq >= 200.0f && freq < 600.0f) { lowMidEnergy += e; ++lowMidCount; }
        if (freq >= 2500.0f && freq <= 5500.0f) { presenceEnergy += e; ++presenceCount; }
        if (freq > 9000.0f) { airEnergy += e; ++airCount; }
    }

    const float lowMidAvg = lowMidCount > 0 ? lowMidEnergy / static_cast<float> (lowMidCount) : avgEnergy;
    const float presenceAvg = presenceCount > 0 ? presenceEnergy / static_cast<float> (presenceCount) : avgEnergy;
    const float airAvg = airCount > 0 ? airEnergy / static_cast<float> (airCount) : avgEnergy;

    if (lowMidAvg > avgEnergy + 3.0f)
    {
        EQSuggestion mud;
        mud.type = FilterType::Bell;
        mud.frequency = 350.0f;
        mud.gainDb = -juce::jmin (5.0f, (lowMidAvg - avgEnergy) * 0.5f);
        mud.q = 1.2f;
        mud.reason = "Reduce mud";
        suggestions.push_back (mud);
    }

    if (presenceAvg < avgEnergy - 2.5f)
    {
        EQSuggestion presence;
        presence.type = FilterType::Bell;
        presence.frequency = 3800.0f;
        presence.gainDb = juce::jmin (4.0f, (avgEnergy - presenceAvg) * 0.4f + 1.5f);
        presence.q = 1.0f;
        presence.reason = "Add presence";
        suggestions.push_back (presence);
    }

    if (airAvg < avgEnergy - 4.0f)
    {
        EQSuggestion air;
        air.type = FilterType::HighShelf;
        air.frequency = 11000.0f;
        air.gainDb = juce::jmin (3.5f, (avgEnergy - airAvg) * 0.3f + 1.0f);
        air.q = 0.7f;
        air.reason = "Restore air";
        suggestions.push_back (air);
    }

    if (suggestions.empty())
    {
        EQSuggestion gentle;
        gentle.type = FilterType::HighShelf;
        gentle.frequency = 10000.0f;
        gentle.gainDb = 1.5f;
        gentle.q = 0.7f;
        gentle.reason = analyzer.hasSignal() ? "Gentle polish (balanced signal)" : "Default polish (play audio for analysis)";
        suggestions.push_back (gentle);
    }

    return suggestions;
}

std::vector<int> SmartEQAssistant::findPeaks (const std::array<float, SpectrumAnalyzer::fftSize / 2>& spectrum,
                                              float thresholdDb) const
{
    std::vector<int> peaks;
    for (int i = 4; i < SpectrumAnalyzer::fftSize / 2 - 4; ++i)
    {
        const float val = spectrum[static_cast<size_t> (i)];
        if (val < thresholdDb)
            continue;

        bool isPeak = true;
        for (int j = -3; j <= 3; ++j)
        {
            if (j == 0) continue;
            if (spectrum[static_cast<size_t> (i + j)] > val)
            {
                isPeak = false;
                break;
            }
        }

        if (isPeak)
            peaks.push_back (i);
    }
    return peaks;
}

float SmartEQAssistant::estimateLowEndBuildup (const std::array<float, SpectrumAnalyzer::fftSize / 2>& spectrum,
                                               double sampleRate) const
{
    float subEnergy = 0.0f;
    float midEnergy = 0.0f;
    int subCount = 0, midCount = 0;

    for (int i = 2; i < SpectrumAnalyzer::fftSize / 2; ++i)
    {
        const float freq = static_cast<float> (i) * static_cast<float> (sampleRate)
                         / static_cast<float> (SpectrumAnalyzer::fftSize);
        const float e = spectrum[static_cast<size_t> (i)];
        if (freq < 80.0f) { subEnergy += e; ++subCount; }
        else if (freq >= 250.0f && freq < 700.0f) { midEnergy += e; ++midCount; }
    }

    if (subCount == 0 || midCount == 0)
        return 0.0f;

    return (subEnergy / static_cast<float> (subCount)) - (midEnergy / static_cast<float> (midCount));
}

} // namespace eqplus
