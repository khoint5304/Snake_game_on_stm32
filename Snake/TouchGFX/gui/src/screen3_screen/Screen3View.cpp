#include <gui/screen3_screen/Screen3View.hpp>
#include <touchgfx/Unicode.hpp>

Screen3View::Screen3View()
{
    scoreBuffer[0] = '0';
    scoreBuffer[1] = 0;
    highScoreBuffer[0] = '0';
    highScoreBuffer[1] = 0;
}

void Screen3View::setupScreen()
{
    Screen3ViewBase::setupScreen();

    // Update score displays when screen is shown
    updateScoreDisplay();
}

void Screen3View::tearDownScreen()
{
    Screen3ViewBase::tearDownScreen();
}

void Screen3View::updateScoreDisplay()
{
    // Get scores from presenter
    uint16_t lastScore = presenter->getLastScore();
    uint16_t highScore = presenter->getHighScore();

    // Update score buffer
    touchgfx::Unicode::snprintf(scoreBuffer, 10, "%d", lastScore);
    textArea_Score.setWildcard(scoreBuffer);
    textArea_Score.invalidate();

    // Update high score buffer
    touchgfx::Unicode::snprintf(highScoreBuffer, 10, "%d", highScore);
    textArea_Highscore.setWildcard(highScoreBuffer);
    textArea_Highscore.invalidate();
}
