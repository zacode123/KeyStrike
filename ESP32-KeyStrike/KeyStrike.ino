// ==========================================================
// KeyStrike - BLE HID Keyboard Emulation Tool
// ==========================================================
// Copyright (c) 2026 zacode123
//
// MIT License
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom
// the Software is furnished to do so, subject to the following
// conditions:
//
// The above copyright notice and this permission notice shall
// be included in all copies or substantial portions of the
// Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
// OR OTHER DEALINGS IN THE SOFTWARE.
// ==========================================================

#ifndef WIFI_SSID
  #define WIFI_SSID                        "WIFI_SSID"
  #define WIFI_PASS                        "WIFI_PASS"
#endif

#define WAKEUP_BUTTON                       GPIO_NUM_0

#define LIGHT_SLEEP_TIMEOUT               (30*60*1000)  // 30 min
#define DEEP_SLEEP_TIMEOUT              (1*60*60*1000)  // 1 hour

#define VERSION                                 "v6.0"

#define C_RESET                              "\033[0m"
#define C_BOLD                               "\033[1m"
#define C_WHITE                             "\033[37m"
#define C_RED                               "\033[31m"
#define C_GREEN                             "\033[32m"
#define C_YELLOW                            "\033[33m"
#define C_BLUE                              "\033[34m"
#define C_CYAN                              "\033[36m"
#define C_MAGENTA                           "\033[35m"

#include <WiFi.h>
#include <nvs_flash.h>
#include <Preferences.h>
#include <KeyStrike.h>

void          logPrintln(String msg);
void          onPassKey(uint32_t passkey);
void          initWiFi();
void          initBLE();
String        fit(String s);
String        line(String s);
String        readableBytes(uint64_t bytes);
const char*   flashModeToString(FlashMode_t mode);
const char*   resetReasonToString(esp_reset_reason_t reason);
String        macToString(uint64_t mac);
void          printSysInfo();
bool          extractQuoted(String &src, String &a, String &b);
void          handleCMD(String data);

bool          bleRunning           = false;
bool          passkeyPending       = false;
uint32_t      passkeyTimestamp     = 0;
bool          awaitingResetConfirm = false;
unsigned long lastActivity         = 0;

Preferences   prefs;
KeyStrike     keystrike;
WiFiServer    server(80);
WiFiClient    activeClient;

void logPrintln(String msg) {
  Serial.println(msg);
  if (activeClient && activeClient.connected()) activeClient.println(msg);
}

void onPassKey(uint32_t passkey) {
  String passkeyStr = String(passkey);
  while (passkeyStr.length() < 6) passkeyStr = "0" + passkeyStr;
  logPrintln("");
  logPrintln(C_CYAN "╔══════════════════════════════════════════════════╗" C_RESET);
  logPrintln(line(C_BOLD C_MAGENTA "               BLE PASSKEY REQUEST"));
  logPrintln(C_CYAN "╠══════════════════════════════════════════════════╣" C_RESET);
  logPrintln(line("                   Code: " + passkeyStr));
  logPrintln(line(""));
  logPrintln(line("        Verify the passkey on your device"));
  logPrintln(line(""));
  logPrintln(line("                 To approve, use:"));
  logPrintln(line("                   BLE PASSKEY APPROVE"));
  logPrintln(line(""));
  logPrintln(line("                  To deny, use:"));
  logPrintln(line("                     BLE PASSKEY DENY"));
  logPrintln(line(""));
  logPrintln(line("          Auto-approves after 30 seconds"));
  logPrintln(C_CYAN "╚══════════════════════════════════════════════════╝" C_RESET);
  logPrintln("");
  passkeyPending = true;
  passkeyTimestamp = millis();
}

void initWiFi() {
  String ssid = prefs.getString("wifi_ssid", WIFI_SSID);
  String pass = prefs.getString("wifi_pass", WIFI_PASS);
  delay(3000);
  Serial.println(C_BLUE "[*] Connecting to WiFi..." C_RESET);
  WiFi.begin(ssid.c_str(), pass.c_str());
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500);
    Serial.print(C_YELLOW "." C_RESET);
    retries++;
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(C_RED "\n[!] WiFi Failed!" C_RESET);
  } else {
    Serial.println(C_GREEN "\n[✓] Connected!" C_RESET);
    Serial.print(C_CYAN "[*] IP: " C_RESET);
    Serial.println(WiFi.localIP().toString());
  }
}

void initBLE() {
  String name = prefs.getString("name", "KeyStrike " VERSION);
  String manuf = prefs.getString("manuf", "zacode123");
  int secMode = prefs.getInt("sec_type", 0); 
  keystrike = KeyStrike(name.c_str(), manuf.c_str());
  if (secMode == 0) {
    logPrintln(C_YELLOW "[!] Security: NONE (Just Works)" C_RESET);
    keystrike.setSecurityMode(KeyStrikeSecurity::JustWorks); 
  } else {
    logPrintln(C_CYAN "[*] Security: PASSKEY" C_RESET);
    keystrike.setSecurityMode(KeyStrikeSecurity::Passkey);
    keystrike.setPasskeyCallback(onPassKey);
  }
  keystrike.begin();
  bleRunning = true;
  logPrintln(C_GREEN "[✓] BLE Started" C_RESET);
}

void setup() {
  Serial.begin(115200);
  pinMode(WAKEUP_BUTTON, INPUT_PULLUP);
  Serial.println(F(
    "\n\n  _  __             ____   _          _  _\n"
    " | |/ / ___  _   _ / ___| | |_  _ __ (_)| | __ ___\n"
    " | ' / / _ \\| | | |\\___ \\ | __|| '__|| || |/ // _ \\\n"
    " | . \\|  __/| |_| | ___) || |_ | |   | ||   <|  __/\n"
    " |_|\\_\\\\___| \\__, ||____/  \\__||_|   |_||_|\\_\\\\___|\n"
    "             |___/\n"
    "\n          BLE HID Keyboard Emulation Tool\n"
    "            ---------- " VERSION " ----------\n"
  ));
  esp_sleep_wakeup_cause_t reason = esp_sleep_get_wakeup_cause();
  if (reason == ESP_SLEEP_WAKEUP_EXT0) {
    Serial.println(C_CYAN "[BOOT] Woke from deep sleep — reconnecting..." C_RESET);
  } else {
    Serial.println(C_CYAN "[BOOT] Cold boot — starting." C_RESET);
  }
  prefs.begin("sys", false);
  initWiFi();
  initBLE();
  server.begin();
  Serial.println(C_BLUE "[*] TCP Server started" C_RESET);
  lastActivity = millis();
}

void loop() {
  WiFiClient client = server.available();
  if (client && client.connected()) {
    activeClient = client;
    Serial.println(C_GREEN "[+] Client connected" C_RESET);
    while (client.connected()) {
      if (client.available()) {
        String data = client.readStringUntil('\n');
        data.trim();
        Serial.print(C_CYAN "[TCP] " C_RESET);
        Serial.println(data);
        handleCMD(data);
      }
    }
    client.stop();
    Serial.println(C_RED "[-] Client disconnected" C_RESET);
  }
  if (passkeyPending && millis() - passkeyTimestamp > 30000) {
    logPrintln(C_GREEN "[AUTO] Passkey auto-approved after 30s" C_RESET);
    keystrike.confirmPasskey(true);
    passkeyPending = false;
  }
  if (millis() - lastActivity > LIGHT_SLEEP_TIMEOUT) {
    logPrintln(C_YELLOW "[POWER] Light Sleep" C_RESET);
    keystrike.beforeSleep();
    esp_sleep_enable_ext0_wakeup(WAKEUP_BUTTON, 0);
    esp_sleep_enable_timer_wakeup(DEEP_SLEEP_TIMEOUT - LIGHT_SLEEP_TIMEOUT - 102);
    delay(100);
    esp_light_sleep_start();
    esp_sleep_wakeup_cause_t reason = esp_sleep_get_wakeup_cause();
    if (reason == ESP_SLEEP_WAKEUP_TIMER) {
      logPrintln(C_RED "[POWER] Deep Sleep" C_RESET);
      esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
      delay(100);
      esp_deep_sleep_start();
    }
    keystrike.afterWake();
    logPrintln(C_GREEN "[POWER] Wake" C_RESET);
    lastActivity = millis();
  }
}

String fit(String s) {
    int visibleLen = 0;
    bool esc = false;
    for (char c : s) {
        if (c == '\033') {
            esc = true;
        } else if (esc && c == 'm') {
            esc = false;
        } else if (!esc) {
            visibleLen++;
        }
    }
    if (visibleLen > 50) {
        return s.substring(0, 50);
    }
    int pad = 50 - visibleLen;
    for (int i = 0; i < pad; i++) {
        s += ' ';
    }
    return s;
}

String line(String s) { return C_CYAN "║" + fit(s) + C_CYAN + "║" C_RESET; }

String readableBytes(uint64_t bytes) {
  const char* units[] = {"B", "KB", "MB", "GB"};
  double value = (double)bytes;
  int unit = 0;
  while (value >= 1024.0 && unit < 3) {
    value /= 1024.0;
    unit++;
  }
  char buf[32];
  if (unit == 0) {
    snprintf(buf, sizeof(buf), "%llu %s", (unsigned long long)bytes, units[unit]);
  } else {
    snprintf(buf, sizeof(buf), "%.2f %s", value, units[unit]);
  }
  return String(buf);
}

const char* flashModeToString(FlashMode_t mode) {
  switch (mode) {
    case FM_QIO:       return "QIO";
    case FM_QOUT:      return "QOUT";
    case FM_DIO:       return "DIO";
    case FM_DOUT:      return "DOUT";
    case FM_FAST_READ: return "FAST_READ";
    case FM_SLOW_READ: return "SLOW_READ";
    default:           return "UNKNOWN";
  }
}

const char* resetReasonToString(esp_reset_reason_t reason) {
  switch (reason) {
    case ESP_RST_POWERON:   return "Power on";
    case ESP_RST_EXT:       return "External reset";
    case ESP_RST_SW:        return "Software reset";
    case ESP_RST_PANIC:     return "Panic / exception";
    case ESP_RST_INT_WDT:   return "Interrupt watchdog";
    case ESP_RST_TASK_WDT:  return "Task watchdog";
    case ESP_RST_WDT:       return "Watchdog";
    case ESP_RST_DEEPSLEEP: return "Wake from deep sleep";
    case ESP_RST_BROWNOUT:  return "Brownout";
    case ESP_RST_SDIO:      return "SDIO reset";
    default:                return "Unknown";
  }
}

String macToString(uint64_t mac) {
  char buf[32];
  snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X", (uint8_t)(mac >> 40), (uint8_t)(mac >> 32), (uint8_t)(mac >> 24), (uint8_t)(mac >> 16), (uint8_t)(mac >> 8), (uint8_t)(mac));
  return String(buf);
}

void printSysInfo() {
  uint32_t flashHz  = ESP.getFlashChipSpeed();
  uint32_t flashMHz = flashHz / 1000000;
  unsigned long s = millis() / 1000;
  String uptime = (s/3600 ? String(s/3600)+" hour " : "") + (s/60%60 ? String(s/60%60)+" min " : "") + (s%60 ? String(s%60)+" sec" : "");
  if (uptime == "") uptime = "0 sec";
  logPrintln("");
  logPrintln(C_CYAN "╔══════════════════════════════════════════════════╗" C_RESET);
  logPrintln(line(C_BOLD C_MAGENTA "                 ESP32 INFORMATION"));
  logPrintln(C_CYAN "╠══════════════════════════════════════════════════╣" C_RESET);
  logPrintln(line(C_YELLOW "    SYSTEM"));
  logPrintln(line(C_BLUE "        Chip Model    " C_CYAN ": " C_GREEN + String(ESP.getChipModel())));
  logPrintln(line(C_BLUE "        Chip Revision " C_CYAN ": " C_GREEN + String(ESP.getChipRevision())));
  logPrintln(line(C_BLUE "        Chip Cores    " C_CYAN ": " C_GREEN + String(ESP.getChipCores())));
  logPrintln(line(C_BLUE "        SDK Version   " C_CYAN ": " C_GREEN + String(ESP.getSdkVersion())));
  logPrintln(line(C_BLUE "        Core Version  " C_CYAN ": " C_GREEN + String(ESP.getCoreVersion())));
  logPrintln(line(C_BLUE "        CPU Freq      " C_CYAN ": " C_GREEN + String(ESP.getCpuFreqMHz()) + C_YELLOW " MHz"));
  logPrintln(line(C_BLUE "        Uptime        " C_CYAN ": " C_GREEN + uptime));
  logPrintln(line(C_BLUE "        Reset Reason  " C_CYAN ": " C_RED   + String(resetReasonToString(esp_reset_reason()))));
  logPrintln(line(C_BLUE "        eFuse MAC     " C_CYAN ": " C_GREEN + macToString(ESP.getEfuseMac())));
  logPrintln(C_CYAN "╠══════════════════════════════════════════════════╣" C_RESET);
  logPrintln(line(C_YELLOW "    MEMORY"));
  logPrintln(line(C_BLUE "        Free Heap     " C_CYAN ": " C_GREEN + readableBytes(ESP.getFreeHeap())));
  logPrintln(line(C_BLUE "        Min Free Heap " C_CYAN ": " C_GREEN + readableBytes(ESP.getMinFreeHeap())));
  logPrintln(line(C_BLUE "        Max Alloc     " C_CYAN ": " C_GREEN + readableBytes(ESP.getMaxAllocHeap())));
  logPrintln(line(C_BLUE "        Heap Size     " C_CYAN ": " C_GREEN + readableBytes(ESP.getHeapSize())));
  logPrintln(C_CYAN "╠══════════════════════════════════════════════════╣" C_RESET);
  logPrintln(line(C_YELLOW "    FLASH"));
  logPrintln(line(C_BLUE "        Flash Size    " C_CYAN ": " C_GREEN + readableBytes(ESP.getFlashChipSize())));
  logPrintln(line(C_BLUE "        Flash Speed   " C_CYAN ": " C_GREEN + String(flashMHz) + C_YELLOW " MHz"));
  logPrintln(line(C_BLUE "        Flash Freq    " C_CYAN ": " C_GREEN + String(ESP.getFlashFrequencyMHz()) + C_YELLOW " MHz"));
  logPrintln(line(C_BLUE "        Flash Source  " C_CYAN ": " C_GREEN + String(ESP.getFlashSourceFrequencyMHz()) + C_YELLOW " MHz"));
  logPrintln(line(C_BLUE "        Flash Divider " C_CYAN ": " C_GREEN + String(ESP.getFlashClockDivider())));
  logPrintln(line(C_BLUE "        Flash Mode    " C_CYAN ": " C_MAGENTA + String(flashModeToString(ESP.getFlashChipMode())) + C_GREEN " (" + String((int)ESP.getFlashChipMode()) + ")"));
  logPrintln(C_CYAN "╠══════════════════════════════════════════════════╣" C_RESET);
  logPrintln(line(C_YELLOW "    SKETCH"));
  logPrintln(line(C_BLUE "        Sketch Size   " C_CYAN ": " C_GREEN + readableBytes(ESP.getSketchSize())));
  logPrintln(line(C_BLUE "        Free OTA Space" C_CYAN ": " C_GREEN + readableBytes(ESP.getFreeSketchSpace())));
  logPrintln(C_CYAN "╚══════════════════════════════════════════════════╝" C_RESET);
  logPrintln("");
}

bool extractQuoted(String &src, String &a, String &b) {
  int i1 = src.indexOf('"');
  int i2 = src.indexOf('"', i1 + 1);
  if (i1 == -1 || i2 == -1) return false;
  a = src.substring(i1 + 1, i2);
  int i3 = src.indexOf('"', i2 + 1);
  int i4 = src.indexOf('"', i3 + 1);
  if (i3 != -1 && i4 != -1) b = src.substring(i3 + 1, i4);
  else b = "";
  return true;
}

void handleCMD(String data) {
  lastActivity = millis();
  if (awaitingResetConfirm) {
    if (data == "YES") {
      logPrintln(C_RED "[!] Confirmed. Wiping NVS..." C_RESET);
      prefs.end();
      nvs_flash_erase();
      nvs_flash_init();
      logPrintln(C_GREEN "[✓] Reset complete. Restarting..." C_RESET);
      delay(2000);
      ESP.restart();
    } else if (data == "NO") {
      logPrintln(C_GREEN "[*] Reset aborted. System safe." C_RESET);
      awaitingResetConfirm = false;
    } else {
      logPrintln(C_YELLOW "[?] Please type YES or NO to proceed.\n" C_RESET);
    }
    return;
  }
  if (data == "RESET") {
    awaitingResetConfirm = true;
    logPrintln("");
    logPrintln(C_RED "╔══════════════════════════════════════════════════╗" C_RESET);
    logPrintln(line(C_BOLD C_RED "             !!! FACTORY RESET !!!                "));
    logPrintln(C_RED "╠══════════════════════════════════════════════════╣" C_RESET);
    logPrintln(line(C_YELLOW "  Do you want to reset ALL data and settings?     "));
    logPrintln(line(C_YELLOW "  This action CANNOT be undone.                   "));
    logPrintln(line(""));
    logPrintln(line(C_WHITE "  Type " C_GREEN "YES" C_WHITE " to confirm or " C_RED "NO" C_WHITE " to cancel.         "));
    logPrintln(C_RED "╚══════════════════════════════════════════════════╝" C_RESET);
    return;
  }
  if (data.startsWith("BLE")) {
    int i = data.indexOf(' ');
    if (i == -1) {
      logPrintln(C_YELLOW "Supported BLE options : INFO / INIT / ( END / STOP ) / POWER / SECURITY / PASSKEY / CLEARBONDS / RESET" C_RESET);
      return;
    }
    String cmd = data.substring(i + 1);
    cmd.trim();
    if (cmd.startsWith("INFO")) {
      int s = cmd.indexOf(' ');
      if (s == -1) {
        logPrintln(""); logPrintln(C_CYAN "╔══════════════════════════════════════════════════╗" C_RESET);
        logPrintln(line(C_BOLD C_MAGENTA "                  BLE INFORMATION"));
        logPrintln(C_CYAN "╠══════════════════════════════════════════════════╣" C_RESET);
        logPrintln(line(String(" ") + C_YELLOW "Device Name  : " + C_WHITE + prefs.getString("name", "KeyStrike " VERSION)));
        logPrintln(line(String(" ") + C_YELLOW "Manufacturer : " + C_WHITE + prefs.getString("manuf", "zacode123")));
        String bleStatus = bleRunning ? String(C_GREEN "Running") : String(C_RED "Stopped");
        logPrintln(line(String(" ") + C_YELLOW "Status         : " + bleStatus));
        logPrintln(line(String(" ") + C_YELLOW "Connected      : " + String(keystrike.isConnected() ? C_GREEN "Yes" : C_RED "No")));
        logPrintln(line(String(" ") + C_YELLOW "Paired         : " + String(keystrike.isPaired() ? C_GREEN "Yes" : C_RED "No")));
        logPrintln(line(String(" ") + C_YELLOW "Address        : " + C_WHITE + NimBLEDevice::getAddress().toString().c_str()));
        logPrintln(line(String(" ") + C_YELLOW "MTU Size       : " + C_WHITE + String(NimBLEDevice::getMTU())));
        logPrintln(line(String(" ") + C_YELLOW "TX Power       : " + C_WHITE + String(NimBLEDevice::getPower())));
        logPrintln(line(String(" ") + C_YELLOW "Security       : " + C_WHITE + "Bonding/Encryption"));
        logPrintln(line(String(" ") + C_YELLOW "Security Mode  : " + C_WHITE + (prefs.getInt("sec_type", 0) ? "Passkey" : "None (Just Works)")));
        logPrintln(C_CYAN "╚══════════════════════════════════════════════════╝" C_RESET);
        logPrintln("");
        return;
      }
      String args = cmd.substring(s + 1);
      String name, manuf;
      if (!extractQuoted(args, name, manuf)) {
        logPrintln(C_YELLOW "Usage: BLE INFO <NAME> <MANUFACTURER>" C_RESET);
        return;
      }
      prefs.putString("name", name);
      if (manuf.length() > 0) {
        prefs.putString("manuf", manuf);
      }
      logPrintln(C_GREEN "[ACTION] BLE config saved, restarting..." C_RESET);
      delay(500);
      ESP.restart();
    }
    if (cmd == "INIT") {
      if (!bleRunning) {
        logPrintln(C_GREEN "[ACTION] Starting BLE" C_RESET);
        initBLE();
      }
      return;
    }
    if (cmd == "END" || cmd == "STOP") {
      if (bleRunning) {
        logPrintln(C_RED "[ACTION] Stopping BLE" C_RESET);
        keystrike.end();
        bleRunning = false;
        logPrintln(C_RED "[✓] BLE Stopped" C_RESET);
      }
      return;
    }
    if (cmd.startsWith("POWER")) {
      int s = cmd.indexOf(' ');
      String args = cmd.substring(s + 1);
      if (s == -1 || args.indexOf(' ') != -1) {
        logPrintln(C_RED "[ERR] Usage: BLE POWER <1-8>" C_RESET);
        return;
      }
      int level = args.toInt();
      if (level < 1 || level > 8) {
        logPrintln(C_RED "[ERR] Invalid TX Power Level!" C_RESET);
        logPrintln(C_YELLOW "Allowed levels: 1–8" C_RESET);
        return;
      }
      keystrike.setTxPower(level);
      int dbm = (level - 1) * 3 - 12;
      logPrintln(C_GREEN "[✓] TX Power set → Level " + String(level) + " (" + String(dbm) + " dBm)" C_RESET);
      return;
    }
    if (cmd.startsWith("SECURITY ")) {
      String mode = cmd.substring(9);
      mode.trim();
      if (mode == "PASSKEY") {
        prefs.putInt("sec_type", 1);
        logPrintln(C_GREEN "[ACTION] Security set to PASSKEY. Restarting..." C_RESET);
        delay(1000);
        ESP.restart();
      } else if (mode == "NONE") {
        prefs.putInt("sec_type", 0);
        logPrintln(C_YELLOW "[ACTION] Security set to NONE (Just Works). Restarting..." C_RESET);
        delay(1000);
        ESP.restart();
      } else {
        logPrintln(C_RED "[ERR] Usage: BLE SECURITY PASSKEY/NONE" C_RESET);
      }
      return;
    }
    if (cmd.startsWith("PASSKEY ")) {
      if (!passkeyPending) {
        logPrintln(C_RED "[ERR] No pending passkey request" C_RESET);
        return;
      }
      String action = cmd.substring(8);
      action.trim();
      if (action == "APPROVE") {
        logPrintln(C_GREEN "[✓] Passkey Approved" C_RESET);
        keystrike.confirmPasskey(true);
        passkeyPending = false;
      } else if (action == "DENY") {
        logPrintln(C_RED "[X] Passkey Denied" C_RESET);
        keystrike.confirmPasskey(false);
        passkeyPending = false;
      } else {
        logPrintln(C_YELLOW "Usage: PASSKEY APPROVE/DENY" C_RESET);
      }
      return;
    }
    if (cmd == "CLEARBONDS") {
      logPrintln(C_YELLOW "[Bonds] Clearing all stored bonds..." C_RESET);
      keystrike.clearBonds();
      logPrintln(C_GREEN "[Bonds] Done. Re-advertising — pair again with passkey." C_RESET);
      return;
    }
    if (cmd == "RESET") {
      prefs.remove("name");
      prefs.remove("manuf");
      logPrintln(C_GREEN "[ACTION] BLE reset, restarting..." C_RESET);
      delay(500);
      ESP.restart();
    }
    logPrintln(C_RED "Unknown BLE option : " + String(C_WHITE) + cmd + C_RESET);
    logPrintln(C_YELLOW "Supported BLE options : INFO / INIT / ( END / STOP ) / POWER / CLEARBONDS / RESET" C_RESET);
    return;
  }
  if (data.startsWith("WIFI")) {
    int i = data.indexOf(' ');
    if (i == -1) {
      logPrintln(C_YELLOW "Supported WIFI options : INFO / RESET" C_RESET);
      return;
    }
    String cmd = data.substring(i + 1);
    cmd.trim();
    if (cmd.startsWith("INFO")) {
      int s = cmd.indexOf(' ');
      if (s == -1) {
        logPrintln(""); logPrintln(C_CYAN "╔══════════════════════════════════════════════════╗" C_RESET);
        logPrintln(line(C_BOLD C_MAGENTA "                  WIFI INFORMATION"));
        logPrintln(C_CYAN "╠══════════════════════════════════════════════════╣" C_RESET);
        logPrintln(line(String(" ") + C_YELLOW "SSID        : " + C_WHITE + WiFi.SSID()));
        logPrintln(line(String(" ") + C_YELLOW "Password    : " + C_WHITE + prefs.getString("wifi_pass", WIFI_PASS)));
        String status = (WiFi.status() == WL_CONNECTED) ? String(C_GREEN "Connected") : String(C_RED "Disconnected");
        logPrintln(line(String(" ") + C_YELLOW "Status      : " + status));
        if (WiFi.status() == WL_CONNECTED) {
          logPrintln(line(String(" ") + C_YELLOW "IP Address  : " + C_WHITE + WiFi.localIP().toString()));
          logPrintln(line(String(" ") + C_YELLOW "Gateway     : " + C_WHITE + WiFi.gatewayIP().toString()));
          logPrintln(line(String(" ") + C_YELLOW "Subnet Mask : " + C_WHITE + WiFi.subnetMask().toString()));
          logPrintln(line(String(" ") + C_YELLOW "DNS         : " + C_WHITE + WiFi.dnsIP().toString()));
        }
        logPrintln(line(String(" ") + C_YELLOW "MAC Address : " + C_WHITE + WiFi.macAddress()));
        logPrintln(line(String(" ") + C_YELLOW "Hostname    : " + C_WHITE + WiFi.getHostname()));
        if (WiFi.status() == WL_CONNECTED) {
          logPrintln(line(String(" ") + C_YELLOW "RSSI        : " + C_WHITE + String(WiFi.RSSI()) + " dBm"));
          int rssi = WiFi.RSSI();
          String quality;
          if (rssi > -50) quality = C_GREEN "Excellent";
          else if (rssi > -60) quality = C_GREEN "Good";
          else if (rssi > -70) quality = C_YELLOW "Fair";
          else quality = C_RED "Weak";
          logPrintln(line(String(" ") + C_YELLOW "Signal Qual : " + quality));
        }
        logPrintln(line(String(" ") + C_YELLOW "BSSID       : " + C_WHITE + WiFi.BSSIDstr()));
        logPrintln(line(String(" ") + C_YELLOW "Channel     : " + C_WHITE + String(WiFi.channel())));
        logPrintln(C_CYAN "╚══════════════════════════════════════════════════╝" C_RESET);
        logPrintln("");
        return;
      }
      String args = cmd.substring(s + 1);
      String ssid, pass;
      if (!extractQuoted(args, ssid, pass)) {
        logPrintln(C_YELLOW "Usage: WIFI INFO <SSID> <PASS>" C_RESET);
        return;
      }
      prefs.putString("wifi_ssid", ssid);
      prefs.putString("wifi_pass", pass);
      logPrintln(C_GREEN "[ACTION] WiFi config saved, restarting..." C_RESET);
      delay(500);
      ESP.restart();
    }
    if (cmd == "RESET") {
      prefs.remove("wifi_ssid");
      prefs.remove("wifi_pass");
      logPrintln(C_GREEN "[ACTION] WiFi reset, restarting..." C_RESET);
      delay(500);
      ESP.restart();
    }
    logPrintln(C_RED "Unknown WIFI option : " + String(C_WHITE) + cmd + C_RESET);
    logPrintln(C_YELLOW "Supported WIFI options : INFO / RESET" C_RESET);
    return;
  }
  if (data == "SYSINFO") {
    printSysInfo();
    return;
  }
  if (data.startsWith("RESTART")) {
    logPrintln(C_RED "[ACTION] Restarting ESP" C_RESET);
    delay(2000);
    ESP.restart();
    return;
  }
  if (!keystrike.isConnected()) {
    logPrintln(C_RED "[BLE] No device connected!" C_RESET);
    return;
  } else {
    if (!keystrike.isPaired()) {
      logPrintln(C_RED "[BLE] Device pairing has not been completed yet!" C_RESET);
      return;
    } else {
      if (data.startsWith("TYPE ")) {
        logPrintln(C_GREEN "[ACTION] Typing text" C_RESET);
        keystrike.print(data.substring(5));
        return;
      }
      if (data.startsWith("RELEASE")) {
        String keyStr = data.substring(7);
        keyStr.trim();
        if (keyStr == "ALL") {
          logPrintln(C_GREEN "[ACTION] Release all keys" C_RESET);
          keystrike.releaseAll();
        } else {
          uint16_t key = strtol(keyStr.c_str(), NULL, 16);
          logPrintln(C_GREEN "[ACTION] Release key" C_RESET); 
          if (key <= 0xFF) {
            keystrike.release((uint8_t)key);
          } else {
            keystrike.release((uint16_t)key);
          }
        }
        return;
      }
      int sp1 = data.indexOf(' ');
      if (sp1 > 0) {
        String action = data.substring(0, sp1);
        int sp2 = data.indexOf(' ', sp1 + 1);
        int sp3 = data.indexOf(' ', sp2 + 1);
        String usageStr;
        String keyStr;
        String modStr = "";
        if (sp2 > 0) {
          usageStr = data.substring(sp1 + 1, sp2);
          if (sp3 > 0) {
            keyStr = data.substring(sp2 + 1, sp3);
            modStr = data.substring(sp3 + 1);
          } else {
            keyStr = data.substring(sp2 + 1);
          }
        } else {
          return;
        }
        uint16_t usageId = strtol(usageStr.c_str(), NULL, 16);
        uint16_t key     = strtol(keyStr.c_str(), NULL, 16);
        uint8_t  mod     = 0;
        if (modStr.length() > 0) {
          mod = strtol(modStr.c_str(), NULL, 16);
        }
        if (action == "PRESS") {
          logPrintln(C_GREEN "[ACTION] Press key" C_RESET);
          if (usageId == 0x0C) {
            keystrike.press((uint8_t)key, mod);
          } else {
            if (mod > 0) {
              logPrintln(C_RED "[WARN] Modifier will be ignored in Consumer / Media keys" C_RESET);
            }
            keystrike.press((uint16_t)key);
          }
        }
        if (action == "TAP") {
          logPrintln(C_GREEN "[ACTION] Tap key" C_RESET);
          if (usageId == 0x07) {
            keystrike.tap((uint8_t)key, mod);
          } else {
            if (mod > 0) {
              logPrintln(C_RED "[WARN] Modifier will be ignored in Consumer / Media keys" C_RESET);
            }
            keystrike.tap((uint16_t)key);
          }
        }
        return;
      }
    }
  }
  logPrintln(C_RED "[!] Unknown command" C_RESET);
}
