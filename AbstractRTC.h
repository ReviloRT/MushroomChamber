
#ifndef ABSTRACT_RTC
#define ABSTRACT_RTC

#include "Defines.h"
#include <RTClib.h>

String dataFileName(DateTime targetTime) {
  return String(DATALOG_FILE) + "/data" + String(targetTime.year()) + "_"  + String(targetTime.month()) + "_" + String(targetTime.day()) + ".csv";
}

class Abstract_RTC {
private:
  RTC_DS3231 rtc;
  DateTime RTCTime;
  unsigned long lastRTCUpdate;

public:
  Abstract_RTC();
  void begin();
  DateTime tnow();
  void updateRTC();
};

Abstract_RTC::Abstract_RTC() {
  this->lastRTCUpdate = 0;
}

void Abstract_RTC::begin() {
  this->rtc.begin();
  delay(10);
  this->RTCTime = this->rtc.now();
  this->lastRTCUpdate = millis();
  Serial.print("Current RTC date/time: ");
  Serial.println(this->RTCTime.timestamp());
  this->updateRTC();
}

DateTime Abstract_RTC::tnow() {
  if (millis()-this->lastRTCUpdate > RTC_UPDATE_DELAY_MS) {
    this->RTCTime = this->rtc.now();
    Serial.print("Current RTC date/time: ");
    Serial.println(this->RTCTime.timestamp());
    this->lastRTCUpdate = millis();
    if (!this->RTCTime.isValid()) {
      error = error | RTCERROR;
    }
  }
  return this->RTCTime+TimeSpan((millis()-this->lastRTCUpdate)/1000);
}

void Abstract_RTC::updateRTC() {
  if (!(error & RTCERROR)) {
    Serial.println("Update Time? ('yes'/any)");
    delay(2000);
    String buf = "";
    while (Serial.available()) {
      buf += Serial.read();
    }
    if (buf == "12110111510") { // 12110111510 == "yes"
      // ask user to enter new date and time
      const char txt[6][15] = { "year [4-digit]", "month [1~12]", "day [1~31]",
                                "hours [0~23]", "minutes [0~59]", "seconds [0~59]"};
      String str = "";
      long newDate[6];

      while (Serial.available()) {
        Serial.read();  // clear serial buffer
      }

      for (int i = 0; i < 6; i++) {

        Serial.print("Enter ");
        Serial.print(txt[i]);
        Serial.print(": ");

        while (!Serial.available()) {
          ; // wait for user input
        }

        str = Serial.readString();  // read user input
        newDate[i] = str.toInt();   // convert user input to number and save to array

        Serial.println(newDate[i]); // show user input
      }

      // update RTC
      this->rtc.adjust(DateTime(newDate[0], newDate[1], newDate[2], newDate[3], newDate[4], newDate[5]));
      delay(10);
      this->RTCTime = this->rtc.now();
      this->lastRTCUpdate = millis();
      Serial.print("RTC Updated to date/time: ");
      Serial.println(this->RTCTime.timestamp());
    } else {
      Serial.println("Continuing");
    }
  }
}

#endif
