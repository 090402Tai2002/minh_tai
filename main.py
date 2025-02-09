import paho.mqtt.client as mqtt
from ultralytics import YOLO
import cv2
import numpy as np
import os
import time
from datetime import datetime
from scipy.spatial import distance as dist
import json

# Function to read data from JSON file
def read_json_data(file_path):
    try:
        with open(file_path, "r") as f:
            data = json.load(f)
            pixels_per_mm = data.get("average_pixels_per_mm")
            if pixels_per_mm:
                print(f"Pixels per mm calibration value found: {pixels_per_mm}")
                return pixels_per_mm
            else:
                print("Invalid data in JSON file or missing 'average_pixels_per_mm'.")
                return None
    except FileNotFoundError:
        print(f"The file {file_path} does not exist!")
        return None
    except json.JSONDecodeError:
        print("Error or invalid JSON file!")
        return None

# Function to calculate the maximum length from contour (based on the distance between contour points)
def calculate_shrimp_length(contour, pixels_per_mm):
    contour_points = contour.reshape(-1, 2)
    max_distance = 0
    for i in range(len(contour_points)):
        for j in range(i + 1, len(contour_points)):
            d = dist.euclidean(contour_points[i], contour_points[j])
            max_distance = max(max_distance, d)
    return max_distance / pixels_per_mm  # Convert pixel to mm

# Function to get weight and feed percentage based on shrimp length
def get_feed_data(length_mm, avg_length_mm):
    length_to_data = {
        (0, 3.9): (0.5, 20),
        (4, 4.9): (1, 18),
        (5, 5.5): (1.25, 18),
        (5.6, 6.1): (1.75, 15),
        (6.2, 6.6): (2.25, 8),
        (6.7, 7.1): (2.75, 8),
        (7.2, 7.6): (3.38, 6.5),
        (7.7, 8.1): (4.13, 6.5),
        (8.2, 8.7): (5, 5.5),
        (8.8, 9.3): (6, 4.5),
        (9.4, 9.9): (7.25, 4),
        (10, 10.5): (8.5, 3.5),
        (10.5, 11.1): (10, 3.5),
        (11.2, 11.8): (12, 3.33),
        (11.9, 12.4): (14.25, 3.06),
        (12.5, 12.9): (16.25, 3.06),
        (13, 13.4): (18.38, 2.88),
        (13.5, 13.8): (20.25, 2.68),
        (13.9, 14.1): (22.25, 2.46),
        (14.2, 14.4): (24.25, 2.10),
        (14.5, 14.7): (26, 1.9),
        (14.8, 100): (27.88, 1.9)
    }

    feed_data = []
    for length_range, (weight, feed_percentage) in length_to_data.items():
        if length_range[0] * 10 <= avg_length_mm <= length_range[1] * 10:
            feed_data = {
                'avg_length_mm': avg_length_mm,
                'weight': weight,
                'feed_percentage': feed_percentage,
                'feed_amount': weight * (feed_percentage / 100)
            }
            break

    # If no length found in the data, return None
    if not feed_data:
        feed_data = {
            'avg_length_mm': avg_length_mm,
            'weight': None,
            'feed_percentage': None,
            'feed_amount': None
        }

    return feed_data

# MQTT setup
MQTT_BROKER = "192.168.0.189"  # Replace with your MQTT broker address
MQTT_PORT = 1883
MQTT_TOPIC = "shrimp/feeding_data"

# Initialize MQTT client
mqtt_client = mqtt.Client()
mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)

# Load YOLO model
model = YOLO("/home/tai/Desktop/code/best.pt")  # Replace with your model file

# Confidence threshold
conf_threshold = 0.7

# Folder to save processed images
output_folder = "/home/tai/Desktop/code/processed_images"
os.makedirs(output_folder, exist_ok=True)

# JSON file to save calibration data
calibration_file = "/home/tai/Desktop/code/calibration_data.json"

# Read pixels per mm from the JSON file
pixels_per_mm = read_json_data(calibration_file)
if pixels_per_mm is None:
    print("Pixels per mm calibration value not found in JSON file. Exiting program.")
    exit()

# Open webcam
cap = cv2.VideoCapture(0)
if not cap.isOpened():
    print("Unable to open webcam.")
    exit()

# Number of images to process
num_images = 5
print("Automatically capturing 5 images from the webcam.")

# List to store image processing data
image_data = []

for i in range(num_images):
    # Capture frame from the webcam
    ret, frame = cap.read()
    if not ret:
        print("Failed to capture frame from webcam.")
        break

    # Run YOLO model to detect objects
    results = model.predict(source=frame, conf=conf_threshold, save=False, show=False)

    # Count the number of detected objects (shrimp)
    shrimp_count = len(results[0].boxes)  # Number of bounding boxes
    print(f"Number of shrimp detected: {shrimp_count}")

    # Initialize list to store shrimp lengths
    lengths = []

    # Annotate the frame with detection information
    annotated_frame = frame.copy()

    # If there are detected objects, process each bounding box
    if shrimp_count > 0:
        for j, box in enumerate(results[0].boxes.xyxy):  # Get coordinates of bounding boxes
            x_min, y_min, x_max, y_max = map(int, box.tolist())  # Convert to int
            shrimp_roi = frame[y_min:y_max, x_min:x_max]  # Crop the shrimp region

            # Process image to find contour
            gray = cv2.cvtColor(shrimp_roi, cv2.COLOR_BGR2GRAY)
            blurred = cv2.GaussianBlur(gray, (5, 5), 0)
            _, thresh = cv2.threshold(blurred, 50, 255, cv2.THRESH_BINARY_INV + cv2.THRESH_OTSU)

            # Find largest contour
            contours, _ = cv2.findContours(thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
            if contours:
                largest_contour = max(contours, key=cv2.contourArea)

                # Calculate shrimp length from contour
                length_mm = calculate_shrimp_length(largest_contour, pixels_per_mm)
                lengths.append(length_mm)

                # Draw contour and display information on the frame
                cv2.drawContours(shrimp_roi, [largest_contour], -1, (0, 255, 0), 2)
                cv2.putText(annotated_frame, f"Shrimp {j + 1}: {length_mm:.2f} mm", 
                            (x_min, y_min - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)
                print(f"- Shrimp {j + 1}: Length: {length_mm:.2f} mm")

                # Draw bounding box around detected shrimp
                cv2.rectangle(annotated_frame, (x_min, y_min), (x_max, y_max), (0, 255, 255), 2)

    # Calculate average length if there are detected shrimp
    avg_length = np.mean(lengths) if lengths else 0
    if avg_length > 0:
        # Get feed data from get_feed_data function
        feed_data = get_feed_data(avg_length, avg_length)
        cv2.putText(annotated_frame, f"Average Length: {avg_length:.2f} mm", (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 0), 2)
        print(f"Average length of shrimp: {avg_length:.2f} mm")
        # Publish feed data to MQTT
        mqtt_client.publish(MQTT_TOPIC, json.dumps(feed_data))
        print(f"Feed data sent to MQTT: {feed_data}")
    else:
        cv2.putText(annotated_frame, "No shrimp detected", (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 0, 255), 2)
        print("No shrimp detected in the frame.")
        feed_data = {
            'avg_length_mm': 0,
            'weight': None,
            'feed_percentage': None,
            'feed_amount': None
        }

    # Save the annotated image
    image_name = f"image_{i + 1}_processed.jpg"
    output_path = os.path.join(output_folder, image_name)
    cv2.imwrite(output_path, annotated_frame)
    print(f"Processed image {i + 1}/{num_images} saved: {output_path}")

    # Save processing info to local list
    image_info = {
        "date": datetime.now().strftime("%Y-%m-%d"),
        "time": datetime.now().strftime("%H:%M:%S"),
        "image_name": image_name,
        "shrimp_count": shrimp_count,
        "average_length_mm": round(avg_length, 2),
        "feed_data": feed_data
    }
    image_data.append(image_info)

    # Display the annotated frame
    cv2.imshow(f"Result for Image {i + 1}", annotated_frame)
    cv2.waitKey(500)  # Display the frame for 500ms

    # Wait for 1 second before capturing the next image
    time.sleep(1)

# Release resources
cap.release()
cv2.destroyAllWindows()

# Save the processing information to a JSON file
log_file = os.path.join(output_folder, "shrimp_detection_log.json")
with open(log_file, "w") as f:
    json.dump(image_data, f, indent=4)
print(f"Shrimp detection information has been saved to the file: {log_file}")

# Disconnect MQTT
mqtt_client.disconnect()
