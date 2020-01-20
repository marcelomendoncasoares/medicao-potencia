
// Code by Marcelo Soares and Douglas Quintanilha
// Released to the public domain


/*----------------------------------------------------------------------------
 *  Implementation of the class Communicate
 *  Serial communication management
 */

#include "Communicate.h"

// Initialization of global or static variables 
volatile char Communicate::request = 0;


//==============================================================================
// Checks if there is any char in Serial Monitor buffer at each interruption
//
void serialEvent() {
  while (Serial.available()) {
    char inChar = Serial.read();
    if (Communicate::request == 0) {
      Communicate::request = inChar;
    }
  }
}
//------------------------------------------------------------------------------


//==============================================================================
// SETUP of the class object
//
void Communicate::begin(uint16_t baudRate=9600) {
  pinMode(BLUETOOTH_STATE_PIN, INPUT);
  Serial.begin(baudRate);
  Bluetooth.attachInterrupt(bluetoothEvent);
  Bluetooth.begin(baudRate);
}
//------------------------------------------------------------------------------


//==============================================================================
// Read any existing serial data to clear the buffer
//
void Communicate::clearSerialBuffer() {
  while (commPort->available() && commPort->read() >= 0) {
    SysCall::yield();
  }
}
//------------------------------------------------------------------------------


//==============================================================================
// Wait for bluetooth or serial monitor connection
//
void Communicate::waitForConnection() {
  while (!isDeviceConnected()) {
    SysCall::yield();
  }
}
//------------------------------------------------------------------------------


//==============================================================================
// Wait for and return input
//
char* Communicate::waitForInput() {

  // Disable interrupt on bluetooth to enable serial reading
  Bluetooth.detachInterrupt();

  ArduinoInStream cin(*commPort, cinBuff, sizeof(cinBuff));
  while (!(cin >> cinBuff)) {
    cin.readline();
  }

  // Reenable interrupt on bluetooth
  Bluetooth.attachInterrupt(bluetoothEvent);
  return cinBuff;
}
//------------------------------------------------------------------------------


//==============================================================================
// Check wether USB or Bluetooth are connected
//
bool Communicate::isBluetoothConnected() {

  if (digitalRead(BLUETOOTH_STATE_PIN)) {
    if (!bluetoothConnected) {
      commPort = &Bluetooth;
      *cout = ArduinoOutStream(Bluetooth);
      serialMonitorConnected = false;
      bluetoothConnected = true;
    }
    return true;
  }

  bluetoothConnected = false;
  return false;
}

// As "if (Serial)" always return true on Arduino UNO, must receive a char to acuse connection
// Only "disconnect" on bluetooth connection
bool Communicate::isUSBConnected() {

  if (serialMonitorConnected) {
    return true;
  }

  if (Serial.available()) {
    commPort = &Serial;
    *cout = ArduinoOutStream(Serial);
    serialMonitorConnected = true;
    bluetoothConnected = false;
    return true;
  }

  serialMonitorConnected = false;
  return false;
}
//------------------------------------------------------------------------------