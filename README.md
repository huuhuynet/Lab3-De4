# LAB 3 – ĐỀ 4
## HỆ THỐNG GIÁM SÁT VÀ ĐIỀU KHIỂN MÔI TRƯỜNG BỂ NUÔI THỦY SẢN

Hệ thống sử dụng ESP32 và FreeRTOS để giám sát nhiệt độ, mực nước trong bể nuôi. Dữ liệu được truyền qua MQTT đến Website để theo dõi và điều khiển máy bơm, van xả.

### 1. Công nghệ sử dụng

- **Phần cứng:** ESP32, HC-SR04, DS18B20, LED mô phỏng máy bơm và van xả.
- **Hệ điều hành thời gian thực:** FreeRTOS.
- **Giao tiếp:** Wi-Fi, MQTT.
- **Website:** HTML, CSS, JavaScript, Chart.js.
- **Môi trường mô phỏng:** Wokwi.
- **MQTT Broker:** HiveMQ Public Broker.

### 2. Chức năng hệ thống

- Đo khoảng cách từ cảm biến đến mặt nước và nhiệt độ nước.
- Hiển thị dữ liệu trên Serial Monitor và Website.
- Biểu diễn dữ liệu bằng biểu đồ theo thời gian.
- Tự động điều khiển máy bơm và van xả theo ngưỡng khoảng cách.
- Hỗ trợ hai chế độ AUTO và MANUAL.
- Điều khiển thiết bị từ Website thông qua MQTT.

### 3. Cấu trúc dự án

```text
Lab3-De4/
├── ESP32/       # Chương trình điều khiển ESP32
├── Wokwi/       # Sơ đồ mạch mô phỏng
├── Web/         # Giao diện giám sát và điều khiển
├── Images/      # Hình ảnh minh họa, kiểm thử
├── Report/      # Báo cáo đồ án
├── README.md
└── .gitignore
```

### 4. Hướng dẫn chạy

1. Mở dự án Wokwi và khởi chạy mô phỏng ESP32.
2. Kiểm tra kết nối Wi-Fi và MQTT Broker.
3. Mở file `Web/index.html` bằng trình duyệt.
4. Theo dõi dữ liệu cảm biến và thử nghiệm các chức năng điều khiển trên Website.

**Cấu hình MQTT:**

- Broker: `broker.hivemq.com`
- Port: `1883`
- Sensor topic: `lab3de4/aquarium/demo01/sensor`
- State topic: `lab3de4/aquarium/demo01/state`

### 5. Báo cáo

Báo cáo chi tiết được lưu tại thư mục `Report/`.

### 6. Tác giả

- **Họ tên:** Nguyễn Hứu Huy
- **Lớp:** N02