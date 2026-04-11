#!/bin/bash
set -e
VERSION="v4.2"
MODE="${1:-install}"
USR="${PREFIX:-/usr}"
INSTALL_DIR="$USR/bin"
LIB_DIR="$USR/share/keystrike"
REPO_URL="https://raw.githubusercontent.com/zacode123/KeyStrike/main"
C_RESET="\e[0m"
C_RED="\e[1;31m"
C_GREEN="\e[1;32m"
C_YELLOW="\e[1;33m"
C_CYAN="\e[1;36m"
[[ "$MODE" == "update" ]] && echo -e "${C_YELLOW}[*] Updating KeyStrike to $VERSION...${C_RESET}" || echo -e "${C_YELLOW}[*] Installing KeyStrike $VERSION...${C_RESET}"
mkdir -p "$INSTALL_DIR" "$LIB_DIR"
curl -sL "$REPO_URL/keystrike" -o keystrike
curl -sL "$REPO_URL/key_library.json" -o key_library.json
[[ ! -f keystrike || ! -f key_library.json ]] && { echo -e "${C_RED}[!] Download failed${C_RESET}"; exit 1; }
cp keystrike "$INSTALL_DIR/keystrike"
cp key_library.json "$LIB_DIR/"
chmod +x "$INSTALL_DIR/keystrike"
rm -f keystrike key_library.json install.sh
[[ "$MODE" == "update" ]] && echo -e "${C_GREEN}[✓] KeyStrike update completed successfully${C_RESET}" || {
    echo -e "${C_GREEN}[✓] KeyStrike installed successfully${C_RESET}"
    echo -e "${C_CYAN}[*] Run: keystrike${C_RESET}"
}
