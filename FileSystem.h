
// Code by Marcelo Soares and Douglas Quintanilha
// Released to the public domain

#ifndef _FILE_SYSTEM_H_
#define _FILE_SYSTEM_H_

#include <SPI.h>
#include <SdFat.h>

#include "Arduino.h"
#include "Measure.h"
#include "Communicate.h"
#include "TimeCounter.h"

extern TimeCounter timeCounter;

// CSV separator
#define COMMA       ";"
#define DATA_HEADER "date;time;current(A);voltage(V);realPower(W);apparentPower(VA);powerFactor;windowTime(s)"


/*----------------------------------------------------------------------------
 *  Class FileSystem
 *  SD Card file system management
 */
class FileSystem {

  public:
    FileSystem() {}

    bool begin();
    bool saveActiveSession(char* dir, char* fileName);
    bool deleteAutoconfigFile(char* fileName);
    bool restoreSession(char* fileName);
    bool changeDir(char* dir);
    bool makeDir(char* dir);
    bool recordValues(char* fileName, Measure* measure);
    bool transferFile(char* fileName, ArduinoOutStream* cout, Communicate* communicate);
    void printFreeSpace(ArduinoOutStream* cout);
    void listFiles(Stream* commPort);
    bool wipeSDCard(Stream* commPort);
    static void FATDateTime(uint16_t* date, uint16_t* time);

  private:
    SdFat sd;
};


#endif // _FILE_SYSTEM_H_
