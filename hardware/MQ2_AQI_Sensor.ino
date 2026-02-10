/*
 * MQ2 Gas Sensor - AQI Calculator
 * 
 * This sketch reads data from MQ2 gas sensor and calculates Air Quality Index (AQI)
 * MQ2 detects: LPG, Propane, Hydrogen, Methane, Alcohol, Smoke, CO
 * 
 * Connections:
 * - MQ2 VCC -> Arduino 5V
 * - MQ2 GND -> Arduino GND
 * - MQ2 A0 (Analog) -> Arduino A0
 * - MQ2 D0 (Digital) -> Not used in this sketch
 * 
 * Load resistor: 10KΩ (usually built into sensor module)
 */

// Pin definitions
const int MQ2_PIN = A0;  // Analog pin connected to MQ2 sensor

// Calibration parameters
const float RL = 10.0;  // Load resistance in kΩ
const float RO = 10.0;  // Sensor resistance in clean air (calibrate this)

// AQI calculation parameters
const int SAMPLE_INTERVAL = 1000;  // Sample every 1 second
const int AVERAGE_SAMPLES = 10;    // Number of samples for averaging

// Variables
float sensorValue = 0;
float sensorVoltage = 0;
float sensorResistance = 0;
float gasRatio = 0;
float gasPPM = 0;
float aqiValue = 0;

// For averaging
float readings[AVERAGE_SAMPLES];
int readIndex = 0;
float total = 0;
float average = 0;

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  Serial.println("MQ2 Gas Sensor - AQI Calculator");
  Serial.println("Initializing sensor...");
  Serial.println("Please wait 20 seconds for sensor warm-up...");
  
  // Wait for sensor to stabilize
  delay(20000);
  
  // Initialize readings array
  for (int i = 0; i < AVERAGE_SAMPLES; i++) {
    readings[i] = 0;
  }
  
  Serial.println("Sensor ready!");
  Serial.println("==========================================");
  Serial.println("Time(s)\tRaw\tVoltage(V)\tResistance(kΩ)\tPPM\tAQI\tStatus");
  Serial.println("==========================================");
}

void loop() {
  // Read analog value
  sensorValue = analogRead(MQ2_PIN);
  
  // Convert to voltage (0-5V range, 10-bit ADC)
  sensorVoltage = (sensorValue / 1024.0) * 5.0;
  
  // Calculate sensor resistance
  // Formula: Rs = ((Vc - VRL) / VRL) * RL
  // Where Vc = 5V, VRL = sensorVoltage, RL = load resistance
  if (sensorVoltage > 0) {
    sensorResistance = ((5.0 - sensorVoltage) / sensorVoltage) * RL;
  } else {
    sensorResistance = 0;
  }
  
  // Calculate gas ratio (Rs/Ro)
  gasRatio = sensorResistance / RO;
  
  // Calculate PPM (simplified - actual formula depends on gas type)
  // Using exponential approximation for MQ2
  // For different gases, use appropriate formula
  if (gasRatio > 0) {
    // General formula: PPM = a * (Rs/Ro)^b
    // For MQ2 detecting multiple gases, using a simplified conversion
    // This is a simplified model - adjust based on your specific gas
    gasPPM = pow(10, ((log10(gasRatio) - 0.5) / -0.4));  // Approximate formula
    gasPPM = constrain(gasPPM, 0, 10000);  // Limit to reasonable range
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
  Serial.print(millis() / 1000);
  Serial.print("\t");
  Serial.print(sensorValue);
  Serial.print("\t");
  Serial.print(sensorVoltage, 3);
  Serial.print("\t\t");
  Serial.print(sensorResistance, 2);
  Serial.print("\t\t");
  Serial.print(average, 1);
  Serial.print("\t");
  Serial.print(aqiValue, 1);
  Serial.print("\t");
  Serial.println(getAQIStatus(aqiValue));
  
  // Wait before next reading
  delay(SAMPLE_INTERVAL);
}

/*
 * Calculate Air Quality Index (AQI)
 * AQI scale: 0-500 (US EPA standard)
 * 
 * For MQ2 sensor detecting multiple gases, we'll use a simplified approach
 * based on gas concentration levels
 */
float calculateAQI(float ppm) {
  float aqi;
  
  // Simplified AQI calculation based on gas concentration
  // Thresholds can be adjusted based on your requirements
  
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
    // Unhealthy: 151-200 AQI
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

/*
 * Get AQI status description
 */
String getAQIStatus(float aqi) {
  if (aqi <= 50) {
    return "Good";
  } else if (aqi <= 100) {
    return "Moderate";
  } else if (aqi <= 150) {
    return "Unhealthy-Sensitive";
  } else if (aqi <= 200) {
    return "Unhealthy";
  } else if (aqi <= 300) {
    return "Very Unhealthy";
  } else {
    return "Hazardous";
  }
}










