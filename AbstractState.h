#ifndef ABSTRACT_STATE
#define ABSTRACT_STATE

#include "Defines.h"
#include "AbstractRTC.h"

class Abstract_State {
public:
  Abstract_RTC *rtc;
  float temperature;
  float humidity;
  float co2Level;
  float TVOCLevel;
  int usHumState;
  int fan1State;
  int fan2State;
  float humiditySetPoint;
  float co2SetPoint;
  float f1Withf2;

  Abstract_State(Abstract_RTC *rtc);
  void begin();
  String getDataString();
  String getSettingsString();
  void updateActuatorState();

};


Abstract_State::Abstract_State(Abstract_RTC *rtc) {
  this->rtc = rtc;
  this->temperature = 0;
  this->humidity = 0;
  this->co2Level = 0;
  this->TVOCLevel = 0;
  this->usHumState = 1;
  this->fan1State = 1;
  this->fan2State = 1;

  this->humiditySetPoint = 90;
  this->co2SetPoint = 600;
  this->f1Withf2 = 2000;
}

void Abstract_State::begin() {
  Serial.println("State initialized");
}

String Abstract_State::getDataString() {
  String dataString(this->rtc->tnow().unixtime());
  dataString += ',' + String(this->temperature);
  dataString += ',' + String(this->humidity);
  dataString += ',' + String(this->co2Level);
  dataString += ',' + String(this->TVOCLevel);
  dataString += ',' + String(this->usHumState);
  dataString += ',' + String(this->fan1State);
  dataString += ',' + String(this->fan2State);
  dataString += ',' + String(error);
  return dataString + '\n';
}

String Abstract_State::getSettingsString() {
  String dataString(this->rtc->tnow().unixtime());
  dataString += ',' + String(this->humiditySetPoint);
  dataString += ',' + String(this->co2SetPoint);
  dataString += ',' + String(this->f1Withf2);
  return dataString;
}

void Abstract_State::updateActuatorState() {
  // Set pins according to data
  if ((this->humidity < this->humiditySetPoint) || (error & THERROR)) {
    this->usHumState = 1;
  } else {
    this->usHumState = 0;
  }

  if ((this->co2Level > this->co2SetPoint) || (error & CO2ERROR)) {
    this->fan2State = 1;
  } else {
    this->fan2State = 0;
  }

  if ((this->usHumState == 1) || (this->co2Level > this->f1Withf2)) {
    this->fan1State = 1;
  } else {
    this->fan1State = 0;
  }
}

#endif
