#ifndef _TAREA2_RECTANGLE_HPP
#define _TAREA2_RECTANGLE_HPP

#include "Element.hpp"
#include "Box2.hpp"

namespace Tarea2
{
/**
 * Rectangle element.
 */
class Rectangle: public Element
{
public:
    Rectangle();
    ~Rectangle();

    Box2 getBoundingBox() const;

    const Vector2 &getExtent() const;
    void setExtent(const Vector2 &extent);

    void setHealth(int health);
    int getHealth() const;

    void draw(Renderer *renderer);

    virtual void playerTouch(Player *player, const Vector2 &direction);

private:
    void takeDamage();

    Vector2 extent;
    int health;
};

}
#endif //_TAREA2_RECTANGLE_HPP
