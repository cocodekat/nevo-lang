#!/bin/bash

TARGET_DIR="$HOME/nevo"
mkdir -p "$TARGET_DIR/Libraries"
mkdir -p "$TARGET_DIR/Modes"

echo "Compiling Nevo Compiler..."
clang "../nevo.c"   -o "$TARGET_DIR/nevo"
clang "../n.c"      -o "$TARGET_DIR/Modes/n"
clang "../web-n.c"  -o "$TARGET_DIR/Modes/webn"

echo "Enabling Builtin Functions..."
mv "../arradd.h" "$TARGET_DIR/Libraries"
mv "../h1.h" "$TARGET_DIR/Libraries"
mv "../sha256.h" "$TARGET_DIR/Libraries"

echo "Cleaning Up Temporary Python Files..."
rm -rf Python-3.12.6 Python-3.12.6.tar.xz

echo "✅ Done! Everything You Need Is Inside Of $TARGET_DIR"
read -p "Press Enter To Continue..."
