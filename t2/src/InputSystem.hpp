#ifndef _TAREA2_INPUT_SYSTEM_HPP
#define _TAREA2_INPUT_SYSTEM_HPP

#include "EventListener.hpp"

namespace Tarea2
{
class Game;

/**
 * Input handling system.
 */
class InputSystem: public EventListener
{
public:
    InputSystem(Game *game);
    ~InputSystem();

    bool initialize();

    void update(float delta);

    virtual void handleEvent(Event *event);

private:
    void onMouseMove(MouseMoveEvent *event);
    void onKeyDown(KeyEvent *event);
    void onKeyUp(KeyEvent *event);

    bool mouseControl;

    Game *game;
    int mouseX, mouseY;
};

}

#endif //_TAREA2_INPUT_SYSTEM_HPP
