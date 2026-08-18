#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include "graph_backend.h"

void graph_render(draw_ctx *ctx){
    glRasterPos2i(0,ctx->height-1);
    glPixelZoom(1,-1);
    glDrawPixels(ctx->width,
     	 ctx->height,
         GL_BGRA,
     	 GL_UNSIGNED_INT_8_8_8_8_REV,
     	 ctx->fb);
}

void graph_make_viewport(u32 w, u32 h, u32 *fb){
    glViewport( 0, 0, w, h );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( 0, w, 0, h, -1, 1 );
}

void graph_resize_viewport(draw_ctx *ctx, u32 w, u32 h){
    glViewport( 0, 0, w, h );
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, 0, h, -1, 1);
}