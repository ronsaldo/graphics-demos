#include <stdio.h>
#include "Rectangle.hpp"
#include "Renderer.hpp"
#include "Sector.hpp"
#include "Player.hpp"

namespace Tarea2
{

const Color HealthColors[] = {
    Color::red(),
    Color::yellow(),
    Color::cyan(),
    Color::blue(),
};

const int StartHealth = 4;

Rectangle::Rectangle()
{
    health = StartHealth;
}

Rectangle::~Rectangle()
{
}

Box2 Rectangle::getBoundingBox() const
{
    return Box2(getExtent()*-0.5f, getExtent()*0.5f);
}

const Vector2 &Rectangle::getExtent() const
{
    return this->extent;
}

void Rectangle::setExtent(const Vector2 &extent)
{
    this->extent = extent;
}

void Rectangle::setHealth(int health)
{
    this->health = health;
    if(health == 0)
    {
        getSector()->destroyElement(this);
    }
    else
    {
        setColor(HealthColors[health-1]);
    }
}

int Rectangle::getHealth() const
{
    return this->health;
}

void Rectangle::draw(Renderer *renderer)
{
    Vector2 corners[] = {
        Vector2(-extent.x*0.5f, -extent.y*0.5f),
        Vector2(-extent.x*0.5f, extent.y*0.5f),
        Vector2(extent.x*0.5f, extent.y*0.5f),
        Vector2(extent.x*0.5f, -extent.y*0.5f),
    };

    Vector2 normals[] = {
        Vector2(-1.0f, 0.0f), // Left
        Vector2(0.0f, 1.0f), // Top
        Vector2(1.0f, 0.0f), // Right
        Vector2(0.0f, -1.0f), // Bottom
    };

    Color color = getColor();

    SimpleMeshBuilder *builder = NULL;
    if(isBackground())
        builder = renderer->getBackgroundBuilder();
    else
        builder = renderer->getShadowBuilder();
    builder->setTransform(getTransform());

    // Left
    builder->addVertex(corners[0], color, normals[0]);
    builder->addVertex(corners[1], color, normals[0]);

    // Top
    builder->addVertex(corners[1], color, normals[1]);
    builder->addVertex(corners[2], color, normals[1]);

    // Right
    builder->addVertex(corners[2], color, normals[2]);
    builder->addVertex(corners[3], color, normals[2]);

    // Bottom
    builder->addVertex(corners[3], color, normals[3]);
    builder->addVertex(corners[0], color, normals[3]);

    // Add the triangle fan.
    for(int i = 2; i < 8; ++i)
    {
        builder->addIndex(0);
        builder->addIndex(i-1);
        builder->addIndex(i);
    }
    builder->endTriangles();
}

void Rectangle::playerTouch(Player *player, const Vector2 &direction)
{
    if(!isBlocking())
        return;

    if(direction.y > 0.0f)
    {
        // Elastic bounce.
        Vector2 velocity = player->getVelocity();
        if(health > 1)
            velocity.y = -velocity.y;
        player->setVelocity(velocity);
        takeDamage();
    }
    else if(direction.y < 0.0f)
    {
        player->jump();
        takeDamage();
    }
}

void Rectangle::takeDamage()
{
    health--;
    setHealth(health);
}

} // namespace Tarea2
