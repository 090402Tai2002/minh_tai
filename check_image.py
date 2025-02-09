from ultralytics import YOLO
import cv2
import numpy as np
import os
import time
from datetime import datetime
from scipy.spatial import distance as dist
import json

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

# Hàm tính chiều dài lớn nhất từ contour
def calculate_shrimp_length(contour, pixels_per_mm):
    contour_points = contour.reshape(-1, 2)
    max_distance = 0
    for i in range(len(contour_points)):
        for j in range(i + 1, len(contour_points)):
            d = dist.euclidean(contour_points[i], contour_points[j])
            max_distance = max(max_distance, d)
    return max_distance / pixels_per_mm  # Chuyển đổi pixel sang mm

# Tải mô hình YOLO
model = YOLO("best.pt")  # Thay bằng tệp mô hình của bạn

# Ngưỡng tin cậy
conf_threshold = 0.5

# Thư mục lưu ảnh sau xử lý
output_folder = "processed_images"
os.makedirs(output_folder, exist_ok=True)

# Tệp JSON để lưu thông tin hiệu chỉnh
calibration_file = "calibration_data.json"

# Đọc hệ số pixels/mm từ tệp JSON
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
    print(f"Số lượng tôm phát hiện: {shrimp_count}")

    # Khởi tạo danh sách lưu chiều dài từng tôm
    lengths = []

    # Annotate khung hình để hiển thị thông tin
    annotated_frame = frame.copy()

    # Nếu có đối tượng, xử lý từng bounding box
    if shrimp_count > 0:
        for j, box in enumerate(results[0].boxes.xyxy):  # Lấy tọa độ của bounding boxes
            x_min, y_min, x_max, y_max = map(int, box.tolist())  # Chuyển tọa độ sang kiểu int
            shrimp_roi = frame[y_min:y_max, x_min:x_max]  # Cắt vùng chứa tôm

            # Xử lý ảnh để tìm contour
            gray = cv2.cvtColor(shrimp_roi, cv2.COLOR_BGR2GRAY)
            blurred = cv2.GaussianBlur(gray, (5, 5), 0)
            _, thresh = cv2.threshold(blurred, 50, 255, cv2.THRESH_BINARY_INV + cv2.THRESH_OTSU)

            # Tìm contour lớn nhất
            contours, _ = cv2.findContours(thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
            if contours:
                largest_contour = max(contours, key=cv2.contourArea)

                # Tính chiều dài tôm bằng contour
                length_mm = calculate_shrimp_length(largest_contour, pixels_per_mm)
                lengths.append(length_mm)

                # Vẽ contour và ghi thông tin lên khung hình
                cv2.drawContours(shrimp_roi, [largest_contour], -1, (0, 255, 0), 2)
                cv2.putText(annotated_frame, f"Tôm {j + 1}: {length_mm:.2f} mm", 
                            (x_min, y_min - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)
                print(f"- Tôm {j + 1}: Chiều dài: {length_mm:.2f} mm")

    # Tính chiều dài trung bình nếu có đối tượng
    avg_length = np.mean(lengths) if lengths else 0
    if avg_length > 0:
        cv2.putText(annotated_frame, f"Chiều dài trung bình: {avg_length:.2f} mm", (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 0), 2)
        print(f"Chiều dài trung bình của tôm: {avg_length:.2f} mm")
    else:
        cv2.putText(annotated_frame, "Không phát hiện tôm", (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 0, 255), 2)
        print("Không phát hiện được tôm trong khung hình.")

    # Lưu ảnh đã annotate
    image_name = f"image_{i + 1}_processed.jpg"
    output_path = os.path.join(output_folder, image_name)
    cv2.imwrite(output_path, annotated_frame)
    print(f"Đã lưu ảnh xử lý {i + 1}/{num_images}: {output_path}")

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
