#pragma once

///This file provides graphics library entry points for the same functions the window usually handles, 
//since rendering and windowing are separate.

#include "types.h"
#include "graphic_types.h"

void graph_init();
void graph_make_viewport(u32 w, u32 h);
void graph_resize_viewport(draw_ctx *ctx, u32 w, u32 h);
void graph_render(draw_ctx *ctx);