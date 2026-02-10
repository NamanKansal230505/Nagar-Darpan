/*
 * ESP32 TDS Meter with Firebase Realtime Database
 * 
 * This sketch reads TDS (Total Dissolved Solids) sensor data
 * and writes it to Firebase Realtime Database at /tds node
 * 
 * Hardware Connections:
 * - TDS Sensor VCC -> 3.3V or 5V
 * - TDS Sensor GND -> GND
 * - TDS Sensor Signal -> GPIO 34 (ADC1_CH6 - works with WiFi)
 * 
 * Note: Use ADC1 pins (GPIO 32-39) as ADC2 cannot be used with WiFi
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

// TDS Sensor pin (must be ADC1 compatible - GPIO 32-39)
#define TDS_SENSOR_PIN 34

// TDS calculation parameters
#define VREF 3.3              // ADC reference voltage (ESP32 is 3.3V)
#define ADC_RESOLUTION 4096.0  // ESP32 ADC resolution (12-bit)
#define TDS_FACTOR 0.5         // TDS conversion factor (calibration constant)

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
const unsigned long sendInterval = 2000; // Send data every 2 seconds

// TDS reading variables
float tdsValue = 0.0;
float averageVoltage = 0.0;

void setup() {
  Serial.begin(115200);
  
  // Initialize TDS sensor pin
  pinMode(TDS_SENSOR_PIN, INPUT);
  
  // Configure ADC attenuation for 0-3.3V range
  // ATTEN_DB_11 allows 0-3.3V input range
  analogSetAttenuation(ADC_11db);  // 0-3.3V range
  analogSetWidth(12);  // 12-bit resolution (0-4095)
  
  Serial.println();
  Serial.println("ESP32 TDS Meter with Firebase");
  Serial.println("==============================");
  Serial.print("TDS Sensor initialized on GPIO ");
  Serial.println(TDS_SENSOR_PIN);
  Serial.println("ADC configured for 0-3.3V range");
  Serial.println();
  
  // Test reading before WiFi
  Serial.println("Testing sensor connection...");
  int testReading = analogRead(TDS_SENSOR_PIN);
  float testVoltage = (testReading / 4096.0) * 3.3;
  Serial.print("Raw ADC reading: ");
  Serial.println(testReading);
  Serial.print("Test voltage: ");
  Serial.print(testVoltage, 3);
  Serial.println(" V");
  
  if (testReading == 0) {
    Serial.println("WARNING: Sensor reading is 0!");
    Serial.println("Please check:");
    Serial.println("1. Sensor VCC connected to 3.3V or 5V");
    Serial.println("2. Sensor GND connected to GND");
    Serial.println("3. Sensor signal wire connected to GPIO 34");
    Serial.println("4. Sensor is powered on");
  }
  Serial.println();
  
  // Connect to WiFi
  Serial.println();
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
  Serial.println("Starting TDS readings...");
  Serial.println();
}

// Read TDS sensor and calculate TDS value
float readTDS() {
  // Read multiple samples for averaging
  int samples = 20;  // Increased samples for better accuracy
  float sum = 0.0;
  int rawSum = 0;
  
  for (int i = 0; i < samples; i++) {
    int analogValue = analogRead(TDS_SENSOR_PIN);
    rawSum += analogValue;
    float voltage = (analogValue / 4096.0) * 3.3;  // Use 4096.0 for proper division
    sum += voltage;
    delay(20); // Slightly longer delay between readings
  }
  
  int rawAverage = rawSum / samples;
  averageVoltage = sum / samples;
  
  // Debug: Print raw ADC value occasionally
  static unsigned long lastDebug = 0;
  if (millis() - lastDebug > 5000) {
    Serial.print("Raw ADC Average: ");
    Serial.print(rawAverage);
    Serial.print(" | Voltage: ");
    Serial.print(averageVoltage, 3);
    Serial.println(" V");
    lastDebug = millis();
  }
  
  // Calculate TDS value (ppm)
  // TDS sensors typically output higher voltage with higher TDS
  // Formula varies by sensor, but common is: TDS (ppm) = voltage * conversion factor
  // For TDS sensors, a typical formula is: TDS = (voltage * 1000) / calibration_factor
  // Or: TDS = voltage * TDS_FACTOR * 1000
  
  // Standard TDS calculation: TDS (ppm) = (voltage * 1000) / 2.0
  // Adjust calibration factor as needed
  if (averageVoltage > 0.01) {  // Only calculate if we have a valid reading
    tdsValue = (averageVoltage * 1000.0) / 2.0;  // Adjust divisor based on calibration
  } else {
    tdsValue = 0.0;
  }
  
  return tdsValue;
}

void loop() {
  if (Firebase.ready() && (millis() - sendDataPrevMillis > sendInterval || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    
    // Read TDS value
    float currentTDS = readTDS();
    
    // Print to Serial Monitor with detailed info
    Serial.print("Raw ADC: ");
    int rawRead = analogRead(TDS_SENSOR_PIN);
    Serial.print(rawRead);
    Serial.print(" | Voltage: ");
    Serial.print(averageVoltage, 3);
    Serial.print(" V | TDS: ");
    Serial.print(currentTDS, 1);
    Serial.println(" ppm");
    
    // Write TDS value to Firebase at /tds node
    if (Firebase.RTDB.setFloat(&fbdo, "tds", currentTDS)) {
      Serial.print("TDS written to Firebase: ");
      Serial.print(currentTDS);
      Serial.println(" ppm");
    } else {
      Serial.print("Failed to write TDS to Firebase: ");
      Serial.println(fbdo.errorReason());
    }
    
    Serial.println();
  }
  
  // Small delay to prevent watchdog issues
  delay(100);
}

