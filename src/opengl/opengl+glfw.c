#define GLFW_INCLUDE_NONE
#include "glfw3.h"

extern GLFWwindow* _window;

void graph_init(){
    glfwMakeContextCurrent(_window);
}

void graph_setup(){

}

void win_swap(){
    glfwSwapBuffers(_window);
}