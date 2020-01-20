
// Code by Marcelo Soares and Douglas Quintanilha
// Released to the public domain


/*----------------------------------------------------------------------------
 *  Implementation of the class TimeCounter
 *  Date and time counter and formatter
 */

#include "TimeCounter.h"


//==============================================================================
// SETUP of the class object
//
void TimeCounter::begin() {
  Wire.begin();
  rtc.begin();
  updateDateTime();
}
//------------------------------------------------------------------------------


//==============================================================================
// Update DateTime object and formatted strings -- Return day has changed
//
bool TimeCounter::updateDateTime() {

  dt = rtc.now();
  sprintf(nowTime, HOUR_FORMAT, int(dt.hour()), int(dt.minute()), int(dt.second()));

  if (presentDay != int(dt.day())) {
    presentDay = int(dt.day());
    sprintf(today, DATE_FORMAT, presentDay, int(dt.month()), int(dt.year()));
    return true;
  }

  return false;
}
//------------------------------------------------------------------------------