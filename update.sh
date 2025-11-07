#!/bin/bash
# Simple script to push a single file or directory to GitHub

# Check for arguments
if [ $# -lt 1 ]; then
    echo "Usage:"
    echo "  ./update.sh <file>"
    echo "  ./update.sh -dir <folder>"
    exit 1
fi

# Handle directory mode
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

    echo "📂 Updating directory: $TARGET"
    git add "$TARGET"
    git commit -m "Update directory $TARGET"
    git push origin main
else
    TARGET="$1"

    if [ ! -f "$TARGET" ]; then
        echo "❌ '$TARGET' is not a file!"
        exit 1
    fi

    echo "📄 Updating file: $TARGET"
    git add "$TARGET"
    git commit -m "Update $TARGET"
    git push origin main
fi
