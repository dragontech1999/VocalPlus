#!/bin/bash
# Build a standalone macOS installer (.pkg) for the full Vocal+ suite by Neeberman.
# Plugins are signed with Apple Development (required for Logic Pro verification).
# The .pkg itself is ad-hoc signed so no Developer ID Installer cert is needed.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
CONFIG="${1:-Release}"
VERSION="1.0.3"
PKG_ID="com.vocalplus.plugins"
OUTPUT_NAME="VocalPlus-Plugins-${VERSION}.pkg"
DESKTOP="${HOME}/Desktop"

detect_sign_identity() {
    security find-identity -v -p codesigning 2>/dev/null \
        | awk -F'"' '/Apple Development/ { print $2; exit }'
}

SIGN_IDENTITY="${SIGN_IDENTITY:-$(detect_sign_identity)}"

if [[ -z "$SIGN_IDENTITY" ]]; then
    echo "ERROR: No Apple Development certificate found."
    echo "Logic Pro requires properly signed plugins — ad-hoc plugin signing will show as incompatible."
    echo "Create one in Xcode → Settings → Accounts → Manage Certificates → + Apple Development"
    exit 1
fi

echo "Plugin signing identity: $SIGN_IDENTITY"
echo "Package: unsigned (no Developer ID Installer cert; plugins inside are properly signed)"

WORK=$(mktemp -d)
STAGING="$WORK/staging"
SCRIPTS="$WORK/scripts"
trap 'rm -rf "$WORK"' EXIT

mkdir -p "$STAGING/Library/Application Support/VocalPlus/Install/Components"
mkdir -p "$STAGING/Library/Application Support/VocalPlus/Install/VST3"
mkdir -p "$SCRIPTS"

sign_au() {
    local component="$1"
    local binary_name="$2"

    local binary="$component/Contents/MacOS/$binary_name"
    [[ -f "$binary" ]] || return 0

    codesign --force --sign "$SIGN_IDENTITY" "$binary"
    codesign --force --sign "$SIGN_IDENTITY" "$component"
    codesign --verify --strict "$component"

    if codesign -dv "$component" 2>&1 | grep -q "Signature=adhoc"; then
        echo "ERROR: $component still ad-hoc after signing"
        exit 1
    fi
}

sign_bundle_deep() {
    local bundle="$1"
    local entitlements="$2"

    [[ -d "$bundle" ]] || return 0

    codesign --force --sign "$SIGN_IDENTITY" \
        --entitlements "$entitlements" \
        --options runtime \
        --deep \
        "$bundle"
    codesign --verify --deep --strict "$bundle"
}

stage_plugin() {
    local src="$1"
    local dest="$2"
    local binary_name="$3"
    local kind="$4"
    local entitlements="${5:-}"

    if [[ ! -f "$src/Contents/MacOS/$binary_name" ]]; then
        echo "ERROR: Missing build artefact: $src"
        echo "Run: ./build.sh build"
        exit 1
    fi

    rm -rf "$dest"
    ditto "$src" "$dest"

    if [[ "$kind" == "au" ]]; then
        sign_au "$dest" "$binary_name"
    else
        sign_bundle_deep "$dest" "$entitlements"
    fi

    echo "Staged: $(basename "$dest")"
}

# --- Stage signed plugins ---

VOCAL="$PROJECT_DIR/Build/VocalPlus_artefacts/$CONFIG"
EQ="$PROJECT_DIR/Build/EQPlus_artefacts/$CONFIG"
TUNE="$PROJECT_DIR/Build/TunePlus_artefacts/$CONFIG"
VOX="$PROJECT_DIR/Build/VoxPlus_artefacts/$CONFIG"
VC="$PROJECT_DIR/Build/VocalChangePlus_artefacts/$CONFIG"
VAI="$PROJECT_DIR/Build/VocalAIPlus_artefacts/$CONFIG"

INSTALL_ROOT="$STAGING/Library/Application Support/VocalPlus/Install"

stage_plugin "$VOCAL/AU/VocalPlus.component" \
    "$INSTALL_ROOT/Components/VocalPlus.component" \
    "VocalPlus" "au"

stage_plugin "$EQ/AU/EQPlus.component" \
    "$INSTALL_ROOT/Components/EQPlus.component" \
    "EQPlus" "au"

stage_plugin "$TUNE/AU/TunePlus.component" \
    "$INSTALL_ROOT/Components/TunePlus.component" \
    "TunePlus" "au"

stage_plugin "$VOX/AU/VoxPlus.component" \
    "$INSTALL_ROOT/Components/VoxPlus.component" \
    "VoxPlus" "au"

stage_plugin "$VC/AU/VocalChangePlus.component" \
    "$INSTALL_ROOT/Components/VocalChangePlus.component" \
    "VocalChangePlus" "au"

stage_plugin "$VAI/AU/VocalAIPlus.component" \
    "$INSTALL_ROOT/Components/VocalAIPlus.component" \
    "VocalAIPlus" "au"

stage_plugin "$VOCAL/VST3/VocalPlus.vst3" \
    "$INSTALL_ROOT/VST3/VocalPlus.vst3" \
    "VocalPlus" "vst3" "$PROJECT_DIR/cmake/VocalPlus_VST3.entitlements"

stage_plugin "$EQ/VST3/EQPlus.vst3" \
    "$INSTALL_ROOT/VST3/EQPlus.vst3" \
    "EQPlus" "vst3" "$PROJECT_DIR/cmake/EQPlus_VST3.entitlements"

stage_plugin "$TUNE/VST3/TunePlus.vst3" \
    "$INSTALL_ROOT/VST3/TunePlus.vst3" \
    "TunePlus" "vst3" "$PROJECT_DIR/cmake/TunePlus_VST3.entitlements"

stage_plugin "$VOX/VST3/VoxPlus.vst3" \
    "$INSTALL_ROOT/VST3/VoxPlus.vst3" \
    "VoxPlus" "vst3" "$PROJECT_DIR/cmake/VoxPlus_VST3.entitlements"

stage_plugin "$VC/VST3/VocalChangePlus.vst3" \
    "$INSTALL_ROOT/VST3/VocalChangePlus.vst3" \
    "VocalChangePlus" "vst3" "$PROJECT_DIR/cmake/VocalChangePlus_VST3.entitlements"

stage_plugin "$VAI/VST3/VocalAIPlus.vst3" \
    "$INSTALL_ROOT/VST3/VocalAIPlus.vst3" \
    "VocalAIPlus" "vst3" "$PROJECT_DIR/cmake/VocalAIPlus_VST3.entitlements"

# --- Post-install: copy Vocal+ plugins to the console user's Library only ---
# Does NOT touch AudioUnitCache, AudioComponentRegistrar, or any other plugins.

cat > "$SCRIPTS/postinstall" << 'POSTINSTALL'
#!/bin/bash
set -e

SRC="/Library/Application Support/VocalPlus/Install"
CONSOLE_USER=$(stat -f%Su /dev/console 2>/dev/null || true)

if [[ -z "$CONSOLE_USER" || "$CONSOLE_USER" == "root" || "$CONSOLE_USER" == "loginwindow" ]]; then
    echo "No logged-in user — Vocal+ plugins staged at $SRC"
    echo "Log in and run: sudo \"$SRC/../reinstall-for-user.sh\""
    exit 0
fi

USER_HOME=$(dscl . -read "/Users/$CONSOLE_USER" NFSHomeDirectory 2>/dev/null | awk '{print $2}')
if [[ -z "$USER_HOME" || ! -d "$USER_HOME" ]]; then
    echo "Could not resolve home for user $CONSOLE_USER"
    exit 1
fi

AU_DEST="$USER_HOME/Library/Audio/Plug-Ins/Components"
VST3_DEST="$USER_HOME/Library/Audio/Plug-Ins/VST3"
mkdir -p "$AU_DEST" "$VST3_DEST"

# Remove only Vocal+ family duplicates (legacy names + old system-wide copies)
rm -rf "$AU_DEST/Vocal+.component"
rm -rf "$AU_DEST/VocalPlus.component"
rm -rf "$AU_DEST/EQPlus.component"
rm -rf "$AU_DEST/TunePlus.component"
rm -rf "$AU_DEST/VoxPlus.component"
rm -rf "$AU_DEST/VocalChangePlus.component"
rm -rf "$AU_DEST/VocalAIPlus.component"
rm -rf "$VST3_DEST/Vocal+.vst3"
rm -rf "$VST3_DEST/VocalPlus.vst3"
rm -rf "$VST3_DEST/EQPlus.vst3"
rm -rf "$VST3_DEST/TunePlus.vst3"
rm -rf "$VST3_DEST/VoxPlus.vst3"
rm -rf "$VST3_DEST/VocalChangePlus.vst3"
rm -rf "$VST3_DEST/VocalAIPlus.vst3"
rm -rf "/Library/Audio/Plug-Ins/Components/Vocal+.component"
rm -rf "/Library/Audio/Plug-Ins/Components/VocalPlus.component"
rm -rf "/Library/Audio/Plug-Ins/Components/EQPlus.component"
rm -rf "/Library/Audio/Plug-Ins/Components/TunePlus.component"
rm -rf "/Library/Audio/Plug-Ins/Components/VoxPlus.component"
rm -rf "/Library/Audio/Plug-Ins/Components/VocalChangePlus.component"
rm -rf "/Library/Audio/Plug-Ins/Components/VocalAIPlus.component"
rm -rf "/Library/Audio/Plug-Ins/VST3/Vocal+.vst3"
rm -rf "/Library/Audio/Plug-Ins/VST3/VocalPlus.vst3"
rm -rf "/Library/Audio/Plug-Ins/VST3/EQPlus.vst3"
rm -rf "/Library/Audio/Plug-Ins/VST3/TunePlus.vst3"
rm -rf "/Library/Audio/Plug-Ins/VST3/VoxPlus.vst3"
rm -rf "/Library/Audio/Plug-Ins/VST3/VocalChangePlus.vst3"
rm -rf "/Library/Audio/Plug-Ins/VST3/VocalAIPlus.vst3"

install_bundle() {
    local src="$1"
    local dest="$2"
    [[ -d "$src" ]] || return 0
    rm -rf "$dest"
    ditto "$src" "$dest"
    chown -R "$CONSOLE_USER:staff" "$dest"
}

install_bundle "$SRC/Components/VocalPlus.component" "$AU_DEST/VocalPlus.component"
install_bundle "$SRC/Components/EQPlus.component" "$AU_DEST/EQPlus.component"
install_bundle "$SRC/Components/TunePlus.component" "$AU_DEST/TunePlus.component"
install_bundle "$SRC/Components/VoxPlus.component" "$AU_DEST/VoxPlus.component"
install_bundle "$SRC/Components/VocalChangePlus.component" "$AU_DEST/VocalChangePlus.component"
install_bundle "$SRC/Components/VocalAIPlus.component" "$AU_DEST/VocalAIPlus.component"
install_bundle "$SRC/VST3/VocalPlus.vst3" "$VST3_DEST/VocalPlus.vst3"
install_bundle "$SRC/VST3/EQPlus.vst3" "$VST3_DEST/EQPlus.vst3"
install_bundle "$SRC/VST3/TunePlus.vst3" "$VST3_DEST/TunePlus.vst3"
install_bundle "$SRC/VST3/VoxPlus.vst3" "$VST3_DEST/VoxPlus.vst3"
install_bundle "$SRC/VST3/VocalChangePlus.vst3" "$VST3_DEST/VocalChangePlus.vst3"
install_bundle "$SRC/VST3/VocalAIPlus.vst3" "$VST3_DEST/VocalAIPlus.vst3"

echo "Vocal+ plugins installed for $CONSOLE_USER"
echo "Quit Logic (Cmd+Q), reopen, then rescan only if Vocal+ does not appear."
exit 0
POSTINSTALL

chmod +x "$SCRIPTS/postinstall"

# --- Build component package ---

PKG_UNSIGNED="$WORK/unsigned.pkg"
pkgbuild \
    --root "$STAGING" \
    --scripts "$SCRIPTS" \
    --identifier "$PKG_ID" \
    --version "$VERSION" \
    --install-location / \
    "$PKG_UNSIGNED"

# --- Ad-hoc sign the installer package ---

OUTPUT="$DESKTOP/$OUTPUT_NAME"
rm -f "$OUTPUT"
cp "$PKG_UNSIGNED" "$OUTPUT"

echo ""
echo ">>> Verifying plugin signatures in package payload..."
for comp in VocalPlus EQPlus TunePlus VoxPlus VocalChangePlus VocalAIPlus; do
    comp_path="$INSTALL_ROOT/Components/${comp}.component"
    info=$(codesign -dv "$comp_path" 2>&1) || true
    if echo "$info" | grep -q "Signature=adhoc"; then
        echo "ERROR: $comp is ad-hoc signed — Logic will mark it incompatible."
        exit 1
    fi
    echo "OK: $comp ($(echo "$info" | grep TeamIdentifier | head -1))"
done

echo ""
echo ">>> AU validation..."
auval -v aufx VcP1 VcPl >/dev/null 2>&1 && echo "OK: Vocal+ auval PASS" || echo "WARN: Vocal+ auval had issues"
auval -v aufx EqP1 VcPl >/dev/null 2>&1 && echo "OK: EQ+ auval PASS" || echo "WARN: EQ+ auval had issues"
auval -v aufx TnP1 VcPl >/dev/null 2>&1 && echo "OK: Tune+ auval PASS" || echo "WARN: Tune+ auval had issues"
auval -v aufx VxP1 VcPl >/dev/null 2>&1 && echo "OK: VOX auval PASS" || echo "WARN: VOX auval had issues"
auval -v aufx VcC1 VcPl >/dev/null 2>&1 && echo "OK: VocalChange+ auval PASS" || echo "WARN: VocalChange+ auval had issues"
auval -v aufx VaI1 VcPl >/dev/null 2>&1 && echo "OK: VocalAI+ auval PASS" || echo "WARN: VocalAI+ auval had issues"

echo ""
echo "Installer saved to: $OUTPUT"
echo ""
echo "To install: double-click the package on your Desktop."
echo "Plugins install to ~/Library/Audio/Plug-Ins/ for the logged-in user only."
echo "Other plugins are not modified. After install, quit Logic (Cmd+Q) and reopen."
echo "If Vocal+ does not appear: Logic Pro → Settings → Plug-In Manager → Reset & Rescan Selection"
echo "If other plugins are missing after a previous install: ./scripts/recover-logic-plugins.sh"
