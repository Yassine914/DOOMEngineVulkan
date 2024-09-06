#include "DEngine.h"
#include "app/app.h"

int main()
{
    LINFO(false, "DOOM Engine v0.0.1\n");

    App *app = new App(1920 / 2, 1080 / 2);

    app->Run();

    delete app;

    std::vector<std::pair<i32, u32>> hellos{

    };
}