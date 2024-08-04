#ifndef LENS_CAP_H
#define LENS_CAP_H

#include <Arduino.h>
#include <light.h>
#include <Servo.h>

typedef enum {
    CAP_STATE_CLOSED,
    CAP_STATE_OPEN,
    CAP_STATE_UNDEF, //manual switch sets to auto mode
} CapState;


class LensCap {
public:
    LensCap(int control_pin, int open_pin, int close_pin);
    void setup();
    void open();
    void close();
    void update();
    void moveServo();
    CapState manual_state; //state commanded by switches, takes precedence over driver
    CapState driver_state;
    CapState real_state; // actual state of the light

private:
    int _control_pin;
    int _open_pin;
    int _close_pin;
    Servo _cap_servo;
    int _current_angle;
    int _target_angle;
    const int _step_delay = 2000; // Delay between steps in microseconds
    const int _step_size = 1;   // Change in angle per step
    unsigned long _last_update_time = 0;
};

#endif
