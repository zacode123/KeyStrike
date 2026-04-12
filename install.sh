#!/bin/bash
# ==========================================================
# KeyStrike - BLE HID Keyboard Emulation Tool
# ==========================================================
# Copyright (c) 2026 zacode123
#
# MIT License
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom
# the Software is furnished to do so, subject to the following
# conditions:
#
# The above copyright notice and this permission notice shall
# be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
# OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
# OR OTHER DEALINGS IN THE SOFTWARE.
# ==========================================================
set -e
VERSION="v4.3"
ORIGINAL_VERSION=$(keystrike -v 2>/dev/null | tr -d '\033' | sed 's/\[[0-9;]*m//g' | grep -o 'v[0-9.]*' || true)
USR="${PREFIX:-/usr}"
INSTALL_DIR="$USR/bin"
LIB_DIR="$USR/share/keystrike"
REPO_URL="https://raw.githubusercontent.com/zacode123/KeyStrike/main"
C_RESET="\e[0m"
C_RED="\e[1;31m"
C_GREEN="\e[1;32m"
C_YELLOW="\e[1;33m"
C_CYAN="\e[1;36m"
if [[ -n "$ORIGINAL_VERSION" && "$ORIGINAL_VERSION" == "$VERSION" ]]; then echo -e "${C_GREEN}[✓] KeyStrike is already at the latest version ($VERSION)${C_RESET}"
else [[ -n "$ORIGINAL_VERSION" ]] && echo -e "${C_YELLOW}[*] Updating KeyStrike $ORIGINAL_VERSION to $VERSION...${C_RESET}" || echo -e "${C_YELLOW}[*] Installing KeyStrike $VERSION...${C_RESET}"
fi
mkdir -p "$LIB_DIR"
curl -fsL "$REPO_URL/keystrike" -o keystrike
curl -fsL "$REPO_URL/key_library.json" -o key_library.json
[[ ! -f keystrike || ! -f key_library.json ]] && { echo -e "${C_RED}[!] Download failed${C_RESET}"; exit 1; }
cp keystrike "$INSTALL_DIR"
cp key_library.json "$LIB_DIR/"
chmod +x "$INSTALL_DIR/keystrike"
rm -f keystrike key_library.json install.sh
if [[ -n "$ORIGINAL_VERSION" ]]; then
    [[ "$ORIGINAL_VERSION" != "$VERSION" ]] && echo -e "${C_GREEN}[✓] KeyStrike update completed successfully${C_RESET}"
else
    echo -e "${C_GREEN}[✓] KeyStrike installed successfully${C_RESET}"
    echo -e "${C_CYAN}[*] Run: keystrike${C_RESET}"
fi
