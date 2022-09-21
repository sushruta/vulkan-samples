#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include "app.h"

/**
#include "vktexture.hpp"

struct Vertex {
    glm::vec3 pos;
    glm::vec2 uv;
};

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::vec3 lightPosition;
};

class VulkanExample {
private:
    struct {
        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;
        GLFWwindow* window;

        std::vector<const char*> deviceExtensions{
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        std::vector<const char*> validationLayers{
            "VK_LAYER_KHRONOS_validation"
        };
    } vkapp;

    struct {
        VkSurfaceKHR surface;

        VkSwapchainKHR swapchain;
        std::vector<VkImage> swapchainImages;
        VkFormat swapchainImageFormat;
        VkExtent2D swapchainExtent;
        std::vector<VkImageView> swapchainImageViews;
        std::vector<VkFrameBuffer> swapchainFramebuffers;
    } swapchain;

    struct {
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice device;

        VkQueue graphicsQueue;
        VkQueue presentQueue;
    } device;

    struct {
        VkPipelineVertexInputStateCreateInfo inputState;
        std::vector<VkVertexInputBindingDescription> bindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    } vertices;
    
    struct {
        VkQueue graphicsQueue;
        VkQueue presentQueue;
    } graphics;
    
    struct {} compute;
};

**/

int main(int argc, char **argv) {
    VulkanApplication vkapp(512, 512, "simple vulkan app");

    try {
        vkapp.run();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
