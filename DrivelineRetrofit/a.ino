#include <HX711_ADC.h>
// -------------------- Pin definitions --------------------
const int DOUT_FRONT = 3;
const int CLOCK_FRONT = 2;
const int DOUT_REAR = 4;
const int CLOCK_REAR = 5;
// -------------------- HX711 objects --------------------
HX711_ADC loadFront(DOUT_FRONT, CLOCK_FRONT);
HX711_ADC loadRear(DOUT_REAR, CLOCK_REAR);
// -------------------- Calibration factors (counts per Newton) --------------------
float calibFront = 2843.8; // replace with your measured factor
float calibRear = 2900.5;  // replace with your measured factor
// -------------------- Tare values (raw counts at zero load) --------------------
long tareFront = 0;
long tareRear = 0;
// -------------------- Optional smoothing --------------------
const int smoothSamples = 5;
void setup()
{
	Serial.begin(115200);
	while (!Serial)
	{
		delay(10);
	}
	// Initialize load cells
	loadFront.begin();
	loadRear.begin();
	delay(200); // allow HX711 to settle
	// ----- Tare procedure -----
	Serial.println("Taring load cells...");
	loadFront.start(2000);
	while (!loadFront.update())
	{
	}
	tareFront = loadFront.getData();
	loadRear.start(2000);
	while (!loadRear.update())
	{
	}
	tareRear = loadRear.getData();
	Serial.println("Tare complete.");
	Serial.println("time_ms,force_front_N,force_rear_N"); // CSV header
}
void loop()
{
	long rawFrontSum = 0;
	long rawRearSum = 0;
	// Read multiple samples for smoothing
	for (int i = 0; i < smoothSamples; i++)
	{
		loadFront.update();
		loadRear.update();
		rawFrontSum += loadFront.getData();
		rawRearSum += loadRear.getData();
	}
	// Average readings
	long rawFrontAvg = rawFrontSum / smoothSamples;
	long rawRearAvg = rawRearSum / smoothSamples;
	// Convert to Newtons
	float forceFront = (rawFrontAvg - tareFront) / calibFront;
	float forceRear = (rawRearAvg - tareRear) / calibRear;
	// Print CSV: front and rear
	unsigned long t = millis();
	Serial.print(t);
	Serial.print(",");
	Serial.print(forceFront, 3);
	Serial.print(",");
	Serial.println(forceRear, 3);
	delay(10); // ~100 Hz output, adjust if needed
}