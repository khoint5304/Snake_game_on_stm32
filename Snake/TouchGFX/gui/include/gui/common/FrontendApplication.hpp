#ifndef FRONTENDAPPLICATION_HPP
#define FRONTENDAPPLICATION_HPP

#include <gui_generated/common/FrontendApplicationBase.hpp>
#include <touchgfx/Callback.hpp>

class FrontendHeap;

using namespace touchgfx;

class FrontendApplication : public FrontendApplicationBase
{
public:
    FrontendApplication(Model &m, FrontendHeap &heap);
    virtual ~FrontendApplication() {}

    virtual void handleTickEvent()
    {
        model.tick();
        FrontendApplicationBase::handleTickEvent();
    }

    // Go to Screen3 (Game Over screen) - no transition
    void gotoScreen3ScreenNoTransition();

private:
    void gotoScreen3ScreenNoTransitionImpl();
    touchgfx::Callback<FrontendApplication> screen3TransitionCallback;
};

#endif // FRONTENDAPPLICATION_HPP
