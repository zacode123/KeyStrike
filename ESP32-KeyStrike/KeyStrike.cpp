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

#include "KeyStrike.h"

static const uint8_t _hidReportDescriptor[] = {
    0x05, 0x01, 0x09, 0x06, 0xA1, 0x01, 0x85, 0x01,
    0x05, 0x07, 0x19, 0xE0, 0x29, 0xE7, 0x15, 0x00,
    0x25, 0x01, 0x75, 0x01, 0x95, 0x08, 0x81, 0x02,
    0x95, 0x01, 0x75, 0x08, 0x81, 0x01, 0x95, 0x06,
    0x75, 0x08, 0x15, 0x00, 0x26, 0xE7, 0x00, 0x05,
    0x07, 0x19, 0x00, 0x2A, 0xE7, 0x00, 0x81, 0x00,
    0xC0, 0x05, 0x0C, 0x09, 0x01, 0xA1, 0x01, 0x85,
    0x02, 0x15, 0x00, 0x26, 0xFF, 0x03, 0x19, 0x00,
    0x2A, 0xFF, 0x03, 0x75, 0x10, 0x95, 0x01, 0x81,
    0x00, 0xC0
};

static const uint8_t _kcTable[95] = {
    KEY_SPACE, KEY_1, KEY_APOSTROPHE, KEY_3, KEY_4, KEY_5, KEY_7,
    KEY_APOSTROPHE, KEY_9, KEY_0, KEY_8, KEY_EQUAL, KEY_COMMA,
    KEY_MINUS, KEY_DOT, KEY_SLASH, KEY_0, KEY_1, KEY_2, KEY_3,
    KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_SEMICOLON,
    KEY_SEMICOLON, KEY_COMMA, KEY_EQUAL, KEY_DOT, KEY_SLASH,
    KEY_2, KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H,
    KEY_I, KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q,
    KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,
    KEY_LEFTBRACE, KEY_BACKSLASH, KEY_RIGHTBRACE, KEY_6, KEY_MINUS,
    KEY_GRAVE, KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G,
    KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P,
    KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y,
    KEY_Z, KEY_LEFTBRACE, KEY_BACKSLASH, KEY_RIGHTBRACE, KEY_GRAVE,
};

static const uint8_t _modTable[95] = {
    0, KEY_MOD_LSHIFT, KEY_MOD_LSHIFT, KEY_MOD_LSHIFT, KEY_MOD_LSHIFT,
    KEY_MOD_LSHIFT, KEY_MOD_LSHIFT, 0, KEY_MOD_LSHIFT, KEY_MOD_LSHIFT,
    KEY_MOD_LSHIFT, KEY_MOD_LSHIFT, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, KEY_MOD_LSHIFT, 0, KEY_MOD_LSHIFT, 0, KEY_MOD_LSHIFT, KEY_MOD_LSHIFT,
    KEY_MOD_LSHIFT, KEY_MOD_LSHIFT, KEY_MOD_LSHIFT, KEY_MOD_LSHIFT,
    KEY_MOD_LSHIFT, KEY_MOD_LSHIFT, KEY_MOD_LSHIFT, KEY_MOD_LSHIFT,
    KEY_MOD_LSHIFT, KEY_MOD_LSHIFT, KEY_MOD_LSHIFT, KEY_MOD_LSHIFT,
    KEY_MOD_LSHIFT, KEY_MOD_LSHIFT, KEY_MOD_LSHIFT, KEY_MOD_LSHIFT,
    KEY_MOD_LSHIFT, KEY_MOD_LSHIFT, KEY_MOD_LSHIFT, KEY_MOD_LSHIFT,
    KEY_MOD_LSHIFT, KEY_MOD_LSHIFT, KEY_MOD_LSHIFT, KEY_MOD_LSHIFT,
    KEY_MOD_LSHIFT, KEY_MOD_LSHIFT, KEY_MOD_LSHIFT, 0, 0, 0, KEY_MOD_LSHIFT,
    KEY_MOD_LSHIFT, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, KEY_MOD_LSHIFT, KEY_MOD_LSHIFT, KEY_MOD_LSHIFT,
    KEY_MOD_LSHIFT,
};

void KeyStrike_Internal::ServerCallbacks::onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) {
    _parent->_onConnect(connInfo.getConnHandle());
}
void KeyStrike_Internal::ServerCallbacks::onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) {
    (void)reason;
    _parent->_onDisconnect();
}
void KeyStrike_Internal::ServerCallbacks::onAuthenticationComplete(NimBLEConnInfo& connInfo) {
    _parent->_onAuthComplete(connInfo.isEncrypted());
}
void KeyStrike_Internal::ServerCallbacks::onConfirmPassKey(NimBLEConnInfo& connInfo, uint32_t pass_key) {
    _parent->_onConfirmPassKey(pass_key);
}

void KeyStrike::_idleTimerCallback(TimerHandle_t xTimer) {
    KeyStrike* self = static_cast<KeyStrike*>(pvTimerGetTimerID(xTimer));
    if (!self) return;
    self->_pendingIdleTransition = true;
    self->_connState = _ConnState::Idle;
    self->_reportPrimingNeeded = true;
}

KeyStrike::KeyStrike(const char* deviceName, const char* manufName)
    : _deviceName(deviceName ? deviceName : "KeyStrike KB"),
      _manufName(manufName ? manufName : "zacode123"),
      _batteryLevel(100),
      _secMode(KeyStrikeSecurity::JustWorks), _state(_BLEState::Stopped),
      _connected(false), _authenticated(false), _consumerActive(false),
      _reportPrimingNeeded(true), _lastReportMs(0), _txPowerLevel(8),
      _connState(_ConnState::Disconnected), _connHandle(BLE_HS_CONN_HANDLE_NONE),
      _idleTimer(nullptr), _pendingIdleTransition(false), _cbPassKey(nullptr),
      _pServer(nullptr), _pHID(nullptr), _pKeyboardInput(nullptr),
      _pConsumerInput(nullptr), _pServerCb(nullptr) {
    memset(_keyReport, 0, sizeof(_keyReport));
}

void KeyStrike::begin() {
    if (_state == _BLEState::Running) return;
    if (NimBLEDevice::isInitialized()) {
        setTxPower(_txPowerLevel);
        NimBLEDevice::startAdvertising();
        _state = _BLEState::Running;
        return;
    }
    NimBLEDevice::init(_deviceName);
    uint32_t t = millis();
    while (!NimBLEDevice::isInitialized()) {
        delay(10);
        if (millis() - t > 5000) return;
    }
    setTxPower(_txPowerLevel);
    if (_secMode == KeyStrikeSecurity::Passkey) {
        NimBLEDevice::setSecurityAuth(BLE_SM_PAIR_AUTHREQ_BOND | BLE_SM_PAIR_AUTHREQ_MITM | BLE_SM_PAIR_AUTHREQ_SC);
        NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_YESNO);
    } else {
        NimBLEDevice::setSecurityAuth(BLE_SM_PAIR_AUTHREQ_BOND | BLE_SM_PAIR_AUTHREQ_SC);
        NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);
    }
    NimBLEDevice::setSecurityInitKey(BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID);
    NimBLEDevice::setSecurityRespKey(BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID);
    _pServer = NimBLEDevice::createServer();
    if (!_pServerCb) _pServerCb = new KeyStrike_Internal::ServerCallbacks(this);
    _pServer->setCallbacks(_pServerCb);
    _pHID = new NimBLEHIDDevice(_pServer);
    _pHID->setManufacturer(_manufName);
    _pHID->setPnp(0x01, 0x02E5, 0x0001, 0x0100);
    _pHID->setHidInfo(0x00, 0x01);
    _pHID->setReportMap((uint8_t*)_hidReportDescriptor, sizeof(_hidReportDescriptor));
    _pKeyboardInput = _pHID->getInputReport(HID_REPORT_ID_KEYBOARD);
    _pConsumerInput = _pHID->getInputReport(HID_REPORT_ID_CONSUMER);
    _pHID->setBatteryLevel(_batteryLevel);
    _pHID->startServices();
    _idleTimer = xTimerCreate("KBIdle", pdMS_TO_TICKS(HID_IDLE_THRESHOLD_MS), pdFALSE, this, _idleTimerCallback);
    NimBLEAdvertising* pAdv = NimBLEDevice::getAdvertising();
    pAdv->setAppearance(0x03C1);
    pAdv->addServiceUUID(_pHID->getHidService()->getUUID());
    pAdv->addServiceUUID(_pHID->getBatteryService()->getUUID());
    pAdv->setPreferredParams(0x10, 0x20);
    pAdv->enableScanResponse(true);
    pAdv->setMinInterval(0x20);
    pAdv->setMaxInterval(0x40);
    NimBLEAdvertisementData scanResponse;
    scanResponse.setName(_deviceName);
    pAdv->setScanResponseData(scanResponse);
    NimBLEDevice::startAdvertising();
    _state = _BLEState::Running;
}

void KeyStrike::end() {
    if (_state != _BLEState::Running) return;
    _stopIdleTimer();
    _pendingIdleTransition = false;
    _connState = _ConnState::Disconnected;
    _connHandle = BLE_HS_CONN_HANDLE_NONE;
    _state = _BLEState::Stopped;
    if (_connected && _pServer != nullptr) {
        auto peers = _pServer->getPeerDevices();
        for (auto& handle : peers) _pServer->disconnect(handle);
        delay(150);
    }
    NimBLEDevice::stopAdvertising();
    _connected = false;
    _authenticated = false;
    memset(_keyReport, 0, sizeof(_keyReport));
    _consumerActive = false;
    _reportPrimingNeeded = true;
}

bool KeyStrike::isConnected() const { return _connected; }
bool KeyStrike::isPaired() const { return _authenticated; }

void KeyStrike::clearBonds() {
    int numBonds = NimBLEDevice::getNumBonds();
    for (int i = numBonds - 1; i >= 0; i--) {
        NimBLEDevice::deleteBond(NimBLEDevice::getBondedAddress(i));
    }
}

void KeyStrike::setSecurityMode(KeyStrikeSecurity mode) { _secMode = mode; }
void KeyStrike::setPasskeyCallback(void (*cb)(uint32_t passkey)) { _cbPassKey = cb; }

void KeyStrike::confirmPasskey(bool confirm) {
    if (_connHandle != BLE_HS_CONN_HANDLE_NONE && _pServer != nullptr) {
        NimBLEConnInfo peerInfo = _pServer->getPeerInfo(_connHandle);
        NimBLEDevice::injectConfirmPasskey(peerInfo, confirm);
    }
}

void KeyStrike::setTxPower(uint8_t level) {
    if (level < 1) level = 1;
    else if (level > 8) level = 8;
    static const esp_power_level_t pwrMap[8] = {
        ESP_PWR_LVL_N12, ESP_PWR_LVL_N9, ESP_PWR_LVL_N6, ESP_PWR_LVL_N3,
        ESP_PWR_LVL_N0, ESP_PWR_LVL_P3, ESP_PWR_LVL_P6, ESP_PWR_LVL_P9
    };
    _txPowerLevel = level;
    if (NimBLEDevice::isInitialized()) NimBLEDevice::setPower(pwrMap[level - 1]);
}

void KeyStrike::setBatteryLevel(uint8_t level) {
    _batteryLevel = (level > 100) ? 100 : ((level == 0) ? 1 : level);
    if (_pHID != nullptr) _pHID->setBatteryLevel(_batteryLevel, true);
}

void KeyStrike::_updateConnParams(uint16_t minInterval, uint16_t maxInterval, uint16_t latency, uint16_t timeout) {
    if (_connHandle == BLE_HS_CONN_HANDLE_NONE) return;
    ble_gap_upd_params params;
    params.itvl_min = minInterval;
    params.itvl_max = maxInterval;
    params.latency = latency;
    params.supervision_timeout = timeout;
    params.min_ce_len = BLE_GAP_INITIAL_CONN_MIN_CE_LEN;
    params.max_ce_len = BLE_GAP_INITIAL_CONN_MAX_CE_LEN;
    ble_gap_update_params(_connHandle, &params);
}

void KeyStrike::_transitionToActive() {
    _connState = _ConnState::Active;
    _pendingIdleTransition = false;
    _updateConnParams(HID_CONN_INTERVAL, HID_CONN_INTERVAL, 0, HID_CONN_TIMEOUT);
    _startIdleTimer();
}

void KeyStrike::_startIdleTimer() { if (_idleTimer) xTimerReset(_idleTimer, 0); }
void KeyStrike::_stopIdleTimer() { if (_idleTimer) xTimerStop(_idleTimer, 0); }

void KeyStrike::press(uint8_t keycode, uint8_t modifiers) {
    if (!_connected) return;
    if (_pendingIdleTransition) _transitionToActive();
    else if (_connState == _ConnState::Active) _startIdleTimer();
    if (_isModifier(keycode)) _keyReport[0] |= _keycodeToModBit(keycode);
    else _addKeycode(keycode);
    _keyReport[0] |= modifiers;
    _sendKeyReport();
}

void KeyStrike::release(uint8_t keycode) {
    if (!_connected) return;
    if (keycode == KEY_NONE) { releaseAll(); return; }
    if (_pendingIdleTransition) _transitionToActive();
    else if (_connState == _ConnState::Active) _startIdleTimer();
    if (_isModifier(keycode)) _keyReport[0] &= ~_keycodeToModBit(keycode);
    else _removeKeycode(keycode);
    _sendKeyReport();
}

void KeyStrike::press(uint16_t usageId) {
    if (!_connected) return;
    if (_pendingIdleTransition) _transitionToActive();
    else if (_connState == _ConnState::Active) _startIdleTimer();
    _consumerActive = true;
    _sendConsumerReport(usageId);
}

void KeyStrike::release(uint16_t usageId) {
    (void)usageId;
    if (!_connected) return;
    if (_pendingIdleTransition) _transitionToActive();
    else if (_connState == _ConnState::Active) _startIdleTimer();
    _consumerActive = false;
    _sendConsumerReport(0x0000);
}

void KeyStrike::releaseAll() {
    if (!_connected) return;
    if (_pendingIdleTransition) _transitionToActive();
    else if (_connState == _ConnState::Active) _startIdleTimer();
    memset(_keyReport, 0, sizeof(_keyReport));
    _sendKeyReport();
    if (_consumerActive) {
        _consumerActive = false;
        _sendConsumerReport(0x0000);
    }
}

void KeyStrike::tap(uint8_t keycode, uint8_t modifiers, uint16_t delayMs, uint16_t keyGap) {
    if (!_connected) return;
    uint8_t savedMods = _keyReport[0];
    press(keycode, modifiers);
    if(delayMs > 0) delay(delayMs); else delay(1);
    if (_isModifier(keycode)) _keyReport[0] &= ~_keycodeToModBit(keycode);
    else _removeKeycode(keycode);
    _keyReport[0] = savedMods;
    _sendKeyReport();
    if(keyGap > 0) delay(keyGap); else delay(1);
}

void KeyStrike::tap(uint16_t usageId, uint16_t delayMs, uint16_t keyGap) {
    if (!_connected) return;
    press(usageId);
    if(delayMs > 0) delay(delayMs); else delay(1);
    release(usageId);
    if(keyGap > 0) delay(keyGap); else delay(1);
}

size_t KeyStrike::write(uint8_t c) {
    if (!_connected) return 0;
    if (c == '\n' || c == '\r') { tap(KEY_RETURN); return 1; }
    if (c == '\t') { tap(KEY_TAB); return 1; }
    if (c == 0x08) { tap(KEY_BACKSPACE); return 1; }
    if (c == 0x1B) { tap(KEY_ESCAPE); return 1; }
    if (c >= 0x20 && c <= 0x7E) {
        uint8_t idx = c - 0x20;
        uint8_t keycode = _kcTable[idx];
        if (keycode != 0) {
            tap(keycode, _modTable[idx]);
            return 1;
        }
    }
    return 0;
}

size_t KeyStrike::write(const uint8_t* buffer, size_t size) {
    if (!_connected || size == 0) return 0;
    size_t written = 0;
    for (size_t i = 0; i < size; i++) written += write(buffer[i]);
    return written;
}

void KeyStrike::beforeSleep() {
    _stopIdleTimer();
    _pendingIdleTransition = false;
    releaseAll();
    _reportPrimingNeeded = true;
    _lastReportMs = 0;
}

void KeyStrike::afterWake() {
    uint32_t deadline = millis() + 15000;
    if (_authenticated) {
        while (_authenticated && millis() < deadline) delay(10);
        if (_authenticated) return;
    }
    while (!_authenticated && millis() < deadline) delay(100);
    if (!_authenticated) return;
    delay(HID_AFTER_WAKE_SETTLE_MS);
    _startIdleTimer();
    _lastReportMs = millis();
}

void KeyStrike::_onConnect(uint16_t connHandle) {
    _connected = true;
    _connHandle = connHandle;
    _connState = _ConnState::Connecting;
}

void KeyStrike::_onDisconnect() {
    _stopIdleTimer();
    _pendingIdleTransition = false;
    _connState = _ConnState::Disconnected;
    _connHandle = BLE_HS_CONN_HANDLE_NONE;
    _connected = false;
    _authenticated = false;
    memset(_keyReport, 0, sizeof(_keyReport));
    _consumerActive = false;
    _reportPrimingNeeded = true;
    _lastReportMs = 0;
    if (_state == _BLEState::Running) NimBLEDevice::startAdvertising();
}

void KeyStrike::_onAuthComplete(bool success) {
    if (success) {
        _authenticated = true;
        _connState = _ConnState::Active;
        _updateConnParams(HID_CONN_INTERVAL, HID_CONN_INTERVAL, 0, HID_CONN_TIMEOUT);
        _startIdleTimer();
    } else {
        _authenticated = false;
        _connState = _ConnState::Connecting;
    }
}

void KeyStrike::_onConfirmPassKey(uint32_t passkey) {
    if (_cbPassKey) {
        _cbPassKey(passkey);
    } else {
        confirmPasskey(true);
    }
}

void KeyStrike::_sendKeyReport() {
    if (!_pKeyboardInput || !_connected) return;
    bool needsPrime = _reportPrimingNeeded || ((millis() - _lastReportMs) >= HID_WINDOWS_PRIME_MS);
    if (needsPrime) {
        bool reportEmpty = true;
        for (int i = 0; i < HID_KEYBOARD_REPORT_SIZE; i++) {
            if (_keyReport[i] != 0) { reportEmpty = false; break; }
        }
        if (!reportEmpty) {
            uint8_t empty[HID_KEYBOARD_REPORT_SIZE] = {};
            _pKeyboardInput->setValue(empty, HID_KEYBOARD_REPORT_SIZE);
            _pKeyboardInput->notify();
            delay(5);
        }
        _reportPrimingNeeded = false;
    }
    _pKeyboardInput->setValue(_keyReport, HID_KEYBOARD_REPORT_SIZE);
    int retries = 0;
    while (!_pKeyboardInput->notify() && _connected && retries++ < 5) vTaskDelay(1);
    _lastReportMs = millis();
}

void KeyStrike::_sendConsumerReport(uint16_t usageId) {
    if (!_pConsumerInput || !_connected) return;
    uint8_t report[HID_CONSUMER_REPORT_SIZE];
    report[0] = (uint8_t)(usageId & 0xFF);
    report[1] = (uint8_t)(usageId >> 8);
    bool needsPrime = _reportPrimingNeeded || ((millis() - _lastReportMs) >= HID_WINDOWS_PRIME_MS);
    if (needsPrime && usageId != 0x0000) {
        uint8_t empty[HID_CONSUMER_REPORT_SIZE] = {};
        _pConsumerInput->setValue(empty, HID_CONSUMER_REPORT_SIZE);
        _pConsumerInput->notify();
        delay(5);
        _reportPrimingNeeded = false;
    }
    _pConsumerInput->setValue(report, HID_CONSUMER_REPORT_SIZE);
    int retries = 0;
    while (!_pConsumerInput->notify() && _connected && retries++ < 5) vTaskDelay(1);
    _lastReportMs = millis();
}

bool KeyStrike::_addKeycode(uint8_t keycode) {
    for (int i = 2; i < 8; i++) if (_keyReport[i] == keycode) return true;
    for (int i = 2; i < 8; i++) {
        if (_keyReport[i] == 0) {
            _keyReport[i] = keycode;
            return true;
        }
    }
    return false;
}

bool KeyStrike::_removeKeycode(uint8_t keycode) {
    for (int i = 2; i < 8; i++) {
        if (_keyReport[i] == keycode) {
            _keyReport[i] = 0;
            return true;
        }
    }
    return false;
}

bool KeyStrike::_isModifier(uint8_t keycode) {
    return (keycode >= KEY_LCTRL && keycode <= KEY_RGUI);
}

uint8_t KeyStrike::_keycodeToModBit(uint8_t keycode) {
    return (1 << (keycode - KEY_LCTRL));
}
