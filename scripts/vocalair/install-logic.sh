#!/bin/bash
# Install VocalAir+ AUv3 app into /Applications and register with Logic.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"

# shellcheck source=../lib/logic-sign.sh
source "$SCRIPT_DIR/../lib/logic-sign.sh"

SRC_APP="$PROJECT_DIR/VocalAirPlus/build/Build/Products/Release/AIV.app"
STAGE_APP="$PROJECT_DIR/VocalAirPlus/build/Build/Products/Release/VocalAirPlus-staged.app"
DEST_APP="/Applications/VocalAirPlus.app"

echo ">>> Removing legacy iairu registrations..."
pluginkit -r com.iairu.VocalAirPlus.VocalAirPlusExtension 2>/dev/null || true
pluginkit -r com.iairu.AIV.AIVExtension 2>/dev/null || true
for legacy_appex in \
    "$HOME/Documents/VocalAirPlus/build/Build/Products/Release/AIV.app/Contents/PlugIns/AIVExtension.appex" \
    "$HOME/Documents/VocalAirPlus/build/Build/Products/Release/AIVExtension.appex"; do
    [[ -e "$legacy_appex" ]] && pluginkit -r "$legacy_appex" 2>/dev/null || true
done
rm -rf "/Applications/AIV.app" 2>/dev/null || true

if [[ ! -d "$SRC_APP" ]]; then
    echo "ERROR: VocalAir+ build missing. Run scripts/vocalair/build-vocalair.sh first."
    exit 1
fi

rm -rf "$STAGE_APP"
ditto "$SRC_APP" "$STAGE_APP"
/usr/libexec/PlistBuddy -c "Set :CFBundleName VocalAirPlus" "$STAGE_APP/Contents/Info.plist" 2>/dev/null || true
/usr/libexec/PlistBuddy -c "Add :CFBundleDisplayName string VocalAir+" "$STAGE_APP/Contents/Info.plist" 2>/dev/null \
    || /usr/libexec/PlistBuddy -c "Set :CFBundleDisplayName VocalAir+" "$STAGE_APP/Contents/Info.plist"

"$SCRIPT_DIR/sign-auv3.sh" "$STAGE_APP"

echo ">>> Installing to $DEST_APP..."
rm -rf "$DEST_APP"
ditto "$STAGE_APP" "$DEST_APP"
clear_plugin_quarantine "$DEST_APP"

refresh_au_registry
open -g "$DEST_APP"
sleep 3
osascript -e 'tell application "VocalAirPlus" to quit' 2>/dev/null || true

pluginkit -m -i com.vocalplus.vocalairplus.VocalAirPlusExtension 2>/dev/null | head -3 || true

echo "OK: VocalAir+ installed — Logic → Audio Units → Vocal+ → VocalAir+"
