#include <SDL/SDL.h>
#include <stdio.h>

#include "Application.hpp"

using namespace T3;

int main(int argc, const char *argv[])
{
    Application app;
    if(!app.initialize(argc, argv))
        return -1;
    app.run();
    
    return 0;
}

