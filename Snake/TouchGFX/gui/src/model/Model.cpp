#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include <gui/common/SnakeGame.hpp>
#include <gui/common/FrontendHeap.hpp>
#include <gui/common/SnakeInterface.h>

// =====================================================
// SnakeGame Implementation (merged from SnakeGame.cpp)
// =====================================================

SnakeGame::SnakeGame()
    : snakeLength(3), currentDirection(SNAKE_DIR_UP), nextDirection(SNAKE_DIR_UP), score(0), gameOver(false), difficulty(NORMAL), randomState(12345), bigFoodActive(false), bigFoodStartTime(0), foodEatenCount(0), pendingSound(SOUND_NONE)
{
    init();
}

void SnakeGame::init()
{
    reset();
}

void SnakeGame::reset()
{
    // Initialize snake in center of game area
    snakeLength = 3;

    // Head position (center of grid)
    snake[0].x = GRID_WIDTH / 2;
    snake[0].y = GRID_HEIGHT / 2;

    // Body segments (below head - snake starts moving up)
    snake[1].x = GRID_WIDTH / 2;
    snake[1].y = GRID_HEIGHT / 2 + 1;

    // Tail
    snake[2].x = GRID_WIDTH / 2;
    snake[2].y = GRID_HEIGHT / 2 + 2;

    currentDirection = SNAKE_DIR_UP;
    nextDirection = SNAKE_DIR_UP;
    score = 0;
    gameOver = false;

    // Reset BigFood state
    bigFoodActive = false;
    bigFoodStartTime = 0;
    foodEatenCount = 0;
    pendingSound = SOUND_NONE;

    // Spawn initial food
    spawnFood();
}

bool SnakeGame::update()
{
    if (gameOver)
    {
        return false;
    }

    // Apply buffered direction
    currentDirection = nextDirection;

    // Move snake
    moveSnake();

    // Update BigFood timer
    updateBigFood();

    // Check for BigFood collision first (higher priority)
    if (bigFoodActive)
    {
        // BigFood is 2x2 cells, check if head overlaps
        if (snake[0].x >= bigFood.x && snake[0].x < bigFood.x + 2 &&
            snake[0].y >= bigFood.y && snake[0].y < bigFood.y + 2)
        {
            growSnake();
            // Score: 500 * (5000 - t) / 5000, where t is ms elapsed
            uint32_t elapsedMs = getBigFoodElapsedMs();
            if (elapsedMs > BIGFOOD_DURATION_MS)
                elapsedMs = BIGFOOD_DURATION_MS;
            uint32_t bigFoodScore = (BIGFOOD_MAX_SCORE * (BIGFOOD_DURATION_MS - elapsedMs)) / BIGFOOD_DURATION_MS;
            score += (uint16_t)bigFoodScore;

            bigFoodActive = false;
            bigFoodStartTime = 0;
            pendingSound = SOUND_EAT_BIGFOOD;
        }
    }

    // Check for normal food collision
    if (snake[0] == food)
    {
        growSnake();
        // Score based on difficulty: EASY=1, NORMAL=3, HARD=5, INSANE=8, NIGHTMARE=12
        switch (difficulty)
        {
        case EASY:
            score += 1;
            break;
        case NORMAL:
            score += 3;
            break;
        case HARD:
            score += 5;
            break;
        case INSANE:
            score += 8;
            break;
        case NIGHTMARE:
            score += 12;
            break;
        }
        spawnFood();
        pendingSound = SOUND_EAT_FOOD;

        // Check if BigFood should appear
        foodEatenCount++;
        if (foodEatenCount >= BIGFOOD_APPEAR_AFTER && !bigFoodActive)
        {
            spawnBigFood();
            foodEatenCount = 0;
        }
    }

    // Check for wall or self collision
    if (checkCollision())
    {
        gameOver = true;
        pendingSound = SOUND_GAME_OVER;
        return false;
    }

    return true;
}

void SnakeGame::setDirection(SnakeDirection dir)
{
    // Prevent 180-degree turns
    if ((currentDirection == SNAKE_DIR_UP && dir == SNAKE_DIR_DOWN) ||
        (currentDirection == SNAKE_DIR_DOWN && dir == SNAKE_DIR_UP) ||
        (currentDirection == SNAKE_DIR_LEFT && dir == SNAKE_DIR_RIGHT) ||
        (currentDirection == SNAKE_DIR_RIGHT && dir == SNAKE_DIR_LEFT))
    {
        return;
    }

    nextDirection = dir;
}

void SnakeGame::setDifficulty(Difficulty diff)
{
    difficulty = diff;
}

void SnakeGame::cycleDifficulty()
{
    switch (difficulty)
    {
    case EASY:
        difficulty = NORMAL;
        break;
    case NORMAL:
        difficulty = HARD;
        break;
    case HARD:
        difficulty = INSANE;
        break;
    case INSANE:
        difficulty = NIGHTMARE;
        break;
    case NIGHTMARE:
        difficulty = EASY;
        break;
    }
}

uint32_t SnakeGame::getTickInterval() const
{
    // TouchGFX typically runs at 60 FPS, so tick() is called every ~16.67ms
    // We need to return the interval in number of ticks
    // Easy: 1 cell/sec = 1000ms interval = 60 ticks
    // Normal: 3 cells/sec = 333ms interval = 20 ticks
    // Hard: 5 cells/sec = 200ms interval = 12 ticks
    // Insane: 8 cells/sec = 125ms interval = 7.5 ticks
    // Nightmare: 12 cells/sec = 83ms interval = 5 ticks
    switch (difficulty)
    {
    case EASY:
        return 60; // 1 move per second
    case NORMAL:
        return 20; // 3 moves per second
    case HARD:
        return 12; // 5 moves per second
    case INSANE:
        return 7; // 8 moves per second
    case NIGHTMARE:
        return 5; // 12 moves per second
    default:
        return 20;
    }
}

void SnakeGame::moveSnake()
{
    // Store previous position of each segment
    Position prevPos = snake[0];
    Position currentPos;

    // Move head based on direction
    switch (currentDirection)
    {
    case SNAKE_DIR_UP:
        snake[0].y--;
        break;
    case SNAKE_DIR_DOWN:
        snake[0].y++;
        break;
    case SNAKE_DIR_LEFT:
        snake[0].x--;
        break;
    case SNAKE_DIR_RIGHT:
        snake[0].x++;
        break;
    }

    // Wrap around when hitting walls (teleport to opposite side)
    if (snake[0].x < 0)
        snake[0].x = GRID_WIDTH - 1;
    else if (snake[0].x >= GRID_WIDTH)
        snake[0].x = 0;

    if (snake[0].y < 0)
        snake[0].y = GRID_HEIGHT - 1;
    else if (snake[0].y >= GRID_HEIGHT)
        snake[0].y = 0;

    // Move body segments to follow
    for (uint8_t i = 1; i < snakeLength; i++)
    {
        currentPos = snake[i];
        snake[i] = prevPos;
        prevPos = currentPos;
    }
}

void SnakeGame::growSnake()
{
    if (snakeLength < MAX_SNAKE_LENGTH)
    {
        // Add new segment at the end (will be placed correctly on next move)
        snake[snakeLength] = snake[snakeLength - 1];
        snakeLength++;
    }
}

void SnakeGame::spawnFood()
{
    Position newPos;

    // Simple linear congruential generator for randomness
    do
    {
        randomState = randomState * 1103515245 + 12345;
        newPos.x = (randomState >> 16) % GRID_WIDTH;

        randomState = randomState * 1103515245 + 12345;
        newPos.y = (randomState >> 16) % GRID_HEIGHT;
    } while (isPositionOnSnake(newPos) || isPositionOnBigFood(newPos));

    food = newPos;
}

void SnakeGame::spawnBigFood()
{
    Position newPos;

    // BigFood is 2x2 cells (20x20 pixels)
    // Make sure it fits within the grid
    do
    {
        randomState = randomState * 1103515245 + 12345;
        newPos.x = (randomState >> 16) % (GRID_WIDTH - 1); // -1 because 2 cells wide

        randomState = randomState * 1103515245 + 12345;
        newPos.y = (randomState >> 16) % (GRID_HEIGHT - 1); // -1 because 2 cells tall
    } while (isPositionOnSnake(newPos) ||
             (newPos.x == food.x && newPos.y == food.y));

    bigFood = newPos;
    bigFoodActive = true;
    bigFoodStartTime = Snake_GetTickMs(); // Record start time in ms
}

void SnakeGame::updateBigFood()
{
    if (bigFoodActive)
    {
        uint32_t elapsedMs = Snake_GetTickMs() - bigFoodStartTime;
        if (elapsedMs >= BIGFOOD_DURATION_MS)
        {
            // BigFood expired after 5000ms
            bigFoodActive = false;
            bigFoodStartTime = 0;
        }
    }
}

uint32_t SnakeGame::getBigFoodTimeLeftMs() const
{
    if (!bigFoodActive)
        return 0;
    uint32_t elapsedMs = Snake_GetTickMs() - bigFoodStartTime;
    if (elapsedMs >= BIGFOOD_DURATION_MS)
        return 0;
    return BIGFOOD_DURATION_MS - elapsedMs;
}

uint32_t SnakeGame::getBigFoodElapsedMs() const
{
    if (!bigFoodActive)
        return BIGFOOD_DURATION_MS;
    return Snake_GetTickMs() - bigFoodStartTime;
}

bool SnakeGame::isPositionOnBigFood(Position pos)
{
    if (!bigFoodActive)
        return false;

    // BigFood occupies 2x2 cells
    return (pos.x >= bigFood.x && pos.x < bigFood.x + 2 &&
            pos.y >= bigFood.y && pos.y < bigFood.y + 2);
}

bool SnakeGame::checkCollision()
{
    // Wall collision is now handled by wrap-around in moveSnake()
    // Only check self collision (head with body)
    for (uint8_t i = 1; i < snakeLength; i++)
    {
        if (snake[0] == snake[i])
        {
            return true;
        }
    }

    return false;
}

bool SnakeGame::isPositionOnSnake(Position pos)
{
    for (uint8_t i = 0; i < snakeLength; i++)
    {
        if (snake[i] == pos)
        {
            return true;
        }
    }
    return false;
}

SnakeDirection SnakeGame::getSegmentDirection(uint8_t index) const
{
    if (index >= snakeLength - 1)
    {
        // For the last segment (tail), use direction from previous segment
        if (snakeLength >= 2)
        {
            int16_t dx = snake[snakeLength - 2].x - snake[snakeLength - 1].x;
            int16_t dy = snake[snakeLength - 2].y - snake[snakeLength - 1].y;

            if (dx > 0)
                return SNAKE_DIR_RIGHT;
            if (dx < 0)
                return SNAKE_DIR_LEFT;
            if (dy > 0)
                return SNAKE_DIR_DOWN;
            if (dy < 0)
                return SNAKE_DIR_UP;
        }
        return currentDirection;
    }

    // Direction is determined by next segment position
    int16_t dx = snake[index].x - snake[index + 1].x;
    int16_t dy = snake[index].y - snake[index + 1].y;

    if (dx > 0)
        return SNAKE_DIR_RIGHT;
    if (dx < 0)
        return SNAKE_DIR_LEFT;
    if (dy > 0)
        return SNAKE_DIR_DOWN;
    if (dy < 0)
        return SNAKE_DIR_UP;

    return currentDirection;
}

uint32_t SnakeGame::getRandomSeed()
{
    // In a real embedded system, you might use a timer or other hardware RNG
    // For now, just increment the state
    randomState++;
    return randomState;
}

// =====================================================
// SnakeInterface Implementation (C interface for main.c)
// =====================================================

extern "C"
{
    void Snake_UpdateButtonStates(int up, int down, int left, int right)
    {
        // Get the model instance from FrontendHeap and update button states
        FrontendHeap &heap = FrontendHeap::getInstance();
        heap.model.updateButtonStates(
            up != 0,
            down != 0,
            left != 0,
            right != 0);
    }
}

// =====================================================
// Model Implementation
// =====================================================

Model::Model()
    : modelListener(0), buttonUp(false), buttonDown(false), buttonLeft(false), buttonRight(false), prevButtonUp(false), prevButtonDown(false), prevButtonLeft(false), prevButtonRight(false), highScore(0), lastScore(0)
{
    // Load high score from Flash storage at startup
    loadHighScoreFromFlash();
}

void Model::tick()
{
    // Notify listener about button state changes (edge detection)
    if (modelListener)
    {
        // Check for new button presses (rising edge: was not pressed, now pressed)
        if (buttonUp && !prevButtonUp)
        {
            modelListener->buttonPressed(SNAKE_DIR_UP);
        }
        if (buttonDown && !prevButtonDown)
        {
            modelListener->buttonPressed(SNAKE_DIR_DOWN);
        }
        if (buttonLeft && !prevButtonLeft)
        {
            modelListener->buttonPressed(SNAKE_DIR_LEFT);
        }
        if (buttonRight && !prevButtonRight)
        {
            modelListener->buttonPressed(SNAKE_DIR_RIGHT);
        }
    }

    // Update previous states
    prevButtonUp = buttonUp;
    prevButtonDown = buttonDown;
    prevButtonLeft = buttonLeft;
    prevButtonRight = buttonRight;
}

void Model::updateButtonStates(bool up, bool down, bool left, bool right)
{
    buttonUp = up;
    buttonDown = down;
    buttonLeft = left;
    buttonRight = right;
}

void Model::saveGameScore(uint16_t score)
{
    lastScore = score;
    if (score > highScore)
    {
        highScore = score;
        // Save new high score to Flash storage (persists across resets)
        Snake_SaveHighScore(score);
    }
}

void Model::loadHighScoreFromFlash()
{
    // Load high score from Flash storage
    highScore = Snake_LoadHighScore();
}
