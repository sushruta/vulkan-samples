#include "vkapp.h"

class HelloVulkan : public VulkanApplication {
    void initResources();
    void drawFrame();
    void waitBeforeRedraw();
    void cleanupResources();
};

void HelloVulkan::initResources() {}

void HelloVulkan::drawFrame() {}

void HelloVulkan::waitBeforeRedraw() {}

void HelloVulkan::cleanupResources() {}

int main(int argc, char **argv) {
    HelloVulkan hvapp;

    try {
        hvapp.run();
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
