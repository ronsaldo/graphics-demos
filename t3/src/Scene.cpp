#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <assert.h>
#include <string.h>
#include "rapidxml.hpp"
#include "Scene.hpp"

namespace T3
{

//----------------------------------------------------------------------------
// Camera
//

Camera::Camera()
{
    position = Vector3(0, 0, -5);
    orientation = Matrix3::identity();
    screenPlaneVerts[0] = Vector3(-4, 3, 5);
    screenPlaneVerts[1] = Vector3(4, 3, 5);
    screenPlaneVerts[2] = Vector3(4, -3, 5);
    screenPlaneVerts[3] = Vector3(-4, -3, 5);
}

Camera::~Camera()
{
}

const Vector3 &Camera::getPosition() const
{
    return position;
}

void Camera::setPosition(const Vector3 &newPosition)
{
    position = newPosition;
}

const Matrix3 &Camera::getOrientation() const
{
    return orientation;
}

void Camera::setOrientation(const Matrix3 &newOrientation)
{
    orientation = newOrientation;
}

const Vector3 *Camera::getScreenPlaneVerts() const
{
    return screenPlaneVerts;
}


//----------------------------------------------------------------------------
// Scene
//

Scene::Scene()
    : daySky(false), skyRadius(1000.0f), starThreshold(0.9f), starScale(1.0f),
        sunColor(1, 1, 1, 1), sunDirection(Vector3(0, 1, 0).normalized())
{
}

Scene::~Scene()
{
    for(size_t i = 0; i < materials.size(); ++i)
        delete materials[i];
    for(size_t i = 0; i < textures.size(); ++i)
        delete textures[i];
    for(size_t i = 0; i < shapes.size(); ++i)
        delete shapes[i];
}

//----------------------------------------------------------------------------
// Materials
//

size_t Scene::getMaterialCount() const
{
    Lock l(mutex);
    return materials.size();
}

void Scene::addMaterial(Material* material)
{
    Lock l(mutex);
    material->setId(materials.size());
    materials.push_back(material);
}

Material *Scene::getMaterial(size_t index)
{
    Lock l(mutex);
    return materials[index];
}

//----------------------------------------------------------------------------
// Textures
//

size_t Scene::getTextureCount() const
{
    Lock l(mutex);
    return textures.size();
}

void Scene::addTexture(Texture* texture)
{
    Lock l(mutex);
    texture->setId(textures.size());
    textures.push_back(texture);
}

Texture *Scene::createColorTexture(const Color &color)
{
    Lock l(mutex);
    Texture *tex = new Texture(color);
    addTexture(tex);
    return tex;
}

int Scene::createColorTextureId(const Color &color)
{
    Lock l(mutex);
    return createColorTexture(color)->textureId;
}

Texture *Scene::getTexture(size_t index)
{
    Lock l(mutex);
    return textures[index];
}

//----------------------------------------------------------------------------
// Shapes
//

size_t Scene::getShapeCount() const
{
    Lock l(mutex);
    return shapes.size();
}

void Scene::addShape(Shape *shape)
{
    Lock l(mutex);
    shapes.push_back(shape);
}

Shape *Scene::getShape(size_t index)
{
    Lock l(mutex);
    return shapes[index];
}

//----------------------------------------------------------------------------
// Camera
//

Camera Scene::getCamera()
{
    Lock l(mutex);
    return camera;
}

void Scene::setCamera(const Camera &camera)
{
    Lock l(mutex);
    this->camera = camera;
}

//----------------------------------------------------------------------------
// Sky
//

bool Scene::isDay() const
{
    Lock l(mutex);
    return daySky;
}

void Scene::setDay(bool value)
{
    Lock l(mutex);
    daySky = value;
}

float Scene::getSkyRadius() const
{
    Lock l(mutex);
    return skyRadius;
}

void Scene::setSkyRadius(float newRadius)
{
    Lock l(mutex);
    skyRadius = newRadius;
}

float Scene::getStarThreshold() const
{
    Lock l(mutex);
    return starThreshold;
}

void Scene::setStarThreshold(float newThreshold)
{
    Lock l(mutex);
    starThreshold = newThreshold;
}

float Scene::getStarScale() const
{
    Lock l(mutex);
    return starScale;
}

void Scene::setStarScale(float newScale)
{
    Lock l(mutex);
    starScale = newScale;
}

Color Scene::getSunColor() const
{
    Lock l(mutex);
    return sunColor;
}

void Scene::setSunColor(const Color &color)
{
    Lock l(mutex);
    sunColor = color;
}

Vector3 Scene::getSunDirection() const
{
    Lock l(mutex);
    return sunDirection;
}

void Scene::setSunDirection(const Vector3 &direction)
{
    Lock l(mutex);
    sunDirection = direction;
}

//-----------------------------------------------------------------------
// Buffer Writing
//

inline void writeUInt(unsigned int value, unsigned char **dst)
{
    *((unsigned int*)*dst) = value;
    *dst += 4;
}

SceneDataHolder *Scene::getSceneData()
{
    Lock l(mutex);

    // Compute the sizes.
    size_t size = 4*sizeof(unsigned int);
    size += sizeof(Material)*materials.size();
    size += sizeof(Texture)*textures.size();

    // Compute the offsets.
    std::vector<size_t> shapeOffsets;
    size_t paddedOffsets = (shapes.size() + 3) & ~3;
    size_t startOffset = size + sizeof(unsigned int)*paddedOffsets;
    size_t offset = startOffset;
    for(size_t i = 0; i < shapes.size(); ++i)
    {
        shapeOffsets.push_back(offset);
        offset += Shape::size(shapes[i]);
    }
    size = offset;
    
    // Allocate the space.
    unsigned char *data = new unsigned char[size];
    unsigned char *dst = data;
    writeUInt(materials.size(), &dst);
    writeUInt(textures.size(), &dst);
    writeUInt(shapes.size(), &dst);
    dst += sizeof(unsigned int);

    // Copy materials.
    for(size_t i = 0; i < materials.size(); ++i)
    {
        memcpy(dst, materials[i], sizeof(Material));
        dst += sizeof(Material);
    }

    // Copy the textures.
    for(size_t i = 0; i < textures.size(); ++i)
    {
        memcpy(dst, textures[i], sizeof(Texture));
        dst += sizeof(Texture);
    }

    // Copy the shapes offsets.
    for(size_t i = 0; i < shapeOffsets.size(); ++i)
    {
        writeUInt(shapeOffsets[i], &dst);
    }

    // Copy the shapes;
    dst = data + startOffset;
    for(size_t i = 0; i < shapes.size(); ++i)
    {
        size_t shapeSize = Shape::size(shapes[i]);
        memcpy(dst, shapes[i], shapeSize);
        dst += shapeSize;
    }
    assert(dst == data + size);

    // Create the data holder.
    return new SceneDataHolder(data, size);
}

//-------------------------------------------------------------
// Scene loading
//

inline std::vector<char> readWholeFile(const char* filename)
{
    // open the file:
    std::ifstream file(filename, std::ios::binary);

    // read the data:
    return std::vector<char> ((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
}

using namespace rapidxml;

inline const char *getAttribute(rapidxml::xml_node<> *node, const char *name, const char *def=NULL)
{
    rapidxml::xml_attribute<> *attr = node->first_attribute(name);
    if(attr)
        return attr->value();
    return def;
}

inline bool getBooleanAttribute(rapidxml::xml_node<> *node, const char *name, bool def = false)
{
    const char *val = getAttribute(node, name);
    if(!val)
        return def;

    return strcmp(val, "0") && strcmp(val, "false");
}

inline int getIntAttribute(rapidxml::xml_node<> *node, const char *name, int def = 0)
{
    const char *val = getAttribute(node, name);
    if(!val)
        return def;

    return atoi(val);
}

inline float getScalarAttribute(rapidxml::xml_node<> *node, const char *name, float def = 0.0f)
{
    const char *val = getAttribute(node, name);
    if(!val)
        return def;

    return atof(val);
}

inline Vector3 getVectorAttribute(rapidxml::xml_node<> *node, const char *name, const Vector3 &def = Vector3::zero())
{
    const char *val = getAttribute(node, name);
    if(!val)
        return def;

    Vector3 res;
    sscanf(val, "%f %f %f", &res.x, &res.y, &res.z);
    return res;
}

inline Color getColorAttribute(rapidxml::xml_node<> *node, const char *name, const Color &def = Color(0,0,0,0))
{
    const char *val = getAttribute(node, name);
    if(!val)
        return def;

    Color res;
    sscanf(val, "%f %f %f %f", &res.r, &res.g, &res.b, &res.a);
    return res;
}

static void loadNoiseData(xml_node<> *node, NoiseElement *noiseElement)
{
    if(!node)
        return;

    std::string type = getAttribute(node, "type", "color");
    if(type == "noise" || type == "clouds")
        noiseElement->type = Texture::TT_Noise;
    else if(type == "abs-noise" || type == "hard-clouds")
        noiseElement->type = Texture::TT_AbsNoise;
    else if(type == "marble")
        noiseElement->type = Texture::TT_Marble;
    else if(type == "marble-soft")
        noiseElement->type = Texture::TT_MarbleSoft;
    else
        noiseElement->type = Texture::TT_None;

    // Fractal noise parameters.
    noiseElement->depth = getIntAttribute(node, "depth", 1);
    noiseElement->persistence = getScalarAttribute(node, "persistence", 0.5);

    // Noise result transform.
    noiseElement->noiseScale = getScalarAttribute(node, "noise-scale", 1.0);
    noiseElement->noiseOffset = getScalarAttribute(node, "noise-offset", 0.0);

    // Input coordinates transform.
    noiseElement->coordScale = getVectorAttribute(node, "coord-scale", Vector3(1, 1, 1));
    noiseElement->coordOffset = getVectorAttribute(node, "coord-offset", Vector3::zero());

    // Marble direction.
    noiseElement->direction = getVectorAttribute(node, "direction", Vector3(1, 0, 0).normalized());
}

static Texture *loadTexture(xml_node<> *node)
{
    Texture *texture = new Texture();

    // Noise data.
    loadNoiseData(node, texture);

    // Coloring.
    texture->startColor = getColorAttribute(node, "start-color", color_black());
    texture->color = getColorAttribute(node, "color", color_white());
    return texture;
}

inline int getTextureId(std::map<std::string, Texture*> &textures, const char *name)
{
    if(!name || !strcmp(name, "<black>"))
        return -1;
    else if(!strcmp(name, "<white>"))
        return -2;
    return textures[name]->getId();
}

inline int getTextureIdAttribute(xml_node<> *node, std::map<std::string, Texture*> &textures, const char *name, int def=-1)
{
    const char *val = getAttribute(node, name);
    if(!val)
        return def;
    return getTextureId(textures, val);
}

static Material *loadMaterial(std::map<std::string, Texture*> &textures, xml_node<> *node)
{
    Material *material = new Material();
    material->light = getBooleanAttribute(node, "light", false);

    // Normal mapping.
    material->normalTexture = getTextureIdAttribute(node, textures, "normal-texture", -1);

    // Emission
    material->emissionTexture = getTextureIdAttribute(node, textures, "emission-texture", -2);
    material->emission = getScalarAttribute(node, "emission", 0.0f);

    // Diffuse
    material->diffuseTexture = getTextureIdAttribute(node, textures, "diffuse-texture", -2);
    material->diffuse = getScalarAttribute(node, "diffuse", 1.0f);

    // Specular.
    material->specularTexture = getTextureIdAttribute(node, textures, "specular-texture", -2);
    material->specular = getScalarAttribute(node, "specular", 1.0f);
    material->shininess = getScalarAttribute(node, "shininess", 20.0f);

    // Reflection/Refraction.
    material->reflection = getScalarAttribute(node, "reflection", 0.0f);
    material->refraction = getScalarAttribute(node, "refraction", 0.0f);
    material->refractionIndex = getScalarAttribute(node, "refraction-index", 1.05f);
    return material;
}

static Shape *loadSphereShape(xml_node<> *node)
{
    return new SphereShape(getVectorAttribute(node, "center"), getScalarAttribute(node, "radius"));
}

static Shape *loadPlaneShape(xml_node<> *node)
{
    return new PlaneShape(getVectorAttribute(node, "normal"), getScalarAttribute(node, "distance"));
}

static Shape *loadTerrainShape(xml_node<> *node)
{
    TerrainShape *terrain = new TerrainShape();
    loadNoiseData(node->first_node("noise"), &terrain->noise);
    terrain->boundingBox.min = getVectorAttribute(node, "min");
    terrain->boundingBox.max = getVectorAttribute(node, "max");
    return terrain;
}

static Shape *loadShape(xml_node<> *node)
{
    std::string type = getAttribute(node, "type");
    if(type == "sphere")
        return loadSphereShape(node);
    else if(type == "plane")
        return loadPlaneShape(node);
    else if(type == "terrain")
        return loadTerrainShape(node);
    else
        return NULL;
}

static void loadSky(xml_node<> *node, Scene *scene)
{
    scene->setDay(getBooleanAttribute(node, "day", false));
    scene->setSkyRadius(getScalarAttribute(node, "radius", 1000.0f));
    scene->setStarThreshold(getScalarAttribute(node, "star-threshold", 0.9f));
    scene->setStarScale(getScalarAttribute(node, "star-scale", 1.0f));

    xml_node<> *sunNode = node->first_node("sun");
    if(sunNode)
    {
        scene->setSunColor(getColorAttribute(sunNode, "color", Color(1, 1, 1, 1)));
        scene->setSunDirection(getVectorAttribute(sunNode, "direction", Vector3(0, 1, 0)).normalized());
    }
}

Scene *Scene::loadFromFile(const std::string &filename)
{
    // Read the scene content.
    std::vector<char> sceneFileData = readWholeFile(filename.c_str());
    sceneFileData.push_back(0);

    xml_document<> doc;    // character type defaults to char
    doc.parse<0> (&sceneFileData[0]);    // 0 means default parse flags
    
    // Get the root node.
    rapidxml::xml_node<> *rootNode = doc.first_node("scene");

    // Create the scene.
    Scene *scene = new Scene();
    std::map<std::string, Texture*> textureMap;
    std::map<std::string, Material*> materialMap;

    // Load the sky
    xml_node<> *skyNode = rootNode->first_node("sky");
    if(skyNode)
        loadSky(skyNode, scene);

    // Load the textures.
    xml_node<> *texturesNode = rootNode->first_node("textures");
    if(texturesNode)
    {
        xml_node<> *textureNode = texturesNode->first_node("texture");
        for(; textureNode; textureNode = textureNode->next_sibling("texture"))
        {
            std::string name = getAttribute(textureNode, "name", "");
            Texture *texture = loadTexture(textureNode);
            textureMap[name] = texture;
            scene->addTexture(texture);
        }
    }
    
    // Load the materials.
    xml_node<> *materialsNode = rootNode->first_node("materials");
    if(materialsNode)
    {
        xml_node<> *materialNode = materialsNode->first_node("material");
        for(; materialNode; materialNode = materialNode->next_sibling("material"))
        {
            std::string name = getAttribute(materialNode, "name", "");
            Material *material = loadMaterial(textureMap, materialNode);
            materialMap[name] = material;
            scene->addMaterial(material);
        }
    }

    // Load the shapes.
    xml_node<> *shapesNode = rootNode->first_node("shapes");
    if(shapesNode)
    {
        xml_node<> *shapeNode = shapesNode->first_node("shape");
        for(; shapeNode; shapeNode = shapeNode->next_sibling("shape"))
        {
            Shape *shape = loadShape(shapeNode);
            if(!shape)
                continue;
            shape->materialId = materialMap[getAttribute(shapeNode, "material")]->getId();
            scene->addShape(shape);
        }
    }

    return scene;
}

}

