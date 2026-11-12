# LAB 3 – ĐỀ 4

## HỆ THỐNG GIÁM SÁT VÀ ĐIỀU KHIỂN MÔI TRƯỜNG BỂ NUÔI THỦY SẢN

**Công nghệ:** ESP32, FreeRTOS, MQTT, HTML, CSS, JavaScript

## 1. Giới thiệu
Dự án xây dựng hệ thống theo dõi mực nước và nhiệt độ nước trong bể nuôi. ESP32 đọc dữ liệu cảm biến, xử lý bằng FreeRTOS và gửi dữ liệu lên MQTT Broker để Website hiển thị. Hệ thống cũng hỗ trợ điều khiển máy bơm và van xả.

> Cập nhật nội dung này sau khi hoàn thành mô phỏng và kiểm thử thực tế.

## 2. Chức năng chính
- Đo khoảng cách từ cảm biến HC-SR04 đến mặt nước.
- Đo nhiệt độ nước bằng cảm biến DS18B20.
- Hiển thị dữ liệu trên Serial Monitor.
- Sử dụng các Task và Queue của FreeRTOS để xử lý dữ liệu.
- Gửi dữ liệu lên MQTT Broker.
- Hiển thị giá trị và biểu đồ trên Website.
- Điều khiển máy bơm và van xả từ Website ở chế độ thủ công.
- Tự động điều khiển theo ngưỡng mực nước được quy định trong đề tài.

## 3. Phần cứng và công nghệ
| Thành phần | Vai trò |
|---|---|
| ESP32 | Vi điều khiển trung tâm |
| HC-SR04 | Đo khoảng cách đến mặt nước |
| DS18B20 | Đo nhiệt độ nước |
| Máy bơm | Cấp nước cho bể |
| Van xả | Xả nước khỏi bể |
| FreeRTOS | Quản lý các tác vụ |
| MQTT | Truyền dữ liệu và lệnh điều khiển |
| HTML, CSS, JavaScript | Xây dựng Website |

## 4. Sơ đồ hệ thống
Thêm ảnh sơ đồ tổng thể vào `Images/` rồi bỏ chú thích bên dưới:

<!-- ![Sơ đồ hệ thống](Images/system_diagram.png) -->

## 5. Cấu trúc chương trình
- **Task đọc dữ liệu:** đọc cảm biến, in dữ liệu ra Serial và gửi dữ liệu vào Queue.
- **Task truyền dữ liệu:** kết nối Wi-Fi, nhận dữ liệu từ Queue và publish lên MQTT.
- **Task cảnh báo:** nhận dữ liệu từ Queue cảnh báo và xử lý điều khiển theo ngưỡng.

Cập nhật mô tả nếu cách phân chia Task trong chương trình cuối cùng có thay đổi.

## 6. Giao diện Website
Ảnh giao diện được lưu trong `Images/`.

<!-- ![Giao diện Website](Images/dashboard.png) -->

## 7. Mô phỏng Wokwi
- Link dự án Wokwi: **Dán liên kết Wokwi tại đây**
- File sơ đồ mạch: `Wokwi/diagram.json`

## 8. Hướng dẫn chạy
1. Mở dự án Wokwi theo liên kết ở mục 7 hoặc mở sơ đồ trong thư mục `Wokwi/`.
2. Mở chương trình ESP32 trong thư mục `ESP32/`.
3. Kiểm tra cấu hình Wi-Fi và MQTT trước khi chạy.
4. Chạy mô phỏng và kiểm tra dữ liệu trên Serial Monitor.
5. Mở `Web/index.html` bằng trình duyệt hoặc triển khai Website theo cách được hướng dẫn trong phần Web.
6. Đảm bảo Website và ESP32 sử dụng cùng MQTT Broker và cùng tên topic.

> Điền địa chỉ Broker, cổng kết nối, topic và thư viện cần thiết sau khi chốt cấu hình. Không đưa mật khẩu hoặc thông tin bí mật lên repository công khai.

## 9. Kết quả thực hiện
Thêm ảnh kiểm thử vào `Images/`, ví dụ:
- `circuit.png`: mạch mô phỏng.
- `serial_monitor.png`: dữ liệu trên Serial Monitor.
- `dashboard.png`: giao diện Website.
- `mqtt_test.png`: kiểm tra trao đổi MQTT (nếu có).

Chỉ đưa vào đây kết quả đã chạy và kiểm tra thực tế.

## 10. Báo cáo
Báo cáo hoàn chỉnh được lưu tại `Report/Bao_cao.pdf`.

## 11. Tác giả
- Họ tên: Nguyễn Hứu Huy
- Lớp: N02
