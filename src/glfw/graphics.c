#include "string/string.h"
#include "alloc/allocate.h"
#include "keyboard_input.h"
#include "mouse_input.h"
#include "win_backend.h"
#include "graph_backend.h"

void destroy_draw_ctx(draw_ctx *ctx){
    // glfwTerminate();
}

void commit_draw_ctx(draw_ctx *ctx){
    graph_render(ctx);
    win_render();
}

extern int print(const char *fmt, ...);
extern float display_scale;

void request_draw_ctx(draw_ctx *ctx){
    uint32_t w = ctx->width ?: 600;
    uint32_t h = ctx->height ?: 300;
    gpu_point size = win_make(w,h);

    graph_init();

    ctx->fb = zalloc(size.x*size.y*sizeof(color));
    ctx->width = size.x;
    ctx->height = size.y;
    ctx->stride = sizeof(color) * size.x;
    ctx->display_scale = size.x/w;
    display_scale = ctx->display_scale;
        
    graph_make_viewport(size.x, size.y,ctx->fb);
    win_prepare_input();
}

#define INPUT_BUFFER_CAPACITY 64

extern kbd_event event_queue[];
extern int kbd_event_read;
extern int kbd_event_write;
bool read_event(kbd_event *out){
    if (kbd_event_read == kbd_event_write) return false;

    *out = event_queue[kbd_event_read];
    kbd_event_read = (kbd_event_read + 1) % INPUT_BUFFER_CAPACITY;
    
    return true;
}