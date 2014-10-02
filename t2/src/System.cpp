#include <stdio.h>
#include <SDL/SDL.h>
#include <GL/glew.h>
#include "System.hpp"

namespace Tarea2
{

// Animation/physics intervals.
const Uint32 UpdateInterval = 1000/60;

System::System()
{
    width_ = 640;
    height_ = 480;
}

System::~System()
{
}

int System::getWidth() const
{
    return width_;
}

int System::getHeight() const
{
    return height_;
}

void System::setTitle(const std::string &title)
{
    SDL_WM_SetCaption(title.c_str(), NULL);
}

bool System::initialize(int argc, const char *argv[])
{
    // Initialize SDL.
    int ret = SDL_Init(SDL_INIT_VIDEO);
    if(ret)
    {
        fprintf(stderr, "Failed to initialize SDL\n");
        return false;
    }

    // Initialize OpenGL
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 4);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);

    if(!SDL_SetVideoMode(width_, height_, 32, SDL_OPENGL))
    {
        fprintf(stderr, "Failed to set video mode.\n");
        return false;
    }

    // Initialize GLEW
    GLenum err = glewInit();
    if(GLEW_OK != err)
    {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        return -1;
    }

    return true;
}

void System::mainloop()
{
    Uint32 newTime = SDL_GetTicks();
    Uint32 oldTime, deltaTime;
    int framesCount = 0;
    Uint32 fpsTime = 0;
    oldTime = newTime;

    // Performs updates in intervals.
    Uint32 availableTime = 0;

    // Main loop
    quitting_ = false;
    while(!quitting_)
    {
        // Read events from SDL.
        sendEvents();

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
            sprintf(title, "%03.2f FPS", framesCount*scale);
            setTitle(title);
            framesCount = 0;
            fpsTime = 0;
        }

        // Add available time.
        availableTime += deltaTime;

        // Perform updates by intervals.
        while(availableTime >= UpdateInterval)
        {     
            float delta = UpdateInterval*0.001;
            FrameUpdateEvent updateEvent(Event::FrameUpdate, delta);
            fire(&updateEvent);
            availableTime -= UpdateInterval;
        }

        // Send a frame event.
        Event drawEvent(Event::FrameDraw);
        fire(&drawEvent);

        // Swap buffers.
        SDL_GL_SwapBuffers();
        framesCount++;

        // Don't eat the CPU.
        int sleepTime = 5 - deltaTime;
        if(sleepTime > 0)
            SDL_Delay(sleepTime);
    }
}


void System::shutdown()
{
    SDL_Quit();
}

void System::quit()
{
    quitting_ = true;
}

void System::sendEvents()
{
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
        case SDL_KEYUP:
        case SDL_KEYDOWN:
            {
                KeyEvent keyEvent(event.key.state == SDL_PRESSED ? Event::KeyDown : Event::KeyUp,
                               event.key.keysym.sym);
                fire(&keyEvent);
            }
            break;
        case SDL_ACTIVEEVENT:
            {
                if(event.active.state == SDL_APPMOUSEFOCUS)
                {
                    Event ev(event.active.gain ? Event::MouseEnter : Event::MouseLeave);
                    fire(&ev);
                }
            }
            break;
        case SDL_MOUSEMOTION:
            {
                MouseMoveEvent moveEvent(Event::MouseMove, event.motion.x, event.motion.y);
                fire(&moveEvent);
            }
            break;
        case SDL_QUIT:
            quit();
            break;
        }
    }
}

}

