from kivy.app import App
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.button import Button
from kivy.uix.label import Label
from kivy.clock import Clock
from kivy.core.window import Window
from kivy.uix.filechooser import FileChooserListView
from kivy.uix.popup import Popup

import asyncio
from bleak import BleakClient, BleakScanner
import threading
import os

# --- BLE config ---
OTA_NAME = "DUY"
OTA_CHAR_UUID = "51adc254-f484-46db-97d6-d6037473cfc7"
CHUNK_SIZE = 512

class OTAApp(App):
    def build(self):
        self.firmware_path = None

        Window.size = (500, 320)
        layout = BoxLayout(orientation='vertical', padding=20, spacing=20)
        self.label = Label(
            text="Chọn file firmware .bin trước khi update",
            size_hint=(1, 0.2),
            halign="left",
            valign="top"
        )
        self.label.bind(size=self.label.setter('text_size'))

        self.btn_choose = Button(text="Chọn firmware .bin", size_hint=(1, 0.2))
        self.btn_update = Button(text="Update Firmware", size_hint=(1, 0.2))

        self.btn_choose.bind(on_press=self.choose_file)
        self.btn_update.bind(on_press=self.on_update_pressed)

        layout.add_widget(self.label)
        layout.add_widget(self.btn_choose)
        layout.add_widget(self.btn_update)

        return layout

    def choose_file(self, instance):
        chooser = FileChooserListView(filters=["*.bin"])
        popup = Popup(title="Chọn file .bin", content=chooser, size_hint=(0.9, 0.9))
        chooser.bind(on_submit=lambda fc, selection, touch: self.file_chosen(popup, selection))
        popup.open()

    def file_chosen(self, popup, selection):
        if selection:
            self.firmware_path = selection[0]
            self.label.text = f"File đã chọn: {os.path.basename(self.firmware_path)}"
        popup.dismiss()

    def on_update_pressed(self, instance):
        if not self.firmware_path:
            self.label.text = "Vui lòng chọn file .bin trước khi update"
            return
        self.label.text = "Bắt đầu gửi firmware..."
        threading.Thread(target=lambda: asyncio.run(self.send_firmware())).start()

    async def find_device(self, name, timeout=10):
        for _ in range(timeout):
            devices = await BleakScanner.discover()
            for d in devices:
                if d.name == name:
                    return d
            await asyncio.sleep(1)
        return None

    async def send_firmware(self):
        self.update_label("🔍 Tìm ESP32-OTA...")
        device = await self.find_device(OTA_NAME)
        if not device:
            self.update_label(" Không tìm thấy thiết bị OTA")
            return

        try:
            async with BleakClient(device) as client:
                if not client.is_connected:
                    self.update_label("Không thể kết nối ESP32-OTA")
                    return

                # Bắt đầu nhận notify từ ESP
                ack_event = asyncio.Event()
                def handle_notify(sender, data):
                    msg = data.decode(errors='ignore').strip()
                    # Hiển thị phản hồi lên label (giữ lại các dòng)
                    self.append_label(f"ESP: {msg}")
                    if msg == "OK":
                        ack_event.set()
                    elif msg == "UPDATE_SUCCESS":
                        self.append_label(" ESP đã cập nhật thành công!")

                await client.start_notify(OTA_CHAR_UUID, handle_notify)

                self.update_label("Gửi 'OTA_BEGIN'...")
                await client.write_gatt_char(OTA_CHAR_UUID, b"OTA_BEGIN")
                await asyncio.sleep(0.5)

                self.update_label("Gửi firmware OTA...")
                with open(self.firmware_path, "rb") as f:
                    chunk_num = 0
                    while True:
                        chunk = f.read(CHUNK_SIZE)
                        if not chunk:
                            break
                        ack_event.clear()
                        await client.write_gatt_char(OTA_CHAR_UUID, chunk)
                        self.append_label(f"Chunk {chunk_num} ({len(chunk)} bytes)")
                        try:
                            await asyncio.wait_for(ack_event.wait(), timeout=2)
                        except asyncio.TimeoutError:
                            self.append_label(f"Timeout ở chunk {chunk_num}, hủy tiến trình.")
                            await client.stop_notify(OTA_CHAR_UUID)
                            return
                        chunk_num += 1
                        await asyncio.sleep(0.01)

                self.update_label("Gửi 'EOF' kết thúc OTA...")
                await client.write_gatt_char(OTA_CHAR_UUID, b"EOF")
                await asyncio.sleep(1)

                await client.stop_notify(OTA_CHAR_UUID)
                self.append_label("Gửi xong toàn bộ firmware")
        except Exception as e:
            self.update_label(f" OTA lỗi: {e}")


    def update_label(self, msg):
        # Ghi đè label
        Clock.schedule_once(lambda dt: self.label.setter('text')(self.label, msg))

    def append_label(self, msg):
        # Thêm dòng mới vào label, giữ lại tối đa 10 dòng
        def update(dt):
            lines = self.label.text.split('\n')
            lines.append(msg)
            if len(lines) > 10:
                lines = lines[-10:]
            self.label.text = '\n'.join(lines)
        Clock.schedule_once(update)

if __name__ == "__main__":
    OTAApp().run()