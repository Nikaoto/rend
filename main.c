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
    if (a.y == b.y && a.y == c.y) return;

    // Sort by y coord ascending (a.y < b.y< c.y)
    if (a.y > b.y) SWAP(a, b, Vec2i);
    if (b.y > c.y) SWAP(b, c, Vec2i);
    if (a.y > c.y) SWAP(a, c, Vec2i);
/*    
    line(a.x, a.y, c.x, c.y);
    line(a.x, a.y, b.x, b.y);
    line(b.x, b.y, c.x, c.y);
*/
    int ac_dy = c.y - a.y;
    int ac_dx = c.x - a.x;
    if (ac_dy == 0) {
        SWAP(b, c, Vec2i);
        ac_dx = c.x - a.x;
        ac_dy = c.y - a.y;
    }
    float ac_k = (float) ac_dx / (float) ac_dy;

    // Draw ab segment
    int ab_dy = b.y - a.y;
    int ab_dx = b.x - a.x;
    if (ab_dy != 0) {
        float ab_k = (float) ab_dx / (float) ab_dy;
        if (ab_dx < 0) {
            SWAP(ac_k, ab_k, float);
        }
        for (int y = a.y; y <= b.y; y++) {
            for (int x = a.x + ac_k * (y - a.y); x <= a.x + ab_k * (y - a.y); x++) {
                gfx_point(x, y);
            }
        }

        // Swap them back
        if (ab_dx < 0) {
            SWAP(ac_k, ab_k, float);
        }
    }
    printf("done first half\n");
    
    // Draw bc segment
    int bc_dy = c.y - b.y;
    int bc_dx = c.x - b.x;
    if (bc_dy != 0) {
        float bc_k = (float) bc_dx / (float) bc_dy;
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
    printf("done!\n");
}

char* obj_file_name = "african_head.obj";

int main(void)
{
    int width = 600;
    int height = 600;
    gfx_open(width, height, "rend");

    char c;
    Model* m = parse_obj_file(obj_file_name);
    while(1) {
        gfx_color(255, 255, 255);

        /* triangle(new_Vec2i(10, 70),   new_Vec2i(50, 160),  new_Vec2i(70, 80)); */
        /* triangle(new_Vec2i(180, 50),  new_Vec2i(150, 1),   new_Vec2i(70, 180)); */
        /* triangle(new_Vec2i(180, 150), new_Vec2i(120, 160), new_Vec2i(130, 180)); */
        /* triangle(new_Vec2i(200, 200), new_Vec2i(200, 0), new_Vec2i(300, 0)); */
        /* triangle(new_Vec2i(300, 0), new_Vec2i(400, 0), new_Vec2i(400, 200)); */

        for (int i = 0; i < m->face_count; i++) {
            Vec2i screen_coords[3];
            for (int j = 0; j < 3; j++) {
                Vec3 v = m->vertices[m->faces[i][j]];
                screen_coords[j].x = (v.x + 1.) * width/2;
                screen_coords[j].y = height - (v.y + 1.) * height/2;
            }
            gfx_color(randint(256), randint(256), randint(256));
           gfx_line(screen_coords[0].x, screen_coords[0].y, screen_coords[1].x, screen_coords[1].y);
            gfx_line(screen_coords[1].x, screen_coords[1].y, screen_coords[2].x, screen_coords[2].y);
            gfx_line(screen_coords[0].x, screen_coords[0].y, screen_coords[2].x, screen_coords[2].y);
            triangle(screen_coords[0], screen_coords[1], screen_coords[2]);
        }
        c = gfx_wait();
        if (c == 'q' || c == '\x1b') break;
    }

    for (int i = 0; i < m->face_count; i++) {
        free(m->faces[i]);
    }
    free(m->faces);
    free(m->vertices);
    free(m);

    return 0;
}
