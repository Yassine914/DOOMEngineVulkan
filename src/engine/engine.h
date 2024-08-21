#include <DEngine.h>
#include "../core/window.h"

class Engine
{
    private:
    Window *window;

    //_____ VK VARS _____
    vk::Instance vkInstance{nullptr};

    // debug callback and dynamic loader
    vk::DebugUtilsMessengerEXT debugMessenger{nullptr};
    vk::DispatchLoaderDynamic dldi;

    // device
    vk::PhysicalDevice physicalDevice{nullptr};

    public:
    Engine();

    //_____ VK SPECIFIC _____
    void MakeVKInstance(std::string name);
    void MakeVKDebugMessenger();
    void ChooseVKPhysicalDevice();

    //_____ ENGINE SPECIFIC _____
    bool RunEngine()
    {
        window->NewFrame();
        return !window->WindowShouldClose();
    }

    ~Engine();
};