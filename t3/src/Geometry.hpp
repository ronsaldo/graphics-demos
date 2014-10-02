#ifndef T3_GEOMETRY_HPP
#define T3_GEOMETRY_HPP

#include "CommonCL.hpp"
#include "Noise.hpp"

/**
 * AABox
 */
class AABox
{
public:
    AABox() {}
    AABox(const Vector3 &min, const Vector3 &max)
        : min(min), max(max) {}
    ~AABox() {}

    bool contains(const Vector3 &point) const
    {
        return min.x <= point.x && point.x <= max.x &&
                min.y <= point.y && point.y <= max.y &&
                min.z <= point.z && point.z <= max.z;
    }

    Vector3 center() const
    {
        return (max - min)*0.5f;
    }

    float height() const
    {
        return max.y - min.y;
    }

    Vector3 min, max;
};

/**
 * Ray.
 */
class Ray
{
public:
    Ray() {}
    Ray(const Vector3 &start, const Vector3 &direction)
        : start(start), direction(direction) {}
    ~Ray() {}
    
    Vector3 at(float amount) const
    {
        return start + direction*amount;
    }

    Vector3 start;
    Vector3 direction;

public:
    static Ray fromPoints(const Vector3 &start, const Vector3 &dest)
    {
        Vector3 dir = dest - start;
        return Ray(start, normalize(dir));
    }
};

/**
 * Noise element.
 */
class NoiseElement
{
public:
    enum Type
    {
        TT_None,
        TT_Noise,
        TT_AbsNoise,
        TT_Marble,
        TT_MarbleSoft,
    };

    NoiseElement()
        : type(TT_None), depth(1), persistence(0.5f), normalScale(1.0f), noiseScale(1.5f), noiseOffset(0.0f),
                coordScale(make_vector3(1, 1, 1)), coordOffset(make_vector3(0, 0, 0)) {}
    ~NoiseElement() {}

    float sumNoise(Vector3 position) const
    {
        float res = 0.0f;
        float fact = 1.0f;
        float invFact = 1.0f;
        for(int i = 1; i <= depth; ++i, fact *=2.0f, invFact*=persistence)
            res += noise3D(position*fact)*invFact;
        return res;
    }

    float sumAbsNoise(Vector3 position) const
    {
        float res = 0.0f;
        float fact = 1.0f;
        float invFact = 1.0f;
        for(int i = 1; i <= depth; ++i, fact *=2.0f, invFact*=persistence)
            res += fabs(noise3D(position*fact))*invFact;
        return res;
    }

    float cloudsNoise(Vector3 position) const
    {
        return sumNoise(position*coordScale + coordOffset);
    }

    float hardCloudsNoise(Vector3 position) const
    {
        return sumAbsNoise(position*coordScale + coordOffset);
    }

    float marbleNoise(Vector3 position) const
    {
        return sin(dot(position*coordScale + coordOffset, direction) + sumAbsNoise(position*coordScale + coordOffset));
    }

    float softMarbleNoise(Vector3 position) const
    {
        return sin(dot(position*coordScale + coordOffset, direction) + sumNoise(position*coordScale + coordOffset));
    }

    float computeNoiseFunction(Vector3 position) const
    {
        switch(type)
        {
        case TT_Noise:
            return cloudsNoise(position)*noiseScale + noiseOffset;
        case TT_AbsNoise:
            return hardCloudsNoise(position)*noiseScale + noiseOffset;
        case TT_Marble:
            return marbleNoise(position)*noiseScale + noiseOffset;
        case TT_MarbleSoft:
            return softMarbleNoise(position)*noiseScale + noiseOffset;
        default:
        case TT_None:
            return 1.0f;
        }
    }

    Vector3 computeNormal(Vector3 position, Vector3 normal) const
    {
        const float delta = 0.0005f;
        if(type == TT_None)
            return normal;

        int bestAxis = bestVectorAxis(normal);
        Vector3 du = vectorAxis(vector_axis[bestAxis+1])*delta;
        Vector3 dv = vectorAxis(vector_axis[bestAxis+2])*delta;

        // Sample the height.
#ifndef RAYTRACER_ACCU_GRADIENT
        float height = computeNoiseFunction(position);
        float heightDU = computeNoiseFunction(position + du) - height;
        float heightDV = computeNoiseFunction(position + dv) - height;

        Vector3 gradU = du + normal*normalScale*heightDU;
        Vector3 gradV = dv + normal*normalScale*heightDV;
#else
        float height = computeNoiseFunction(position);
        float heightDU = (computeNoiseFunction(position + du) - computeNoiseFunction(position - du))*0.5f;
        float heightDV = (computeNoiseFunction(position + dv) - computeNoiseFunction(position - dv))*0.5f;

        Vector3 gradU = 2.0*du + normal*normalScale*heightDU;
        Vector3 gradV = 2.0*dv + normal*normalScale*heightDV;
#endif

        Vector3 N = normalize(cross(gradV, gradU));
        N = (dot(N, normal) < 0.0) ? -N : N;
        return N;
    }

    Type type;

    // Procedural noise.
    int depth;
    float persistence;
    float normalScale;
    float noiseScale;
    float noiseOffset;
    Vector3 coordScale;
    Vector3 coordOffset;
    Vector3 direction;

};

/**
 * Shape base class.
 */
class Shape
{
public:
    enum Type
    {
        ShapeType_Plane = 0,
        ShapeType_Sphere,
        ShapeType_Terrain,
    };

    ~Shape() {}

    static size_t size(const __global Shape *shape);
    static float intersects(const __global Shape *shape, const Ray &ray);
    static Vector3 normalAt(const __global Shape *shape, Vector3 position);
    static Vector3 lightDir(const __global Shape *shape, Vector3 position);
    static float computeSideFactor(const __global Shape *shape, Vector3 position);

    int materialId;

    Type getType() const
    {
        return type;
    }

protected:

    Shape(Type type, int materialId = 0)
        : materialId(materialId), type(type) {}

private:
    Type type;
};

/**
 * PlaneShape
 */
class PlaneShape: public Shape
{
public:
    PlaneShape()
        : Shape(ShapeType_Plane) {}
    PlaneShape(const Vector3 &normal, const float distance, int materialId=-1)
        : Shape(ShapeType_Plane, materialId), distance(distance), normal(normal)  {}

    ~PlaneShape() {}

    float intersects(const Ray &ray) const
    {
        float d = dot(normal, ray.direction);
        if(d != 0.0)
            return -(dot(normal, ray.start) + distance) / d;
        return -1.0f;
    }

    Vector3 normalAt(Vector3 pos) const
    {
        return normal;
    }

    Vector3 lightDir(Vector3 pos) const
    {
        return -normal;
    }

    float computeSideFactor(Vector3 position) const
    {
        return 1.0;
    }

    float distance;
    Vector3 normal;
};

/**
 * SphereShape
 */
class SphereShape: public Shape
{
public:
    SphereShape()
        : Shape(ShapeType_Sphere){}
    SphereShape(const Vector3 &position, const float radius, int materialId=-1)
        : Shape(ShapeType_Sphere, materialId), radius(radius), invRadius(1.0f/radius), position(position)  {}

    ~SphereShape() {}

    float intersects(const Ray &ray) const
    {
	    Vector3 v = ray.start - position;
	    float b = -dot(v, ray.direction );
	    float det = (b*b) - dot(v, v) + radius*radius;

	    if (det > 0)
	    {
		    det = sqrt( det );
		    float i1 = b - det;
		    float i2 = b + det;
		    if (i2 > 0)
                return (i1 < 0) ? i2 : i1;
	    }

        return -1.0f;
    }

    Vector3 normalAt(Vector3 pos) const
    {
        return (pos - position)*invRadius;
    }

    Vector3 lightDir(Vector3 pos) const
    {
        return position - pos;
    }

    float computeSideFactor(Vector3 p) const
    {
        Vector3 v = p - position;
        return (dot(v, v) > radius*radius) ? 1.0f : -1.0f;
    }

    float radius;
    float invRadius;
    Vector3 position;
};

/**
 * Terrain shape.
 */
class TerrainShape: public Shape
{
public:
    TerrainShape()
        : Shape(ShapeType_Terrain) {}
    ~TerrainShape() {}

    float intersects(const Ray &ray) const
    {
        return -1.0f;
    }

    Vector3 normalAt(Vector3 pos) const
    {
        return normalize(pos - boundingBox.center());
    }

    Vector3 lightDir(Vector3 pos) const
    {
        return boundingBox.center() - pos;
    }

    float computeSideFactor(Vector3 position) const
    {
        return 1.0;
    }

    NoiseElement noise;
    AABox boundingBox;
};

/**
 * Reports the size of a shape.
 */
inline size_t Shape::size(const __global Shape *shape)
{
    switch(shape->type)
    {
    case ShapeType_Plane:
        return sizeof(PlaneShape);
    case ShapeType_Sphere:
        return sizeof(SphereShape);
    case ShapeType_Terrain:
        return sizeof(TerrainShape);
    default:
        return 0;
    }

}

/**
 * Dispatch shape intersects.
 */
inline float Shape::intersects(const __global Shape *shape, const Ray &ray)
{
    switch(shape->type)
    {
    case ShapeType_Plane:
        return ((const __global PlaneShape*) shape)->intersects(ray);
    case ShapeType_Sphere:
        return ((const __global SphereShape*) shape)->intersects(ray);
    case ShapeType_Terrain:
        return ((const __global TerrainShape*) shape)->intersects(ray);
    default:
        return -1.0;
    }
}

/**
 * Dispatch normal computation.
 */
inline Vector3 Shape::normalAt(const __global Shape *shape, Vector3 position)
{
    switch(shape->type)
    {
    case ShapeType_Plane:
        return ((const __global PlaneShape*) shape)->normalAt(position);
    case ShapeType_Sphere:
        return ((const __global SphereShape*) shape)->normalAt(position);
    case ShapeType_Terrain:
        return ((const __global TerrainShape*) shape)->normalAt(position);
    default:
        return make_vector3(0.0, 1.0, 0.0);
    }
}

/**
 * Dispatch light direction computation.
 */
inline Vector3 Shape::lightDir(const __global Shape *shape, Vector3 position)
{
    switch(shape->type)
    {
    case ShapeType_Plane:
        return ((const __global PlaneShape*) shape)->lightDir(position);
    case ShapeType_Sphere:
        return ((const __global SphereShape*) shape)->lightDir(position);
    case ShapeType_Terrain:
        return ((const __global TerrainShape*) shape)->lightDir(position);
    default:
        return make_vector3(0.0f, 1.0f, 0.0f);
    }
}

/**
 *
 */
inline float Shape::computeSideFactor(const __global Shape *shape, Vector3 position)
{
    switch(shape->type)
    {
    case ShapeType_Plane:
        return ((const __global PlaneShape*) shape)->computeSideFactor(position);
    case ShapeType_Sphere:
        return ((const __global SphereShape*) shape)->computeSideFactor(position);
    case ShapeType_Terrain:
        return ((const __global TerrainShape*) shape)->computeSideFactor(position);
    default:
        return 1.0;
    }
}

/**
 * Texture
 */
class Texture: public NoiseElement
{
public:
    Texture()
        : textureId(-1), startColor(color_black()), color(color_white()) {}
    Texture(const Color &color)
        : textureId(-1), startColor(color_black()), color(color) {}
    ~Texture() {}


    int getId() const
    {
        return this->textureId;
    }

    void setId(int newId)
    {
        this->textureId = newId;
    }

    Color computeColor(Vector3 position) const
    {
        if(type == TT_None)
            return color;
        return mix(startColor, color, computeNoiseFunction(position));
    }
    
    int textureId;
    Color startColor;
    Color color;
};

/**
 * Material
 */
class Material
{
public:
    Material()
        : light(false), materialId(-1), 
           emissionTexture(-1), diffuseTexture(-1), specularTexture(-1),
           normalTexture(-1),
           emission(0.0f), diffuse(1.0f), specular(1.0f), shininess(20.0f),
           reflection(0.0f), refraction(0.0f), refractionIndex(1.0f) {}
    ~Material() {}

    int getId() const
    {
        return this->materialId;
    }

    void setId(int newId)
    {
        this->materialId = newId;
    }

    bool light;
    int materialId;

    int emissionTexture;
    int diffuseTexture;
    int specularTexture;
    int normalTexture;

    float emission;
    float diffuse;
    float specular;
    float shininess;

    float reflection;
    float refraction;
    float refractionIndex;
    Vector4 padding; // To ensure alignment.
};

/**
 * Scene buffer
 * ------------------
 * unsigned int numMaterials;
 * unsigned int numShapes;
 * unsigned int numTextures;
 * unsigned int padding[];
 * Material materials[numMaterials];
 * Texture textures[numTextures];
 * unsigned int shapeOffsets[numShapes];
 * Shape shapes[numShapes];
 */
class SceneAccess
{
public:
    SceneAccess(const __global unsigned char *data)
        : data(data)
    {
        readStructure();
    }

    unsigned int getMaterialCount() const
    {
        return numMaterials;
    }

    unsigned int getShapeCount() const
    {
        return numShapes;
    }

    unsigned int getTextureCount() const
    {
        return numTextures;
    }

    const __global Material *getMaterial(size_t id) const
    {
        return &materials[id];
    }

    const __global Texture *getTexture(size_t id) const
    {
        return &textures[id];
    }

    const __global Shape *getShape(size_t id) const
    {
        return (__global Shape*)(shapeOffsets[id] + data);
    }

    bool firstIntersection(const Ray &ray, float *amount, const __global Shape **element) const
    {
        *amount = -1.0f;
        *element = NULL;

        for(unsigned int i = 0; i < numShapes; ++i)
        {
            const __global Shape *shape = getShape(i);
            float res = Shape::intersects(shape, ray);
            if(res >= 0.0f && (*element == NULL || res < *amount))
            {
                *amount = res;
                *element = shape;
            }
        }

        return *element != NULL;
    }

    bool blockedLine(const Ray &ray, const __global Shape *testShape) const
    {
        float maxAmount = Shape::intersects(testShape, ray);
        if(maxAmount <= 0.0)
            return true;

        for(unsigned int i = 0; i < numShapes; ++i)
        {
            const __global Shape *shape = getShape(i);
            if(shape == testShape)
                continue;

            float res = Shape::intersects(shape, ray);
            if(res >= 0.0f && res < maxAmount)
                return true;
        }

        return false;
    }

private:
    int readInt(const __global unsigned char *data)
    {
        return *((__global int*)data);
    }

    void readStructure()
    {
        numMaterials = readInt(data);
        numTextures = readInt(data + sizeof(int));
        numShapes = readInt(data + 2*sizeof(int));

        materials = (__global Material*)(data + sizeof(int)*4);
        textures = (__global Texture*)(data + sizeof(int)*4 + numMaterials*sizeof(Material));
        shapeOffsets = (const __global unsigned int *)(data + sizeof(int)*4 + numMaterials*sizeof(Material) + numTextures*sizeof(Texture));
    }

    unsigned int numMaterials;
    unsigned int numTextures;
    unsigned int numShapes;
    const __global Material *materials;
    const __global Texture *textures;
    const __global unsigned int *shapeOffsets;
    const __global unsigned char *data;
};

#endif //T3_GEOMETRY_HPP

