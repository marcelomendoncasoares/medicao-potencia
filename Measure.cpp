
// Code by Marcelo Soares and Douglas Quintanilha
// Released to the public domain


/*----------------------------------------------------------------------------
 *  Implementation of the class Measure
 *  Sampling of current and voltage signals, calculation of RMS values
 */

#include "Measure.h"


//==============================================================================
// SETUP of aquisition input pins and variable initial values
//
void Measure::begin(uint16_t samplesPerWindow=5000, // Sampling rate
                    uint16_t numWindows=1            // Number of windows to average sampled RMS values
                   ) {

  pinMode(STANDARD_CURRENT_PIN, INPUT);
  pinMode(AMPLIFIED_CURRENT_PIN, INPUT);
  pinMode(VOLTAGE_PIN, INPUT);

  SAMPLES_PER_WINDOW = samplesPerWindow;
  NUM_WINDOWS = numWindows;

  vccRef = 0.0;
  sumSqrCurrent = 0;
  sumSqrVoltage = 0;
  sumInstPower = 0;
  zeroCurrent = 0;
  zeroVoltage = 0;
  sumZeroCurrent = 0;
  sumZeroVoltage = 0;
  sumCurrent = 0;
  sumVoltage = 0;
  sumRealPower = 0;
  sumApparentPower = 0;
  sumPowerFactor = 0;

  // Calibrate the analog read reference value
  calibrateVccRef();
  
  // Calculate initial zero value for each measuring entry
  for (uint16_t sampleIndex = 0; sampleIndex < SAMPLES_PER_WINDOW; ++sampleIndex) {
    sumZeroCurrent += float(analogRead(currentPin));
    sumZeroVoltage += float(analogRead(VOLTAGE_PIN));
  }
  calculateZeroValues();
}
//------------------------------------------------------------------------------


//==============================================================================
// Vref calibration precision test
//
void Measure::calibrateVccRef() {

  float maxVccRef;
  long temp;

  // Set the reference to Vcc and the measurement to the internal 1.1V reference
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA, ADSC)); // Measuring
  temp = ADCL; // Must read ADCL first - it then locks ADCH
  temp |= ADCH << 8;

  maxVccRef = INTERNAL_VREF_VALUE * 1024 / temp;
  if (maxVccRef > vccRef) {
    vccRef = maxVccRef;
  }
}
//------------------------------------------------------------------------------


//==============================================================================
// Calculate the zero to be used as next sample DC value
//
void Measure::calculateZeroValues() {
  zeroCurrent = sumZeroCurrent / SAMPLES_PER_WINDOW;
  zeroVoltage = sumZeroVoltage / SAMPLES_PER_WINDOW;
  sumZeroCurrent = 0;
  sumZeroVoltage = 0;
}
//------------------------------------------------------------------------------


//==============================================================================
// Samples acquisition
//
void Measure::acquireSamples() {

  float current, voltage;

  for (uint16_t sampleIndex = 0; sampleIndex < SAMPLES_PER_WINDOW; ++sampleIndex) {

    current = float(analogRead(currentPin));
    voltage = float(analogRead(VOLTAGE_PIN));

    sumZeroCurrent += current;
    sumZeroVoltage += voltage;

    // Remove the DC value of the signals, using the previous sample as reference
    current -= zeroCurrent;
    voltage -= zeroVoltage;

    // Calculation of the real power by integration of the voltage and current product
    sumSqrCurrent += current * current;
    sumSqrVoltage += voltage * voltage;
    sumInstPower += voltage * current;
  }
}
//------------------------------------------------------------------------------


//==============================================================================
// Calculate the RMS value of voltage and current, and the power consumption
//
void Measure::calculateRMSAndPowerValues() {

  // Remove the current gain (squared because is being removed after the squared sum of each sample)
  if (currentPin == AMPLIFIED_CURRENT_PIN) {
    sumSqrCurrent /= (CURRENT_GAIN * CURRENT_GAIN);
    sumInstPower /= CURRENT_GAIN;
  }

  currentRMS = sqrt(sumSqrCurrent / SAMPLES_PER_WINDOW) * (VOLTS_PER_UNITY * vccRef) / SENSOR_SENSIBILITY;
  voltageRMS = sqrt(sumSqrVoltage / SAMPLES_PER_WINDOW) * (VOLTS_PER_UNITY * vccRef) / VOLTAGE_MEASURING_RATIO;
  realPower = sumInstPower * (VOLTS_PER_UNITY * vccRef) * (VOLTS_PER_UNITY * vccRef) / (SENSOR_SENSIBILITY * VOLTAGE_MEASURING_RATIO * SAMPLES_PER_WINDOW);
  apparentPower = voltageRMS * currentRMS;
  powerFactor = realPower / apparentPower;

  sumVoltage += voltageRMS;
  sumCurrent += currentRMS;
  sumRealPower += realPower;
  sumApparentPower += apparentPower;
  sumPowerFactor += powerFactor;

  // Resets all acumulation variables
  sumSqrCurrent = 0;
  sumSqrVoltage = 0;
  sumInstPower = 0;

  // Alternate the sampling pin if the current value is too low
  if (currentRMS < CURRENT_ENTRY_SHIFT_VALUE) {
    currentPin = AMPLIFIED_CURRENT_PIN;
  }
  else {
    currentPin = STANDARD_CURRENT_PIN;
  }
}
//-----------------------------------------------------------------------------


//==============================================================================
// Calculate the average values acumutaed through the windows
//
void Measure::calculateAverageRMSAndPowerValues() {

  currentRMS = sumCurrent / NUM_WINDOWS;
  voltageRMS = sumVoltage / NUM_WINDOWS;
  realPower = sumRealPower / NUM_WINDOWS;
  apparentPower = sumApparentPower / NUM_WINDOWS;
  powerFactor = sumPowerFactor / NUM_WINDOWS;

  sumCurrent = 0;
  sumVoltage = 0;
  sumRealPower = 0;
  sumApparentPower = 0;
  sumPowerFactor = 0;
}
//------------------------------------------------------------------------------


//==============================================================================
// Sampling of the signals and calculation of RMS values
//
void Measure::acquireAndCalculate() {
  sTime = millis();

  // Repeats the sample reading and calculation to store only the average value
  for (uint16_t windowCounter = 0; windowCounter < NUM_WINDOWS; ++windowCounter) {
    calibrateVccRef();
    acquireSamples();
    calculateZeroValues();
    calculateRMSAndPowerValues();
  }

  eTime = millis();

  calculateAverageRMSAndPowerValues();
}
//------------------------------------------------------------------------------
