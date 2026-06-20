#include "TunePlusPresetManager.h"
#include "../../DSP/PresetManager.h"

namespace tuneplus
{

vocalplus::HarmonyVoiceSettings TunePlusPresetManager::voice (bool enabled, int interval, float level, float pan,
                                                              float formant, float delayMs, float vibratoDepth)
{
    vocalplus::HarmonyVoiceSettings v;
    v.enabled = enabled;
    v.intervalSemitones = interval;
    v.level = level;
    v.pan = pan;
    v.formantShift = formant;
    v.delayMs = delayMs;
    v.vibratoDepth = vibratoDepth;
    return v;
}

CompressorSettings TunePlusPresetManager::comp (bool enabled, float threshold, float ratio, float attack,
                                              float release, float makeup, float knee, float highPass, float mix)
{
    CompressorSettings c;
    c.enabled = enabled;
    c.thresholdDb = threshold;
    c.ratio = ratio;
    c.attackMs = attack;
    c.releaseMs = release;
    c.makeupDb = makeup;
    c.kneeDb = knee;
    c.highPassHz = highPass;
    c.mix = mix;
    return c;
}

TunePlusPresetManager::TunePlusPresetManager()
{
    // ── Vocal compressor presets ─────────────────────────────────────────────
    addCompressorPreset ({ "Broadcast Vocal", "Vocal", "Tight broadcast chain with fast attack and gentle knee.",
        comp (true, -24.0f, 5.0f, 6.0f, 90.0f, 8.0f, 8.0f, 100.0f) });
    addCompressorPreset ({ "Radio Ready", "Vocal", "Classic radio vocal — moderate ratio, smooth release.",
        comp (true, -20.0f, 4.5f, 10.0f, 130.0f, 7.0f, 10.0f, 90.0f) });
    addCompressorPreset ({ "Aggressive Rap", "Hip-Hop", "Punchy, in-your-face rap vocal with heavy gain reduction.",
        comp (true, -28.0f, 8.0f, 2.0f, 70.0f, 10.0f, 3.0f, 120.0f) });
    addCompressorPreset ({ "Smooth Ballad", "Ballad", "Slow attack preserves breath and dynamics for emotional vocals.",
        comp (true, -18.0f, 3.5f, 30.0f, 220.0f, 5.0f, 12.0f, 70.0f) });
    addCompressorPreset ({ "Pop Punch", "Pop", "Bright pop vocal with controlled peaks and forward presence.",
        comp (true, -22.0f, 5.5f, 8.0f, 100.0f, 9.0f, 6.0f, 110.0f) });
    addCompressorPreset ({ "Podcast Leveler", "Speech", "Gentle leveling for spoken word and podcast dialogue.",
        comp (true, -22.0f, 3.0f, 20.0f, 200.0f, 4.0f, 15.0f, 80.0f) });
    addCompressorPreset ({ "Gospel Power", "Gospel", "Sustained compression for powerful belting and choir leads.",
        comp (true, -15.0f, 5.0f, 12.0f, 180.0f, 5.5f, 8.0f, 85.0f) });
    addCompressorPreset ({ "EDM Sidechain Feel", "Electronic", "Fast pump-style compression for EDM toplines.",
        comp (true, -26.0f, 8.0f, 1.5f, 60.0f, 10.0f, 2.0f, 140.0f) });
    addCompressorPreset ({ "Country Twang", "Country", "Light touch preserving Nashville pick dynamics.",
        comp (true, -17.0f, 3.0f, 18.0f, 160.0f, 3.5f, 10.0f, 95.0f) });
    addCompressorPreset ({ "Transparent Glue", "Utility", "Parallel-style gentle glue at 50% mix.",
        comp (true, -12.0f, 2.0f, 25.0f, 300.0f, 2.0f, 20.0f, 60.0f, 0.5f) });

    // ── Genre presets (auto-tune + harmony + linked compressor) ──────────────
    {
        GenrePreset p;
        p.name = "Pop Lead";
        p.category = "Pop";
        p.description = "Chart-ready lead with tight correction and airy doubles.";
        p.linkedCompressorIndex = 4; // Pop Punch
        p.settings.harmony.autoTune = { true, 0.92f, 18.0f, 0.85f, 0, vocalplus::ScaleType::major, 0.0f };
        p.settings.harmony.dryLevel = 0.25f;
        p.settings.harmony.wetLevel = 1.0f;
        p.settings.harmony.stereoWidth = 1.1f;
        p.settings.harmony.blendToMono = false;
        p.settings.harmony.voices = {
            voice (true, 4, 0.72f, -0.35f, 1.05f, 12.0f, 0.01f),
            voice (true, 7, 0.55f, 0.45f, 0.95f, 18.0f, 0.015f),
            voice (false, -3, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f)
        };
        p.settings.compressor = getCompressorPreset (4).settings;
        addGenrePreset (std::move (p));
    }

    {
        GenrePreset p;
        p.name = "Hard Tune FX";
        p.category = "Hip-Hop / R&B";
        p.description = "Robotic zero-tolerance tuning with wide stereo doubles.";
        p.linkedCompressorIndex = 2;
        p.settings.harmony.autoTune = { true, 1.0f, 0.0f, 0.5f, 0, vocalplus::ScaleType::naturalMinor, 0.0f };
        p.settings.harmony.dryLevel = 0.05f;
        p.settings.harmony.wetLevel = 1.0f;
        p.settings.harmony.stereoWidth = 1.4f;
        p.settings.harmony.voices = {
            voice (true, 0, 0.55f, -0.7f, 1.15f, 8.0f, 0.0f),
            voice (true, 0, 0.55f, 0.7f, 0.85f, 16.0f, 0.0f),
            voice (true, 7, 0.35f, 0.0f, 1.0f, 0.0f, 0.02f)
        };
        p.settings.compressor = getCompressorPreset (2).settings;
        addGenrePreset (std::move (p));
    }

    {
        GenrePreset p;
        p.name = "Trap Melodic";
        p.category = "Hip-Hop / R&B";
        p.description = "Melodic trap with hard tune and aggressive vocal compression.";
        p.linkedCompressorIndex = 2;
        p.settings.harmony.autoTune = { true, 0.98f, 5.0f, 0.6f, 0, vocalplus::ScaleType::naturalMinor, -4.0f };
        p.settings.harmony.dryLevel = 0.1f;
        p.settings.harmony.wetLevel = 1.05f;
        p.settings.harmony.stereoWidth = 1.35f;
        p.settings.harmony.voices = {
            voice (true, 12, 0.35f, -0.5f, 1.1f, 6.0f, 0.03f),
            voice (true, -12, 0.3f, 0.5f, 0.9f, 10.0f, 0.03f),
            voice (false, 7, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f)
        };
        p.settings.compressor = getCompressorPreset (2).settings;
        addGenrePreset (std::move (p));
    }

    {
        GenrePreset p;
        p.name = "Gospel Stack";
        p.category = "Gospel / Soul";
        p.description = "Lush thirds and fifths with gentle vibrato and room.";
        p.linkedCompressorIndex = 6;
        p.settings.harmony.autoTune = { true, 0.65f, 35.0f, 0.9f, 0, vocalplus::ScaleType::major, 0.0f };
        p.settings.harmony.dryLevel = 0.4f;
        p.settings.harmony.wetLevel = 0.95f;
        p.settings.harmony.stereoWidth = 1.25f;
        p.settings.harmony.blendToMono = false;
        p.settings.harmony.voices = {
            voice (true, 4, 0.85f, -0.55f, 1.0f, 22.0f, 0.025f),
            voice (true, 7, 0.8f, 0.55f, 1.0f, 28.0f, 0.02f),
            voice (true, -3, 0.55f, 0.0f, 0.92f, 35.0f, 0.03f)
        };
        p.settings.compressor = getCompressorPreset (6).settings;
        addGenrePreset (std::move (p));
    }

    {
        GenrePreset p;
        p.name = "Indie Natural";
        p.category = "Indie / Folk";
        p.description = "Transparent pitch assist preserving natural vibrato.";
        p.linkedCompressorIndex = 9;
        p.settings.harmony.autoTune = { true, 0.35f, 55.0f, 0.95f, 0, vocalplus::ScaleType::major, -3.0f };
        p.settings.harmony.dryLevel = 0.55f;
        p.settings.harmony.wetLevel = 0.85f;
        p.settings.harmony.stereoWidth = 0.9f;
        p.settings.harmony.voices = {
            voice (true, 12, 0.2f, -0.2f, 1.08f, 30.0f, 0.01f),
            voice (false, 7, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f),
            voice (false, -5, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f)
        };
        p.settings.compressor = getCompressorPreset (9).settings;
        addGenrePreset (std::move (p));
    }

    {
        GenrePreset p;
        p.name = "EDM Festival";
        p.category = "Electronic";
        p.description = "Octave stacks with detuned unison for festival hook vocals.";
        p.linkedCompressorIndex = 7;
        p.settings.harmony.autoTune = { true, 0.88f, 12.0f, 0.7f, 0, vocalplus::ScaleType::naturalMinor, 0.0f };
        p.settings.harmony.dryLevel = 0.15f;
        p.settings.harmony.wetLevel = 1.05f;
        p.settings.harmony.stereoWidth = 1.5f;
        p.settings.harmony.voices = {
            voice (true, 12, 0.5f, -0.85f, 1.2f, 5.0f, 0.04f),
            voice (true, -12, 0.45f, 0.85f, 0.8f, 7.0f, 0.04f),
            voice (true, 7, 0.3f, 0.0f, 1.1f, 0.0f, 0.05f)
        };
        p.settings.compressor = getCompressorPreset (7).settings;
        addGenrePreset (std::move (p));
    }

    {
        GenrePreset p;
        p.name = "Choir Cathedral";
        p.category = "Cinematic";
        p.description = "Formant-shifted ensemble for epic cinematic beds.";
        p.linkedCompressorIndex = 6;
        p.settings.harmony.autoTune = { true, 0.55f, 28.0f, 0.8f, 0, vocalplus::ScaleType::major, 0.0f };
        p.settings.harmony.dryLevel = 0.3f;
        p.settings.harmony.wetLevel = 1.0f;
        p.settings.harmony.stereoWidth = 1.35f;
        p.settings.harmony.voices = {
            voice (true, 4, 0.55f, -0.6f, 0.75f, 40.0f, 0.02f),
            voice (true, -3, 0.5f, 0.6f, 1.25f, 45.0f, 0.025f),
            voice (true, 7, 0.45f, 0.0f, 0.85f, 50.0f, 0.03f)
        };
        p.settings.compressor = getCompressorPreset (6).settings;
        addGenrePreset (std::move (p));
    }

    {
        GenrePreset p;
        p.name = "Country Honky";
        p.category = "Country";
        p.description = "Nashville-style tight thirds with pedal-steel space.";
        p.linkedCompressorIndex = 8;
        p.settings.harmony.autoTune = { true, 0.72f, 22.0f, 0.88f, 0, vocalplus::ScaleType::major, 2.0f };
        p.settings.harmony.dryLevel = 0.45f;
        p.settings.harmony.wetLevel = 0.9f;
        p.settings.harmony.stereoWidth = 1.0f;
        p.settings.harmony.voices = {
            voice (true, 4, 0.6f, -0.4f, 1.0f, 15.0f, 0.008f),
            voice (true, -3, 0.35f, 0.35f, 1.02f, 20.0f, 0.01f),
            voice (false, 7, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f)
        };
        p.settings.compressor = getCompressorPreset (8).settings;
        addGenrePreset (std::move (p));
    }

    {
        GenrePreset p;
        p.name = "Lo-Fi Bedroom";
        p.category = "Lo-Fi";
        p.description = "Soft correction with detuned whisper doubles.";
        p.linkedCompressorIndex = 9;
        p.settings.harmony.autoTune = { true, 0.4f, 45.0f, 0.92f, 0, vocalplus::ScaleType::pentatonicMinor, -8.0f };
        p.settings.harmony.dryLevel = 0.5f;
        p.settings.harmony.wetLevel = 0.75f;
        p.settings.harmony.stereoWidth = 0.85f;
        p.settings.harmony.voices = {
            voice (true, 0, 0.25f, -0.25f, 1.05f, 35.0f, 0.06f),
            voice (true, 0, 0.25f, 0.25f, 0.95f, 42.0f, 0.07f),
            voice (false, 7, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f)
        };
        p.settings.compressor = getCompressorPreset (9).settings;
        addGenrePreset (std::move (p));
    }

    {
        GenrePreset p;
        p.name = "Latin Reggaeton";
        p.category = "Latin";
        p.description = "Tight melodic correction for reggaeton and Latin pop hooks.";
        p.linkedCompressorIndex = 4;
        p.settings.harmony.autoTune = { true, 0.9f, 10.0f, 0.75f, 0, vocalplus::ScaleType::naturalMinor, 0.0f };
        p.settings.harmony.dryLevel = 0.2f;
        p.settings.harmony.wetLevel = 1.0f;
        p.settings.harmony.stereoWidth = 1.2f;
        p.settings.harmony.voices = {
            voice (true, 7, 0.4f, -0.4f, 1.0f, 10.0f, 0.02f),
            voice (true, -5, 0.3f, 0.4f, 0.95f, 14.0f, 0.015f),
            voice (false, 12, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f)
        };
        p.settings.compressor = getCompressorPreset (4).settings;
        addGenrePreset (std::move (p));
    }

    {
        GenrePreset p;
        p.name = "Metal Scream";
        p.category = "Rock / Metal";
        p.description = "Fast retune with minimal harmony for aggressive vocals.";
        p.linkedCompressorIndex = 2;
        p.settings.harmony.autoTune = { true, 0.95f, 8.0f, 0.55f, 0, vocalplus::ScaleType::naturalMinor, 0.0f };
        p.settings.harmony.dryLevel = 0.35f;
        p.settings.harmony.wetLevel = 0.95f;
        p.settings.harmony.stereoWidth = 1.15f;
        p.settings.harmony.voices = {
            voice (true, 12, 0.25f, -0.3f, 1.3f, 4.0f, 0.0f),
            voice (false, 7, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f),
            voice (false, -12, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f)
        };
        p.settings.compressor = getCompressorPreset (2).settings;
        addGenrePreset (std::move (p));
    }

    {
        GenrePreset p;
        p.name = "Jazz Standards";
        p.category = "Jazz";
        p.description = "Light touch correction respecting jazz phrasing and vibrato.";
        p.linkedCompressorIndex = 3;
        p.settings.harmony.autoTune = { true, 0.25f, 65.0f, 0.98f, 0, vocalplus::ScaleType::dorian, -5.0f };
        p.settings.harmony.dryLevel = 0.65f;
        p.settings.harmony.wetLevel = 0.7f;
        p.settings.harmony.stereoWidth = 0.95f;
        p.settings.harmony.voices = {
            voice (true, 3, 0.15f, -0.15f, 1.0f, 25.0f, 0.02f),
            voice (false, 7, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f),
            voice (false, -3, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f)
        };
        p.settings.compressor = getCompressorPreset (3).settings;
        addGenrePreset (std::move (p));
    }
}

void TunePlusPresetManager::addGenrePreset (GenrePreset preset)
{
    genrePresets.push_back (std::move (preset));
}

void TunePlusPresetManager::addCompressorPreset (CompressorPreset preset)
{
    compressorPresets.push_back (std::move (preset));
}

const GenrePreset& TunePlusPresetManager::getGenrePreset (int index) const
{
    jassert (index >= 0 && index < static_cast<int> (genrePresets.size()));
    return genrePresets[static_cast<size_t> (index)];
}

const CompressorPreset& TunePlusPresetManager::getCompressorPreset (int index) const
{
    jassert (index >= 0 && index < static_cast<int> (compressorPresets.size()));
    return compressorPresets[static_cast<size_t> (index)];
}

juce::StringArray TunePlusPresetManager::getGenreCategories() const
{
    juce::StringArray cats;
    for (const auto& p : genrePresets)
        if (! cats.contains (p.category))
            cats.add (p.category);
    return cats;
}

juce::StringArray TunePlusPresetManager::getCompressorCategories() const
{
    juce::StringArray cats;
    for (const auto& p : compressorPresets)
        if (! cats.contains (p.category))
            cats.add (p.category);
    return cats;
}

juce::StringArray TunePlusPresetManager::getNoteNames()
{
    return vocalplus::PresetManager::getNoteNames();
}

juce::StringArray TunePlusPresetManager::getScaleNames()
{
    return vocalplus::PresetManager::getScaleNames();
}

} // namespace tuneplus
