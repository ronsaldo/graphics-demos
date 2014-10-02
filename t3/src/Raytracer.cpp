#include <string>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <string.h>
#include "Vector4.hpp"
#include "Application.hpp"
#include "Raytracer.hpp"
#include "Display.hpp"
#include "Scene.hpp"
#include "Image.hpp"

namespace T3
{

inline std::vector<char> readWholeFile(const char* filename)
{
    // open the file:
    std::ifstream file(filename, std::ios::binary);

    // read the data:
    return std::vector<char> ((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
}

Raytracer::Raytracer(Application *app)
    : app(app)
{
    selectedPlatform = 0;
    sceneData = NULL;
    sceneDataBuffer = NULL;
}

Raytracer::~Raytracer()
{
}

bool Raytracer::initialize()
{
    // Get the width and the height of the surface.
    Display *display = app->getDisplay();
    width = display->getWidth();
    height = display->getHeight();
    skyWidth = 512;
    skyHeight = 512;

    // Reset the thread finish flag.
    threadFinishFlag = false;

    // Start the thread.
    Lock l(threadMutex);
    thread = SDL_CreateThread(&threadEntryPoint, this);

    // Wait for initialization to finish.
    threadStartedCond.wait(l);
    bool result = threadStartedSuccess;
    return result;
}

void Raytracer::shutdown()
{
    // Set the thread finish flag.
    {
        Lock l(threadMutex);
        threadFinishFlag = true;
    }

    // Wait the thread to finish.
    SDL_WaitThread(thread, NULL);
}

bool Raytracer::initializeOpenCL()
{
    char buffer[1024];

    // Query the number of platforms.
    cl_uint numplatforms;
    clGetPlatformIDs(0, NULL, &numplatforms);

    // Query the platforms
    cl_platform_id *platforms = new cl_platform_id[numplatforms];
    clGetPlatformIDs(numplatforms, platforms, &numplatforms);

    printf("OpenCL platforms[%d]\n", numplatforms);
    for(cl_uint i = 0; i < numplatforms; ++i)
    {
        // Print the name and the vendor.
        clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, sizeof(buffer), buffer, NULL);
        if(i == (cl_uint)selectedPlatform)
            printf("[Selected]");
        printf("Name: %s\n", buffer);
        clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, sizeof(buffer), buffer, NULL);
        printf("Vendor: %s\n", buffer);

    }

    cl_platform_id currentPlatform = platforms[selectedPlatform];
    delete [] platforms;

    // The context properties.
    cl_context_properties contextProperties[] = {
         CL_CONTEXT_PLATFORM, (cl_context_properties)currentPlatform,
         0,
    };

    // Try to create a GPU only context.
    cl_int errCode;
    computeContext = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_GPU , NULL, NULL, &errCode);

    // Try with any other device, such as a CPU.
    if(!computeContext)
        computeContext = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_DEFAULT , NULL, NULL, &errCode);

    // Check the error code.
    if(!computeContext)
    {
        const char *msg = "Unknown error.";

        switch(errCode)
        {
        case CL_INVALID_PLATFORM:
            msg = "Invalid platform.";
            break;
        case CL_INVALID_DEVICE_TYPE:
            msg = "Invalid device type.";
            break;
        case CL_DEVICE_NOT_AVAILABLE:
            msg = "Device not available.";
            break;
        case CL_DEVICE_NOT_FOUND:
            msg = "Device not found.";
            break;
        default:
            break;
        }

        fprintf(stderr, "Failed to create OpenCL context: %s\n", msg);
        return false;
    }

    // Query the compute device.
    // Assume that there's at least one.
    clGetContextInfo(computeContext, CL_CONTEXT_DEVICES, sizeof(computeDevice), &computeDevice, NULL);

    // Create the command queue.
    commandQueue = clCreateCommandQueue(computeContext, computeDevice, 0, NULL);
    if(!commandQueue)
    {
        fprintf(stderr, "Failed to create the command queue.\n");
        return false;
    }

    return true;
}

void Raytracer::shutdownOpenCL()
{
    clReleaseProgram(raytracerProgram);
    frontBuffer.release();
    backBuffer.release();
    clReleaseCommandQueue(commandQueue);
    clReleaseContext(computeContext);
}


bool Raytracer::createResources()
{
    // Create the frame buffers.
    if(!frontBuffer.create(computeContext, width, height) ||
        !backBuffer.create(computeContext, width, height) ||
        !createImages())
        return false;

    // Load the raytracer program
    std::vector<char> sourceCode = readWholeFile("cl/Raytracer.cl");
    size_t sourceCodeSize = sourceCode.size();
    const char *sourceCodePtr = &sourceCode[0];
    raytracerProgram = clCreateProgramWithSource(computeContext, 1, &sourceCodePtr, &sourceCodeSize, NULL);
    if(!raytracerProgram)
    {
        fprintf(stderr, "Failed to create the raytracer program.\n");
        return false;
    }

    // Build the raytracer program.
    cl_int error = clBuildProgram(raytracerProgram, 1, &computeDevice, "-D CL_RAYTRACER -x clc++ -I cl", NULL, NULL);
    size_t bufferSize;
    clGetProgramBuildInfo(raytracerProgram, computeDevice,  CL_PROGRAM_BUILD_LOG, 0, NULL, &bufferSize);
    char *buffer = new char[bufferSize+1];
    clGetProgramBuildInfo(raytracerProgram, computeDevice,  CL_PROGRAM_BUILD_LOG, bufferSize, buffer, NULL);
    fprintf(stderr, "%s\n", buffer);
    delete [] buffer;
    if(error != CL_SUCCESS)
    {
        fprintf(stderr, "Failed to build the raytracer program:\n");
        return false;
    }

    // Create the primary rays kernel
    primaryRaysKernel = clCreateKernel(raytracerProgram, "castPrimaryRays", NULL);
    if(!primaryRaysKernel)
    {
        fprintf(stderr, "Failed to create the primary rays kernel.\n");
        return false;
    }

    // Create the day sky kernel
    daySkyCreationKernel = clCreateKernel(raytracerProgram, "createDaySky", NULL);
    if(!daySkyCreationKernel)
    {
        fprintf(stderr, "Failed to create the day sky generation kernel.\n");
        return false;
    }

    // Create the night sky kernel
    nightSkyCreationKernel = clCreateKernel(raytracerProgram, "createNightSky", NULL);
    if(!nightSkyCreationKernel)
    {
        fprintf(stderr, "Failed to create the night sky generation kernel.\n");
        return false;
    }

    return true;
}

bool Raytracer::createImages()
{
    unsigned int desc[] = {
      0, (unsigned int)skyWidth, (unsigned int)skyHeight,
    };

    size_t bufferSize = skyWidth*skyHeight*sizeof(Color);
    imagesDescBuffer = clCreateBuffer(computeContext,  CL_MEM_READ_ONLY |  CL_MEM_COPY_HOST_PTR , sizeof(desc), desc, NULL);
    imagesBuffer = clCreateBuffer(computeContext, CL_MEM_READ_WRITE, bufferSize, NULL, NULL);
    if(!imagesDescBuffer || !imagesBuffer)
    {
        fprintf(stderr, "Failed to create images buffers.\n");
        return false;
    }

    return true;
}


bool Raytracer::initializeRaytracerThread()
{
    if(!initializeOpenCL())
        return false;

    if(!createResources())
        return false;
    createImages();

    // TODO: Try to uploada changing scene.
    uploadScene();
    return true;
}

int Raytracer::threadEntryPoint(void *obj)
{
    return static_cast<Raytracer*> (obj)->threadEntry();
}

int Raytracer::threadEntry()
{
    // Thread iniitialization.
    bool success = initializeRaytracerThread();
    {
        Lock l(threadMutex);
        threadStartedSuccess = success;
        threadStartedCond.broadcast();
        if(!success)
            return 0;
    }

    // Create the sky once.
    createSky();

    // Thread main loop.
    for(;;)
    {
        // Read the finish flag.
        {
            Lock l(threadMutex);
            if(threadFinishFlag)
                break;
        }

        // Perform the job.
        raytracerJob();
    }

    shutdownOpenCL();

    return 0;
}


void Raytracer::raytracerJob()
{
    if(lastSunDir != app->getScene()->getSunDirection())
        createSky();

    uploadScene();
    castPrimaryRays();
    displayFrameBuffer();
}

void Raytracer::swapBuffers()
{
    FrameBuffer temp = backBuffer;
    backBuffer = frontBuffer;
    frontBuffer = temp;
}

void Raytracer::uploadScene()
{
    if(sceneData)
        delete sceneData;
    if(sceneDataBuffer)
        clReleaseMemObject(sceneDataBuffer);

    sceneData = app->getScene()->getSceneData();
    sceneDataBuffer = clCreateBuffer(computeContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR , sceneData->getSize(), (void*)sceneData->getData(), NULL);
}

void Raytracer::createNightSky()
{
    // Choose the kernel
    cl_kernel kernel = nightSkyCreationKernel;

    // Set the buffer arguments.
    int offset = 0;
    int width = skyWidth;
    int height = skyHeight;
    clSetKernelArg(kernel, 0, sizeof(offset), &offset);
    clSetKernelArg(kernel, 1, sizeof(width), &width);
    clSetKernelArg(kernel, 2, sizeof(height), &height);
    clSetKernelArg(kernel, 3, sizeof(imagesBuffer), &imagesBuffer);

    // Stars
    Scene *scene = app->getScene();
    float skyRadius = scene->getSkyRadius();
    float starScale = scene->getStarScale();
    float starThreshold = scene->getStarThreshold();
    clSetKernelArg(kernel, 4, sizeof(skyRadius), &skyRadius);
    clSetKernelArg(kernel, 5, sizeof(starScale), &starScale);
    clSetKernelArg(kernel, 6, sizeof(starThreshold), &starThreshold);

    // Run the kernel.
    size_t globalWorkSize[] = {skyWidth, skyHeight};
    clEnqueueNDRangeKernel(commandQueue, kernel, 2, NULL, globalWorkSize, NULL, 0, NULL, NULL);
}

void Raytracer::createDaySky()
{
    // Choose the kernel
    cl_kernel kernel = daySkyCreationKernel;

    // Set the arguments.
    int offset = 0;
    int width = skyWidth;
    int height = skyHeight;
    clSetKernelArg(kernel, 0, sizeof(offset), &offset);
    clSetKernelArg(kernel, 1, sizeof(width), &width);
    clSetKernelArg(kernel, 2, sizeof(height), &height);
    clSetKernelArg(kernel, 3, sizeof(imagesBuffer), &imagesBuffer);

    // Set the sky parameters.
    Scene *scene = app->getScene();
    float skyRadius = scene->getSkyRadius();
    Color sunColor = scene->getSunColor();
    Vector4 sunDirection = scene->getSunDirection();
    clSetKernelArg(kernel, 4, sizeof(skyRadius), &skyRadius);
    clSetKernelArg(kernel, 5, sizeof(sunColor), &sunColor);
    clSetKernelArg(kernel, 6, sizeof(sunDirection), &sunDirection);

    // Run the kernel.
    size_t globalWorkSize[] = {skyWidth, skyHeight};
    clEnqueueNDRangeKernel(commandQueue, kernel, 2, NULL, globalWorkSize, NULL, 0, NULL, NULL);

    // Store the sun direction.
    lastSunDir = scene->getSunDirection();;
}

void Raytracer::createSky()
{
    if(app->getScene()->isDay())
        createDaySky();
    else
        createNightSky();
}

void Raytracer::castPrimaryRays()
{
    // Compute the camera parameters.
    Camera camera = app->getScene()->getCamera();
    Matrix3 orientation = camera.getOrientation();
    Vector4 cameraPosition = camera.getPosition();

    const Vector3 *screenPlaneVerts = camera.getScreenPlaneVerts();
    Vector4 screenPlaneVertsTrans[4];
    for(int i = 0; i < 4; ++i)
        screenPlaneVertsTrans[i] = orientation*screenPlaneVerts[i] + camera.getPosition();

    // Set the arguments.
    clSetKernelArg(primaryRaysKernel, 0, sizeof(sceneDataBuffer), &sceneDataBuffer);
    clSetKernelArg(primaryRaysKernel, 1, sizeof(cameraPosition), &cameraPosition);
    clSetKernelArg(primaryRaysKernel, 2, sizeof(screenPlaneVertsTrans[0]), &screenPlaneVertsTrans[0]);
    clSetKernelArg(primaryRaysKernel, 3, sizeof(screenPlaneVertsTrans[1]), &screenPlaneVertsTrans[1]);
    clSetKernelArg(primaryRaysKernel, 4, sizeof(screenPlaneVertsTrans[2]), &screenPlaneVertsTrans[2]);
    clSetKernelArg(primaryRaysKernel, 5, sizeof(screenPlaneVertsTrans[3]), &screenPlaneVertsTrans[3]);
    clSetKernelArg(primaryRaysKernel, 6, sizeof(imagesDescBuffer), &imagesDescBuffer);
    clSetKernelArg(primaryRaysKernel, 7, sizeof(imagesBuffer), &imagesBuffer);
    frontBuffer.setArguments(primaryRaysKernel, 8);

    // Run the kernel.
    size_t globalWorkSize[] = {width, height};
    clEnqueueNDRangeKernel(commandQueue, primaryRaysKernel, 2, NULL, globalWorkSize, NULL, 0, NULL, NULL);
}

void Raytracer::displayFrameBuffer()
{
    // Create the image.
    Image2D *image = new Image2D(width, height);

    // Read the frame buffer data.
    size_t origin[] = {0, 0, 0};
    size_t region[] = {width, height, 1};
    clEnqueueReadImage(commandQueue, frontBuffer.colorBuffer, CL_TRUE, origin, region,
            width*sizeof(Color), 0, image->getPixels(), 0, NULL, NULL);

    // Send the image to the display.
    app->getDisplay()->setImage(image);

}


//--------------------------------------------------------------
// Framebuffer.

bool FrameBuffer::create(cl_context context, int width, int height)
{   
    // Framebuffer format.
    cl_image_format framebufferFormat;
    memset(&framebufferFormat, 0, sizeof(framebufferFormat));
    framebufferFormat.image_channel_order = CL_RGBA;
    framebufferFormat.image_channel_data_type = CL_FLOAT;

    // Framebuffer description.
    cl_image_desc framebufferDesc;
    memset(&framebufferDesc, 0, sizeof(framebufferDesc));
    framebufferDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
    framebufferDesc.image_width = width;
    framebufferDesc.image_height = height;
    framebufferDesc.image_depth = 1;
    framebufferDesc.image_array_size = 1;

    // Create the buffers.
    colorBuffer = clCreateImage(context, CL_MEM_READ_WRITE, &framebufferFormat, &framebufferDesc,
                                        NULL, NULL);
    if(!colorBuffer)
    {
        fprintf(stderr, "Failed to create a framebuffer.\n");
        return false;
    }

    return true;
}

void FrameBuffer::setArguments(cl_kernel kernel, int start)
{
    clSetKernelArg(kernel, start, sizeof(colorBuffer), &colorBuffer);
}

void FrameBuffer::release()
{
    clReleaseMemObject(colorBuffer);
}

}

