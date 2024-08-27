#include "engine.h"

#include "shader.h"
#include "render.h"

// constructor
Engine::Engine(i32 width, i32 height, Window *window) : width{width}, height{height}, window{window}
{
    //_____ VULKAN INIT _____
    MakeVKInstance(window->GetTitle());
    dldi = vk::DispatchLoaderDynamic(vkInstance, vkGetInstanceProcAddr);

    if(ENGINE_DEBUG)
        MakeVKDebugMessenger();

    ChooseVKPhysicalDevice();
    MakeVKLogicalDevice(physicalDevice);
    MakeVKQueues(device, physicalDevice);

    MakeVKSwapChain(device, physicalDevice, surface);

    MakeVKGraphicsPipeline();

    InitializeVKDrawing();
}

// clang-format off

#pragma region VKInstance

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

    //_____ SURFACE _____
    VkSurfaceKHR cSurface;
    if(glfwCreateWindowSurface(vkInstance, window->GetWindow(), nullptr, &cSurface) != VK_SUCCESS)
    {
        LERROR("failed to create a window surface for GLFW and Vulkan.\n");
        exit(303);
    }

    surface = vk::SurfaceKHR(cSurface);
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

#pragma endregion

#pragma region PhysicalDevice

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

#pragma endregion

#pragma region LogicalDevice 
struct QueueFamilyIndices
{
    std::optional<u32> graphicsFamily;
    std::optional<u32> presentFamily;

    inline bool IsComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

QueueFamilyIndices FindQueueFamilies(vk::PhysicalDevice phyDevice, vk::SurfaceKHR surface)
{
    QueueFamilyIndices indices;
    std::vector<vk::QueueFamilyProperties> queueFamilies = phyDevice.getQueueFamilyProperties();

    if(ENGINE_DEBUG)
        LDEBUG(true, "physical device: \"" << phyDevice.getProperties().deviceName << "\" can support "
                                           << queueFamilies.size() << " queue families\n");

    i32 i = 0;
    for(vk::QueueFamilyProperties qFamily : queueFamilies)
    {
        if(qFamily.queueFlags & vk::QueueFlagBits::eGraphics)
            indices.graphicsFamily = i;

        if(phyDevice.getSurfaceSupportKHR(i, surface))
            indices.presentFamily = i;

        if(indices.IsComplete())
            break;

        i++;
    }

    return indices;
}

void Engine::MakeVKLogicalDevice(vk::PhysicalDevice phyDevice)
{
    QueueFamilyIndices indices = FindQueueFamilies(phyDevice, surface);
    
    std::vector<u32> uniqueIndices;
    uniqueIndices.push_back(indices.graphicsFamily.value());
    if(indices.graphicsFamily.value() != indices.presentFamily.value())
        uniqueIndices.push_back(indices.presentFamily.value());
    
    f32 qPriority = 1.0f; // qPriorite E [0, 1]

    std::vector<vk::DeviceQueueCreateInfo> qCreateInfo;
    for(u32 qFamilyIndex : uniqueIndices)
    {
        qCreateInfo.push_back(vk::DeviceQueueCreateInfo(
            vk::DeviceQueueCreateFlags(),
            qFamilyIndex,
            1, // only 1 queue
            &qPriority
        ));
    }

    // device extensions
    std::vector<const char *> deviceEXT = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };


    vk::PhysicalDeviceFeatures deviceFeat = vk::PhysicalDeviceFeatures();

    std::vector<const char *> enabledLayers;
    if(ENGINE_DEBUG)
        enabledLayers.push_back("VK_LAYER_KHRONOS_validation");

    vk::DeviceCreateInfo deviceInfo = vk::DeviceCreateInfo(
        vk::DeviceCreateFlags(),
        (u32) qCreateInfo.size(), qCreateInfo.data(), // queue data
        (u32) enabledLayers.size(), enabledLayers.data(), // validation layers
        (u32) deviceEXT.size(), deviceEXT.data(), // device extensions
        &deviceFeat // device features
    );

    try
    {
        device = phyDevice.createDevice(deviceInfo);

        if(ENGINE_DEBUG)
            LINFO(true, "GPU has been successfully abstracted\n");
    }
    catch(vk::SystemError err)
    {
        LERROR("VULKAN ERROR: failed to create logical device.\n\t" << err.what() << "\n");
        exit(202);
    }
}

void Engine::MakeVKQueues(vk::Device logDevice, vk::PhysicalDevice phyDevice)
{
    QueueFamilyIndices indices = FindQueueFamilies(phyDevice, surface);
    graphicsQueue = logDevice.getQueue(indices.graphicsFamily.value(), 0);
    presentQueue = logDevice.getQueue(indices.presentFamily.value(), 0);
}

struct SwapChainSupportDetails
{
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

SwapChainSupportDetails QuerySwapChainSupport(vk::PhysicalDevice phyDevice, vk::SurfaceKHR surface)
{
    SwapChainSupportDetails support;

    support.capabilities = phyDevice.getSurfaceCapabilitiesKHR(surface);
    support.formats = phyDevice.getSurfaceFormatsKHR(surface);
    support.presentModes = phyDevice.getSurfacePresentModesKHR(surface);
    
    return support;   
}

vk::SurfaceFormatKHR ChooseSwapChainSurfaceFormat(std::vector<vk::SurfaceFormatKHR> formats)
{
    for(vk::SurfaceFormatKHR fmt: formats)
    {
        if(fmt.format == vk::Format::eB8G8R8A8Unorm
            && fmt.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            return fmt;
    }

    return formats[0];
}

vk::PresentModeKHR ChooseSwapChainPresentMode(std::vector<vk::PresentModeKHR> modes)
{
    for(vk::PresentModeKHR mode : modes)
    {
        if(mode == vk::PresentModeKHR::eMailbox)
            return mode;
    }

    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D ChooseSwapChainExtent(vk::SurfaceCapabilitiesKHR capabilities, u32 width, u32 height)
{
    if(capabilities.currentExtent.width != UINT32_MAX)
        return capabilities.currentExtent;
        
    vk::Extent2D ext = {width, height};

    ext.width = std::min(
        capabilities.maxImageExtent.width,
        std::max(capabilities.minImageExtent.width, width)
    );

    ext.height = std::min(
        capabilities.maxImageExtent.height,
        std::max(capabilities.minImageExtent.height, height)
    );

    return ext;
}

void Engine::MakeVKSwapChain(vk::Device logicalDevice, vk::PhysicalDevice physicalDevice, 
                             vk::SurfaceKHR surface)
{
    SwapChainSupportDetails support = QuerySwapChainSupport(physicalDevice, surface);

    vk::SurfaceFormatKHR fmt = ChooseSwapChainSurfaceFormat(support.formats);
    vk::PresentModeKHR mode = ChooseSwapChainPresentMode(support.presentModes);
    vk::Extent2D ext = ChooseSwapChainExtent(support.capabilities, width, height);

    u32 imageCount = std::min(
        support.capabilities.maxImageCount,
        support.capabilities.minImageCount + 1
    );

    vk::SwapchainCreateInfoKHR swapchainInfo = vk::SwapchainCreateInfoKHR(
        vk::SwapchainCreateFlagsKHR(),
        surface, imageCount,
        fmt.format, fmt.colorSpace,
        ext, 1, vk::ImageUsageFlagBits::eColorAttachment
    );

    QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, surface);
    u32 qFamilyIndices[2] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if(indices.graphicsFamily.value() != indices.presentFamily.value())
    {
        swapchainInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        swapchainInfo.queueFamilyIndexCount = 2;
        swapchainInfo.pQueueFamilyIndices = qFamilyIndices;
    }
    else
    {
        swapchainInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }

    swapchainInfo.preTransform = support.capabilities.currentTransform;
    swapchainInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    swapchainInfo.presentMode = mode;
    swapchainInfo.clipped = VK_TRUE;

    swapchainInfo.oldSwapchain = vk::SwapchainKHR(nullptr);

    SwapChainBundle bundle{};

    try
    {
        bundle.swapchain = logicalDevice.createSwapchainKHR(swapchainInfo);
    }
    catch (vk::SystemError err)
    {
        LERROR("VULKAN ERROR: couldn't createe a swapchain.\n\t" << err.what() << "\n");
        exit(505);
    }

    std::vector<vk::Image> images = logicalDevice.getSwapchainImagesKHR(bundle.swapchain);
    bundle.frames.resize(images.size());
    for(u64 i = 0; i < images.size(); i++)
    {
        // creating an image view per image.
        vk::ImageViewCreateInfo imViewInfo{};
        imViewInfo.image = images[i];
        imViewInfo.viewType = vk::ImageViewType::e2D;
        imViewInfo.format = fmt.format;
        
        imViewInfo.components.r = vk::ComponentSwizzle::eIdentity;
        imViewInfo.components.g = vk::ComponentSwizzle::eIdentity;
        imViewInfo.components.b = vk::ComponentSwizzle::eIdentity;
        imViewInfo.components.a = vk::ComponentSwizzle::eIdentity;
        
        imViewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        imViewInfo.subresourceRange.baseMipLevel = 0;
        imViewInfo.subresourceRange.levelCount = 1;
        imViewInfo.subresourceRange.baseArrayLayer = 0;
        imViewInfo.subresourceRange.layerCount = 1;
        
        bundle.frames[i].image = images[i];
        bundle.frames[i].imageView = logicalDevice.createImageView(imViewInfo);
    }

    bundle.format = fmt.format;
    bundle.extent = ext;

    swapchain = bundle;

    maxFramesInFlight = static_cast<i32>(swapchain.frames.size());
    frameNum = 0;
}

#pragma endregion

#pragma region Pipeline

struct GraphicsPipelineInBundle
{
    vk::Device device;
    vk::Extent2D swapchainExtent;
    vk::Format swapchainImageFormat;

    std::string vertexFilepath;
    std::string fragmentFilepath;
};

vk::PipelineLayout CreateGraphicsPipelineLayout(vk::Device device)
{
    vk::PipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.flags = vk::PipelineLayoutCreateFlags();
    layoutInfo.setLayoutCount = 0;
    layoutInfo.pushConstantRangeCount = 0;

    try
    {
        return device.createPipelineLayout(layoutInfo);
    }
    catch (vk::SystemError err)
    {
        LERROR("VULKAN ERROR: couldn't create pipeline layout.\n\t" << err.what() << "\n");
        return nullptr;
    }
}

vk::RenderPass CreateGraphicsPipelineRenderPass(vk::Device device, vk::Format fmt)
{
    vk::AttachmentDescription colorAtt{};
    colorAtt.flags = vk::AttachmentDescriptionFlags();
    colorAtt.format = fmt;
    colorAtt.samples = vk::SampleCountFlagBits::e1;
    colorAtt.loadOp = vk::AttachmentLoadOp::eClear;
    colorAtt.storeOp  = vk::AttachmentStoreOp::eStore;
    colorAtt.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colorAtt.stencilStoreOp  = vk::AttachmentStoreOp::eDontCare;
    colorAtt.initialLayout = vk::ImageLayout::eUndefined;
    colorAtt.finalLayout = vk::ImageLayout::ePresentSrcKHR;

    vk::AttachmentReference colorAttRef{};
    colorAttRef.attachment = 0;
    colorAttRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::SubpassDescription subpass{};
    subpass.flags = vk::SubpassDescriptionFlags();
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttRef;

    vk::RenderPassCreateInfo renderPassInfo{};
    renderPassInfo.flags = vk::RenderPassCreateFlags();
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAtt;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    try
    {
        return device.createRenderPass(renderPassInfo);
    }
    catch(vk::SystemError err)
    {
        LERROR("VULKAN ERROR: couldn't create render pass.\n\t" << err.what() << "\n");
    }

    return nullptr;
}

GraphicsPipelineBundle CreateGraphicsPipeline(GraphicsPipelineInBundle spec)
{
    vk::GraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.flags = vk::PipelineCreateFlags();

    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

    // vertex input
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.flags =  vk::PipelineVertexInputStateCreateFlags();
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    pipelineInfo.pVertexInputState = &vertexInputInfo;

    // input assembly
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
    inputAssemblyInfo.flags = vk::PipelineInputAssemblyStateCreateFlags();
    inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
    pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;

    // vertex shader
    vk::ShaderModule vertShader = DEUtil::CreateShaderModule(spec.vertexFilepath.c_str(), spec.device);
    vk::PipelineShaderStageCreateInfo vertShaderInfo{};
    vertShaderInfo.flags = vk::PipelineShaderStageCreateFlags();
    vertShaderInfo.stage = vk::ShaderStageFlagBits::eVertex;
    vertShaderInfo.module = vertShader;
    vertShaderInfo.pName = "main"; // NOTE: hardcoded name
    shaderStages.push_back(vertShaderInfo);
    
    // viewport and scissor
    vk::Viewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = spec.swapchainExtent.width;
    viewport.height = spec.swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vk::Rect2D scissor{};
    scissor.offset.x = 0.0f;
    scissor.offset.y = 0.0f;
    scissor.extent = spec.swapchainExtent;

    vk::PipelineViewportStateCreateInfo viewportStateInfo{};
    viewportStateInfo.flags = vk::PipelineViewportStateCreateFlags();
    viewportStateInfo.viewportCount = 1;
    viewportStateInfo.pViewports = &viewport;
    viewportStateInfo.scissorCount = 1;
    viewportStateInfo.pScissors = &scissor;
    pipelineInfo.pViewportState = &viewportStateInfo;

    // rasterizer
    vk::PipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.flags = vk::PipelineRasterizationStateCreateFlags();
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = vk::PolygonMode::eFill;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = vk::CullModeFlagBits::eBack;
    rasterizer.frontFace = vk::FrontFace::eClockwise;
    rasterizer.depthBiasEnable = VK_FALSE;
    pipelineInfo.pRasterizationState = &rasterizer;

    // fragment shader
    vk::ShaderModule fragShader = DEUtil::CreateShaderModule(spec.fragmentFilepath.c_str(), spec.device);
    vk::PipelineShaderStageCreateInfo fragShaderInfo{};
    fragShaderInfo.flags = vk::PipelineShaderStageCreateFlags();
    fragShaderInfo.stage = vk::ShaderStageFlagBits::eFragment;
    fragShaderInfo.module = fragShader;
    fragShaderInfo.pName = "main"; // NOTE: hardcoded name
    shaderStages.push_back(fragShaderInfo);

    pipelineInfo.stageCount = shaderStages.size();
    pipelineInfo.pStages = shaderStages.data();

    // multisampling
    vk::PipelineMultisampleStateCreateInfo multisampling{};
    multisampling.flags = vk::PipelineMultisampleStateCreateFlags();
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
    pipelineInfo.pMultisampleState = &multisampling;

    // color blend
    vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR |
        vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |vk::ColorComponentFlagBits::eA;
    colorBlendAttachment.blendEnable = VK_FALSE;
    
    vk::PipelineColorBlendStateCreateInfo colorBlend{};
    colorBlend.flags = vk::PipelineColorBlendStateCreateFlags();
    colorBlend.logicOpEnable = VK_FALSE;
    colorBlend.logicOp = vk::LogicOp::eCopy;
    colorBlend.attachmentCount = 1;
    colorBlend.pAttachments =  &colorBlendAttachment;
    colorBlend.blendConstants[0] = 0.0f;
    colorBlend.blendConstants[1] = 0.0f;
    colorBlend.blendConstants[2] = 0.0f;
    colorBlend.blendConstants[3] = 0.0f;
    pipelineInfo.pColorBlendState = &colorBlend;

    // pipeline layout
    vk::PipelineLayout layout = CreateGraphicsPipelineLayout(spec.device);
    pipelineInfo.layout = layout;

    // render pass
    vk::RenderPass renderPass = CreateGraphicsPipelineRenderPass(spec.device, spec.swapchainImageFormat);
    pipelineInfo.renderPass = renderPass;

    // misc
    pipelineInfo.basePipelineHandle = nullptr;

    // pipeline
    vk::Pipeline pipeline;
    try
    {
        pipeline = spec.device.createGraphicsPipeline(nullptr, pipelineInfo).value;
    }
    catch(vk::SystemError err)
    {
        LERROR("VULKAN ERROR: failed to create graphics pipeline.\n\t" << err.what() << "\n");
    }
    

    GraphicsPipelineBundle out;
    out.layout = layout;
    out.renderPass = renderPass;
    out.pipeline = pipeline;
    
    spec.device.destroyShaderModule(vertShader);
    spec.device.destroyShaderModule(fragShader);
    return out;
}

void Engine::MakeVKGraphicsPipeline()
{
    GraphicsPipelineInBundle spec = {
        .device           = device,
        .swapchainExtent  = swapchain.extent,
        .swapchainImageFormat = swapchain.format,

        .vertexFilepath   = RES_PATH"shaders/basic.vert.spv", // NOTE: hardcoded filepath
        .fragmentFilepath = RES_PATH"shaders/basic.frag.spv", // NOTE: hardcoded filepath
    };

    pipeline = CreateGraphicsPipeline(spec);
}

#pragma endregion

#pragma region InitFinalization 

struct FrameBufferIn
{
    vk::Device device;
    vk::RenderPass renderPass;
    vk::Extent2D swapchainExtent;
};

void CreateFrameBuffers(FrameBufferIn bufferIn, std::vector<SwapChainFrame> &frames)
{
    for(i32 i = 0; i < frames.size(); i++)
    {
        std::vector<vk::ImageView> att = {
            frames[i].imageView
        };

        vk::FramebufferCreateInfo bufferInfo{};
        bufferInfo.flags = vk::FramebufferCreateFlags();
        bufferInfo.attachmentCount = att.size();
        bufferInfo.pAttachments = att.data();
        bufferInfo.renderPass = bufferIn.renderPass;
        bufferInfo.width = bufferIn.swapchainExtent.width;
        bufferInfo.height = bufferIn.swapchainExtent.height;
        bufferInfo.layers = 1;

        try
        {
            frames[i].frameBuffer = bufferIn.device.createFramebuffer(bufferInfo);
        }
        catch(vk::SystemError err)
        {
            LERROR("VULKAN ERROR: couldn't create a frame buffer.\n\t" << err.what() << "\n");
        }
    }
}

struct CommandBufferIn
{
    vk::Device device;
    vk::CommandPool commandPool;
    std::vector<SwapChainFrame> &frames;
};

vk::CommandPool CreateCommandPool(vk::Device device, vk::PhysicalDevice phyDevice, vk::SurfaceKHR surface)
{
    QueueFamilyIndices qFamilyIndices = FindQueueFamilies(phyDevice, surface);

    vk::CommandPoolCreateInfo poolInfo{};
    poolInfo.flags = vk::CommandPoolCreateFlags() | vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    poolInfo.queueFamilyIndex = qFamilyIndices.graphicsFamily.value();

    try
    {
        return device.createCommandPool(poolInfo);
    }
    catch(vk::SystemError err)
    {
        LERROR("VULKAN ERROR: couldn't create a command pool.\n\t" << err.what() << "\n");
        return nullptr;
    }
}

vk::CommandBuffer CreateCommandBuffers(CommandBufferIn cmdIn)
{
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.commandPool = cmdIn.commandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = 1;

    for(i32 i = 0; i < cmdIn.frames.size(); i++)
    {
        try
        {
            cmdIn.frames[i].commandBuffer = cmdIn.device.allocateCommandBuffers(allocInfo)[0];
        }
        catch(vk::SystemError err)
        {
            LERROR("VULKAN ERROR: couldn't allocate command buffers.\n\t" << err.what() << "\n");
        }
    }

    try
    {
        vk::CommandBuffer mainBuff = cmdIn.device.allocateCommandBuffers(allocInfo)[0];
        return mainBuff;
    }
    catch(vk::SystemError err)
    {
        LERROR("VULKAN ERROR: couldn't allocate main command buffer.\n\t" << err.what() << "\n");
        return nullptr;
    }
}

vk::Semaphore CreateSemaphore(vk::Device device)
{
    vk::SemaphoreCreateInfo spInfo{};
    spInfo.flags = vk::SemaphoreCreateFlags();

    try
    {
        return device.createSemaphore(spInfo);
    }
    catch (vk::SystemError err)
    {
        LERROR("VULKAN ERROR: couldn't create semaphore.\n\t" << err.what() << "\n");
        return nullptr;
    }
}

vk::Fence CreateFence(vk::Device device)
{
    vk::FenceCreateInfo fenceInfo{};
    fenceInfo.flags = vk::FenceCreateFlags() | vk::FenceCreateFlagBits::eSignaled;

    try
    {
        return device.createFence(fenceInfo);
    }
    catch(vk::SystemError err)
    {
        LERROR("VULKAN ERROR: couldn't create a fence.\n\t" << err.what() << "\n");
        return nullptr;
    }
}

void Engine::InitializeVKDrawing()
{
    FrameBufferIn frameBufferIn = {
        .device = device,
        .renderPass = pipeline.renderPass,
        .swapchainExtent = swapchain.extent,
    };
    CreateFrameBuffers(frameBufferIn, swapchain.frames);
    
    commandPool = CreateCommandPool(device, physicalDevice, surface);

    CommandBufferIn cmdIn = {
        .device = device,
        .commandPool = commandPool,
        .frames = swapchain.frames,
    };
    mainCommandBuffer = CreateCommandBuffers(cmdIn);

    for(SwapChainFrame &frame : swapchain.frames)
    {
        frame.inFlight = CreateFence(device);
        frame.imageAvailable = CreateSemaphore(device);
        frame.renderFinished = CreateSemaphore(device);
    }
}

#pragma endregion

#pragma region Drawing

void Engine::RecordVKDrawCommands(vk::CommandBuffer commandBuffer, u32 imageIdx)
{
    vk::CommandBufferBeginInfo beginInfo{};
    try
    {
        commandBuffer.begin(beginInfo);
    }
    catch (vk::SystemError err)
    {
        LERROR("VULKAN ERROR: couldn't begin a command buffer.\n\t" << err.what() << "\n");
    }

    vk::RenderPassBeginInfo passInfo{};
    passInfo.renderPass = pipeline.renderPass;
    passInfo.framebuffer = swapchain.frames[imageIdx].frameBuffer;
    passInfo.renderArea.offset.x = 0;
    passInfo.renderArea.offset.x = 0;
    passInfo.renderArea.extent = swapchain.extent;

    vk::ClearValue clearColor = {std::array<f32, 4>{1.0f, 0.5f, 0.25f, 1.0f}};
    passInfo.clearValueCount = 1;
    passInfo.pClearValues = &clearColor;

    commandBuffer.beginRenderPass(&passInfo, vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.pipeline);

    commandBuffer.draw(3, 1, 0, 0);

    commandBuffer.endRenderPass();

    try
    {
        commandBuffer.end();
    }
    catch (vk::SystemError err)
    {
        LERROR("VULKAN ERROR: couldn't end a command buffer.\n\t" << err.what() << "\n");
    }

}

void Engine::Render()
{
    device.waitForFences(1, &swapchain.frames[frameNum].inFlight, VK_TRUE, UINT64_MAX);
    device.resetFences(1, &swapchain.frames[frameNum].inFlight);

    u32 imageIdx = device.acquireNextImageKHR(swapchain.swapchain, UINT64_MAX, swapchain.frames[frameNum].imageAvailable, nullptr).value;

    vk::CommandBuffer cmdBuffer = swapchain.frames[frameNum].commandBuffer;
    cmdBuffer.reset();

    RecordVKDrawCommands(cmdBuffer, imageIdx);

    vk::SubmitInfo submitInfo{};
    vk::Semaphore waitSemaphores[] = {swapchain.frames[frameNum].imageAvailable};
    vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;
    vk::Semaphore signalSemaphores[] = {swapchain.frames[frameNum].renderFinished};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    try
    {
        graphicsQueue.submit(submitInfo, swapchain.frames[frameNum].inFlight);
    }
    catch (vk::SystemError err)
    {
        LERROR("VULKAN ERROR: couldn't submit draw command buffer to graphics queue.\n\t" << err.what() << "\n");
    }

    vk::PresentInfoKHR presentInfo{};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores; // NOTE: might be wrong
    vk::SwapchainKHR swapchains[] = {swapchain.swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIdx;

    presentQueue.presentKHR(presentInfo);

    frameNum = (frameNum + 1) % maxFramesInFlight;
}

#pragma endregion

// clang-format on

Engine::~Engine()
{
    device.waitIdle();

    device.destroyCommandPool(commandPool);

    device.destroyPipeline(pipeline.pipeline);
    device.destroyPipelineLayout(pipeline.layout);
    device.destroyRenderPass(pipeline.renderPass);

    for(SwapChainFrame frame : swapchain.frames)
    {
        device.destroyFence(frame.inFlight);

        device.destroySemaphore(frame.imageAvailable);
        device.destroySemaphore(frame.renderFinished);

        device.destroyImageView(frame.imageView);
        device.destroyFramebuffer(frame.frameBuffer);
    }

    device.destroySwapchainKHR(swapchain.swapchain);
    device.destroy();

    vkInstance.destroySurfaceKHR(surface);
    if(ENGINE_DEBUG)
        vkInstance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dldi);

    vkInstance.destroy();
    delete window;
}