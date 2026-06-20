#!/bin/bash
# Build VocalAir+ (AUv3 vocal chain) from upstream LogicAIV, signed for Vocal+ / Logic Pro.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
ACTION="${1:-build}"

chmod +x "$SCRIPT_DIR/"*.sh

detect_team_id() {
    security find-certificate -a -c "Apple Development" -p 2>/dev/null \
        | openssl x509 -noout -subject 2>/dev/null \
        | sed -n 's/.*OU=\([^/]*\).*/\1/p' \
        | head -1
}

TEAM_ID="${VOCALPLUS_DEVELOPMENT_TEAM:-$(detect_team_id)}"

case "$ACTION" in
    fetch)
        "$SCRIPT_DIR/fetch-github.sh"
        "$SCRIPT_DIR/rebrand-vocalair.sh"
        ;;
    build)
        "$SCRIPT_DIR/fetch-github.sh"
        "$SCRIPT_DIR/rebrand-vocalair.sh"
        if [[ -z "$TEAM_ID" ]]; then
            echo "ERROR: Apple Development team required for Logic Pro."
            exit 1
        fi
        echo "Using Apple Development team: $TEAM_ID"
        xcodebuild -project "$PROJECT_DIR/VocalAirPlus/LogicAIV/AIV.xcodeproj" \
            -scheme "AIV macOS" \
            -configuration Release \
            -destination 'platform=macOS' \
            -derivedDataPath "$PROJECT_DIR/VocalAirPlus/build" \
            DEVELOPMENT_TEAM="${TEAM_ID}" \
            CODE_SIGN_STYLE=Automatic \
            CODE_SIGN_IDENTITY="Apple Development" \
            clean build
        "$SCRIPT_DIR/sign-auv3.sh"
        ;;
    install|logic)
        "$SCRIPT_DIR/install-logic.sh"
        ;;
    *)
        echo "Usage: $0 [fetch|build|install|logic]"
        exit 1
        ;;
esac
