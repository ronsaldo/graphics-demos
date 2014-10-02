#ifndef _TAREA2_SECTOR_HPP
#define _TAREA2_SECTOR_HPP

#include <list>
#include <vector>
#include "Color.hpp"
#include "Element.hpp"
#include "Matrix4.hpp"

namespace Tarea2
{
class Renderer;
class Rectangle;
class Game;

/**
 * Sector.
 */
class Sector: public Element
{
public:
    Sector(Game *game);
    ~Sector();

    Game *getGame();

    const Color &getAmbientLight() const;
    void setAmbientLight(const Color &color);

    float getAmbientIntensity() const;
    void setAmbientIntensity(float intensity);

    const Vector2 &getGravity() const;
    void setGravity(const Vector2 &gravity);

    bool isBlockedBox(const Box2 &box, Element **collisionElement = NULL);
    bool collidesWithGround(const Box2 &box);

    void addElement(Element *element);
    void addPlayer(Element *element);
    void addBackground();
    void destroyElement(Element *element);
    void clear();

    void generateBlocks();
    void destroyBlocks();

    void update(float delta);
    void draw(Renderer *renderer);

    void changedPlayerPosition(const Vector2 &position);

    Matrix4 getCameraTransform() const;

    bool isStarted() const;
    void started();

    float computeMaxHeight() const;

    virtual void playerTouch(Player *player, const Vector2 &direction);

private:
    float generateScalar();
    float generateRange(float xmin, float xmax);
    float oscillatingRange(float min, float max, float freq, float x);

    int generateIntRange(int min, int max);
    Vector2 generatePositionAtHeight(float y);
    Rectangle *generateRectangleAtHeight(float y);

    float getUpperBound() const;
    float getLowerBound() const;

    void computeDifficulty();

    Color nextBackgroundColor() const;
    void updateBackground(float delta);

    Game *game;

    // Elements
    typedef std::list<Element*> Elements;
    Elements elements;
    std::vector<Element*> lastFloorElements;

    // Background.
    Element *background;
    Color backgroundColorStart;
    Color backgroundColorNext;
    float backgroundFade;

    // Global attributes
    Color ambientLight;
    float ambientIntensity;
    Vector2 gravity;

    // Game tracking
    float currentHeight;
    float lastAddedHeight;
    bool gameStarted;

    // Difficulty.
    float heightRange;
    float widthRange;
   
};
} // namespace Tarea2

#endif //_TAREA2_SECTOR_HPP
