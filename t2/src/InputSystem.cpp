#include <stdio.h>
#include <SDL/SDL_keysym.h>
#include "InputSystem.hpp"
#include "Game.hpp"

namespace Tarea2
{
const float PlayerSpeed = 20.0f;

InputSystem::InputSystem(Game *game)
    : game(game)
{
    mouseControl = false;
}

InputSystem::~InputSystem()
{
}

bool InputSystem::initialize()
{
    game->getSystem().registerListener(this);
    return true;
}

void InputSystem::handleEvent(Event *event)
{
    switch(event->getType())
    {
    case Event::KeyUp:
        onKeyUp(static_cast<KeyEvent*> (event));
        break;
    case Event::KeyDown:
        onKeyDown(static_cast<KeyEvent*> (event));
        break;
    case Event::MouseLeave:
        mouseControl = false;
        {
            Player *player = game->getPlayer();
            if(player)
            {
                Vector2 velocity = player->getVelocity();
                velocity.x = 0.0f;
                player->setVelocity(velocity);
            }
        }
        break;
    case Event::MouseMove:
        onMouseMove(static_cast<MouseMoveEvent*> (event));
        break;
    default:
        // Do nothing
        break;
    }
}

void InputSystem::update(float delta)
{
    if(!mouseControl)
        return;

    Player *player = game->getPlayer();
    if(!player)
        return;

    // Compute the view position.
    Vector2 viewPosition = game->getRenderer()->windowToView(mouseX, mouseY);
    float dir = viewPosition.x - player->getPosition().x;

    // Compute the new velocity.
    const float DirectionMargin = 0.5f;
    Vector2 velocity = player->getVelocity();
    if(dir > DirectionMargin)
        velocity.x = PlayerSpeed;
    else if(dir < -DirectionMargin)
        velocity.x = -PlayerSpeed;
    else
        velocity.x = 0.0f;
    player->setVelocity(velocity);
}

void InputSystem::onMouseMove(MouseMoveEvent *event)
{
    mouseControl = true;
    mouseX = event->getX(); mouseY = event->getY();
}

void InputSystem::onKeyDown(KeyEvent *event)
{
    Player *player = game->getPlayer();
    if(!player)
        return;

    Vector2 velocity = player->getVelocity();
    bool jump = false;

    switch(event->getKeyCode())
    {
    case SDLK_SPACE:
        // Restart on game over.
        if(game->isGameOver())
        {
            game->restart();
            return;
        }
        if(player->isOnLowerGround())
            jump = true;
        break;
    case SDLK_LEFT:
        velocity.x = -PlayerSpeed;
        break;
    case SDLK_RIGHT:
        velocity.x = PlayerSpeed;
        break;
    default:
        break;
    }

    if(!mouseControl)
        player->setVelocity(velocity);
    if(jump)
        player->jump();
}

void InputSystem::onKeyUp(KeyEvent *event)
{
    Player *player = game->getPlayer();
    if(!player)
        return;

    Vector2 velocity = player->getVelocity();
    switch(event->getKeyCode())
    {
    case SDLK_LEFT:
        if(velocity.x < 0.0f)
            velocity.x = 0.0f;
        break;
    case SDLK_RIGHT:
        if(velocity.x > 0.0)
            velocity.x = 0.0f;
        break;
    default:
        break;
    }

    if(!mouseControl)
        player->setVelocity(velocity);
}


}

