#include <Arduino.h>
#include <Wire.h>
#include <SD.h>
// #include <FlexCAN_T4.h>
#include <HX711_ADC.h>
//----------------------------------------------------------------------------//


//----------------------------------------------------------------------------//
#define INTERVAL      15 // ms
// #define CAN_BAUDRATE 500000 // 500 kbps
// #define MSG_ID 0x18FF50E5 // Extended 29 bit ID
#define SD_CARD_FILE_NAME "dline.csv"
#define LED_PIN 13
//----------------------------------------------------------------------------//


//----------------------------------------------------------------------------//
const int DOUT_FRONT_LEFT = 2;
const int SCK_FRONT_LEFT  = 4;
const int DOUT_REAR_LEFT  = 6;
const int SCK_REAR_LEFT   = 8;

const int DOUT_FRONT_RIGHT = 9;
const int SCK_FRONT_RIGHT  = 10;
const int DOUT_REAR_RIGHT  = 14;
const int SCK_REAR_RIGHT   = 15;

HX711_ADC loadFrontLeft(DOUT_FRONT_LEFT, SCK_FRONT_LEFT);
HX711_ADC loadRearLeft(DOUT_REAR_LEFT,  SCK_REAR_LEFT);

HX711_ADC loadFrontRight(DOUT_FRONT_RIGHT, SCK_FRONT_RIGHT);
HX711_ADC loadRearRight(DOUT_REAR_RIGHT,  SCK_REAR_RIGHT);

// float CALIB_FRONT_LEFT = -376.11;
// float CALIB_REAR_LEFT  = -380.880615;

// float CALIB_FRONT_RIGHT = -571.0;
// float CALIB_REAR_RIGHT  = -766.15;
float CALIB_FRONT_LEFT = 1.0;
float CALIB_REAR_LEFT  = 1.0;

float CALIB_FRONT_RIGHT = 1.0;
float CALIB_REAR_RIGHT  = 1.0;

// const float KNOWN_MASS_KG = 11.3398f;
// const float GRAVITY = 9.80665f;

float g_tareFrontLeft = 0.0;
float g_tareRearLeft = 0.0;

float g_tareFrontRight= 0.0;
float g_tareRearRight = 0.0;

// const int SMOOTH_SAMPLES = 5;
//----------------------------------------------------------------------------//

// FlexCAN_T4<CAN3, RX_SIZE_256, TX_SIZE_16> can;
unsigned long g_previousMillis = 0;
bool g_ledState = false;
bool g_sdCardInitialized = false;
//----------------------------------------------------------------------------//


//----------------------------------------------------------------------------//
void setup()
{
	// Initialize serial
  	Serial.begin(115200);
	Serial.println("Driveline Retrofit Board Starting...");
	
	// Initialize HX711 load cells -------------------------------------------//
	Serial.println("Initializing HX711 load cells...");
	loadFrontLeft.begin();
	loadRearLeft.begin();
	loadFrontRight.begin();
	loadRearRight.begin();
	delay(200); // Allow HX711 to settle
	
	// Tare load cells -------------------------------------------------------//
	// Serial.println("Taring load cells...");
	// loadFrontLeft.start(2000);
	// while (!loadFrontLeft.update()) {
	// 	// Wait for tare to complete
	// }
	// g_tareFrontLeft = loadFrontLeft.getData();
	// Serial.print("Front Left tare: ");
	// Serial.println(g_tareFrontLeft, 3);
	
	// loadRearLeft.start(2000);
	// while (!loadRearLeft.update()) {
	// 	// Wait for tare to complete
	// }
	// g_tareRearLeft = loadRearLeft.getData();
	// Serial.print("Rear Left tare: ");
	// Serial.println(g_tareRearLeft, 3);

	// loadFrontRight.start(2000);
	// while (!loadFrontRight.update()) {
	// 	// Wait for tare to complete
	// }
	// g_tareFrontRight = loadFrontRight.getData();
	// Serial.print("Front Right tare: ");
	// Serial.println(g_tareFrontRight, 3);

	// loadRearRight.start(2000);
	// while (!loadRearRight.update()) {
	// 	// Wait for tare to complete
	// }
	// g_tareRearRight = loadRearRight.getData();
	// Serial.print("Rear Right tare: ");
	// Serial.println(g_tareRearRight, 3);

	// Serial.println("Tare complete.");
	
	// Initialize CAN bus ----------------------------------------------------//
	// Serial.println("Initializing CAN...");
	// can.begin();
	// can.setBaudRate(CAN_BAUDRATE);
	
	// Configure pins --------------------------------------------------------//
	Serial.println("Configuring pins...");
	pinMode(LED_PIN, OUTPUT);
	
	// Initialize SD card ----------------------------------------------------//
	Serial.println("Initializing SD card...");
	if (!SD.begin(BUILTIN_SDCARD)) {
		Serial.println("Card failed, or not present.");
		g_sdCardInitialized = false;
	} else {
		Serial.println("Card initialized.");
		g_sdCardInitialized = true;
	}

	Serial.println("Setup complete.");
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
	
	// Read HX711 load cells with smoothing ----------------------------------//
	// double rawFrontSum = 0;
	// double rawRearSum = 0;
	
	// for (int i = 0; i < SMOOTH_SAMPLES; i++) {
	// 	loadFront.update();
	// 	loadRear.update();
	// 	rawFrontSum += loadFront.getData();
	// 	rawRearSum += loadRear.getData();
	// }
	
	// Average readings
	// double rawFrontAvg = rawFrontSum / SMOOTH_SAMPLES;
	// double rawRearAvg = rawRearSum / SMOOTH_SAMPLES;

	loadFrontLeft.update();
	loadRearLeft.update();
	loadFrontRight.update();
	loadRearRight.update();
	float rawFrontLeft = loadFrontLeft.getData();
	float rawRearLeft  = loadRearLeft.getData();
	float rawFrontRight = loadFrontRight.getData();
	float rawRearRight = loadRearRight.getData();
	
	// Convert to Newtons
	float forceFrontLeft = (rawFrontLeft - g_tareFrontLeft) / CALIB_FRONT_LEFT;
	float forceRearLeft = (rawRearLeft - g_tareRearLeft) / CALIB_REAR_LEFT;
	float forceFrontRight = (rawFrontRight - g_tareFrontRight) / CALIB_FRONT_RIGHT;
	float forceRearRight = (rawRearRight - g_tareRearRight) / CALIB_REAR_RIGHT;

	// Serial print for debugging --------------------------------------------//
	Serial.print("Time (ms): ");
	Serial.print(millis());
	Serial.print(", Front Left: ");
	Serial.print(forceFrontLeft, 3);
	Serial.print(", Rear Left: ");
	Serial.println(forceRearLeft, 3);
	Serial.print(", Front Right: ");
	Serial.print(forceFrontRight, 3);
	Serial.print(", Rear Right: ");
	Serial.println(forceRearRight, 3);
	
	// // Convert force values to integers for CAN packing (scale as needed)
	// // You may want to adjust scaling factor based on your force range
	// int16_t forceFrontInt = (int16_t)(forceFront * 100); // 0.01 N resolution
	// int16_t forceRearInt = (int16_t)(forceRear * 100);   // 0.01 N resolution
	
	// Pack force readings into CAN message ----------------------------------//
	// uint8_t msgData[8] = { 0 };
	
	// // Pack front force (16 bits)
	// msgData[0] = forceFrontInt & 0xFF;
	// msgData[1] = (forceFrontInt >> 8) & 0xFF;
	
	// // Pack rear force (16 bits)
	// msgData[2] = forceRearInt & 0xFF;
	// msgData[3] = (forceRearInt >> 8) & 0xFF;
	
	// // Remaining 4 bytes available for future use or additional sensors
	// msgData[4] = 0;
	// msgData[5] = 0;
	// msgData[6] = 0;
	// msgData[7] = 0;
	
	// Send CAN message ------------------------------------------------------//
	// CAN_message_t msg;
	// msg.id = MSG_ID;
	// msg.len = 8;
	// memcpy(msg.buf, msgData, 8);
	// msg.flags.extended = true;
	// can.write(msg);
	
	// Log to SD card if initialized -----------------------------------------//
	if (g_sdCardInitialized) {
		File logFile = SD.open(SD_CARD_FILE_NAME, FILE_WRITE);
		if (logFile) {
			logFile.print(millis());
			logFile.print(",");
			logFile.print(forceFrontLeft, 3);
			logFile.print(",");
			logFile.print(forceRearLeft, 3);
			logFile.print(",");
			logFile.print(forceFrontRight, 3);
			logFile.print(",");
			logFile.println(forceRearRight, 3);
			logFile.close();
		} else {
			Serial.println("Error opening log file.");
		}
	}
	
	// Toggle LED state ------------------------------------------------------//
  	g_ledState = !g_ledState;
  	digitalWrite(LED_PIN, g_ledState);
}
//----------------------------------------------------------------------------//





