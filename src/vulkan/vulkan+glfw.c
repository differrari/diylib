#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "graph_backend.h"

// extern GLFWwindow* _window;

void graph_setup(){
    // glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

const char** window_make_vk_extensions(uint32_t *amount){
    if (!amount) return 0;

    return glfwGetRequiredInstanceExtensions(amount);
}