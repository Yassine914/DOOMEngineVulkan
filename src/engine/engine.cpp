#include "engine.h"

Engine::Engine() : window{new Window()}
{
    //_____ WINDOW INIT ______
    window->SetWindowSize(1920 / 2, 1080 / 2);
    window->SetFullscreen(false);
    window->SetResizeable(true);
    window->SetVSync(true);

    window->InitializeWindow();

    //_____ VULKAN INIT _____
    MakeVKInstance(window->GetTitle());
    dldi = vk::DispatchLoaderDynamic(vkInstance, vkGetInstanceProcAddr);

    if(ENGINE_DEBUG)
        MakeVKDebugMessenger();

    ChooseVKPhysicalDevice();
}

bool VKSupports(std::vector<const char *> &extensions, std::vector<const char *> &layers)
{
    // extensions
    std::vector<vk::ExtensionProperties> supportedExt = vk::enumerateInstanceExtensionProperties();
    {
        std::string msg{};
        for(vk::ExtensionProperties extProp : supportedExt)
        {
            const char *extName = extProp.extensionName;
            msg.append("\t\t").append(extName).append("\n");
        }
        LDEBUG(true, "device can support these extensions:\n" << msg);
    }

    for(const char *extension : extensions)
    {
        bool found = false;
        for(vk::ExtensionProperties extProp : supportedExt)
        {
            if(strcmp(extension, extProp.extensionName) == 0)
            {
                found = true;
                LDEBUG(true, "extension: " << extension << " is supported.\n");
            }
        }

        if(!found)
        {
            LERROR("extension: " << extension << " is not suppoted.\n");
            return false;
        }
    }

    // layers
    std::vector<vk::LayerProperties> supportedLayers = vk::enumerateInstanceLayerProperties();
    {
        std::string msg{};
        for(vk::LayerProperties lyProp : supportedLayers)
        {
            const char *lyName = lyProp.layerName;
            msg.append("\t\t").append(lyName).append("\n");
        }
        LDEBUG(true, "device can support these layers: \n" << msg);
    }

    for(const char *layer : layers)
    {
        bool found = false;
        for(vk::LayerProperties lyProp : supportedLayers)
        {
            if(strcmp(layer, lyProp.layerName) == 0)
            {
                found = true;
                LDEBUG(true, "layer: " << layer << " is supported.\n");
            }
        }

        if(!found)
        {
            LERROR("layer: " << layer << " is not suppoted.\n");
            return false;
        }
    }

    return true;
}

void Engine::MakeVKInstance(std::string name)
{
    //_____ VK APP INFO _____
    u32 version = 0;
    vkEnumerateInstanceVersion(&version);

    LINFO(false, "using vulkan v" << VK_API_VERSION_MAJOR(version) << "." << VK_API_VERSION_MINOR(version) << "\n");

    // clang-format off
    version = VK_MAKE_API_VERSION(
        VK_API_VERSION_VARIANT(version),
        VK_API_VERSION_MAJOR(version),
        VK_API_VERSION_MINOR(version),
        0 // set patch number to 0 (support all patches)
    );

    vk::ApplicationInfo appInfo = vk::ApplicationInfo(
        name.c_str(),
        version,
        "DOOMEngine",
        version,
        version
    );

    //_____ EXTENTIONS _____
    u32 glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if(ENGINE_DEBUG)
        extensions.push_back("VK_EXT_debug_utils");

    //______ VALIDATION LAYERS ______
    std::vector<const char *> layers;

    if(ENGINE_DEBUG)
        layers.push_back("VK_LAYER_KHRONOS_validation");

    // _____ VK INSTANCE _____
    if(!VKSupports(extensions, layers)) {
        LERROR("some extensions and/or layers are not supported\n");
        return;
    }

    vk::InstanceCreateInfo createInfo(
        vk::InstanceCreateFlags(),
        &appInfo,
        (u32) layers.size(), layers.data(), // enabled layers
        (u32) extensions.size(), extensions.data() // enabled extensions
    );

    try
    {
        vkInstance = vk::createInstance(createInfo, nullptr);
    }
    catch(vk::SystemError err)
    {
        LERROR("failed to create a vulkan instance: " << err.what() << "\n");
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagBitsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData
)
{
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        LERROR("vk validation layer: " << pCallbackData->pMessage << "\n");
    
    return VK_FALSE;
}

void Engine::MakeVKDebugMessenger()
{
    vk::DebugUtilsMessengerCreateInfoEXT createInfo = vk::DebugUtilsMessengerCreateInfoEXT(
        vk::DebugUtilsMessengerCreateFlagsEXT(),
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        (PFN_vkDebugUtilsMessengerCallbackEXT) debugCallback,
        (void *) nullptr,
        (const void *) nullptr
    );

    debugMessenger = vkInstance.createDebugUtilsMessengerEXT(createInfo, nullptr, dldi);
}

void LogPhysicalDeviceProperties(vk::PhysicalDevice device)
{
    vk::PhysicalDeviceProperties prop = device.getProperties();

    LINFO(true, "physical device: \n");
    LINFO(true, "\tdevice name: " << prop.deviceName << "\n");

    std::string deviceType{};
    switch(prop.deviceType)
    {
        case vk::PhysicalDeviceType::eCpu:
            deviceType = "CPU";
            break;
        case vk::PhysicalDeviceType::eDiscreteGpu:
            deviceType = "Discrete GPU";
            break;
        case vk::PhysicalDeviceType::eIntegratedGpu:
            deviceType = "Integrated GPU";
            break;
        case vk::PhysicalDeviceType::eVirtualGpu:
            deviceType = "Virtual GPU";
            break;
        case vk::PhysicalDeviceType::eOther:
            deviceType = "Unknown GPU/CPU";
            break;
        default:
            deviceType = "Other";
            break;
    }
    LINFO(true, "\tdevice type: " << deviceType << "\n");
}

bool VKPhysicalDeviceSuitable(vk::PhysicalDevice device)
{
    const std::vector<const char *> requiredEXT = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    if(ENGINE_DEBUG)
    {
        LINFO(true, "requesting device extensions: \n");
        for(const char *ext : requiredEXT)
            LINFO(true, "\t" << ext << "\n");
    }

    std::set<std::string> req(requiredEXT.begin(), requiredEXT.end());

    if(ENGINE_DEBUG)
        LDEBUG(true, "supported extensions: \n");
    
    for(vk::ExtensionProperties &ext : device.enumerateDeviceExtensionProperties())
    {
        if(ENGINE_DEBUG)
            LDEBUG(true, "\t" << ext.extensionName << "\n");
        
        // remove from temp list
        req.erase(ext.extensionName);
    }

    // true if req is empty (meaning all exts are supported)
    return req.empty();
}

i32 VKRatePhysicalDevice(vk::PhysicalDevice device)
{
    // TODO: add props and features depending on engine
    //      capabilities and requirements.
    i32 score;

    //_____ DEVICE PROPERTIES _____
    vk::PhysicalDeviceProperties deviceProp = device.getProperties();
    if(deviceProp.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
        score += 1000;

    score += deviceProp.limits.maxImageDimension2D;

    //_____ DEVICE FEATURES _____
    vk::PhysicalDeviceFeatures deviceFeat = device.getFeatures();
    if(!deviceFeat.geometryShader)
        return 0;

    return score;
}

void Engine::ChooseVKPhysicalDevice()
{
    std::vector<vk::PhysicalDevice> devices = vkInstance.enumeratePhysicalDevices();
    std::multimap<i32, vk::PhysicalDevice> candidates;

    if(devices.size() == 0)
        LERROR("failed to find any GPUs to run on.\n");

    if(ENGINE_DEBUG)
        LINFO(true, "there are " << devices.size() << " physical device(s)\n");
    
    for(vk::PhysicalDevice device : devices)
    {
        if(ENGINE_DEBUG)
            LogPhysicalDeviceProperties(device);

        if(VKPhysicalDeviceSuitable(device))
        {
            // rank devices
            i32 score = VKRatePhysicalDevice(device);
            candidates.insert(std::make_pair(score, device));

            
        }
    }

    // check if the last candidate has a score greater than 0.
    // and the last candidate should have the greater value (ordered map)
    if(candidates.rbegin()->first > 0)
    {
        physicalDevice = candidates.rbegin()->second;
        
        if(ENGINE_DEBUG)
        {
            LINFO(true, "selected GPU (physical device): \n");
            LogPhysicalDeviceProperties(physicalDevice);
        }
    }
    else
    {
        LERROR("couldn't find a suitable GPU with Vulkan Support\n");
        exit(101);
    }
}

// clang-format on

//_____ QUEUE FAMILIES _____
struct QueueFamilyIndices
{
    std::optional<u32> graphicsFamily;
    std::optional<u32> presentFamily;
};

Engine::~Engine()
{
    vkInstance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dldi);
    vkInstance.destroy();
    delete window;
}