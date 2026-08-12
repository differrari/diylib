#include "keyboard_input.h"
#include "string/slice.h"
// #include "syscalls/syscalls.h"
#include "draw/draw.h"
#include "input_keycodes.h"

extern int graph_test();
extern void request_draw_ctx(draw_ctx *ctx);

int main(){

    draw_ctx ctx = {};
    ctx.width = 1920;
    ctx.height = 1080;

    request_draw_ctx(&ctx);
    return 0;

    // while (!should_close_ctx(&ctx)){
    //     fb_clear(&ctx, 0xffb4dd13);

    //     fb_draw_slice(&ctx, SLICE("Hello, World."), 20, 20, 3, 0xFF000000);

    //     kbd_event ev = {};
    //     if (read_event(&ev) && ev.key == KEY_ESC) halt(0);

    //     commit_draw_ctx(&ctx);

    // }

}