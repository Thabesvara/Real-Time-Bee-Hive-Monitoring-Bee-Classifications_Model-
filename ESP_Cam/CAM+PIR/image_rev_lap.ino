#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

// WiFi credentials
const char* ssid = "Thabesh";
const char* password = "123456789";

// Laptop server to receive image
const char* uploadServer = "http://192.168.21.161:8000/upload";

// PIR Sensor GPIO
#define PIR_PIN 13

// Web server on port 80
WebServer server(80);

// Camera configuration (AI Thinker)
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
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

void startCamera() {
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

  if(psramFound()) {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_CIF;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}

// Function to capture and send to laptop
void captureAndSendImage() {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  HTTPClient http;
  http.begin(uploadServer);
  http.addHeader("Content-Type", "image/jpeg");

  int httpResponseCode = http.POST(fb->buf, fb->len);
  if (httpResponseCode > 0) {
    Serial.printf("Image sent successfully: %d\n", httpResponseCode);
  } else {
    Serial.printf("Failed to send image. Code: %d\n", httpResponseCode);
  }

  http.end();
  esp_camera_fb_return(fb);
}

// Web server root — serve page with stream and capture button
void handleRoot() {
  String html = R"rawliteral(
    <html>
    <head>
      <title>ESP32-CAM Stream</title>
      <style>
        body { text-align: center; font-family: Arial; }
        button { padding: 10px 20px; font-size: 16px; margin-top: 20px; }
      </style>
    </head>
    <body>
      <h1>📷 ESP32-CAM Live</h1>
      <img src="/stream" width="640" /><br>
      <button onclick="fetch('/capture')">📸 Capture Image</button>
    </body>
    </html>
  )rawliteral";
  server.send(200, "text/html", html);
}

// MJPEG video stream handler
void handleStream() {
  WiFiClient client = server.client();

  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  server.sendContent(response);

  while (1) {
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }

    response = "--frame\r\n";
    response += "Content-Type: image/jpeg\r\n\r\n";
    server.sendContent(response);

    client.write(fb->buf, fb->len);
    server.sendContent("\r\n");

    esp_camera_fb_return(fb);

    if (!client.connected()) break;
  }
}

// Manual capture endpoint
void handleManualCapture() {
  Serial.println("Manual capture requested.");
  captureAndSendImage();
  server.send(200, "text/plain", "Image captured and sent.");
}

void setup() {
  Serial.begin(115200);
  pinMode(PIR_PIN, INPUT);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());

  startCamera();

  server.on("/", handleRoot);
  server.on("/stream", HTTP_GET, handleStream);
  server.on("/capture", HTTP_GET, handleManualCapture);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();

  // Motion Detection
  if (digitalRead(PIR_PIN) == HIGH) {
    Serial.println("🔍 Motion detected! Capturing image...");
    captureAndSendImage();
    delay(10000); // avoid spamming
  }
}
