# Snake Game on STM32F429I-DISCO

## 1. Giới thiệu

### 1.1. Tổng Quan Dự Án

Dự án trò chơi Snake được triển khai trên STM32F429I-DISCO board sử dụng TouchGFX framework để tạo giao diện đồ họa. Đây là một trò chơi rắn săn mồi cổ điển với các tính năng nâng cao như nhiều cấp độ khó, BigFood có thời gian giới hạn, hiệu ứng âm thanh, và lưu trữ điểm cao vào Flash memory.

### 1.2. Video

Please insert link here

### 1.3. Phân công công việc

| Thành viên        | Mã số sinh viên  | Công việc                         |
|-------------------|------------------|-----------------------------------|
| Nguyễn Thái Khôi  | 20224868         | Lắp đặt phần cứng, xử lý đồ họa   |
| Đào Phúc Long     | 20220034         | Xử lý logic game                  |
| Vũ Tùng Lâm       | 20225140         | Xử lý âm thanh                    |


## 2. Thiết Kế Phần Cứng (Hardware Design)

### 2.1. Thông Số Kỹ Thuật Phần Cứng

#### Bộ Xử Lý và Bộ Nhớ
- **MCU**: STM32F429ZIT6 (ARM Cortex-M4, 180MHz)
- **Flash Memory**: 2MB Internal Flash
  - Sector 0-22: Code + Constants (1920 KB)
  - Sector 23: Persistent Data (128 KB) - dành cho High Score
- **Internal RAM**: 192 KB
- **External SDRAM**: 8MB (IS42S16400J) qua FMC interface

#### Display
- **Type**: ILI9341 LCD
- **Resolution**: 240x320 pixels
- **Color Format**: RGB565 (16-bit color)
- **Interface**: LTDC (LCD-TFT Display Controller)

#### Ngoại Vi Chính
- **LTDC** (LCD-TFT Display Controller) - để hiển thị
- **DMA2D** - hardware graphics acceleration
- **FMC** - SDRAM interface
- **GPIO** - buttons và audio output

---

### 2.2. GPIO Mapping

| Pin   | Function        | Type    | Config          | Description                    |
|-------|----------------|---------|-----------------|--------------------------------|
| PD4   | BTN_UP         | Input   | Pull-up         | UP button (active LOW)          |
| PD5   | BTN_DOWN       | Input   | Pull-up         | DOWN button (active LOW)        |
| PD6   | BTN_LEFT       | Input   | Pull-up         | LEFT button (active LOW)        |
| PD7   | BTN_RIGHT      | Input   | Pull-up         | RIGHT button (active LOW)       |
| PG13  | BUZZER         | Output  | Push-pull       | Buzzer control (active HIGH)    |
| PD12  | ISD1820_PLAY   | Output  | Push-pull       | ISD1820 PLAY-L (active LOW pulse)|
| PC2   | LCD_CS         | Output  | Push-pull       | LCD chip select                 |
| PD13  | LCD_WRX        | Output  | Push-pull       | LCD write/read select           |
| PE2-5 | Debug GPIO     | Output  | Push-pull       | Performance testing pins        |

---

### 2.3. Sơ Đồ Dây Nối (Wiring Diagrams)

#### 2.3.1. Nút Bấm
```
STM32 PD4-7 (with internal pull-up) ── Button ── GND
```

**Chi tiết**:
- Tất cả 4 nút (UP, DOWN, LEFT, RIGHT) được cấu hình ở chế độ Input với Pull-up
- Active LOW: Khi nhấn, GPIO đọc LOW (0), khi không nhấn đọc HIGH (1)
- Debouncing: Polling mỗi 20ms trong defaultTask

#### 2.3.2. Buzzer
```
STM32 PG13 ── Buzzer (+) ── Buzzer (-) ── GND
```

**Chi tiết**:
- PG13 là output Push-pull
- Buzzer hoạt động ở chế độ Active HIGH (PG13 = HIGH → buzzer phát âm)
- Được dùng cho:
  - Eat food: 100ms beep
  - Eat BigFood: 300ms beep
  - Game Over: 1000ms beep

#### 2.3.3. ISD1820 Audio Module
```
ISD1820 Module     STM32F429I-DISCO
VCC  ───────────── 3.3V or 5V
GND  ───────────── GND
PLAY-L ─────────── PD12
SP+/SP- ────────── Speaker
REC  ───────────── Manual record button on module
MIC  ───────────── Microphone input (for recording only)
```

**Chi tiết**:
- PD12 là output Push-pull
- PLAY-L: Active LOW pulse trigger (100ms LOW pulse để phát)
- Dùng để phát âm thanh game over (pre-recorded audio)
- REC button trên module để recording thủ công
- MIC input cho phép recording từ microphone

---

### 2.4. Bản Đồ Bộ Nhớ (Memory Map)

#### 2.4.1. Flash Memory (2MB = 0x200000 bytes)
```
0x08000000 ─────────┐
            ...     │ Sector 0-22: Code + Constants (1920 KB)
0x081DFFFF ─────────┤
0x081E0000 ─────────┐
            ...     │ Sector 23: Persistent Data - High Score (128 KB)
0x081FFFFF ─────────┘
```

**Sector 23 Layout** (0x081E0000 - 0x081FFFFF):
```c
typedef struct {
    uint32_t magic;       // 0x534E414B ("SNAK")
    uint16_t highScore;   // Điểm cao nhất (2 bytes)
    uint16_t reserved1;   // Dự trữ (2 bytes)
    uint32_t playCount;   // Số lần chơi (4 bytes)
    uint32_t checksum;    // CRC32 checksum (4 bytes)
    // ... padding to fill 128KB
} FlashStorageData_t;
```

**Features**:
- Magic number: 0x534E414B để verify data hợp lệ
- Checksum: Để detect data corruption
- Write optimization: Chỉ ghi khi có high score mới
- Erase cycles: ~10K-100K cycles, tiết kiệm bằng cách giảm writes

#### 2.4.2. SDRAM Memory (8MB = 0x800000 bytes)
```
0xD0000000 ─────────┐
            ...     │ TouchGFX Frame Buffer (~150 KB)
            ...     │ TouchGFX Assets Cache (sprites, fonts, etc.)
            ...     │ FreeRTOS Heap
            ...     │ Game state, snake segments, etc.
0xD07FFFFF ─────────┘
```

**Allocation**:
- Frame Buffer: 240 × 320 × 2 bytes (RGB565) = 150 KB
- Assets Cache: ~500 KB (bitmaps, fonts)
- FreeRTOS Heap: ~5 MB (remaining)
- Game Memory: < 50 KB (snake data, game state)

#### 2.4.3. Internal RAM (192 KB = 0x30000 bytes)
```
0x20000000 ─────────┐
            ...     │ Stack, Global variables
            ...     │ FreeRTOS Task stacks:
            ...     │   - defaultTask: 512 bytes
            ...     │   - GUI_Task: 32 KB
            ...     │ Heap (small objects)
0x2002FFFF ─────────┘
```

---

### 2.5. Cấu Hình Xung Nhịp (Clock Configuration)

#### 2.5.1. Sơ Đồ Clock Tree
```
HSE (8 MHz) → PLL × 45 → 360 MHz (with Over-Drive mode)
                      ↓ ÷2
                SYSCLK = 180 MHz
                      ↓
        ┌─────────────┼─────────────┐
        ↓             ↓             ↓
    AHB (÷1)     APB1 (÷4)      APB2 (÷2)
    180 MHz      45 MHz         90 MHz
        ↓             ↓             ↓
   LTDC         I2C3           TIM7, SPI5
   DMA2D        FreeRTOS        GPIO
   FMC SDRAM
```

#### 2.5.2. Chi Tiết
- **HSE** (High Speed External): 8 MHz (từ crystal oscillator trên board)
- **PLL** (Phase Locked Loop): Nhân 45 lần → 360 MHz
- **Over-Drive Mode**: Cho phép SYSCLK = 180 MHz (vượt 168 MHz normal limit)
- **SYSCLK**: 180 MHz (CPU clock)
- **AHB**: 180 MHz (không chia) - cho LTDC, DMA2D, FMC SDRAM
- **APB1**: 45 MHz (÷4) - cho I2C3, FreeRTOS timer
- **APB2**: 90 MHz (÷2) - cho TIM7, SPI5, GPIO fast speed

---

### 2.6. Cấu Hình Ngoại Vi (Peripheral Configuration)

#### 2.6.1. LTDC (LCD-TFT Display Controller)

**Mục đích**: Điều khiển LCD display ILI9341

**Cấu hình**:
- **Timing**: 240x320 @ 60 Hz
- **Pixel Clock**: ~9.6 MHz
- **Color Format**: RGB565 (16-bit, 5-6-5 bits per RGB)
- **Layer 0**: 
  - Pixel Format: RGB565
  - Frame Buffer Address: SDRAM @ 0xD0000000
  - Size: 240 × 320 pixels
  - Single buffering (không double-buffer do SDRAM limited)

**Refresh Cycle**:
```
Horizontal Timing:
  Display Width: 240 pixels
  Front Porch: 16 pixels
  Sync Width: 10 pixels  
  Back Porch: 40 pixels
  Total: ~306 pixels = 31.875 μs @ 9.6 MHz

Vertical Timing:
  Display Height: 320 pixels
  Front Porch: 4 lines
  Sync Width: 2 lines
  Back Porch: 2 lines
  Total: ~328 lines = 10.44 ms
  Frame Rate: 95.8 Hz (throttled to 60 Hz by software)
```

#### 2.6.2. DMA2D (Graphics Accelerator)

**Mục đích**: Tăng tốc độ xử lý đồ họa (blitting, fills)

**Cấu hình**:
- **Mode**: Memory-to-Memory with PFC (Pixel Format Conversion)
- **Usage**: 
  - Bitmap blitting (drawing snake, food, UI)
  - Rectangle fills (background)
  - Color conversion (nếu cần)
- **Performance**: 
  - Tốc độ: ~2-4 pixels per clock cycle
  - Giảm CPU workload từ 80% xuống ~40-50%

#### 2.6.3. FMC SDRAM Interface

**Mục đích**: Giao tiếp với external 8MB SDRAM

**Cấu hình**:
- **Bank**: Bank 2 (Chip Select 2)
- **Device**: IS42S16400J (16Mb SDRAM, 1M × 16 bits)
- **Data Width**: 16-bit (2 bytes)
- **Address Lines**: A0-A12 (4096 rows)
- **CAS Latency**: 3 cycles
- **Refresh Cycle**: 
  - Refresh Period: 64ms (typical for SDRAM)
  - Number of Rows: 4096
  - Refresh Interval: 64ms / 4096 rows = 15.625 μs per row
  - Refresh Count: Configured trong STM32CubeMX

**Timing Parameters**:
```
tAC (Access Time): 15 ns
tOH (Output Hold): 2.5 ns
Setup Time: 2 ns
Hold Time: 1 ns
CAS Latency: 3 CLK @ 90 MHz (divided from 180 MHz AHB)
```

**Memory Map**:
- Start Address: 0xD0000000
- End Address: 0xD07FFFFF (8MB)
- TouchGFX Frame Buffer: 0xD0000000 (150 KB)

---

### 2.7. Các Thông Số Tính Toán

#### 2.7.1. Game Rendering
```
Frame Buffer Size: 240 × 320 × 2 bytes = 153.6 KB
Refresh Rate: 60 FPS
Frame Time: 16.67 ms
Pixel Throughput: 240 × 320 × 60 = 4.608 Mpixels/s

LTDC Pixel Clock: 9.6 MHz
DMA2D Throughput: ~2-4 pixels per clock (with PFC)
Effective Throughput: 19.2 - 38.4 Mpixels/s (plenty for rendering)
```

#### 2.7.2. Memory Bandwidth
```
SDRAM Clock: 90 MHz (AHB ÷2)
Data Width: 16-bit (2 bytes)
Max Bandwidth: 90 MHz × 2 bytes = 180 MB/s

Typical Usage:
- Frame Buffer Read: 240 × 320 × 2 bytes × 60 fps = 9.2 MB/s
- Texture/Asset Reads: ~50 MB/s
- Headroom: ~121 MB/s (67% utilization)
```

#### 2.7.3. CPU Processing
```
SYSCLK: 180 MHz
Game Update Frequency: 60 FPS
Time per Frame: 16.67 ms

Typical CPU Cycles per Frame:
- Game Logic: 5-10% (snake movement, collision, food spawn)
- GUI Rendering: 30-40% (widget updates, bitmap drawing)
- RTOS Overhead: 5-10%
- DMA2D Hardware: 40-50% (offloads from CPU)

Total CPU Usage: 40-50% (during gameplay)
```

---

### 2.8. Power Supply & Electrical

#### 2.8.1. Power Sources
- **Main**: USB 5V (via ST-Link)
- **Board Regulators**: 
  - 3.3V (for MCU, peripherals, LCD, sensors)
  - 1.2V (core voltage, regulated internally)

#### 2.8.2. GPIO Drive Capabilities
- **Output**: 
  - Standard Output: 25 mA max per pin
  - Buzzer (PG13): ~10 mA 
  - ISD1820 (PD12): ~5 mA (open-drain trigger)
  
- **Input**:
  - Buttons (PD4-7): Internal pull-up (~100 kΩ)
  - Noise immunity: Schmitt trigger inputs

#### 2.8.3. Supply Current Estimation
```
Running (normal gameplay):
  MCU Core: ~40-50 mA
  SDRAM: ~20-30 mA
  LCD Display: ~50-80 mA
  Other peripherals: ~10-20 mA
  Total: ~150-200 mA

Sleep Mode (if implemented):
  CPU stopped: ~5-10 mA
  Peripherals off: ~30-50 mA
  Total: ~35-60 mA
```

## 3. Thiết Kế Phần Mềm (Software Design)

### 3.1. Thông Số Kỹ Thuật Phần Mềm
- **Framework**: TouchGFX 4.26.0, STM32CubeMX 6.16.0, STM32CubeF4 Firmware v1.28.3
- **RTOS**: FreeRTOS (2 tasks: defaultTask 512 bytes stack, GUI_Task 32KB stack)
- **Architecture**: Model-View-Presenter (MVP) pattern
- **Screen Resolution**: 240x320 pixels, portrait orientation
- **Color Depth**: 16-bit RGB565
- **Frame Rate**: 60 FPS

---

### 3.2. Cấu Trúc Tổng Thể (MVP Pattern)

```
┌─────────────────────────────────────────────────────┐
│                    main.c (C)                       │
│  - Hardware Initialization                          │
│  - FreeRTOS scheduler                               │
│  - GPIO Button Polling (defaultTask)                │
│  - Flash Storage Management                         │
│  - Audio Hardware Control                           │
└──────────────────┬──────────────────────────────────┘
                   │ extern "C" interface
                   ▼
┌─────────────────────────────────────────────────────┐
│          SnakeInterface.h (API declaration)         │
│  - Snake_UpdateButtonStates()                       │
│  - Snake_PlayBuzzer()                               │
│  - Snake_PlayMusic()                                │
│  - Snake_GetTickMs()                                │
│  - Snake_SaveHighScore() / Snake_LoadHighScore()    │
└──────────────────┬──────────────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────────────┐
│              Model.hpp/cpp (C++)                    │
│  1) Model                                           │
│     - Lưu trạng thái button                         │
│     - Edge detection trong tick()                   │
│     - Gọi ModelListener::buttonPressed()            │
│     - Lưu lastScore, highScore                      │
│                                                     │
│  2) SnakeGame (logic game thuần)                    │
│     - Di chuyển rắn                                 │
│     - Va chạm, food, big food                       │
│     - Score, difficulty                             │
│     - Sinh sound event                              │
│                                                     │
│  3) SnakeInterface implementation                   │
│     - Bridge từ main.c sang Model                   │
└──────────────────┬──────────────────────────────────┘
                   │ MVP: Model ↔ Presenter ↔ View
                   ▼
┌─────────────────────────────────────────────────────┐
│           Presenters (Screen1-3Presenter)           │
│  - Business logic mediation                         │
│  - Model ↔ View communication                       │
└──────────────────┬──────────────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────────────┐
│              Views (Screen1-3View)                  │
│  - UI rendering and updates                         │
│  - User interaction handling                        │
│  - Screen1: Menu + Difficulty selection             │
│  - Screen2: Game play                               │
│  - Screen3: Game Over + Scores                      │
└─────────────────────────────────────────────────────┘
```

Để đơn giản hoá project, nhóm gộp implementation của SnakeGame và SnakeInterface vào Model.cpp. Về mặt kiến trúc, đây không phải cách tổ chức tối ưu, nhưng với project này việc gộp giúp giảm số lượng file và đơn giản hóa việc debug trên hệ nhúng.

---

### 3.3. Luồng hoạt động của hệ thống

#### 3.3.1. Khởi Động Hệ Thống
```
1. main() → HAL_Init() → SystemClock_Config()
2. Peripheral Init: GPIO, LTDC, DMA2D, FMC SDRAM
3. FlashStorage_Init()
4. FreeRTOS osKernelInitialize()
5. Create Tasks:
   - defaultTask (button polling, buzzer control)
   - GUI_Task (TouchGFX rendering 60 FPS)
6. osKernelStart() → RTOS scheduler running
7. TouchGFX init → Model() constructor → loadHighScoreFromFlash()
8. Screen1View (Menu) hiển thị
```

#### 3.3.2. Game Loop (Screen2)
```
defaultTask (mỗi 20ms):
1. Đọc trạng thái GPIO của các nút điều khiển (PD4-PD7, active LOW with pull-up)
2. Debouncing check
3. Snake_UpdateButtonStates() → Model.updateButtonStates()
4. Update buzzer GPIO (PG13) nếu buzzerEndTick expired
5. Nếu cả 4 nút được giữ trong 3 giây → FlashStorage_EraseAll()

GUI_Task / Screen2View::handleTickEvent() (mỗi ~16.67ms @ 60 FPS):
1. tickCounter++
2. if (tickCounter >= getTickInterval()) {
   3. game->update():
      a. moveSnake() - di chuyển head, body theo sau
      b. Check food collision:
         - Normal food: score += difficulty points, spawnFood()
         - BigFood: score += time-based points (500 → 0)
      c. checkCollision() - tự va chạm → game over
      d. updateBigFood() - check 5-second timer expiration
   4. updateSnakeDisplay() - render snake với đúng bitmap
   5. updateFoodDisplay() - cập nhật hiển thị thức ăn
   6. updateBigFoodDisplay()
   7. updateScoreDisplay() - cập nhật hiển thị điểm số
   8. Reset tickCounter = 0
}
3. handleSoundEvent() - kích hoạt buzzer/ISD1820
4. Nếu phát hiện trạng thái Game Over → gotoScreen3() - chuyển sang Screen3 sau 1 giây.
```

#### 3.3.3. High Score Persistence
```
Game Over:
1. Screen2View phát hiện trạng thái Game Over
2. Gửi điểm số hiện tại về Model
3. Model::saveGameScore(score): Nếu score > highScore thì cập nhật highScore trong Flash.
4. Chuyển sang Screen3 hiển thị điểm ván vừa chơi, highScore.

Lần khởi động tiếp theo:
1. FlashStorage_Init() → ReadDataFromFlash()
2. Kiểm tra dữ liệu hợp lệ
    - nếu hợp lệ thì sử dụng highScore đã lưu.
    - nếu không hợp lệ thì highScore = 0 (first boot hoặc corrupted data)
```