#include "engine.h"

Engine::Engine()
{
    // instance
    vkInstance = DE::VK::MakeInstance("DOOM Engine");

#ifdef DDEBUG // debug messenger
    dispatchLoader = vk::DispatchLoaderDynamic(vkInstance, vkGetInstanceProcAddr);
    debugMessenger = DE::VK::MakeDebugMessenger(vkInstance, dispatchLoader);
#endif

    // devices
    phyDevice = DE::VK::ChoosePhysicalDevice(vkInstance);

    DE::VK::DeviceProperties props;
    props         = DE::VK::MakeLogicalDevice(phyDevice);
    device        = props.device;
    graphicsQueue = DE::VK::GetDeviceQueue(device, props.indices.graphicsFamily.value(), 0);
}

Engine::~Engine()
{
    device.destroy();

#ifdef DDEBUG // debug messenger
    vkInstance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dispatchLoader);
#endif

    vkInstance.destroy();
}