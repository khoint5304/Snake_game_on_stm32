#include <gui/screen2_screen/Screen2View.hpp>
#include <images/BitmapDatabase.hpp>
#include <touchgfx/Color.hpp>

Screen2View::Screen2View()
    : game(0), tickCounter(0), currentSegmentCount(0), gameStarted(false), gameOverDelay(0)
{
    // Initialize score buffer
    scoreBuffer[0] = '0';
    scoreBuffer[1] = 0;
}

void Screen2View::setupScreen()
{
    Screen2ViewBase::setupScreen();

    // Get game reference from presenter/model
    game = &presenter->getSnakeGame();

    // Reset game when entering screen
    game->reset();

    // Setup snake container
    snakeContainer.setPosition(0, 0, GAME_AREA_WIDTH, GAME_AREA_HEIGHT);
    add(snakeContainer);

    // Hide the default images placed in designer (we'll manage them dynamically)
    image1.setVisible(false);
    image2.setVisible(false);
    image3.setVisible(false);
    image4.setVisible(false);

    // Initialize snake segment images
    for (int i = 0; i < MAX_DISPLAY_SEGMENTS; i++)
    {
        snakeSegments[i].setVisible(false);
        snakeContainer.add(snakeSegments[i]);
    }

    // Initialize BigFood image
    bigFoodImage.setVisible(false);
    add(bigFoodImage);

    // Set up score display - change color to white so it's visible on black background
    textArea1.setColor(touchgfx::Color::getColorFromRGB(255, 255, 255));

    // Set initial wildcard buffer for score
    touchgfx::Unicode::snprintf(scoreBuffer, 10, "%d", 0);
    textArea1.setWildcard(scoreBuffer);

    // Initial display update
    updateSnakeDisplay();
    updateFoodDisplay();
    updateBigFoodDisplay();
    updateScoreDisplay();

    tickCounter = 0;
    gameStarted = true;
    gameOverDelay = 0;
}

void Screen2View::tearDownScreen()
{
    Screen2ViewBase::tearDownScreen();
    gameStarted = false;
}

void Screen2View::handleTickEvent()
{
    if (!game || !gameStarted)
    {
        return;
    }

    // Handle sound events
    handleSoundEvent();

    // Handle game over state - delay before transitioning
    if (game->isGameOver())
    {
        gameOverDelay++;
        if (gameOverDelay >= 60) // About 1 second delay at 60 FPS
        {
            // Save score to model before transitioning
            presenter->saveScore(game->getScore());

            // Transition to Screen3 (Game Over screen) - no transition
            application().gotoScreen3ScreenNoTransition();
        }
        return;
    }

    tickCounter++;

    // Update BigFood display every tick (for timer countdown visual)
    updateBigFoodDisplay();

    // Check if it's time to update game based on difficulty
    if (tickCounter >= game->getTickInterval())
    {
        tickCounter = 0;

        // Update game logic
        bool continueGame = game->update();

        if (continueGame)
        {
            // Update display
            updateSnakeDisplay();
            updateFoodDisplay();
            updateBigFoodDisplay();
            updateScoreDisplay();
        }
        // If game over, the next tick will handle the transition
    }
}

void Screen2View::handleSoundEvent()
{
    SoundEvent event = game->getSoundEvent();
    switch (event)
    {
    case SOUND_EAT_FOOD:
        Snake_PlayBuzzer(100); // 0.1 second
        break;
    case SOUND_EAT_BIGFOOD:
        Snake_PlayBuzzer(300); // 0.3 second
        break;
    case SOUND_GAME_OVER:
        Snake_PlayBuzzer(1000); // 1 second buzzer
        Snake_PlayMusic();      // Play ISD1820 music
        break;
    default:
        break;
    }
}

void Screen2View::onButtonPressed(SnakeDirection dir)
{
    if (game && !game->isGameOver())
    {
        game->setDirection(dir);
    }
}

bool Screen2View::isTurnSegment(uint8_t index, SnakeDirection &fromDir, SnakeDirection &toDir)
{
    if (index == 0 || index >= game->getSnakeLength() - 1)
    {
        return false; // Head and tail are not turn segments
    }

    // Get positions of previous (towards head), current, and next (towards tail) segments
    Position prev = game->getSnakeSegment(index - 1); // towards head
    Position curr = game->getSnakeSegment(index);
    Position next = game->getSnakeSegment(index + 1); // towards tail

    // Calculate direction from current to prev (towards head)
    int16_t dx1 = prev.x - curr.x;
    int16_t dy1 = prev.y - curr.y;

    // Handle wrap-around
    if (dx1 > 1)
        dx1 = -1;
    if (dx1 < -1)
        dx1 = 1;
    if (dy1 > 1)
        dy1 = -1;
    if (dy1 < -1)
        dy1 = 1;

    // Calculate direction from next to current (from tail towards this segment)
    int16_t dx2 = curr.x - next.x;
    int16_t dy2 = curr.y - next.y;

    // Handle wrap-around
    if (dx2 > 1)
        dx2 = -1;
    if (dx2 < -1)
        dx2 = 1;
    if (dy2 > 1)
        dy2 = -1;
    if (dy2 < -1)
        dy2 = 1;

    // toDir = direction this segment is "exiting" (towards head)
    if (dx1 > 0)
        toDir = SNAKE_DIR_RIGHT;
    else if (dx1 < 0)
        toDir = SNAKE_DIR_LEFT;
    else if (dy1 > 0)
        toDir = SNAKE_DIR_DOWN;
    else if (dy1 < 0)
        toDir = SNAKE_DIR_UP;
    else
        return false;

    // fromDir = direction this segment is "entering" (from tail)
    if (dx2 > 0)
        fromDir = SNAKE_DIR_RIGHT;
    else if (dx2 < 0)
        fromDir = SNAKE_DIR_LEFT;
    else if (dy2 > 0)
        fromDir = SNAKE_DIR_DOWN;
    else if (dy2 < 0)
        fromDir = SNAKE_DIR_UP;
    else
        return false;

    // It's a turn if directions are perpendicular
    bool fromHorizontal = (fromDir == SNAKE_DIR_LEFT || fromDir == SNAKE_DIR_RIGHT);
    bool toHorizontal = (toDir == SNAKE_DIR_LEFT || toDir == SNAKE_DIR_RIGHT);

    return fromHorizontal != toHorizontal;
}

uint16_t Screen2View::getHeadBitmapId(SnakeDirection dir)
{
    // HEAD = up (original), HEAD1 = 90° CCW = left, HEAD2 = 180° = down, HEAD3 = 270° CCW = right
    switch (dir)
    {
    case SNAKE_DIR_UP:
        return BITMAP_HEAD_ID;
    case SNAKE_DIR_LEFT:
        return BITMAP_HEAD1_ID;
    case SNAKE_DIR_DOWN:
        return BITMAP_HEAD2_ID;
    case SNAKE_DIR_RIGHT:
        return BITMAP_HEAD3_ID;
    default:
        return BITMAP_HEAD_ID;
    }
}

uint16_t Screen2View::getTailBitmapId(SnakeDirection dir)
{
    // TAIL = up (original), TAIL1 = 90° CCW = left, TAIL2 = 180° = down, TAIL3 = 270° CCW = right
    switch (dir)
    {
    case SNAKE_DIR_UP:
        return BITMAP_TAIL_ID;
    case SNAKE_DIR_LEFT:
        return BITMAP_TAIL1_ID;
    case SNAKE_DIR_DOWN:
        return BITMAP_TAIL2_ID;
    case SNAKE_DIR_RIGHT:
        return BITMAP_TAIL3_ID;
    default:
        return BITMAP_TAIL_ID;
    }
}

uint16_t Screen2View::getMidBitmapId(SnakeDirection dir)
{
    // MID = vertical (original), MID1 = horizontal (90° CCW)
    if (dir == SNAKE_DIR_LEFT || dir == SNAKE_DIR_RIGHT)
    {
        return BITMAP_MID1_ID;
    }
    return BITMAP_MID_ID;
}

uint16_t Screen2View::getTurnBitmapId(SnakeDirection fromDir, SnakeDirection toDir)
{
    // TURN (┌): coming from UP going RIGHT, or coming from LEFT going DOWN
    // TURN1 (└): coming from DOWN going RIGHT, or coming from LEFT going UP
    // TURN2 (┘): coming from DOWN going LEFT, or coming from RIGHT going UP
    // TURN3 (┐): coming from UP going LEFT, or coming from RIGHT going DOWN

    // TURN (┌)
    if ((fromDir == SNAKE_DIR_UP && toDir == SNAKE_DIR_RIGHT) ||
        (fromDir == SNAKE_DIR_LEFT && toDir == SNAKE_DIR_DOWN))
    {
        return BITMAP_TURN_ID;
    }
    // TURN1 (└)
    else if ((fromDir == SNAKE_DIR_DOWN && toDir == SNAKE_DIR_RIGHT) ||
             (fromDir == SNAKE_DIR_LEFT && toDir == SNAKE_DIR_UP))
    {
        return BITMAP_TURN1_ID;
    }
    // TURN2 (┘)
    else if ((fromDir == SNAKE_DIR_DOWN && toDir == SNAKE_DIR_LEFT) ||
             (fromDir == SNAKE_DIR_RIGHT && toDir == SNAKE_DIR_UP))
    {
        return BITMAP_TURN2_ID;
    }
    // TURN3 (┐)
    else if ((fromDir == SNAKE_DIR_UP && toDir == SNAKE_DIR_LEFT) ||
             (fromDir == SNAKE_DIR_RIGHT && toDir == SNAKE_DIR_DOWN))
    {
        return BITMAP_TURN3_ID;
    }

    return BITMAP_TURN_ID;
}

void Screen2View::updateSnakeDisplay()
{
    if (!game)
        return;

    uint8_t snakeLen = game->getSnakeLength();

    // Hide previously shown segments that are no longer needed
    for (uint8_t i = snakeLen; i < currentSegmentCount; i++)
    {
        snakeSegments[i].setVisible(false);
    }

    // Update each snake segment
    for (uint8_t i = 0; i < snakeLen && i < MAX_DISPLAY_SEGMENTS; i++)
    {
        Position pos = game->getSnakeSegment(i);

        // Set position
        int16_t pixelX = gridToPixelX(pos.x);
        int16_t pixelY = gridToPixelY(pos.y);
        snakeSegments[i].setXY(pixelX, pixelY);

        // Determine which bitmap to use
        uint16_t bitmapId;

        if (i == 0)
        {
            // Head - use current direction
            bitmapId = getHeadBitmapId(game->getCurrentDirection());
        }
        else if (i == snakeLen - 1)
        {
            // Tail - use segment direction
            SnakeDirection tailDir = game->getSegmentDirection(i);
            bitmapId = getTailBitmapId(tailDir);
        }
        else
        {
            // Body segment - check if it's a turn
            SnakeDirection fromDir, toDir;
            if (isTurnSegment(i, fromDir, toDir))
            {
                bitmapId = getTurnBitmapId(fromDir, toDir);
            }
            else
            {
                // Straight segment
                SnakeDirection segDir = game->getSegmentDirection(i);
                bitmapId = getMidBitmapId(segDir);
            }
        }

        // Apply bitmap
        snakeSegments[i].setBitmap(touchgfx::Bitmap(bitmapId));
        snakeSegments[i].setVisible(true);
        snakeSegments[i].invalidate();
    }

    currentSegmentCount = snakeLen;
    snakeContainer.invalidate();
}

void Screen2View::updateFoodDisplay()
{
    if (!game)
        return;

    Position foodPos = game->getFoodPosition();

    // Use image4 for food (from the base class)
    image4.setXY(gridToPixelX(foodPos.x), gridToPixelY(foodPos.y));
    image4.setBitmap(touchgfx::Bitmap(BITMAP_FOOD_ID));
    image4.setVisible(true);
    image4.invalidate();
}

void Screen2View::updateBigFoodDisplay()
{
    if (!game)
        return;

    if (game->isBigFoodActive())
    {
        Position bigFoodPos = game->getBigFoodPosition();

        // BigFood is 20x20 pixels (2x2 cells)
        bigFoodImage.setXY(gridToPixelX(bigFoodPos.x), gridToPixelY(bigFoodPos.y));
        bigFoodImage.setBitmap(touchgfx::Bitmap(BITMAP_BIGFOOD_ID));
        bigFoodImage.setVisible(true);
        bigFoodImage.invalidate();
    }
    else
    {
        bigFoodImage.setVisible(false);
        bigFoodImage.invalidate();
    }
}

void Screen2View::updateScoreDisplay()
{
    if (!game)
        return;

    uint16_t score = game->getScore();

    // Convert score to unicode string
    touchgfx::Unicode::snprintf(scoreBuffer, 10, "%d", score);

    // Update the text area with wildcard
    textArea1.setWildcard(scoreBuffer);
    textArea1.invalidate();
}
