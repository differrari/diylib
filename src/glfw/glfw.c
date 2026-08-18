#include <vulkan/vulkan.h>
#include "glfw3.h"
#include "graph_backend.h"
#include "syscalls/syscalls.h"
#include "win_backend.h"
#include "keycode_convert.h"

GLFWwindow *_window = {};


static void error_callback(int error, const char* description)
{
    print("GLFW Error: %s", description);
    halt(-1);
}

void win_make(u32 w, u32 h){
    glfwSetErrorCallback(error_callback);
#if __linux__
    // glfwInitHint(GLFW_WAYLAND_LIBDECOR, GLFW_WAYLAND_DISABLE_LIBDECOR);
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
    glfwInitHint(GLFW_X11_XCB_VULKAN_SURFACE, GLFW_TRUE);
    #endif
    glfwInit();
    
    glfwDefaultWindowHints();
#if __APPLE__
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);
#endif
    graph_setup();
    _window = glfwCreateWindow(w, h, "vulkantest", 0, 0);
}

void win_render(){
    win_swap();
    glfwPollEvents();
}

#define INPUT_BUFFER_CAPACITY 64

kbd_event event_queue[INPUT_BUFFER_CAPACITY];
int kbd_event_read;
int kbd_event_write;
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    uint32_t next_index = (kbd_event_write + 1) % INPUT_BUFFER_CAPACITY;

    bool is_mod = (key >= GLFW_KEY_LEFT_SHIFT && key <= GLFW_KEY_RIGHT_SUPER);
    key = glfw_to_redacted[key];

    int press_ev = is_mod ? MOD_PRESS : KEY_PRESS;
    int release_ev = is_mod ? MOD_RELEASE : KEY_RELEASE;
    
    event_queue[kbd_event_write] = (kbd_event){
        .type = action == GLFW_PRESS || action == GLFW_REPEAT ? press_ev : release_ev,
        .key = is_mod ? 0 : key,
        .modifier = is_mod ? key : 0
    };
    kbd_event_write = next_index;

    if (kbd_event_write == kbd_event_read)
        kbd_event_read = (kbd_event_read + 1) % INPUT_BUFFER_CAPACITY;
}

double x_pos, y_pos, old_x, old_y;
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

void get_mouse_status(mouse_data *in){
    in->raw.scroll = (u8)scroll;
    scroll = 0;
    in->raw.buttons = 0;
    for (int i = 0; i < 3; i++)
        in->raw.buttons |= (glfwGetMouseButton(_window, i) & 1) << i;
    in->raw.x = x_pos - old_x;
    in->raw.y = y_pos - old_y;
    in->position.x = x_pos;
    in->position.y = y_pos;
    old_x = x_pos;
    old_y = y_pos;
}

bool should_close_ctx(){
    return glfwWindowShouldClose(_window);
}

void win_prepare_input(){
    glfwSetKeyCallback(_window, key_callback);
    glfwSetCursorPosCallback(_window, cursor_position_callback);
    glfwSetScrollCallback(_window, scroll_callback);
}