#ifndef SCREEN1VIEW_HPP
#define SCREEN1VIEW_HPP

#include <gui_generated/screen1_screen/Screen1ViewBase.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>
#include <touchgfx/Unicode.hpp>

class Screen1View : public Screen1ViewBase
{
public:
    Screen1View();
    virtual ~Screen1View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

    // Override the virtual function for changing difficulty
    virtual void Change_Snake_Speed();

protected:
    // Update difficulty display (wildcard text)
    void updateDifficultyDisplay();

private:
    // Buffer for difficulty text wildcard (max: "NIGHTMARE" = 9 chars + null)
    touchgfx::Unicode::UnicodeChar difficultyBuffer[15];
};

#endif // SCREEN1VIEW_HPP
