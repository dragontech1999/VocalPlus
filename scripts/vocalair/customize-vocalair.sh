#!/bin/bash
# VocalAir+ UI and runtime patches applied after upstream fetch/rebrand.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
SRC_DIR="$PROJECT_DIR/VocalAirPlus/LogicAIV"
OVERRIDES="$SCRIPT_DIR/overrides"

if [[ ! -d "$SRC_DIR/AIV.xcodeproj" ]]; then
    echo "ERROR: LogicAIV source missing."
    exit 1
fi

cp "$OVERRIDES/AIVMainView.swift" \
    "$SRC_DIR/Shared/Support/User Interface/AIVMainView.swift"

/usr/libexec/PlistBuddy -c "Add :CFBundleDisplayName string VocalAir+" \
    "$SRC_DIR/macOS/AIV/Info.plist" 2>/dev/null \
    || /usr/libexec/PlistBuddy -c "Set :CFBundleDisplayName VocalAir+" \
        "$SRC_DIR/macOS/AIV/Info.plist"

python3 - <<PY
from pathlib import Path

src = Path("$SRC_DIR")
storyboard = src / "macOS/AIV/Base.lproj/Main.storyboard"
text = storyboard.read_text()
text = text.replace('title="AIV"', 'title="VocalAir+"')
text = text.replace('frameAutosaveName="AIV"', 'frameAutosaveName="VocalAirPlus"')
text = text.replace('width="900" height="560"', 'width="1125" height="700"')
storyboard.write_text(text)

manager = src / "Shared/AudioUnitManager.swift"
mtext = manager.read_text()
replacements = {
    "componentSubType = 0x666c7472 /*'fltr'*/": "componentSubType = 0x56614131 /*'VaA1'*/",
    "componentManufacturer = 0x69616972 /*'iair'*/": "componentManufacturer = 0x5663506c /*'VcPl'*/",
    'private let componentName = "iairu: AIVAlpha"': 'private let componentName = "Vocal+: VocalAir+"',
    "componentSubType: 0x666c7472, // 'fltr'": "componentSubType: 0x56614131, // 'VaA1'",
    "componentManufacturer: 0x69616972, // 'iair'": "componentManufacturer: 0x5663506c, // 'VcPl'",
    'let bundleID = "com.iairu.AIV.AIVExtension"': 'let bundleID = "com.vocalplus.vocalairplus.VocalAirPlusExtension"',
}
for old, new in replacements.items():
    mtext = mtext.replace(old, new)
manager.write_text(mtext)

main_vc = src / "macOS/AIV/MainViewController.swift"
mvc = main_vc.read_text()
if 'window.title = "VocalAir+"' not in mvc:
    mvc = mvc.replace(
        '            }\n        }\n    }\n\n    private func embedPlugInView()',
        '            }\n        }\n\n        if let window = view.window {\n'
        '            window.title = "VocalAir+"\n'
        '            window.setContentSize(NSSize(width: 1125, height: 700))\n'
        '        }\n    }\n\n    private func embedPlugInView()',
        1,
    )
    main_vc.write_text(mvc)
PY

echo ">>> VocalAir+ customized (in-app name, 125% rack view, VcPl/VaA1 runtime IDs)"
