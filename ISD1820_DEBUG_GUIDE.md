# 🔧 ISD1820 Audio Module Debug Guide

## ❌ Vấn đề: Không phát nhạc khi Game Over

### 📋 Checklist kiểm tra:

#### 1. **Kiểm tra kết nối phần cứng**
```
ISD1820 Pin  →  STM32F429 Pin
---------------------------------
PLAY-L/PLAY-E →  PE9 (GPIOE Pin 9)
REC           →  PE10 (GPIOE Pin 10)
VCC           →  3.3V hoặc 5V
GND           →  GND
SP+/SP-       →  Loa (hoặc dùng loa onboard)
```

**Lưu ý quan trọng:**
- ✅ Kiểm tra dây nối chắc chắn
- ✅ Module ISD1820 được cấp nguồn đúng (3.3V hoặc 5V)
- ✅ Loa được kết nối tốt (hoặc dùng jack 3.5mm/loa onboard module)

#### 2. **Đã ghi âm vào ISD1820 chưa?**

⚠️ **Module ISD1820 CẦN phải có audio đã được ghi sẵn** trước khi PLAY!

**Cách ghi âm:**

**Phương pháp 1: Dùng nút REC trên module (khuyến nghị)**
1. Nhấn và giữ nút **REC-M** trên module ISD1820
2. Nói hoặc phát nhạc vào micro (trong khoảng 10s)
3. Nhả nút REC-M
4. Nhấn nút **PLAY-E** trên module để test

**Phương pháp 2: Dùng code STM32**
```c
// Thêm vào main() hoặc gọi từ button
Snake_TestISD1820Record();  // Ghi 5 giây
HAL_Delay(1000);            // Đợi 1 giây
Snake_TestISD1820Play();    // Test phát lại
```

#### 3. **Test phần cứng độc lập**

Thêm code test vào `main()` trong [main.c](Snake/Core/Src/main.c):

```c
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  
  // TEST CODE - Uncomment để test
  // HAL_Delay(3000);  // Đợi 3 giây
  // Snake_TestISD1820Play();  // Test play ngay khi khởi động
  
  /* Infinite loop */
  for (;;)
  {
    // ... existing code ...
```

#### 4. **Kiểm tra logic trigger**

ISD1820 có 2 loại trigger:
- **PLAY-L**: Level trigger (giữ LOW trong khoảng thời gian)
- **PLAY-E**: Edge trigger (chuyển từ HIGH→LOW)

**Code hiện tại dùng:** Edge trigger (50ms pulse)

Nếu không hoạt động, thử đổi sang pulse ngắn hơn hoặc dài hơn:

```c
// Trong Snake_PlayMusic() - thử các giá trị khác nhau:
#define ISD1820_PLAY_PULSE_MS 50  // Hiện tại
// Thử: 20, 30, 100, 150, 200
```

#### 5. **Kiểm tra pull-up/pull-down**

Một số module ISD1820 cần pull-up resistor bên ngoài:
- Thêm điện trở 10kΩ từ PLAY-L/PLAY-E lên VCC
- Thêm điện trở 10kΩ từ REC lên VCC

#### 6. **Debug bằng LED hoặc oscilloscope**

**Test GPIO hoạt động:**
```c
// Thêm vào Snake_PlayMusic() để test
HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_SET);  // LED xanh on
HAL_GPIO_WritePin(ISD1820_PLAY_GPIO_Port, ISD1820_PLAY_Pin, GPIO_PIN_RESET);
HAL_Delay(50);
HAL_GPIO_WritePin(ISD1820_PLAY_GPIO_Port, ISD1820_PLAY_Pin, GPIO_PIN_SET);
HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_RESET);  // LED xanh off
```

Nếu LED nhấp nháy → GPIO hoạt động OK → Vấn đề ở ISD1820 hoặc kết nối

#### 7. **Thử logic đảo ngược**

Một số module có thể cần logic ngược:

```c
void Snake_PlayMusic(void)
{
  // Thử logic ngược (HIGH pulse)
  HAL_GPIO_WritePin(ISD1820_PLAY_GPIO_Port, ISD1820_PLAY_Pin, GPIO_PIN_SET);
  HAL_Delay(50);
  HAL_GPIO_WritePin(ISD1820_PLAY_GPIO_Port, ISD1820_PLAY_Pin, GPIO_PIN_RESET);
}
```

### 🔍 Nguyên nhân thường gặp:

1. ❌ **Chưa ghi audio vào module** (phổ biến nhất!)
2. ❌ Dây nối lỏng hoặc sai pin
3. ❌ Module không được cấp nguồn
4. ❌ Loa không kết nối hoặc bị hỏng
5. ❌ Module ISD1820 bị lỗi phần cứng
6. ❌ Pulse width không phù hợp với module cụ thể

### ✅ Cách test từng bước:

1. **Test module độc lập**: Nhấn nút PLAY-E trên module
   - Nếu phát → Module OK, lỗi ở code/kết nối
   - Nếu không phát → Ghi audio trước hoặc module lỗi

2. **Test GPIO**: Dùng LED hoặc multimeter đo PE9
   - Phải thấy pulse khi game over

3. **Test code**: Gọi `Snake_TestISD1820Play()` trong main()
   - Nếu phát → Logic game có vấn đề
   - Nếu không phát → Vấn đề ở driver/hardware

4. **Test timing**: Thử các giá trị pulse khác nhau (20-200ms)

### 📝 Log debug:

Để debug, có thể thêm printf/UART log:

```c
void Snake_PlayMusic(void)
{
  printf("Playing ISD1820 music...\n");  // Cần setup UART
  HAL_GPIO_WritePin(ISD1820_PLAY_GPIO_Port, ISD1820_PLAY_Pin, GPIO_PIN_RESET);
  musicPlayEndTick = HAL_GetTick() + ISD1820_PLAY_PULSE_MS;
}
```

### 🎯 Giải pháp khuyến nghị:

1. ✅ **Ghi audio trước** bằng nút REC-M trên module
2. ✅ **Test bằng nút PLAY-E** trên module trước
3. ✅ **Gọi `Snake_TestISD1820Play()`** trong code
4. ✅ **Kiểm tra kết nối** với multimeter
5. ✅ **Thử thay đổi pulse width** nếu cần

---

**Cập nhật:** 2026-01-30
