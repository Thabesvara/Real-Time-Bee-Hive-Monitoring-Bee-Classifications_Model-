#include <WiFi.h>
#include <WebServer.h>
#include <HX711.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// === WiFi Credentials ===
const char* ssid = "Thabesh";
const char* password = "123456789";

// === Pins ===
#define DHTPIN 4
#define DHTTYPE DHT22
#define DS18B20_PIN 15
#define MIC_PIN 34
#define IR1_PIN 14
#define IR2_PIN 27
#define HX711_DT 21
#define HX711_SCK 22

// === Calibration ===
const float Vref = 0.02857;
const int sampleWindow = 100;
float calibration_factor = 19851;
int BASELINE = 1938;

// === Sensor Objects ===
DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(DS18B20_PIN);
DallasTemperature ds18b20(&oneWire);
HX711 scale;
WebServer server(80);

// === Sensor Reading Function ===
String getSensorData() {
  // IR
  String ir1 = digitalRead(IR1_PIN) ? "NONE" : "DETECTED";
  String ir2 = digitalRead(IR2_PIN) ? "NONE" : "DETECTED";

  // DHT22
  float dhtTemp = dht.readTemperature();
  float dhtHumidity = dht.readHumidity();
  if (isnan(dhtTemp)) dhtTemp = -999;
  if (isnan(dhtHumidity)) dhtHumidity = -999;

  // DS18B20
  ds18b20.requestTemperatures();
  float dsTemp = ds18b20.getTempCByIndex(0);

  // Load cell
  float weight = scale.get_units();

  // Microphone (dB avg over 3s)
  float sumDB = 0;
  int count = 0;
  unsigned long start = millis();
  while (millis() - start < 3000) {
    int reading = analogRead(MIC_PIN);
    int level = abs(reading - BASELINE);
    if (level == 0) level = 1;
    float dB = 20.0 * log10(level);
    sumDB += dB;
    count++;
    delay(sampleWindow);
    server.handleClient();
  }
  float avgDB = sumDB / count;

  // Return as JSON
  String json = "{";
  json += "\"ir1\":\"" + ir1 + "\",";
  json += "\"ir2\":\"" + ir2 + "\",";
  json += "\"dhtTemp\":" + String(dhtTemp, 2) + ",";
  json += "\"dhtHumidity\":" + String(dhtHumidity, 2) + ",";
  json += "\"dsTemp\":" + String(dsTemp, 2) + ",";
  json += "\"weight\":" + String(weight, 2) + ",";
  json += "\"db\":" + String(avgDB, 2);
  json += "}";
  return json;
}

// === HTML Page ===
String htmlPage = R"rawliteral(
<!DOCTYPE html><html>
<head>
  <title>ESP32 Sensor Dashboard</title>
  <script src='https://cdn.jsdelivr.net/npm/chart.js'></script>
  <style>
    body {
      font-family: Arial, sans-serif;
      padding: 20px;
    }
    .reading-box {
      margin-bottom: 20px;
    }
    .graph-container {
      width: 10cm;
      height: 10cm;
      border: 2px solid #333;
      padding: 10px;
      box-sizing: border-box;
    }
    canvas {
      width: 100% !important;
      height: 100% !important;
    }
  </style>
</head>
<body>
  <h2>ESP32 Real-Time Sensor Dashboard</h2>

  <div class="reading-box">
    <p><strong>IR Sensor 1:</strong> <span id="ir1">...</span></p>
    <p><strong>IR Sensor 2:</strong> <span id="ir2">...</span></p>
    <p><strong>DHT22 Temp:</strong> <span id="dhtTemp">...</span> °C</p>
    <p><strong>DHT22 Humidity:</strong> <span id="dhtHumidity">...</span> %</p>
    <p><strong>DS18B20 Temp:</strong> <span id="dsTemp">...</span> °C</p>
    <p><strong>Weight:</strong> <span id="weight">...</span> kg</p>
  </div>

  <div class="graph-container">
    <canvas id="dbChart"></canvas>
  </div>

  <script>
    const ctx = document.getElementById('dbChart').getContext('2d');
    const dbChart = new Chart(ctx, {
      type: 'line',
      data: {
        labels: [],
        datasets: [{
          label: 'Sound Level (dB)',
          data: [],
          borderColor: 'blue',
          borderWidth: 2,
          tension: 0.3
        }]
      },
      options: {
        scales: {
          y: { min: 0, max: 100 },
          x: { title: { display: true, text: 'Time (x 3s)' } }
        }
      }
    });

    let timeCounter = 0;
    async function updateData() {
      const response = await fetch('/data');
      const data = await response.json();
      document.getElementById('ir1').textContent = data.ir1;
      document.getElementById('ir2').textContent = data.ir2;
      document.getElementById('dhtTemp').textContent = data.dhtTemp;
      document.getElementById('dhtHumidity').textContent = data.dhtHumidity;
      document.getElementById('dsTemp').textContent = data.dsTemp;
      document.getElementById('weight').textContent = data.weight;

      dbChart.data.labels.push(timeCounter++);
      dbChart.data.datasets[0].data.push(parseFloat(data.db));
      if (dbChart.data.labels.length > 20) {
        dbChart.data.labels.shift();
        dbChart.data.datasets[0].data.shift();
      }
      dbChart.update();
    }

    setInterval(updateData, 3000);
  </script>
</body></html>
)rawliteral";


// === Server Handlers ===
void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

void handleData() {
  String json = getSensorData();
  server.send(200, "application/json", json);
}

// === Setup ===
void setup() {
  Serial.begin(115200);
  dht.begin();
  ds18b20.begin();
  scale.begin(HX711_DT, HX711_SCK);
  scale.set_scale(calibration_factor);
  scale.tare();

  pinMode(IR1_PIN, INPUT);
  pinMode(IR2_PIN, INPUT);
  analogReadResolution(12);

  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected! IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
}

// === Main Loop ===
void loop() {
  server.handleClient();
}