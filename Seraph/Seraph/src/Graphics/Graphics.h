#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib> 
#include <vector>
#include <optional>

#include <cstdint> // Necessary for uint32_t
#include <limits> // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

struct vertices {
    int pos[3];
    unsigned char colour[4];
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

class Graphics {
private:

    //const char* shaderSource = "src/Graphics/Shaders/Basic.shader";

    GLFWwindow* window = NULL;
    VkInstance instance;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphicsQueue, presentQueue;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapChain;

    int screenWidth = 1920; // to detect later and change
    int screenHeight = 1080; // can and will change later
    //float fovMultiplier = 1.0f; // will add settiing to adjust.
    //int maxNumVerts = 1000;

    std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
    VkDebugUtilsMessengerEXT debugMessenger;
    std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    //float aspectRatio = (float)screenHeight / (float)screenWidth;

    //int maxSightDistance = 1000000; // 1000m , values stored as integer millimeters.
    //int minSightDistance = 1;         // 1mm , values stored as integer millimeters.
    
    //float fieldOfView = tan(M_PI/2 * fovMultiplier);

  
public:
	Graphics();
	~Graphics();

    void draw();
	int cleanup();
    
    int initGraphics();
    int initWindow();
    int initVulkan();
    void createInstance();
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
    void createSurface();

    bool isWindowOpen();
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkValidationLayerSupport();
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    std::vector<const char*> getRequiredExtensions();
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    void setupDebugMessenger();
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {

        std::cout << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
};