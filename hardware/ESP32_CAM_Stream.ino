/*
 * ESP32-CAM Video Streaming
 * 
 * This sketch connects to WiFi and streams video to a local IP address
 * 
 * Hardware Connections:
 * - ESP32-CAM board (no additional wiring needed)
 * 
 * Access the stream at:
 * - http://[ESP32_IP_ADDRESS]/stream
 * - http://[ESP32_IP_ADDRESS] - Web interface
 */

#include "esp_camera.h"
#include <WiFi.h>

// WiFi credentials
#define WIFI_SSID "Airtel_M-1102"
#define WIFI_PASSWORD "amc@m1102"

// Camera pin definitions (for ESP32-CAM AI-Thinker)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM     25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Onboard LED pin (ESP32-CAM AI-Thinker)
#define LED_GPIO_NUM      4

// Start web server on port 80
WiFiServer server(80);

// Stream URI
String stream_uri = "/stream";
String led_on_uri = "/led/on";
String led_off_uri = "/led/off";

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  Serial.println("ESP32-CAM Video Streaming");
  Serial.println("==========================");
  
  // Initialize onboard LED (default OFF)
  pinMode(LED_GPIO_NUM, OUTPUT);
  digitalWrite(LED_GPIO_NUM, LOW);
  Serial.println("Onboard LED initialized (OFF)");
  
  // Initialize camera
  Serial.println("Initializing camera...");
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // Camera settings - optimized for memory usage
  if(psramFound()){
    config.frame_size = FRAMESIZE_CIF; // 400x296
    config.jpeg_quality = 12;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_QVGA; // 320x240
    config.jpeg_quality = 15;
    config.fb_count = 1;
  }
  
  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  
  Serial.println("Camera initialized successfully!");
  
  // Connect to WiFi
  Serial.println();
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("WiFi connected!");
  
  // Print local IP address
  Serial.println("==========================");
  Serial.print("Local IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("==========================");
  Serial.println();
  Serial.print("Stream URL: http://");
  Serial.print(WiFi.localIP());
  Serial.println("/stream");
  Serial.println();
  
  // Start web server
  server.begin();
  Serial.println("Web server started!");
  Serial.println("Ready to stream video.");
  Serial.println();
}

void loop() {
  WiFiClient client = server.available();
  
  if (client) {
    Serial.println("New client connected");
    
    String request = "";
    unsigned long timeout = millis() + 5000; // 5 second timeout
    
    // Read request with timeout
    while (client.connected() && millis() < timeout) {
      if (client.available()) {
        String line = client.readStringUntil('\n');
        request += line;
        request += "\n";
        
        // Empty line indicates end of HTTP headers
        if (line.length() == 1 || line.length() == 0) {
          // Check what was requested
          if (request.indexOf(stream_uri) >= 0) {
            // Stream video
            sendStream(client);
          } else if (request.indexOf(led_on_uri) >= 0) {
            // Turn LED on
            digitalWrite(LED_GPIO_NUM, HIGH);
            sendLEDResponse(client, true);
          } else if (request.indexOf(led_off_uri) >= 0) {
            // Turn LED off
            digitalWrite(LED_GPIO_NUM, LOW);
            sendLEDResponse(client, false);
          } else {
            // Send web interface
            sendWebPage(client);
          }
          break;
        }
        timeout = millis() + 5000; // Reset timeout on data
      }
    }
    
    // Give client time to receive data
    delay(100);
    client.stop();
    Serial.println("Client disconnected");
  }
  
  delay(10); // Small delay to prevent watchdog issues
}

void sendStream(WiFiClient client) {
  Serial.println("Starting video stream...");
  
  // Send HTTP headers
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: multipart/x-mixed-replace; boundary=123456789000000000000987654321");
  client.println("Access-Control-Allow-Origin: *");
  client.println();
  client.flush();
  
  unsigned long lastFrame = millis();
  
  while (client.connected()) {
    // Check if client is still connected
    if (!client.connected()) {
      Serial.println("Client disconnected during stream");
      break;
    }
    
    // Capture frame
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      delay(100);
      continue; // Try again instead of breaking
    }
    
    // Check if enough time has passed for next frame (control frame rate)
    unsigned long currentTime = millis();
    if (currentTime - lastFrame < 150) { // ~6-7 FPS for stability
      esp_camera_fb_return(fb);
      delay(20);
      continue;
    }
    lastFrame = currentTime;
    
    // Send frame boundary and headers
    String header = "--123456789000000000000987654321\r\n";
    header += "Content-Type: image/jpeg\r\n";
    header += "Content-Length: " + String(fb->len) + "\r\n\r\n";
    
    client.print(header);
    
    // Send image data
    client.write(fb->buf, fb->len);
    
    // Send frame end
    client.print("\r\n");
    client.flush(); // Ensure data is sent
    
    // Return frame buffer
    esp_camera_fb_return(fb);
    
    delay(20); // Small delay to prevent overwhelming the connection
  }
  
  // Flush any remaining data
  client.flush();
  delay(100);
  
  Serial.println("Stream ended");
}

void sendLEDResponse(WiFiClient client, bool ledState) {
  // Send JSON response for LED control
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Access-Control-Allow-Origin: *");
  client.println();
  client.print("{\"success\":true,\"led\":");
  client.print(ledState ? "true" : "false");
  client.println("}");
  client.flush();
  
  // Log LED state change
  Serial.print("LED turned ");
  Serial.println(ledState ? "ON" : "OFF");
}

void sendWebPage(WiFiClient client) {
  // Send HTML page with video stream
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println();
  client.println("<!DOCTYPE html>");
  client.println("<html>");
  client.println("<head>");
  client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
  client.println("<title>ESP32-CAM Stream</title>");
  client.println("<style>");
  client.println("body { margin: 0; background: #000; display: flex; align-items: center; justify-content: center; height: 100vh; }");
  client.println("img { max-width: 100%; max-height: 100%; }");
  client.println("</style>");
  client.println("</head>");
  client.println("<body>");
  client.print("<img src=\"");
  client.print(stream_uri);
  client.println("\">");
  client.println("</body>");
  client.println("</html>");
}
