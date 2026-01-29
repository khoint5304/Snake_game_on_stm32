#include <gui/screen2_screen/Screen2View.hpp>
#include <gui/screen2_screen/Screen2Presenter.hpp>

Screen2Presenter::Screen2Presenter(Screen2View &v)
    : view(v)
{
}

void Screen2Presenter::activate()
{
}

void Screen2Presenter::deactivate()
{
}

SnakeGame &Screen2Presenter::getSnakeGame()
{
    return model->getSnakeGame();
}

void Screen2Presenter::saveScore(uint16_t score)
{
    model->saveGameScore(score);
}

void Screen2Presenter::buttonPressed(SnakeDirection dir)
{
    // Forward button press to view
    view.onButtonPressed(dir);
}
