#!/bin/bash
# Regenerates moduleinfo.json for VocalPlus and EQPlus VST3 bundles.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
CONFIG="${1:-Release}"

HELPER="$PROJECT_DIR/Build/$CONFIG/juce_vst3_helper"
VERSION="${VOCALPLUS_VERSION:-1.0.0}"

if [[ ! -x "$HELPER" ]]; then
    echo "juce_vst3_helper not found at: $HELPER"
    echo "Build the juce_vst3_helper target first (or run ./build.sh build)."
    exit 1
fi

fix_vst3() {
    local vst3_bundle="$1"
    local output="$vst3_bundle/Contents/Resources/moduleinfo.json"

    if [[ ! -d "$vst3_bundle" ]]; then
        echo "Skip (not found): $vst3_bundle"
        return 0
    fi

    mkdir -p "$(dirname "$output")"

    if ! codesign --verify --deep "$vst3_bundle" &>/dev/null; then
        codesign -f -s - "$vst3_bundle"
    fi

    echo "Generating moduleinfo.json for $vst3_bundle"
    "$HELPER" \
        -create \
        -version "$VERSION" \
        -path "$vst3_bundle" \
        -output "$output"

    echo "Created: $output"
}

fix_vst3 "$PROJECT_DIR/Build/VocalPlus_artefacts/$CONFIG/VST3/VocalPlus.vst3"
fix_vst3 "$PROJECT_DIR/Build/EQPlus_artefacts/$CONFIG/VST3/EQPlus.vst3"
fix_vst3 "$PROJECT_DIR/Build/TunePlus_artefacts/$CONFIG/VST3/TunePlus.vst3"
fix_vst3 "$PROJECT_DIR/Build/VoxPlus_artefacts/$CONFIG/VST3/VoxPlus.vst3"
fix_vst3 "$PROJECT_DIR/Build/VocalChangePlus_artefacts/$CONFIG/VST3/VocalChangePlus.vst3"
fix_vst3 "$PROJECT_DIR/Build/VocalAIPlus_artefacts/$CONFIG/VST3/VocalAIPlus.vst3"
