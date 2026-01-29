#include <gui/common/FrontendApplication.hpp>
#include <gui/common/FrontendHeap.hpp>
#include <touchgfx/transitions/NoTransition.hpp>
#include <gui/screen3_screen/Screen3View.hpp>
#include <gui/screen3_screen/Screen3Presenter.hpp>

FrontendApplication::FrontendApplication(Model &m, FrontendHeap &heap)
    : FrontendApplicationBase(m, heap),
      screen3TransitionCallback(this, &FrontendApplication::gotoScreen3ScreenNoTransitionImpl)
{
}

void FrontendApplication::gotoScreen3ScreenNoTransition()
{
    pendingScreenTransitionCallback = &screen3TransitionCallback;
}

void FrontendApplication::gotoScreen3ScreenNoTransitionImpl()
{
    touchgfx::makeTransition<Screen3View, Screen3Presenter, touchgfx::NoTransition, Model>(&currentScreen, &currentPresenter, frontendHeap, &currentTransition, &model);
}
