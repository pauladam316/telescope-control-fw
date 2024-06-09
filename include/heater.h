#ifndef HEATER_H
#define HEATER_H

#include <Arduino.h>
#include <thermistor.h>

struct HeaterConfig {
    int heater_pin;
    int heater_on_pin; //pin to force heater on when high
    int heater_off_pin; //pin to force heater off when high
    Thermistor* heater_temp_sensor;
    Thermistor* reference_temp_sensor;
    float temp_delta; //how many degrees above the reference temp to keep the heater at
    float hysteresis; //in degrees c
};

typedef enum {
    HEATER_STATE_OFF,
    HEATER_STATE_ON,
    HEATER_STATE_UNDEF, //manual switch sets to auto mode
} HeaterCommandState;

typedef enum {
    HEATER_STATUS_OFF,
    HEATER_STATUS_ON
} HeaterState;

class Heater {
public:
    Heater(const HeaterConfig& config);
    
    // Method to read the temperature in Celsius
    void disable_active_control();
    void enable_active_control();
    void update();

    HeaterCommandState manual_state; //state commanded by switches, takes precedence over driver
    HeaterCommandState driver_state;
    HeaterState real_state; // actual state of the light

    float target = 0;

private:
    int _heater_pin;
    int _heater_on_pin; 
    int _heater_off_pin;
    Thermistor* _heater_temp_sensor;
    Thermistor* _reference_temp_sensor;
    float _temp_delta;
    float _hysteresis; 
};

#endif
