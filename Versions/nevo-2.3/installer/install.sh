#!/bin/bash

# ── Detect the directory where this script is ──
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

TARGET_DIR="$HOME/nevo"

# ── Building needed folders
mkdir -p "$TARGET_DIR/Libraries/Images"
mkdir -p "$TARGET_DIR/Modes"

# ── Compiling the compiler
echo "Compiling Nevo Compiler..."
clang "../nevo.c" -o "$TARGET_DIR/nevo"
clang "../modes/n.c", "../auto_var.c", "../ban_list.c" -o "$TARGET_DIR/Modes/n"

# ── Moving the needed .h files
echo "Enabling Builtin Functions..."
mv "../Libraries/arradd.h" "$TARGET_DIR/Libraries"
mv "../Libraries/h1.h" "$TARGET_DIR/Libraries"
mv "../Libraries/sha256.h" "$TARGET_DIR/Libraries"

mv "../libraries/images/npxm.h" "$TARGET_DIR/Libraries/Images"
mv "../libraries/images/stb_image_write.h" "$TARGET_DIR/Libraries/Images"
mv "../libraries/images/stb_image.h" "$TARGET_DIR/Libraries/Images"

# ── Done!
echo "✅ Done! Everything You Need Is Inside Of $TARGET_DIR"
read -p "Press Enter To Continue..."
