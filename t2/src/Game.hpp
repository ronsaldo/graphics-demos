#ifndef _TAREA2_GAME_HPP
#define _TAREA2_GAME_HPP

#include "EventListener.hpp"
#include "Renderer.hpp"
#include "InputSystem.hpp"
#include "Player.hpp"
#include "Sector.hpp"
#include "System.hpp"

namespace Tarea2
{

/**
 * Game manager.
 */
class Game: EventListener
{
public:
    Game();
    ~Game();

    bool initialize();
    void restart();

    bool isGameOver() const;
    void gameOver();

    int run(int argc, const char *argv[]);

    virtual void handleEvent(Event *event);

    System &getSystem(); 
    Sector *getSector();
    Player *getPlayer();
    Renderer *getRenderer();

private:
    System system;
    InputSystem inputSystem;
    Renderer renderer;
    Sector *sector;
    Player *player;
    bool gameOverFlag;
};

} // namespace Tarea2

#endif //_TAREA2_GAME_HPP
