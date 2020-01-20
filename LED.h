
// Code by Marcelo Soares and Douglas Quintanilha
// Released to the public domain

#ifndef _LED_H_
#define _LED_H_

#include "Arduino.h"


/*----------------------------------------------------------------------------
 *  Class LED
 *  Manipulation of an emergency/alert LED
 */
class LED {
  public:
    LED(uint8_t emergencyLEDPin=LED_BUILTIN) {
      EMERGENCY_LED_PIN = emergencyLEDPin;
    }

    void begin(bool initiateOn=false) {
    	pinMode(EMERGENCY_LED_PIN, OUTPUT);
    	if (initiateOn) {
    		setOn();
    	}
    }
    
    void setOn() { digitalWrite(EMERGENCY_LED_PIN, HIGH); }
    void setOff() { digitalWrite(EMERGENCY_LED_PIN, LOW); }

    void blink(uint16_t cycles=1, uint16_t interval=200) {
      if (interval == 0) {
        return;
      }
      uint8_t actualState = digitalRead(EMERGENCY_LED_PIN);
      for (uint16_t cycle=0; cycle < cycles; ++cycle) {
        setOn();
        delay(interval);
        setOff();
        delay(interval);
      }
      digitalWrite(EMERGENCY_LED_PIN, actualState);
    }

  private:
    uint8_t EMERGENCY_LED_PIN;
};


#endif // _LED_H_