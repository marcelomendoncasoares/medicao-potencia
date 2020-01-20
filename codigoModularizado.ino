
// Code by Marcelo Soares and Douglas Quintanilha
// Released to the public domain

#include <SPI.h>
#include <SdFat.h>
#include <NeoSWSerial.h>
#include <Wire.h>
#include <RTClib.h>

#include "Measure.h"
#include "Communicate.h"
#include "FileSystem.h"
#include "TimeCounter.h"
#include "LED.h"


#define SAMPLES_PER_WINDOW 5000
#define NUM_WINDOWS        5

#define STANDARD_CURRENT_PIN    A0
#define AMPLIFIED_CURRENT_PIN   A1
#define VOLTAGE_PIN             A2
#define SENSOR_SENSIBILITY      0.100
#define VOLTAGE_MEASURING_RATIO 5.0/680
#define MAX_CURRENT_VALUE       20
#define CURRENT_GAIN            10

#define BLUETOOTH_STATE_PIN 5
#define BLUETOOTH_TX_PIN    6
#define BLUETOOTH_RX_PIN    7
#define EMERGENCY_LED_PIN   8

// File name format 'YYYY.MM.DD.csv'
#define FILE_NAME_FORMAT "%4d.%02d.%02d.csv"
#define AUTOCONFIG_FILE  "autoconfig.txt"


//==============================================================================
// Auxiliar modules configuration constants and variables
//
TimeCounter timeCounter;
ArduinoOutStream cout(Serial);

LED led(EMERGENCY_LED_PIN);
Communicate communicate(BLUETOOTH_STATE_PIN, BLUETOOTH_TX_PIN, BLUETOOTH_RX_PIN, &cout);
Measure measure(STANDARD_CURRENT_PIN, AMPLIFIED_CURRENT_PIN, VOLTAGE_PIN, MAX_CURRENT_VALUE, CURRENT_GAIN, SENSOR_SENSIBILITY, VOLTAGE_MEASURING_RATIO);
FileSystem fileSystem;

char fileName[15];
bool monitoring = false;


//==============================================================================
// Declare reset function @ address 0 --- Reset Arduino via software
//
void(* resetFunc) (void) = 0;


//==============================================================================
// Wait blinking the LED until serial connection to print an error message
//
void haltOnError(const __FlashStringHelper* errorMessage) {
  
  // Register the error date and time
  timeCounter.updateDateTime();

  // Await user handshake
  while (!communicate.isDeviceConnected()) {
    led.blink();
  }
  
  cout << F("\nSystem halted! Error ocurred at ") << timeCounter.getDate() << ' ' << timeCounter.getTime() << endl;
  cout << errorMessage << endl;
  
  cout << F("\nType any character to RESET...") << endl;
  communicate.waitForInput();
  resetFunc();
}
//------------------------------------------------------------------------------


//==============================================================================
// Print function to show the calculated RMS and Power values 
//
inline void printAverageValues() {
  
  if (!communicate.isDeviceConnected() or !monitoring) {
    return;
  }
  
  cout << ' ' << timeCounter.getDate() << ' ' << timeCounter.getTime() << setprecision(3) << F(" (sample of ") << measure.getLastPeriod() << F(" s)") << endl;
  cout << F("  |  VccRef: ") << measure.getVccRef() << F(" V") << endl;
  cout << F("  |  Current: ") << measure.getCurrentRMS() << F(" A ") << F("(zero = ") << measure.getZeroCurrent() << F(" V");
  measure.isAmplified() ? cout << F(", amplified)") << endl : cout << F(", not amplified)") << endl;
  cout << F("  |  Voltage: ") << measure.getVoltageRMS()  << F(" V ") << F("(zero = ") << measure.getZeroVoltage() << F(" V)") << endl;
  cout << F("  |  Real power: ") << measure.getRealPower() << F(" Watts") << endl;
  cout << F("  |  Apparent power: ") << measure.getApparentPower() << F(" VA") << endl;
  cout << F("  |  Power factor: ") << measure.getPowerFactor() << endl << endl;
}
//------------------------------------------------------------------------------


//==============================================================================
// Filename manipulation
//
void updateDateTimeAndFileName() {
  if (timeCounter.updateDateTime()) {
    sprintf(fileName, FILE_NAME_FORMAT, timeCounter.getYear(), timeCounter.getMonth(), timeCounter.getDay());
  }
}

// Reset filename to present day file
void resetFileName() {
  timeCounter.updateDateTime();
  sprintf(fileName, FILE_NAME_FORMAT, timeCounter.getYear(), timeCounter.getMonth(), timeCounter.getDay());
}
//------------------------------------------------------------------------------


//==============================================================================
// Configure active directory to store measure files
//
void configureDirectory() {

  cout << F("Enter folder name: ");
  char* dir = new char[20];
  strcpy(dir, communicate.waitForInput());

  if (!fileSystem.makeDir(dir)) {
    haltOnError(F("Create folder failed!"));
  }

  cout << F("Configure autoreset to this session? (Y/N) ");
  if (!strcmp(communicate.waitForInput(), "Y")) {
    if (!fileSystem.saveActiveSession(dir, AUTOCONFIG_FILE)) {
      haltOnError(F("Could not create autoconfig file!"));
    }
    cout << F("Autoconfig file created!") << endl;
  }

  if (!fileSystem.changeDir(dir)) {
    haltOnError(F("CHDIR to folder failed!"));
  }
  delete[] dir;
}
//------------------------------------------------------------------------------


//==============================================================================
// Checks wether data should be transmitted via bluetooth to anorther connected device
//
inline void checkAndTransmitData() {

  if (!communicate.isDeviceConnected() or !communicate.getRequest()) {
    return;
  }

  // Stop bluetooth from listening interrupts;
  communicate.bluetoothIgnore();

  // Evaluate the option requested
  switch (communicate.getRequest()) {

    // Option F: transfer last active (F)ile
    case 'F':
      fileSystem.transferFile(fileName, &cout, &communicate);
      break;

    // Option A: transfer (A)ll month files
    case 'A':
      timeCounter.updateDateTime();
      for (uint8_t dayCounter = 1; dayCounter <= timeCounter.getDay(); ++dayCounter) {
        sprintf(fileName, FILE_NAME_FORMAT, timeCounter.getYear(), timeCounter.getMonth(), dayCounter);
        fileSystem.transferFile(fileName, &cout, &communicate);
      }
      break;

    // Option L: (L)ist all files in SD Card
    case 'L':
      fileSystem.listFiles(communicate.getCommPort());
      break;

    // Option P: (P)rint free space in SD Card
    case 'P':
      fileSystem.printFreeSpace(&cout);
      break;

    // Option C: (C)hange active directory
    case 'C':
      configureDirectory();
      break;

    // Option M: turn (M)onitoring of reading results on/off
    case 'M':
      monitoring = !monitoring;
      cout << F("Monitoring ");
      (monitoring) ? cout << F("ON!") << endl : cout << F("OFF!") << endl;
      break;

    // Option D: (D)elete autoconfig file
    case 'D':
      cout << F("Confirm delete? (Y/N) ");
      if (strcmp(communicate.waitForInput(), "Y")) {
        cout << F("Canceled!") << endl;
        break;
      }
      if (!fileSystem.deleteAutoconfigFile(AUTOCONFIG_FILE)) {
        cout << F("Could not delete autoconfig file!") << endl;
        break;
      }      
      cout << F("Autoconfig file deleted!") << endl;
      break;

    // Option X: (W)ipe all files in SD Card
    case 'W':
      cout << F("Confirm wipe? (Y/N) ");
      if (strcmp(communicate.waitForInput(), "Y")) {
        cout << F("Wipe canceled!") << endl;
        break;
      }
      if (!fileSystem.wipeSDCard(communicate.getCommPort())) {
        haltOnError(F("Wipe failed!"));
      }
      cout << F("SD Card successfully wiped!") << endl;
      configureDirectory();
      break;

    // Option R: (R)eset device
    case 'R':
      resetFunc();
      break;

    // Option not recognized
    default:
      cout << F("Option '") << communicate.getRequest() << F("' invalid!") << endl;
  }

  // Sends an "END" message to close transmission
  cout << F("END") << endl << endl;

  communicate.resetRequest();
  communicate.clearSerialBuffer();
  communicate.bluetoothListen();
}
//------------------------------------------------------------------------------


//==============================================================================
// Initialization of the code
//
void setup() {

  //Initialize the objects
  communicate.begin();
  timeCounter.begin();
  measure.begin(SAMPLES_PER_WINDOW, NUM_WINDOWS);
  led.begin(true);

  communicate.isDeviceConnected();

  // Initialize the SD Card module
  if (!fileSystem.begin()) {
    haltOnError(F("File System initialization failed!"));
  }

  // Define the actual dateTime and filename
  resetFileName();

  // Request the name of the folder to store reading files
  // If not present, wait for serial connection and folder name input
  if (!fileSystem.restoreSession(AUTOCONFIG_FILE)) {
    communicate.waitForConnection();
    communicate.clearSerialBuffer();
    configureDirectory();
  }

  cout << F("Setup complete...\n") << endl;
  communicate.clearSerialBuffer();
  communicate.bluetoothListen();
  led.setOff();
}
//------------------------------------------------------------------------------


//==============================================================================
// Reading and continuos recording of measures
//
void loop() {
  measure.acquireAndCalculate();
  updateDateTimeAndFileName();
  printAverageValues();

  if (!fileSystem.recordValues(fileName, &measure)) {
    haltOnError(F("Could not open/create file to write!"));
  }

  checkAndTransmitData();
}
//------------------------------------------------------------------------------