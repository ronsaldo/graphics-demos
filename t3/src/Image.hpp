#ifndef T3_IMAGE_HPP
#define T3_IMAGE_HPP

#include "Color.hpp"

namespace T3
{

/**
 * Simple image 2d.
 */
class Image2D
{
public:
    Image2D(int w, int h);
    ~Image2D();

    int getWidth() const;
    int getHeight() const;
    const Color *getPixels() const;
    Color *getPixels();

private:
    int width, height;
    Color *pixels;
};

}

#endif //T3_IMAGE_HPP

