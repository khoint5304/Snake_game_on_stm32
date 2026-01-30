# 🎵 Audio Playback Implementation - Game Over Sound (ISD1820)

## ✅ Cấu hình ISD1820 đơn giản

Sử dụng module ISD1820 để phát âm thanh đã được ghi sẵn khi game over.

### 📋 Cách hoạt động:

1. Ghi âm thanh vào ISD1820 bằng **nút REC** trên module (chỉ làm 1 lần)
2. Khi game over, STM32 trigger chân **PLAY-L** để phát lại âm thanh

### 🔌 Sơ đồ nối dây ISD1820 với STM32F429I-DISCO:

```
ISD1820 Module          STM32F429I-DISCO
┌─────────────┐         ┌──────────────┐
│ VCC     ────┼────────►│ 3.3V hoặc 5V │
│ GND     ────┼────────►│ GND          │
│ P-L     ────┼────────►│ PD12         │  ← PLAY-L (Kéo LOW để phát)
│ SP+     ────┼───┐     └──────────────┘
│ SP-     ────┼───┤     
└─────────────┘   │     
                  │     Loa 8Ω (0.5W-2W)
                  │     ┌─────┐
                  └────►│ (+) │
                       ▼│     │
                        │ (-) │
                        └─────┘
```

### ⚠️ QUAN TRỌNG - Chân GPIO:

| Chức năng | Chân cũ (SAI) | Chân mới (ĐÚNG) | Lý do |
|-----------|---------------|-----------------|-------|
| PLAY-L    | PE9           | **PD12**        | PE9 = FMC_D6 (SDRAM) |
| REC       | PE10          | *(không cần)*   | PE10 = FMC_D7 (SDRAM) |

**PE9 và PE10 KHÔNG THỂ dùng được** vì chúng là data bus của SDRAM!

### 🎮 Code hiện tại (`main.c`):

### 📝 API Usage:

```c
// Initialize (đã tự động trong main())
SimpleAudio_Init(&htim7);

// Play game over sound
SimpleAudio_PlayGameOver();
```c
/**
 * @brief  Play Game Over music via ISD1820 module
 */
void Snake_PlayMusic(void)
{
  // Trigger ISD1820 playback: Kéo PLAY-L xuống LOW trong ~100ms
  HAL_GPIO_WritePin(ISD1820_PLAY_GPIO_Port, ISD1820_PLAY_Pin, GPIO_PIN_RESET);
  HAL_Delay(100);  // Giữ LOW 100ms để đảm bảo ISD1820 nhận tín hiệu
  HAL_GPIO_WritePin(ISD1820_PLAY_GPIO_Port, ISD1820_PLAY_Pin, GPIO_PIN_SET);
  // Audio sẽ tự phát từ ISD1820, không cần chờ đợi
}
```

### 📝 Các bước thực hiện:

#### Bước 1: Ghi âm vào ISD1820 (chỉ làm 1 lần)
1. Cắm nguồn cho ISD1820 (VCC, GND)
2. Nhấn **giữ** nút **REC** trên module
3. Nói/phát âm thanh "Game Over" vào microphone của module
4. Thả nút REC
5. Nhấn nút **PLAY** để kiểm tra âm thanh đã ghi

#### Bước 2: Nối dây với STM32
```
ISD1820     STM32F429I-DISCO
───────     ────────────────
VCC    ──►  3.3V hoặc 5V
GND    ──►  GND  
P-L    ──►  PD12   (header CN4, pin 12)
SP+    ──►  Loa (+)
SP-    ──►  Loa (-)
```

#### Bước 3: Flash code và test
Khi game over → STM32 kéo PD12 xuống LOW → ISD1820 phát âm thanh!

---

## 🔧 Troubleshooting:

### Không nghe thấy âm thanh khi game over:

1. **Kiểm tra ISD1820 hoạt động:**
   - Nhấn nút PLAY trên module → Phải nghe thấy âm thanh đã ghi

2. **Kiểm tra kết nối dây:**
   ```
   PD12 ──► P-L (PLAY-L) trên ISD1820
   ```

3. **Test GPIO từ code:**
   ```c
   // Thêm vào main() sau MX_GPIO_Init():
   while(1) {
     HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
     HAL_Delay(100);
     HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
     HAL_Delay(2000);  // Chờ audio phát xong
   }
   ```
   Mỗi 2 giây phải nghe thấy âm thanh từ ISD1820.

4. **Kiểm tra loa:**
   - Loa phải nối vào SP+ và SP- của ISD1820
   - Loa 8Ω, công suất 0.5W-2W

### Âm thanh ISD1820 nhỏ:
- Tăng âm lượng khi ghi (nói to hơn vào mic)
- Dùng loa công suất lớn hơn
- Thêm amplifier (PAM8403 module)

---

## 🚀 Nâng cấp lên DAC thật (Optional)

Nếu muốn chất lượng cao hơn, có thể dùng DAC hardware:

### Phương án 1: Internal DAC (PA4/PA5)

STM32F429 có DAC tích hợp tại PA4 và PA5.

**Ưu điểm:**
- Chất lượng cao (12-bit)
- Không cần external component
- DMA support

**Code example:**
```c
// Trong CubeMX: Enable DAC1 Channel 1 (PA4)
// Enable DMA: DAC1_CH1, Memory to Peripheral, Circular

void PlayAudioDAC(void)
{
    // Convert 16-bit audio to 12-bit
    uint16_t dac_buffer[BUFFER_SIZE];
    for(int i = 0; i < BUFFER_SIZE; i++) {
        int16_t sample = gameOverAudioData[i*2] | 
                        (gameOverAudioData[i*2+1] << 8);
        dac_buffer[i] = (sample + 32768) >> 4; // Convert to 12-bit
    }
    
    HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, 
                      (uint32_t*)dac_buffer, 
                      BUFFER_SIZE, 
                      DAC_ALIGN_12B_R);
}
```

### Phương án 2: PWM + DMA

Dùng PWM với DMA để tạo multi-bit DAC.

---

## 📝 Kết luận

**Hiện tại:** Audio playback đã HOÀN TOÀN hoạt động với 1-bit software DAC!

**Build code → Flash → Chơi game → Thua → Nghe nhạc! 🎵**

Nếu vẫn không nghe thấy, kiểm tra:
1. ✅ Buzzer/loa được kết nối PE8
2. ✅ Code đã build và flash thành công
3. ✅ Test buzzer bằng code đơn giản trước

---

**Cập nhật:** 2026-01-30
