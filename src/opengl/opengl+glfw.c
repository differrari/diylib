#define GLFW_INCLUDE_NONE
#include "glfw3.h"

extern GLFWwindow* _window;

void graph_init(){
    glfwMakeContextCurrent(_window);
}