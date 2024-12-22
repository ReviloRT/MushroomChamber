
#include "Defines.h"

#include "AbstractRTC.h"
#include "AbstractState.h"
#include "AbstractSensors.h"
#include "AbstractStorage.h"
#include "AbstractServer.h"


unsigned long loop1Time = 0;
unsigned long loop2Time = 0;
unsigned long loop3Time = 0;
unsigned long loop4Time = 0;

Abstract_RTC rtc;
Abstract_State state(&rtc);
Abstract_Sensors sensors(&state);
Abstract_Storage storage;
Abstract_Server server(&state, &storage);


void setup() {
  // Initialise serial
  Serial.begin(115200);
  delay(1000);
  Serial.println("Serial Active");
  Serial.print("Running v");
  Serial.println(CODEVERSION);

  pinMode(PIN_USHUM, OUTPUT);
  pinMode(PIN_FAN1, OUTPUT);
  pinMode(PIN_FAN2, OUTPUT);
  digitalWrite(PIN_USHUM, LOW);
  digitalWrite(PIN_FAN1, LOW);
  digitalWrite(PIN_FAN2, LOW);
  Serial.println("Pins initialized");

  rtc.begin();
  state.begin();
  sensors.begin();
  storage.begin();
  server.begin();

  storage.getSettings(&state);

  if (error) {
    Serial.print("Error: ");
    Serial.println(error);
  }
}

void loop() {
  if (millis() > loop1Time + LOOP_1_TIME) {
    // Update loop time
    loop1Time = millis();
    // Continually update the co2 readings to maintain callibration
    sensors.updateCO2();
  }

  if (millis() > loop2Time + LOOP_2_TIME) {
    // Update timer
    loop2Time = millis();
    // Get temperature values and update the CO2 sensor with those values
    sensors.updateTAndH();
    sensors.updateCO2Humidity();
  }

  if (millis() > loop3Time + LOOP_3_TIME) {
    // Update timer
    loop3Time = millis();

    state.updateActuatorState();

    digitalWrite(PIN_USHUM, !state.usHumState);
    digitalWrite(PIN_FAN1, !state.fan1State);
    digitalWrite(PIN_FAN2, !state.fan2State);

    Serial.println("Actuator Update");

    storage.datalogEntry(&state);
  }

  if (millis() > loop4Time + LOOP_4_TIME) {
    loop4Time = millis();

    // Error responses
    if (error) {
      Serial.print("Error:");
      Serial.println(error);
      // Reset the CO2 sensor if it is returning bad values
      if (error & CO2ERROR) {
        sensors.initCO2();
      }
      if (error & SDERROR) {
        storage.restartSD();
      }
    }
  }

}
