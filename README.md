<div align="center">
    <img width="512" height="279" alt="KeyStrike Banner" src="https://github.com/user-attachments/assets/799ecca7-1b43-477b-8815-0d8cc31c902e"/>
    <h1>⚡ KeyStrike v6.0</h1>
    <p><b>The Ultimate ESP32-S3/C3 BLE HID Scripting Engine</b></p>
</div>

---

### 🚀 What's New in v6.0 (The Logic Engine)
Version 6.0 transforms KeyStrike from a simple injector into a full-blown automation platform.
 * **🧠 Logic Engine:** Support for IF/ELSE conditionals, FOR loops, and WHILE loops directly in your payloads.
 * **📦 Modular Functions:** Define FUNCTION name()...END blocks to reuse code and keep payloads clean.
 * **🎨 Pro Logging:** New LOG_INFO, LOG_OK, and LOG_ERR commands with ANSI escape support (\n, \t, \e) and beautiful color gradients.
 * **📑 Dynamic Variables:** Capture system info into variables (e.g., $MY_IP = [SYSINFO]) or use native $VAR = ... syntax.
 * **📚 Expanded Library:** Over 140+ HID keys including media, brightness, and OS-specific app launchers.

---

### ⚙️ Core Features
 * **Wireless HID Emulation:** Full Bluetooth Low Energy (BLE) keyboard support.
 * **Ducky-Plus Syntax:** Familiar Ducky Script base extended with logic and variables.
 * **Cross-Platform CLI:** Native support for **Linux** and **Termux** (Android).
 * **Zero-Footprint Cleanup:** Automated memory management that clears variables and functions on exit.
 * **Persistent Config:** Remembers your ESP32 IP and settings across sessions.

---
 
### 📦 Installation & Setup
**🔹 Quick Install (Termux & Linux)**
```bash
curl -fsL https://raw.githubusercontent.com/zacode123/KeyStrike/main/install.sh | bash
```

---

**🔹 Usage**
| Command | Description |
|---|---|
| `keystrike` | Launch interactive shell / Run payload |
| `keystrike -v` | Check current version |
| `keystrike update` | Pull the latest update |
| `keystrike uninstall` | Complete removal of KeyStrike |

---

### 📜 Scripting Example
KeyStrike v6.0 allows you to write advanced payloads like this:
```bash
REM Define a function
FUNCTION OPEN_TERMINAL()
    TAP LWIN R
    DELAY 500
    STRING "cmd"
    TAP ENTER
END

OPEN_TERMINAL()
DELAY 1000
STRINGLN "echo KeyStrike v6.0 was here."

LOG_OK "Payload Finished at $_TIME"
```

---

### 🛠 Technical Specifications
 * **Standard:** HID over GATT (HOGP) v1.22
 * **Backend:** Bash 4.0+ / Python 3.x
 * **Hardware:** ESP32, ESP32-S3, or ESP32-C3
 * **Transmission:** 16-bit Consumer Reports & 8-bit Keyboard Boot Reports

---

### ⚠️ Safety Notice
 * This project is created for **educational and research purposes only**.
 * The developer does not take responsibility for any misuse.
 * Always obtain permission before testing on devices you do not own.

---

### 🤝 Contributing
KeyStrike is an evolving platform. We welcome contributions to:
 * **ESP32 Firmware:** Improving connection stability and latency.
 * **Key Modules:** Mapping more HID codes for specific layouts (ISO/JIS).
 * **Payloads:** Sharing creative automation scripts.

**Happy Hacking!** ⚡
