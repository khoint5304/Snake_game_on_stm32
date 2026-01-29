#ifndef MODELLISTENER_HPP
#define MODELLISTENER_HPP

#include <gui/model/Model.hpp>
#include <gui/common/SnakeGame.hpp>

class ModelListener
{
public:
    ModelListener() : model(0) {}

    virtual ~ModelListener() {}

    void bind(Model *m)
    {
        model = m;
    }

    // Called when a button is pressed (edge detection)
    virtual void buttonPressed(SnakeDirection dir) {}

protected:
    Model *model;
};

#endif // MODELLISTENER_HPP
