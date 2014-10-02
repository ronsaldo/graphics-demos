#include "Element.hpp"

namespace Tarea2
{
Element::Element()
{
    blocking = false;
    background = false;
    playerSupport = false;
}

Element::~Element()
{
}

Sector *Element::getSector() const
{
    return this->sector;
}

void Element::setSector(Sector *sector)
{
    this->sector = sector;
}

bool Element::isBlocking() const
{
    return this->blocking;
}

bool Element::isPlayerSupport() const
{
    return this->playerSupport;
}

void Element::setPlayerSupport(bool support)
{
    this->playerSupport = support;
}

bool Element::isBlockingBox(const Box2 &box) const
{
    Box2 bbox = getBoundingBox().translate(getPosition());
    return !box.outside(bbox);
}

void Element::setBlocking(bool blocking)
{
    this->blocking = blocking;
}

bool Element::isBackground() const
{
    return this->background;
}

void Element::setBackground(bool background)
{
    this->background = background;
}

const Color &Element::getColor() const
{
    return this->color;
}

void Element::setColor(const Color &color)
{
    this->color = color;
}

const Vector2 &Element::getPosition() const
{
    return this->position;
}

void Element::setPosition(const Vector2 &position)
{
    this->position = position;
}

const Vector2 &Element::getVelocity()
{
    return this->velocity;
}

void Element::setVelocity(const Vector2 &velocity)
{
    this->velocity = velocity;
}

Box2 Element::getBoundingBox() const
{
    return Box2();
}

Matrix3 Element::getTransform()
{
    return Matrix3::translation(position);
}

void Element::draw(Renderer *renderer)
{
}

void Element::update(float delta)
{
    setPosition(getPosition() + velocity*delta);
}

void Element::playerTouch(Player *player, const Vector2 &direction)
{
}

} // namespace Tarea2
