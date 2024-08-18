#include "DEngine.h"
#include "engine/engine.h"

int main()
{
    LINFO(false, "DOOM Engine v0.0.1\n");

    Engine *engine = new Engine();

    while(engine->RunEngine())
    {
    }

    return 0;
}