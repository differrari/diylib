#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

extern GLFWwindow* _window;

void graph_setup(){
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

//NOTE: something about syscalls.h breaks this function, most likely an overwritten function, so don't use it in this file
void graph_init(){
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
}