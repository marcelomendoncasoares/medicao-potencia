
// Code by Marcelo Soares and Douglas Quintanilha
// Released to the public domain

#ifndef _MEASURE_H_
#define _MEASURE_H_

#include "Arduino.h"


// Arduino DAC sensibility --- Is multiplied by VccRef, which is the 5V reference used by ADC
#define VOLTS_PER_UNITY 1.0/1024
#define INTERNAL_VREF_VALUE 1.1034

/*----------------------------------------------------------------------------
 *  Class Measure
 *  Sampling of current and voltage signals, calculation of RMS values
 */
class Measure {
  public:
    Measure(uint8_t standardCurrentPin,
            uint8_t amplifiedCurrentPin,
            uint8_t voltagePin,
            uint8_t maxCurrentValue,    // ACS712 current sensor specification (5 | 20 | 30)
            uint8_t currentGain,        // Current gain of amplified current pin
            float sensorSensibility,    // ACS712 current sensor specification (5A:0.185 | 20A:0.100 | 30A:0.066)
            float voltageMeasuringRatio // RMS AC grid voltage = 127 Volts (5V arduino / -Vp to +Vp AC grid voltage)
           ) {

      STANDARD_CURRENT_PIN = standardCurrentPin;
      AMPLIFIED_CURRENT_PIN = amplifiedCurrentPin;
      VOLTAGE_PIN = voltagePin;
      CURRENT_GAIN = currentGain;
      CURRENT_ENTRY_SHIFT_VALUE = maxCurrentValue / currentGain;
      SENSOR_SENSIBILITY = sensorSensibility;
      VOLTAGE_MEASURING_RATIO = voltageMeasuringRatio;

      currentPin = standardCurrentPin;

      sTime = 0;
      eTime = 0;
    }

    void begin(uint16_t samplesPerWindow=5000, uint16_t numWindows=1);
    void acquireAndCalculate();
    bool isAmplified() const { return (currentPin == AMPLIFIED_CURRENT_PIN); }
    float getZeroVoltage() const { return zeroVoltage * VOLTS_PER_UNITY * vccRef; }
    float getZeroCurrent() const { return zeroCurrent * VOLTS_PER_UNITY * vccRef; }
    float getVccRef() const { return vccRef; }
    float getVoltageRMS() const { return voltageRMS; } 
    float getCurrentRMS() const { return currentRMS; }
    float getRealPower() const { return realPower; }
    float getApparentPower() const { return apparentPower; }
    float getPowerFactor() const { return powerFactor; }
    float getLastPeriod() const { return float(eTime - sTime)/1000; }

  private:
    void calibrateVccRef();
    void acquireSamples();
    void calculateZeroValues();
    void calculateRMSAndPowerValues();
    void calculateAverageRMSAndPowerValues();

    uint32_t sTime, eTime;
    uint8_t currentPin;

    float vccRef;
    float sumSqrCurrent, sumSqrVoltage, sumInstPower;
    float zeroCurrent, zeroVoltage;
    float sumZeroCurrent, sumZeroVoltage;
    float currentRMS, voltageRMS, realPower, apparentPower, powerFactor;
    float sumCurrent, sumVoltage, sumRealPower, sumApparentPower, sumPowerFactor;

    uint8_t STANDARD_CURRENT_PIN, AMPLIFIED_CURRENT_PIN, VOLTAGE_PIN;
    uint16_t SAMPLES_PER_WINDOW, NUM_WINDOWS;
    float SENSOR_SENSIBILITY, VOLTAGE_MEASURING_RATIO;
    float CURRENT_GAIN, CURRENT_ENTRY_SHIFT_VALUE;
};


#endif // _MEASURE_H_