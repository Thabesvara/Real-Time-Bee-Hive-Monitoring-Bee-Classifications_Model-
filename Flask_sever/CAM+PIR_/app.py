from flask import Flask, render_template, request, send_from_directory
from datetime import datetime
import os
from yolov5.detect import run as detect_yolo  # Adjust if needed

app = Flask(__name__)

# Folder setup
UPLOAD_FOLDER = 'static/uploads'
DETECTED_FOLDER = 'static/detected'
os.makedirs(UPLOAD_FOLDER, exist_ok=True)
os.makedirs(DETECTED_FOLDER, exist_ok=True)

app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER
app.config['DETECTED_FOLDER'] = DETECTED_FOLDER

# Route: Home
@app.route('/')
def index():
    images = sorted([f for f in os.listdir(DETECTED_FOLDER) if f.endswith(('.jpg', '.png'))], reverse=True)
    latest = images[0] if images else None
    prediction_text = ""

    if latest:
        label_path = os.path.join(DETECTED_FOLDER, latest).replace(".jpg", ".txt").replace(".png", ".txt")
        if os.path.exists(label_path):
            with open(label_path, 'r') as f:
                lines = f.readlines()
                classes = [line.split()[0] for line in lines]
                unique_classes = set(classes)
                prediction_text = f"Detected bee types: {', '.join(unique_classes)}"
        else:
            prediction_text = "No bee detected."

    return render_template('index.html', latest_image=latest, prediction_text=prediction_text)

# Route: Upload image
@app.route('/upload', methods=['POST'])
def upload_image():
    now = datetime.now().strftime("%Y%m%d_%H%M%S")
    filename = f"image_{now}.jpg"
    input_path = os.path.join(UPLOAD_FOLDER, filename)

    with open(input_path, 'wb') as f:
        f.write(request.data)

    # YOLOv5 Detection - FIXED imgsz to be a tuple
    detect_yolo(
        weights='yolov5/best.pt',
        source=input_path,
        imgsz=(640, 640),  # FIXED
        conf_thres=0.4,
        save_txt=True,
        save_conf=True,
        project=DETECTED_FOLDER,
        name='.',
        exist_ok=True
    )

    # Move processed image to DETECTED_FOLDER (if not already saved there)
    detected_image_path = os.path.join(DETECTED_FOLDER, filename)
    if not os.path.exists(detected_image_path):
        os.rename(input_path, detected_image_path)

    return "Image received and processed", 200

# Route: Gallery
@app.route('/gallery')
def gallery():
    images = sorted([f for f in os.listdir(DETECTED_FOLDER) if f.endswith(('.jpg', '.png'))], reverse=True)
    return render_template('gallery.html', images=images)

# Route: Serve image
@app.route('/uploads/<filename>')
def uploaded_file(filename):
    return send_from_directory(DETECTED_FOLDER, filename)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8000)

