#ifndef _TAREA2_BOX2_HPP
#define _TAREA2_BOX2_HPP

#include <algorithm>
#include <math.h>
#include "Vector2.hpp"

namespace Tarea2
{

/**
 * 2D axis-aligned box.
 */
class Box2
{
public:
    Box2()
        : min(INFINITY, INFINITY), max(-INFINITY, -INFINITY) {}
    Box2(const Vector2 &min, const Vector2 &max)
        : min(min), max(max) {}
    Box2(float minX, float minY, float maxX, float maxY)
        : min(minX, minY), max(maxX, maxY) {}
    ~Box2() {}

    Vector2 min, max;

public:
    bool isEmpty() const
    {
        return min.x > max.x || min.y > max.y;
    }

    bool contains(const Vector2 &point) const
    {
        return min.x <= point.x && point.x <= max.x &&
               min.y <= point.y && point.y <= max.y;
    }

    bool contains(const Box2 &b) const
    {
        return min.x <= b.min.x && b.max.x <= max.x &&
               min.y <= b.min.y && b.max.y <= max.y;
    }

    bool outside(const Box2 &b) const
    {
        return b.max.x < min.x || b.min.x > max.x ||
               b.max.y < min.y || b.min.y > max.y;
    }

    bool outsideHorizontal(const Box2 &b) const
    {
        return b.max.x < min.x || b.min.x > max.x;
    }


    bool intersect(const Box2 &b) const
    {
        return !outside(b) && !contains(b);
    }

    Box2 join(const Box2 &b) const
    {
        return Box2(std::min(min.x, b.min.x), std::min(min.y, b.min.y),
                    std::max(max.x, b.max.x), std::max(max.y, b.max.y));
    }

    Box2 translate(const Vector2 &t) const
    {
        return Box2(min + t, max + t);
    }

    float height() const
    {
        return max.y - min.y;
    }

    float width() const
    {
        return max.x - min.x;
    }
};

} // namespace Tarea2

#endif //_TAREA2_BOX2_HPP

