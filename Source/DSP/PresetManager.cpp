#include "PresetManager.h"

namespace vocalplus
{

HarmonyVoiceSettings PresetManager::voice (bool enabled, int interval, float level, float pan,
                                            float formant, float delayMs, float vibratoDepth)
{
    HarmonyVoiceSettings v;
    v.enabled = enabled;
    v.intervalSemitones = interval;
    v.level = level;
    v.pan = pan;
    v.formantShift = formant;
    v.delayMs = delayMs;
    v.vibratoDepth = vibratoDepth;
    return v;
}

PresetManager::PresetManager()
{
    // Pop Lead — tight correction, subtle doubles (inspired by modern pop vocal stacks)
    {
        FactoryPreset p;
        p.name = "Pop Lead";
        p.category = "Pop";
        p.description = "Fast retune with airy third-above double for chart-ready leads.";
        p.settings.autoTune = { true, 0.92f, 18.0f, 0.85f, 0, ScaleType::major, 0.0f };
        p.settings.dryLevel = 0.25f;
        p.settings.wetLevel = 1.0f;
        p.settings.stereoWidth = 1.1f;
        p.settings.blendToMono = false;
        p.settings.voices = {
            voice (true, 4, 0.72f, -0.35f, 1.05f, 12.0f, 0.01f),
            voice (true, 7, 0.55f, 0.45f, 0.95f, 18.0f, 0.015f),
            voice (false, -3, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f)
        };
        addPreset (std::move (p));
    }

    // T-Pain Effect — hard tune, minimal dry (classic trap/R&B)
    {
        FactoryPreset p;
        p.name = "Hard Tune FX";
        p.category = "Hip-Hop / R&B";
        p.description = "Zero-tolerance robotic correction with wide stereo doubles.";
        p.settings.autoTune = { true, 1.0f, 0.0f, 0.5f, 0, ScaleType::naturalMinor, 0.0f };
        p.settings.dryLevel = 0.05f;
        p.settings.wetLevel = 1.0f;
        p.settings.stereoWidth = 1.4f;
        p.settings.voices = {
            voice (true, 0, 0.55f, -0.7f, 1.15f, 8.0f, 0.0f),
            voice (true, 0, 0.55f, 0.7f, 0.85f, 16.0f, 0.0f),
            voice (true, 7, 0.35f, 0.0f, 1.0f, 0.0f, 0.02f)
        };
        addPreset (std::move (p));
    }

    // Gospel Stack — rich thirds and fifths (Harmony Engine style)
    {
        FactoryPreset p;
        p.name = "Gospel Stack";
        p.category = "Gospel / Soul";
        p.description = "Lush third and fifth harmonies with gentle vibrato and room.";
        p.settings.autoTune = { true, 0.65f, 35.0f, 0.9f, 0, ScaleType::major, 0.0f };
        p.settings.dryLevel = 0.4f;
        p.settings.wetLevel = 0.95f;
        p.settings.stereoWidth = 1.25f;
        p.settings.blendToMono = false;
        p.settings.voices = {
            voice (true, 4, 0.85f, -0.55f, 1.0f, 22.0f, 0.025f),
            voice (true, 7, 0.8f, 0.55f, 1.0f, 28.0f, 0.02f),
            voice (true, -3, 0.55f, 0.0f, 0.92f, 35.0f, 0.03f)
        };
        addPreset (std::move (p));
    }

    // Indie Natural — transparent correction (Logic Pitch Correction style)
    {
        FactoryPreset p;
        p.name = "Indie Natural";
        p.category = "Indie / Folk";
        p.description = "Transparent pitch assist with high tolerance preserving vibrato.";
        p.settings.autoTune = { true, 0.35f, 55.0f, 0.95f, 0, ScaleType::major, -3.0f };
        p.settings.dryLevel = 0.12f;
        p.settings.wetLevel = 1.0f;
        p.settings.stereoWidth = 0.9f;
        p.settings.voices = {
            voice (true, 12, 0.2f, -0.2f, 1.08f, 30.0f, 0.01f),
            voice (false, 7, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f),
            voice (false, -5, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f)
        };
        addPreset (std::move (p));
    }

    // EDM Chops — octave stacks and wide chorus
    {
        FactoryPreset p;
        p.name = "EDM Chops";
        p.category = "Electronic";
        p.description = "Octave stacks with detuned unison for festival hook vocals.";
        p.settings.autoTune = { true, 0.88f, 12.0f, 0.7f, 0, ScaleType::naturalMinor, 0.0f };
        p.settings.dryLevel = 0.15f;
        p.settings.wetLevel = 1.05f;
        p.settings.stereoWidth = 1.5f;
        p.settings.voices = {
            voice (true, 12, 0.5f, -0.85f, 1.2f, 5.0f, 0.04f),
            voice (true, -12, 0.45f, 0.85f, 0.8f, 7.0f, 0.04f),
            voice (true, 7, 0.3f, 0.0f, 1.1f, 0.0f, 0.05f)
        };
        addPreset (std::move (p));
    }

    // Choir Cathedral — formant-shifted ensemble
    {
        FactoryPreset p;
        p.name = "Choir Cathedral";
        p.category = "Cinematic";
        p.description = "Formant-shifted ensemble voices for epic cinematic beds.";
        p.settings.autoTune = { true, 0.55f, 28.0f, 0.8f, 0, ScaleType::major, 0.0f };
        p.settings.dryLevel = 0.3f;
        p.settings.wetLevel = 1.0f;
        p.settings.stereoWidth = 1.35f;
        p.settings.voices = {
            voice (true, 4, 0.55f, -0.6f, 0.75f, 40.0f, 0.02f),
            voice (true, -3, 0.5f, 0.6f, 1.25f, 45.0f, 0.025f),
            voice (true, 7, 0.45f, 0.0f, 0.85f, 50.0f, 0.03f)
        };
        addPreset (std::move (p));
    }

    // Country Honky — tight thirds, slight detune
    {
        FactoryPreset p;
        p.name = "Country Honky";
        p.category = "Country";
        p.description = "Classic Nashville-style tight thirds with pedal-steel space.";
        p.settings.autoTune = { true, 0.72f, 22.0f, 0.88f, 0, ScaleType::major, 2.0f };
        p.settings.dryLevel = 0.45f;
        p.settings.wetLevel = 0.9f;
        p.settings.stereoWidth = 1.0f;
        p.settings.voices = {
            voice (true, 4, 0.6f, -0.4f, 1.0f, 15.0f, 0.008f),
            voice (true, -3, 0.35f, 0.35f, 1.02f, 20.0f, 0.01f),
            voice (false, 7, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f)
        };
        addPreset (std::move (p));
    }

    // Lo-Fi Bedroom — soft, detuned, intimate
    {
        FactoryPreset p;
        p.name = "Lo-Fi Bedroom";
        p.category = "Lo-Fi";
        p.description = "Soft correction with detuned whisper doubles.";
        p.settings.autoTune = { true, 0.4f, 45.0f, 0.92f, 0, ScaleType::pentatonicMinor, -8.0f };
        p.settings.dryLevel = 0.5f;
        p.settings.wetLevel = 0.75f;
        p.settings.stereoWidth = 0.85f;
        p.settings.voices = {
            voice (true, 0, 0.25f, -0.25f, 1.05f, 35.0f, 0.06f),
            voice (true, 0, 0.25f, 0.25f, 0.95f, 42.0f, 0.07f),
            voice (false, 7, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f)
        };
        addPreset (std::move (p));
    }
}

void PresetManager::addPreset (FactoryPreset preset)
{
    presets.push_back (std::move (preset));
}

const FactoryPreset& PresetManager::getPreset (int index) const
{
    jassert (index >= 0 && index < static_cast<int> (presets.size()));
    return presets[static_cast<size_t> (index)];
}

int PresetManager::findPresetIndexByName (const juce::String& name) const
{
    for (int i = 0; i < static_cast<int> (presets.size()); ++i)
        if (presets[static_cast<size_t> (i)].name == name)
            return i;

    return 0;
}

juce::StringArray PresetManager::getNoteNames()
{
    return { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
}

juce::StringArray PresetManager::getScaleNames()
{
    return { "Chromatic", "Major", "Natural Minor", "Harmonic Minor",
             "Pentatonic Major", "Pentatonic Minor", "Dorian", "Mixolydian" };
}

} // namespace vocalplus
