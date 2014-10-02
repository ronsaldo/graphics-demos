#ifndef _TAREA2_ELEMENT_HPP_
#define _TAREA2_ELEMENT_HPP_

#include "Box2.hpp"
#include "Vector2.hpp"
#include "Color.hpp"
#include "Matrix3.hpp"

namespace Tarea2
{
class Sector;
class Renderer;
class Player;

/**
 * Element in the plane.
 */
class Element
{
public:
    Element();
    virtual ~Element();

    Sector *getSector() const;
    void setSector(Sector *sector);

    bool isBlocking() const;
    void setBlocking(bool blocking);

    bool isBlockingBox(const Box2 &box) const;

    bool isPlayerSupport() const;
    void setPlayerSupport(bool support);

    bool isBackground() const;
    void setBackground(bool background);

    const Color &getColor() const;
    void setColor(const Color &color);

    const Vector2 &getPosition() const;
    void setPosition(const Vector2 &position);

    const Vector2 &getVelocity();
    void setVelocity(const Vector2 &velocity);

    Matrix3 getTransform();

    virtual Box2 getBoundingBox() const;

    virtual void draw(Renderer *renderer);
    virtual void update(float delta);

    virtual void playerTouch(Player *player, const Vector2 &direction);

private:
    bool background;
    bool blocking;
    bool playerSupport;

    Color color;
    Vector2 position;
    Vector2 velocity;

    Sector *sector;
};
} // namespace Tarea2

#endif //_TAREA2_ELEMENT_HPP_
