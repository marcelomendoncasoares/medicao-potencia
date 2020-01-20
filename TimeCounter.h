
// Code by Marcelo Soares and Douglas Quintanilha
// Released to the public domain

#ifndef _TIME_COUNTER_H_
#define _TIME_COUNTER_H_

#include <Wire.h>
#include <RTClib.h>
#include <SdFat.h>

#include "Arduino.h"

// Date and Time formatting strings
#define DATE_FORMAT "%02d/%02d/%4d"
#define HOUR_FORMAT "%02d:%02d:%02d"


/*----------------------------------------------------------------------------
 *  Class TimeCounter
 *  Date and time counter and formatter
 */
class TimeCounter {

  public:
    TimeCounter() {
      presentDay = 0;
    }

    void begin();
    bool updateDateTime();
    uint8_t getSeconds() const { return dt.second(); }
    uint8_t getMinutes() const { return dt.minute(); }
    uint8_t getHour() const { return dt.hour(); }
    uint8_t getDay() const { return dt.day(); }
    uint8_t getMonth() const { return dt.month(); }
    uint16_t getYear() const { return dt.year(); }
    char* getDate() const { return today; }
    char* getTime() const { return nowTime; }

  private:
    DS3231 rtc;
    DateTime dt;

    uint8_t presentDay;
    char today[11];    // Format DD/MM/YYYY
    char nowTime[9];   // Format hh:mm:ss
};


#endif // _TIME_COUNTER_H_
