#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>
#include "graph_backend.h"
#include "syscalls/syscalls.h"

extern GLFWwindow* _window;

void graph_setup(){
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

const char** window_make_vk_extensions(uint32_t *amount){
    if (!amount) return 0;

    return glfwGetRequiredInstanceExtensions(amount);
}

VkResult window_make_vk_surface(VkInstance instance, VkSurfaceKHR *surface){
    return glfwCreateWindowSurface(instance, _window, NULL, surface);
}