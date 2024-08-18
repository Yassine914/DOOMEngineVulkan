#include "engine.h"

Engine::Engine() : window{new Window()}
{
    window->SetWindowSize(1920 / 2, 1080 / 2);
    window->SetFullscreen(false);
    window->SetResizeable(true);
    window->SetVSync(true);

    window->InitializeWindow();

    MakeVKInstance(window->GetTitle());
}

bool Engine::VKSupports(std::vector<const char *> &extensions, std::vector<const char *> &layers)
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
    // clang-format on

    try
    {
        vkInstance = vk::createInstance(createInfo, nullptr);
    }
    catch(vk::SystemError err)
    {
        LERROR("failed to create a vulkan instance: " << err.what() << "\n");
    }
}

Engine::~Engine()
{
    vkInstance.destroy();
    delete window;
}