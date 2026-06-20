#!/bin/bash
# Shared Logic Pro signing helpers for the Vocal+ plugin family.
# Local installs must use Apple Development (not ad-hoc) for Logic Pro compatibility.

detect_sign_identity() {
    security find-identity -v -p codesigning 2>/dev/null \
        | awk -F'"' '/Apple Development/ { print $2; exit }'
}

detect_team_id() {
    security find-certificate -a -c "Apple Development" -p 2>/dev/null \
        | openssl x509 -noout -subject 2>/dev/null \
        | sed -n 's/.*OU=\([^/]*\).*/\1/p' \
        | head -1
}

require_apple_development() {
    SIGN_IDENTITY="${SIGN_IDENTITY:-$(detect_sign_identity)}"

    if [[ -z "$SIGN_IDENTITY" ]]; then
        echo "ERROR: No Apple Development certificate found."
        echo "Xcode → Settings → Accounts → Manage Certificates → + Apple Development"
        exit 1
    fi

    echo "Signing identity: $SIGN_IDENTITY"
}

# AU: sign inner binary + bundle, no hardened runtime, no timestamp (timestamp service often unavailable).
sign_au_for_logic() {
    local component="$1"
    local binary_name="$2"

    local binary="$component/Contents/MacOS/$binary_name"

    if [[ ! -f "$binary" ]]; then
        echo "Skip AU (binary missing): $component"
        return 0
    fi

    echo "Signing AU: $component"
    codesign --force --sign "$SIGN_IDENTITY" "$binary"
    codesign --force --sign "$SIGN_IDENTITY" "$component"
    codesign --verify --strict --verbose=2 "$component"

    if codesign -dv "$component" 2>&1 | grep -q "Signature=adhoc"; then
        echo "ERROR: $component still ad-hoc after signing"
        exit 1
    fi
}

sign_bundle_deep_for_logic() {
    local bundle="$1"
    local entitlements="$2"

    if [[ ! -d "$bundle" ]]; then
        echo "Skip (not found): $bundle"
        return 0
    fi

    echo "Signing: $bundle"
    codesign --force --sign "$SIGN_IDENTITY" \
        --entitlements "$entitlements" \
        --options runtime \
        --deep \
        "$bundle"
    codesign --verify --deep --strict --verbose=2 "$bundle"
}

install_plugin_bundle() {
    local src="$1"
    local dest="$2"
    local binary_name="$3"

    if [[ ! -f "$src/Contents/MacOS/$binary_name" ]]; then
        echo "Skip install (AU not built): $src"
        rm -rf "$dest"
        return 0
    fi

    rm -rf "$dest"
    ditto "$src" "$dest"
    echo "Installed → $dest"
}

clear_plugin_quarantine() {
    local path="$1"
    if [[ -e "$path" ]]; then
        xattr -cr "$path" 2>/dev/null || true
    fi
}

refresh_au_registry() {
    killall -9 AudioComponentRegistrar 2>/dev/null || true
    rm -rf "$HOME/Library/Caches/AudioUnitCache"
    mkdir -p "$HOME/Library/Caches/AudioUnitCache"
}

verify_au_for_logic() {
    local component="$1"
    local name="$2"

    if [[ ! -d "$component" ]]; then
        echo "ERROR: $name not installed at $component"
        exit 1
    fi

    local info
    info=$(codesign -dv "$component" 2>&1) || true

    if echo "$info" | grep -q "Signature=adhoc"; then
        echo "ERROR: $name is ad-hoc signed. Logic will mark it incompatible/unsigned."
        exit 1
    fi

    if echo "$info" | grep -q "TeamIdentifier=not set"; then
        echo "ERROR: $name has no TeamIdentifier."
        exit 1
    fi

    echo "OK: $name signed ($(echo "$info" | grep TeamIdentifier | head -1))"
}
