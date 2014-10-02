#ifndef T3_DISPLAY_HPP
#define T3_DISPLAY_HPP

#include <SDL/SDL.h>
#include "Image.hpp"

namespace T3
{

class Application;

/**
 * T3 display.
 */
class Display
{
public:
    Display(Application *app);
    ~Display();

    int getWidth() const;
    int getHeight() const;

    bool initialize();
    void run();
    void setImage(Image2D *image);

private:
    void keyDown(SDLKey key);
    void keyUp(SDLKey key);

    void receiveEvents();
    void displayFrame();
    void convertCurrentImage();

    // The application.
    Application *app;

    // Window extents.
    int width, height, bpp;

    // Surface
    SDL_Surface *mainSurface;

    // Application flags.
    bool quit;

    // Image.
    SDL_mutex *mutex;
    Image2D *currentImage;
    int frameCount;
    int lastDisplayedFrame;
};


}

#endif //T3_DISPLAY_HPP

