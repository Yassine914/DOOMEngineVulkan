#pragma once

#include <DEngine.h>
#include "../core/window.h"
#include "../engine/engine.h"

class App
{
    private:
    Engine *graphicsEngine;
    Window *window;

    f64 lastTime, currentTime;
    i32 numFrames;
    f32 frameTime;

    private:
    void CalculateFrameRate();

    public:
    App(i32 width, i32 height);

    void Run();

    ~App();
};