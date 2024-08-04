#include <Arduino.h>

#include <thermistor.h>
#include <heater.h>
#include <light.h>
#include <lens_cap.h>

#define TEMP_1_PIN A5
#define TEMP_2_PIN A6
#define TEMP_3_PIN A3
#define TEMP_4_PIN A4

#define HEATER_1_ON_PIN A0
#define HEATER_1_OFF_PIN A1

#define HEATER_2_ON_PIN 3
#define HEATER_2_OFF_PIN 2

#define HEATER_3_ON_PIN 5
#define HEATER_3_OFF_PIN 4

#define LENS_CAP_CONTROL_PIN 6

#define LENS_CAP_OPEN_PIN 8
#define LENS_CAP_CLOSE_PIN 7

#define HEATER_1_CONTROL_PIN 11
#define HEATER_2_CONTROL_PIN 10
#define HEATER_3_CONTROL_PIN 9

#define LIGHT_CONTROL_PIN 12
#define LIGHT_ON_PIN A7
#define LIGHT_OFF_PIN 13

#define HEATER_1_ENABLE 0x01
#define HEATER_1_DISABLE 0x02

#define HEATER_2_ENABLE 0x03
#define HEATER_2_DISABLE 0x04

#define HEATER_3_ENABLE 0x05
#define HEATER_3_DISABLE 0x06

#define LENS_CAP_OPEN 0x07
#define LENS_CAP_CLOSE 0x08

#define LIGHT_ON 0x09
#define LIGHT_OFF 0x0A

#define uint unsigned int

#define SYNC_BYTE 0x50

#define NUM_TEMP_SENSORS 4
#define NUM_DIGITAL_SENSORS 4

ThermistorConfig temp_1_config = {
    TEMP_1_PIN,         // pin
    10000,      // seriesResistor
    10000,      // nominalResistance
    25,         // nominalTemperature
    3950,       // bCoefficient
    1023,       // adcMax
    5.0,        // supplyVoltage
    5           // numSamples
};

ThermistorConfig temp_2_config = {
    TEMP_2_PIN,         // pin
    10000,      // seriesResistor
    10000,      // nominalResistance
    25,         // nominalTemperature
    3950,       // bCoefficient
    1023,       // adcMax
    5.0,        // supplyVoltage
    5           // numSamples
};

ThermistorConfig temp_3_config = {
    TEMP_3_PIN,         // pin
    10000,      // seriesResistor
    10000,      // nominalResistance
    25,         // nominalTemperature
    3950,       // bCoefficient
    1023,       // adcMax
    5.0,        // supplyVoltage
    5           // numSamples
};

ThermistorConfig temp_reference_config = {
    TEMP_4_PIN,         // pin
    10000,      // seriesResistor
    10000,      // nominalResistance
    25,         // nominalTemperature
    3950,       // bCoefficient
    1023,       // adcMax
    5.0,        // supplyVoltage
    5           // numSamples
};

Thermistor temp_1_sensor(temp_1_config);
Thermistor temp_2_sensor(temp_2_config);
Thermistor temp_3_sensor(temp_3_config);
Thermistor temp_reference_sensor(temp_reference_config);
Thermistor* thermistors[NUM_TEMP_SENSORS] = {&temp_1_sensor, &temp_2_sensor, &temp_3_sensor, &temp_reference_sensor };

HeaterConfig heater_1_config = {
    HEATER_1_CONTROL_PIN,
    HEATER_1_ON_PIN,
    HEATER_1_OFF_PIN,
    &temp_1_sensor,
    &temp_reference_sensor,
    2,
    2,
};

HeaterConfig heater_2_config = {
    HEATER_2_CONTROL_PIN,
    HEATER_2_ON_PIN,
    HEATER_2_OFF_PIN,
    &temp_2_sensor,
    &temp_reference_sensor,
    2,
    2,
};

HeaterConfig heater_3_config = {
    HEATER_3_CONTROL_PIN,
    HEATER_3_ON_PIN,
    HEATER_3_OFF_PIN,
    &temp_3_sensor,
    &temp_reference_sensor,
    2,
    2,
};

Heater heater_1(heater_1_config);
Heater heater_2(heater_2_config);
Heater heater_3(heater_3_config);

Light flat_light(LIGHT_CONTROL_PIN, LIGHT_ON_PIN, LIGHT_OFF_PIN);

LensCap lens_cap(LENS_CAP_CONTROL_PIN, LENS_CAP_OPEN_PIN, LENS_CAP_CLOSE_PIN);

long cmd_timer = 0;

typedef enum {
    STATE_IDLE,
    STATE_READING_SYNC,
    STATE_READING_PAYLOAD,
} CommandReadState;

CommandReadState command_read_state = STATE_IDLE;
byte serial_buffer[100] = {0};
unsigned int buff_idx = 0;
uint sync_bytes_read = 0;
uint payload_length = 0;


void setup() {
  pinMode(HEATER_2_ON_PIN, INPUT);
  pinMode(HEATER_2_OFF_PIN, INPUT);
  pinMode(HEATER_3_ON_PIN, INPUT);
  pinMode(HEATER_3_OFF_PIN, INPUT);

  pinMode(LENS_CAP_OPEN_PIN, INPUT);
  pinMode(LENS_CAP_CLOSE_PIN, INPUT);

  pinMode(HEATER_1_CONTROL_PIN, OUTPUT);
  pinMode(HEATER_2_CONTROL_PIN, OUTPUT);
  pinMode(HEATER_3_CONTROL_PIN, OUTPUT);

  pinMode(LIGHT_CONTROL_PIN, OUTPUT);

  pinMode(LIGHT_ON_PIN, INPUT);
  pinMode(LIGHT_OFF_PIN, INPUT);

  digitalWrite(HEATER_1_CONTROL_PIN, LOW);
  digitalWrite(HEATER_2_CONTROL_PIN, LOW);
  digitalWrite(HEATER_3_CONTROL_PIN, LOW);

  Serial.begin(57600);
 
  lens_cap.setup();

}

void parse_command(byte data) {
    switch (data) {
      case HEATER_1_ENABLE:
        heater_1.enable_active_control();
        break;
      case HEATER_1_DISABLE:
        heater_1.disable_active_control();
        break;
      case HEATER_2_ENABLE:
        heater_2.enable_active_control();
        break;
      case HEATER_2_DISABLE:
        heater_2.disable_active_control();
        break;
      case HEATER_3_ENABLE:
        heater_3.enable_active_control();
        break;
      case HEATER_3_DISABLE:
        heater_3.disable_active_control();
        break;
      case LENS_CAP_OPEN:
        lens_cap.open();
        break;
      case LENS_CAP_CLOSE:
        lens_cap.close();
        break;
      case LIGHT_ON:
        flat_light.turn_on();
        break;
      case LIGHT_OFF:
        flat_light.turn_off();
        break;
  }
}

void send_telemetry() {
  uint8_t header[3] = {0x50, 0x50, 0x50};
  Serial.write(header, 3);
  for (int i = 0; i < NUM_TEMP_SENSORS; i++) {
    float value = thermistors[i]->LastReadingC;
    byte* value_ptr = (byte*)&value;
    for (int i = 0; i < sizeof(float); i++) {
      Serial.write(value_ptr[i]);
    }
  }
  Serial.write(static_cast<uint8_t>(heater_1.driver_state));
  Serial.write(static_cast<uint8_t>(heater_1.manual_state));
  Serial.write(static_cast<uint8_t>(heater_1.real_state));
  Serial.write(static_cast<uint8_t>(heater_2.driver_state));
  Serial.write(static_cast<uint8_t>(heater_2.manual_state));
  Serial.write(static_cast<uint8_t>(heater_2.real_state));
  Serial.write(static_cast<uint8_t>(heater_3.driver_state));
  Serial.write(static_cast<uint8_t>(heater_3.manual_state));
  Serial.write(static_cast<uint8_t>(heater_3.real_state));
  Serial.write(static_cast<uint8_t>(flat_light.driver_state));
  Serial.write(static_cast<uint8_t>(flat_light.manual_state));
  Serial.write(static_cast<uint8_t>(flat_light.real_state));
  Serial.write(static_cast<uint8_t>(lens_cap.driver_state));
  Serial.write(static_cast<uint8_t>(lens_cap.manual_state));
  Serial.write(static_cast<uint8_t>(lens_cap.real_state));
}

void loop() {

  temp_1_sensor.readTemperatureC();
  temp_2_sensor.readTemperatureC();
  temp_3_sensor.readTemperatureC();
  temp_reference_sensor.readTemperatureC();

  heater_1.update();
  heater_2.update();
  heater_3.update();

  flat_light.update();
  lens_cap.update();

  send_telemetry();

  if (Serial.available() > 0) {
    // Read the incoming byte:
    byte data = Serial.read();
    switch (command_read_state) {
      case STATE_IDLE:
        if (data == SYNC_BYTE) {
          buff_idx = 0;
          sync_bytes_read = 1;
          command_read_state = STATE_READING_SYNC;
        }
        break;
      case STATE_READING_SYNC:
        if (data != SYNC_BYTE) {
          command_read_state = STATE_IDLE;
        }
        else {
          sync_bytes_read ++;
          if (sync_bytes_read == 3) {
            command_read_state = STATE_READING_PAYLOAD;
          }
        }
        break;
      case STATE_READING_PAYLOAD:
        command_read_state = STATE_IDLE;
        parse_command(data);
        
        break;
    }
  }
}