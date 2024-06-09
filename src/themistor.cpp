#include "Thermistor.h"

Thermistor::Thermistor(const ThermistorConfig& config)
    : _pin(config.pin), _seriesResistor(config.seriesResistor), 
      _nominalResistance(config.nominalResistance), 
      _nominalTemperature(config.nominalTemperature), 
      _bCoefficient(config.bCoefficient), 
      _adcMax(config.adcMax), 
      _supplyVoltage(config.supplyVoltage),
      _numSamples(config.numSamples) {}

int Thermistor::readADC() {
    int total = 0;
    for (int i = 0; i < _numSamples; i++) {
        total += analogRead(_pin);
        delay(10);  // Small delay between samples to get stable readings
    }
    return total / _numSamples;
}

float Thermistor::calculateResistance(int adcValue) {
    float voltage = adcValue * (_supplyVoltage / _adcMax);
    return (_supplyVoltage * _seriesResistor / voltage) - _seriesResistor;
}

float Thermistor::readTemperatureC() {
    int adcValue = readADC();
    float resistance = calculateResistance(adcValue);
    
    float steinhart;
    steinhart = resistance / _nominalResistance; // (R/Ro)
    steinhart = log(steinhart); // ln(R/Ro)
    steinhart /= _bCoefficient; // 1/B * ln(R/Ro)
    steinhart += 1.0 / (_nominalTemperature + 273.15); // + (1/To)
    steinhart = 1.0 / steinhart; // Invert
    steinhart -= 273.15; // Convert to Celsius
    LastReadingC = steinhart;
    return steinhart;
}