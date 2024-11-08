#include "light.h"
#include <Arduino.h>
#include <EEPROM.h>

#define LIGHT_STATE_EEPROM_ADDR 10

Light::Light(int control_pin, int on_pin, int off_pin)
    : _control_pin(control_pin),
    _on_pin(on_pin),
    _off_pin(off_pin) { 
        EEPROM.get(LIGHT_STATE_EEPROM_ADDR, driver_state);
        //driver_state = LightState::LIGHT_STATE_OFF;
        manual_state = LightState::LIGHT_STATE_UNDEF;
        real_state = LightState::LIGHT_STATE_OFF;
    }


void Light::turn_on() {
    driver_state = LightState::LIGHT_STATE_ON;
    EEPROM.put(LIGHT_STATE_EEPROM_ADDR, driver_state);
}

void Light::turn_off() {
    driver_state = LightState::LIGHT_STATE_OFF;
    EEPROM.put(LIGHT_STATE_EEPROM_ADDR, driver_state);
}

void Light::update() {
    if (analogRead(_on_pin) > 500) { //a7 can only be read as analog input
        manual_state = LightState::LIGHT_STATE_ON;
    }
    else if (digitalRead(_off_pin) == HIGH) {
        manual_state = LightState::LIGHT_STATE_OFF;
    }
    else {
        manual_state = LightState::LIGHT_STATE_UNDEF;
    }

    if (manual_state == LightState::LIGHT_STATE_ON) {
        digitalWrite(_control_pin, HIGH);
        real_state = LightState::LIGHT_STATE_ON;
    }
    else if (manual_state == LightState::LIGHT_STATE_OFF) {
        digitalWrite(_control_pin, LOW);
        real_state = LightState::LIGHT_STATE_OFF;
    }
    else {
        if (driver_state == LightState::LIGHT_STATE_ON) {
            digitalWrite(_control_pin, HIGH);
            real_state = LightState::LIGHT_STATE_ON;
        }
        else {
            digitalWrite(_control_pin, LOW);
            real_state = LightState::LIGHT_STATE_OFF;
        }
    }

}