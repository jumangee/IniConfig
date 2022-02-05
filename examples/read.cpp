#include <Arduino.h>

#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32

#include <IniConfig.h>

#define DEBUG_SERIAL	1
#if DEBUG_SERIAL == 1
	#define DEBUGLN(t) Serial.println(t);
	#define DEBUG(t) Serial.print(t);
#else
	#define DEBUGLN(t) /**/
	#define DEBUG(t) /**/
#endif

#define GETFREERAM DEBUG("--- Free RAM: "); DEBUGLN(ESP.getFreeHeap());

uint32_t startFreeMem;

bool init_microSD_card( void ){
	DEBUGLN("Starting SD Card");

	if (!SD_MMC.begin("/sdcard", true))	{
		DEBUGLN("SD Card Mount Failed");
		return false;
	}

	uint8_t cardType = SD_MMC.cardType();
	if(cardType == CARD_NONE){
		DEBUGLN("No SD Card attached");
		return false;
	}

	DEBUGLN("Ok!");
	return true;
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting!");

	if (!init_microSD_card()) {
        DEBUGLN("SD init fail");
		delay(100000);
		ESP.restart();
		return;
	}
}

void loop() {
	GETFREERAM
	startFreeMem = ESP.getFreeHeap();
    DEBUGLN("...");

    IniConfig* cfg = new IniConfig(F("/example1.ini"), &SD_MMC);

    DEBUG("last_update=");
    DEBUGLN(cfg->getString(F("main"), F("last_update"), F("")));
    
    DEBUG("test=");
    DEBUGLN(cfg->getString(F("main"), F("test"), F("default")));

    DEBUG("size=");
    DEBUGLN(cfg->getInt(F("int"), F("size"), 0));

	delete cfg;

    DEBUGLN("*********");

	delay(5000);
}