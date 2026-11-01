#include "utils/menu.h"
#include "menu/load.h"
#include "menu/render.h"
#include "header_utils/composite.h"

void menu_init(){
    menu_reset();
    setup_menu();
    
    print("Loaded menu");
}

draw_ctx menu_ctx;

bool menu_render(draw_ctx *ctx){
    if (!ctx) return false;
    if (!menu_ctx.width) menu_ctx = dummy_draw_ctx(ctx->width, ctx->height);
    mouse_data data = {};
    get_mouse_status(&data);
    set_menu_color(0xFF123456);
    bool clicked = draw_menu(&menu_ctx, data.position, data.raw.buttons & (1 << LMB));
    composite(&menu_ctx, (int_point){}, 1, ctx, draw_ctx_rect(ctx));
    return clicked;
}