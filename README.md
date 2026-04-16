<div align="center">
    <img width="512" height="279" alt="KeyStrike Banner" src="https://github.com/user-attachments/assets/799ecca7-1b43-477b-8815-0d8cc31c902e"/>
    <h1>⚡ KeyStrike v6.0</h1>
    <p><b>The Ultimate ESP32-S3/C3 BLE HID Scripting Engine</b></p>
</div>

---

### 🚀 What's New in v6.0 (The Logic Engine)
Version 6.0 transforms KeyStrike from a simple injector into a full-blown automation platform.
 * **Dual mode:** KeyStrike now supports two modes interactive mode (Ducky shell) and payload mode. Use `keystrike --help` for more info.
 * **Payload:** KeyStrike supports .ks(KeyStrike Script file) payloads. Also supports a `--loop <n>` option for running the payload in loop n times.
 * **🧠 Logic Engine:** Support for `IF/ELSEIF/ELSE` conditionals, `FOR` loops, and `WHILE` loops directly in your payloads.
 * **📦 Modular Functions:** Define `FUNC name()...END` blocks to reuse code and keep payloads clean.
 * **📑 Variables support:** Define a variable using `$A = "Hi"`, use it as `LOG_INFO "Value: $A"`.
 * **🎨 Pro Logging:** New LOG_INFO, LOG_OK, and LOG_ERR logging commands with beautiful color gradients. Supports control characters too (\n, \r, ...).
 * **📚 Expanded Library:** 140 HID keys including media, brightness, and OS-specific app launchers.

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

### 🧠 Advanced Firmware Features
The KeyStrike firmware is designed for stability and "stealth" in professional environments.
 * **Smart Power Management:** Integrated **Light & Deep Sleep** logic. The device stays active for 30 minutes, enters light sleep to save power, and transitions to deep sleep after 1 hour (Wake up via GPIO 0).
 * **Interactive ANSI Console:** Real-time feedback with high-visibility color-coded tables for system health, BLE state, and WiFi signal quality.
 * **Factory Reset System:** Robust data management using nvs_flash. Wipe all stored SSIDs and custom BLE configurations with a secure confirmation prompt.
 * **Dynamic Security Mode:** Change security mode any time. Supports 'PASSKEY' and 'None (Just Works)'.
 * **Auto-Passkey Approval:** Secure pairing with support for **Passkey Verification**. Includes a 30-second auto-approval window.
 * **Dynamic Configuration:** Change your ESP32 Configuration on the fly without re-flashing.

---

## 🔁 Loops

KeyStrike supports iterative execution using FOR and WHILE loops.


---

### 📌 FOR Loop

*Executes a block for a defined numeric range.*

**✔ Syntax**
```ks
FOR $VAR IN <start>..<end>
    <commands>
END
```
**✔ Example**
```ks
FOR $I IN 1..3
    LOG_INFO "Iteration: $I"
END
```

**✔ Notes**

* Range is inclusive (1..3 → runs 3 times)

* Loop variable ($I) is automatically incremented

* Supports use inside functions and nested blocks

---

### 📌 WHILE Loop

*Executes a block while a condition is true.*

**✔ Syntax**
```ks
WHILE "<condition>"
    <commands>
END
```
**✔ Example**
```ks
$X = 1

WHILE "$X" != "4"
    LOG_INFO "Value: $X"
    $X = EXEC expr $X + 1
END
```
**✔ Notes**

* Condition is evaluated before each iteration

* Loop continues until condition becomes false

* Variables must be updated manually inside the loop

---

### ⚡ For more Syntax References

* **Ducky syntax & commands** → `HELP | H | ?`

* **Key library** (key codes & usage) → `KEYS | K    HELP | H | ?`

* **KeyStrike CLI usage** → `keystrike -h` or `keystrike --help`

---

### 🔧 Tech Stack
 * **Framework:** Arduino ESP32 Core
 * **Library:** NimBLE-Arduino (Optimized for low memory footprint & MTU detection)
 * **Storage:** Preferences.h for non-volatile storage (NVS)
 * **Protocol:** TCP/IP Port 80 for remote payload injection

---

### 🛠 How to Flash
 1. Install the **ESP32** board in your Arduino IDE.
 2. Install the **NimBLE-Arduino**.
 3. Open the ESP32-KeyStrike folder.
 4. Set your WIFI_SSID and WIFI_PASS in the header.
 5. Upload to your ESP32-S3 / C3 / Classic.
    
---

### 🛠 Technical Specifications
 * **Standard:** HID over GATT (HOGP) v1.22
 * **Hardware:** ESP32, ESP32-S3, or ESP32-C3
 * **Transmission:** 16-bit Consumer Reports & 8-bit Keyboard Boot Reports

---

### ⚠️ Safety Notice
 * This project is created for **educational and research purposes only**.
 * The developer does not take responsibility for any misuse.
 * Always obtain permission before testing on other's devices.

---

### 🤝 Contributing
KeyStrike is an evolving platform. We welcome contributions to:
 * **ESP32 Firmware:** Improving connection stability and latency.
 * **Key Modules:** Mapping more HID codes for specific layouts (ISO/JIS).
 * **Payloads:** Sharing creative automation scripts.
