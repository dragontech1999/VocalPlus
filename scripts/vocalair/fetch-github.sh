#!/bin/bash
# Fetch LogicAIV upstream (vocal chain AUv3) — source only, no local JUCE builds.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
SRC_DIR="$PROJECT_DIR/VocalAirPlus/LogicAIV"
CACHE_DIR="$PROJECT_DIR/.upstream/logic-plugins"
REPO_URL="${LOGIC_PLUGINS_REPO:-https://github.com/iairu/logic-plugins.git}"
REF="${LOGIC_PLUGINS_REF:-main}"

echo ">>> Fetching upstream LogicAIV ($REF)..."

if [[ -d "$CACHE_DIR/.git" ]]; then
    git -C "$CACHE_DIR" fetch --depth 1 origin "$REF"
    git -C "$CACHE_DIR" checkout -f FETCH_HEAD
else
    rm -rf "$CACHE_DIR"
    mkdir -p "$(dirname "$CACHE_DIR")"
    git clone --depth 1 --branch "$REF" "$REPO_URL" "$CACHE_DIR"
fi

COMMIT=$(git -C "$CACHE_DIR" rev-parse --short HEAD)
echo ">>> Upstream commit: $COMMIT"

rm -rf "$SRC_DIR"
mkdir -p "$(dirname "$SRC_DIR")"
ditto "$CACHE_DIR/LogicAIV" "$SRC_DIR"

echo "$COMMIT" > "$PROJECT_DIR/VocalAirPlus/.upstream-commit"
echo ">>> LogicAIV ready at $SRC_DIR"
