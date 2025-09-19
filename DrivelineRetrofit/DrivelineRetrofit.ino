#include <Arduino.h>
#include <Wire.h>
#include <SD.h>
#include <FlexCAN_T4.h>
//----------------------------------------------------------------------------//


//----------------------------------------------------------------------------//
#define INTERVAL      15 // ms
#define ADC_PIN_COUNT 6

#define CAN_BAUDRATE 500000 // 500 kbps

#define ADC_RESOLUTION 10 // bits

#define MSG_ID 0x18FF50E5 // Extended 29 bit ID

#define SD_CARD_FILE_NAME "dline.csv"

#define LED_PIN 13

const uint8_t ADC_PINS[ADC_PIN_COUNT] = {14, 15, 16, 17, 18, 19};

FlexCAN_T4<CAN3, RX_SIZE_256, TX_SIZE_16> can;

unsigned long g_previousMillis = 0;
bool g_ledState = false;
bool g_sdCardInitialized = false;
//----------------------------------------------------------------------------//


//----------------------------------------------------------------------------//
void setup()
{
  	Serial.begin(115200);

	Serial.println("Driveline Retrofit Board Starting...");

	Serial.println("Initializing CAN...");
	can.begin();
	can.setBaudRate(CAN_BAUDRATE);


	Serial.println("Configuring pins...");
	pinMode(LED_PIN, OUTPUT);

	for (uint8_t i = 0; i < ADC_PIN_COUNT; i++) {
		pinMode(ADC_PINS[i], INPUT);
	}

	Serial.println("Initializing SD card...");
	if (!SD.begin(BUILTIN_SDCARD)) {
		Serial.println("Card failed, or not present.");
		g_sdCardInitialized = false;
	} else {
		Serial.println("Card initialized.");
		g_sdCardInitialized = true;
	}



}
//----------------------------------------------------------------------------//


//----------------------------------------------------------------------------//
void loop() {
	// Interval timing -------------------------------------------------------//
	unsigned long diff = millis() - g_previousMillis;
	if (diff < INTERVAL) {
  		return;
  	}
	g_previousMillis = millis();
	Serial.print("Interval: ");
	Serial.println(diff);

	// Read ADC channels -----------------------------------------------------//
	int readings[ADC_PIN_COUNT] = { 0 };
	for (uint8_t i = 0; i < ADC_PIN_COUNT; i++) {
		readings[i] = analogRead(ADC_PINS[i]);
	}

	// Pack ADC readings into CAN message ------------------------------------//
	uint8_t msgData[8] = { 0 };
	uint16_t bitPos = 0;
	for (uint8_t i = 0; i < ADC_PIN_COUNT; i++) {
		uint16_t val = readings[i];
		for (uint8_t b = 0; b < ADC_RESOLUTION; b++) {
			if (val & (1 << b)) {
				msgData[bitPos / 8] |= (1 << (bitPos % 8));
			}
			bitPos++;
		}
	}

	// Send CAN message ------------------------------------------------------//
	CAN_message_t msg;
	msg.id = MSG_ID;
	msg.len = 8;
	memcpy(msg.buf, msgData, 8);
	msg.flags.extended = true;
	can.write(msg);

	// Log to SD card if initialized -----------------------------------------//
	if (g_sdCardInitialized) {
		File logFile = SD.open(SD_CARD_FILE_NAME, FILE_WRITE);
		if (logFile) {
			logFile.print(millis());
			for (uint8_t i = 0; i < ADC_PIN_COUNT; i++) {
				logFile.print(",");
				logFile.print(readings[i]);
			}
			logFile.println();
			logFile.close();
		} else {
			Serial.println("Error opening log file.");
		}
	}

	// Toggle LED state ------------------------------------------------------//
  	g_ledState = !g_ledState;
  	digitalWrite(LED_PIN, led);
}
//----------------------------------------------------------------------------//
