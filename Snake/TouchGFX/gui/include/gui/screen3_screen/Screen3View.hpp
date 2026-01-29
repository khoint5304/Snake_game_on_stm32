#ifndef SCREEN3VIEW_HPP
#define SCREEN3VIEW_HPP

#include <gui_generated/screen3_screen/Screen3ViewBase.hpp>
#include <gui/screen3_screen/Screen3Presenter.hpp>

class Screen3View : public Screen3ViewBase
{
public:
    Screen3View();
    virtual ~Screen3View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

protected:
    // Update score displays
    void updateScoreDisplay();

private:
    // Score buffers for wildcard text
    touchgfx::Unicode::UnicodeChar scoreBuffer[10];
    touchgfx::Unicode::UnicodeChar highScoreBuffer[10];
};

#endif // SCREEN3VIEW_HPP
