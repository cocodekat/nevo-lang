#!/bin/bash
# Quiet script to push a single file or directory to GitHub

# -----------------------------
# Check for arguments
# -----------------------------
if [ $# -lt 1 ]; then
    echo "Usage:"
    echo "  ./update.sh <file>"
    echo "  ./update.sh -dir <folder>"
    exit 1
fi

# -----------------------------
# Handle directory mode
# -----------------------------
if [ "$1" == "-dir" ]; then
    if [ -z "$2" ]; then
        echo "❌ Missing directory argument."
        echo "Usage: ./update.sh -dir <folder>"
        exit 1
    fi

    TARGET="$2"

    if [ ! -d "$TARGET" ]; then
        echo "❌ '$TARGET' is not a directory!"
        exit 1
    fi

    git add "$TARGET" >/dev/null 2>&1
    git commit -m "Update directory $TARGET" >/dev/null 2>&1
    git push origin main >/dev/null 2>&1
else
    TARGET="$1"

    if [ ! -f "$TARGET" ]; then
        echo "❌ '$TARGET' is not a file!"
        exit 1
    fi

    git add "$TARGET" >/dev/null 2>&1
    git commit -m "Update $TARGET" >/dev/null 2>&1
    git push origin main >/dev/null 2>&1
fi
echo done