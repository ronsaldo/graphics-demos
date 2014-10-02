#ifndef T3_SCENE_HPP
#define T3_SCENE_HPP

#include <vector>
#include <string>
#include "Geometry.hpp"
#include "Matrix3.hpp"
#include "Threading.hpp"

namespace T3
{

/**
 * Serialized scene data holder.
 */
class SceneDataHolder
{
public:
    SceneDataHolder(unsigned char *data, size_t size)
        : data(data), size(size) {}
    ~SceneDataHolder()
    {
        delete [] data;
    }

    const unsigned char *getData() const
    {
        return data;
    }

    size_t getSize() const
    {
        return size;
    }

private:
    unsigned char *data;
    size_t size;
};

/**
 * Camera
 */
class Camera
{
public:
    Camera();
    ~Camera();

    const Vector3 &getPosition() const;
    void setPosition(const Vector3 &newPosition);

    const Matrix3 &getOrientation() const;
    void setOrientation(const Matrix3 &newOrientation);

    const Vector3 *getScreenPlaneVerts() const;

private:
    Vector3 position;
    Matrix3 orientation;
    Vector3 screenPlaneVerts[4];
};

/**
 * Scene.
 */
class Scene
{
public:
    Scene();
    ~Scene();

    // Materials
    size_t getMaterialCount() const;
    void addMaterial(Material* material);
    Material *getMaterial(size_t index);

    // Textures.
    size_t getTextureCount() const;
    void addTexture(Texture* texture);
    Texture *getTexture(size_t index);
    Texture *createColorTexture(const Color &color);
    int createColorTextureId(const Color &color);

    // Shapes
    size_t getShapeCount() const;
    void addShape(Shape *shape);
    Shape *getShape(size_t index);

    // Scene data.
    SceneDataHolder *getSceneData();

    // Camera
    Camera getCamera();
    void setCamera(const Camera &camera);

    // Sky.
    bool isDay() const;
    void setDay(bool value);

    float getSkyRadius() const;
    void setSkyRadius(float newRadius);

    float getStarThreshold() const;
    void setStarThreshold(float newThreshold);

    float getStarScale() const;
    void setStarScale(float newScale);

    Color getSunColor() const;
    void setSunColor(const Color &color);

    Vector3 getSunDirection() const;
    void setSunDirection(const Vector3 &direction);

    // File loading.
    static Scene *loadFromFile(const std::string &filename);

private:
    std::vector<Material*> materials;
    std::vector<Texture*> textures;
    std::vector<Shape*> shapes;

    // Sky
    bool daySky;
    float skyRadius;
    float starThreshold;
    float starScale;
    Color sunColor;
    Vector3 sunDirection;

    // Camera
    Camera camera;

    Mutex mutex;
};
} // namespace T3

#endif //T3_SCENE_HPP

