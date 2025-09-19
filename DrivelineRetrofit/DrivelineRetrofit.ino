#include <Arduino.h>
#include <Wire.h>
#include <FlexCAN_T4.h>
//----------------------------------------------------------------------------//


//----------------------------------------------------------------------------//
#define INTERVAL      15 // ms
#define ADC_PIN_COUNT 6

#define ADC_RESOLUTION 10 // bits

#define MSG_ID 0x18FF50E5 // Extended 29 bit ID

#define LED_PIN 13

const uint8_t ADC_PINS[ADC_PIN_COUNT] = {14, 15, 16, 17, 18, 19};

FlexCAN_T4<CAN3, RX_SIZE_256, TX_SIZE_16> can1;

unsigned long previousMillis = 0;
bool ledState = false;
//----------------------------------------------------------------------------//


//----------------------------------------------------------------------------//
void setup()
{
	can1.begin();
	can1.setBaudRate(500000); // Set CAN bitrate, 500kb

  	Serial.begin(115200);

	pinMode(LED_PIN, OUTPUT);

	for (uint8_t i = 0; i < ADC_PIN_COUNT; i++) {
		pinMode(ADC_PINS[i], INPUT);
	}

}
//----------------------------------------------------------------------------//


//----------------------------------------------------------------------------//
void loop() {
	// Interval timing -------------------------------------------------------//
	unsigned long diff = millis() - previousMillis;
	if (diff < INTERVAL) {
  		return;
  	}
	previousMillis = millis();
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
	can1.write(msg);

	// Toggle LED state ------------------------------------------------------//
  	ledState = !ledState;
  	digitalWrite(LED_PIN, led);
}
//----------------------------------------------------------------------------//
