#include <stdio.h>
#include "Game.hpp"

namespace Tarea2
{
Game::Game()
    : inputSystem(this), renderer(this), sector(NULL), player(NULL)
{
}

Game::~Game()
{
}

bool Game::initialize()
{
    // Set the title.
    system.setTitle("Tarea 2 - Computacion Grafica");

    // Register myself.
    system.registerListener(this);

    // Initialize the input system.
    if(!inputSystem.initialize())
        return false;

    // Initialize the renderer.
    if(!renderer.initialize())
        return false;

    return true;
}

void Game::restart()
{
    // Delete the old sector.
    delete sector;
    sector = new Sector(this);
    sector->generateBlocks();

    // Add the player.
    player = new Player();
    sector->addPlayer(player);

    // Add the background.
    sector->addBackground();

    // Reset the game over flag.
    gameOverFlag = false;
}

bool Game::isGameOver() const
{
    return gameOverFlag;
}

void Game::gameOver()
{
    // Set the game over flag.
    gameOverFlag = true;

    // Notify the lost.
    player->lost();
}

int Game::run(int argc, const char *argv[])
{
    // Initialize the system.
    if(!system.initialize(argc, argv))
        return -1;

    // Iinitialize the game.
    if(!initialize())
        return -1;

    // Restart the game
    restart();

    // Run the application.
    system.mainloop();
    system.shutdown();

    return 0;
}

void Game::handleEvent(Event *event)
{
    switch(event->getType())
    {
    case Event::FrameUpdate:
        {
            FrameUpdateEvent *update = static_cast<FrameUpdateEvent*> (event);
            inputSystem.update(update->getDelta());
            if(sector)
                sector->update(update->getDelta());
        }
        break;
    default:
        //Do nothing.
        break;
    }
}

System &Game::getSystem()
{
    return system;
}

Sector *Game::getSector()
{
    return sector;
}

Player *Game::getPlayer()
{
    return player;
}

Renderer *Game::getRenderer()
{
    return &renderer;
}
}
