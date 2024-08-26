#include "app.h"

App::App(i32 width, i32 height) : window{new Window()}
{
    //_____ WINDOW INIT ______
    window->SetWindowSize(width, height);
    window->SetFullscreen(false);
    window->SetResizeable(true);
    window->SetVSync(true);

    window->InitializeWindow();

    //_____ ENGINE INIT _____
    graphicsEngine = new Engine(width, height, window);
}

void App::CalculateFrameRate()
{
    currentTime = window->GetCurrentTime();
    f64 deltaTime = currentTime - lastTime;

    if(deltaTime >= 1)
    {
        i32 framerate = std::max(1, i32(numFrames / deltaTime));

        std::stringstream title;
        title << "DOOM Engine -- FPS: " << framerate;
        window->SetWindowTitle(title.str());

        lastTime = currentTime;
        numFrames = -1;
        frameTime = f32(1000.0 / framerate);
    }

    numFrames++;
}

void App::Run()
{
    while(!window->WindowShouldClose())
    {
        window->NewFrame();

        graphicsEngine->Render();

        CalculateFrameRate();
    }
}

App::~App()
{
    delete graphicsEngine;
    delete window;
}