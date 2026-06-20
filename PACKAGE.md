# Vocal+ Full Package

**Version 1.0.3** · Created by **Neeberman**

The complete **Vocal+** plugin suite for Logic Pro and any AU/VST3 host. All plugins share the Vocal+ manufacturer identity, neon-green professional UI, edge-resize editor shell, and Apple Development signing workflow for Logic Pro compatibility.

Repository: [github.com/dragontech1999/VocalPlus](https://github.com/dragontech1999/VocalPlus)

---

## Package contents

| Plugin | Type | Logic name | Purpose |
|--------|------|------------|---------|
| **Vocal+** | AU / VST3 / Standalone | Vocal+ | Auto-tune, harmony voices, stem export |
| **Tune+** | AU / VST3 / Standalone | Tune+ | Auto-tune, harmony, vocal compressor |
| **EQ+** | AU / VST3 / Standalone | EQ+ | Dynamic EQ, spectrum analyzer, smart assistant |
| **VOX+** | AU / VST3 / Standalone | VOX+ | Vocal multi-FX channel strip (voice, dynamics, character, delay, reverb, chop) |
| **VocalChange+** | AU / VST3 / Standalone | VocalChange+ | Real-time voice transformation (pitch, formant, robot, filters) |
| **VocalAI+** | AU / VST3 / Standalone | VocalAI+ | AI genre-aware vocal mastering (analysis engine + 7-stage chain) |
| **VocalAir+** | AUv3 app extension | VocalAir+ | AI vocal chain (installed as `/Applications/VocalAirPlus.app`) |

All AUv2 plugins use manufacturer code **VcPl** and appear under **Audio Units → Vocal+** in Logic Pro.

---

## Quick install (macOS)

### Option A — Installer package (recommended)

1. Download **`VocalPlus-Plugins-1.0.3.pkg`** from the [GitHub Releases](https://github.com/dragontech1999/VocalPlus/releases) page.
2. Double-click the `.pkg` and follow the prompts.
3. Plugins install to:
   - `~/Library/Audio/Plug-Ins/Components/` (AU)
   - `~/Library/Audio/Plug-Ins/VST3/` (VST3)
4. Quit Logic Pro completely (**Cmd+Q**), reopen, then **Settings → Plug-In Manager → Reset & Rescan Selection**.

### Option B — Build from source

Requires Xcode 15+, Apple Development certificate, and macOS 11+:

```bash
git clone https://github.com/dragontech1999/VocalPlus.git
cd VocalPlus
chmod +x build.sh scripts/*.sh scripts/lib/*.sh scripts/vocalair/*.sh
./build.sh xcode
./build.sh package    # builds all plugins + Desktop installer
./build.sh logic      # sign + install for Logic Pro
```

See **[INSTALL.md](INSTALL.md)** for signing details and troubleshooting.

---

## Plugin guide

### Vocal+

Core vocal production plugin: YIN pitch detection, scale-aware auto-tune, three independent harmony singers, eight genre presets, mono blend or multi-stem WAV export.

**Best for:** Lead vocals, stacked harmonies, quick pop/R&B/gospel/EDM vocal production.

### Tune+

Focused auto-tune workstation with integrated vocal compressor, harmony engine, and genre presets tuned for modern vocal production.

**Best for:** Dedicated tuning sessions, vocal compression alongside pitch correction.

### EQ+

Dynamic parametric EQ with real-time spectrum display, smart EQ assistant, and reference matching tools.

**Best for:** Surgical vocal EQ, tonal balance, mix-ready vocal shaping.

### VOX+

Full vocal channel strip inspired by modern vocal FX workflows: voice (pitch, formant, unison, correction), six macro FX modules (dynamics, character, filter, delay, reverb, chop), global auto-level and glue.

**Best for:** All-in-one vocal chain in a single insert, genre presets from pop to podcast.

### VocalChange+

Real-time voice morphing: pitch/formant shift, robot/chorus/distortion, phone/radio/space filters, AI morph control, 20+ character presets.

**Best for:** Character voices, creative FX, narration, content creation.

### VocalAI+

Ports the **Neeberman** VocalForgeAI mastering workflow into a real-time plugin:

- **Analysis engine** — LUFS, peak, crest, spectral centroid, sibilance/mud/presence metrics
- **AI Master button** — genre-aware chain configuration (Pop, Hip-Hop, R&B, Rock, Country, EDM, Jazz, Acoustic, Trap, Soul)
- **7-stage chain** — noise reduction, 4-band EQ, compressor, de-esser, exciter, reverb, limiter
- **12+ presets** — genre masters plus Podcast Voice and Lo-Fi Warmth

**Best for:** Release-ready vocal masters, genre-targeted loudness, intelligent starting points.

### VocalAir+

AUv3 vocal chain app extension. Built separately via `./build.sh vocalair`. Installed to `/Applications/VocalAirPlus.app`.

**Best for:** Logic-native AUv3 vocal AI chain alongside the Vocal+ AUv2 suite.

---

## Build commands

| Command | Action |
|---------|--------|
| `./build.sh xcode` | Generate Xcode project (auto-detects Apple Development team) |
| `./build.sh build` | Release-build all plugins, sign, install locally |
| `./build.sh logic` | Re-sign + install for Logic (no rebuild) |
| `./build.sh sign` | Build AU+VST3, sign, install |
| `./build.sh package` | Build + create `~/Desktop/VocalPlus-Plugins-1.0.3.pkg` |
| `./build.sh vocalair` | Build/install VocalAir+ AUv3 |
| `./build.sh clean` | Remove build directories |

---

## Signing & Logic Pro

Logic Pro requires plugins signed with an **Apple Development** certificate (not ad-hoc). The build system:

1. Sets `COPY_PLUGIN_AFTER_BUILD FALSE` to prevent JUCE from installing unsigned builds
2. Signs with your keychain **Apple Development** identity via `scripts/sign-local.sh`
3. Validates with `auval` before packaging

Verify after install:

```bash
codesign -dv ~/Library/Audio/Plug-Ins/Components/VocalAIPlus.component 2>&1 | grep TeamIdentifier
auval -v aufx VaI1 VcPl   # VocalAI+
auval -v aufx VcP1 VcPl   # Vocal+
auval -v aufx VxP1 VcPl   # VOX+
```

---

## AU validation codes

| Plugin | TYPE | SUBT | MANU |
|--------|------|------|------|
| Vocal+ | aufx | VcP1 | VcPl |
| EQ+ | aufx | EqP1 | VcPl |
| Tune+ | aufx | TnP1 | VcPl |
| VOX+ | aufx | VxP1 | VcPl |
| VocalChange+ | aufx | VcC1 | VcPl |
| VocalAI+ | aufx | VaI1 | VcPl |

---

## Project structure

```
VocalPlus/
├── PACKAGE.md              # This file — full package reference
├── README.md               # Vocal+ core plugin overview
├── INSTALL.md              # Logic Pro installation guide
├── CMakeLists.txt          # All JUCE plugin targets
├── build.sh                # Build orchestrator
├── Assets/                 # Plugin icons
├── Source/
│   ├── PluginProcessor.*   # Vocal+ (core)
│   ├── EQPlus/             # EQ+ plugin
│   ├── TunePlus/           # Tune+ plugin
│   ├── VoxPlus/            # VOX+ plugin
│   ├── VocalChangePlus/    # VocalChange+ plugin
│   ├── VocalAIPlus/        # VocalAI+ plugin
│   ├── UI/                 # Shared VocalPlusEditorShell
│   └── DSP/                # Shared DSP modules
├── cmake/                  # Code-signing entitlements
├── scripts/                # Sign, package, Logic install scripts
└── .github/workflows/      # CI release on version tags
```

---

## Troubleshooting

| Issue | Fix |
|-------|-----|
| Plugin shows **Incompatible** in Logic | Run `./build.sh logic` on your Mac with Apple Development cert |
| Plugin not appearing | Quit Logic, rescan in Plug-In Manager |
| **Signature=adhoc** | Rebuild with `./build.sh package` — never copy Build/ artefacts manually |
| Other plugins missing | `./scripts/recover-logic-plugins.sh` |
| VocalAir+ issues | `./build.sh vocalair install` |

---

## Credits

**Created by Neeberman**

- Product family: **Vocal+**
- Publisher repository: [dragontech1999/VocalPlus](https://github.com/dragontech1999/VocalPlus)
- Built with [JUCE](https://juce.com) 8.0.6

Copyright © 2026 **Neeberman**. All rights reserved.
