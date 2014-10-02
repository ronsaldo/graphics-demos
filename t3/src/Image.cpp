#include "Image.hpp"

namespace T3
{

Image2D::Image2D(int w, int h)
    : width(w), height(h)
{
    pixels = new Color[w*h];
}

Image2D::~Image2D()
{
    delete [] pixels;
}

int Image2D::getWidth() const
{
    return width;
}

int Image2D::getHeight() const
{
    return height;
}

const Color *Image2D::getPixels() const
{
    return pixels;
}

Color *Image2D::getPixels()
{
    return pixels;
}

}

