#ifndef THERMISTOR_H
#define THERMISTOR_H

#include <Arduino.h>

struct ThermistorConfig {
    int pin;
    int seriesResistor;
    int nominalResistance;
    int nominalTemperature;
    int bCoefficient;
    int adcMax;
    float supplyVoltage;
    int numSamples;
};

class Thermistor {
public:
    Thermistor(const ThermistorConfig& config);
    
    // Method to read the temperature in Celsius
    float readTemperatureC();
    float LastReadingC;

private:
    int _pin;
    int _seriesResistor;
    int _nominalResistance;
    int _nominalTemperature;
    int _bCoefficient;
    int _adcMax;
    float _supplyVoltage;
    int _numSamples;
    
    // Method to read the ADC value
    int readADC();
    
    // Method to calculate the thermistor resistance
    float calculateResistance(int adcValue);
};

#endif
