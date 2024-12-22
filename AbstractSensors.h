
#ifndef ABSTRACT_SENSORS
#define ABSTRACT_SENSORS

#include "Defines.h"
#include "AbstractState.h"
#include <Wire.h>


#define TEMP_HUMID_ADDRESS 0x5C
#define CO2_ADDRESS 0x58
#define CRC_POLYNOMIAL 0x31 //P(x)=x^8+x^5+x^4+1 = 100110001
#define MOVING_AVE_LEN 10
#define DO_NOT_RECORD_INIT 25

class Abstract_Sensors {
private:
  Abstract_State *state;
  float co2MovingAverage[MOVING_AVE_LEN];
  int doNotRecordCounter;
  uint8_t generateCRC(uint8_t byte1, uint8_t byte2);
public:
  Abstract_Sensors(Abstract_State *state);
  void begin();
  void updateTAndH();
  void initCO2();
  void updateCO2Humidity();
  void updateCO2();
};

Abstract_Sensors::Abstract_Sensors(Abstract_State *state) {
  this->state = state;
  for (size_t i = 0; i < MOVING_AVE_LEN; i++) {
    this->co2MovingAverage[i] = 0;
  }
  this->doNotRecordCounter = DO_NOT_RECORD_INIT;
}

void Abstract_Sensors::begin() {
  Wire.begin(PIN_SCL,PIN_SDA);
  this->initCO2();
  Serial.println("Sensors initialized");
}

void Abstract_Sensors::updateTAndH(){
//  Wake the sensor
  Wire.beginTransmission(TEMP_HUMID_ADDRESS);
  Wire.write(0x00);
  Wire.endTransmission();
  delay(10);
  
  Wire.beginTransmission(TEMP_HUMID_ADDRESS);
  Wire.write(0x03);
  Wire.write(0x00);
  Wire.write(0x04);
  Wire.endTransmission();
  
  delay(2);
  Wire.requestFrom(TEMP_HUMID_ADDRESS,6);

  Wire.read();
  Wire.read();
  uint8_t HMSB = Wire.read();
  uint8_t HLSB = Wire.read();
  uint8_t TMSB = Wire.read();
  uint8_t TLSB = Wire.read();
  Wire.read();
  Wire.read();
  Wire.endTransmission();
  
  float hbuf = ((HMSB << 8) | HLSB)/10.0;
  float tbuf = ((TMSB << 8) | TLSB)/10.0;

  if ((hbuf == 6553.5) || (tbuf == 6553.5) || (hbuf == 0)){
    error = error | THERROR;
  } else {
    this->state->humidity = hbuf;
    this->state->temperature = tbuf;
    error = error & ~THERROR;
  }
}

void Abstract_Sensors::initCO2(){
  Wire.beginTransmission(CO2_ADDRESS);
  Wire.write(0x20);
  Wire.write(0x03);
  if (Wire.endTransmission()) {
    error = error | CO2ERROR;
  } else {
    this->doNotRecordCounter = DO_NOT_RECORD_INIT;
    delay(10);
    error = error & ~CO2ERROR;
  }
}

void Abstract_Sensors::updateCO2Humidity(){
  float absHum = 216.7 * (6.112 * exp(this->state->humidity / 100.0 * 17.62 * this->state->temperature / (243.12 + this->state->temperature)))/(273.15+this->state->temperature);

  uint8_t AhMsb = min((int)absHum,255);
  uint8_t AhLsb = ((absHum-((int)absHum))*256.0);
  uint8_t crcbyte = generateCRC(AhMsb,AhLsb);

  Wire.beginTransmission(CO2_ADDRESS);
  Wire.write(0x20);
  Wire.write(0x61);
  Wire.write(AhMsb);
  Wire.write(AhLsb);
  Wire.write(crcbyte);
  delay(10);
  Wire.endTransmission();
}

void Abstract_Sensors::updateCO2(){
  Wire.beginTransmission(CO2_ADDRESS);
  Wire.write(0x20);
  Wire.write(0x08);
  if (Wire.endTransmission(false)) {
    error = error | CO2ERROR;
    return;
  } else {
    error = error & ~CO2ERROR;
  }
  delay(10);
  Wire.requestFrom(CO2_ADDRESS,6,false);

  uint8_t CoeMsb = Wire.read();
  uint8_t CoeLsb = Wire.read();
  Wire.read();
  uint8_t TvocMsb = Wire.read();
  uint8_t TvocLsb = Wire.read();
  Wire.read();
  if (Wire.endTransmission()) {
    error = error | CO2ERROR;
  } else {
    error = error & ~CO2ERROR;
  }

  float cbuf = ((CoeMsb << 8) | CoeLsb);
  float tbuf = ((TvocMsb << 8) | TvocLsb);

  if (((cbuf ==  400) && (tbuf ==  0)) || ((cbuf == this->state->co2Level) && (tbuf == this->state->TVOCLevel)) || (cbuf ==  0xffff) || (tbuf ==  0xffff)) {
    this->doNotRecordCounter --;
    if (this->doNotRecordCounter < -DO_NOT_RECORD_INIT) {
      error = error | CO2ERROR;
    } else {
      error = error & ~CO2ERROR;
    }
  } else {
    this->doNotRecordCounter = 0;
    this->state->co2Level = 0;
    for (int i = MOVING_AVE_LEN-2; i >= 0; i--) {
      this->co2MovingAverage[i+1] = this->co2MovingAverage[i];
      this->state->co2Level += this->co2MovingAverage[i];
    }
    this->co2MovingAverage[0] = cbuf;
    this->state->co2Level += this->co2MovingAverage[0];
    this->state->co2Level /= MOVING_AVE_LEN;
    this->state->TVOCLevel = tbuf;
  }

}

uint8_t Abstract_Sensors::generateCRC(uint8_t byte1, uint8_t byte2) {
  // calculates 8-Bit checksum with given polynomial
  uint8_t crc = 0xFF;

  crc ^= byte1;
  for (uint8_t b = 0; b < 8; b++) {
    if (crc & 0x80)
      crc = (crc << 1) ^ CRC_POLYNOMIAL;
    else
      crc <<= 1;
  }

  crc ^= byte2;
  for (uint8_t b = 0; b < 8; b++) {
    if (crc & 0x80)
      crc = (crc << 1) ^ CRC_POLYNOMIAL;
    else
      crc <<= 1;
  }
  return crc;
}

#endif
