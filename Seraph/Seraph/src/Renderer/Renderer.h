#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "CommonIncludes.h"
#include <vector>
#include <optional>
#include <set>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <fstream>
#include <array>
#include "glm/glm.hpp"

#include "src/Engine/Entity/Vertex.h"

const float pi = 3.14159f;

namespace Renderer {
	struct Context;
}

const int MAX_FRAMES_IN_FLIGHT = 2;
static uint32_t currentFrame = 0;

struct VertexInputDescription {

	std::vector<VkVertexInputBindingDescription> bindings;
	std::vector<VkVertexInputAttributeDescription> attributes;

	VkPipelineVertexInputStateCreateFlags flags = 0;
};

// A struct that will manage our indices, we have to find physical devices that supports each feature to use said feature.
struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

// Initialises glfw window.
void initWindow(Renderer::Context& renderer, const char* windowName, int width, int height);

// runs all initialisiations required by vulkan
void initVulkan(Renderer::Context& renderer);

// creates the vulkan debugging logger. It also shows tips for performance and curenlt outputs everything.
void setupDebugMessenger(Renderer::Context& renderer);

// searches through available devices and picks the most suitable one (if any).
void pickPhysicalDevice(Renderer::Context& renderer);

void createInstance(Renderer::Context& renderer);
void createLogicalDevice(Renderer::Context& renderer);
void createSurface(Renderer::Context& renderer);
void createSwapChain(Renderer::Context& renderer);
void createImageViews(Renderer::Context& renderer);
void createDescriptorSetLayout(Renderer::Context& renderer);
void createGraphicsPipeline(Renderer::Context& renderer);
bool checkDeviceExtensionSupport(VkPhysicalDevice device);
void createRenderPass(Renderer::Context& renderer);
void createFramebuffers(Renderer::Context& renderer);
void createCommandPool(Renderer::Context& renderer);
void createTextureImage(Renderer::Context& renderer);
void createTextureImageView(Renderer::Context& renderer);
void createTextureSampler(Renderer::Context& renderer);
void createDepthResources(Renderer::Context& renderer);
void createVertexBuffer(Renderer::Context& renderer);
void createIndexBuffer(Renderer::Context& renderer);
void createUniformBuffers(Renderer::Context& renderer);
void createDescriptorPool(Renderer::Context& renderer);
void createDescriptorSets(Renderer::Context& renderer);
void createCommandBuffer(Renderer::Context& renderer);
void createSyncObjects(Renderer::Context& renderer);

void updateVertexBuffer(Renderer::Context& renderer);
//void updateIndexBuffer(Renderer::Context& renderer);


void transitionImageLayout(Renderer::Context& renderer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
VkImageView createImageView(Renderer::Context& renderer, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
void createBuffer(Renderer::Context& renderer, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
void copyBufferToImage(Renderer::Context& renderer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

// Finds the families that each device is in. This lets us check each devices features.
QueueFamilyIndices findQueueFamilies(Renderer::Context& renderer, VkPhysicalDevice device);
SwapChainSupportDetails querySwapChainSupport(Renderer::Context& renderer, VkPhysicalDevice device);
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
bool checkValidationLayerSupport();
std::vector<const char*> getRequiredExtensions();
bool checkDeviceExtensionSupport(VkPhysicalDevice device);
bool isDeviceSuitable(Renderer::Context& renderer, VkPhysicalDevice device);

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" }; // Currently using defualt layers as set by KHRONOS
const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

namespace Renderer {

	void initRenderer(Context& renderer);
	void runFrame(Context& renderer);
	bool frameClosed(Context& renderer);
	void cleanupRenderer(Context& renderer);
	void waitForDeviceIdle(Context& renderer);
	void updateBuffer(Context& renderer);

	struct Context {
		// This context uses vulkan only for now.	

		GLFWwindow* window;

		float theta = 90.0f;
		float Znear = 0.1f;
		float Zfar = 1000.0f;

		bool initialised = false;

		const char* windowName = nullptr;
		uint16_t windowHeight = 0;
		uint16_t windowWidth = 0;

		VkInstance vk_instance; // Serves as a handle, that is required, to use vulkan.
		VkDebugUtilsMessengerEXT vk_debugMessenger; // An object that directs all vulkan degugging to a function that directs them to the Seraph Logger.
		VkPhysicalDevice vk_physicalDevice = VK_NULL_HANDLE; // A struct that stores data on your physical adevice in your hardware that allows us to check the different features and limitations it has.
		VkDevice vk_device; // Serves as a handle over the physical device just as vk_instance serves over vulkan.
		VkQueue vk_graphicsQueue;
		VkQueue vk_presentQueue;
		VkSurfaceKHR vk_surface;
		VkSwapchainKHR vk_swapChain;
		VkFormat vk_swapChainImageFormat;
		VkExtent2D vk_swapChainExtent;
		VkRenderPass vk_renderPass;
		VkDescriptorSetLayout vk_descriptorSetLayout;
		VkPipelineLayout vk_pipelineLayout;
		VkPipeline vk_graphicsPipeline;
		VkCommandPool vk_commandPool;
		VkBuffer vk_vertexBuffer;
		VkDeviceMemory vk_vertexBufferMemory;
		VkBuffer vk_indexBuffer;
		VkDeviceMemory vk_indexBufferMemory;
		std::vector<VkCommandBuffer> vk_commandBuffers;

		std::vector<VkBuffer> vk_uniformBuffers;
		std::vector<VkDeviceMemory> vk_uniformBuffersMemory;
		std::vector<void*> vk_uniformBuffersMapped;
		VkDescriptorPool vk_descriptorPool;
		std::vector<VkDescriptorSet> vk_descriptorSets;

		std::vector<VkImage> vk_swapChainImages;
		std::vector<VkImageView> vk_swapChainImageViews;
		std::vector<VkFramebuffer> vk_swapChainFramebuffers;

		std::vector<VkSemaphore> vk_imageAvailableSemaphores;
		std::vector<VkSemaphore> vk_renderFinishedSemaphores;
		std::vector<VkFence> vk_inFlightFences;

		VkImage vk_textureImage;
		VkImageView vk_textureImageView;
		VkSampler vk_textureSampler;
		VkDeviceMemory vk_textureImageMemory;

		VkImage vk_depthImage;
		VkDeviceMemory vk_depthImageMemory;
		VkImageView vk_depthImageView;

		bool vk_framebufferResized = false;

		std::vector<Vertex> vertices;
		std::vector<uint16_t> indices;

		struct UniformBufferObject {
			glm::mat4 proj;
		};

		Context();
		Context(const char* windowName, int width, int height);

		~Context() {
			if (initialised) {
				cleanupRenderer(*this);
			}
		}

	};

}