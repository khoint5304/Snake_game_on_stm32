#ifndef SNAKEGAME_HPP
#define SNAKEGAME_HPP

#include <stdint.h>

// Game constants
#define CELL_SIZE 10                               // 10x10 pixel per cell
#define GAME_AREA_WIDTH 240                        // box1 width
#define GAME_AREA_HEIGHT 280                       // box1 height
#define GRID_WIDTH (GAME_AREA_WIDTH / CELL_SIZE)   // 24 cells
#define GRID_HEIGHT (GAME_AREA_HEIGHT / CELL_SIZE) // 28 cells
#define MAX_SNAKE_LENGTH 100                       // Maximum snake length

// BigFood constants
#define BIGFOOD_SIZE 20          // 20x20 pixels
#define BIGFOOD_APPEAR_AFTER 5   // Appear after eating 5 normal food
#define BIGFOOD_DURATION_MS 5000 // 5000 milliseconds (5 seconds real time)
#define BIGFOOD_MAX_SCORE 500    // Maximum score (at t=0)

// Sound event types
enum SoundEvent
{
    SOUND_NONE = 0,
    SOUND_EAT_FOOD,    // 0.1s beep
    SOUND_EAT_BIGFOOD, // 0.3s beep
    SOUND_GAME_OVER    // 1.0s beep
};

// Directions (renamed to avoid conflict with touchgfx::Direction)
enum SnakeDirection
{
    SNAKE_DIR_UP = 0,
    SNAKE_DIR_DOWN,
    SNAKE_DIR_LEFT,
    SNAKE_DIR_RIGHT
};

// Difficulty levels
enum Difficulty
{
    EASY = 0, // 1 cell per second, 1 point per food
    NORMAL,   // 3 cells per second, 3 points per food
    HARD,     // 5 cells per second, 5 points per food
    INSANE,   // 8 cells per second, 8 points per food
    NIGHTMARE // 12 cells per second, 12 points per food
};

// Position structure
struct Position
{
    int16_t x; // Grid x (0 to GRID_WIDTH-1)
    int16_t y; // Grid y (0 to GRID_HEIGHT-1)

    bool operator==(const Position &other) const
    {
        return x == other.x && y == other.y;
    }
};

// Snake game class
class SnakeGame
{
public:
    SnakeGame();

    // Game control
    void init();
    void reset();
    bool update(); // Returns false if game over

    // Input
    void setDirection(SnakeDirection dir);

    // Difficulty
    void setDifficulty(Difficulty diff);
    Difficulty getDifficulty() const { return difficulty; }
    void cycleDifficulty(); // Cycle through EASY -> NORMAL -> HARD -> EASY

    // Get tick interval based on difficulty (in milliseconds)
    uint32_t getTickInterval() const;

    // Getters
    uint16_t getScore() const { return score; }
    bool isGameOver() const { return gameOver; }

    // Snake body access
    uint8_t getSnakeLength() const { return snakeLength; }
    Position getSnakeHead() const { return snake[0]; }
    Position getSnakeSegment(uint8_t index) const { return snake[index]; }
    Position getSnakeTail() const { return snake[snakeLength - 1]; }

    // Food position
    Position getFoodPosition() const { return food; }

    // BigFood
    bool isBigFoodActive() const { return bigFoodActive; }
    Position getBigFoodPosition() const { return bigFood; }
    uint32_t getBigFoodTimeLeftMs() const; // Time left in ms
    uint32_t getBigFoodElapsedMs() const;  // Time elapsed in ms

    // Sound event (check and clear)
    SoundEvent getSoundEvent()
    {
        SoundEvent e = pendingSound;
        pendingSound = SOUND_NONE;
        return e;
    }

    // Direction getter
    SnakeDirection getCurrentDirection() const { return currentDirection; }
    SnakeDirection getSegmentDirection(uint8_t index) const;

private:
    void moveSnake();
    void growSnake();
    void spawnFood();
    void spawnBigFood();
    void updateBigFood();
    bool checkCollision();
    bool isPositionOnSnake(Position pos);
    bool isPositionOnBigFood(Position pos);
    uint32_t getRandomSeed();

    // Snake body (head at index 0)
    Position snake[MAX_SNAKE_LENGTH];
    uint8_t snakeLength;

    // Food position
    Position food;

    // BigFood
    Position bigFood;
    bool bigFoodActive;
    uint32_t bigFoodStartTime; // Start time in ms (real time)
    uint8_t foodEatenCount;    // Count of normal food eaten

    // Sound event
    SoundEvent pendingSound;

    // Game state
    SnakeDirection currentDirection;
    SnakeDirection nextDirection; // Buffered input to prevent 180-degree turns
    uint16_t score;
    bool gameOver;
    Difficulty difficulty;

    // Simple random state
    uint32_t randomState;
};

#endif // SNAKEGAME_HPP
