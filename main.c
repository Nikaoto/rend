#include <stdio.h>
#include <stdlib.h>
#include "rend.h"
#include "gfx.h"
#include "objparser.h"
#include "math.h"

void line(int x0, int y0, int x1, int y1)
{
    int steep = 0;
    if (abs(x0 - x1) < abs(y0 - y1)) {
        SWAP(x0, y0, int);
        SWAP(x1, y1, int);
        steep = 1;
    }

    if (x0 > x1) {
        SWAP(x0, x1, int);
        SWAP(y0, y1, int);
    }

    int dx = x1 - x0;
    int yinc = abs(y1 - y0) * 2;
    int yjump = 0;
    for (int x = x0, y = y0; x <= x1; x++) {
        if (steep)
            gfx_point(y, x);
        else
            gfx_point(x, y);

        yjump += yinc;

        if (yjump >= dx) {
            y += y0 < y1 ? 1 : -1;
            yjump -= dx * 2;
        }
    }
}

void rect(int x0, int y0, int x1, int y1)
{
    line(x0, y0, x1, y0);
    line(x1, y0, x1, y1);
    line(x1, y1, x0, y1);
    line(x0, y1, x0, y0);
}

void triangle(Vec2i a, Vec2i b, Vec2i c)
{
    line(a.x, a.y, b.x, b.y);
    line(a.x, a.y, c.x, c.y);
    line(b.x, b.y, c.x, c.y);
}

char* obj_file_name = "african_head.obj";

int main(void)
{
    int width = 600;
    int height = 600;
    gfx_open(width, height, "rend");

    char c;
//    Model* m = parse_obj_file("african_head.obj");
    while(1) {
        gfx_color(255, 255, 255);

        Vec2i t0[3] = {new_Vec2i(10, 70),   new_Vec2i(50, 160),  new_Vec2i(70, 80)}; 
        Vec2i t1[3] = {new_Vec2i(180, 50),  new_Vec2i(150, 1),   new_Vec2i(70, 180)}; 
        Vec2i t2[3] = {new_Vec2i(180, 150), new_Vec2i(120, 160), new_Vec2i(130, 180)}; 
        triangle(t0[0], t0[1], t0[2]);
        triangle(t1[0], t1[1], t1[2]);
        triangle(t2[0], t2[1], t2[2]);
        
        /* for (int i = 0; i < m->face_count; i++) { */
        /*     for (int j = 0; j < 3; j++) { */
        /*         Vec3 v0 = m->vertices[m->faces[i][j]]; */
        /*         Vec3 v1 = m->vertices[m->faces[i][(j+1)%3]]; */
        /*         int x0 = (v0.x + 1.) * width/2; */
        /*         int y0 = height - (v0.y + 1.) * height/2; */
        /*         int x1 = (v1.x + 1.) * width/2; */
        /*         int y1 = height - (v1.y + 1.) * height/2; */
        /*         line(x0, y0, x1, y1); */
        /*     } */
        /* } */
        c = gfx_wait();
        if (c == 'q' || c == '\x1b') break;
    }

    /* for (int i = 0; i < m->face_count; i++) { */
    /*     free(m->faces[i]); */
    /* } */
    /* free(m->faces); */
    /* free(m->vertices); */
    /* free(m); */
    return 0;
}
