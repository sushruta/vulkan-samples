#export VK_ICD_FILENAMES=/Users/sashidhar/VulkanSDK/1.3.224.0/macOS/share/vulkan/icd.d/MoltenVK_icd.json
#export VK_LAYER_PATH=/Users/sashidhar/VulkanSDK/1.3.224.0/macOS/share/vulkan/explicit_layer.d

VULKAN_DIR = /Users/sashidhar/VulkanSDK/1.3.224.0/macOS
ISTB = /usr/local/include/stb
ITOBJL = /usr/local/include/tinyobjloader

CFLAGS = -std=c++20 -O2
IFLAGS = -I $(VULKAN_DIR)/include -I /usr/local/include -I $(ISTB) -I $(ITOBJL)
LDFLAGS = -L /usr/local/lib -L $(VULKAN_DIR)/lib -lvulkan -lglfw3
FRAMEWORKFLAGS = -framework Cocoa -framework IOKit

export VK_ICD_FILENAMES=$(VULKAN_DIR)/share/vulkan/icd.d/MoltenVK_icd.json
export VK_LAYER_PATH=$(VULKAN_DIR)/share/vulkan/explicit_layer.d

shaders/vert.spv: shaders/shader.vert
	glslc shaders/shader.vert -o shaders/vert.spv

shaders/frag.spv: shaders/shader.frag
	glslc shaders/shader.frag -o shaders/frag.spv

build: main.cpp *.h shaders/vert.spv shaders/frag.spv
	g++-12 $(CFLAGS) $(IFLAGS) main.cpp $(LDFLAGS) $(FRAMEWORKFLAGS)

clean:
	rm -rf shaders/vert.spv shaders/frag.spv a.out

rm-assets:
	rm -rf models textures

get-assets:
	mkdir -p models textures
	curl "https://vulkan-tutorial.com/resources/viking_room.obj" --output models/viking_room.obj
	curl "https://vulkan-tutorial.com/resources/viking_room.png" --output textures/viking_room.png
