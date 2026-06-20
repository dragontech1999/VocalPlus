# Vocal+

**Created by Neeberman**

**Vocal+** is a macOS audio plugin suite for Logic Pro and any AU/VST3 host. It includes AUv2 effects (Vocal+, Tune+, EQ+, VOX+, VocalChange+, **VocalAI+**) and **VocalAir+**, an AUv3 vocal chain app extension in the same family.

For the complete suite reference — all plugins, install options, build commands, and AU codes — see **[PACKAGE.md](PACKAGE.md)**.

See **[INSTALL.md](INSTALL.md)** for Logic Pro setup (Apple Development signing required).

## Features (Vocal+ core plugin)

- **Auto-Tune Engine** — YIN pitch detection, scale-aware quantization (major, minor, pentatonic, modes), adjustable retune speed, tolerance, and formant preservation
- **3 Harmony Singers** — Independent interval, level, pan, formant shift, delay, and vibrato per voice
- **8 Professional Presets** — Pop Lead, Hard Tune FX, Gospel Stack, Indie Natural, EDM Chops, Choir Cathedral, Country Honky, Lo-Fi Bedroom
- **Blend or Stems** — Mix corrected vocal + harmonies to a single stereo track in Logic, or record and export individual WAV stems
- **Formats** — AU (Logic Pro), VST3, and **Standalone** app for side-chaining without a DAW

## Requirements

- macOS 11.0 or later (Apple Silicon + Intel universal binary)
- Xcode 15+ (for building)
- CMake 3.22+ (included via `./build.sh` if not installed)

## Build

```bash
cd VocalPlus
chmod +x build.sh scripts/sign-local.sh scripts/lib/logic-sign.sh
./build.sh xcode    # generate Xcode project (auto-detects your Apple Development team)
./build.sh build    # build + sign + install plugins locally
./build.sh package  # build full suite + Desktop installer
./build.sh logic    # re-sign + install for Logic (no rebuild)
./build.sh vocalair # build + install VocalAir+ AUv3 only
```

Signing uses your **Apple Development** certificate from Xcode (free Apple ID). This replaces ad-hoc signing so macOS Gatekeeper keeps trusting Vocal+ on your Mac. Certificates last about **one year** — renew in **Xcode → Settings → Accounts → Manage Certificates** when expired.

To re-sign without rebuilding:

```bash
./build.sh sign
```

In Xcode, build the **VocalPlus_AU**, **VocalPlus_VST3**, or **VocalPlus_Standalone** scheme.

Plugins are copied automatically to:

- AU: `~/Library/Audio/Plug-Ins/Components/VocalPlus.component` (shows as **Vocal+** in Logic)
- VST3: `~/Library/Audio/Plug-Ins/VST3/VocalPlus.vst3`

If the VST3 build fails with a `moduleinfotool` usage error, run:

```bash
./build.sh fix-vst3
./build.sh sign
```

## Logic Pro Setup

1. Build and install the AU component.
2. Rescan plugins in Logic: **Logic Pro → Settings → Plug-In Manager → Reset & Rescan Selection**.
3. Insert **Vocal+** on a vocal track (Audio FX slot).
4. Choose a preset, set key/scale, enable harmony singers as needed.
5. For a blended result, leave **Mono Blend** on — output is a single mixed vocal.

### Side-chain / Standalone

Launch **Vocal+** from Applications or the Standalone build target. Route your mic or track into the standalone input via your audio interface or BlackHole/Loopback virtual driver, then record the output back into Logic.

## Stem Export

1. Click **Record Stems** and play the passage you want to capture.
2. Click **Export WAV Stems**.
3. Files are saved to `~/Music/VocalPlus Exports/Session_<timestamp>/`:
   - `01_Dry.wav`
   - `02_Corrected.wav`
   - `03_Harmony_1.wav` … `05_Harmony_3.wav`
   - `06_Blend.wav`

Import these into Logic or any DAW for further editing.

## Presets Overview

| Preset | Style | Description |
|--------|-------|-------------|
| Pop Lead | Pop | Fast retune, airy third-above double |
| Hard Tune FX | Hip-Hop/R&B | Robotic zero-tolerance tuning, wide doubles |
| Gospel Stack | Gospel/Soul | Lush thirds and fifths with vibrato |
| Indie Natural | Indie/Folk | Transparent correction, preserves vibrato |
| EDM Chops | Electronic | Octave stacks, detuned unison |
| Choir Cathedral | Cinematic | Formant-shifted ensemble |
| Country Honky | Country | Nashville-style tight thirds |
| Lo-Fi Bedroom | Lo-Fi | Soft, detuned whisper doubles |

## Project Structure

```
VocalPlus/
├── PACKAGE.md              # Full suite reference (all plugins)
├── CMakeLists.txt          # JUCE plugin definitions
├── build.sh                # Generate Xcode / build
├── Assets/                 # App icons
├── Source/
│   ├── PluginProcessor.*   # Vocal+ host integration
│   ├── PluginEditor.*      # Vocal+ UI
│   ├── EQPlus/ TunePlus/ VoxPlus/ VocalChangePlus/ VocalAIPlus/
│   ├── DSP/                # Shared pitch, harmony, export
│   └── UI/                 # Shared editor shell + look and feel
└── Build/                  # Generated Xcode project (after build.sh)
```

## License

Copyright © 2026 **Neeberman**. All rights reserved.

Repository: [github.com/dragontech1999/VocalPlus](https://github.com/dragontech1999/VocalPlus)
