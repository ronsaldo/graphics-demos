#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <algorithm>
#include "Sector.hpp"
#include "Rectangle.hpp"
#include "Player.hpp"
#include "Game.hpp"

namespace Tarea2
{

const float Gravity = -30.0f;

const float HorizontalBound = 25.0f;
const float VerticalBound = 20.0f;

const float GroundHeight = -20.0f;
const float PlayerMargin = 0.01f;
const float BoundMargin = 0.05f;

const float BlockHeight = 2.f;

const float MinHeight = 7.0f;
const float MaxHeight = 10.0f;
const float HeightRangeFrequency = 0.02f;

const float MinWidth = 6.0f;
const float MaxWidth = 12.0f;
const float WidthRangeFrequency = 0.01f;

const int MinBlocksPerLevel = 1;
const int MaxBlocksPerLevel = 3;

const float MaxAllowedHorizontalDelta = 15;

const float AmbientIntensityStart = 0.4;
const float AmbientIntensitySpeed = -0.002;

const int NumBackgroundColors = 6;
const Color BackgroundColors[] = {
    Color(1.0, 0.4, 0.0, 1.0), // Red
    Color(0.4, 1.0, 0.4, 1.0), // Green
    Color(0.4, 0.4, 1.0, 1.0), // Blue
    Color(1.0, 1.0, 1.0, 1.0), // Yellow
    Color(1.0, 0.4, 1.0, 1.0), // Magenta
    Color(0.4, 1.0, 1.0, 1.0), // Cyan
};

const float BackgroundFadeSpeed = 0.2f;

const int StartHealth = 4;
const int HealthDivider = 300;

Sector::Sector(Game *game)
    : game(game)
{
    srand(time(NULL));
    ambientLight = Color::white();
    ambientIntensity = AmbientIntensityStart;
    gravity = Vector2(0.0, Gravity);
    currentHeight = 0.0f;
    lastAddedHeight = GroundHeight;
    gameStarted = false;
    background = NULL;
    computeDifficulty();
}

Sector::~Sector()
{
    clear();
}

Game *Sector::getGame()
{
    return game;
}

const Color &Sector::getAmbientLight() const
{
    return this->ambientLight;
}

void Sector::setAmbientLight(const Color &color)
{
    this->ambientLight = color;
}

float Sector::getAmbientIntensity() const
{
    return this->ambientIntensity;
}

void Sector::setAmbientIntensity(float intensity)
{
    this->ambientIntensity = intensity;
}

const Vector2 &Sector::getGravity() const
{
    return this->gravity;
}

void Sector::setGravity(const Vector2 &gravity)
{
    this->gravity = gravity;
}

float Sector::generateScalar()
{
    return (float)rand()/(float)RAND_MAX;
}

float Sector::generateRange(float xmin, float xmax)
{
    return (xmax - xmin)*generateScalar() + xmin;
}

int Sector::generateIntRange(int min, int max)
{
    return rand()%(max - min) + min;
}

Vector2 Sector::generatePositionAtHeight(float y)
{
    return Vector2(generateRange(-20.0f, 20.0f), y);
}

Rectangle *Sector::generateRectangleAtHeight(float y)
{
    Rectangle *rect = new Rectangle();
    rect->setBlocking(true);
    rect->setPosition(generatePositionAtHeight(y));
    rect->setExtent(Vector2(generateRange(MinWidth, MinWidth + widthRange), BlockHeight));
    rect->setHealth(std::max(1, StartHealth - (int)(y/HealthDivider)));
    return rect;
}

bool Sector::collidesWithGround(const Box2 &box)
{
    return box.min.y <= GroundHeight + BoundMargin*2.0f;
}

static bool overlaps(Element *element, const std::vector<Element*> &elements)
{
    Box2 elementBbox = element->getBoundingBox().translate(element->getPosition());
    for(size_t i = 0; i < elements.size(); ++i)
    {
        Element *testElement = elements[i];
        Box2 bbox = testElement->getBoundingBox().translate(testElement->getPosition());
        if(!elementBbox.outsideHorizontal(bbox))
            return true;
    }
    return false;
}

static void enforceMaxDistance(Element *element, const std::vector<Element*> &elements)
{
    float minDist = 0.0f;
    float sign;
    Element *minElement = NULL;
    for(size_t i = 0; i < elements.size(); ++i)
    {
        Element *e = elements[i];
        float dist = element->getPosition().x - e->getPosition().x;
        if(dist >= 0.0f)
        {
            sign = 1.0f;
        }
        else
        {
            dist = -dist;
            sign = -1.0f;
        }

        if(minElement == NULL || dist < minDist)
        {
            minDist = dist;
            minElement = e;
        }
    }

    if(minElement != NULL && minDist > MaxAllowedHorizontalDelta)
    {
        Vector2 position = element->getPosition();
        position.x = minElement->getPosition().x + sign*MaxAllowedHorizontalDelta;

        // Make sure its in bounds.
        Box2 bbox = element->getBoundingBox().translate(position);
        if(bbox.max.x > HorizontalBound)
            position.x -= bbox.max.x - HorizontalBound;
        else if(bbox.min.x < -HorizontalBound)
            position.x += -HorizontalBound - bbox.min.x;
        element->setPosition(position);
    }
}

void Sector::generateBlocks()
{
    std::vector<Element*> floorElements;

    while(lastAddedHeight <= getUpperBound() + VerticalBound)
    {
        // Compute the new height.
        float delta = generateRange(MinHeight, MinHeight + heightRange);
        float newHeight = lastAddedHeight + delta;

        // Spawn the blocks.
        int numBlocks = generateIntRange(MinBlocksPerLevel, MaxBlocksPerLevel);
        floorElements.clear();
        for(int i = 0; i < numBlocks; ++i)
        {
            Element *element = generateRectangleAtHeight(newHeight);

            // Prevent overlapping with the elements in this floor and the last floor.
            bool fixed = false;
            for(int j = 0; j < 10; ++j)
            {
                if(!overlaps(element, floorElements) &&
                   !overlaps(element, lastFloorElements))
                {
                    fixed = true;
                    break;
                }
                element->setPosition(generatePositionAtHeight(newHeight));
            }

            // Prevent pathological case.
            if(i == 0)
            {
                // Make sure its added
                if(!fixed)
                {
                    element->setPosition(Vector2(0, newHeight));
                    fixed = true;
                }
            
                // Fix the horizontal distance.
                enforceMaxDistance(element, lastFloorElements);
            }

            // Add the element if fixed.
            if(fixed)
            {
                floorElements.push_back(element);
                addElement(element);
            }
            else
            {
                delete element;
            }
        }

        // Store the new height.
        lastAddedHeight = newHeight;
        lastFloorElements = floorElements;
    }
}

bool Sector::isBlockedBox(const Box2 &box, Element **collisionElement)
{
    if(collisionElement)
        *collisionElement = NULL;

    // Check the world bounds.
    if(box.min.x <= -HorizontalBound || box.max.x >= HorizontalBound)
    {
        if(collisionElement)
            *collisionElement = this;
        return true;
    }

    // Check the lower bound.
    if(box.min.y <= GroundHeight || box.max.y <= getLowerBound())
    {
        if(collisionElement)
            *collisionElement = this;
        return true;
    }

    // Check collisions with other elements.
    Elements::const_iterator it = elements.begin();
    for(; it != elements.end(); ++it)
    {
        Element *element = *it;
        if(element->isBlocking() && element->isBlockingBox(box))
        {
            if(collisionElement)
                *collisionElement = element;
            return true;
        }
    }

    return false;
}

void Sector::destroyBlocks()
{
    Elements::iterator it = elements.begin();
    while(it != elements.end())
    {
        Element *element = *it;
        Box2 bbox = element->getBoundingBox().translate(element->getPosition());
        if(!element->isBackground() && bbox.max.y <= getLowerBound() - BoundMargin)
            elements.erase(it++);
        else
            it++;
    }
}

void Sector::addElement(Element *element)
{
    element->setSector(this);
    elements.push_back(element);
}

void Sector::addBackground()
{
    Rectangle *rect = new Rectangle();
    rect->setBackground(true);
    rect->setPosition(Vector2::zero());
    rect->setExtent(Vector2(50, 100));
    rect->setColor(BackgroundColors[0]);
    addElement(rect);

    // Background color changing.
    background = rect;
    backgroundColorStart = BackgroundColors[rand()%NumBackgroundColors];
    backgroundColorNext = nextBackgroundColor();
    backgroundFade = 0.0f;
}

void Sector::addPlayer(Element *element)
{
    addElement(element);
    Box2 bbox = element->getBoundingBox();
    element->setPosition(Vector2(0.0f, GroundHeight + bbox.height()*0.5f + PlayerMargin));
}

void Sector::destroyElement(Element *element)
{
    Elements::iterator it = elements.begin();
    for(; it != elements.end(); ++it)
    {
        if(*it == element)
        {
            elements.erase(it);
            break;
        }
    }

    delete element;
}

void Sector::clear()
{
    Elements::iterator it = elements.begin();
    for(; it != elements.end(); ++it)
        delete *it;
    elements.clear();
}

void Sector::update(float delta)
{
    // Update the elements.
    Elements::iterator it = elements.begin();
    for(; it != elements.end(); ++it)
        (*it)->update(delta);

    // Update the background.
    updateBackground(delta);
}

void Sector::draw(Renderer *renderer)
{
    Elements::iterator it = elements.begin();
    for(; it != elements.end(); ++it)
        (*it)->draw(renderer);
}

Matrix4 Sector::getCameraTransform() const
{
    return Matrix4::translation(Vector2(0.0f, -currentHeight));
}

float Sector::getLowerBound() const
{
    return currentHeight - VerticalBound;
}

float Sector::getUpperBound() const
{
    return currentHeight + VerticalBound;
}

void Sector::changedPlayerPosition(const Vector2 &position)
{
    if(position.y <= currentHeight)
        return;

    // HACK: Translate background.
    float delta = position.y - currentHeight;
    Elements::iterator it = elements.begin();
    for(; it != elements.end(); ++it)
    {
        Element *element = *it;
        if(element->isBackground())
            element->setPosition(element->getPosition() + Vector2(0.0f, delta));
    }

    // Attenuate the ambient lighting.
    ambientIntensity += AmbientIntensitySpeed*delta;
    if(ambientIntensity < 0)
        ambientIntensity = 0.0f;

    // Set the new height.
    currentHeight = position.y;
    computeDifficulty();
    destroyBlocks();
    generateBlocks();
}

bool Sector::isStarted() const
{
    return gameStarted;
}

void Sector::started()
{
    gameStarted = true;
}

float Sector::computeMaxHeight() const
{
    float res = GroundHeight;
    Elements::const_iterator it = elements.begin();
    for(; it != elements.end(); ++it)
    {
        Element *element = *it;
        if(element->isPlayerSupport())
            res = std::max(res, element->getPosition().y);
    }

    return res;
}

void Sector::playerTouch(Player *player, const Vector2 &direction)
{
    if(direction.y >= 0.0f)
        return;

    Box2 bbox = player->getBoundingBox().translate(player->getPosition());
    if(!collidesWithGround(bbox))
    {
        game->gameOver();
        ambientLight = Color(1.0f, 0.3f, 0.3f, 1.0f);
        ambientIntensity = 0.5f;
    }
}

float Sector::oscillatingRange(float min, float max, float freq, float x)
{
    return (max - min)*cos(freq*x);
}

void Sector::computeDifficulty()
{
    heightRange = oscillatingRange(MinHeight, MaxHeight, HeightRangeFrequency, currentHeight);
    widthRange = oscillatingRange(MinWidth, MaxWidth, WidthRangeFrequency, currentHeight);
}

Color Sector::nextBackgroundColor() const
{
    Color res;
    do
    {
        res = BackgroundColors[rand() % NumBackgroundColors];
    } while(res.closeTo(backgroundColorStart));
    return res;
}

void Sector::updateBackground(float delta)
{
    // Compute the fade factor.
    backgroundFade += BackgroundFadeSpeed*delta;

    // Use the next fade color when appropiate.
    if(backgroundFade >= 1.0f)
    {
        backgroundColorStart = backgroundColorNext;
        backgroundColorNext = nextBackgroundColor();
        backgroundFade = 0.0f;
    }

    // Set the background.
    if(background)
        background->setColor(lerpColor(backgroundFade, backgroundColorStart, backgroundColorNext));

}

}

