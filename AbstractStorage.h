#ifndef ABSTRACT_STORAGE
#define ABSTRACT_STORAGE

#include "Defines.h"
#include "AbstractState.h"

#include <SPI.h>
#include <SD.h>
#include <FS.h>


class Abstract_Storage {
public:
  Abstract_Storage();
  void begin();
  void restartSD();
  void datalogEntry(Abstract_State *);
  void setSettings(Abstract_State *);
  void getSettings(Abstract_State *);
  File returnDatalogFile(Abstract_State *, int);
};

Abstract_Storage::Abstract_Storage() {
  Serial.println("Storage initialized");
}

void Abstract_Storage::begin(){
  // Initialise SD card
  if (!SD.begin(PIN_CHIPSELECT)) {
    Serial.println("Card failed, or not present");
    error = error | SDERROR;
  } else {
    error = error & ~SDERROR;
    if (!SD.open(DATALOG_FILE)) {
     SD.mkdir(DATALOG_FILE);
    }
   Serial.println("Card initialized");
  }

  // Onboard file system
  SPIFFS.begin();
  Serial.println("SPIFFS Initalised");
  
}

void Abstract_Storage::restartSD() {
  // Initialise SD card
  if (!SD.begin(PIN_CHIPSELECT)) {
    Serial.println("Card failed, or not present");
    error = error | SDERROR;
  } else {
    error = error & ~SDERROR;
   Serial.println("Card initialized");
  }
}

void Abstract_Storage::datalogEntry(Abstract_State *state){
  String filename = dataFileName(state->rtc->tnow());
  String text = state->getDataString();

  if (!(error & SDERROR)) {
    File file = SD.open(filename, FILE_WRITE);
    if (file) {
      if (file.size() != 0){
        file.seek(file.size()-1);
        file.print('\n');
      }
      file.print(text);
    } else {
      Serial.print("error opening ");
      Serial.println(filename);
      error = error | SDERROR;
    }
    file.close();
  }
}

void Abstract_Storage::setSettings(Abstract_State *state){
  String text = state->getSettingsString();
  SPIFFS.remove(SETTINGS_FILE);
  File file = SPIFFS.open(SETTINGS_FILE, "w");
  if (file) {
    file.print(text);
  } else {
    Serial.print("error opening ");
    Serial.println(SETTINGS_FILE);
  }
  file.close();
}

void Abstract_Storage::getSettings(Abstract_State *state){
  File file = SPIFFS.open(SETTINGS_FILE, "r");
  int commaCount = 0;
  String text = "";
  while (file.available()) {
    char c = file.read();
    if (c == ',') {
      commaCount ++;
      if (commaCount == 2) {
        state->humiditySetPoint = text.toFloat();
      } else if (commaCount == 3) {
        state->co2SetPoint = text.toFloat();
      } else if (commaCount == 4) {
        state->f1Withf2 = text.toFloat();
      }
      text = "";
    } else {
      text += c;
    }
  }

}

File Abstract_Storage::returnDatalogFile(Abstract_State *state, int sub){
    String filename = dataFileName(state->rtc->tnow()-TimeSpan(sub,0,0,0));
    File file = SD.open(filename, FILE_READ);
    return file;
}

#endif
