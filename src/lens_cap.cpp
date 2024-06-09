#include "lens_cap.h"
#include <Arduino.h>

LensCap::LensCap(int control_pin, int open_pin, int close_pin)
    : _control_pin(control_pin),
    _open_pin(open_pin),
    _close_pin(close_pin) { 
        driver_state = CapState::CAP_STATE_CLOSED;
        manual_state = CapState::CAP_STATE_UNDEF;
        real_state = CapState::CAP_STATE_CLOSED;
        
    }

void LensCap::setup() {
    _cap_servo.attach(_control_pin, 620, 2420);
}

void LensCap::open() {
    driver_state = CapState::CAP_STATE_OPEN;
}

void LensCap::close() {
    driver_state = CapState::CAP_STATE_CLOSED;
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
        _cap_servo.write(180);
        real_state = CapState::CAP_STATE_OPEN;
    }
    else if (manual_state == CapState::CAP_STATE_CLOSED) {
        _cap_servo.write(0);
        real_state = CapState::CAP_STATE_CLOSED;
    }
    else {
        if (driver_state == CapState::CAP_STATE_OPEN) {
            _cap_servo.write(180);
            real_state = CapState::CAP_STATE_OPEN;
        }
        else {
            _cap_servo.write(0);
            real_state = CapState::CAP_STATE_CLOSED;
        }
    }

}