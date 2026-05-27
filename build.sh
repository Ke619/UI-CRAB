#!/bin/bash
set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT_NAME="HeadcrabUpdater-x86_64.AppImage"

echo "╔══════════════════════════════════════╗"
echo "║     HEADCRAB UPDATER - BUILD         ║"
echo "╚══════════════════════════════════════╝"
echo ""

echo "[*] Checking dependencies..."
command -v python3 >/dev/null || { echo "[!] python3 not found"; exit 1; }
python3 -c "from PyQt5.QtWidgets import QApplication" 2>/dev/null || { echo "[!] PyQt5 not found"; exit 1; }
command -v curl >/dev/null || { echo "[!] curl not found"; exit 1; }
echo "[✓] Dependencies OK"

if [ ! -f "$SCRIPT_DIR/appimagetool" ]; then
    echo "[*] Downloading appimagetool..."
    curl -fsSL -o "$SCRIPT_DIR/appimagetool" \
      "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage"
    chmod +x "$SCRIPT_DIR/appimagetool"
    echo "[✓] appimagetool ready"
fi

echo "[*] Building AppImage..."
cd "$SCRIPT_DIR"
ARCH=x86_64 ./appimagetool AppDir "$OUTPUT_NAME"

echo ""
echo "[✓] Done! Created: $SCRIPT_DIR/$OUTPUT_NAME"
