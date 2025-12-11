#!/bin/bash

# ── Detect the directory where this script is ──
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR" || exit 1

TARGET_DIR="$HOME/nevo"

# ── Create needed folders
mkdir -p "$TARGET_DIR/Modes"

# ── Check that source files exist
echo "Checking source files..."
for f in ../nevo.c ../modes/n.c ../auto_var.c ../ban_list.c; do
    if [ ! -f "$f" ]; then
        echo "❌ Missing source file: $f"
        exit 1
    fi
done

# ── Compile the main nevo executable
echo "Compiling Nevo Compiler..."
clang "../nevo.c" -o "$TARGET_DIR/nevo" || { echo "❌ Failed to compile nevo.c"; exit 1; }

# ── Compile the n compiler
echo "Compiling n compiler..."
clang "../modes/n.c" "../auto_var.c" "../ban_list.c" -o "$TARGET_DIR/Modes/n" || { echo "❌ Failed to compile n.c"; exit 1; }

# ── Move needed Libraries folder
echo "Enabling Builtin Functions..."
rm -rf "$TARGET_DIR/Libraries"
mv "../Libraries" "$TARGET_DIR"

# ── Done
echo "✅ Installation complete! Everything is inside $TARGET_DIR"
read -p "Press Enter to continue..."
