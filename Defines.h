
#ifndef MUSHROOMDEFINITIONS
#define MUSHROOMDEFINITIONS

#define CODEVERSION 11.0

#define LOOP_1_TIME 1000
#define LOOP_2_TIME 10000
#define LOOP_3_TIME 120000
#define LOOP_4_TIME 30000
#define RTC_UPDATE_DELAY_MS 60000

#define PIN_CHIPSELECT D8
#define PIN_FAN1 0
#define PIN_FAN2 2
#define PIN_USHUM 16
#define PIN_SDA 5
#define PIN_SCL 4

#define THERROR 1
#define SDERROR 1<<2
#define CO2ERROR 1<<1
#define RTCERROR 1<<3
#define SPIFFSERROR 1<<4

#define DATALOG_FILE "/mushrooms"
#define SETTINGS_FILE "settings.txt"
#define HTML_FILE "/main.html"

#define APSSID "FruitingChamberAP"
#define PASSWORD "MuchRoom"

uint8_t error = 0x00;

#endif
