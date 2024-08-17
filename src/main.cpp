#include "core/defines.h"
#include "core/logger.h"

#include "core/window.h"

int main()
{
    LINFO(false, "DOOM Engine v0.0.1\n");

    Window *window = new Window(720, 640, "DOOM Engine Vulkan v0.0.1", false, false);
    window->InitializeWindow();

    while(!window->WindowShouldClose())
    {
        window->NewFrame();
    }

    return 0;
}