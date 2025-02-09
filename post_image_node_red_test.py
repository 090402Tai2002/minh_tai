from ultralytics import YOLO
import cv2
import numpy as np
import os
import time
from datetime import datetime
from scipy.spatial import distance as dist
import json
import requests

# Hàm tính điểm trung bình giữa hai điểm
def midpoint(ptA, ptB):
    return ((ptA[0] + ptB[0]) * 0.5, (ptA[1] + ptB[1]) * 0.5)

# Hàm đọc dữ liệu từ tệp JSON
def read_json_data(file_path):
    try:
        with open(file_path, "r") as f:
            data = json.load(f)
            average_pixels_per_mm = data.get("average_pixels_per_mm")
            if average_pixels_per_mm:
                print(f"Hệ số pixels/mm đã lưu: {average_pixels_per_mm}")
                return average_pixels_per_mm
            else:
                print("Dữ liệu trong tệp JSON không hợp lệ hoặc thiếu 'average_pixels_per_mm'.")
                return None
    except FileNotFoundError:
        print(f"Tệp {file_path} không tồn tại!")
        return None
    except json.JSONDecodeError:
        print("Tệp JSON bị lỗi hoặc không hợp lệ!")
        return None

# Hàm lấy thông tin trọng lượng và tỷ lệ cho ăn từ chiều dài tôm
def get_feed_data(length_mm):
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
    
    for length_range, (weight, feed_percentage) in length_to_data.items():
        if length_range[0] <= length_mm <= length_range[1]:
            return weight, feed_percentage
    return None, None

# Tải mô hình YOLO
model = YOLO("best.pt")  # Thay "best.pt" bằng tệp mô hình YOLO của bạn

# Ngưỡng tin cậy
conf_threshold = 0.5

# Thư mục lưu ảnh sau xử lý
output_folder = "processed_images"
os.makedirs(output_folder, exist_ok=True)

# Tệp JSON để lưu thông tin hiệu chỉnh
calibration_file = "calibration_data.json"

# Đọc hệ số pixels/mm từ tệp JSON hoặc sử dụng giá trị mặc định
pixels_per_mm = read_json_data(calibration_file)
if pixels_per_mm is None:
    print("Không tìm thấy hệ số từ tệp JSON, thoát chương trình.")
    exit()

# Mở webcam
cap = cv2.VideoCapture(0)
if not cap.isOpened():
    print("Không thể mở webcam.")
    exit()

# Số ảnh cần xử lý
num_images = 5
print("Tự động chụp 5 ảnh từ webcam.")

# Danh sách lưu thông tin xử lý ảnh
image_data = []

for i in range(num_images):
    # Đọc khung hình từ webcam
    ret, frame = cap.read()
    if not ret:
        print("Không thể chụp khung hình từ webcam.")
        break

    # Chạy mô hình YOLO để phát hiện đối tượng
    results = model.predict(source=frame, conf=conf_threshold, save=False, show=False)

    # Đếm số lượng đối tượng (tôm)
    shrimp_count = len(results[0].boxes)  # Số lượng bounding boxes

    # Khởi tạo giá trị mặc định cho avg_length
    avg_length = 0

    # Tính toán chiều dài của mỗi tôm (nếu có)
    lengths = []
    if shrimp_count > 0:
        for box in results[0].boxes.xyxy:  # Lấy tọa độ của bounding boxes
            x_min, y_min, x_max, y_max = box
            length_mm = dist.euclidean((x_min, y_min), (x_min, y_max)) / pixels_per_mm  # Tính chiều dài bằng mm
            lengths.append(length_mm)

        avg_length = np.mean(lengths) if lengths else 0  # tinh bang mm

    # Tính lượng thức ăn dựa trên chiều dài trung bình
    weight, feed_percentage, feed_amount = None, None, None
    if avg_length > 0:
        weight, feed_percentage = get_feed_data(avg_length)
        if weight is not None:
            feed_amount = weight * (feed_percentage / 100)
            print(f"\nChiều dài trung bình: {avg_length:.2f} mm")
            print(f"- Trọng lượng thân: {weight:.2f} gram.")
            print(f"- Tỷ lệ cho ăn: {feed_percentage:.2f}%.")
            print(f"- Lượng thức ăn cần: {feed_amount:.2f} gram.")
        else:
            print(f"Chiều dài {avg_length:.2f} cm không nằm trong khoảng hỗ trợ.")
    else:
        print("Không phát hiện được đối tượng nào trong khung hình.")

    # Annotate khung hình để trực quan hóa
    annotated_frame = frame

    # Lưu ảnh đã annotate
    image_name = f"image_{i + 1}_processed.jpg"
    output_path = os.path.join(output_folder, image_name)
    cv2.imwrite(output_path, annotated_frame)
    print(f"Đã lưu ảnh xử lý {i + 1}/{num_images}: {output_path}")

    # Lưu thông tin vào danh sách cục bộ
    image_info = {
        "date": datetime.now().strftime("%Y-%m-%d"),
        "time": datetime.now().strftime("%H:%M:%S"),
        "image_name": image_name,
        "shrimp_count": shrimp_count,
        "average_length_mm": round(avg_length, 2),
        "weight": round(weight, 2) if weight else None,
        "feed_percentage": round(feed_percentage, 2) if feed_percentage else None,
        "feed_amount": round(feed_amount, 2) if feed_amount else None
    }
    image_data.append(image_info)

    # Hiển thị khung hình đã annotate
    cv2.imshow(f"Result for Image {i + 1}", annotated_frame)
    cv2.waitKey(500)  # Hiển thị khung hình trong 500ms

    # Đợi 1 giây trước khi chụp ảnh tiếp theo
    time.sleep(1)

# Giải phóng tài nguyên
cap.release()
cv2.destroyAllWindows()

# Lưu thông tin xử lý vào tệp JSON
log_file = os.path.join(output_folder, "shrimp_detection_log.json")
with open(log_file, "w") as f:
    json.dump(image_data, f, indent=4)
print(f"Thông tin phát hiện tôm đã được lưu vào tệp: {log_file}")

# Gửi ảnh cuối cùng lên Node-RED
node_red_url = "http://192.168.0.189:1880/upload"  # Node-RED URL

def send_to_node_red(image_path):
    try:
        with open(image_path, "rb") as img_file:
            files = {"file": (os.path.basename(image_path), img_file, "image/jpeg")}
            response = requests.post(node_red_url, files=files)
            if response.status_code == 200:
                print(f"Đã gửi ảnh thành công đến Node-RED: {image_path}")
            else:
                print(f"Lỗi khi gửi ảnh lên Node-RED: {response.status_code}, {response.text}")
    except Exception as e:
        print(f"Lỗi khi gửi ảnh lên Node-RED: {e}")

if image_data:
    last_image_path = os.path.join(output_folder, image_data[-1]["image_name"])
    send_to_node_red(last_image_path)
else:
    print("Không có ảnh nào để gửi lên Node-RED.")
