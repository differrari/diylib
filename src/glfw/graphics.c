#include "string/string.h"
#include "ui/draw/draw.h"
#include "alloc/allocate.h"
#include "keyboard_input.h"
#include "mouse_input.h"
#include "keycode_convert.h"
#include "win_backend.h"
#include "graph_backend.h"

void destroy_draw_ctx(draw_ctx *ctx){
    // glfwTerminate();
}

void commit_draw_ctx(draw_ctx *ctx){
    // graph_render(ctx);
    // glfwSwapBuffers(_window);
    // glfwPollEvents();
}

void resize_draw_ctx(draw_ctx *ctx, uint32_t width, uint32_t height){
    // release(ctx->fb);
    // ctx->width = width;
    // ctx->height = height;
    // ctx->fb = zalloc(width*height*sizeof(color));
    // ctx->stride = 4 * width;
    // glfwSetWindowSize(_window, width, height);
    // graph_resize_viewport(ctx, width, height);
}

// extern void graph_test();

extern void* malloc(size_t size);

void request_draw_ctx(draw_ctx *ctx){
    uint32_t w = ctx->width ? ctx->width : 600;
    uint32_t h = ctx->height ? ctx->height : 300;
    win_make();

    ctx->fb = malloc(w*h*sizeof(color));//TODO: This has to be malloc due to some weird glfw/gtk mem corruption happening
    ctx->width = w;
    ctx->height = h;
    ctx->stride = sizeof(color) * w;    

    graph_init();

    ctx->width = w;
    ctx->height = h;
    
    graph_make_viewport(w, h);

}

bool read_event(kbd_event *out){
    // if (kbd_event_read == kbd_event_write) return false;

    // *out = event_queue[kbd_event_read];
    // kbd_event_read = (kbd_event_read + 1) % INPUT_BUFFER_CAPACITY;
    
    // return true;
}

void get_mouse_status(mouse_data *in){
    // in->raw.scroll = (u8)scroll;
    // scroll = 0;
    // in->raw.buttons = 0;
    // for (int i = 0; i < 3; i++)
    //     in->raw.buttons |= (glfwGetMouseButton(_window, i) & 1) << i;
    // in->raw.x = x_pos - old_x;
    // in->raw.y = y_pos - old_y;
    // in->position.x = x_pos;
    // in->position.y = y_pos;
    // old_x = x_pos;
    // old_y = y_pos;
}

bool should_close_ctx(){
    return false;
    // return glfwWindowShouldClose(_window);
}