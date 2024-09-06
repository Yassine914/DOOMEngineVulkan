#include "init.h"

namespace DE::VK {

// ______ VK INSTANCE ______
bool CheckExtensionSupport(std::vector<const char *> reqExtensions)
{
    std::vector<vk::ExtensionProperties> exts = vk::enumerateInstanceExtensionProperties();

#ifdef DDEBUG
    LTRACE(true, "supported vulkan extensions: \n");
    for(auto ext : exts)
    {
        LTRACE(true, "\text: " << ext.extensionName << "\n");
    }
    LTRACE(true, "----------------------------\n");
#endif

    for(const char *extName : reqExtensions)
    {
        bool found = false;

        for(const auto &extProp : exts)
        {
            if(strcmp(extName, extProp.extensionName) == 0)
            {
                found = true;
                break;
            }
        }

        if(!found)
        {
            LERROR("VULKAN ERROR: extension " << extName << " is not supported.\n");
            return false;
        }
    }

    return true;
}

bool CheckValidationLayerSupport(std::vector<const char *> reqLayers)
{
    std::vector<vk::LayerProperties> layers = vk::enumerateInstanceLayerProperties();

#ifdef DDEBUG
    LTRACE(true, "supported vulkan layers: \n");
    for(auto ly : layers)
    {
        LTRACE(true, "\tlayer: " << ly.layerName << "\n");
    }
    LTRACE(true, "----------------------------\n");
#endif

    for(const char *layerName : reqLayers)
    {
        bool found = false;

        for(const auto &layerProp : layers)
        {
            if(strcmp(layerName, layerProp.layerName) == 0)
            {
                found = true;
                break;
            }
        }

        if(!found)
        {
            LERROR("VULKAN ERROR: validation layer " << layerName << " is not supported.\n");
            return false;
        }
    }

    return true;
}

// clang-format off
vk::Instance MakeInstance(std::string name)
{
    //_____ VK VERSION ____
    u32 version = vk::enumerateInstanceVersion();

#ifdef DDEBUG
    LINFO(false, "Using Vulkan v" << vk::apiVersionMajor(version) << '.' << vk::apiVersionMinor(version) << "\n");
#endif

    version = vk::makeApiVersion(
        (u32) vk::apiVersionVariant(version),
        (u32) vk::apiVersionMajor(version),
        (u32) vk::apiVersionMinor(version),
        (u32) 0 // set patch no. to 0 (for maximum support)
    );

    vk::ApplicationInfo appInfo(
        name.c_str(),
        version,
        "DOOM Engine",
        version,
        version
    );

    //____ EXTENSIONS ____
    u32 glfwExtCount = 0;
    const char **glfwExts = glfwGetRequiredInstanceExtensions(&glfwExtCount);

    std::vector<const char *> extensions(glfwExts, glfwExts + glfwExtCount);

    // for MacOS
    extensions.push_back(vk::KHRPortabilityEnumerationExtensionName);
    extensions.push_back(vk::KHRSurfaceExtensionName);

#ifdef DDEBUG // for DDEBUGging
    extensions.push_back(vk::EXTDebugUtilsExtensionName);
#endif

    // TODO: push back any wanted vulkan extensions.
    if(!CheckExtensionSupport(extensions))
    {
        LERROR("VULKAN ERROR: 1 or more extension was not supported.\n");
        exit(1);
    }

    //___ VALIDATION LAYERS ___
    const std::vector<const char *> layers = {
        "VK_LAYER_KHRONOS_validation"
    };

    if(!CheckValidationLayerSupport(layers))
    {
        LERROR("VULKAN ERROR: 1 or more validation layer was not supported.\n");
        exit(1);
    }

    std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

    //_____ VK INSTANCE ______
    vk::InstanceCreateInfo instanceInfo(
        vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR,
        &appInfo,
        (u32) 0, nullptr, // layers
        (u32) extensions.size(), extensions.data() // extensions
    );

#ifdef DDEBUG // NOTE: enable validation layers.
    instanceInfo.enabledLayerCount = (u32) layers.size();
    instanceInfo.ppEnabledLayerNames = layers.data();
#endif

    vk::Instance instance;
    try
    {
        instance = vk::createInstance(instanceInfo, nullptr);
        return instance;
    }
    catch(vk::SystemError err)
    {
        LERROR("VULKAN ERROR: couldn't create a vulkan instance.\n\t" << err.what() << "\n");
        return nullptr;
    }
}

// _____ DEBUG MESSENGER _____
static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    vk::DebugUtilsMessageTypeFlagsEXT messageType,
                                                    const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                    void *pUserData)
{
    if(messageSeverity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
        LERROR("Vulkan " << pCallbackData->pMessage << "\n");

    return vk::False;
}

vk::DebugUtilsMessengerEXT MakeDebugMessenger(const vk::Instance &vkInstance, vk::DispatchLoaderDynamic dispatch)
{
    vk::DebugUtilsMessengerCreateInfoEXT createInfo(
        vk::DebugUtilsMessengerCreateFlagsEXT(),
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,

        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        
        (PFN_vkDebugUtilsMessengerCallbackEXT) DebugCallback,
        (void *) nullptr,
        (const void *) nullptr
    );

    vk::DebugUtilsMessengerEXT debugMessenger;

    debugMessenger = vkInstance.createDebugUtilsMessengerEXT(createInfo, nullptr, dispatch);
    return debugMessenger;
}

// clang-format on

} // namespace DE::VK