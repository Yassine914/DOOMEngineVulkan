#include "device.h"

namespace DE::VK {

QueueFamilyIndices FindQueueFamilies(vk::PhysicalDevice device)
{
    QueueFamilyIndices indices;

    std::vector<vk::QueueFamilyProperties> qFamilies = device.getQueueFamilyProperties();

    i32 i = 0;
    for(const auto &qFamily : qFamilies)
    {
        if(qFamily.queueFlags & vk::QueueFlagBits::eGraphics)
            indices.graphicsFamily = i;

        if(indices.IsComplete())
            break;

        i++;
    }

    return indices;
}

//_____ PHYSICAL DEVICE _____

// clang-format off
bool IsPhysicalDeviceSuitable(vk::PhysicalDevice device)
{
    const std::vector<const char *> reqExtensions = {
        vk::KHRSwapchainExtensionName
    };

    std::vector<vk::ExtensionProperties> properties = device.enumerateDeviceExtensionProperties();

    std::set<std::string> req(reqExtensions.begin(), reqExtensions.end());
    for(auto prop : properties)
    {
        req.erase(prop.extensionName);
    }

    QueueFamilyIndices indices = FindQueueFamilies(device);

    return req.empty() && indices.IsComplete();
}
// clang-format on

i32 RatePhysicalDevice(vk::PhysicalDevice device)
{
    i32 score = 0;

    //_____ PROPERTIES _____
    vk::PhysicalDeviceProperties properties = device.getProperties();

    if(properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
        score += 1000;

    // maximum possible size of textures.
    score += properties.limits.maxImageDimension2D;

    //_____ FEATURES ______
    vk::PhysicalDeviceFeatures features = device.getFeatures();

    // return 0 if Geometry Shader not supported
    score = (features.geometryShader) ? score : 0;

    return score;
}

vk::PhysicalDevice ChoosePhysicalDevice(const vk::Instance &instance)
{
    std::vector<vk::PhysicalDevice> devices = instance.enumeratePhysicalDevices();
    LASSERT(devices.size() > 0, "VULKAN ERROR: couldn't find any physical device");

    std::vector<std::pair<i32, vk::PhysicalDevice>> suitableDevices;

    i32 maxScore = INT32_MIN;
    for(const auto &device : devices)
    {
        if(IsPhysicalDeviceSuitable(device))
        {
            i32 score = RatePhysicalDevice(device);
            suitableDevices.push_back(std::make_pair(score, device));

            maxScore = std::max(score, maxScore);
        }
    }

    vk::PhysicalDevice phyDevice;

    for(const auto &[score, device] : suitableDevices)
    {
        if(score == maxScore)
        {
            phyDevice = device;
            break;
        }
    }

#ifdef DDEBUG
    LINFO(true, "chosen physical device: " << phyDevice.getProperties().deviceName << "\n");
#endif

    return phyDevice;
}

//______ LOGICAL DEVICE ______

// clang-format off
DeviceProperties MakeLogicalDevice(vk::PhysicalDevice phyDevice)
{
    //______ QUEUE FAMILIES ______
    QueueFamilyIndices indices = FindQueueFamilies(phyDevice);
    f32 qPriority = 1.0f;

    vk::DeviceQueueCreateInfo qCreateInfo(
        vk::DeviceQueueCreateFlags(),
        indices.graphicsFamily.value(),
        1,
        &qPriority
    );

    //______ DEVICE EXTS ________
    std::vector<const char *> extensions = {
        vk::KHRSwapchainExtensionName,
    };

    //_____ VALIDATION LAYERS ____
    std::vector<const char *> layers;
#ifdef DDEBUG
    layers.push_back("VK_LAYER_KHRONOS_validation");
#endif

    vk::PhysicalDeviceFeatures features;

    vk::DeviceCreateInfo deviceInfo(
        vk::DeviceCreateFlags(),
        (u32) 1, &qCreateInfo, // queues
        (u32) layers.size(), layers.data(), // layers
        (u32) extensions.size(), extensions.data(), // extensions
        &features // features
    );

    DeviceProperties prop;
    try
    {
        prop.device = phyDevice.createDevice(deviceInfo);
    }
    catch(vk::SystemError err)
    {
        LERROR("VULKAN ERROR: couldn't create a logical device.\n\t" << err.what() << "\n");
        return {};
    }

    prop.indices = indices;

    return prop;
}

// indices.graphicsFamily.value() or indices.presentFamily.value() for the qFamilyIndex
vk::Queue GetDeviceQueue(const vk::Device &device, const i32 qFamilyIndex, const i32 qIndex)
{
    vk::Queue queue;
    try
    {
        queue = device.getQueue(qFamilyIndex, qIndex);
    }
    catch(vk::SystemError err)
    {
        LERROR("VULKAN ERROR: couldn't retrieve a queue from the device.\n\t" << err.what() << "\n");
        queue = nullptr;
    }

    return queue;
}

// clang-format on

} // namespace DE::VK