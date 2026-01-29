#ifndef SCREEN2PRESENTER_HPP
#define SCREEN2PRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>
#include <gui/common/SnakeGame.hpp>

using namespace touchgfx;

class Screen2View;

class Screen2Presenter : public touchgfx::Presenter, public ModelListener
{
public:
    Screen2Presenter(Screen2View &v);

    /**
     * The activate function is called automatically when this screen is "switched in"
     * (ie. made active). Initialization logic can be placed here.
     */
    virtual void activate();

    /**
     * The deactivate function is called automatically when this screen is "switched out"
     * (ie. made inactive). Teardown functionality can be placed here.
     */
    virtual void deactivate();

    virtual ~Screen2Presenter() {}

    // Get snake game from model
    SnakeGame &getSnakeGame();

    // Save score to model
    void saveScore(uint16_t score);

    // Override button pressed callback from ModelListener
    virtual void buttonPressed(SnakeDirection dir);

private:
    Screen2Presenter();

    Screen2View &view;
};

#endif // SCREEN2PRESENTER_HPP
