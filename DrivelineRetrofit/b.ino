// #include <HX711_ADC.h>
// // -------------------- Pin definitions (per Teensy) --------------------
// // Front load cell
// const int DOUT_FRONT = 2;
// const int SCK_FRONT  = 4;
// // Rear load cell
// const int DOUT_REAR  = 6;
// const int SCK_REAR   = 8;
// // -------------------- HX711 objects --------------------
// HX711_ADC loadFront(DOUT_FRONT, SCK_FRONT);
// HX711_ADC loadRear (DOUT_REAR,  SCK_REAR);
// // -------------------- Calibration factors (counts per Newton) --------------------
// float calibFront = 2800.0;
// float calibRear  = -380.880615;
// // -------------------- Tare values --------------------
// long tareFront = 0, tareRear = 0;
// // -------------------- Smoothing --------------------
// const int smoothSamples = 5;
// // -------------------- Calibration weight --------------------
// const float KNOWN_MASS_KG = 11.3398f;
// const float g = 9.80665f;
// // -------------------- Helper --------------------
// long readAverage(HX711_ADC& dev, int samples) {
//   long sum = 0;
//   for (int i = 0; i < samples; i++) {
//     dev.update();
//     sum += dev.getData();
//   }
//   return sum / samples;
// }
// void setup() {
//   Serial.begin(115200);
//   while (!Serial) { delay(10); }
//   loadFront.begin();
//   loadRear.begin();
//   delay(200);
//   // Tare
//   Serial.println("Taring load cells...");
//   loadFront.start(2000); while (!loadFront.update()) {} tareFront = loadFront.getData();
//   loadRear.start(2000);  while (!loadRear.update())  {} tareRear  = loadRear.getData();
//   Serial.println("Tare complete.");
//   Serial.println("Type '?' for commands.");
//   // ---- CHANGE THIS HEADER per Teensy ----
//   Serial.println("time_ms,force_front_left_N,force_rear_left_N");
// }
// void loop() {
//   if (Serial.available()) {
//     char c = Serial.read();
//     if (c == '?') {
//       Serial.println("Commands:");
//       Serial.println("f = calibrate Front");
//       Serial.println("r = calibrate Rear");
//       Serial.println("z = re-tare both");
//       Serial.println("? = help");
//     }
//     else if (c == 'z') {
//       Serial.println("Re-taring...");
//       loadFront.start(1500); while (!loadFront.update()) {} tareFront = loadFront.getData();
//       loadRear.start(1500);  while (!loadRear.update())  {} tareRear  = loadRear.getData();
//       Serial.println("Tare updated.");
//     }
//     else if (c == 'f') {
//       Serial.println("Place known mass on FRONT");
//       long raw = readAverage(loadFront, 120);
//       long delta = raw - tareFront;
//       float newCal = delta / (KNOWN_MASS_KG * g);
//       Serial.print("calibFront = "); Serial.println(newCal, 6);
//     }
//     else if (c == 'r') {
//       Serial.println("Place known mass on REAR");
//       long raw = readAverage(loadRear, 120);
//       long delta = raw - tareRear;
//       float newCal = delta / (KNOWN_MASS_KG * g);
//       Serial.print("calibRear = "); Serial.println(newCal, 6);
//     }
//   }
//   // Read both load cells
//   long rawFront = readAverage(loadFront, smoothSamples);
//   long rawRear  = readAverage(loadRear, smoothSamples);
//   float forceFront = (rawFront - tareFront) / calibFront;
//   float forceRear  = (rawRear  - tareRear)  / calibRear;
//   // ---- CHANGE THESE LABELS per Teensy ----
//   unsigned long t = millis();
//   Serial.print(t); Serial.print(",");
//   Serial.print(forceFront, 3); Serial.print(",");
//   Serial.println(forceRear, 3);
//   delay(10); // ~100 Hz
// }