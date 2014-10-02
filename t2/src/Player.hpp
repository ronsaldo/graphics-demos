#ifndef _TAREA2_PLAYER_HPP
#define _TAREA2_PLAYER_HPP

#include "Color.hpp"
#include "Element.hpp"

namespace Tarea2
{

/**
 * Player element
 */
class Player: public Element
{
public:
    Player();
    ~Player();

    const Color &getLightColor() const;
    void setLightColor(const Color &c);

    float getLightRadius() const;
    void setLightRadius(float radius);

    float getLightIntensity() const;
    void setLightIntensity(float intensity);

    bool isOnGround() const;
    bool isOnLowerGround() const;
    void jump();
    void lost();

    virtual Box2 getBoundingBox() const;

    virtual void draw(Renderer *renderer);
    virtual void update(float delta);

private:
    Vector2 collisionSweep(const Vector2 &a, const Vector2 &b, bool *collided=NULL, Element **collisionElement=NULL);
    void updateLight(float delta);
    void rechargeLight();

    float size;
    float currentLightRadius;
    float currentLightIntensity;
    float flickerFrequency;

    float lightRadius;
    float lightRadiusSpeed;
    float lightIntensity;
    float lightIntensitySpeed;
    float passedTime;
    Color lightColor;
    bool lostFlag;
};

}

#endif //_TAREA2_PLAYER_HPP
