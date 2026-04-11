#!/bin/bash
set -e
VERSION="v4.2"
ORIGINAL_VERSION=$(keystrike -v | grep -oE 'v[0-9]+\.[0-9]+')
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
mkdir -p "$INSTALL_DIR" "$LIB_DIR"
curl -sL "$REPO_URL/keystrike" -o keystrike
curl -sL "$REPO_URL/key_library.json" -o key_library.json
[[ ! -f keystrike || ! -f key_library.json ]] && { echo -e "${C_RED}[!] Download failed${C_RESET}"; exit 1; }
cp keystrike "$INSTALL_DIR/keystrike"
cp key_library.json "$LIB_DIR/"
chmod +x "$INSTALL_DIR/keystrike"
rm -f keystrike key_library.json install.sh
[[ -n "$ORIGINAL_VERSION" ]] && echo -e "${C_GREEN}[✓] KeyStrike update completed successfully${C_RESET}" || {
    echo -e "${C_GREEN}[✓] KeyStrike installed successfully${C_RESET}"
    echo -e "${C_CYAN}[*] Run: keystrike${C_RESET}"
}
