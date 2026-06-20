#!/bin/bash
# Signs Vocal+ family plugins with Apple Development and installs for Logic Pro.
# Must run AFTER xcode build — never rely on JUCE auto-copy (installs ad-hoc builds).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
CONFIG="${1:-Release}"

# shellcheck source=lib/logic-sign.sh
source "$SCRIPT_DIR/lib/logic-sign.sh"

require_apple_development

# Vocal+
VOCAL="$PROJECT_DIR/Build/VocalPlus_artefacts/$CONFIG"
sign_au_for_logic "$VOCAL/AU/VocalPlus.component" "VocalPlus"
sign_bundle_deep_for_logic "$VOCAL/VST3/VocalPlus.vst3" "$PROJECT_DIR/cmake/VocalPlus_VST3.entitlements"
sign_bundle_deep_for_logic "$VOCAL/Standalone/VocalPlus.app" "$PROJECT_DIR/cmake/VocalPlus_Standalone.entitlements"
install_plugin_bundle "$VOCAL/AU/VocalPlus.component" "$HOME/Library/Audio/Plug-Ins/Components/VocalPlus.component" "VocalPlus"
install_plugin_bundle "$VOCAL/VST3/VocalPlus.vst3" "$HOME/Library/Audio/Plug-Ins/VST3/VocalPlus.vst3" "VocalPlus"
clear_plugin_quarantine "$HOME/Library/Audio/Plug-Ins/Components/VocalPlus.component"
clear_plugin_quarantine "$HOME/Library/Audio/Plug-Ins/VST3/VocalPlus.vst3"

# EQ+
EQ="$PROJECT_DIR/Build/EQPlus_artefacts/$CONFIG"
sign_au_for_logic "$EQ/AU/EQPlus.component" "EQPlus"
sign_bundle_deep_for_logic "$EQ/VST3/EQPlus.vst3" "$PROJECT_DIR/cmake/EQPlus_VST3.entitlements"
sign_bundle_deep_for_logic "$EQ/Standalone/EQPlus.app" "$PROJECT_DIR/cmake/EQPlus_Standalone.entitlements"
install_plugin_bundle "$EQ/AU/EQPlus.component" "$HOME/Library/Audio/Plug-Ins/Components/EQPlus.component" "EQPlus"
install_plugin_bundle "$EQ/VST3/EQPlus.vst3" "$HOME/Library/Audio/Plug-Ins/VST3/EQPlus.vst3" "EQPlus"
clear_plugin_quarantine "$HOME/Library/Audio/Plug-Ins/Components/EQPlus.component"

# Tune+
TUNE="$PROJECT_DIR/Build/TunePlus_artefacts/$CONFIG"
sign_au_for_logic "$TUNE/AU/TunePlus.component" "TunePlus"
sign_bundle_deep_for_logic "$TUNE/VST3/TunePlus.vst3" "$PROJECT_DIR/cmake/TunePlus_VST3.entitlements"
sign_bundle_deep_for_logic "$TUNE/Standalone/TunePlus.app" "$PROJECT_DIR/cmake/TunePlus_Standalone.entitlements"
install_plugin_bundle "$TUNE/AU/TunePlus.component" "$HOME/Library/Audio/Plug-Ins/Components/TunePlus.component" "TunePlus"
install_plugin_bundle "$TUNE/VST3/TunePlus.vst3" "$HOME/Library/Audio/Plug-Ins/VST3/TunePlus.vst3" "TunePlus"
clear_plugin_quarantine "$HOME/Library/Audio/Plug-Ins/Components/TunePlus.component"

# VOX
VOX="$PROJECT_DIR/Build/VoxPlus_artefacts/$CONFIG"
sign_au_for_logic "$VOX/AU/VoxPlus.component" "VoxPlus"
sign_bundle_deep_for_logic "$VOX/VST3/VoxPlus.vst3" "$PROJECT_DIR/cmake/VoxPlus_VST3.entitlements"
sign_bundle_deep_for_logic "$VOX/Standalone/VoxPlus.app" "$PROJECT_DIR/cmake/VoxPlus_Standalone.entitlements"
install_plugin_bundle "$VOX/AU/VoxPlus.component" "$HOME/Library/Audio/Plug-Ins/Components/VoxPlus.component" "VoxPlus"
install_plugin_bundle "$VOX/VST3/VoxPlus.vst3" "$HOME/Library/Audio/Plug-Ins/VST3/VoxPlus.vst3" "VoxPlus"
clear_plugin_quarantine "$HOME/Library/Audio/Plug-Ins/Components/VoxPlus.component"

# VocalChange+
VC="$PROJECT_DIR/Build/VocalChangePlus_artefacts/$CONFIG"
sign_au_for_logic "$VC/AU/VocalChangePlus.component" "VocalChangePlus"
sign_bundle_deep_for_logic "$VC/VST3/VocalChangePlus.vst3" "$PROJECT_DIR/cmake/VocalChangePlus_VST3.entitlements"
sign_bundle_deep_for_logic "$VC/Standalone/VocalChangePlus.app" "$PROJECT_DIR/cmake/VocalChangePlus_Standalone.entitlements"
install_plugin_bundle "$VC/AU/VocalChangePlus.component" "$HOME/Library/Audio/Plug-Ins/Components/VocalChangePlus.component" "VocalChangePlus"
install_plugin_bundle "$VC/VST3/VocalChangePlus.vst3" "$HOME/Library/Audio/Plug-Ins/VST3/VocalChangePlus.vst3" "VocalChangePlus"
clear_plugin_quarantine "$HOME/Library/Audio/Plug-Ins/Components/VocalChangePlus.component"

# VocalAI+
VAI="$PROJECT_DIR/Build/VocalAIPlus_artefacts/$CONFIG"
sign_au_for_logic "$VAI/AU/VocalAIPlus.component" "VocalAIPlus"
sign_bundle_deep_for_logic "$VAI/VST3/VocalAIPlus.vst3" "$PROJECT_DIR/cmake/VocalAIPlus_VST3.entitlements"
sign_bundle_deep_for_logic "$VAI/Standalone/VocalAIPlus.app" "$PROJECT_DIR/cmake/VocalAIPlus_Standalone.entitlements"
install_plugin_bundle "$VAI/AU/VocalAIPlus.component" "$HOME/Library/Audio/Plug-Ins/Components/VocalAIPlus.component" "VocalAIPlus"
install_plugin_bundle "$VAI/VST3/VocalAIPlus.vst3" "$HOME/Library/Audio/Plug-Ins/VST3/VocalAIPlus.vst3" "VocalAIPlus"
clear_plugin_quarantine "$HOME/Library/Audio/Plug-Ins/Components/VocalAIPlus.component"

echo ""
echo "All plugins signed with Apple Development and installed."
