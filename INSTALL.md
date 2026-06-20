# Vocal+ Installation for Logic Pro

Vocal+ family: AUv2 plugins plus **VocalAir+** (AUv3 vocal chain).

## Requirements

- macOS 11.0 or later (VocalAir+ requires macOS 12+)
- Logic Pro (or any AU host)
- Xcode 15+ with **Apple Development** certificate (free Apple ID)

## Logic Pro install (recommended)

Build and sign on your Mac so Logic sees a valid **TeamIdentifier** (not ad-hoc):

```bash
cd ~/Documents/VocalPlus
chmod +x build.sh scripts/*.sh scripts/lib/*.sh scripts/vocalair/*.sh
./build.sh build               # build + Apple Development sign + install (AUv2 + VocalAir+)
```

Or re-sign / reinstall without a full rebuild:

```bash
./build.sh logic
```

AUv2 plugins are installed to:

- `~/Library/Audio/Plug-Ins/Components/VocalPlus.component` → **Vocal+**
- `~/Library/Audio/Plug-Ins/Components/TunePlus.component` → **Tune+**
- `~/Library/Audio/Plug-Ins/Components/VoxPlus.component` → **VOX**
- `~/Library/Audio/Plug-Ins/Components/VocalChangePlus.component` → **VocalChange+**
- `~/Library/Audio/Plug-Ins/Components/EQPlus.component` → **EQ+**

**VocalAir+** (AUv3) is installed as a container app:

- `/Applications/VocalAirPlus.app` → **Audio Units → Vocal+ → VocalAir+**

Build VocalAir+ alone:

```bash
./build.sh vocalair build
./build.sh vocalair install
```

## Logic Pro setup

1. Quit Logic completely (Cmd+Q).
2. Open Logic → **Settings → Plug-In Manager**.
3. **Reset & Rescan Selection** (or full rescan if plugins are missing).
4. Enable plugins under manufacturer **Vocal+**.
5. Insert **Audio Units → Vocal+ → Vocal+** on a vocal track (or **VocalAir+** for the AUv3 chain).

## Verify signing

```bash
codesign -dv ~/Library/Audio/Plug-Ins/Components/VocalPlus.component 2>&1 | grep -E 'Authority|TeamIdentifier|Signature'
auval -v aufx VcP1 VcPl
pluginkit -m -i com.vocalplus.vocalairplus.VocalAirPlusExtension
```

You should **not** see `Signature=adhoc` or `TeamIdentifier=not set`.

## Troubleshooting

| Issue | Fix |
|-------|-----|
| **Incompatible / unsigned in Logic** | Run `./build.sh logic` — JUCE auto-copy installs ad-hoc builds; we disable that and sign locally. |
| **VocalAir+ incompatible** | Remove old install: `pluginkit -r com.iairu.VocalAirPlus.VocalAirPlusExtension`; then `./build.sh vocalair install`. Open `/Applications/VocalAirPlus.app` once. |
| **Plugin damaged / quarantined** | `xattr -cr ~/Library/Audio/Plug-Ins/Components/VocalPlus.component` and `xattr -cr /Applications/VocalAirPlus.app` |
| **Not showing in Logic** | `killall -9 AudioComponentRegistrar` then rescan in Plug-In Manager |
| **Other plugins disappeared** | `./scripts/recover-logic-plugins.sh` — do **not** wipe system AU caches unless needed |
| **No Apple Development cert** | Xcode → Settings → Accounts → Manage Certificates → **+ Apple Development** |

## GitHub releases

CI builds on tags (`v*`) compile the project for verification. **Logic Pro requires a local build** with your Apple Development certificate — pre-built ad-hoc binaries from CI will not validate in Logic.
