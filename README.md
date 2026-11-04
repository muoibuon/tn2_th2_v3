# TH2 — Timer, non-blocking, Deep Sleep (AET2022)

Họ tên: … · MSSV: … · Board: ESP32 DevKit (ESP32-D0WD-V3, CH340)

> Slide mục lục ghi 4 bài, nhưng nội dung và yêu cầu nộp là **2 bài, mỗi bài vd1/vd2 + bài tập** → repo theo 2 bài.

## Cấu trúc

| Bài | File | Nội dung |
|---|---|---|
| **Bài 1** | `vd1_dung_delay.cpp` | 3 việc nối tiếp bằng `delay()` |
| | `vd2_non_blocking.cpp` | 3 việc non-blocking bằng `millis()` |
| | `bai_tap_so_sanh_3_cach.cpp` | Đo lệch nhịp: delay / millis / timer phần cứng |
| **Bài 2** | `vd1_rtc_data_attr.cpp` | Biến giữ (RTC) / biến mất qua deep sleep |
| | `vd2_thuc_day_setup.cpp` | Thức = chạy lại `setup()`, đọc wakeup cause (timer / nút ext0) |
| | `bai_tap_che_do_nang_luong.cpp` | Active → Modem → Light → Deep, quan sát qua LED, Serial, web, OLED |

Nạp: `cd bai1` → `pio run -e <env> -t upload` → `pio device monitor`.
Mô phỏng: mở thư mục bài trong VS Code → F1 → *Wokwi: Start Simulator*.

## Sơ đồ & ảnh

- `fritzing/` — sơ đồ Fritzing (`.fzz`). Bài 1, Bài 2 dùng chung sơ đồ TH1.
- `thuc_te/` — ảnh mạch thật và ảnh kết quả.

Đấu nối: LED → GPIO2 (330 Ω) · biến trở → GPIO34 · nút GPIO4 ↔ GND (+10 kΩ lên 3V3 cho deep sleep).
Bài 2 bài tập thêm OLED (tùy chọn): SDA 21, SCL 22, 3V3, GND.

## Bài 1 — Kết quả (chu kỳ 100 ms, CPU bận 0–30 ms)

| Cách | Lệch TB | Lệch max |
|---|---|---|
| `delay()` | ≈ 16 000 µs | 34 012 µs |
| `millis()` | ≈ 7 800 µs | 27 072 µs |
| Timer phần cứng | ≈ 0,1 µs | 1 µs |

**Kết luận:** `delay()` cộng dồn trễ, luôn chậm. `millis()` giữ nhịp trung bình nhưng còn jitter vì phải chờ `loop()`. Timer phần cứng tạo ngắt độc lập với `loop()` nên gần như không lệch.

## Bài 2 — Kết quả

| Chế độ | LED / Wi-Fi | RAM thường | Sau khi thức |
|---|---|---|---|
| Active | … | … | … |
| Modem-sleep | … | … | … |
| Light-sleep | … | … | … |
| Deep-sleep | … | … | … |

`soLanThuc` (RTC) qua các lần thức: … · `demThuong`: … · wakeup cause: timer = …, nút = …

**Giới hạn:** không đo dòng thật — dòng ngủ cỡ µA, và LED nguồn, CH340, LDO trên board tiêu thụ thêm.
