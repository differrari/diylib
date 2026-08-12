#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <vulkan/vulkan.h>
#endif
#include "graph_backend.h"
#include "syscalls/syscalls.h"

void graph_render(draw_ctx *ctx){
    // print("TODO: render loop here");
}

void graph_make_viewport(u32 w, u32 h){
    print("TODO: surface creation here");
}

void graph_resize_viewport(draw_ctx *ctx, u32 w, u32 h){
    print("TODO: resize surface here");
}