#pragma once

#include "types.h"
#include "graphic_types.h"

gpu_point win_make(u32 w, u32 h);
void win_prepare_input();
void win_render();
void win_swap();