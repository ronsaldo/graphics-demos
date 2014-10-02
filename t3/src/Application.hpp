#ifndef T3_APPLICATION_HPP
#define T3_APPLICATION_HPP

#include "Display.hpp"
#include "Raytracer.hpp"
#include "Scene.hpp"

namespace T3
{

/**
 * T3 Application.
 */
class Application
{
public:
    Application();
    ~Application();

    /// Gets the current display.
    Display *getDisplay();

    /// Gets the current scene.
    Scene *getScene();

    /// Sets the current scene.
    void setScene(Scene *newScene);

    /// Initializes the applications.
    bool initialize(int argc, const char *argv[]);

    /// Enters into the application main loop.
    void run();

    /// Notification about the main window being closed.
    void quit();

    /// Shutdowns the application.
    void shutdown();

    /// Updates the application.
    void update(float delta);

    // Gets the camera velocity.
    const Vector3 &getVelocity();

    // Set the camera velocity.
    void setVelocity(const Vector3 &newVelocity);

    // Gets the camera angular velocity.
    const Vector3 &getAngularVelocity();

    // Set the camera angular velocity.
    void setAngularVelocity(const Vector3 &newVelocity);

    // Gets the rotation.
    const Vector3 &getRotation() const;

    // Sets the rotation.
    void setRotation(const Vector3 &rotation);

private:
    void createScene();

    Display display;
    Raytracer raytracer;
    Scene *scene;

    // Camera control.
    Vector3 velocity;
    Vector3 rotation;
    Vector3 angularVelocity;
    float elapsedTime;

    // Scene mutex.
    Mutex mutex;
};

} // namespace T3

#endif //T3_APPLICATION_HPP

