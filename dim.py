import cv2
import numpy as np
from scipy.spatial import distance as dist
from imutils import perspective
from imutils import contours
import imutils
import os
import time
import json

# Hàm tính điểm trung bình giữa hai điểm
def midpoint(ptA, ptB):
    return ((ptA[0] + ptB[0]) * 0.5, (ptA[1] + ptB[1]) * 0.5)

# Chiều dài cạnh của hình vuông tham chiếu (mm)
REF_SQUARE_SIDE_MM = 40  # Thay bằng chiều dài cạnh thực tế của hình vuông tham chiếu

# Mở webcam
cap = cv2.VideoCapture(0)

# Kiểm tra nếu webcam không mở được
if not cap.isOpened():
    print("Không thể mở webcam!")
    exit()

# Tạo thư mục lưu ảnh nếu chưa tồn tại
output_folder = "processed_images"
os.makedirs(output_folder, exist_ok=True)

# Danh sách lưu hệ số pixels/mm
pixelsPerMetric_list = []
num_images = 5

print(f"Đang tự động chụp {num_images} lần để tính hệ số pixels/mm trung bình...")

for i in range(num_images):
    print(f"Chụp lần {i + 1}/{num_images}...")
    ret, frame = cap.read()
    if not ret:
        print("Không thể nhận khung hình từ webcam!")
        break

    # Tiền xử lý ảnh
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)  # Chuyển sang ảnh xám
    gray = cv2.GaussianBlur(gray, (7, 7), 0)       # Làm mịn ảnh
    edged = cv2.Canny(gray, 50, 100)               # Phát hiện cạnh
    edged = cv2.dilate(edged, None, iterations=1)  # Giãn nở cạnh
    edged = cv2.erode(edged, None, iterations=1)   # Xói mòn để làm sạch cạnh

    # Tìm các contours trong ảnh
    cnts = cv2.findContours(edged.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    cnts = imutils.grab_contours(cnts)

    # Kiểm tra nếu không tìm thấy contours
    if len(cnts) == 0:
        print("Không tìm thấy contours nào!")
        continue

    # Sắp xếp contours từ trái sang phải
    (cnts, _) = contours.sort_contours(cnts)

    for c in cnts:
        # Bỏ qua các contours nhỏ
        if cv2.contourArea(c) < 100:
            continue

        # Tính toán hộp bao quanh
        box = cv2.minAreaRect(c)
        box = cv2.boxPoints(box) if imutils.is_cv3() else cv2.boxPoints(box)
        box = np.array(box, dtype="int")

        # Sắp xếp các điểm và vẽ hộp bao quanh
        box = perspective.order_points(box)
        cv2.drawContours(frame, [box.astype("int")], -1, (0, 255, 0), 2)

        # Tính các điểm trung bình
        (tl, tr, br, bl) = box
        (tltrX, tltrY) = midpoint(tl, tr)
        (blbrX, blbrY) = midpoint(bl, br)
        (tlblX, tlblY) = midpoint(tl, bl)
        (trbrX, trbrY) = midpoint(tr, br)

        # Tính khoảng cách giữa các điểm trung bình (cạnh bounding box)
        dA = dist.euclidean((tltrX, tltrY), (blbrX, blbrY))  # Chiều dài
        dB = dist.euclidean((tlblX, tlblY), (trbrX, trbrY))  # Chiều rộng

        # Tính hệ số pixels/mm
        pixelsPerMetric_mm = dB / REF_SQUARE_SIDE_MM
        pixelsPerMetric_list.append(pixelsPerMetric_mm)

        # Hiển thị hệ số pixels/mm lên khung hình
        cv2.putText(frame, f"Pixels/mm: {pixelsPerMetric_mm:.2f}",
                    (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 2)

        # Hiển thị khung hình chính với kết quả
        cv2.imshow("Processed Image", frame)

        # Lưu ảnh đã xử lý vào thư mục
        image_name = f"processed_image_{i + 1}.jpg"
        output_path = os.path.join(output_folder, image_name)
        cv2.imwrite(output_path, frame)
        print(f"Ảnh đã xử lý được lưu tại: {output_path}")

        # Chờ 1 giây trước khi chuyển sang lần chụp tiếp theo
        cv2.waitKey(1000)
        break

# Tính hệ số pixels/mm trung bình
average_pixelsPerMetric = np.mean(pixelsPerMetric_list) if pixelsPerMetric_list else 0
print(f"Hệ số pixels/mm trung bình: {average_pixelsPerMetric:.2f}")

# Lưu hệ số pixels/mm vào tệp JSON
calibration_file = "calibration_data.json"
with open(calibration_file, "w") as f:
    json.dump({"average_pixels_per_mm": average_pixelsPerMetric}, f, indent=4)
print(f"Hệ số pixels/mm trung bình đã được lưu tại: {calibration_file}")

# Giải phóng tài nguyên
cap.release()
cv2.destroyAllWindows()
