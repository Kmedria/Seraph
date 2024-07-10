#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib> 
#include <vector>

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

struct vertices {
    int pos[3];
    unsigned char colour[4];
};

class Graphics {
private:

    //const char* shaderSource = "src/Graphics/Shaders/Basic.shader";

    GLFWwindow* window = NULL;
    VkInstance instance;
    //unsigned int VBO = -1, VAO = -1, EBO = -1;
    //unsigned int shaderProgram;

    int screenWidth = 1920; // to detect later and change
    int screenHeight = 1080; // can and will change later
    //float fovMultiplier = 1.0f; // will add settiing to adjust.
    //int maxNumVerts = 1000;

    std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

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
    void pickPhysicalDevice();

    bool isWindowOpen();
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions();
    void setupDebugMessenger();
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {

        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

};