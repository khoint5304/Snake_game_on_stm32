# 🔥 Hướng dẫn ghi Audio từ audio_data.h vào ISD1820

## 📋 Tổng quan

File này hướng dẫn cách ghi audio từ file `audio_data.h` (trong code STM32) vào module ISD1820 một lần duy nhất. Sau khi ghi xong, module sẽ tự động phát audio đó khi được trigger qua PE9.

## 🔌 Bước 1: Kết nối Hardware (Tạm thời)

**Cần chuẩn bị:**
- 1 điện trở 1kΩ (hoặc 470Ω - 2.2kΩ)
- 2-3 dây jumper

**Sơ đồ kết nối:**

```
STM32 PE8 → Điện trở 1kΩ → ISD1820 MIC (hoặc A1/Audio In)
STM32 GND → ISD1820 GND (nếu chưa nối)
```

**Lưu ý:**
- Điện trở để giảm biên độ tín hiệu từ PE8 (3.3V digital) xuống mức phù hợp cho MIC input
- Đây là kết nối **TẠM THỜI** - chỉ cần khi ghi audio
- Sau khi ghi xong, **ngắt dây** khỏi MIC

**Các kết nối khác giữ nguyên:**
- PE9 → ISD1820 PLAY-L/PLAY-E ✅
- PE10 → ISD1820 REC ✅
- Loa → ISD1820 SP+/SP- ✅
- VCC, GND đã kết nối ✅

## 💻 Bước 2: Thêm Code vào main()

Mở file [main.c](Snake/Core/Src/main.c) và thêm code sau:

**Tìm đoạn này (khoảng dòng 218):**
```c
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */
```

**Thay thế bằng:**
```c
  /* USER CODE BEGIN 2 */
  
  /* ========== BURN AUDIO TO ISD1820 - RUN ONCE ========== */
  /* IMPORTANT: 
   * 1. Connect: PE8 → 1kΩ resistor → ISD1820 MIC
   * 2. Build and flash this code
   * 3. Wait ~10 seconds, you'll hear audio playback test
   * 4. After successful burn, COMMENT OUT the line below
   * 5. Disconnect the PE8 → MIC wire
   * 6. Rebuild and reflash
   */
  Snake_BurnAudioToISD1820();
  
  /* ======================================================= */

  /* USER CODE END 2 */
```

## 🔨 Bước 3: Build và Flash

1. **Build project**: Project → Build All
2. **Flash vào board**: Run → Debug/Run
3. **Chờ ~10 giây**

**Những gì sẽ xảy ra:**
1. Đợi 1 giây
2. Bắt đầu ghi (REC active)
3. Phát audio từ `audio_data.h` qua PE8 (~4 giây)
4. Dừng ghi
5. **Test phát lại** - bạn sẽ nghe audio từ loa

## ✅ Bước 4: Kiểm tra kết quả

**Nếu thành công:**
- Nghe thấy audio phát từ loa ISD1820
- Chất lượng âm thanh có thể hơi méo (do 1-bit DAC)

**Nếu không nghe thấy gì:**
- Kiểm tra lại kết nối PE8 → resistor → MIC
- Kiểm tra điện trở (nên dùng 1kΩ)
- Thử tăng/giảm giá trị điện trở (470Ω - 2.2kΩ)

## 🧹 Bước 5: Dọn dẹp (SAU KHI GHI XONG)

1. **Ngắt kết nối** PE8 khỏi MIC của ISD1820
2. **Comment code** trong main.c:

```c
  /* USER CODE BEGIN 2 */
  
  // Snake_BurnAudioToISD1820();  // ← COMMENTED OUT
  
  /* USER CODE END 2 */
```

3. **Build lại** và **flash**
4. **Giờ chơi game** - khi thua sẽ nghe audio!

## 🎯 Bước 6: Sử dụng bình thường

Sau khi hoàn tất các bước trên:

**Code trong `Snake_PlayMusic()` sẽ chạy:**
```c
void Snake_PlayMusic(void)
{
  // Trigger ISD1820 to play recorded audio
  HAL_GPIO_WritePin(ISD1820_PLAY_GPIO_Port, ISD1820_PLAY_Pin, GPIO_PIN_RESET);
  HAL_Delay(100);
  HAL_GPIO_WritePin(ISD1820_PLAY_GPIO_Port, ISD1820_PLAY_Pin, GPIO_PIN_SET);
}
```

**Mỗi khi game over → ISD1820 tự động phát audio đã ghi!**

## 🔧 Troubleshooting

### Vấn đề: Không nghe thấy âm thanh khi ghi

**Nguyên nhân có thể:**
1. Điện trở quá lớn → tín hiệu yếu
2. Điện trở quá nhỏ → tín hiệu quá mạnh, méo
3. Kết nối lỏng
4. MIC input sai pin

**Giải pháp:**
- Thử điện trở khác: 470Ω, 1kΩ, 2.2kΩ
- Kiểm tra chắc chắn kết nối với MIC pin đúng
- Dùng multimeter đo điện áp tại MIC (khi phát audio)

### Vấn đề: Âm thanh bị méo

**Nguyên nhân:**
- Audio từ `audio_data.h` là 1-bit DAC (chất lượng thấp)
- Tín hiệu quá mạnh cho MIC input

**Giải pháp:**
- Tăng điện trở lên 2.2kΩ hoặc 4.7kΩ
- Hoặc dùng phương pháp ghi từ PC/phone (chất lượng tốt hơn)

### Phương pháp thay thế: Ghi từ PC/Phone

Nếu muốn chất lượng cao hơn:

1. **Chuẩn bị file audio** trên PC (file .wav từ audio_data.h)
2. **Kết nối:** PC headphone jack → Aux cable → ISD1820 MIC
3. **Code đơn giản:**

```c
// Trong main():
HAL_Delay(3000);
Snake_StartRecording();
HAL_Delay(5000);  // PHÁT AUDIO TỪ PC NGAY!
Snake_StopRecording();
```

4. **Chất lượng sẽ tốt hơn nhiều**

## 📝 Tóm tắt quy trình

```
1. Kết nối PE8 → 1kΩ → MIC
           ↓
2. Thêm Snake_BurnAudioToISD1820() vào main()
           ↓
3. Build + Flash
           ↓
4. Chờ ~10s → Nghe test audio
           ↓
5. Ngắt PE8 khỏi MIC
           ↓
6. Comment code burn
           ↓
7. Build + Flash lại
           ↓
8. Chơi game → Nghe nhạc khi thua! 🎵
```

## ⚡ Lưu ý quan trọng

- ⚠️ **Chỉ chạy burn code MỘT LẦN**
- ⚠️ **Nhớ comment lại** sau khi ghi xong
- ⚠️ **Ngắt PE8 khỏi MIC** trước khi dùng bình thường
- ✅ Audio được lưu **vĩnh viễn** trong ISD1820
- ✅ Có thể ghi đè bằng cách chạy lại

---

**Ngày tạo:** 2026-01-30  
**Version:** 1.0
