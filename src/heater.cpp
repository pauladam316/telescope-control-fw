#include "heater.h"
#include <Arduino.h>
#include <EEPROM.h>

Heater::Heater(const HeaterConfig& config)
    : _heater_pin(config.heater_pin),
      _heater_temp_sensor(config.heater_temp_sensor), 
      _reference_temp_sensor(config.reference_temp_sensor), 
      _temp_delta(config.temp_delta), 
      _hysteresis(config.hysteresis),
      _heater_on_pin(config.heater_on_pin),
      _heater_off_pin(config.heater_off_pin),
      _eeprom_addr(config.eeprom_addr) {
        EEPROM.get(_eeprom_addr, driver_state);
        //driver_state = HeaterCommandState::HEATER_STATE_OFF;
        manual_state = HeaterCommandState::HEATER_STATE_UNDEF;
        real_state = HeaterState::HEATER_STATUS_OFF;
}


void Heater::disable_active_control() {
    driver_state = HeaterCommandState::HEATER_STATE_OFF;
    EEPROM.put(_eeprom_addr, driver_state);
}

void Heater::enable_active_control() {
    driver_state = HeaterCommandState::HEATER_STATE_ON;    
    EEPROM.put(_eeprom_addr, driver_state);
}

void Heater::update() {
    if (digitalRead(_heater_on_pin) == HIGH) {
        manual_state = HeaterCommandState::HEATER_STATE_ON;
    }
    else if (digitalRead(_heater_off_pin) == HIGH) {
        manual_state = HeaterCommandState::HEATER_STATE_OFF;
    }
    else {
        manual_state = HeaterCommandState::HEATER_STATE_UNDEF;
    }

    if (manual_state == HeaterCommandState::HEATER_STATE_ON) {
        digitalWrite(_heater_pin, HIGH);
        real_state = HeaterState::HEATER_STATUS_ON;
    }
    else if (manual_state == HeaterCommandState::HEATER_STATE_OFF) {
        digitalWrite(_heater_pin, LOW);
        real_state = HeaterState::HEATER_STATUS_OFF;
    }
    else {
        if (driver_state == HeaterCommandState::HEATER_STATE_ON) {
            if (real_state == HeaterState::HEATER_STATUS_OFF) {
                target = _reference_temp_sensor->LastReadingC + _temp_delta - _hysteresis;
                if (_heater_temp_sensor->LastReadingC < target) {
                    real_state = HeaterState::HEATER_STATUS_ON;
                    digitalWrite(_heater_pin, HIGH);
                }
            } 
            else if (real_state == HeaterState::HEATER_STATUS_ON) {
                target = _reference_temp_sensor->LastReadingC + _temp_delta + _hysteresis;
                if (_heater_temp_sensor->LastReadingC > target) {
                    real_state = HeaterState::HEATER_STATUS_OFF;
                    digitalWrite(_heater_pin, LOW);
                }
            }
        }
        else {
            real_state = HeaterState::HEATER_STATUS_OFF;
            digitalWrite(_heater_pin, LOW);
        }
    }
}