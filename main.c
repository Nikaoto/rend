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
    // Sort by y coord ascending (a.y < b.y< c.y)
    if (a.y > b.y) SWAP(a, b, Vec2i);
    if (b.y > c.y) SWAP(b, c, Vec2i);
    if (a.y > c.y) SWAP(a, c, Vec2i);
    
    gfx_color(255, 0, 0);
    line(a.x, a.y, c.x, c.y);
    line(a.x, a.y, b.x, b.y);
    line(b.x, b.y, c.x, c.y);

    int ac_dy = c.y - a.y;
    int ac_dx = c.x - a.x;
    float ac_k = (float) ac_dx / ac_dy;

    // Draw ab segment
    gfx_color(255, 255, 255);
    int ab_dy = b.y - a.y;
    int ab_dx = b.x - a.x;
    float ab_k = (float) ab_dx / ab_dy;
    if (ab_dx < 0) {
        SWAP(ac_k, ab_k, float);
    }
    for (int y = a.y; y <= b.y; y++) {
        for (int x = a.x + ac_k * (y - a.y); x <= a.x + ab_k * (y - a.y); x++) {
            gfx_point(x, y);
        }
    }

    gfx_color(255, 0, 0);
    line(a.x, a.y, c.x, c.y);
    line(a.x, a.y, b.x, b.y);
    line(b.x, b.y, c.x, c.y);

    // Swap them back
    if (ab_dx < 0) {
        SWAP(ac_k, ab_k, float);
    }
    
    // Draw bc segment
    gfx_color(255, 0, 255);
    int bc_dy = c.y - b.y;
    int bc_dx = c.x - b.x;
    float bc_k = (float) bc_dx / bc_dy;
    int xoff = a.x + ac_k * (b.y - a.y);
    int xoff2 = b.x;
    if (bc_dx > 0) {
        SWAP(a.x, b.x, int);
        SWAP(ac_k, bc_k, float);
        xoff = a.x;
        xoff2 = b.x + bc_k * (b.y - a.y);
    }
    for (int y = b.y; y <= c.y; y++) {
        for (int x = xoff + ac_k * (y - b.y); x <= xoff2 + bc_k * (y - b.y); x++) {
            gfx_point(x, y);
        }
    }
}

char* obj_file_name = "african_head.obj";

int main(void)
{
    int width = 600;
    int height = 600;
    gfx_open(width, height, "rend");

    char c;
    /* Model* m = parse_obj_file(obj_file_name); */
    while(1) {
        gfx_color(255, 255, 255);

        Vec2i t0[3] = {new_Vec2i(10, 70), new_Vec2i(50, 160), new_Vec2i(70, 80)};
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
