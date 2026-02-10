/*
 * ESP32 Smart Dustbin with Ultrasonic Sensor and Firebase
 * 
 * This sketch measures dustbin fill level using an ultrasonic sensor (HC-SR04)
 * and writes the fill percentage to Firebase Realtime Database at /dustbin node
 * 
 * Hardware Connections:
 * - Ultrasonic Sensor VCC -> 5V (or 3.3V)
 * - Ultrasonic Sensor GND -> GND
 * - Ultrasonic Sensor Trig -> GPIO 5 (or any digital pin)
 * - Ultrasonic Sensor Echo -> GPIO 18 (or any digital pin)
 * 
 * Note: Change DUSTBIN_HEIGHT_CM variable to match your dustbin height
 */

#include <WiFi.h>
#include <Firebase_ESP_Client.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "Airtel_M-1102"
#define WIFI_PASSWORD "amc@m1102"

/* 2. Define the API Key */
#define API_KEY "AIzaSyA27lgwgBhByNuye_b5hVOZTeK7IjLLbis"

/* 3. Define the RTDB URL */
#define DATABASE_URL "https://saarthi-84622-default-rtdb.asia-southeast1.firebasedatabase.app/"

/* 4. Define the user Email and password that already registered or added in your project */
#define USER_EMAIL "naman.saarthi@gmail.com"
#define USER_PASSWORD "12345678"

// Ultrasonic Sensor pins
#define TRIG_PIN 5   // Trigger pin
#define ECHO_PIN 18  // Echo pin

// ============================================
// CONFIGURE DUSTBIN HEIGHT HERE (in cm)
// ============================================
#define DUSTBIN_HEIGHT_CM 50.0  // Change this to your dustbin height in centimeters
// ============================================

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
const unsigned long sendInterval = 2000; // Send data every 2 seconds

// Variables for ultrasonic sensor
float distance = 0.0;
float fillLevel = 0.0;  // Fill percentage (0-100%)

void setup() {
  Serial.begin(115200);
  
  // Initialize ultrasonic sensor pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  Serial.println();
  Serial.println("ESP32 Smart Dustbin with Firebase");
  Serial.println("==================================");
  Serial.print("Dustbin Height: ");
  Serial.print(DUSTBIN_HEIGHT_CM);
  Serial.println(" cm");
  Serial.print("Ultrasonic Sensor - Trig: GPIO ");
  Serial.print(TRIG_PIN);
  Serial.print(" | Echo: GPIO ");
  Serial.println(ECHO_PIN);
  Serial.println();
  
  // Connect to WiFi
  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(WIFI_SSID);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  
  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  // Comment or pass false value when WiFi reconnection will control by your code or third party library
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

  // Limit the size of response payload to be collected in FirebaseData
  fbdo.setResponseSize(2048);

  Firebase.begin(&config, &auth);

  Firebase.setDoubleDigits(5);

  config.timeout.serverResponse = 10 * 1000;
  
  Serial.println("Firebase initialized!");
  Serial.println("Starting dustbin monitoring...");
  Serial.println();
}

// Read distance from ultrasonic sensor (returns distance in cm)
float readUltrasonicDistance() {
  // Clear trigger pin
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  
  // Send 10us pulse to trigger pin
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // Read echo pin - returns duration in microseconds
  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout (max ~5m range)
  
  // Calculate distance in cm
  // Speed of sound = 343 m/s = 0.0343 cm/us
  // Distance = (duration * speed of sound) / 2 (round trip)
  float distance_cm = (duration * 0.0343) / 2.0;
  
  // If reading is invalid (timeout or too far), return previous value
  if (duration == 0 || distance_cm > DUSTBIN_HEIGHT_CM * 1.5) {
    return distance; // Return previous valid reading
  }
  
  return distance_cm;
}

// Calculate fill level percentage
float calculateFillLevel(float distance_cm) {
  // Distance from sensor to top of garbage
  // If distance is close to 0, dustbin is full
  // If distance is close to DUSTBIN_HEIGHT_CM, dustbin is empty
  
  // Clamp distance to valid range
  if (distance_cm < 2.0) {
    distance_cm = 2.0; // Minimum distance (sensor can't measure below ~2cm)
  }
  if (distance_cm > DUSTBIN_HEIGHT_CM) {
    distance_cm = DUSTBIN_HEIGHT_CM; // Maximum distance (dustbin height)
  }
  
  // Calculate empty space from top
  float emptySpace = distance_cm - 2.0; // Subtract minimum sensor distance
  
  // Calculate filled space
  float filledSpace = DUSTBIN_HEIGHT_CM - emptySpace;
  
  // Calculate fill percentage (0% = empty, 100% = full)
  float fillPercent = (filledSpace / DUSTBIN_HEIGHT_CM) * 100.0;
  
  // Clamp to 0-100%
  if (fillPercent < 0) fillPercent = 0;
  if (fillPercent > 100) fillPercent = 100;
  
  return fillPercent;
}

void loop() {
  if (Firebase.ready() && (millis() - sendDataPrevMillis > sendInterval || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    
    // Read distance from ultrasonic sensor (multiple readings for accuracy)
    float sumDistance = 0.0;
    int validReadings = 0;
    
    for (int i = 0; i < 5; i++) {
      float reading = readUltrasonicDistance();
      if (reading > 0 && reading <= DUSTBIN_HEIGHT_CM * 1.2) {
        sumDistance += reading;
        validReadings++;
      }
      delay(50); // Small delay between readings
    }
    
    if (validReadings > 0) {
      distance = sumDistance / validReadings; // Average distance
    }
    
    // Calculate fill level percentage
    fillLevel = calculateFillLevel(distance);
    
    // Print to Serial Monitor
    Serial.print("Distance: ");
    Serial.print(distance, 1);
    Serial.print(" cm | Fill Level: ");
    Serial.print(fillLevel, 1);
    Serial.println(" %");
    
    // Write fill level to Firebase at /dustbin node
    if (Firebase.RTDB.setFloat(&fbdo, "dustbin", fillLevel)) {
      Serial.print("Fill level written to Firebase: ");
      Serial.print(fillLevel, 1);
      Serial.println(" %");
    } else {
      Serial.print("Failed to write to Firebase: ");
      Serial.println(fbdo.errorReason());
    }
    
    Serial.println();
  }
  
  // Small delay to prevent watchdog issues
  delay(100);
}










