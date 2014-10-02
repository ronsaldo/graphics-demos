#ifndef T3_RAYTRACER_HPP
#define T3_RAYTRACER_HPP

#include "Vector3.hpp"
#include "Threading.hpp"
#include <CL/cl.h>

namespace T3
{
class Application;
class SceneDataHolder;

/**
 * T3 raytracer frame buffer.
 */
class FrameBuffer
{
public:
    void setArguments(cl_kernel kernel, int start);
    bool create(cl_context context, int width, int height);
    void release();

    cl_mem colorBuffer;
};

/**
 * T3 raytracer.
 */
class Raytracer
{
public:
    Raytracer(Application *app);
    ~Raytracer();

    /// Initializes the raytracer.
    bool initialize();

    /// Shuts down the raytracer.
    void shutdown();

private:
    bool initializeRaytracerThread();
    bool initializeOpenCL();
    bool createResources();
    bool createImages();

    void shutdownOpenCL();
    void raytracerJob();

    // Main scene.
    void clearFrameBuffer();
    void swapBuffers();
    void uploadScene();
    void castPrimaryRays();
    void displayFrameBuffer();

    // Sky
    void createNightSky();
    void createDaySky();
    void createSky();

    static int threadEntryPoint(void *obj);
    int threadEntry();

    Application *app;

    // Width, height.
    size_t width, height;
    size_t skyWidth, skyHeight;

    // Raytracer thread and mutex.
    SDL_Thread *thread;
    Mutex threadMutex;

    // Last sun direction.
    Vector3 lastSunDir;

    // Thread initialitation condition.
    bool threadStartedSuccess;
    Condition threadStartedCond;

    // Thread finish flag.
    bool threadFinishFlag;

    // OpenCL
    int selectedPlatform;
    cl_context computeContext;
    cl_device_id computeDevice;
    cl_command_queue commandQueue;
    cl_program raytracerProgram;

    // Images.
    cl_mem imagesDescBuffer;
    cl_mem imagesBuffer;

    // Frame buffers.
    FrameBuffer backBuffer, frontBuffer;

    // Scene data.
    SceneDataHolder *sceneData;
    cl_mem sceneDataBuffer;

    // Kernels
    cl_kernel primaryRaysKernel;
    cl_kernel daySkyCreationKernel;
    cl_kernel nightSkyCreationKernel;
};

} // namespace T3

#endif //T3_RAYTRACER_HPP

