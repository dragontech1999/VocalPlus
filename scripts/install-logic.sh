#!/bin/bash
# Full clean install for Logic Pro — Vocal+ AU only.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

echo ">>> Removing old Vocal+ installs..."
rm -rf "$HOME/Library/Audio/Plug-Ins/Components/Vocal+.component"
rm -rf "$HOME/Library/Audio/Plug-Ins/Components/VocalPlus.component"
rm -rf "$HOME/Library/Audio/Plug-Ins/VST3/Vocal+.vst3"
rm -rf "$HOME/Library/Audio/Plug-Ins/VST3/VocalPlus.vst3"

echo ">>> Clearing Audio Unit caches..."
killall -9 AudioComponentRegistrar 2>/dev/null || true
rm -rf "$HOME/Library/Caches/AudioUnitCache"
mkdir -p "$HOME/Library/Caches/AudioUnitCache"

CMAKE="${CMAKE:-cmake}"
if ! command -v cmake &>/dev/null; then
    if [[ -x "/tmp/cmake-3.31.6-macos-universal/CMake.app/Contents/bin/cmake" ]]; then
        CMAKE="/tmp/cmake-3.31.6-macos-universal/CMake.app/Contents/bin/cmake"
    else
        echo "CMake not found."
        exit 1
    fi
fi

echo ">>> Building Vocal+ AU (Release)..."
"$PROJECT_DIR/build.sh" xcode
"$CMAKE" --build "$PROJECT_DIR/Build" --config Release --target VocalPlus_AU

echo ">>> Signing and installing for Logic..."
chmod +x "$PROJECT_DIR/scripts/"*.sh
"$PROJECT_DIR/scripts/refresh-logic.sh" Release

AU="$HOME/Library/Audio/Plug-Ins/Components/VocalPlus.component"
echo ">>> Installed: $AU"
codesign -dv "$AU" 2>&1 | rg "Authority|TeamIdentifier|Signature=adhoc" || true

echo ""
echo "Open Logic → Settings → Plug-In Manager → Reset & Rescan Selection"
echo "Enable Vocal+ under the Vocal+ folder."
