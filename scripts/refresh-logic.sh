#!/bin/bash
# Clean install Vocal+ family (AUv2 + VocalAir+ AUv3) for Logic Pro with Apple Development signing.
# JUCE must NOT auto-copy (COPY_PLUGIN_AFTER_BUILD FALSE) — auto-copy installs ad-hoc builds.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
CONFIG="${1:-Release}"

# shellcheck source=lib/logic-sign.sh
source "$SCRIPT_DIR/lib/logic-sign.sh"

echo ">>> Removing old / duplicate Vocal+ family plugins..."
rm -rf "$HOME/Library/Audio/Plug-Ins/Components/Vocal+.component"
rm -rf "$HOME/Library/Audio/Plug-Ins/Components/VocalPlus.component"
rm -rf "$HOME/Library/Audio/Plug-Ins/Components/EQPlus.component"
rm -rf "$HOME/Library/Audio/Plug-Ins/Components/TunePlus.component"
rm -rf "$HOME/Library/Audio/Plug-Ins/Components/VoxPlus.component"
rm -rf "$HOME/Library/Audio/Plug-Ins/Components/VocalChangePlus.component"
rm -rf "$HOME/Library/Audio/Plug-Ins/Components/VocalAIPlus.component"
rm -rf "$HOME/Library/Audio/Plug-Ins/VST3/Vocal+.vst3"
rm -rf "$HOME/Library/Audio/Plug-Ins/VST3/VocalPlus.vst3"
rm -rf "$HOME/Library/Audio/Plug-Ins/VST3/EQPlus.vst3"
rm -rf "$HOME/Library/Audio/Plug-Ins/VST3/TunePlus.vst3"
rm -rf "$HOME/Library/Audio/Plug-Ins/VST3/VoxPlus.vst3"
rm -rf "$HOME/Library/Audio/Plug-Ins/VST3/VocalChangePlus.vst3"
rm -rf "$HOME/Library/Audio/Plug-Ins/VST3/VocalAIPlus.vst3"

echo ">>> Clearing Audio Unit registry..."
refresh_au_registry

echo ">>> Signing and installing from Release artefacts..."
chmod +x "$PROJECT_DIR/scripts/"*.sh "$PROJECT_DIR/scripts/lib/"*.sh 2>/dev/null || true
"$PROJECT_DIR/scripts/sign-local.sh" "$CONFIG"

verify_au_for_logic "$HOME/Library/Audio/Plug-Ins/Components/VocalPlus.component" "Vocal+"
verify_au_for_logic "$HOME/Library/Audio/Plug-Ins/Components/TunePlus.component" "Tune+"
verify_au_for_logic "$HOME/Library/Audio/Plug-Ins/Components/VoxPlus.component" "VOX"
verify_au_for_logic "$HOME/Library/Audio/Plug-Ins/Components/VocalChangePlus.component" "VocalChange+"
verify_au_for_logic "$HOME/Library/Audio/Plug-Ins/Components/VocalAIPlus.component" "VocalAI+"

echo ""
echo ">>> Refreshing Audio Unit registry..."
refresh_au_registry
sleep 1

echo ""
echo ">>> AU validation (TYPE SUBT MANU)..."
auval -v aufx VcP1 VcPl >/dev/null 2>&1 && echo "OK: Vocal+ auval PASS" || echo "WARN: Vocal+ auval had issues"
auval -v aufx TnP1 VcPl >/dev/null 2>&1 && echo "OK: Tune+ auval PASS" || echo "WARN: Tune+ auval had issues"
auval -v aufx VxP1 VcPl >/dev/null 2>&1 && echo "OK: VOX auval PASS" || echo "WARN: VOX auval had issues"
auval -v aufx VcC1 VcPl >/dev/null 2>&1 && echo "OK: VocalChange+ auval PASS" || echo "WARN: VocalChange+ auval had issues"
auval -v aufx VaI1 VcPl >/dev/null 2>&1 && echo "OK: VocalAI+ auval PASS" || echo "WARN: VocalAI+ auval had issues"

echo ""
echo ">>> Building and installing VocalAir+ (AUv3)..."
chmod +x "$PROJECT_DIR/scripts/vocalair/"*.sh 2>/dev/null || true
if "$PROJECT_DIR/scripts/vocalair/build-vocalair.sh" build; then
    "$PROJECT_DIR/scripts/vocalair/build-vocalair.sh" install
else
    echo "WARN: VocalAir+ build skipped or failed — AUv2 plugins are still installed."
fi

echo ""
echo ">>> Done. Quit Logic completely (Cmd+Q), reopen, then:"
echo "    Logic Pro → Settings → Plug-In Manager → Reset & Rescan Selection"
echo "    Look under manufacturer: Vocal+ (Vocal+, Tune+, VocalAir+, …)"
echo ""
echo "If macOS quarantined a download, run:"
echo "    xattr -cr ~/Library/Audio/Plug-Ins/Components/VocalPlus.component"
