#!/bin/bash
# Install CMake into VocalPlus/.tools for local builds (no Homebrew required).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
TOOLS_DIR="$PROJECT_DIR/.tools"
CMAKE_DIR="$TOOLS_DIR/cmake-3.31.6-macos-universal"
CMAKE_BIN="$CMAKE_DIR/CMake.app/Contents/bin/cmake"
VERSION="3.31.6"
URL="https://github.com/Kitware/CMake/releases/download/v${VERSION}/cmake-${VERSION}-macos-universal.tar.gz"

if [[ -x "$CMAKE_BIN" ]]; then
    echo "CMake already installed: $("$CMAKE_BIN" --version | head -1)"
    exit 0
fi

mkdir -p "$TOOLS_DIR"
TMP="$TOOLS_DIR/cmake-macos-universal.tar.gz"

echo ">>> Downloading CMake ${VERSION}..."
curl -fsSL -o "$TMP" "$URL"
tar xzf "$TMP" -C "$TOOLS_DIR"
rm -f "$TMP"

echo ">>> Installed: $("$CMAKE_BIN" --version | head -1)"
echo ">>> You can now run: cd ~/Documents/VocalPlus && ./build.sh xcode"
