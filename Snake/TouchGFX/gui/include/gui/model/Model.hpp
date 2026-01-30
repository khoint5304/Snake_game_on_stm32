#ifndef MODEL_HPP
#define MODEL_HPP

#include <gui/common/SnakeGame.hpp>
#include <gui/common/SnakeInterface.h>

class ModelListener;

class Model
{
public:
    Model();

    void bind(ModelListener *listener)
    {
        modelListener = listener;
    }

    void tick();

    // Snake game instance (shared across screens)
    SnakeGame &getSnakeGame() { return snakeGame; }

    // Button state polling (called from main.c via extern "C")
    void updateButtonStates(bool up, bool down, bool left, bool right);

    // Get current button states
    bool isButtonUpPressed() const { return buttonUp; }
    bool isButtonDownPressed() const { return buttonDown; }
    bool isButtonLeftPressed() const { return buttonLeft; }
    bool isButtonRightPressed() const { return buttonRight; }

    // Score management
    uint16_t getHighScore() const { return highScore; }
    uint16_t getLastScore() const { return lastScore; }
    void saveGameScore(uint16_t score);

    // Load high score from Flash storage (called at startup)
    void loadHighScoreFromFlash();

protected:
    ModelListener *modelListener;

private:
    SnakeGame snakeGame;

    // Button states
    bool buttonUp;
    bool buttonDown;
    bool buttonLeft;
    bool buttonRight;

    // Previous button states for edge detection
    bool prevButtonUp;
    bool prevButtonDown;
    bool prevButtonLeft;
    bool prevButtonRight;

    // Score tracking
    uint16_t highScore;
    uint16_t lastScore;
};

#endif // MODEL_HPP
