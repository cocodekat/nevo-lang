#!/bin/bash
# ── Installer for Nevo (macOS) ──
# Double-click this in Finder to run

# Get the directory of this script
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Use AppleScript to open Terminal.app and run the installer in an interactive shell
osascript <<EOF
tell application "Terminal"
    activate
    do script "cd \"$SCRIPT_DIR\" && bash \"$SCRIPT_DIR/install_nevo_inner.sh\""
end tell
EOF
