#include "glfw3.h"

GLFWwindow *_window = {};

void win_make(){
    glfwInit();
    // glfwSetErrorCallback(error_callback);

    glfwDefaultWindowHints();
#if __linux__
    glfwWindowHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
#endif
#if __APPLE__
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);
#endif
    _window = glfwCreateWindow(1920, 1080, "vulkantest", 0, 0);
}

void win_prepare_input(){
    // glfwSetKeyCallback(_window, key_callback);
    // glfwSetCursorPosCallback(_window, cursor_position_callback);
    // glfwSetScrollCallback(_window, scroll_callback);
}

static void error_callback(int error, const char* description)
{
    // print("Error: %s", description);
}

// #define INPUT_BUFFER_CAPACITY 64

// static kbd_event event_queue[INPUT_BUFFER_CAPACITY];
// static int kbd_event_read;
// static int kbd_event_write;
// static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
// {
//     uint32_t next_index = (kbd_event_write + 1) % INPUT_BUFFER_CAPACITY;

//     bool is_mod = (key >= GLFW_KEY_LEFT_SHIFT && key <= GLFW_KEY_RIGHT_SUPER);
//     key = glfw_to_redacted[key];

//     int press_ev = is_mod ? MOD_PRESS : KEY_PRESS;
//     int release_ev = is_mod ? MOD_RELEASE : KEY_RELEASE;
    
//     event_queue[kbd_event_write] = (kbd_event){
//         .type = action == GLFW_PRESS || action == GLFW_REPEAT ? press_ev : release_ev,
//         .key = is_mod ? 0 : key,
//         .modifier = is_mod ? key : 0
//     };
//     kbd_event_write = next_index;

//     if (kbd_event_write == kbd_event_read)
//         kbd_event_read = (kbd_event_read + 1) % INPUT_BUFFER_CAPACITY;
// }

double x_pos, y_pos;
static int old_x = 0;
static int old_y = 0;
static double scroll;

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    x_pos = xpos;
    y_pos = ypos;
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    scroll = yoffset;
}