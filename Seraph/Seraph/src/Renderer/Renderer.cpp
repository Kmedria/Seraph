#include "Renderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "Depedencies/stb_image.h"

Vec3 operator*(double scale, Vec3& rhs) {
    Vec3 temp = { scale * rhs.x, scale * rhs.y, scale * rhs.z };
    return temp;
}

Vec3 operator/(Vec3& lhs, double scale) {
    Vec3 temp = { lhs.x / scale, lhs.y / scale, lhs.z / scale };
    return temp;
}

Vec3& operator/=(Vec3& lhs, double scale) {
    lhs.x /= scale;
    lhs.y /= scale;
    lhs.z /= scale;
    return lhs;
}

Renderer::Context::Context() {
    windowName = "Seraph-Core";
    windowWidth = 960;
    windowHeight = 540;
}

Renderer::Context::Context(const char* name, int width, int height) {
    windowName = name;
    windowWidth = width;
    windowHeight = height;
}

void Renderer::initRenderer(Renderer::Context& renderer) {
    initWindow(renderer, renderer.windowName, renderer.windowWidth, renderer.windowHeight);
    initVulkan(renderer);
    renderer.initialised = true;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData);

static std::vector<char> readFile(const std::string& filename);

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

void cleanupSwapChain(Renderer::Context& renderer) {

    vkDestroyImageView(renderer.vk_device, renderer.vk_depthImageView, nullptr);
    vkDestroyImage(renderer.vk_device, renderer.vk_depthImage, nullptr);
    vkFreeMemory(renderer.vk_device, renderer.vk_depthImageMemory, nullptr);

    for (size_t i = 0; i < renderer.vk_swapChainFramebuffers.size(); i++) {
        vkDestroyFramebuffer(renderer.vk_device, renderer.vk_swapChainFramebuffers[i], nullptr);
    }

    for (size_t i = 0; i < renderer.vk_swapChainImageViews.size(); i++) {
        vkDestroyImageView(renderer.vk_device, renderer.vk_swapChainImageViews[i], nullptr);
    }

    vkDestroySwapchainKHR(renderer.vk_device, renderer.vk_swapChain, nullptr);

}

VkFormat findSupportedFormat(Renderer::Context& renderer, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(renderer.vk_physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throwError("failed to find supported format!", logLevelError);
}

VkFormat findDepthFormat(Renderer::Context& renderer) {
    return findSupportedFormat(renderer,
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

bool hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void recreateSwapChain(Renderer::Context& renderer) {

    int width = 0, height = 0;
    glfwGetFramebufferSize(renderer.window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(renderer.window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(renderer.vk_device);

    renderer.windowHeight = height;
    renderer.windowWidth = width;

    cleanupSwapChain(renderer);

    createSwapChain(renderer);
    createImageViews(renderer);
    createDepthResources(renderer);
    createFramebuffers(renderer);
}

static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<Renderer::Context*>(glfwGetWindowUserPointer(window));
    app->vk_framebufferResized = true;
}

void initWindow(Renderer::Context& renderer, const char* windowName, int width, int height) {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    renderer.window = glfwCreateWindow(width, height, windowName, nullptr, nullptr);
    glfwSetWindowUserPointer(renderer.window, &renderer);
    glfwSetFramebufferSizeCallback(renderer.window, framebufferResizeCallback);
}

void initVulkan(Renderer::Context& renderer) {
    createInstance(renderer);
    setupDebugMessenger(renderer);
    createSurface(renderer);
    pickPhysicalDevice(renderer);
    createLogicalDevice(renderer);
    createSwapChain(renderer);
    createImageViews(renderer);
    createRenderPass(renderer);
    createDescriptorSetLayout(renderer);
    createGraphicsPipeline(renderer);
    createCommandPool(renderer);
    createTextureImage(renderer);
    createTextureImageView(renderer);
    createTextureSampler(renderer);
    createDepthResources(renderer);
    createFramebuffers(renderer);
    createVertexBuffer(renderer);
    createIndexBuffer(renderer);
    createUniformBuffers(renderer);
    createDescriptorPool(renderer);
    createDescriptorSets(renderer);
    createCommandBuffer(renderer);
    createSyncObjects(renderer);
}

void createInstance(Renderer::Context& renderer) {

    if (enableValidationLayers && !checkValidationLayerSupport()) {
        logRecord("validation layers requested, but not available!", logLevelError);
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Seraph Game Engine";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    auto extensions = getRequiredExtensions();

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }

    if (vkCreateInstance(&createInfo, nullptr, &renderer.vk_instance) != VK_SUCCESS) {
        throwError("failed to create instance!", logLevelCritical);
    }
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    std::string temp = "validation layer: " + std::string(pCallbackData->pMessage);

    switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        logRecord(temp, logLevelInfo);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        logRecord(temp, logLevelError);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        logRecord(temp, logLevelAll);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        logRecord(temp, logLevelWarning);
        break;
    default:
        logRecord(temp);
        break;
    }

    return VK_FALSE;
}

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

void setupDebugMessenger(Renderer::Context& renderer) {
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(renderer.vk_instance, &createInfo, nullptr, &renderer.vk_debugMessenger) != VK_SUCCESS) {
        logRecord("failed to set up debug messenger!", logLevelError);
    }

}

bool isDeviceSuitable(Renderer::Context& renderer, VkPhysicalDevice device) {

    QueueFamilyIndices indices = findQueueFamilies(renderer, device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(renderer, device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(Renderer::Context& renderer, const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    else {
        int width, height;
        glfwGetFramebufferSize(renderer.window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
};

QueueFamilyIndices findQueueFamilies(Renderer::Context& renderer, VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    VkBool32 presentSupport = false;

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {

        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, renderer.vk_surface, &presentSupport);
        if (presentSupport) {
            indices.presentFamily = i;
        }
        if (indices.isComplete()) {
            break;
        }

        i++;
    }

    return indices;
}

void pickPhysicalDevice(Renderer::Context& renderer) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(renderer.vk_instance, &deviceCount, nullptr);

    if (0 == deviceCount) {
        throwError("Failed to find a valid physical device.", logLevelCritical);
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(renderer.vk_instance, &deviceCount, devices.data());

    for (const auto& device : devices) {

        if (isDeviceSuitable(renderer, device)) {
            renderer.vk_physicalDevice = device;
            break;
        }
    }

    if (VK_NULL_HANDLE == renderer.vk_physicalDevice) {
        throwError("No GPU that meet the physical device requirements were found.", logLevelCritical);
    }
}

void createLogicalDevice(Renderer::Context& renderer) {
    QueueFamilyIndices indices = findQueueFamilies(renderer, renderer.vk_physicalDevice);


    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    float queuePriority = 1.0f;

    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{}; // not neccessaary right now, but will be used in advanced devices.
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    
    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(renderer.vk_physicalDevice, &supportedFeatures);

    if (supportedFeatures.fillModeNonSolid) {
        deviceFeatures.fillModeNonSolid = VK_TRUE;
    }

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    createInfo.enabledLayerCount = 0;

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }

    if (vkCreateDevice(renderer.vk_physicalDevice, &createInfo, nullptr, &renderer.vk_device) != VK_SUCCESS) {
        throwError("failed to create logical device!", logLevelError);
    }

    vkGetDeviceQueue(renderer.vk_device, indices.presentFamily.value(), 0, &renderer.vk_presentQueue);
    vkGetDeviceQueue(renderer.vk_device, indices.graphicsFamily.value(), 0, &renderer.vk_graphicsQueue);
};

void createSwapChain(Renderer::Context& renderer) {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(renderer, renderer.vk_physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(renderer, swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = renderer.vk_surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(renderer, renderer.vk_physicalDevice);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(renderer.vk_device, &createInfo, nullptr, &renderer.vk_swapChain) != VK_SUCCESS) {
        throwError("failed to create swap chain!", logLevelError);
    }

    vkGetSwapchainImagesKHR(renderer.vk_device, renderer.vk_swapChain, &imageCount, nullptr);
    renderer.vk_swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(renderer.vk_device, renderer.vk_swapChain, &imageCount, renderer.vk_swapChainImages.data());

    renderer.vk_swapChainImageFormat = surfaceFormat.format;
    renderer.vk_swapChainExtent = extent;

}

void createImageViews(Renderer::Context& renderer) {

    renderer.vk_swapChainImageViews.resize(renderer.vk_swapChainImages.size());

    for (uint32_t i = 0; i < renderer.vk_swapChainImages.size(); i++) {
        renderer.vk_swapChainImageViews[i] = createImageView(renderer, renderer.vk_swapChainImages[i], renderer.vk_swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }

};

VkShaderModule createShaderModule(Renderer::Context& renderer, const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(renderer.vk_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throwError("failed to create shader module!", logLevelError);
    }
    return shaderModule;
}

void createRenderPass(Renderer::Context& renderer) {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = renderer.vk_swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = findDepthFormat(renderer);
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;


    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(renderer.vk_device, &renderPassInfo, nullptr, &renderer.vk_renderPass) != VK_SUCCESS) {
        throwError("failed to create render pass!", logLevelError);
    }
}

uint32_t findMemoryType(Renderer::Context& renderer, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(renderer.vk_physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throwError("failed to find suitable memory type!", logLevelError);
}

void createImage(Renderer::Context& renderer, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(renderer.vk_device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(renderer.vk_device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(renderer, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(renderer.vk_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(renderer.vk_device, image, imageMemory, 0);
}

void createDepthResources(Renderer::Context& renderer) {
    VkFormat depthFormat = findDepthFormat(renderer);
    createImage(renderer, renderer.vk_swapChainExtent.width, renderer.vk_swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, renderer.vk_depthImage, renderer.vk_depthImageMemory);
    renderer.vk_depthImageView = createImageView(renderer, renderer.vk_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

}

VkImageView createImageView(Renderer::Context& renderer, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(renderer.vk_device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}

void createTextureImageView(Renderer::Context& renderer) {
    renderer.vk_textureImageView = createImageView(renderer, renderer.vk_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

void createTextureImage(Renderer::Context& renderer) {
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load("src/Renderer/Textures/childeFace.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        throwError("failed to load texture image!", logLevelError);
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(renderer, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(renderer.vk_device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(renderer.vk_device, stagingBufferMemory);

    stbi_image_free(pixels);

    createImage(renderer, texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, renderer.vk_textureImage, renderer.vk_textureImageMemory);

    transitionImageLayout(renderer, renderer.vk_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(renderer, stagingBuffer, renderer.vk_textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

    transitionImageLayout(renderer, renderer.vk_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkDestroyBuffer(renderer.vk_device, stagingBuffer, nullptr);
    vkFreeMemory(renderer.vk_device, stagingBufferMemory, nullptr);

}

void createTextureSampler(Renderer::Context& renderer) {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(renderer.vk_physicalDevice, &properties);
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy; //////////////// sets max anisotropy. higher leads to less performance but better qualilty
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(renderer.vk_device, &samplerInfo, nullptr, &renderer.vk_textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

void createBuffer(Renderer::Context& renderer, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(renderer.vk_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(renderer.vk_device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(renderer, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(renderer.vk_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(renderer.vk_device, buffer, bufferMemory, 0);
}

VkCommandBuffer beginSingleTimeCommands(Renderer::Context& renderer) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = renderer.vk_commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(renderer.vk_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void endSingleTimeCommands(Renderer::Context& renderer, VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(renderer.vk_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(renderer.vk_graphicsQueue);

    vkFreeCommandBuffers(renderer.vk_device, renderer.vk_commandPool, 1, &commandBuffer);
}

void copyBufferToImage(Renderer::Context& renderer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(renderer);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    endSingleTimeCommands(renderer, commandBuffer);
}

void transitionImageLayout(Renderer::Context& renderer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(renderer);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0; // TODO
    barrier.dstAccessMask = 0; // TODO

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    endSingleTimeCommands(renderer, commandBuffer);
}

void copyBuffer(Renderer::Context& renderer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(renderer);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(renderer, commandBuffer);

}

void updateVertexBuffer(Renderer::Context& renderer) {
    VkDeviceSize bufferSize = sizeof(renderer.vertices[0]) * renderer.vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(renderer, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(renderer.vk_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, renderer.vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(renderer.vk_device, stagingBufferMemory);
    
    vkQueueWaitIdle(renderer.vk_graphicsQueue);
    vkDestroyBuffer(renderer.vk_device, renderer.vk_vertexBuffer, nullptr);
    vkFreeMemory(renderer.vk_device, renderer.vk_vertexBufferMemory, nullptr);
    createBuffer(renderer, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, renderer.vk_vertexBuffer, renderer.vk_vertexBufferMemory);

    copyBuffer(renderer, stagingBuffer, renderer.vk_vertexBuffer, bufferSize);

    vkDestroyBuffer(renderer.vk_device, stagingBuffer, nullptr);
    vkFreeMemory(renderer.vk_device, stagingBufferMemory, nullptr);
}

void updateIndexBuffer(Renderer::Context& renderer) {
    VkDeviceSize bufferSize = sizeof(renderer.indices[0]) * renderer.indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(renderer, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(renderer.vk_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, renderer.indices.data(), (size_t)bufferSize);
    vkUnmapMemory(renderer.vk_device, stagingBufferMemory);
    vkQueueWaitIdle(renderer.vk_graphicsQueue);
    vkDestroyBuffer(renderer.vk_device, renderer.vk_indexBuffer, nullptr);
    vkFreeMemory(renderer.vk_device, renderer.vk_indexBufferMemory, nullptr);
    createBuffer(renderer, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, renderer.vk_indexBuffer, renderer.vk_indexBufferMemory);

    copyBuffer(renderer, stagingBuffer, renderer.vk_indexBuffer, bufferSize);

    vkDestroyBuffer(renderer.vk_device, stagingBuffer, nullptr);
    vkFreeMemory(renderer.vk_device, stagingBufferMemory, nullptr);
}

void createVertexBuffer(Renderer::Context& renderer) {
    VkDeviceSize bufferSize = sizeof(renderer.vertices[0]) * renderer.vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(renderer, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(renderer.vk_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, renderer.vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(renderer.vk_device, stagingBufferMemory);

    createBuffer(renderer, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, renderer.vk_vertexBuffer, renderer.vk_vertexBufferMemory);

    copyBuffer(renderer, stagingBuffer, renderer.vk_vertexBuffer, bufferSize);

    vkDestroyBuffer(renderer.vk_device, stagingBuffer, nullptr);
    vkFreeMemory(renderer.vk_device, stagingBufferMemory, nullptr);

}

void createIndexBuffer(Renderer::Context& renderer) {
    VkDeviceSize bufferSize = sizeof(renderer.indices[0]) * renderer.indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(renderer, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(renderer.vk_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, renderer.indices.data(), (size_t)bufferSize);
    vkUnmapMemory(renderer.vk_device, stagingBufferMemory);

    createBuffer(renderer, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, renderer.vk_indexBuffer, renderer.vk_indexBufferMemory);

    copyBuffer(renderer, stagingBuffer, renderer.vk_indexBuffer, bufferSize);

    vkDestroyBuffer(renderer.vk_device, stagingBuffer, nullptr);
    vkFreeMemory(renderer.vk_device, stagingBufferMemory, nullptr);
}
void createUniformBuffers(Renderer::Context& renderer) {
    VkDeviceSize bufferSize = sizeof(Renderer::Context::UniformBufferObject);

    renderer.vk_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    renderer.vk_uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    renderer.vk_uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        createBuffer(renderer, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, renderer.vk_uniformBuffers[i], renderer.vk_uniformBuffersMemory[i]);

        vkMapMemory(renderer.vk_device, renderer.vk_uniformBuffersMemory[i], 0, bufferSize, 0, &renderer.vk_uniformBuffersMapped[i]);
    }
}

void createDescriptorPool(Renderer::Context& renderer) {
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(renderer.vk_device, &poolInfo, nullptr, &renderer.vk_descriptorPool) != VK_SUCCESS) {
        throwError("failed to create descriptor pool!", logLevelCritical);
    }
}

void createDescriptorSets(Renderer::Context& renderer) {
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, renderer.vk_descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = renderer.vk_descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    renderer.vk_descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(renderer.vk_device, &allocInfo, renderer.vk_descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = renderer.vk_uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(Renderer::Context::UniformBufferObject);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = renderer.vk_descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;

        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pImageInfo = nullptr; // Optional
        descriptorWrite.pTexelBufferView = nullptr; // Optional

        vkUpdateDescriptorSets(renderer.vk_device, 1, &descriptorWrite, 0, nullptr);
    }

}

void createDescriptorSetLayout(Renderer::Context& renderer) {
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if (vkCreateDescriptorSetLayout(renderer.vk_device, &layoutInfo, nullptr, &renderer.vk_descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void createGraphicsPipeline(Renderer::Context& renderer) {
    auto vertShaderCode = readFile("src/Renderer/Shaders/vert.spv");
    auto fragShaderCode = readFile("src/Renderer/Shaders/frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(renderer, vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(renderer, fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();


    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)renderer.vk_swapChainExtent.width;
    viewport.height = (float)renderer.vk_swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = renderer.vk_swapChainExtent;
    std::vector<VkDynamicState> dynamicStates = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // Rasterizer next.
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_TRUE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;  // Draw types
    rasterizer.lineWidth = 1;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {}; // Optional
    depthStencil.back = {}; // Optional

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &renderer.vk_descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(renderer.vk_device, &pipelineLayoutInfo, nullptr, &renderer.vk_pipelineLayout) != VK_SUCCESS) {
        throwError("failed to create pipeline layout!", logLevelError);
    } 

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = renderer.vk_pipelineLayout;
    pipelineInfo.renderPass = renderer.vk_renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional
    pipelineInfo.pDepthStencilState = &depthStencil;

    if (vkCreateGraphicsPipelines(renderer.vk_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &renderer.vk_graphicsPipeline) != VK_SUCCESS) {
        throwError("failed to create graphics pipeline!", logLevelError);
    }

    vkDestroyShaderModule(renderer.vk_device, fragShaderModule, nullptr);
    vkDestroyShaderModule(renderer.vk_device, vertShaderModule, nullptr);
};


void createFramebuffers(Renderer::Context& renderer) {
    renderer.vk_swapChainFramebuffers.resize(renderer.vk_swapChainImageViews.size());

    for (size_t i = 0; i < renderer.vk_swapChainImageViews.size(); i++) {
        
        std::array<VkImageView, 2> attachments = { renderer.vk_swapChainImageViews[i], renderer.vk_depthImageView };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderer.vk_renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = renderer.vk_swapChainExtent.width;
        framebufferInfo.height = renderer.vk_swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(renderer.vk_device, &framebufferInfo, nullptr, &renderer.vk_swapChainFramebuffers[i]) != VK_SUCCESS) {
            throwError("failed to create framebuffer!", logLevelError);
        }
    }
}

void createCommandPool(Renderer::Context& renderer) {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(renderer, renderer.vk_physicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(renderer.vk_device, &poolInfo, nullptr, &renderer.vk_commandPool) != VK_SUCCESS) {
        throwError("failed to create command pool!", logLevelError);
    }
}

void recordCommandBuffer(Renderer::Context& renderer, VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throwError("failed to begin recording command buffer!", logLevelError);
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderer.vk_renderPass;
    renderPassInfo.framebuffer = renderer.vk_swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = renderer.vk_swapChainExtent;

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
    clearValues[1].depthStencil = { 1.0f, 0 };

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.vk_graphicsPipeline);
    
    VkBuffer vertexBuffers[] = { renderer.vk_vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, renderer.vk_indexBuffer, 0, VK_INDEX_TYPE_UINT16);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(renderer.vk_swapChainExtent.width);
    viewport.height = static_cast<float>(renderer.vk_swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = renderer.vk_swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.vk_pipelineLayout, 0, 1, &renderer.vk_descriptorSets[currentFrame], 0, nullptr);
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(renderer.indices.size()), 1, 0, 0, 0);
    // draw cmd, buffer // index count // instance count// first index // vertex offset // first Instance

    vkCmdEndRenderPass(commandBuffer);
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throwError("failed to record command buffer!", logLevelError);
    }
}

void createCommandBuffer(Renderer::Context& renderer) {

    renderer.vk_commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = renderer.vk_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) renderer.vk_commandBuffers.size();

    if (vkAllocateCommandBuffers(renderer.vk_device, &allocInfo, renderer.vk_commandBuffers.data()) != VK_SUCCESS) {
        throwError("failed to allocate command buffers!", logLevelError);
    }
}

static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throwError("failed to open file!", logLevelError);
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;

}   

SwapChainSupportDetails querySwapChainSupport(Renderer::Context& renderer, VkPhysicalDevice device) {
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, renderer.vk_surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, renderer.vk_surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, renderer.vk_surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, renderer.vk_surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, renderer.vk_surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

void createSurface(Renderer::Context& renderer) {
    if (glfwCreateWindowSurface(renderer.vk_instance, renderer.window, nullptr, &renderer.vk_surface) != VK_SUCCESS) {
        throwError("failed to create window surface!", logLevelError);
    }
}

bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

std::vector<const char*> getRequiredExtensions() {

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

bool Renderer::frameClosed(Renderer::Context& renderer) {
    return glfwWindowShouldClose(renderer.window);
}

void Renderer::waitForDeviceIdle(Renderer::Context& renderer) {
    vkDeviceWaitIdle(renderer.vk_device);
};

void createSyncObjects(Renderer::Context& renderer) {

    renderer.vk_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderer.vk_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderer.vk_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(renderer.vk_device, &semaphoreInfo, nullptr, &renderer.vk_imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(renderer.vk_device, &semaphoreInfo, nullptr, &renderer.vk_renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(renderer.vk_device, &fenceInfo, nullptr, &renderer.vk_inFlightFences[i]) != VK_SUCCESS) {
            throwError("failed to create semaphores!", logLevelError);
        }
    }
}

void Renderer::runFrame(Renderer::Context& renderer) {

    vkWaitForFences(renderer.vk_device, 1, &renderer.vk_inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(renderer.vk_device, renderer.vk_swapChain, UINT64_MAX, renderer.vk_imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain(renderer);
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    vkResetFences(renderer.vk_device, 1, &renderer.vk_inFlightFences[currentFrame]);
    vkResetCommandBuffer(renderer.vk_commandBuffers[currentFrame], 0);
    recordCommandBuffer(renderer, renderer.vk_commandBuffers[currentFrame], imageIndex);

    Renderer::Context::UniformBufferObject ubo{};

    float aspectRatio = (renderer.windowHeight + 0.0f)/ renderer.windowWidth;
    float fovRadians = 1.0f / tanf(renderer.theta * 0.5f / 180.0f * pi);

    glm::mat4 proj = {
        aspectRatio * fovRadians,          0,                                                0,                                                                    0, 
                               0, fovRadians,                                                0,                                                                    0,
                               0,          0, renderer.Zfar / (renderer.Zfar - renderer.Znear), (-renderer.Zfar * renderer.Znear) / (renderer.Zfar - renderer.Znear), 
                               0,          0,                                                1,                                                                    0};

    ubo.proj = proj;

    memcpy(renderer.vk_uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { renderer.vk_imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &renderer.vk_commandBuffers[currentFrame];

    VkSemaphore signalSemaphores[] = { renderer.vk_renderFinishedSemaphores[currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(renderer.vk_graphicsQueue, 1, &submitInfo, renderer.vk_inFlightFences[currentFrame]) != VK_SUCCESS) {
        throwError("failed to submit draw command buffer!", logLevelError);
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapChains[] = { renderer.vk_swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional

    result = vkQueuePresentKHR(renderer.vk_presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || renderer.vk_framebufferResized) {
        renderer.vk_framebufferResized = false;
        recreateSwapChain(renderer);
    }
    else if (result != VK_SUCCESS) {
        throwError("failed to present swap chain image!", logLevelError);
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;


    if (renderer.vertices.size() > 0 && renderer.indices.size() > 0) {
        updateVertexBuffer(renderer);
        updateIndexBuffer(renderer);
    }
}

void Renderer::updateBuffer(Context& renderer) {
    updateVertexBuffer(renderer);
    updateIndexBuffer(renderer);
}

void Renderer::cleanupRenderer(Renderer::Context& renderer) {

    renderer.initialised = false;

    cleanupSwapChain(renderer);

    vkDestroySampler(renderer.vk_device, renderer.vk_textureSampler, nullptr);
    vkDestroyImageView(renderer.vk_device, renderer.vk_textureImageView, nullptr);

    vkDestroyImage(renderer.vk_device, renderer.vk_textureImage, nullptr);
    vkFreeMemory(renderer.vk_device, renderer.vk_textureImageMemory, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(renderer.vk_device, renderer.vk_uniformBuffers[i], nullptr);
        vkFreeMemory(renderer.vk_device, renderer.vk_uniformBuffersMemory[i], nullptr);
    }

    vkDestroyDescriptorPool(renderer.vk_device, renderer.vk_descriptorPool, nullptr);

    vkDestroyDescriptorSetLayout(renderer.vk_device, renderer.vk_descriptorSetLayout, nullptr);

    vkDestroyBuffer(renderer.vk_device, renderer.vk_indexBuffer, nullptr);
    vkFreeMemory(renderer.vk_device, renderer.vk_indexBufferMemory, nullptr);

    vkDestroyBuffer(renderer.vk_device, renderer.vk_vertexBuffer, nullptr);
    vkFreeMemory(renderer.vk_device, renderer.vk_vertexBufferMemory, nullptr);

    vkDestroyPipeline(renderer.vk_device, renderer.vk_graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(renderer.vk_device, renderer.vk_pipelineLayout, nullptr);

    vkDestroyRenderPass(renderer.vk_device, renderer.vk_renderPass, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(renderer.vk_device, renderer.vk_renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(renderer.vk_device, renderer.vk_imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(renderer.vk_device, renderer.vk_inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(renderer.vk_device, renderer.vk_commandPool, nullptr);

    vkDestroyDevice(renderer.vk_device, nullptr);

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(renderer.vk_instance, renderer.vk_debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(renderer.vk_instance, renderer.vk_surface, nullptr);
    vkDestroyInstance(renderer.vk_instance, nullptr);

    glfwDestroyWindow(renderer.window);

    glfwTerminate();
}
