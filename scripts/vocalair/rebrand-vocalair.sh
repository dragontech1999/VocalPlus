#!/bin/bash
# Rebrand upstream LogicAIV → VocalAir+ under Vocal+ (VcPl), no iairu identifiers.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
SRC_DIR="$PROJECT_DIR/VocalAirPlus/LogicAIV"

if [[ ! -d "$SRC_DIR/AIV.xcodeproj" ]]; then
    echo "ERROR: Run scripts/vocalair/fetch-github.sh first."
    exit 1
fi

EXT_PLIST="$SRC_DIR/macOS/AIVExtension/Info.plist"
PBXPROJ="$SRC_DIR/AIV.xcodeproj/project.pbxproj"

/usr/libexec/PlistBuddy -c "Set :CFBundleDisplayName VocalAir+" "$EXT_PLIST"
/usr/libexec/PlistBuddy -c "Set :NSExtension:NSExtensionAttributes:AudioComponentBundle com.vocalplus.vocalairplus.VocalAirPlusFramework" "$EXT_PLIST"
/usr/libexec/PlistBuddy -c "Set :NSExtension:NSExtensionAttributes:AudioComponents:0:description VocalAir+" "$EXT_PLIST"
/usr/libexec/PlistBuddy -c "Set :NSExtension:NSExtensionAttributes:AudioComponents:0:name Vocal+: VocalAir+" "$EXT_PLIST"
/usr/libexec/PlistBuddy -c "Set :NSExtension:NSExtensionAttributes:AudioComponents:0:manufacturer VcPl" "$EXT_PLIST"
/usr/libexec/PlistBuddy -c "Set :NSExtension:NSExtensionAttributes:AudioComponents:0:subtype VaA1" "$EXT_PLIST"

python3 - <<PY
from pathlib import Path
p = Path("$PBXPROJ")
text = p.read_text()
replacements = {
    "com.iairu.AIV.AIVFramework": "com.vocalplus.vocalairplus.VocalAirPlusFramework",
    "com.iairu.AIV.AIVExtension": "com.vocalplus.vocalairplus.VocalAirPlusExtension",
    "com.iairu.AIV": "com.vocalplus.vocalairplus",
}
for old, new in replacements.items():
    text = text.replace(old, new)
p.write_text(text)
PY

echo ">>> VocalAir+ rebranded for Vocal+ (VcPl / com.vocalplus.vocalairplus)"

"$SCRIPT_DIR/customize-vocalair.sh"
