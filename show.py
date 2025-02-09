
def get_feed_data(length_cm):
    # Dữ liệu từ bảng, dựa trên các vạch chia của thước
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
    
    # Tìm khoảng chiều dài phù hợp
    for length_range, (weight, feed_percentage) in length_to_data.items():
        if length_range[0] <= length_cm <= length_range[1]:
            # Trả về trọng lượng thân và tỷ lệ cho ăn
            return weight, feed_percentage

    # Nếu chiều dài không nằm trong khoảng, trả về None
    return None, None

# Nhập dữ liệu từ bàn phím
try:
    length = float(input("Nhập chiều dài của con tôm (cm): "))
    weight, feed_percentage = get_feed_data(length)

    if weight is not None and feed_percentage is not None:
        # Tính lượng thức ăn (gram)
        feed_amount = weight * (feed_percentage / 100)
        print(f"\nThông tin cho con tôm dài {length:.1f} cm:")
        print(f"- Trọng lượng thân trung bình: {weight:.2f} gram.")
        print(f"- Tỷ lệ cho ăn: {feed_percentage:.2f}%.")
        print(f"- Lượng thức ăn cần: {feed_amount:.2f} gram.")
    else:
        print("Chiều dài không nằm trong khoảng được hỗ trợ.")
        
except ValueError:
    print("Vui lòng nhập một số hợp lệ.")
