#include <WiFi.h>
#include <Firebase_ESP_Client.h>

// ## Pin Definitions for L298N Driver 2 (Gate 2) ##
// Connect to Motor A output on Driver 2

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

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

const int ledPin1 = 15;

// MQ2 Sensor Pin
const int MQ2_PIN = 34;  // Analog pin for MQ2 sensor

// MQ2 Calibration parameters
const float RL = 10.0;  // Load resistance in kΩ
const float RO = 10.0;  // Sensor resistance in clean air

// Variables for MQ2 sensor
float sensorValue = 0;
float sensorVoltage = 0;
float sensorResistance = 0;
float gasRatio = 0;
float gasPPM = 0;
float aqiValue = 0;

// For averaging
const int AVERAGE_SAMPLES = 10;
float readings[AVERAGE_SAMPLES];
int readIndex = 0;
float total = 0;
float average = 0;

void setup()
{
  Serial.begin(115200);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  
  // Initialize MQ2 sensor pin
  pinMode(MQ2_PIN, INPUT);
  
  // Initialize readings array
  for (int i = 0; i < AVERAGE_SAMPLES; i++) {
    readings[i] = 0;
  }
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
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

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

  // Limit the size of response payload to be collected in FirebaseData
  fbdo.setResponseSize(2048);

  Firebase.begin(&config, &auth);

  Firebase.setDoubleDigits(5);

  config.timeout.serverResponse = 10 * 1000;
  
  // Wait for MQ2 sensor warm-up
  Serial.println("Waiting for MQ2 sensor warm-up (20 seconds)...");
  delay(20000);
  Serial.println("Sensor ready!");
}

void loop()
{
  // Read MQ2 sensor and calculate AQI
  readMQ2Sensor();
  
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();

    // Write AQI value to Firebase
    if (Firebase.RTDB.setFloat(&fbdo, "aqi", aqiValue))
    {
      Serial.print("AQI written to Firebase: ");
      Serial.println(aqiValue);
    }
    else
    {
      Serial.print("Failed to write AQI: ");
      Serial.println(fbdo.errorReason());
    }

    // Handle /activate
    int activate = 0;
    if (Firebase.RTDB.getInt(&fbdo, "activate", &activate))
    {
      Serial.print("On?: ");
      Serial.println(activate);

      if (activate == 1)
      {
        digitalWrite(13, HIGH);
        delay(3000);
        digitalWrite(13, LOW);
        Firebase.RTDB.setInt(&fbdo, "activate", 0);  
      }
    }
  }
}

// Function to read MQ2 sensor and calculate AQI
void readMQ2Sensor()
{
  // Read analog value (ESP32 ADC is 12-bit, so 0-4095)
  sensorValue = analogRead(MQ2_PIN);
  
  // Convert to voltage (0-3.3V range for ESP32, 12-bit ADC)
  sensorVoltage = (sensorValue / 4095.0) * 3.3;
  
  // Calculate sensor resistance
  // Formula: Rs = ((Vc - VRL) / VRL) * RL
  // Where Vc = 3.3V, VRL = sensorVoltage, RL = load resistance
  if (sensorVoltage > 0) {
    sensorResistance = ((3.3 - sensorVoltage) / sensorVoltage) * RL;
  } else {
    sensorResistance = 0;
  }
  
  // Calculate gas ratio (Rs/Ro)
  gasRatio = sensorResistance / RO;
  
  // Calculate PPM (simplified - actual formula depends on gas type)
  if (gasRatio > 0) {
    // General formula: PPM = a * (Rs/Ro)^b
    // Using exponential approximation for MQ2
    gasPPM = pow(10, ((log10(gasRatio) - 0.5) / -0.4));
    gasPPM = constrain(gasPPM, 0, 10000);
  } else {
    gasPPM = 0;
  }
  
  // Add to averaging array
  total = total - readings[readIndex];
  readings[readIndex] = gasPPM;
  total = total + readings[readIndex];
  readIndex = (readIndex + 1) % AVERAGE_SAMPLES;
  average = total / AVERAGE_SAMPLES;
  
  // Calculate AQI
  aqiValue = calculateAQI(average);
  
  // Print to serial monitor
  Serial.print("AQI: ");
  Serial.println(aqiValue);
}

// Function to calculate Air Quality Index (AQI)
// AQI scale: 0-500 (US EPA standard)
float calculateAQI(float ppm) {
  float aqi;
  
  // Simplified AQI calculation based on gas concentration
  if (ppm <= 50) {
    // Good: 0-50 AQI
    aqi = (ppm / 50.0) * 50.0;
  } else if (ppm <= 100) {
    // Moderate: 51-100 AQI
    aqi = 50 + ((ppm - 50) / 50.0) * 50.0;
  } else if (ppm <= 150) {
    // Unhealthy for Sensitive Groups: 101-150 AQI
    aqi = 100 + ((ppm - 100) / 50.0) * 50.0;
  } else if (ppm <= 200) {
    // Unhealthy: 151-200 
    aqi = 150 + ((ppm - 150) / 50.0) * 50.0;
  } else if (ppm <= 300) {
    // Very Unhealthy: 201-300 AQI
    aqi = 200 + ((ppm - 200) / 100.0) * 100.0;
  } else {
    // Hazardous: 301-500 AQI
    aqi = 300 + ((ppm - 300) / 200.0) * 200.0;
    aqi = constrain(aqi, 300, 500);
  }
  
  return constrain(aqi, 0, 500);
}

