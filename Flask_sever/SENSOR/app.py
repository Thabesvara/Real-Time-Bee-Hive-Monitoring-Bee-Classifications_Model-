from flask import Flask, render_template, request, Response
import json
import random
import time

app = Flask(__name__)

# Store the latest data in a dictionary
sensor_data = {
    'dhtTemp': 0.0,
    'dhtHumidity': 0.0,
    'dsTemp': 0.0,
    'weight': 0.0,
    'sound': 0.0,
    'ir1': "NONE",
    'ir2': "NONE"
}

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/receive_data', methods=['POST'])
def receive_data():
    # Receive JSON data from ESP32
    data = request.get_json()
    
    # Update the sensor data with the new values
    sensor_data['dhtTemp'] = data['dhtTemp']
    sensor_data['dhtHumidity'] = data['dhtHumidity']
    sensor_data['dsTemp'] = data['dsTemp']
    sensor_data['weight'] = data['weight']
    sensor_data['sound'] = data['sound']
    sensor_data['ir1'] = data['ir1']
    sensor_data['ir2'] = data['ir2']
    
    return "Data received", 200

def generate_data():
    while True:
        # Yield data as Server-Sent Events (SSE)
        yield f"data: {json.dumps(sensor_data)}\n\n"
        time.sleep(1)  # Send data every 1 second

@app.route('/events')
def events():
    return Response(generate_data(), mimetype='text/event-stream')

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0', port=5000)

