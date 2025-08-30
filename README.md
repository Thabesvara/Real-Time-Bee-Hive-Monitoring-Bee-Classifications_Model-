# 🐝 Smart Beehive Monitoring & Classification System

## 🎯 Objective
To develop a smart beehive monitoring system that:
- Monitors hive health through environmental sensors.
- Tracks bee activity using sound and motion detection.
- Uses an AI model (YOLOv5) for classifying bee types (Queen, Drone, Worker).
- Displays all real-time data on a Flask-hosted web interface.

---

## ⚙️ System Design

### 📡 ESP32-Based Sensor Monitoring
The ESP32 microcontroller collects real-time data from:
- **DS18B20** → Measures inside hive temperature
- **DHT22** → Measures outside hive temperature & humidity
- **Load Cell + HX711** → Tracks hive weight (honey levels & bee population changes)
- **MAX4466 Sound Sensor** → Monitors buzzing levels (activity/stress detection)
- **Two IR Sensors** → Approximate bee entry/exit counting
- Sensor readings are transmitted via **Wi-Fi** to a Flask server.

---

### 📷 ESP32-CAM + PIR-Based Image Capture
- **PIR sensor** detects motion at hive entrance.
- **ESP32-CAM** captures image when motion is detected.
- Captured images are transmitted to the Flask server on a laptop.
- Images are saved in a dedicated folder for classification.

---

### 🤖 Bee Type Detection (YOLOv5)
- A **custom-trained YOLOv5 model** is used to classify:
  - **Worker Bee**
  - **Queen Bee**
  - **Drone Bee**
- Flask server performs inference on images.
- Outputs:
  - Image with bounding boxes & labels
  - Detected bee type(s) & count
  - Displayed on the web interface

---

### 🌐 Flask Server & Web Interface
- Flask server runs on the laptop, handling:
  - Sensor data updates from ESP32
  - Image uploads from ESP32-CAM
  - YOLOv5 inference
  - Rendering a dynamic web page

**Web Interface Includes:**
- 📊 Live sensor values (temperature, humidity, weight, sound, bee count)
- 🖼️ Last captured image
- 🗂️ Gallery of all captured images
- 🐝 YOLOv5 detection results with bee type labels & counts
- 📈 Graphs for sensor data trends

---

## 🔄 Communication Flow
1. **Sensors → ESP32 → Wi-Fi → Flask Server**
2. **PIR detects motion → ESP32-CAM captures image → Sends to Flask**
3. **Flask server → Saves image → YOLOv5 detection → Updates webpage**

---

## 🖥️ Final Web Interface
- ✅ Live Sensor Dashboard
- ✅ Latest Captured Image
- ✅ YOLOv5 Classified Output
- ✅ Gallery of Captured Images
- ✅ Bee Counting Logic via IR Sensors

---

## 🛠️ Hardware Components
- ESP32 Microcontroller 
- ESP32-CAM Module 
- DHT22 (Outside Temp + Humidity) 
- DS18B20 (Inside Temp) 
- HX711 + Load Cell (Weight) 
- MAX4466 Sound Sensor (Hive Activity) 
- IR Sensors ×2 (Bee Counting) 
- PIR Sensor (Motion Detection) 
- Lithium-Ion 3.7V Battery 

---

## 💻 Software Stack
- **Arduino IDE** (ESP32 & ESP32-CAM firmware) 
- **Python (Flask, YOLOv5, PyTorch, OpenCV)** 
- **HTML + CSS** (Web Interface) 

---

## 🚀 Future Scope
- Cloud integration for remote monitoring
- AI/ML for hive sound pattern analysis
- Mobile app for remote beekeeping dashboard
- Solar-powered setup for field deployment

---

## 👨‍💻 Team Members
- **Thabesvara (Thabesh)** – YOLOv5 Bee Classification, Flask Integration 
- **Kumaravel M J** – Sensor Data & ESP32 Integration 
- **Mukunth Kumar** – Hardware Assembly & Testing 
- **Santha Kumar** – System Design & Support
