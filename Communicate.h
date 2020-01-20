
// Code by Marcelo Soares and Douglas Quintanilha
// Released to the public domain

#ifndef _COMMUNICATE_H_
#define _COMMUNICATE_H_

#include <SdFat.h>
#include <NeoSWSerial.h>
#include <Wire.h>

#include "Arduino.h"


/*----------------------------------------------------------------------------
 *  Class Communicate
 *  Serial communication management
 */
class Communicate {

  public:
    Communicate(uint8_t bluetoothStatePin,
                uint8_t bluetoothTxPin,
                uint8_t bluetoothRxPin,
                ArduinoOutStream* coutExtern
               ) : Bluetooth(bluetoothTxPin, bluetoothRxPin) {

      BLUETOOTH_STATE_PIN = bluetoothStatePin;
      cout = coutExtern;
      commPort = &Serial;
      serialMonitorConnected = false;
      bluetoothConnected = false;
    }

    void begin(uint16_t baudRate=9600);
    void clearSerialBuffer();
    void waitForConnection();
    char* waitForInput();
    bool isUSBConnected();
    bool isBluetoothConnected();
    bool isDeviceConnected() const {return isBluetoothConnected() || isUSBConnected(); }
    char getRequest() const { return request; }
    char resetRequest() { request = 0; }
    void bluetoothListen() { Bluetooth.attachInterrupt(bluetoothEvent); }
    void bluetoothIgnore() { Bluetooth.detachInterrupt(); }
    Stream* getCommPort() const { return commPort; }

    static volatile char request;

  private:
    uint8_t BLUETOOTH_STATE_PIN;

    ArduinoOutStream* cout;
    Stream* commPort;
    NeoSWSerial Bluetooth;

    bool serialMonitorConnected;
    bool bluetoothConnected;

    char cinBuff[21];

    // Trigger interrupt when any char is received at the buffer by bluetooth
    static void bluetoothEvent(uint8_t inChar) { if (request == 0) {request = inChar;} }
};


#endif // _COMMUNICATE_H_
