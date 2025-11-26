#!/bin/bash

# ── Detect the directory where this script is ──
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

TARGET_DIR="$HOME/nevo"

# ── Building needed folders
mkdir -p "$TARGET_DIR/Libraries/Images"
mkdir -p "$TARGET_DIR/Modes"

ls ../modes/n.c ../auto_var.c ../ban_list.c

# ── Compiling the compiler
echo "Compiling Nevo Compiler..."
clang "../nevo.c" -o "$TARGET_DIR/nevo"
clang "../modes/n.c" "../auto_var.c" "../ban_list.c" -o "$TARGET_DIR/Modes/n"

# ── Moving the needed .h files
echo "Enabling Builtin Functions..."
mv "../Libraries/" "$TARGET_DIR"

# ── Done!
echo "✅ Done! Everything You Need Is Inside Of $TARGET_DIR"
read -p "Press Enter To Continue..."
