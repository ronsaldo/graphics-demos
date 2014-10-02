#include "Display.hpp"
#include "Application.hpp"

namespace T3
{

const float Speed = 10.0f;
const float AngularSpeed = 2.0f;

Display::Display(Application *app)
    : app(app)
{
    quit = false;
    width = 640;
    height = 480;
    bpp = 32;
    frameCount = 0;
    lastDisplayedFrame = -1;
    currentImage = NULL;
}

Display::~Display()
{
    if(currentImage)
        delete currentImage;
}

int Display::getWidth() const
{
    return width;
}

int Display::getHeight() const
{
    return height;
}

bool Display::initialize()
{
    // Initialize sdl.
    if(SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "Failed to initialize SDL\n");
        return false;
    }

    // Create the video mode.
    mainSurface = SDL_SetVideoMode(width, height, bpp, SDL_HWSURFACE|SDL_DOUBLEBUF);
    if(!mainSurface)
    {
        fprintf(stderr, "Failed to set the video mode\n");
        return false;
    }

    // Set the window title.
    SDL_WM_SetCaption("T3 Terrain Raytracer", NULL);

    // Create the mutex.
    mutex = SDL_CreateMutex();

    return true;

}

void Display::keyDown(SDLKey key)
{
    Vector3 velocity = app->getVelocity();
    Vector3 angularVelocity = app->getAngularVelocity();

    switch(key)
    {
    case SDLK_a:
        velocity.x = -1;
        break;
    case SDLK_d:
        velocity.x = 1;
        break;
    case SDLK_w:
        velocity.z = 1;
        break;
    case SDLK_s:
        velocity.z = -1;
        break;
    case SDLK_LCTRL:
        velocity.y = -1;
        break;
    case SDLK_SPACE:
        velocity.y = 1;
        break;
    case SDLK_ESCAPE:
        quit = true;
        break;
    case SDLK_LEFT:
        angularVelocity.y = 1;
        break;
    case SDLK_RIGHT:
        angularVelocity.y = -1;
        break;
    case SDLK_UP:
        angularVelocity.x = 1;
        break;
    case SDLK_DOWN:
        angularVelocity.x = -1;
        break;
    default:
        break;
    }

    if(velocity.length2() > 0.01)
        velocity = velocity.normalized();
    if(angularVelocity.length2() > 0.01)
        angularVelocity = angularVelocity.normalized();
    app->setVelocity(velocity*Speed);
    app->setAngularVelocity(angularVelocity*AngularSpeed);
}

void Display::keyUp(SDLKey key)
{
    Vector3 velocity = app->getVelocity();
    Vector3 angularVelocity = app->getAngularVelocity();

    switch(key)
    {
    case SDLK_a:
        if(velocity.x < 0)
            velocity.x = 0;
        break;
    case SDLK_d:
        if(velocity.x > 0)
            velocity.x = 0;
        break;
    case SDLK_w:
        if(velocity.z > 0)
            velocity.z = 0;
        break;
    case SDLK_s:
        if(velocity.z < 0)
            velocity.z = 0;
        break;
    case SDLK_LCTRL:
        if(velocity.y < 0)
            velocity.y = 0;
        break;
    case SDLK_SPACE:
        if(velocity.y > 0)
            velocity.y = 0;
        break;
    case SDLK_LEFT:
        if(angularVelocity.y > 0)
            angularVelocity.y = 0;
        break;
    case SDLK_RIGHT:
        if(angularVelocity.y < 0)
            angularVelocity.y = 0;
        break;
    case SDLK_UP:
        if(angularVelocity.x > 0)
            angularVelocity.x = 0;
        break;
    case SDLK_DOWN:
        if(angularVelocity.x < 0)
            angularVelocity.x = 0;
        break;
    default:
        break;
    }

    if(velocity.length2() > 0.1)
        velocity = velocity.normalized();
    if(angularVelocity.length2() > 0.01)
        angularVelocity = angularVelocity.normalized();
    app->setVelocity(velocity*Speed);
    app->setAngularVelocity(angularVelocity*AngularSpeed);
}

void Display::receiveEvents()
{
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
        case SDL_KEYDOWN:
            keyDown(event.key.keysym.sym);
            break;
        case SDL_KEYUP:
            keyUp(event.key.keysym.sym);
            break;
        case SDL_QUIT:
            quit = true;
            break;
        }
    }
}


void Display::displayFrame()
{
    // Check the presence of a current image.
    SDL_LockMutex(mutex);
    int frameCount = this->frameCount;
    if(!currentImage || lastDisplayedFrame == frameCount)
    {
        SDL_UnlockMutex(mutex);
        return;
    }

    // Lock the main surface.
    if(SDL_MUSTLOCK(mainSurface))
        SDL_LockSurface(mainSurface);

    // Convert the current image.
    convertCurrentImage();

    // Set the new last displayed frame.
    lastDisplayedFrame = frameCount;

    // Unlock the surface and the mutex.
    if(SDL_MUSTLOCK(mainSurface))
        SDL_UnlockSurface(mainSurface);

    SDL_UnlockMutex(mutex);

    // Display the changes.
    SDL_Flip(mainSurface);
}

inline float clampChannel(float v)
{
    return (v > 1.0f) ? 1.0f : ((v < 0.0f) ? 0.0f : v);
}

inline Uint8 convertFloatChannel(float v)
{
    return (Uint8)(clampChannel(v)*255.0f + 0.5f);
}

void Display::convertCurrentImage()
{
    Color *src = currentImage->getPixels();
    int *dst = static_cast<int*> (mainSurface->pixels);
    SDL_PixelFormat *format = mainSurface->format;

    for(int y = 0; y < height; ++y)
    {
        int *dstRow = dst + (mainSurface->pitch/4)*y;
        Color *srcRow = src + y*width;
        for(int x = 0; x < width; ++x)
        {
            Color sc = srcRow[x];
            int dc = (convertFloatChannel(sc.r)<<format->Rshift) |
                        (convertFloatChannel(sc.g)<<format->Gshift) |
                        (convertFloatChannel(sc.b)<<format->Bshift);
            dstRow[x] = dc;
        }
    }
}

void Display::run()
{
    Uint32 newTime = SDL_GetTicks();
    Uint32 oldTime, deltaTime;
    int lastFrameCount = 0;
    Uint32 fpsTime = 0;
    oldTime = newTime;

    while(!quit)
    {
        // Compute the delta.
        newTime = SDL_GetTicks();
        deltaTime = newTime - oldTime;
        oldTime = newTime;

        // FPS display.
        fpsTime += deltaTime;
        if(fpsTime >= 1000)
        {
            char title[128];
            float scale = 1000.0f/fpsTime;
            sprintf(title, "T3 Terrain Raytracer - %03.2f FPS", (lastDisplayedFrame - lastFrameCount)*scale);
            SDL_WM_SetCaption(title, NULL);
            lastFrameCount = lastDisplayedFrame;
            fpsTime = 0;
        }

        // Process the events and display.
        app->update(deltaTime*0.001f);
        receiveEvents();
        displayFrame();

        // Don't eat the CPU.
        SDL_Delay(5);
    }

    // Notify the application.
    app->quit();
}

void Display::setImage(Image2D *image)
{
    SDL_LockMutex(mutex);
    delete currentImage;
    currentImage = image;
    frameCount++;
    SDL_UnlockMutex(mutex);
}

}
   
