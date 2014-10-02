#include "Application.hpp"

namespace T3
{

Application::Application()
    : display(this), raytracer(this), scene(NULL)
{
    velocity = Vector3::zero();
    angularVelocity = Vector3::zero();
    elapsedTime = 0.0f;
}

Application::~Application()
{
}

Display *Application::getDisplay()
{
    return &display;
}

Scene *Application::getScene()
{
    Lock l(mutex);
    return scene;
}

void Application::setScene(Scene *newScene)
{
    Lock l(mutex);
    scene = newScene;
}

bool Application::initialize(int argc, const char *argv[])
{
    const char *sceneName = NULL;
    for(int i = 1; i < argc; ++i)
    {
        sceneName = argv[i];
    }

    // Make sure there's a scene
    if(!sceneName)
    {
        fprintf(stderr, "Please, submit a scene\n");
        return false;
    }
    scene = Scene::loadFromFile(sceneName);

    if(!display.initialize())
        return false;
    if(!raytracer.initialize())
        return false;
    return true;
}

void Application::createScene()
{
    // Materials.
    Lock l(mutex);
    scene = new Scene();

    // Ground material.
    int groundTex = scene->createColorTextureId(Color(0.4f, 0.3f, 0.3f));
    Material *groundMaterial = new Material();
    groundMaterial->reflection = 0.2f;
    groundMaterial->diffuseTexture = groundTex;
    groundMaterial->specularTexture = groundTex;
    scene->addMaterial(groundMaterial);
    
    // Big sphere material.
    int bigSphereTex = scene->createColorTextureId(Color(0.7f, 0.7f, 0.7f));
    Material *bigSphereMaterial = new Material();
    bigSphereMaterial->reflection = 1.0f;
    bigSphereMaterial->diffuse = 0.0f;
    bigSphereMaterial->specularTexture = bigSphereTex;
    scene->addMaterial(bigSphereMaterial);

    // Small sphere material.
    int smallSphereTex = scene->createColorTextureId(Color(0.7f, 0.7f, 1.0f));
    Material *smallSphereMaterial = new Material();
    smallSphereMaterial->reflection = 1.0f;
    smallSphereMaterial->diffuse = 0.1f;
    smallSphereMaterial->diffuseTexture = smallSphereTex;
    smallSphereMaterial->specularTexture = smallSphereTex;
    scene->addMaterial(smallSphereMaterial);

    // Light 1 material.
    int light1Tex = scene->createColorTextureId(Color(0.4f, 0.4f, 0.4f));
    Material *light1Material = new Material();
    light1Material->light = true;
    light1Material->emission = 1.0f;
    light1Material->emissionTexture = light1Tex;
    scene->addMaterial(light1Material);

    // Light 2 material.
    int light2Tex = scene->createColorTextureId(Color(0.6f, 0.6f, 0.6f));
    Material *light2Material = new Material();
    light2Material->light = true;
    light2Material->emission = 1.0f;
    light2Material->emissionTexture = light2Tex;
    scene->addMaterial(light2Material);
        
    // Ground plane.
    scene->addShape(new PlaneShape(Vector3(0, 1, 0), 4.4f, groundMaterial->getId()));
    scene->addShape(new SphereShape(Vector3(1, -0.8f, 3), 2.5f, bigSphereMaterial->getId()));
    scene->addShape(new SphereShape(Vector3(-5.5f, -0.5, 7), 2, smallSphereMaterial->getId()));
    scene->addShape(new SphereShape(Vector3(0, 5, 5), 0.1f, light1Material->getId()));
    scene->addShape(new SphereShape(Vector3(2, 5, 1 ), 0.1f, light2Material->getId()));
}

void Application::run()
{
    display.run();
}

void Application::shutdown()
{
    raytracer.shutdown();
}

void Application::quit()
{
    shutdown();
}

void Application::update(float delta)
{
    rotation = rotation + angularVelocity*delta;
    Camera camera = scene->getCamera();
    Matrix3 newOrientation = Matrix3::xyzRot(rotation).transpose();
    camera.setPosition(camera.getPosition() + newOrientation*(velocity*delta));
    camera.setOrientation(newOrientation);
    scene->setCamera(camera);

    elapsedTime += delta;
    float theta = elapsedTime*M_PI*2.0f/40.0f;
    float s = sin(theta);
    float c = cos(theta);
    Vector3 sunDir = Vector3(0.0f, s, c);
    scene->setSunDirection(sunDir);
}

const Vector3 &Application::getVelocity()
{
    return velocity;
}

void Application::setVelocity(const Vector3 &newVelocity)
{
    velocity = newVelocity;
}

const Vector3 &Application::getAngularVelocity()
{
    return angularVelocity;
}

void Application::setAngularVelocity(const Vector3 &newVelocity)
{
    angularVelocity = newVelocity;
}

const Vector3 &Application::getRotation() const
{
    return rotation;
}

void Application::setRotation(const Vector3 &newRotation)
{
    rotation = newRotation;
}


} // namespace T3

