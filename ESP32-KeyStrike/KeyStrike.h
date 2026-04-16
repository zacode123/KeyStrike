#pragma once

#define HID_REPORT_ID_KEYBOARD 0x01
#define HID_REPORT_ID_CONSUMER 0x02
#define HID_KEYBOARD_REPORT_SIZE 8
#define HID_CONSUMER_REPORT_SIZE 2
#define HID_AFTER_WAKE_SETTLE_MS 250
#define HID_WINDOWS_PRIME_MS 800
#define HID_IDLE_THRESHOLD_MS 5000
#define HID_CONN_INTERVAL 6
#define HID_CONN_TIMEOUT 300

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <NimBLEServer.h>
#include <NimBLEHIDDevice.h>
#include <NimBLECharacteristic.h>
#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>
#include "BLEHIDKeys.h"
#include "BLEHIDMediaKeys.h"

enum class KeyStrikeSecurity : uint8_t {
    JustWorks = 0,
    Passkey,
};

class KeyStrike;

namespace KeyStrike_Internal {
class ServerCallbacks : public NimBLEServerCallbacks {
public:
    ServerCallbacks(KeyStrike* parent) : _parent(parent) {}
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override;
    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override;
    void onAuthenticationComplete(NimBLEConnInfo& connInfo) override;
    void onConfirmPassKey(NimBLEConnInfo& connInfo, uint32_t pass_key) override;
private:
    KeyStrike* _parent;
};
}

class KeyStrike : public Print {
public:
    KeyStrike(const char* deviceName = "KeyStrike", const char* manufName = "zacode123");
    void begin();
    void end();
    bool isConnected() const;
    bool isPaired() const;
    void clearBonds();
    void setSecurityMode(KeyStrikeSecurity mode);
    void setPasskeyCallback(void (*cb)(uint32_t passkey));
    void confirmPasskey(bool confirm);
    void setTxPower(uint8_t level);
    void setBatteryLevel(uint8_t level);
    void press(uint8_t keycode, uint8_t modifiers = 0);
    void press(uint16_t usageId);
    void release(uint8_t keycode);
    void release(uint16_t usageId);
    void releaseAll();
    void tap(uint8_t keycode, uint8_t modifiers = 0, uint16_t delayMs = 0, uint16_t keyGap = 0);
    void tap(uint16_t usageId, uint16_t delayMs = 0, uint16_t keyGap = 0);
    size_t write(uint8_t c) override;
    size_t write(const uint8_t* buffer, size_t size) override;
    void beforeSleep();
    void afterWake();
    void _onConnect(uint16_t connHandle);
    void _onDisconnect();
    void _onAuthComplete(bool success);
    void _onConfirmPassKey(uint32_t passkey);
    static void _idleTimerCallback(TimerHandle_t xTimer);

private:
    enum class _BLEState : uint8_t { Stopped = 0, Running = 1 };
    enum class _ConnState : uint8_t { Disconnected = 0, Connecting, Active, Idle };

    const char* _deviceName;
    const char* _manufName;
    uint8_t _batteryLevel;
    KeyStrikeSecurity _secMode;
    _BLEState _state;
    volatile bool _connected;
    volatile bool _authenticated;
    uint8_t _keyReport[HID_KEYBOARD_REPORT_SIZE];
    bool _consumerActive;
    volatile bool _reportPrimingNeeded;
    uint32_t _lastReportMs;
    uint8_t _txPowerLevel;
    volatile _ConnState _connState;
    volatile uint16_t _connHandle;
    TimerHandle_t _idleTimer;
    volatile bool _pendingIdleTransition;
    void (*_cbPassKey)(uint32_t);

    NimBLEServer* _pServer;
    NimBLEHIDDevice* _pHID;
    NimBLECharacteristic* _pKeyboardInput;
    NimBLECharacteristic* _pConsumerInput;
    KeyStrike_Internal::ServerCallbacks* _pServerCb;

    void _sendKeyReport();
    void _sendConsumerReport(uint16_t usageId);
    void _updateConnParams(uint16_t minInterval, uint16_t maxInterval, uint16_t latency, uint16_t timeout);
    void _transitionToActive();
    void _startIdleTimer();
    void _stopIdleTimer();
    bool _addKeycode(uint8_t keycode);
    bool _removeKeycode(uint8_t keycode);
    bool _isModifier(uint8_t keycode);
    uint8_t _keycodeToModBit(uint8_t keycode);
};