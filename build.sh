#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

CMAKE="${CMAKE:-cmake}"
if ! command -v cmake &>/dev/null; then
    LOCAL_CMAKE="$SCRIPT_DIR/.tools/cmake-3.31.6-macos-universal/CMake.app/Contents/bin/cmake"
    if [[ -x "$LOCAL_CMAKE" ]]; then
        CMAKE="$LOCAL_CMAKE"
    elif [[ -x "/tmp/cmake-3.31.6-macos-universal/CMake.app/Contents/bin/cmake" ]]; then
        CMAKE="/tmp/cmake-3.31.6-macos-universal/CMake.app/Contents/bin/cmake"
    elif [[ -x "/Applications/CMake.app/Contents/bin/cmake" ]]; then
        CMAKE="/Applications/CMake.app/Contents/bin/cmake"
    else
        echo "CMake not found. Install CMake or set CMAKE to its path."
        echo "See ~/Desktop/VocalPlus-CMake-Fix.pdf or run: ./scripts/install-cmake.sh"
        exit 1
    fi
fi

detect_team_id() {
    security find-certificate -a -c "Apple Development" -p 2>/dev/null \
        | openssl x509 -noout -subject 2>/dev/null \
        | sed -n 's/.*OU=\([^/]*\).*/\1/p' \
        | head -1
}

TEAM_ID="${VOCALPLUS_DEVELOPMENT_TEAM:-$(detect_team_id)}"
CMAKE_SIGN_ARGS=()

if [[ -n "$TEAM_ID" ]]; then
    CMAKE_SIGN_ARGS=(-DVOCALPLUS_DEVELOPMENT_TEAM="$TEAM_ID")
    echo "Using Apple Development team: $TEAM_ID"
else
    echo "Warning: No Apple Development certificate found. Build will use ad-hoc signing."
    echo "Sign in to Xcode with your Apple ID and create an Apple Development certificate."
fi

ACTION="${1:-xcode}"

case "$ACTION" in
    xcode)
        "$CMAKE" -B Build -G Xcode \
            -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
            -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
            "${CMAKE_SIGN_ARGS[@]}"
        echo ""
        echo "Xcode project generated: $SCRIPT_DIR/Build/VocalPlus.xcodeproj"
        echo "Open with: open Build/VocalPlus.xcodeproj"
        ;;
    build)
        "$CMAKE" --build Build --config Release \
            --target VocalPlus_AU VocalPlus_VST3 VocalPlus_Standalone \
                      EQPlus_AU EQPlus_VST3 EQPlus_Standalone \
                      TunePlus_AU TunePlus_VST3 TunePlus_Standalone \
                      VoxPlus_AU VoxPlus_VST3 VoxPlus_Standalone \
                      VocalChangePlus_AU VocalChangePlus_VST3 VocalChangePlus_Standalone \
                      VocalAIPlus_AU VocalAIPlus_VST3 VocalAIPlus_Standalone
        chmod +x "$SCRIPT_DIR/scripts/"*.sh "$SCRIPT_DIR/scripts/lib/"*.sh 2>/dev/null || true
        "$SCRIPT_DIR/scripts/fix-vst3-moduleinfo.sh" Release || true
        "$SCRIPT_DIR/scripts/refresh-logic.sh" Release
        ;;
    logic)
        chmod +x "$SCRIPT_DIR/scripts/"*.sh "$SCRIPT_DIR/scripts/lib/"*.sh 2>/dev/null || true
        chmod +x "$SCRIPT_DIR/scripts/vocalair/"*.sh 2>/dev/null || true
        "$SCRIPT_DIR/scripts/refresh-logic.sh" "${2:-Release}"
        ;;
    vocalair)
        chmod +x "$SCRIPT_DIR/scripts/vocalair/"*.sh 2>/dev/null || true
        "$SCRIPT_DIR/scripts/vocalair/build-vocalair.sh" "${2:-build}"
        ;;
    fix-vst3)
        chmod +x "$SCRIPT_DIR/scripts/fix-vst3-moduleinfo.sh"
        "$SCRIPT_DIR/scripts/fix-vst3-moduleinfo.sh" "${2:-Release}"
        ;;
    sign)
        "$CMAKE" --build Build --config Release \
            --target VocalPlus_AU VocalPlus_VST3 \
                      EQPlus_AU EQPlus_VST3 \
                      TunePlus_AU TunePlus_VST3 \
                      VoxPlus_AU VoxPlus_VST3 \
                      VocalChangePlus_AU VocalChangePlus_VST3 \
                      VocalAIPlus_AU VocalAIPlus_VST3
        chmod +x "$SCRIPT_DIR/scripts/"*.sh "$SCRIPT_DIR/scripts/lib/"*.sh 2>/dev/null || true
        "$SCRIPT_DIR/scripts/fix-vst3-moduleinfo.sh" Release || true
        "$SCRIPT_DIR/scripts/sign-local.sh" "${2:-Release}"
        ;;
    package)
        "$CMAKE" --build Build --config Release \
            --target VocalPlus_AU VocalPlus_VST3 \
                      EQPlus_AU EQPlus_VST3 \
                      TunePlus_AU TunePlus_VST3 \
                      VoxPlus_AU VoxPlus_VST3 \
                      VocalChangePlus_AU VocalChangePlus_VST3 \
                      VocalAIPlus_AU VocalAIPlus_VST3
        chmod +x "$SCRIPT_DIR/scripts/"*.sh "$SCRIPT_DIR/scripts/lib/"*.sh 2>/dev/null || true
        "$SCRIPT_DIR/scripts/fix-vst3-moduleinfo.sh" Release || true
        "$SCRIPT_DIR/scripts/package-installer.sh" "${2:-Release}"
        ;;
    clean)
        rm -rf Build Build-Ninja
        ;;
    *)
        echo "Usage: ./build.sh [xcode|build|logic|sign|package|fix-vst3|vocalair|clean]"
        exit 1
        ;;
esac
