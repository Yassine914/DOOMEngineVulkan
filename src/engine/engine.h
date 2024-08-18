#include <DEngine.h>
#include "../core/window.h"

class Engine
{
    private:
    Window *window;
    vk::Instance vkInstance{nullptr};
    vk::DebugUtilsMessengerEXT DebugMessenger{nullptr};

    public:
    Engine();

    void MakeVKInstance(std::string name);
    bool VKSupports(std::vector<const char *> &extensions, std::vector<const char *> &layers);

    bool RunEngine()
    {
        window->NewFrame();
        return !window->WindowShouldClose();
    }

    ~Engine();
};