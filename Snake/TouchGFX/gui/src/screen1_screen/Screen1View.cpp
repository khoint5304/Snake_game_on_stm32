#include <gui/screen1_screen/Screen1View.hpp>
#include <touchgfx/Unicode.hpp>
#include <touchgfx/Color.hpp>

Screen1View::Screen1View()
{
}

void Screen1View::setupScreen()
{
    Screen1ViewBase::setupScreen();

    // Update difficulty display on screen entry
    updateDifficultyDisplay();
}

void Screen1View::tearDownScreen()
{
    Screen1ViewBase::tearDownScreen();
}

void Screen1View::Change_Snake_Speed()
{
    // Cycle through difficulty levels
    presenter->cycleDifficulty();

    // Update the difficulty display text
    updateDifficultyDisplay();
}

void Screen1View::updateDifficultyDisplay()
{
    // Get current difficulty and update the text display
    Difficulty diff = presenter->getDifficulty();

    // Update wildcard text and color based on difficulty
    switch (diff)
    {
    case EASY:
        touchgfx::Unicode::strncpy(difficultyBuffer, "EASY", 15);
        textArea1.setColor(touchgfx::Color::getColorFromRGB(0, 255, 0)); // Green
        break;
    case NORMAL:
        touchgfx::Unicode::strncpy(difficultyBuffer, "NORMAL", 15);
        textArea1.setColor(touchgfx::Color::getColorFromRGB(255, 255, 0)); // Yellow
        break;
    case HARD:
        touchgfx::Unicode::strncpy(difficultyBuffer, "HARD", 15);
        textArea1.setColor(touchgfx::Color::getColorFromRGB(255, 0, 0)); // Red
        break;
    case INSANE:
        touchgfx::Unicode::strncpy(difficultyBuffer, "INSANE", 15);
        textArea1.setColor(touchgfx::Color::getColorFromRGB(255, 0, 255)); // Magenta
        break;
    case NIGHTMARE:
        touchgfx::Unicode::strncpy(difficultyBuffer, "NIGHTMARE", 15);
        textArea1.setColor(touchgfx::Color::getColorFromRGB(128, 0, 128)); // Purple
        break;
    }

    textArea1.setWildcard(difficultyBuffer);
    textArea1.invalidate();
}
