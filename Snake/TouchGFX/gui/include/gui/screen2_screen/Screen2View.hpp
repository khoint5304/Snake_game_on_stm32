#ifndef SCREEN2VIEW_HPP
#define SCREEN2VIEW_HPP

#include <gui_generated/screen2_screen/Screen2ViewBase.hpp>
#include <gui/screen2_screen/Screen2Presenter.hpp>
#include <gui/common/SnakeGame.hpp>
#include <touchgfx/widgets/Image.hpp>
#include <touchgfx/containers/Container.hpp>

// Maximum snake segments we can display
#define MAX_DISPLAY_SEGMENTS 100

// External C function for buzzer
extern "C" void Snake_PlayBuzzer(int durationMs);

class Screen2View : public Screen2ViewBase
{
public:
    Screen2View();
    virtual ~Screen2View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

    // Called every frame by TouchGFX
    virtual void handleTickEvent();

    // Called when button is pressed (from presenter)
    void onButtonPressed(SnakeDirection dir);

protected:
    // Update snake display based on game state
    void updateSnakeDisplay();

    // Update food display
    void updateFoodDisplay();

    // Update BigFood display
    void updateBigFoodDisplay();

    // Update score display
    void updateScoreDisplay();

    // Handle sound events
    void handleSoundEvent();

    // Convert grid position to pixel position
    int16_t gridToPixelX(int16_t gridX) { return gridX * CELL_SIZE; }
    int16_t gridToPixelY(int16_t gridY) { return gridY * CELL_SIZE; }

    // Check if segment is a turn segment and get directions
    bool isTurnSegment(uint8_t index, SnakeDirection &fromDir, SnakeDirection &toDir);

    // Get bitmap ID for head based on direction
    uint16_t getHeadBitmapId(SnakeDirection dir);

    // Get bitmap ID for tail based on direction
    uint16_t getTailBitmapId(SnakeDirection dir);

    // Get bitmap ID for mid segment based on direction
    uint16_t getMidBitmapId(SnakeDirection dir);

    // Get bitmap ID for turn segment based on from/to directions
    uint16_t getTurnBitmapId(SnakeDirection fromDir, SnakeDirection toDir);

private:
    // Game reference
    SnakeGame *game;

    // Tick counter for game speed control
    uint32_t tickCounter;

    // Snake body images (dynamically managed)
    touchgfx::Image snakeSegments[MAX_DISPLAY_SEGMENTS];
    uint8_t currentSegmentCount;

    // Container for snake segments
    touchgfx::Container snakeContainer;

    // BigFood image
    touchgfx::Image bigFoodImage;

    // Score text buffer
    touchgfx::Unicode::UnicodeChar scoreBuffer[10];

    // Game started flag
    bool gameStarted;

    // Game over delay counter (for transition)
    uint8_t gameOverDelay;
};

#endif // SCREEN2VIEW_HPP
