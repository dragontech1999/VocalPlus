#!/bin/bash
# Recover third-party plugins in Logic after an aggressive cache wipe.
# Does NOT delete plugin files or caches — only verifies files on disk and
# prints steps to restore Logic's Plug-In Manager listing.

set -euo pipefail

echo "=== Logic Plug-In Recovery ==="
echo ""
echo "Your third-party plugins are almost certainly still on disk."
echo "Logic's Plug-In Manager list can be rebuilt without deleting anything."
echo ""

count_plugins() {
    local dir="$1"
    local label="$2"
    if [[ -d "$dir" ]]; then
        local n
        n=$(find "$dir" -maxdepth 1 \( -name '*.component' -o -name '*.vst3' \) 2>/dev/null | wc -l | tr -d ' ')
        echo "  $label: $n plugins found"
    else
        echo "  $label: (folder not present)"
    fi
}

echo "Plugin files on disk:"
count_plugins "$HOME/Library/Audio/Plug-Ins/Components" "User AU  (~/Library/Audio/Plug-Ins/Components)"
count_plugins "$HOME/Library/Audio/Plug-Ins/VST3" "User VST3 (~/Library/Audio/Plug-Ins/VST3)"
count_plugins "/Library/Audio/Plug-Ins/Components" "System AU  (/Library/Audio/Plug-Ins/Components)"
count_plugins "/Library/Audio/Plug-Ins/VST3" "System VST3 (/Library/Audio/Plug-Ins/VST3)"

echo ""
echo "Steps to restore plugins in Logic:"
echo "  1. Quit Logic completely (Cmd+Q — not just close the window)"
echo "  2. Reopen Logic Pro"
echo "  3. Logic Pro → Settings → Plug-In Manager"
echo "  4. From the Options menu (⋯ or gear): choose"
echo "     \"Reset & Rescan All Plug-Ins\"  (NOT just \"Reset & Rescan Selection\")"
echo "  5. Wait for the full scan to finish — this can take several minutes"
echo ""
echo "If plugins still don't appear, also try:"
echo "  • Logic Pro → Settings → Plug-In Manager → enable \"Show Audio Units\""
echo "  • Restart your Mac, then repeat the rescan above"
echo ""
echo "Do NOT run installers or scripts that wipe ~/Library/Caches/AudioUnitCache"
echo "unless you intend to force a full rescan of every plugin on the system."
