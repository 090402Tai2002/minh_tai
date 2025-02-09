from ultralytics import YOLO
import cv2
import numpy as np
import os
import time
from datetime import datetime
from scipy.spatial import distance as dist
import json

# Hàm tính điểm trung bình giữa hai điểm
def midpoint(ptA, ptB):
    return ((ptA[0] + ptB[0]) * 0.5, (ptA[1] + ptB[1]) * 0.5)

# Hàm tính chiều dài lớn nhất từ contour
def calculate_shrimp_length(contour, pixels_per_mm):
    contour_points = contour.reshape(-1, 2)
    max_distance = 0
    for i in range(len(contour_points)):
        for j in range(i + 1, len(contour_points)):
            d = dist.euclidean(contour_points[i], contour_points[j])
            max_distance = max(max_distance, d)
    return max_distance / pixels_per_mm  # Chuyển đổi pixel sang mm

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
            return {
                "length_mm": length_mm,
                "weight": weight,
                "feed_percentage": feed_percentage,
                "feed_amount": weight * (feed_percentage / 100)
            }
    return {
        "length_mm": length_mm,
        "weight": None,
        "feed_percentage": None,
        "feed_amount": None
    }

# Tải mô hình YOLO
model = YOLO("best.pt")  # Thay "best.pt" bằng tệp mô hình YOLO của bạn

# Ngưỡng tin cậy
conf_threshold = 0.5

# Thư mục lưu ảnh sau xử lý
output_folder = "processed_images"
os.makedirs(output_folder, exist_ok=True)

# Thư mục lưu thông tin thức ăn
feed_data_folder = "feed_data"
os.makedirs(feed_data_folder, exist_ok=True)
feed_data_file = os.path.join(feed_data_folder, "feed_info.json")

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
feed_data_list = []

for i in range(num_images):
    ret, frame = cap.read()
    if not ret:
        print("Không thể chụp khung hình từ webcam.")
        break

    # Chạy mô hình YOLO để phát hiện đối tượng
    results = model.predict(source=frame, conf=conf_threshold, save=False, show=False)
    shrimp_count = len(results[0].boxes.xyxy)  # Số lượng bounding boxes

    # Tính chiều dài trung bình của tôm
    lengths = []
    for contour in results[0].masks.xy:  # Nếu YOLO hỗ trợ trả về contour
        length_mm = calculate_shrimp_length(contour, pixels_per_mm)
        lengths.append(length_mm)
    avg_length = np.mean(lengths) if lengths else 0

    # Lấy thông tin cho ăn từ chiều dài trung bình
    feed_data = get_feed_data(avg_length)
    feed_data_list.append(feed_data)

    # Annotate khung hình
    annotated_frame = frame.copy()
    if shrimp_count > 0:
        for j, box in enumerate(results[0].boxes.xyxy):
            x_min, y_min, x_max, y_max = map(int, box.tolist())
            cv2.rectangle(annotated_frame, (x_min, y_min), (x_max, y_max), (0, 255, 0), 2)
            cv2.putText(annotated_frame, f"{lengths[j]:.2f} mm", 
                        (x_min, y_min - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 1)

    # Lưu ảnh đã annotate
    image_name = f"image_{i + 1}_processed.jpg"
    output_path = os.path.join(output_folder, image_name)
    cv2.imwrite(output_path, annotated_frame)

    # Lưu thông tin vào danh sách
    image_info = {
        "date": datetime.now().strftime("%Y-%m-%d"),
        "time": datetime.now().strftime("%H:%M:%S"),
        "image_name": image_name,
        "shrimp_count": shrimp_count,
        "average_length_mm": round(avg_length, 2),
        **feed_data
    }
    image_data.append(image_info)

    # Hiển thị khung hình đã annotate
    cv2.imshow(f"Result for Image {i + 1}", annotated_frame)
    cv2.waitKey(500)
    time.sleep(1)

# Giải phóng tài nguyên
cap.release()
cv2.destroyAllWindows()

# Lưu thông tin vào tệp JSON
with open(feed_data_file, "w") as f:
    json.dump(feed_data_list, f, indent=4)
print(f"Thông tin cho ăn đã được lưu vào tệp: {feed_data_file}")
