# Snake Game on STM32F429I-DISCO

## 1. Tổng Quan Dự Án

Dự án trò chơi Snake được triển khai trên STM32F429I-DISCO board sử dụng TouchGFX framework để tạo giao diện đồ họa. Đây là một trò chơi rắn săn mồi cổ điển với các tính năng nâng cao như nhiều cấp độ khó, BigFood có thời gian giới hạn, hiệu ứng âm thanh, và lưu trữ điểm cao vào Flash memory.

### Thông Số Kỹ Thuật Phần Cứng
- **MCU**: STM32F429ZIT6 (ARM Cortex-M4, 180MHz)
- **Display**: ILI9341 LCD 240x320 pixels, RGB565 (16-bit color)
- **RAM**: 8MB External SDRAM (IS42S16400J) qua FMC interface
- **Flash**: 2MB Internal Flash (Sector 0-22 cho code, Sector 23 cho data)
- **Peripherals**: 
  - LTDC (LCD-TFT Display Controller) cho hiển thị
  - DMA2D cho hardware graphics acceleration
  - I2C3 cho touch controller (STMPE811)
  - SPI5 cho gyroscope (L3GD20)
  - FMC SDRAM interface
  - TIM7 cho audio playback timer
  - GPIO cho buttons và audio output

### Thông Số Kỹ Thuật Phần Mềm
- **Framework**: TouchGFX 4.26.0, STM32CubeMX 6.16.0, STM32CubeF4 Firmware v1.28.3
- **RTOS**: FreeRTOS (2 tasks: defaultTask 512 bytes stack, GUI_Task 32KB stack)
- **Architecture**: Model-View-Presenter (MVP) pattern
- **Screen Resolution**: 240x320 pixels, portrait orientation
- **Color Depth**: 16-bit RGB565
- **Frame Rate**: 60 FPS

## 2. Kiến Trúc Phần Mềm

### 2.1. Cấu Trúc Tổng Thể (MVP Pattern)

```
┌─────────────────────────────────────────────────────┐
│                    main.c (C)                       │
│  - Hardware Initialization                          │
│  - FreeRTOS Tasks                                   │
│  - GPIO Button Polling (defaultTask)                │
│  - Flash Storage Management                         │
│  - Audio Hardware Control                           │
└──────────────────┬──────────────────────────────────┘
                   │ extern "C" interface
                   ▼
┌─────────────────────────────────────────────────────┐
│          SnakeInterface.h/cpp (C/C++ Bridge)        │
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
│  - SnakeGame instance                               │
│  - Button states management                         │
│  - High score tracking (RAM + Flash)                │
│  - tick() - event distribution to Views             │
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

### 2.2. Module Structure

#### 2.2.1. Core Module (C)
**Vị trí**: `Snake/Core/`

##### main.c
- **Chức năng chính**:
  - Khởi tạo phần cứng: Clock (180MHz), Peripherals (GPIO, I2C, SPI, LTDC, DMA2D, FMC SDRAM)
  - Khởi tạo FreeRTOS với 2 tasks:
    - `defaultTask` (512 bytes stack): Quét nút bấm, quản lý buzzer
    - `GUI_Task` (32KB stack): TouchGFX rendering engine
  - Polling GPIO buttons mỗi 20ms (debouncing)
  - Audio hardware control (buzzer on PG13, ISD1820 on PD12)
  - Flash storage initialization

##### flash_storage.c/h
- **Chức năng**: Persistent storage cho high score
- **Storage Location**: Sector 23 (0x081E0000 - 0x081FFFFF, 128KB)
- **Data Structure**:
  ```c
  typedef struct {
      uint32_t magic;       // 0x534E414B ("SNAK")
      uint16_t highScore;   // Điểm cao nhất
      uint16_t reserved1;   // Dự trữ
      uint32_t playCount;   // Số lần chơi
      uint32_t checksum;    // CRC32-like checksum
  } FlashStorageData_t;
  ```
- **Write Cycle Optimization**: Chỉ ghi khi có high score mới để tiết kiệm flash erase cycles (~10K-100K cycles)
- **Data Integrity**: Magic number + checksum verification

##### audio_playback.c/h
- **Chức năng**: Audio playback qua ISD1820 module
- **Hardware Interface**: 
  - Buzzer (PG13): PWM-like output cho beep sounds
  - ISD1820 (PD12 - PLAY-L): Active-low pulse trigger (100ms)
- **Sound Events**:
  - `SOUND_EAT_FOOD`: 100ms beep
  - `SOUND_EAT_BIGFOOD`: 300ms beep
  - `SOUND_GAME_OVER`: 1000ms beep + ISD1820 music playback

#### 2.2.2. TouchGFX Module (C++)
**Vị trí**: `Snake/TouchGFX/gui/`

##### Model.hpp/cpp
- **Chức năng**: Central data manager trong MVP pattern
- **Responsibilities**:
  - Quản lý `SnakeGame` instance (singleton pattern)
  - Button state management với edge detection
  - High score tracking (RAM cache + Flash sync)
  - Event distribution qua `ModelListener` interface
- **Key Methods**:
  - `updateButtonStates()`: Nhận button states từ main.c
  - `tick()`: Gọi mỗi frame, detect button edge (rising edge)
  - `saveGameScore()`: Auto-save to Flash nếu là high score
  - `loadHighScoreFromFlash()`: Restore từ Flash khi khởi động

##### SnakeGame.hpp/cpp
- **Chức năng**: Core game logic engine
- **Game Constants**:
  ```cpp
  CELL_SIZE = 10 pixels         // Kích thước 1 ô lưới
  GRID_WIDTH = 24 cells         // 240px / 10px
  GRID_HEIGHT = 28 cells        // 280px / 10px
  MAX_SNAKE_LENGTH = 100        // Giới hạn độ dài rắn
  BIGFOOD_DURATION_MS = 5000    // BigFood tồn tại 5 giây
  BIGFOOD_APPEAR_AFTER = 5      // Xuất hiện sau 5 mồi thường
  BIGFOOD_MAX_SCORE = 500       // Điểm tối đa (giảm theo thời gian)
  ```

- **Difficulty Levels**:
  | Level     | Speed (cells/sec) | Points/Food | Tick Interval (frames @ 60 FPS) |
  |-----------|------------------|-------------|--------------------------------|
  | EASY      | 1                | 1           | 60 ticks (~1000ms)             |
  | NORMAL    | 3                | 3           | 20 ticks (~333ms)              |
  | HARD      | 5                | 5           | 12 ticks (~200ms)              |
  | INSANE    | 8                | 8           | 7 ticks (~117ms)               |
  | NIGHTMARE | 12               | 12          | 5 ticks (~83ms)                |

- **Key Algorithms**:

  **1. Snake Movement**:
  ```cpp
  void moveSnake() {
      // Lưu vị trí cũ của head
      Position prevPos = snake[0];
      
      // Di chuyển head theo hướng hiện tại
      switch (currentDirection) {
          case UP:    snake[0].y--; break;
          case DOWN:  snake[0].y++; break;
          case LEFT:  snake[0].x--; break;
          case RIGHT: snake[0].x++; break;
      }
      
      // Wrap-around (teleport) khi chạm tường
      if (snake[0].x < 0) snake[0].x = GRID_WIDTH - 1;
      else if (snake[0].x >= GRID_WIDTH) snake[0].x = 0;
      if (snake[0].y < 0) snake[0].y = GRID_HEIGHT - 1;
      else if (snake[0].y >= GRID_HEIGHT) snake[0].y = 0;
      
      // Di chuyển body segments theo head
      for (int i = 1; i < snakeLength; i++) {
          Position temp = snake[i];
          snake[i] = prevPos;
          prevPos = temp;
      }
  }
  ```

  **2. Collision Detection**:
  ```cpp
  bool checkCollision() {
      // Chỉ kiểm tra va chạm với chính mình (không có tường)
      for (int i = 1; i < snakeLength; i++) {
          if (snake[0] == snake[i])
              return true;  // Game over
      }
      return false;
  }
  ```

  **3. Food Spawning (LCG Random)**:
  ```cpp
  void spawnFood() {
      Position newPos;
      do {
          // Linear Congruential Generator
          randomState = randomState * 1103515245 + 12345;
          newPos.x = (randomState >> 16) % GRID_WIDTH;
          randomState = randomState * 1103515245 + 12345;
          newPos.y = (randomState >> 16) % GRID_HEIGHT;
      } while (isPositionOnSnake(newPos) || isPositionOnBigFood(newPos));
      food = newPos;
  }
  ```

  **4. BigFood Scoring (Time-based)**:
  ```cpp
  uint32_t getBigFoodScore() {
      uint32_t elapsed = Snake_GetTickMs() - bigFoodStartTime;
      if (elapsed > BIGFOOD_DURATION_MS)
          elapsed = BIGFOOD_DURATION_MS;
      // Điểm giảm tuyến tính: 500 → 0 trong 5 giây
      return (BIGFOOD_MAX_SCORE * (BIGFOOD_DURATION_MS - elapsed)) / BIGFOOD_DURATION_MS;
  }
  ```

  **5. Direction Control (Anti-180° Turn)**:
  ```cpp
  void setDirection(SnakeDirection dir) {
      // Ngăn rắn quay 180° (tự ăn chính mình)
      if ((currentDirection == UP && dir == DOWN) ||
          (currentDirection == DOWN && dir == UP) ||
          (currentDirection == LEFT && dir == RIGHT) ||
          (currentDirection == RIGHT && dir == LEFT)) {
          return;  // Bỏ qua lệnh
      }
      nextDirection = dir;  // Buffered input
  }
  ```

##### Screen Views (Screen1-3View)

**Screen1View (Menu)**:
- **Chức năng**: Main menu + difficulty selection
- **UI Elements**:
  - Difficulty display với color coding:
    - EASY: Green (0, 255, 0)
    - NORMAL: Yellow (255, 255, 0)
    - HARD: Red (255, 0, 0)
    - INSANE: Magenta (255, 0, 255)
    - NIGHTMARE: Purple (128, 0, 128)
  - "Start Game" button → Screen2
  - "Change Difficulty" button → cycle through levels
- **Logic**: `cycleDifficulty()` changes EASY→NORMAL→HARD→INSANE→NIGHTMARE→EASY

**Screen2View (Game Play)**:
- **Chức năng**: Main game screen với real-time rendering
- **UI Layout**:
  - Game area: 240x280 pixels (box1 container)
  - Score display: Top-right corner (textArea1)
  - Snake rendering: Dynamic Image array (100 segments max)
  - Food rendering: Single Image at grid position
  - BigFood rendering: 2x2 cells Image (20x20 pixels)

- **Rendering Algorithm**:
  ```cpp
  void updateSnakeDisplay() {
      // Ẩn tất cả segments hiện có
      for (int i = 0; i < currentSegmentCount; i++)
          snakeSegments[i].setVisible(false);
      
      // Render snake mới
      for (int i = 0; i < game->getSnakeLength(); i++) {
          Position pos = game->getSnakeSegment(i);
          
          // Chọn bitmap dựa trên vị trí trong snake
          uint16_t bitmapId;
          if (i == 0) {
              // Head: hướng theo currentDirection
              bitmapId = getHeadBitmapId(game->getCurrentDirection());
          } else if (i == game->getSnakeLength() - 1) {
              // Tail: hướng theo segment trước nó
              bitmapId = getTailBitmapId(game->getSegmentDirection(i));
          } else if (isTurnSegment(i, fromDir, toDir)) {
              // Turn segment: 16 combinations (4 from × 4 to directions)
              bitmapId = getTurnBitmapId(fromDir, toDir);
          } else {
              // Mid segment: straight horizontal/vertical
              bitmapId = getMidBitmapId(game->getSegmentDirection(i));
          }
          
          // Set position và hiển thị
          snakeSegments[i].setBitmap(Bitmap(bitmapId));
          snakeSegments[i].setXY(pos.x * CELL_SIZE, pos.y * CELL_SIZE);
          snakeSegments[i].setVisible(true);
      }
      
      currentSegmentCount = game->getSnakeLength();
      snakeContainer.invalidate();  // Trigger redraw
  }
  ```

- **Turn Segment Detection**:
  ```cpp
  bool isTurnSegment(uint8_t index, SnakeDirection &fromDir, SnakeDirection &toDir) {
      Position prev = game->getSnakeSegment(index - 1);  // Towards head
      Position curr = game->getSnakeSegment(index);
      Position next = game->getSnakeSegment(index + 1);  // Towards tail
      
      // Tính direction từ curr → prev (exit direction)
      int dx1 = prev.x - curr.x;
      int dy1 = prev.y - curr.y;
      // Handle wrap-around...
      toDir = calculateDirection(dx1, dy1);
      
      // Tính direction từ next → curr (enter direction)
      int dx2 = curr.x - next.x;
      int dy2 = curr.y - next.y;
      fromDir = calculateDirection(dx2, dy2);
      
      return (toDir != fromDir);  // Turn nếu hướng vào ≠ hướng ra
  }
  ```

- **BigFood Timer Display**:
  - BigFood image có opacity animation (not implemented in current code)
  - Timer countdown: `getBigFoodTimeLeftMs()` / 5000 * 100 = percentage
  - Auto-disappear sau 5 giây nếu không ăn

- **Game Over Handling**:
  ```cpp
  void handleTickEvent() {
      if (game->isGameOver()) {
          gameOverDelay++;
          if (gameOverDelay >= 60) {  // 1 second delay @ 60 FPS
              presenter->saveScore(game->getScore());
              application().gotoScreen3ScreenNoTransition();
          }
          return;
      }
      
      tickCounter++;
      if (tickCounter >= game->getTickInterval()) {
          tickCounter = 0;
          game->update();  // Update game logic
          updateSnakeDisplay();
          updateFoodDisplay();
          updateBigFoodDisplay();
          updateScoreDisplay();
      }
  }
  ```

**Screen3View (Game Over)**:
- **Chức năng**: Hiển thị kết quả game và high score
- **UI Elements**:
  - Current score display (textArea_Score)
  - High score display (textArea_Highscore)
  - "Play Again" button → Screen2 (reset game)
  - "Main Menu" button → Screen1
- **Data Flow**: `updateScoreDisplay()` pulls từ `presenter->getLastScore()` và `presenter->getHighScore()`

## 3. Luồng Xử Lý Dữ Liệu

### 3.1. Khởi Động Hệ Thống
```
1. main() → HAL_Init() → SystemClock_Config() (180 MHz)
2. Peripheral Init: GPIO, LTDC, DMA2D, FMC SDRAM, I2C, SPI, TIM7
3. FlashStorage_Init() → Load high score từ Sector 23
4. FreeRTOS osKernelInitialize()
5. Create Tasks:
   - defaultTask (button polling, buzzer control)
   - GUI_Task (TouchGFX rendering @ 60 FPS)
6. osKernelStart() → RTOS scheduler running
7. TouchGFX init → Model() constructor → loadHighScoreFromFlash()
8. Screen1View (Menu) hiển thị
```

### 3.2. Game Loop (Screen2)
```
defaultTask (mỗi 20ms):
1. Read GPIO buttons (PD4-PD7, active LOW with pull-up)
2. Debouncing check
3. Snake_UpdateButtonStates() → Model.updateButtonStates()
4. Update buzzer GPIO (PG13) nếu buzzerEndTick expired
5. Check "all 4 buttons pressed 3 seconds" → FlashStorage_EraseAll()

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
   5. updateFoodDisplay()
   6. updateBigFoodDisplay()
   7. updateScoreDisplay()
   8. Reset tickCounter = 0
}
3. handleSoundEvent() - trigger buzzer/ISD1820
4. if (gameOver) wait 1 sec → gotoScreen3()
```

### 3.3. High Score Persistence
```
Game Over:
1. Screen2View detects game->isGameOver() == true
2. Delay 60 frames → presenter->saveScore(score)
3. Model::saveGameScore(score):
   - if (score > highScore) {
       highScore = score;
       Snake_SaveHighScore(score) → FlashStorage_SaveHighScore():
         a. Erase Sector 23 (128KB)
         b. Write struct { magic, highScore, playCount, checksum }
         c. Verify write success
     }
4. Transition to Screen3 (display lastScore, highScore)

Next Boot:
1. FlashStorage_Init() → ReadDataFromFlash()
2. Verify magic == 0x534E414B && checksum valid
3. if valid: return highScore
   else: return 0 (first boot hoặc corrupted data)
```

## 4. Giao Diện Đồ Họa & Assets

### 4.1. Bitmap Assets
**Vị trí**: `Snake/TouchGFX/assets/images/`

| File Name       | Size     | Usage                              |
|----------------|----------|-----------------------------------|
| Head.png       | 10x10    | Snake head (UP direction)          |
| Head1.png      | 10x10    | Snake head (RIGHT direction)       |
| Head2.png      | 10x10    | Snake head (DOWN direction)        |
| Head3.png      | 10x10    | Snake head (LEFT direction)        |
| Mid.png        | 10x10    | Snake body (vertical segment)      |
| Mid1.png       | 10x10    | Snake body (horizontal segment)    |
| Tail.png       | 10x10    | Snake tail (pointing UP)           |
| Tail1.png      | 10x10    | Snake tail (pointing RIGHT)        |
| Tail2.png      | 10x10    | Snake tail (pointing DOWN)         |
| Tail3.png      | 10x10    | Snake tail (pointing LEFT)         |
| Turn.png       | 10x10    | Turn segment (LEFT→UP)             |
| Turn1.png      | 10x10    | Turn segment (DOWN→RIGHT)          |
| Turn2.png      | 10x10    | Turn segment (RIGHT→DOWN)          |
| Turn3.png      | 10x10    | Turn segment (UP→LEFT)             |
| Food.png       | 10x10    | Normal food                        |
| BigFood.png    | 20x20    | BigFood (2x2 cells)                |

### 4.2. Bitmap Selection Logic

**Head Bitmap**:
```cpp
uint16_t getHeadBitmapId(SnakeDirection dir) {
    switch (dir) {
        case SNAKE_DIR_UP:    return BITMAP_HEAD_ID;   // Head.png
        case SNAKE_DIR_RIGHT: return BITMAP_HEAD1_ID;  // Head1.png
        case SNAKE_DIR_DOWN:  return BITMAP_HEAD2_ID;  // Head2.png
        case SNAKE_DIR_LEFT:  return BITMAP_HEAD3_ID;  // Head3.png
    }
}
```

**Turn Bitmap (16 combinations)**:
```cpp
uint16_t getTurnBitmapId(SnakeDirection from, SnakeDirection to) {
    // from = hướng vào, to = hướng ra
    if (from == LEFT && to == UP)     return BITMAP_TURN_ID;   // Turn.png
    if (from == DOWN && to == RIGHT)  return BITMAP_TURN1_ID;  // Turn1.png
    if (from == RIGHT && to == DOWN)  return BITMAP_TURN2_ID;  // Turn2.png
    if (from == UP && to == LEFT)     return BITMAP_TURN3_ID;  // Turn3.png
    // ... (các rotation và flip còn lại)
}
```

### 4.3. Rendering Pipeline
```
LTDC (Layer 0) ← DMA2D ← Frame Buffer (SDRAM @ 240x320x2 bytes = 150KB)
                          ↑
                    TouchGFX Renderer
                          ↑
                    Widget invalidate() calls
                          ↑
                    View::handleTickEvent()
```

- **Frame Buffer**: Single buffering (không double-buffer do SDRAM limited)
- **DMA2D Acceleration**: Sử dụng cho bitmap blitting, color fill
- **TouchGFX Partial Updates**: Chỉ redraw khu vực invalidated (optimize performance)

## 5. Hardware Interface

### 5.1. GPIO Mapping

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

**Button Wiring**:
```
STM32 PD4-7 (with internal pull-up) ──── Button ── GND
```

**Buzzer Wiring**:
```
STM32 PG13 ── Buzzer (+) ── Buzzer (-) ── GND
```

**ISD1820 Wiring**:
```
ISD1820 Module     STM32F429I-DISCO
VCC  ───────────── 3.3V or 5V
GND  ───────────── GND
PLAY-L ───────────── PD12
SP+/SP- ──────────── Speaker
REC  ───────────── Manual record button on module
MIC  ───────────── Microphone input
```

### 5.2. Memory Map

**Flash Memory (2MB total)**:
```
0x08000000 ─────────┐
            ...     │ Sector 0-22: Code + Constants (1920 KB)
0x081DFFFF ─────────┤
0x081E0000 ─────────┐
            ...     │ Sector 23: Persistent Data (128 KB)
0x081FFFFF ─────────┘
```

**SDRAM Memory (8MB total)**:
```
0xD0000000 ─────────┐
            ...     │ TouchGFX Frame Buffer (~150 KB)
            ...     │ TouchGFX Assets Cache
            ...     │ FreeRTOS Heap
0xD07FFFFF ─────────┘
```

**RAM Memory (192 KB internal)**:
```
0x20000000 ─────────┐
            ...     │ Stack, Global variables
            ...     │ FreeRTOS Task stacks
0x2002FFFF ─────────┘
```

### 5.3. Clock Configuration
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
    LTDC, DMA2D   I2C3         TIM7, SPI5
```



## 6. Các Tính Năng Nâng Cao

### 6.1. BigFood Mechanics
- **Trigger**: Xuất hiện sau khi ăn 5 mồi thường
- **Duration**: 5000ms (5 giây)
- **Scoring**: Điểm giảm tuyến tính theo thời gian
  ```
  score(t) = 500 × (5000 - t) / 5000
  t=0ms    → 500 điểm
  t=2500ms → 250 điểm
  t=5000ms → 0 điểm (expired)
  ```
- **Size**: 2×2 cells (20×20 pixels)
- **Collision**: Check nếu snake head overlaps bất kỳ cell nào trong 2×2

### 6.2. Wrap-Around Mechanics
- Không có tường cứng (không game over khi chạm tường)
- Snake teleport sang bên đối diện:
  ```cpp
  if (x < 0) x = GRID_WIDTH - 1;   // Left edge → Right edge
  if (x >= GRID_WIDTH) x = 0;      // Right edge → Left edge
  if (y < 0) y = GRID_HEIGHT - 1;  // Top edge → Bottom edge
  if (y >= GRID_HEIGHT) y = 0;     // Bottom edge → Top edge
  ```
- Game over chỉ xảy ra khi snake head va chạm với body

### 6.3. Buffered Input
- Lệnh di chuyển được buffer trong `nextDirection`
- Ngăn chặn 180° turn (self-collision ngay lập tức)
- Apply vào frame tiếp theo:
  ```cpp
  void update() {
      currentDirection = nextDirection;  // Apply buffered input
      moveSnake();
  }
  ```

### 6.4. High Score Reset
- **Secret Combo**: Giữ cả 4 nút (UP + DOWN + LEFT + RIGHT) trong 3 giây
- **Confirmation**: 2 beep sounds (200ms each, 100ms gap)
- **Action**: `FlashStorage_EraseAll()` → Sector 23 erased
- **Implemented in**: `defaultTask()` polling loop

### 6.5. Difficulty Progression
- Người chơi có thể chọn độ khó trước khi chơi (Screen1)
- Mỗi level có tốc độ và điểm thưởng riêng
- Không auto-scale (người chơi tự chọn challenge level)

## 7. Performance & Optimization

### 7.1. Performance Metrics
- **Frame Rate**: 60 FPS (stable)
- **Memory**:
  - Stack: defaultTask 512 bytes, GUI_Task 32 KB
  - Heap: ~200 KB (TouchGFX assets cache)
  - Frame Buffer: 150 KB (240×320×2)

### 7.2. Optimizations Applied

**TouchGFX Rendering**:
- Partial invalidation: Chỉ redraw snake segments thay đổi
- Bitmap caching: Pre-load tất cả sprites vào SDRAM
- DMA2D hardware acceleration cho blitting

**Game Logic**:
- Fixed-point arithmetic (không dùng float)
- Simple LCG random (không cần hardware RNG)
- Minimal dynamic memory allocation

**FreeRTOS Task Priority**:
- GUI_Task: Normal priority (rendering không bị starve)
- defaultTask: Normal priority (button polling đủ nhanh @ 20ms)


## 8. Source Code Structure
```
Snake_game_on_stm32/
├── README.md                          # This file
├── Snake/
│   ├── Core/
│   │   ├── Inc/
│   │   │   ├── main.h                 # GPIO definitions, extern declarations
│   │   │   ├── flash_storage.h        # Flash API
│   │   │   └── audio_playback.h       # Audio API
│   │   └── Src/
│   │       ├── main.c                 # Hardware init, FreeRTOS tasks
│   │       ├── flash_storage.c        # Persistent storage implementation
│   │       └── audio_playback.c       # Audio hardware control
│   ├── TouchGFX/
│   │   ├── gui/
│   │   │   ├── include/gui/
│   │   │   │   ├── model/
│   │   │   │   │   ├── Model.hpp      # Central data manager
│   │   │   │   │   └── ModelListener.hpp
│   │   │   │   ├── common/
│   │   │   │   │   ├── SnakeGame.hpp  # Core game logic
│   │   │   │   │   └── SnakeInterface.h # C/C++ bridge
│   │   │   │   └── screen[1-3]_screen/
│   │   │   │       ├── Screen[1-3]View.hpp
│   │   │   │       └── Screen[1-3]Presenter.hpp
│   │   │   └── src/
│   │   │       ├── model/
│   │   │       │   └── Model.cpp      # SnakeGame + Model implementation
│   │   │       └── screen[1-3]_screen/
│   │   │           ├── Screen[1-3]View.cpp
│   │   │           └── Screen[1-3]Presenter.cpp
│   │   └── assets/
│   │       └── images/                # Snake sprites, food, bigfood
│   ├── Drivers/                       # STM32 HAL, BSP, CMSIS
│   ├── Middlewares/                   # FreeRTOS, TouchGFX libraries
│   └── STM32F429XX_FLASH.ld           # Modified linker script (Sector 23 reserved)
└── wav_to_c_array.py                  # Audio conversion tool (unused)
```

