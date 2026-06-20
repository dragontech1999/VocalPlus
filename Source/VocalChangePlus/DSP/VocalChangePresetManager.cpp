#include "VocalChangePresetManager.h"

namespace vocalchangeplus
{

VocalChangeSettings VocalChangePresetManager::make (float pitch, float formant, float speed,
                                                  float distortion, float reverb, float robot, float chorus,
                                                  int filter, float correction, float mix, float aiMorph)
{
    VocalChangeSettings s;
    s.pitchShiftSemitones = pitch;
    s.formant = formant;
    s.speed = speed;
    s.distortion = distortion;
    s.reverb = reverb;
    s.robot = robot;
    s.chorus = chorus;
    s.filterMode = filter;
    s.correction = correction;
    s.mix = mix;
    s.aiMorph = aiMorph;
    return s;
}

VocalChangePresetManager::VocalChangePresetManager()
{
    addPreset ({ "Myself", "🎤", "Your natural voice — clean pass-through.",
        make (0, 1.0f, 1.0f, 0, 0, 0, 0, 0, 0, 1.0f, 0) });

    addPreset ({ "Robot", "🤖", "Metallic ring-mod with bit-crushed edges.",
        make (0, 0.55f, 1.0f, 0.55f, 0.15f, 0.9f, 0.2f, 0, 0, 1.0f, 0.2f) });

    addPreset ({ "Baby", "👶", "High-pitched and bright — tiny voice energy.",
        make (8, 1.55f, 1.2f, 0, 0.08f, 0, 0.25f, 0, 0, 1.0f, 0.15f) });

    addPreset ({ "Demon", "👹", "Deep, growling underworld presence.",
        make (-10, 0.42f, 0.8f, 0.75f, 0.5f, 0.45f, 0, 0, 0, 1.0f, 0.35f) });

    addPreset ({ "Space Trooper", "🚀", "Helmet comms with chorus and space filter.",
        make (-4, 0.55f, 0.92f, 0.35f, 0.35f, 0.55f, 0.65f, 3, 0, 1.0f, 0.25f) });

    addPreset ({ "Chipmunk", "🐿️", "Hyper-speed squeaky character voice.",
        make (12, 1.45f, 1.55f, 0, 0, 0, 0.15f, 0, 0, 1.0f, 0) });

    addPreset ({ "Deep Voice", "🎙️", "Broadcast baritone — big and authoritative.",
        make (-11, 0.48f, 0.78f, 0.25f, 0.15f, 0, 0, 0, 0, 1.0f, 0) });

    addPreset ({ "Ghost", "👻", "Ethereal whisper with cavernous reverb.",
        make (-3, 1.25f, 0.88f, 0, 0.85f, 0.15f, 0.75f, 4, 0, 1.0f, 0.4f) });

    addPreset ({ "Phone", "📞", "Telephone band-pass — distant and tinny.",
        make (0, 0.8f, 1.0f, 0.25f, 0, 0.1f, 0, 1, 0, 1.0f, 0) });

    addPreset ({ "Zombie", "🧟", "Undead groan with grit and low formants.",
        make (-7, 0.42f, 0.72f, 0.65f, 0.35f, 0.35f, 0, 0, 0, 1.0f, 0.3f) });

    addPreset ({ "AI Gender Shift", "🧬", "Neural-style formant morph — masculine ↔ feminine.",
        make (-2, 1.35f, 1.0f, 0.05f, 0.12f, 0.15f, 0.2f, 0, 0.2f, 1.0f, 0.85f) });

    addPreset ({ "AI Female", "👩", "Bright feminine AI voice — higher pitch and formants.",
        make (6.0f, 1.48f, 1.0f, 0.04f, 0.1f, 0.08f, 0.22f, 0, 0.12f, 1.0f, 0.92f) });

    addPreset ({ "AI Male", "👨", "Deeper masculine AI voice — lower pitch and formants.",
        make (-6.0f, 0.58f, 0.95f, 0.06f, 0.12f, 0.05f, 0.1f, 0, 0.1f, 1.0f, 0.88f) });

    addPreset ({ "AI Age Morph", "⏳", "Spectral age transformation — younger or older timbre.",
        make (-5, 0.62f, 0.88f, 0.1f, 0.1f, 0.05f, 0.15f, 0, 0.15f, 1.0f, 0.9f) });

    addPreset ({ "T-Pain", "🎵", "Hard auto-tune with modern trap sheen.",
        make (0, 0.85f, 1.0f, 0.15f, 0.25f, 0, 0.4f, 0, 0.98f, 1.0f, 0) });

    addPreset ({ "Megaphone", "📢", "Bullhorn grit with radio filter.",
        make (2, 0.75f, 1.0f, 0.8f, 0.08f, 0.35f, 0, 2, 0, 1.0f, 0) });

    addPreset ({ "AI Clone", "🤖", "Deep spectral clone — uncanny voice double.",
        make (0, 1.05f, 1.0f, 0.08f, 0.18f, 0.25f, 0.35f, 0, 0.35f, 1.0f, 0.95f) });

    addPreset ({ "Alien", "👽", "Otherworldly ring-mod with space chorus.",
        make (5, 0.4f, 1.0f, 0.35f, 0.45f, 0.85f, 0.7f, 3, 0, 1.0f, 0.55f) });

    addPreset ({ "AI Narrator", "🎬", "Documentary AI voice — warm, authoritative, polished.",
        make (-4, 0.78f, 0.9f, 0.12f, 0.22f, 0, 0.12f, 0, 0.1f, 1.0f, 0.7f) });

    addPreset ({ "Neural Vocoder", "🔮", "Free integrated AI-style vocoder morph (on-device DSP).",
        make (0, 0.7f, 1.0f, 0.2f, 0.15f, 0.65f, 0.3f, 2, 0, 1.0f, 1.0f) });

    addPreset ({ "Underwater", "🌊", "Submerged muffled ambience.",
        make (-2, 0.85f, 0.82f, 0, 0.65f, 0, 0.55f, 4, 0, 1.0f, 0.2f) });

    addPreset ({ "Random Beast", "🎲", "Chaos mode — extreme transformation.",
        make (6, 0.5f, 1.35f, 0.85f, 0.65f, 0.95f, 0.8f, 3, 0.4f, 1.0f, 0.8f) });
}

const VoicePreset& VocalChangePresetManager::getPreset (int index) const
{
    index = juce::jlimit (0, getNumPresets() - 1, index);
    return presets[static_cast<size_t> (index)];
}

void VocalChangePresetManager::addPreset (VoicePreset preset)
{
    presets.push_back (std::move (preset));
}

} // namespace vocalchangeplus
