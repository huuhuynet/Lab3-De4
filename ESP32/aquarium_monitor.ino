/*
  ESP32 Aquarium Monitoring System

  TODO:
  1. Khai báo chân HC-SR04, DS18B20, máy bơm và van xả.
  2. Khởi tạo Queue và các Task FreeRTOS.
  3. Đọc cảm biến và in dữ liệu ra Serial.
  4. Kết nối Wi-Fi và MQTT.
  5. Publish dữ liệu và nhận lệnh điều khiển.

  Chưa phải chương trình hoàn chỉnh.
*/

void setup() {
  Serial.begin(115200);
}

void loop() {
  // Các tác vụ chính sẽ được triển khai bằng FreeRTOS.
}
