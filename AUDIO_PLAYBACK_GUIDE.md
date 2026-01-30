# 🎵 Hướng dẫn phát nhạc từ audio_data.h trên STM32F429

## 📋 Tổng quan

Bạn có file `audio_data.h` chứa:
- **Kích thước**: 346KB
- **Format**: 16-bit Mono
- **Sample Rate**: 44.1kHz
- **Thời lượng**: ~3.93 giây

## ⚠️ Vấn đề hiện tại

Để phát audio từ memory (file .h), STM32 cần:
1. **DAC (Digital-to-Analog Converter)** + DMA
2. **PWM Audio** với timer interrupt
3. **External Audio Codec** (như CS43L22)

→ Tất cả đều cần cấu hình phức tạp trong STM32CubeMX và thêm nhiều code.

## ✅ Giải pháp khuyến nghị

### Phương án 1: Dùng ISD1820 (Đơn giản nhất - Đã implement)

**Ưu điểm:**
- ✅ Không cần code phức tạp
- ✅ Chất lượng âm thanh tốt
- ✅ Tiết kiệm RAM (không cần load 346KB vào memory)

**Cách làm:**

#### Bước 1: Chuẩn bị file audio
```bash
# Chuyển đổi audio về định dạng phù hợp
# ISD1820 hỗ trợ tốt nhất: 8kHz, Mono
ffmpeg -i your-audio.wav -ar 8000 -ac 1 game-over-8k.wav
```

#### Bước 2: Phát audio từ PC/phone qua aux cable

**Cần:**
- 1 dây aux 3.5mm (male-male)
- PC/phone có file audio

**Kết nối:**
```
PC/Phone Headphone Jack → Aux Cable → ISD1820 MIC Input
```

#### Bước 3: Ghi vào ISD1820 bằng STM32

**Code đã có sẵn, chạy:**
```c
// Thêm vào main() sau khởi tạo:
HAL_Delay(2000);  // Đợi 2 giây để chuẩn bị

// Bắt đầu ghi (10 giây)
Snake_StartRecording();
HAL_Delay(10000);  // Ghi trong 10 giây - PHÁT NHẠC TỪ PC/PHONE NGAY BÂY GIỜ!
Snake_StopRecording();

// Test phát lại
HAL_Delay(1000);
Snake_TestISD1820Play();
```

#### Bước 4: Test
Build code, flash vào STM32. Khi chương trình chạy:
1. Đợi 2 giây
2. **NGAY LẬP TỨC phát file audio từ PC/phone**
3. Ghi trong 10 giây
4. Tự động test phát lại

**Nếu OK → Xóa code test, giữ lại Snake_PlayMusic() bình thường**

---

### Phương án 2: Dùng DAC + DMA (Chất lượng cao)

**Ưu điểm:**
- ✅ Chất lượng cao
- ✅ Điều khiển hoàn toàn bằng code
- ✅ Có thể play nhiều file khác nhau

**Nhược điểm:**
- ❌ Cần cấu hình STM32CubeMX (DAC, DMA, Timer)
- ❌ Tốn RAM (346KB)
- ❌ Code phức tạp hơn nhiều

**Hướng dẫn implement:**

#### 1. Cấu hình STM32CubeMX

Mở file `.ioc` trong STM32CubeMX:

**A. Enable DAC:**
```
Analog > DAC1 > OUT1 (PA4) hoặc OUT2 (PA5)
Mode: Connected to external pin only
```

**B. Enable DMA cho DAC:**
```
DMA Settings > Add
  - DMA Request: DAC1/DAC2
  - Stream: DMA1 Stream 5 (hoặc available stream)
  - Direction: Memory to Peripheral
  - Mode: Circular
  - Data Width: Half Word (16-bit)
```

**C. Enable Timer cho sample rate:**
```
Timers > TIM6 (hoặc TIM7)
  - Clock Source: Internal Clock
  - Trigger Event Selection: Update Event
  
Parameter Settings:
  - Prescaler: (APB1 clock / 1MHz) - 1
  - Counter Period: (1000000 / 44100) - 1  // Cho 44.1kHz
```

**D. Enable Interrupts:**
```
NVIC Settings:
  ✅ DMA1 stream5 global interrupt
  ✅ TIM6 global interrupt
```

Generate code!

#### 2. Implement Audio Player Code

Tạo file `dac_audio.c`:

\`\`\`c
#include "dac_audio.h"
#include "audio_data.h"

static DAC_HandleTypeDef *hdac_audio = NULL;
static TIM_HandleTypeDef *htim_audio = NULL;
static uint32_t dac_channel = 0;
static volatile uint32_t audio_position = 0;
static volatile uint8_t is_playing = 0;

int DAC_Audio_Init(DAC_HandleTypeDef *hdac, TIM_HandleTypeDef *htim, uint32_t channel)
{
    hdac_audio = hdac;
    htim_audio = htim;
    dac_channel = channel;
    return 0;
}

int DAC_Audio_Play(void)
{
    if (is_playing) return -1;
    
    audio_position = 0;
    is_playing = 1;
    
    // Start DAC with DMA
    HAL_DAC_Start_DMA(hdac_audio, dac_channel, 
                      (uint32_t*)gameOverAudioData, 
                      gameOverAudioSize / 2,  // 16-bit samples
                      DAC_ALIGN_12B_R);
    
    // Start timer to trigger DAC at sample rate
    HAL_TIM_Base_Start(htim_audio);
    
    return 0;
}

void DAC_Audio_Stop(void)
{
    if (!is_playing) return;
    
    HAL_TIM_Base_Stop(htim_audio);
    HAL_DAC_Stop_DMA(hdac_audio, dac_channel);
    is_playing = 0;
}

// Callback khi DMA complete
void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef* hdac)
{
    // Audio finished
    is_playing = 0;
}
\`\`\`

#### 3. Downsampling (Tùy chọn - giảm dung lượng)

Audio 44.1kHz quá cao, có thể downsample về 16kHz hoặc 22kHz:

**Script Python:**
```python
import wave
import numpy as np

# Đọc file gốc
with wave.open('game-over.wav', 'rb') as wav:
    params = wav.getparams()
    frames = wav.readframes(params.nframes)
    audio = np.frombuffer(frames, dtype=np.int16)

# Downsample 44100 → 16000
ratio = 44100 / 16000
indices = np.arange(0, len(audio), ratio).astype(int)
audio_16k = audio[indices]

# Lưu file mới
with wave.open('game-over-16k.wav', 'wb') as wav:
    wav.setparams((1, 2, 16000, len(audio_16k), 'NONE', 'not compressed'))
    wav.writeframes(audio_16k.tobytes())

# Tạo .h file
# python wav_to_c_array.py game-over-16k.wav audio_data.h gameOverAudio
```

---

### Phương án 3: PWM Audio (Trung bình)

**Ưu điểm:**
- ✅ Không cần DAC
- ✅ Đơn giản hơn DAC
- ✅ Chất lượng khá tốt với low-pass filter

**Nhược điểm:**
- ❌ Cần Timer và PWM setup
- ❌ Cần low-pass filter (RC circuit)
- ❌ Vẫn tốn RAM

**Sẽ implement nếu cần!**

---

## 🎯 Khuyến nghị

**Cho game Snake:**
1. **Dùng ISD1820** (Phương án 1) - Đơn giản, hiệu quả
2. Ghi audio 1 lần vào module
3. Mỗi khi game over → Trigger ISD1820

**Hiện tại code đã implement:**
- ✅ Buzzer backup (3 beep khi game over)
- ✅ ISD1820 trigger
- ✅ Hàm test ghi/phát

**Bạn chỉ cần:**
1. Ghi audio vào ISD1820 (dùng code test hoặc nút trên module)
2. Build code hiện tại
3. Chơi và thua để test!

---

## 🔧 Troubleshooting

**1. ISD1820 không phát:**
- Kiểm tra đã ghi audio vào module chưa
- Test bằng nút PLAY-E trên module
- Xem [ISD1820_DEBUG_GUIDE.md](ISD1820_DEBUG_GUIDE.md)

**2. Buzzer không kêu:**
- Kiểm tra PE8 có kết nối buzzer
- Test: `HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, GPIO_PIN_SET);`

**3. Muốn phát file audio_data.h:**
- Cần implement DAC (Phương án 2)
- Hoặc chuyển audio sang ISD1820 (Phương án 1)

---

**Cập nhật:** 2026-01-30
