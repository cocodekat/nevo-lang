#!/bin/bash
set -e

# 1️⃣ Variables
GITHUB_URL="https://github.com/cocodekat/Nevo/raw/main/package.py"
INSTALL_DIR="${HOME}/package"
PYTHON_DIR="$INSTALL_DIR/python"
mkdir -p "$INSTALL_DIR"

echo "📥 Downloading package.py from GitHub …"

# 2️⃣ Download package.py directly
curl -L -o "$INSTALL_DIR/package.py" "$GITHUB_URL"

if [ ! -f "$INSTALL_DIR/package.py" ]; then
    echo "❌ Failed to download package.py"
    exit 1
fi

echo "✅ Downloaded Python file: $INSTALL_DIR/package.py"

# 3️⃣ Download portable Python (Linux/macOS example: Python 3.12 source)
# Change URL if you want a prebuilt portable version for your OS
PYTHON_TAR="$INSTALL_DIR/Python.tgz"
echo "📥 Downloading portable Python …"
curl -L -o "$PYTHON_TAR" "https://www.python.org/ftp/python/3.12.0/Python-3.12.0.tgz"
mkdir -p "$PYTHON_DIR"
tar -xzf "$PYTHON_TAR" -C "$PYTHON_DIR" --strip-components=1
rm -f "$PYTHON_TAR"

# 4️⃣ Setup Python executable
PYTHON_BIN="$PYTHON_DIR/bin/python3"
if [ ! -x "$PYTHON_BIN" ]; then
    # fallback to system python if portable not built
    PYTHON_BIN=$(which python3)
fi

# 5️⃣ Ensure pip & install requirements
"$PYTHON_BIN" -m ensurepip --upgrade
"$PYTHON_BIN" -m pip install --upgrade pip setuptools wheel pyinstaller requests

# 6️⃣ Create launcher package.sh
cat > "$INSTALL_DIR/package.sh" << 'EOF'
#!/bin/bash
set -e

BASE_DIR="$(cd "$(dirname "$0")" && pwd)"

# Use bundled Python if exists
if [ -x "$BASE_DIR/python/bin/python3" ]; then
    PYTHON_BIN="$BASE_DIR/python/bin/python3"
else
    PYTHON_BIN=$(which python3)
    if [ -z "$PYTHON_BIN" ]; then
        echo "❌ No Python found!"
        exit 1
    fi
fi

PACKAGE_PY="$BASE_DIR/package.py"
if [ ! -f "$PACKAGE_PY" ]; then
    echo "❌ package.py not found!"
    exit 1
fi

"$PYTHON_BIN" "$PACKAGE_PY" "$@"
EOF

chmod +x "$INSTALL_DIR/package.sh"

echo "✅ Installation complete!"
echo "Run your package with: $INSTALL_DIR/package.sh -install nevo"
