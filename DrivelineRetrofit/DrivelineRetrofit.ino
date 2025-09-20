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
const int DOUT_FRONT = 2;
const int SCK_FRONT  = 4;
const int DOUT_REAR  = 6;
const int SCK_REAR   = 8;

HX711_ADC loadFront(DOUT_FRONT, SCK_FRONT);
HX711_ADC loadRear (DOUT_REAR,  SCK_REAR);

float CALIB_FRONT = 2800.0;
float CALIB_REAR  = -380.880615;

long TARE_FRONT = 0;
long TARE_REAR = 0;

const int SMOOTH_SAMPLES = 5;
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
	loadFront.begin();
	loadRear.begin();
	delay(200); // Allow HX711 to settle
	
	// Tare load cells -------------------------------------------------------//
	Serial.println("Taring load cells...");
	loadFront.start(2000);
	while (!loadFront.update()) {
		// Wait for tare to complete
	}
	TARE_FRONT = loadFront.getData();
	
	loadRear.start(2000);
	while (!loadRear.update()) {
		// Wait for tare to complete
	}
	TARE_REAR = loadRear.getData();
	Serial.println("Tare complete.");
	
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
	long rawFrontSum = 0;
	long rawRearSum = 0;
	
	for (int i = 0; i < SMOOTH_SAMPLES; i++) {
		loadFront.update();
		loadRear.update();
		rawFrontSum += loadFront.getData();
		rawRearSum += loadRear.getData();
	}
	
	// Average readings
	long rawFrontAvg = rawFrontSum / SMOOTH_SAMPLES;
	long rawRearAvg = rawRearSum / SMOOTH_SAMPLES;
	
	// Convert to Newtons
	float forceFront = (rawFrontAvg - TARE_FRONT) / CALIB_FRONT;
	float forceRear = (rawRearAvg - TARE_REAR) / CALIB_REAR;
	
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
			logFile.print(forceFront, 3);
			logFile.print(",");
			logFile.println(forceRear, 3);
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