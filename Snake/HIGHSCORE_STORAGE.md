# Highscore Flash Storage - Hướng dẫn

## Tổng quan

Tính năng này cho phép lưu điểm cao (highscore) vào bộ nhớ Flash nội của STM32F429, đảm bảo dữ liệu **không bị mất** khi:
- Bấm nút **Reset** trên board
- Tắt nguồn và bật lại
- Nạp lại firmware (nếu code không vượt quá 1920KB)

## Cách hoạt động

### Vùng nhớ Flash được sử dụng
- **Sector 23** (128KB cuối cùng): `0x081E0000 - 0x081FFFFF`
- Vùng này được dành riêng cho lưu trữ dữ liệu, không dùng cho code

### Cấu trúc dữ liệu lưu trữ
```c
typedef struct {
    uint32_t magic;       // "SNAK" - Xác nhận dữ liệu hợp lệ
    uint16_t highScore;   // Điểm cao nhất
    uint16_t reserved1;   // Dự phòng
    uint32_t playCount;   // Số lần chơi
    uint32_t checksum;    // Kiểm tra tính toàn vẹn dữ liệu
} FlashStorageData_t;
```

## Các file liên quan

| File | Mô tả |
|------|-------|
| `Core/Inc/flash_storage.h` | Header file với các API |
| `Core/Src/flash_storage.c` | Implementation Flash storage |
| `TouchGFX/gui/include/gui/common/SnakeInterface.h` | Interface cho TouchGFX |
| `TouchGFX/gui/include/gui/model/Model.hpp` | Model với loadHighScoreFromFlash() |
| `TouchGFX/gui/src/model/Model.cpp` | Logic lưu/load highscore |
| `STM32F429XX_FLASH.ld` | Linker script (đã chỉnh sửa) |

## API Functions

### Từ C (main.c)
```c
// Khởi tạo Flash storage (gọi 1 lần khi khởi động)
void Snake_InitStorage(void);

// Load điểm cao từ Flash
uint16_t Snake_LoadHighScore(void);

// Lưu điểm cao vào Flash (chỉ lưu nếu cao hơn hiện tại)
void Snake_SaveHighScore(uint16_t score);
```

### Từ C++ (TouchGFX Model)
```cpp
// Trong Model class
void loadHighScoreFromFlash();  // Tự động gọi trong constructor
void saveGameScore(uint16_t score);  // Tự động lưu vào Flash nếu là điểm cao mới
```

## Luồng hoạt động

### Khi khởi động:
1. `main()` gọi `FlashStorage_Init()` để khởi tạo module
2. `Model::Model()` gọi `loadHighScoreFromFlash()` để load điểm cao
3. Nếu dữ liệu trong Flash hợp lệ (magic + checksum đúng), highscore được khôi phục
4. Nếu không hợp lệ (lần đầu chạy hoặc dữ liệu hỏng), highscore = 0

### Khi game over:
1. `saveGameScore(score)` được gọi với điểm hiện tại
2. Nếu `score > highScore`:
   - Cập nhật `highScore` trong RAM
   - Gọi `Snake_SaveHighScore(score)` để lưu vào Flash
   - Flash Sector 23 bị xóa và ghi lại với dữ liệu mới

## Lưu ý quan trọng

### ⚠️ Giới hạn số lần ghi Flash
- Flash memory có giới hạn khoảng **10,000 - 100,000** lần xóa/ghi
- Module này chỉ ghi khi có **điểm cao mới** (không ghi nếu điểm thấp hơn)
- Trong sử dụng bình thường, điều này hoàn toàn đủ

### ⚠️ Kích thước code
- Linker script giới hạn vùng FLASH cho code là **1920KB** (giảm từ 2048KB)
- 128KB cuối dành cho storage
- Nếu code vượt quá 1920KB, cần giảm kích thước code hoặc sử dụng sector khác

### ⚠️ Tương thích với ST-Link Programmer
- Khi nạp firmware bằng ST-Link, mặc định chỉ xóa vùng code (0x08000000 - 0x081DFFFF)
- Sector 23 sẽ **không bị xóa**, highscore được giữ nguyên
- Nếu muốn reset highscore, sử dụng **Full Chip Erase** trong STM32CubeProgrammer

## Reset Highscore

### Cách 1: Trong code
```c
#include "flash_storage.h"

// Xóa toàn bộ dữ liệu lưu trữ
FlashStorage_EraseAll();
```

### Cách 2: Sử dụng STM32CubeProgrammer
1. Kết nối board với ST-Link
2. Mở STM32CubeProgrammer
3. Connect → Erasing & programming → Full chip erase
4. Nạp lại firmware

## Troubleshooting

### Highscore không được lưu
1. Kiểm tra linker script có giới hạn FLASH đúng không (1920K)
2. Kiểm tra `FlashStorage_Init()` đã được gọi trong `main()` chưa
3. Thử gọi `FlashStorage_EraseAll()` một lần để reset sector

### Highscore bị reset về 0 sau khi nạp firmware
- Kiểm tra STM32CubeProgrammer không chọn "Full chip erase"
- Chỉ xóa các sector cần thiết (0-22)

### Build lỗi "region FLASH overflowed"
- Code quá lớn (> 1920KB)
- Giải pháp: Tối ưu code hoặc sử dụng Sector 22 thay vì 23

## Mở rộng

Có thể mở rộng module để lưu thêm dữ liệu:
```c
// Trong flash_storage.h, thêm vào FlashStorageData_t:
typedef struct {
    uint32_t magic;
    uint16_t highScore;
    uint16_t reserved1;
    uint32_t playCount;
    uint8_t  lastDifficulty;    // Difficulty cuối cùng
    uint8_t  soundEnabled;       // Bật/tắt âm thanh
    uint16_t reserved2;
    uint32_t checksum;
} FlashStorageData_t;
```

Nhớ cập nhật `CalculateChecksum()` nếu thêm fields mới.
