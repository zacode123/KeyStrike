<div align="center">
  <img width="512" height="279" alt="image_4750e815-88d2-4990-93fa-3b9eb9e1002d" src="https://github.com/user-attachments/assets/799ecca7-1b43-477b-8815-0d8cc31c902e"/>
  <h1>⚡ KeyStrike v4.2</h1>
</div>

> **ESP32 BLE HID keyboard emulation tool.**

---

### 📌 Description
**KeyStrike** is an ESP32-based BLE HID keyboard emulation tool that allows wireless keystroke injection using Bluetooth Low Energy.
It provides a CLI interface for Linux and Termux, enabling HID Keyboard Emulation through BLE and an enormous key library.

### ⚙️ Features
- ESP32 BLE HID Keyboard Emulation.
- CLI-based control system.
- Key injection.
- Huge Key library with Android and desktop controls. (currently IOS is not supported)
- Lightweight and fast installer.
- Linux & Termux support.
- Update system via script.

---

### 📦 Installation
**🔹 Termux / Android**
```bash
apt update  
apt upgrade -y  
apt install curl -y  

curl -sL http://github.com/install.sh | bash
```
**🔹 Linux**
```bash
sudo apt update  
sudo apt upgrade -y  
sudo apt install curl -y  

curl -sL http://your-link/install.sh | bash
```

---

### 🚀 Commands
**Run Tool**
```bash
keystrike

```
**🔄 Update**
```bash
keystrike update
```
**🗑 Uninstall**
```bash
keystrike uninstall
```

---

### ⚠️ Safety Notice
- This project is created for educational and research purposes only.
- It is not intended for malicious use.
- The developer does not take responsibility for any misuse of this tool.
- Users are fully responsible for how they use this software.
- Unauthorized or harmful use of this tool is strictly discouraged.

---

### 🤝 Contribution
Contributions are welcome! You can:
 * 🐛 **Report bugs**
 * 💡 **Suggest features**
 * 🛠 **Improve code**
 * 🧩 **Add new key modules**
 * ⚙️ **Improve ESP32 firmware**

### **Steps to contribute:**
 1. Fork repository
 2. Create feature branch
 3. Commit changes
 4. Push and open pull request
