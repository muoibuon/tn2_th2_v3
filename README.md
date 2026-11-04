# TH2 — Timer, non-blocking, Deep Sleep (AET2022)

**Sinh viên:** Trần Ngô Tiến Đạt · Phạm Hồng Duyên · Dương Việt Hoàng · Phạm Quý Anh

Board ESP32 DevKit · Sơ đồ dùng chung TH1 (`fritzing/`) · Ảnh thực tế (`thuc_te/`)

Nạp: `cd bai1` → `pio run -e <env> -t upload` → `pio device monitor`

## Bài 1 — Timer phần cứng & non-blocking
- `vd1_dung_delay`: LED 500 ms, đọc biến trở 2000 ms, heartbeat 5000 ms viết nối tiếp bằng `delay()`.
- `vd2_non_blocking`: cùng 3 việc, viết non-blocking bằng `millis()`.


## Bài 2 — Deep sleep & các chế độ tiết kiệm điện
- `vd1_rtc_data_attr`: biến `RTC_DATA_ATTR` giữ qua deep sleep, biến thường bị mất.
- `vd2_thuc_day_setup`: thức dậy chạy lại từ `setup()`, đọc nguyên nhân thức (timer / nút GPIO4).
