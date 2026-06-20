#!/bin/bash
# Sign VocalAir+ AUv3 container app with Apple Development (Logic Pro).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"

# shellcheck source=../lib/logic-sign.sh
source "$SCRIPT_DIR/../lib/logic-sign.sh"

APP="${1:-$PROJECT_DIR/VocalAirPlus/build/Build/Products/Release/AIV.app}"

require_apple_development

if [[ ! -d "$APP" ]]; then
    echo "ERROR: App not found: $APP"
    exit 1
fi

FRAMEWORK=$(find "$APP/Contents/Frameworks" -name 'AIVFramework.framework' -maxdepth 1 2>/dev/null | head -1)
APPEX=$(find "$APP/Contents/PlugIns" -name 'AIVExtension.appex' -maxdepth 1 2>/dev/null | head -1)
EXT_ENT="$PROJECT_DIR/VocalAirPlus/LogicAIV/macOS/AIVExtension/AIVExtension.entitlements"
APP_ENT="$PROJECT_DIR/VocalAirPlus/LogicAIV/macOS/AIV/AIV.entitlements"

echo "Signing VocalAir+ app: $APP"

if [[ -n "$FRAMEWORK" ]]; then
    codesign --force --sign "$SIGN_IDENTITY" "$FRAMEWORK/Versions/Current"
fi

if [[ -n "$APPEX" ]]; then
    codesign --force --sign "$SIGN_IDENTITY" --entitlements "$EXT_ENT" "$APPEX"
fi

codesign --force --sign "$SIGN_IDENTITY" --entitlements "$APP_ENT" "$APP"
codesign --verify --deep --strict --verbose=2 "$APP"

if codesign -dv "$APP" 2>&1 | grep -q "Signature=adhoc"; then
    echo "ERROR: VocalAir+ still ad-hoc signed"
    exit 1
fi

echo "OK: VocalAir+ signed ($(codesign -dv "$APP" 2>&1 | grep TeamIdentifier | head -1))"
