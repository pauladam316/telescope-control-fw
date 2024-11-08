#include "lens_cap.h"
#include <Arduino.h>
#include <EEPROM.h>

#define SERVO_MIN 500
#define SERVO_MAX 2500
#define IN_MIN 0
#define IN_MAX 270
#define CLOSED_FQ  ((long)107 - IN_MIN) * (SERVO_MAX - SERVO_MIN) / (IN_MAX - IN_MIN) + SERVO_MIN;
#define OPEN_FQ  ((long)200 - IN_MIN) * (SERVO_MAX - SERVO_MIN) / (IN_MAX - IN_MIN) + SERVO_MIN;
#define DRIVER_STATE_EEPROM_ADDR 0

LensCap::LensCap(int control_pin, int open_pin, int close_pin)
    : _control_pin(control_pin),
    _open_pin(open_pin),
    _close_pin(close_pin) { 
        EEPROM.get(DRIVER_STATE_EEPROM_ADDR, driver_state);
        //driver_state = CapState::CAP_STATE_CLOSED;
        manual_state = CapState::CAP_STATE_UNDEF;
        real_state = CapState::CAP_STATE_CLOSED;
        _current_angle = CLOSED_FQ;
    }
    

void LensCap::setup() {
    _cap_servo.attach(_control_pin, 620, 2420);
}

void LensCap::open() {
    driver_state = CapState::CAP_STATE_OPEN;
    EEPROM.put(DRIVER_STATE_EEPROM_ADDR, driver_state);
}

void LensCap::close() {
    driver_state = CapState::CAP_STATE_CLOSED;
    EEPROM.put(DRIVER_STATE_EEPROM_ADDR, driver_state);
}

void LensCap::update() {
    if (digitalRead(_open_pin) == HIGH) {
        manual_state = CapState::CAP_STATE_OPEN;
    }
    else if (digitalRead(_close_pin) == HIGH) {
        manual_state = CapState::CAP_STATE_CLOSED;
    }
    else {
        manual_state = CapState::CAP_STATE_UNDEF;
    }

    if (manual_state == CapState::CAP_STATE_OPEN) {
        _target_angle = OPEN_FQ;
        real_state = CapState::CAP_STATE_OPEN;
    }
    else if (manual_state == CapState::CAP_STATE_CLOSED) {
        _target_angle = CLOSED_FQ;
        real_state = CapState::CAP_STATE_CLOSED;
    }
    else {
        if (driver_state == CapState::CAP_STATE_OPEN) {
            _target_angle = OPEN_FQ;
            real_state = CapState::CAP_STATE_OPEN;
        }
        else {
            _target_angle = CLOSED_FQ;
            real_state = CapState::CAP_STATE_CLOSED;
        }
    }

    moveServo();
}

void LensCap::moveServo() {
    unsigned long current_time = micros();
    if (current_time - _last_update_time >= _step_delay) {
        if (_current_angle < _target_angle) {
            _current_angle += _step_size;
            if (_current_angle > _target_angle) {
                _current_angle = _target_angle;
            }
        }
        else if (_current_angle > _target_angle) {
            _current_angle -= _step_size;
            if (_current_angle < _target_angle) {
                _current_angle = _target_angle;
            }
        }

        _cap_servo.writeMicroseconds(_current_angle);
        _last_update_time = current_time;
    }
}