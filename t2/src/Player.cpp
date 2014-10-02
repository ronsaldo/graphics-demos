#include <algorithm>
#include <stdio.h>
#include "Sector.hpp"
#include "Player.hpp"
#include "Renderer.hpp"

namespace Tarea2
{
const float JumpImpulse = 30.0f;

const float LightRadiusStart = 25.0f;
const float LightRadiusMin = 3.0f;
const float LightRadiusSpeed = -5.0f;
const float LightRadiusMinSpeed = -25.0f;

const float LightRadiusLostSpeed = -40.0f;

const float LightIntensityStart = 1.0f;
const float LightIntensityMin = 0.01f;
const float LightIntensityMinSpeed = -1.0;
const float LightIntensitySpeed = -0.01;

const float FlickerRadius = 1.0;
const float FlickerFrequency = M_PI*2*4;
const float FlickerIntensityRadius = 0.1;

const float RechargeRadiusSpeedIncrement = -0.5f;
const float RechargeIntensitySpeedIncrement = -0.01f;

Player::Player()
{
    currentLightRadius = lightRadius = LightRadiusStart;
    currentLightIntensity = lightIntensity = LightIntensityStart;
    lightRadiusSpeed = LightRadiusSpeed;
    flickerFrequency = FlickerFrequency;

    lightColor = Color::white();
    lightIntensitySpeed = LightIntensitySpeed;

    size = 1.0f;
    passedTime = 0.0f;
    setBlocking(false);
    lostFlag = false;
}

Player::~Player()
{
}

Box2 Player::getBoundingBox() const
{
    return Box2(-size, -size, size, size);
}

const Color &Player::getLightColor() const
{
    return this->lightColor;
}

void Player::setLightColor(const Color &c)
{
    this->lightColor = c;
}

float Player::getLightRadius() const
{
    return this->lightRadius;
}

void Player::setLightRadius(float radius)
{
    this->currentLightRadius = this->lightRadius = radius;
}

float Player::getLightIntensity() const
{
    return this->lightIntensity;
}

void Player::setLightIntensity(float intensity)
{
    this->currentLightIntensity = this->lightIntensity = intensity;
}

void Player::draw(Renderer *renderer)
{
    Color color = getLightColor();

    SimpleMeshBuilder *builder = renderer->getNoShadowBuilder();
    builder->setTransform(getTransform());

    builder->addVertex(Vector2(-size, -size), color, Vector2::zero());
    builder->addVertex(Vector2(0.0f, size), color, Vector2::zero());
    builder->addVertex(Vector2(size, -size), color, Vector2::zero());
    builder->addIndex(0);
    builder->addIndex(1);
    builder->addIndex(2);
    builder->endTriangles();
}

bool Player::isOnGround() const
{
    const float TestMargin = 0.5f;
    Sector *sector = getSector();
    Box2 bbox = getBoundingBox();
    return sector->isBlockedBox(bbox.translate(getPosition() + Vector2(0.0, -TestMargin)));
}

bool Player::isOnLowerGround() const
{
    Element *element;
    const float TestMargin = 0.5f;
    Sector *sector = getSector();
    Box2 bbox = getBoundingBox();
    return sector->isBlockedBox(bbox.translate(getPosition() + Vector2(0.0, -TestMargin)), &element) && element == getSector();

}

void Player::jump()
{
    Vector2 velocity = getVelocity();
    if(isOnGround() && velocity.y < 0)
    {
        velocity.y = JumpImpulse;
        rechargeLight();
        getSector()->started();
    }
    setVelocity(velocity);
}

void Player::lost()
{
    lostFlag = true;
}

Vector2 Player::collisionSweep(const Vector2 &a, const Vector2 &b, bool *collided, Element **collisionElement)
{
    // Set the initial results.
    if(collided)
        *collided = false;
    if(collisionElement)
        *collisionElement = NULL;

    // Avoid performing the sweep.
    if(a.closeTo(b))
        return a;

    // Get the bounding box and the sector.
    Box2 bbox = getBoundingBox();
    Sector *sector = getSector();

    // Compute the result and the steps.
    const int NumSteps = 10;
    Vector2 step = (b - a)/NumSteps;
    Vector2 res = a;

    // Perform incremental collision tracing.
    for(int i = 0; i < NumSteps; ++i, res = res + step)
    {
        if(sector->isBlockedBox(bbox.translate(res), collisionElement))
        {
            if(collided)
                *collided = true;
            res = res - step;
            break;
        }
    }

    return res;
}

void Player::update(float delta)
{
    // Use the sector for collision detection.
    Sector *sector = getSector();
    Box2 bbox = getBoundingBox();

    // Crappy collision detection.
    Vector2 position = getPosition();
    Vector2 velocity = getVelocity();
    Vector2 gravity = sector->getGravity();

    passedTime += delta;
    if(lostFlag)
    {
        lightRadius += LightRadiusLostSpeed*delta;
        if(lightRadius < 0.0f)
            lightRadius = 0.0f;
        currentLightRadius = lightRadius;
        return;
    }

    // Update the light;
    updateLight(delta);

    // Apply the gravity.
    velocity = velocity + gravity*delta;

    // Move horizontally.
    Element *horizontalElement;
    bool horizontalCollision;
    Vector2 newPosition = position + Vector2(velocity.x, 0.0f)*delta;
    position = collisionSweep(position, newPosition, &horizontalCollision, &horizontalElement);

    // Move vertically.
    Element *verticalElement;
    bool verticalCollision;
    newPosition = position + Vector2(0.0f, velocity.y)*delta;
    position = collisionSweep(position, newPosition, &verticalCollision, &verticalElement);

    // Set the new position and velocity.
    setVelocity(velocity);
    setPosition(position);

    // Dispatch collision events.
    if(horizontalElement && horizontalCollision)
        horizontalElement->playerTouch(this, Vector2(velocity.x, 0.0f).normalized());

    // Dispatch collision events.
    if(verticalElement && verticalCollision)
        verticalElement->playerTouch(this, Vector2(0.0f, velocity.y).normalized());

    // Notify the sector.
    sector->changedPlayerPosition(getPosition());
}

void Player::updateLight(float delta)
{
    if(!getSector()->isStarted())
        return;

    // Light radius.
    currentLightRadius = std::max(LightRadiusMin, currentLightRadius + lightRadiusSpeed*delta);

    // Radius flickering.
    lightRadius = std::max((double)LightRadiusMin, currentLightRadius + FlickerRadius*sin(flickerFrequency*passedTime));

    // Light intensity
    currentLightIntensity = std::max(LightIntensityMin, currentLightIntensity + lightIntensitySpeed*delta);

    // Intensity flickering.
    lightIntensity = currentLightIntensity*(1.0 - FlickerIntensityRadius) +
                     FlickerIntensityRadius*sin(flickerFrequency*passedTime);
    lightIntensity = std::max(LightIntensityMin, lightIntensity);
}

void Player::rechargeLight()
{
    setLightRadius(LightRadiusStart);
    setLightIntensity(LightIntensityStart);

    lightRadiusSpeed = std::max(LightRadiusMinSpeed, lightRadiusSpeed + RechargeRadiusSpeedIncrement);
    lightIntensitySpeed = std::max(LightIntensityMinSpeed, lightIntensitySpeed + RechargeIntensitySpeedIncrement);
    
}

}

