#ifndef LIGHT_H
#define LIGHT_H

#include <Arduino.h>
#include <light.h>

typedef enum {
    LIGHT_STATE_OFF,
    LIGHT_STATE_ON,
    LIGHT_STATE_UNDEF, //manual switch sets to auto mode
} LightState;


class Light {
public:
    Light(int control_pin, int on_pin, int off_pin);
    
    void turn_on();
    void turn_off();
    void update();

    LightState manual_state; //state commanded by switches, takes precedence over driver
    LightState driver_state;
    LightState real_state; // actual state of the light

private:
    int _control_pin;
    int _on_pin;
    int _off_pin;
};

#endif
