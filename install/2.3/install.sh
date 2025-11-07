#!/bin/bash
# Simple installer script: downloads a zip, extracts it, runs a command, and cleans up

# -----------------------------
# Hardcoded variables
# -----------------------------
ZIP_URL="https://github.com/<user>/<repo>/releases/latest/download/package.zip"
DOWNLOADS_DIR="$HOME/Downloads"
ZIP_NAME="nevo.zip"
EXTRACT_DIR="package_extracted"
# Hardcoded command to run inside extracted folder (adjust this)
RUN_COMMAND="$DOWNLOADS_DIR/$ZIP_NAME/nevo-2.3/installer/install.sh"

# -----------------------------
# Download ZIP
# -----------------------------
echo "📥 Downloading zip from GitHub..."
curl -L "$ZIP_URL" -o "$DOWNLOADS_DIR/$ZIP_NAME"

if [ $? -ne 0 ]; then
    echo "❌ Failed to download ZIP."
    exit 1
fi

# -----------------------------
# Extract ZIP
# -----------------------------
echo "📂 Extracting ZIP..."
mkdir -p "$DOWNLOADS_DIR/$EXTRACT_DIR"
unzip -q "$DOWNLOADS_DIR/$ZIP_NAME" -d "$DOWNLOADS_DIR/$EXTRACT_DIR"

if [ $? -ne 0 ]; then
    echo "❌ Failed to extract ZIP."
    exit 1
fi

# -----------------------------
# Run command inside extracted folder
# -----------------------------
echo "⚡ Running command inside extracted folder..."
cd "$DOWNLOADS_DIR/$EXTRACT_DIR" || exit 1

# Hardcoded command
$RUN_COMMAND

# -----------------------------
# Cleanup
# -----------------------------
echo "🧹 Cleaning up..."
cd "$DOWNLOADS_DIR" || exit 1
rm -rf "$ZIP_NAME" "$EXTRACT_DIR"

echo "✅ Done!"
