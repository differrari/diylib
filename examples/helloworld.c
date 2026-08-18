#include "keyboard_input.h"
#include "string/slice.h"
#include "string/string.h"
#include "syscalls/syscalls.h"
#include "draw/draw.h"
#include "input_keycodes.h"
#include "debug/profiler.h"

char buf[256] = {};
int main(){

    draw_ctx ctx = {};
    ctx.width = 1920;
    ctx.height = 1080;

    request_draw_ctx(&ctx);

    int sampler = 0;
    int counter = 0;

    profiler_init();

    while (!should_close_ctx()){
        fb_clear(&ctx, 0xffb4dd13);

        if (counter >= 60){
            counter = 0;
            sampler = 1000/profiler_delta();
        } else profiler_delta();
        counter++;
        
        size_t n = string_format_buf(buf, 56, "Hello world %iFPS", sampler);
        fb_draw_slice(&ctx, (string_slice){buf,n}, 20, 20, 3, 0xFF000000);

        kbd_event ev = {};
        if (read_event(&ev) && ev.key == KEY_ESC) halt(0);

        commit_draw_ctx(&ctx);

    }

}