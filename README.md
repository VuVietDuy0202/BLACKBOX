# 🏗️ Smart Construction Monitor - Hệ Thống Giám Sát An Toàn Công Trình

![PlatformIO](https://img.shields.io/badge/PlatformIO-Core-orange)
![ESP32](https://img.shields.io/badge/Device-ESP32-blue)
![Status](https://img.shields.io/badge/Status-Active-brightgreen)

## 1. Mô tả dự án (Project Description)

**Đặt vấn đề:**
Hiện nay, các công trình xây dựng ngày càng có quy mô lớn và độ cao phức tạp, đi kèm với đó là những rủi ro tiềm ẩn về an toàn lao động và chất lượng kết cấu. Các vấn đề như độ rung lắc bất thường, giàn giáo bị nghiêng, hay việc kiểm soát nhiệt độ/độ ẩm để bảo dưỡng bê tông cần được giám sát chặt chẽ 24/7. Tuy nhiên, việc giám sát thủ công thường tốn kém và không báo cáo kịp thời khi sự cố xảy ra.

**Giải pháp:**
Dự án **Smart Construction Monitor** là thiết bị IoT nhỏ gọn được gắn trực tiếp lên các kết cấu (giàn giáo, cốp pha, cột trụ). Thiết bị giám sát liên tục độ nghiêng, độ rung chấn và môi trường, gửi dữ liệu thời gian thực về trung tâm điều hành và phát cảnh báo ngay lập tức giúp ngăn chặn tai nạn.

## 2. Chức năng chính (Key Features)

- 📐 **Giám sát kết cấu (Structural Health):**
  - **Góc nghiêng (Tilt):** Đo độ nghiêng trục X/Y để phát hiện giàn giáo bị lệch tâm, lún móng.
  - **Độ rung (Vibration):** Phát hiện rung chấn mạnh do gió bão hoặc máy móc va đập.
- 🌡️ **Giám sát môi trường (Environment):**
  - **Bê tông:** Giám sát nhiệt độ/độ ẩm để đảm bảo quy trình đóng rắn (Curing) của bê tông.
  - **An toàn:** Phát hiện nồng độ khí Gas/Khói vượt ngưỡng cho phép.
- 🚨 **Cảnh báo đa tầng (Multi-level Alert):**
  - **Tại chỗ:** Còi Buzzer và Đèn LED cảnh báo ngay khi phát hiện nguy hiểm.
  - **Từ xa:** Gửi tín hiệu báo động về Dashboard Node-RED qua Internet.
- ☁️ **Giám sát thực (Real-time Monitoring):** Gửi gói tin dữ liệu mỗi **1 giây/lần** giúp vẽ biểu đồ chi tiết và phản ứng tức thì.
- 📲 **Cập nhật không dây (OTA):** Nạp firmware từ xa qua WiFi hoặc Bluetooth (BLE) mà không cần tháo thiết bị.

## 3. Phần cứng sử dụng (Hardware)

| Thành phần | Chức năng |
| :--- | :--- |
| **ESP32 (S3/S2)** | Vi điều khiển trung tâm, xử lý WiFi/BLE |
| **MPU6050** | Cảm biến Gia tốc & Con quay hồi chuyển (Đo nghiêng/rung) |
| **DHT11** | Cảm biến Nhiệt độ, Độ ẩm |
| **MQ-2** | Cảm biến Khí Gas/Khói |
| **Active Buzzer** | Còi báo động âm thanh |
| **2 x LEDs** | Đèn báo trạng thái (Warning/Critical) |
| **Nguồn** | 5V DC (Pin dự phòng hoặc Adapter) |

## 4. Sơ đồ kết nối (Pinout)

<img width="799" height="723" alt="Screenshot 2025-12-11 212946" src="https://github.com/user-attachments/assets/ffbd28e2-c769-48f9-8492-d4af095a5c11" />

| Thiết bị | Chân thiết bị | ESP32 GPIO | Giao tiếp |
| :--- | :--- | :--- | :--- |
| **MPU6050** | SDA | 5 | I2C |
| | SCL | 7 | I2C |
| **DHT11** | DATA | 40 | Digital |
| **Gas Sensor**| AO (Analog) | 8 | Analog |
| **Buzzer** | VCC/SIG | 6 | Digital Out |
| **LED 1** | Anode (+) | 35 | Digital Out |
| **LED 2** | Anode (+) | 36 | Digital Out |
| **Button** | BOOT | 0 | Input |

## 5. Phần mềm & Công nghệ

- **Firmware:** C++ (PlatformIO).
  - Thuật toán: Bộ lọc (Filter) để làm mượt dữ liệu góc nghiêng.
- **Giao thức:** MQTT (truyền tải JSON payload 1s/lần).
- **Backend/Frontend:** Node-RED Dashboard.
- **OTA:** Custom BLE Service.

## 6. Luồng hoạt động (Workflow)

Hệ thống hoạt động theo chu kỳ thời gian thực (Real-time Loop):

1.  **Thu thập (Sensing):** Đọc dữ liệu từ MPU6050, DHT11, Gas Sensor liên tục.
2.  **Phân tích & So sánh ngưỡng (Logic):**

    * **Mức cảnh báo (Warning):** Đèn LED sáng
        - Góc nghiêng > `10.0°`
        - Độ rung > `0.5 G`
        - Khí Gas > `300 PPM`
    * **Mức nguy hiểm (Critical):** **Còi Buzzer hú + LED nháy**
        - Góc nghiêng > `15.0°` (Nguy cơ đổ sập)
        - Độ rung > `1.0 G`
        - Khí Gas > `800 PPM` (Nguy cơ cháy/ngạt)
    * **Giám sát Bê tông:**
        - Cảnh báo nếu Nhiệt độ > `35°C` hoặc Độ ẩm < `50%`.

3.  **Truyền tải (Telemetry):**
    - Đóng gói toàn bộ thông số và trạng thái vào JSON.
    - Publish lên MQTT Broker với chu kỳ **1 giây**.

4.  **Hiển thị & Lưu trữ (Server-side):**
    - Node-RED vẽ biểu đồ theo thời gian thực.
    - Tự động ghi lại nhật ký (Log History) khi có sự kiện Warning/Critical xảy ra.

## 7. Giao diện và Kết quả Thực nghiệm (Interface & Results)

Hệ thống cung cấp Dashboard trực quan giúp kỹ sư giám sát toàn diện tình trạng công trình.

1. **Bảng điều khiển trung tâm (Main Dashboard)**
Tổng quan các thông số quan trọng (Rung, Nghiêng, Môi trường).
<img width="100%" alt="Main Dashboard" src="https://github.com/user-attachments/assets/e9c22302-a90b-400c-bce4-2877ad6482e3" />

---

2. **Giám sát trạng thái kết nối**
Hệ thống tự động báo lỗi khi mất kết nối WiFi hoặc mất nguồn thiết bị.
<p align="center">
  <img width="80%" alt="Connection Lost" src="https://github.com/user-attachments/assets/e6a4d4f8-ec48-4d7c-bba1-2763ecc9f8bf" />
</p>

---

3. **Cảnh báo an toàn (Safety Alert)**
Đây là tính năng quan trọng nhất. Khi cảm biến phát hiện **Chỉ số vượt ngưỡng an toàn** (ví dụ: góc nghiêng > 15°, nồng độ gas > 300 ppm), giao diện lập tức phát cảnh báo, hiển thị vị trí và thông số vi phạm để kích hoạt quy trình sơ tán hoặc khắc phục.

Góc nghiêng > 15°
<img width="100%" alt="Tilt Warning" src="https://github.com/user-attachments/assets/fb9d5d69-2bde-4ddf-865b-b82bf597c997" />

---

Nồng độ gas > 300 ppm
<img width="1880" height="902" alt="Screenshot 2025-12-11 202640" src="https://github.com/user-attachments/assets/4ffa9e6c-f19c-4b82-9454-8329fca0c1c4" />

4. **Chi tiết thông số (Detailed Monitoring)**
Thanh điều hướng giúp truy cập các dữ liệu chuyên sâu.
<p align="left">
  <img width="250" alt="Navigation Menu" src="https://github.com/user-attachments/assets/16ebfc71-91c7-4e95-9618-ebda086b67ea" />
</p>

* **Tab Sensor:** Theo dõi biểu đồ Nhiệt độ, Độ ẩm, Khí Gas.
<img width="100%" alt="Sensor Detail" src="https://github.com/user-attachments/assets/34a46ca4-9e8e-4094-b5ef-ed4b118d974a" />

* **Tab Motion:** Phân tích góc nghiêng (Roll/Pitch) và độ rung tổng hợp.
<img width="100%" alt="Motion Detail" src="https://github.com/user-attachments/assets/a68a5f23-a5f5-4899-87da-aac4eb70c62f" />

* **Tab Alerts** đóng vai trò là trung tâm điều hành và bảo trì hệ thống, bao gồm 3 chức năng cốt lõi:

* ***Điều khiển Còi báo (Alarm Control):*** Tính năng can thiệp thủ công (Manual Override). Người vận hành có thể chủ động **BẬT** còi (để diễn tập sơ tán hoặc cảnh báo khẩn cấp) hoặc **TẮT** còi (sau khi đã xử lý xong sự cố) trực tiếp từ giao diện web.
* ***Nhật ký Sự cố (Alert History):*** Thay thế cho thẻ nhớ vật lý, bảng này tự động lưu trữ lịch sử các lần vi phạm an toàn (Rung/Nghiêng/Gas) kèm mốc thời gian và mức độ nghiêm trọng (Severity). Giúp kỹ sư dễ dàng truy xuất dữ liệu quá khứ.
* ***Cập nhật Bảo mật (Secure OTA):*** Khu vực nạp Firmware từ xa được bảo vệ bằng mã xác nhận, đảm bảo chỉ người có thẩm quyền mới được phép can thiệp vào hệ thống.
  
<img width="1871" height="862" alt="Screenshot 2025-12-12 002051" src="https://github.com/user-attachments/assets/fc957912-fda6-4514-9b80-6e52e7921d8e" />

## 8. Hướng dẫn OTA (Cập nhật từ xa)

Hệ thống áp dụng kiến trúc **Dual-Partition** (Phân vùng Factory & App) giúp thiết bị không bao giờ bị hỏng (Anti-brick) ngay cả khi quá trình cập nhật gặp lỗi.

- **BLE OTA (Quy trình an toàn):**
  1. **Kích hoạt:** Truy cập Dashboard, nhập **Mã xác nhận (Password)** để thiết bị khởi động lại vào phân vùng **Factory Mode** (chế độ chờ nạp chuyên biệt).
  2. **Nạp Firmware:** Sử dụng ứng dụng `app_ble_v1.py` trên máy tính để kết nối Bluetooth và gửi file firmware `.bin` mới.
  3. **Hoàn tất:** Sau khi nhận đủ dữ liệu, hệ thống tự động xác thực và chuyển khởi động về phân vùng Ứng dụng (Application) mới.
