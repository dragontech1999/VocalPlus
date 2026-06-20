#include "VoxPlusPresetManager.h"

namespace voxplus
{

FxModuleSettings VoxPlusPresetManager::fx (bool on, int mode, float amount, float tone)
{
    FxModuleSettings m;
    m.enabled = on;
    m.mode = mode;
    m.amount = amount;
    m.tone = tone;
    return m;
}

VoxPlusSettings VoxPlusPresetManager::make (const juce::String&,
                                          float pitch, float formant, float unison, float correction,
                                          int scale,
                                          std::array<FxModuleSettings, 6> modules,
                                          float input, float output,
                                          float glue, float doubling)
{
    VoxPlusSettings s;
    s.pitchShiftSemitones = pitch;
    s.formant = formant;
    s.unison = unison;
    s.correction = correction;
    s.tuneScale = scale;
    s.fx = std::move (modules);
    s.inputGainDb = input;
    s.outputGainDb = output;
    s.superGlue = glue;
    s.doubling = doubling;
    s.voiceMix = 1.0f;
    return s;
}

VoxPlusPresetManager::VoxPlusPresetManager()
{
    addPreset ({ "Pop Lead", "Pop",
        "Chart-ready vocal with airy unison and forward presence.",
        make ("", 0, 1.05f, 0.35f, 0.65f, 1,
              { fx (true, 4, 0.45f), fx (true, 4, 0.2f), fx (true, 0, 0.4f),
                fx (true, 0, 0.15f), fx (true, 0, 0.2f), fx (false, 0, 0.0f) },
              1.0f, 0.0f, 0.25f, 0.3f) });

    addPreset ({ "Trap Hard Tune", "Hip-Hop",
        "Zero-tolerance tuning with punchy dynamics and chop accents.",
        make ("", 0, 0.9f, 0.55f, 0.95f, 2,
              { fx (true, 1, 0.7f), fx (true, 1, 0.35f), fx (true, 0, 0.35f),
                fx (true, 2, 0.1f), fx (false, 0, 0.0f), fx (true, 0, 0.4f) },
              3.0f, 1.0f, 0.45f, 0.5f) });

    addPreset ({ "Gospel Power", "Gospel",
        "Warm body, sustained compression, and shimmering reverb.",
        make ("", 0, 1.1f, 0.45f, 0.5f, 1,
              { fx (true, 0, 0.55f), fx (true, 0, 0.3f), fx (true, 3, 0.25f),
                fx (true, 1, 0.1f), fx (true, 2, 0.35f), fx (false, 0, 0.0f) },
              2.0f, 0.5f, 0.2f, 0.4f) });

    addPreset ({ "Indie Natural", "Indie",
        "Transparent voice shaping with gentle warmth.",
        make ("", 0, 1.0f, 0.15f, 0.25f, 1,
              { fx (true, 2, 0.25f), fx (true, 0, 0.1f), fx (true, 4, 0.1f),
                fx (false, 0, 0.0f), fx (true, 0, 0.12f), fx (false, 0, 0.0f) },
              0.0f, 0.0f, 0.0f, 0.15f) });

    addPreset ({ "EDM Festival", "Electronic",
        "Pumped dynamics, wide delay, and rhythmic chop energy.",
        make ("", 0, 0.95f, 0.6f, 0.8f, 0,
              { fx (true, 5, 0.75f), fx (true, 4, 0.25f), fx (true, 3, 0.45f),
                fx (true, 4, 0.35f), fx (true, 5, 0.3f), fx (true, 2, 0.35f) },
              2.5f, 2.0f, 0.55f, 0.45f) });

    addPreset ({ "Country Twang", "Country",
        "Midrange presence with light saturation and slap delay.",
        make ("", -1, 1.15f, 0.2f, 0.35f, 1,
              { fx (true, 2, 0.35f), fx (true, 2, 0.25f), fx (true, 4, 0.35f),
                fx (true, 0, 0.2f), fx (false, 0, 0.0f), fx (false, 0, 0.0f) },
              1.0f, 0.0f, 0.15f, 0.25f) });

    addPreset ({ "Jazz Intimate", "Jazz",
        "Soft correction with warm character and room reverb.",
        make ("", 0, 1.05f, 0.1f, 0.15f, 4,
              { fx (true, 2, 0.2f), fx (true, 0, 0.15f), fx (true, 1, 0.1f),
                fx (false, 0, 0.0f), fx (true, 0, 0.25f), fx (false, 0, 0.0f) },
              -1.0f, -0.5f, 0.0f, 0.1f) });

    addPreset ({ "Latin Heat", "Latin",
        "Punchy dynamics with bright filter and echo delay.",
        make ("", 0, 1.0f, 0.35f, 0.55f, 1,
              { fx (true, 1, 0.6f), fx (true, 4, 0.3f), fx (true, 0, 0.45f),
                fx (true, 1, 0.3f), fx (true, 1, 0.15f), fx (false, 0, 0.0f) },
              2.0f, 1.0f, 0.3f, 0.35f) });

    addPreset ({ "Rock Scream", "Rock",
        "Aggressive glue, grit character, and super compression.",
        make ("", 2, 0.85f, 0.25f, 0.4f, 0,
              { fx (true, 3, 0.8f), fx (true, 1, 0.65f), fx (true, 0, 0.3f),
                fx (true, 2, 0.15f), fx (true, 3, 0.2f), fx (false, 0, 0.0f) },
              4.0f, 2.0f, 0.7f, 0.2f) });

    addPreset ({ "Lo-Fi Bedroom", "Lo-Fi",
        "Rolled-off highs, tape warmth, and subtle wobble chop.",
        make ("", 0, 1.2f, 0.1f, 0.2f, 4,
              { fx (true, 2, 0.3f), fx (true, 2, 0.55f), fx (true, 1, 0.45f),
                fx (true, 5, 0.15f), fx (true, 5, 0.2f), fx (true, 4, 0.15f) },
              0.0f, -2.0f, 0.1f, 0.0f) });

    addPreset ({ "Podcast Voice", "Spoken",
        "Levelled speech with de-harsh filter and gentle dynamics.",
        make ("", 0, 1.0f, 0.0f, 0.1f, 0,
              { fx (true, 0, 0.35f), fx (false, 0, 0.0f), fx (true, 5, 0.35f),
                fx (false, 0, 0.0f), fx (false, 0, 0.0f), fx (false, 0, 0.0f) },
              0.0f, 0.0f, 0.2f, 0.0f) });

    addPreset ({ "R&B Silky", "R&B",
        "Smooth tuning, tube warmth, and plate reverb sheen.",
        make ("", 0, 1.08f, 0.4f, 0.55f, 2,
              { fx (true, 2, 0.4f), fx (true, 3, 0.25f), fx (true, 3, 0.3f),
                fx (true, 1, 0.12f), fx (true, 1, 0.35f), fx (false, 0, 0.0f) },
              1.0f, 0.0f, 0.2f, 0.35f) });

    addPreset ({ "Hyperpop Shift", "Pop",
        "Extreme pitch/formant shift with shimmer reverb.",
        make ("", 7, 0.75f, 0.7f, 0.85f, 0,
              { fx (true, 1, 0.5f), fx (true, 4, 0.45f), fx (true, 3, 0.55f),
                fx (true, 3, 0.25f), fx (true, 3, 0.45f), fx (true, 3, 0.25f) },
              2.0f, 1.5f, 0.35f, 0.55f) });

    addPreset ({ "Vocoder Feel", "Electronic",
        "Formant-down shift with phone filter and rhythmic gate.",
        make ("", -5, 0.7f, 0.5f, 0.7f, 0,
              { fx (true, 3, 0.55f), fx (true, 5, 0.4f), fx (true, 2, 0.65f),
                fx (true, 0, 0.1f), fx (true, 4, 0.2f), fx (true, 1, 0.5f) },
              1.5f, 0.5f, 0.4f, 0.3f) });

    addPreset ({ "Cathedral Choir", "Cinematic",
        "Wide unison, hall reverb, and smooth dynamics.",
        make ("", 0, 1.15f, 0.75f, 0.45f, 1,
              { fx (true, 2, 0.35f), fx (true, 0, 0.15f), fx (true, 3, 0.2f),
                fx (true, 2, 0.2f), fx (true, 2, 0.65f), fx (false, 0, 0.0f) },
              0.0f, 0.0f, 0.15f, 0.6f) });

    addPreset ({ "Phone Memo", "Creative",
        "Band-pass phone filter with lo-fi character.",
        make ("", 0, 1.0f, 0.0f, 0.0f, 0,
              { fx (false, 0, 0.0f), fx (true, 5, 0.5f), fx (true, 2, 0.75f),
                fx (false, 0, 0.0f), fx (false, 0, 0.0f), fx (false, 0, 0.0f) },
              0.0f, 0.0f, 0.0f, 0.0f) });

    addPreset ({ "Stutter FX", "Creative",
        "Hard chop stutter with aggressive dynamics.",
        make ("", 0, 1.0f, 0.2f, 0.6f, 0,
              { fx (true, 1, 0.65f), fx (true, 1, 0.3f), fx (false, 0, 0.0f),
                fx (false, 0, 0.0f), fx (false, 0, 0.0f), fx (true, 0, 0.75f) },
              2.0f, 1.0f, 0.5f, 0.0f) });

    addPreset ({ "Ballad Air", "Pop",
        "Soft tuning, air filter boost, and ambient wash delay.",
        make ("", 0, 1.1f, 0.45f, 0.4f, 1,
              { fx (true, 2, 0.3f), fx (true, 0, 0.12f), fx (true, 3, 0.5f),
                fx (true, 5, 0.35f), fx (true, 5, 0.4f), fx (false, 0, 0.0f) },
              0.5f, 0.0f, 0.1f, 0.35f) });

    addPreset ({ "Radio Ready", "Utility",
        "Broadcast dynamics with focus EQ and auto level.",
        make ("", 0, 1.0f, 0.15f, 0.35f, 1,
              { fx (true, 0, 0.5f), fx (true, 4, 0.15f), fx (true, 0, 0.25f),
                fx (false, 0, 0.0f), fx (true, 0, 0.1f), fx (false, 0, 0.0f) },
              0.0f, 0.0f, 0.35f, 0.2f) });

    addPreset ({ "Transparent Glue", "Utility",
        "Minimal voice processing with super glue mix bus compression.",
        make ("", 0, 1.0f, 0.0f, 0.1f, 0,
              { fx (true, 4, 0.25f), fx (false, 0, 0.0f), fx (false, 0, 0.0f),
                fx (false, 0, 0.0f), fx (false, 0, 0.0f), fx (false, 0, 0.0f) },
              0.0f, 0.0f, 0.55f, 0.0f) });
}

const GenrePreset& VoxPlusPresetManager::getPreset (int index) const
{
    return presets[static_cast<size_t> (juce::jlimit (0, getNumPresets() - 1, index))];
}

void VoxPlusPresetManager::addPreset (GenrePreset preset)
{
    presets.push_back (std::move (preset));
}

} // namespace voxplus
