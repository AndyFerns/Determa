#!/bin/bash
set -e

echo "------------------------------------------"
echo "  Determa Compiler Installer (Linux/Mac)"
echo "------------------------------------------"
echo ""

# 1. Build
echo "[1/3] Building latest version..."
chmod +x build.sh
./build.sh

# 2. Install
INSTALL_DIR="/usr/local/bin"
BINARY_NAME="determa"

echo ""
echo "[2/3] Installing '$BINARY_NAME' to $INSTALL_DIR..."

# Check if we have write access to the install dir
if [ -w "$INSTALL_DIR" ]; then
    cp bin/determa "$INSTALL_DIR/$BINARY_NAME"
else
    echo "      (Sudo permissions required to copy to system bin)"
    sudo cp bin/determa "$INSTALL_DIR/$BINARY_NAME"
fi

echo ""
echo "[3/3] Verifying installation..."
if command -v $BINARY_NAME >/dev/null 2>&1; then
    echo "[SUCCESS] Determa installed successfully!"
    echo ""
    echo "------------------------------------------"
    echo "Installation Complete."
    echo "Type 'determa' to run the REPL."
    echo "------------------------------------------"
else
    echo "[ERROR] Installation failed. Could not find '$BINARY_NAME' in PATH."
    exit 1
fi