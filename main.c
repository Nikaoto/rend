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

void quad(int x0, int y0, int x1, int y1)
{
    line(x0, y0, x1, y0);
    line(x1, y0, x1, y1);
    line(x1, y1, x0, y1);
    line(x0, y1, x0, y0);
}

void triangle(Vec2i a, Vec2i b, Vec2i c)
{
    if (a.y == b.y && a.y == c.y) return;

    // Sort by y coord ascending (a.y < b.y< c.y)
    if (a.y > b.y) SWAP(a, b, Vec2i);
    if (a.y > c.y) SWAP(a, c, Vec2i);
    if (b.y > c.y) SWAP(b, c, Vec2i);

    int ac_dx = c.x - a.x;
    int ac_dy = c.y - a.y;
    float ac_k = (float) ac_dx / (float) ac_dy;

    int ab_dx = b.x - a.x;
    int ab_dy = b.y - a.y;
    // Can't sweep region from AB to AC if it doesn't exist
    if (ab_dy != 0) {
        float ab_k = (float) ab_dx / (float) ab_dy;

        // Swap so startx <= endx
        int swapped = 0;
        if (ac_k > ab_k) {
            SWAP(ac_k, ab_k, float);
            swapped = 1;
        }
        for (int y = a.y; y <= b.y; y++) {
            int startx = a.x + (y - a.y) * ac_k;
            int endx = a.x + (y - a.y) * ab_k;
            for (int x = startx; x <= endx; x += 1) {
                gfx_point(x, y);
            }
        }
        // Swap them back
        if (swapped) {
            SWAP(ab_k, ac_k, float);
        }
    }

    int bc_dx = c.x - b.x;
    int bc_dy = c.y - b.y;

    /* Can't sweep region from AC to BC if it doesn't exist */
    if (bc_dy != 0) {
        /* printf("drawing second region\n"); */
        float bc_k = (float) bc_dx / (float) bc_dy;

        /* If AC is to the right and BC is to the left */
        if (ac_k < bc_k) {
            SWAP(ac_k, bc_k, float);
        }
        for (int y = c.y; y >= b.y; y--) {
            int startx = c.x + (y - c.y) * ac_k;
            int endx = c.x + (y - c.y) * bc_k;
            for (int x = startx; x <= endx; x++) {
                gfx_point(x, y);
            }
        }
    }

    #if wireframe == 1
    gfx_color(255, 0, 0);
    line(a.x, a.y, c.x, c.y);
    line(a.x, a.y, b.x, b.y);
    line(b.x, b.y, c.x, c.y);
    #endif
}

char* obj_file_name = "african_head.obj";

int main(int argc, char** argv)
{
    if (argc > 1) {
        obj_file_name = argv[1];
    }

    int width = 600;
    int height = 600;
    gfx_open(width, height, "rend");

    char c;
    #if render_model == 1
    Model* m = parse_obj_file(obj_file_name);
    #endif
    while(1) {
        gfx_color(255, 255, 255);

        /* triangle(new_Vec2i(10, 70),   new_Vec2i(50, 160),  new_Vec2i(70, 80)); */
        /* triangle(new_Vec2i(180, 50),  new_Vec2i(150, 1),   new_Vec2i(70, 180)); */
        /* triangle(new_Vec2i(180, 150), new_Vec2i(120, 160), new_Vec2i(130, 180)); */
        /* triangle(new_Vec2i(200, 200), new_Vec2i(200, 0), new_Vec2i(300, 0)); */
        /* triangle(new_Vec2i(300, 0), new_Vec2i(400, 0), new_Vec2i(400, 200)); */
        /* triangle(new_Vec2i(200, 200), new_Vec2i(200, 300), new_Vec2i(300, 300)); */
        /* triangle(new_Vec2i(300, 300), new_Vec2i(400, 300), new_Vec2i(400, 200)); */

        #if render_model == 1
        for (int i = 0; i < m->face_count; i++) {
            Vec2i screen_coords[3];
            for (int j = 0; j < 3; j++) {
                Vec3 v = m->vertices[m->faces[i][j]];
                screen_coords[j].x = (v.x + 1.) * width/2;
                screen_coords[j].y = height - (v.y + 1.) * height/2;
            }
            gfx_color(randint(256), randint(256), randint(256));
            triangle(screen_coords[0], screen_coords[1], screen_coords[2]);
        }
        #endif
        c = gfx_wait();
        if (c == 'q' || c == '\x1b') break;
    }

    #if render_model == 1
    for (int i = 0; i < m->face_count; i++) {
        free(m->faces[i]);
    }
    free(m->faces);
    free(m->vertices);
    free(m);
    #endif

    return 0;
}
