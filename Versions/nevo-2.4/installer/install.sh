#!/bin/bash

# ── Detect the directory where this script is ──
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR" || exit 1

TARGET_DIR="$HOME/nevo"

# ── Building needed folders
mkdir -p "$TARGET_DIR/Modes"

# ── Check that source files exist
echo "Checking source files..."
for f in ../modes/n.c ../auto_var.c ../ban_list.c ../nevo.c; do
    if [ ! -f "$f" ]; then
        echo "❌ Missing source file: $f"
        exit 1
    fi
done

# ── Compiling the compiler
echo "Compiling Nevo Compiler..."
clang "../nevo.c" -o "$TARGET_DIR/nevo" || { echo "❌ Failed to compile nevo.c"; exit 1; }
clang "../modes/n.c" "../auto_var.c" "../ban_list.c" -o "$TARGET_DIR/Modes/n" || { echo "❌ Failed to compile n.c"; exit 1; }

# ── Moving the needed .h files
echo "Enabling Builtin Functions..."
rm -rf "$TARGET_DIR/Libraries"
mv "../Libraries" "$TARGET_DIR"

# ── Detect shell rc file
RC_FILE="$HOME/.bashrc"
if [ -n "$ZSH_VERSION" ]; then
    RC_FILE="$HOME/.zshrc"
fi

# ── Add nevo executable to PATH if not already there
if ! grep -q "$TARGET_DIR" "$RC_FILE"; then
    echo "export PATH=\"$TARGET_DIR:\$PATH\"" >> "$RC_FILE"
    echo "✅ Added $TARGET_DIR to PATH in $RC_FILE"
fi

# ── Source rc file to apply PATH immediately
source "$RC_FILE"

# ── Done!
echo "✅ Done! Everything You Need Is Inside Of $TARGET_DIR"
echo "You can now use 'nevo' anywhere in this terminal."
read -p "Press Enter To Continue..."
