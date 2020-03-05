#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include "rend.h"
#include "gfx.h"
#include "objparser.h"
#include "tgaparser.h"
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

void triangle(Vec3f a, Vec3f b, Vec3f c, float* zbuf)
{
    if (a.y == b.y && a.y == c.y) return;

    // Sort by y coord ascending (a.y < b.y< c.y)
    if (a.y > b.y) SWAP(a, b, Vec3f);
    if (a.y > c.y) SWAP(a, c, Vec3f);
    if (b.y > c.y) SWAP(b, c, Vec3f);

    float ac_dx = c.x - a.x;
    float ac_dy = c.y - a.y;
    float ac_dz = c.z - a.z;
    float ac_k = (float) ac_dx / (float) ac_dy;
    float ac_zk = (float) ac_dz / (float) ac_dy;

    float ab_dx = b.x - a.x;
    float ab_dy = b.y - a.y;
    float ab_dz = b.z - a.z;
    
    // Can't sweep region from AB to AC if it doesn't exist
    if (ab_dy != 0) {
        float ab_k = (float) ab_dx / (float) ab_dy;
        float ab_zk = (float) ab_dz / (float) ab_dy;

        /* Swap so startx <= endx */
        int swapped = 0;
        if (ac_k > ab_k) {
            SWAP(ac_k, ab_k, float);
            swapped = 1;
        }
        /* Swap so startz <= endz */
        int swapped_zk = 0;
        if (ac_zk > ab_zk) {
            SWAP(ac_zk, ab_zk, float);
            swapped_zk = 1;
        }

        for (int y = a.y; y <= b.y; y++) {
            float startz = a.z + (y - a.y) * ac_zk;
            float endz = a.z + (y - a.y) * ab_zk;
            float dz = endz - startz;
            int startx = a.x + (y - a.y) * ac_k;
            int endx = a.x + (y - a.y) * ab_k;
            int dx = endx - startx + 1;
            for (int x = startx; x <= endx; x += 1) {
                float z = startz + dz * (float) ((x - startx) / dx);
                int zi = (int)(x + y * WINDOW_WIDTH);
                if (z > zbuf[zi]) {
                    gfx_point(x, y);
                    zbuf[zi] = z;
                }
            }
        }
        // Swap them back
        if (swapped) {
            SWAP(ab_k, ac_k, float);
        }
        if (swapped_zk) {
            SWAP(ab_zk, ac_zk, float);
        }
    }

    float bc_dx = c.x - b.x;
    float bc_dy = c.y - b.y;
    float bc_dz = c.z - b.z;
    
    /* Can't sweep region from AC to BC if it doesn't exist */
    if (bc_dy != 0) {
        float bc_k = (float) bc_dx / (float) bc_dy;
        float bc_zk = (float) bc_dz / (float) bc_dy;

        /* If AC is to the right and BC is to the left */
        if (ac_k < bc_k) {
            SWAP(ac_k, bc_k, float);
        }
        /* Makes sure that startz < endz  */
        if (ac_zk < bc_zk) {
            SWAP(ac_zk, bc_zk, float);
        }
        for (int y = c.y; y >= b.y; y--) {
            float startz = c.z + (y - c.y) * ac_zk;
            float endz = c.z + (y - c.y) * bc_zk;
            float dz = endz - startz;
            int startx = c.x + (y - c.y) * ac_k;
            int endx = c.x + (y - c.y) * bc_k;
            int dx = endx - startx + 1;
            for (int x = startx; x <= endx; x++) {
                float z = startz + dz * (x - startx) / dx;
                int zi = (int)(x + y * WINDOW_WIDTH);
                if (z > zbuf[zi]) {
                    gfx_point(x, y);
                    zbuf[zi] = z;
                }
            }
        }
    }

    #if WIREFRAME == 1
    gfx_color(255, 0, 0);
    line(a.x, a.y, c.x, c.y);
    line(a.x, a.y, b.x, b.y);
    line(b.x, b.y, c.x, c.y);
    #endif
}

char* obj_file_name = "african_head.obj";
char* texture_file_name = "african_head_diffuse.tga";

int main(int argc, char** argv)
{
    if (argc > 1) {
        obj_file_name = argv[1];
    }
    if (argc > 2) {
        texture_file_name = argv[2];
    }

    int width = WINDOW_WIDTH;
    int height = WINDOW_HEIGHT;
    gfx_open(width, height, "rend");

    Model* m = parse_obj_file(obj_file_name);
    if (m == NULL) {
        printf("Parsing %s failed\n", obj_file_name);
        return 1;
    }

    Bitmap* texture = parse_tga_file(texture_file_name);

    Vec3f starting_light_dir = { .x = 0, .y = 0, .z = 1 };
    Vec3f light_dir = starting_light_dir;
    float *zbuffer = malloc(sizeof(float) * width * height);
    float light_move_amount = 0.05;
    
    while(1) {
        // Clear zbuffer
        for (int i = 0; i < width*height; i++) {
            zbuffer[i] = -FLT_MAX;
        }

        gfx_color(255, 255, 255);

        for (int i = 0; i < m->face_count; i++) {
            // Get screen_coords
            Vec3f screen_coords[3];
            Vec3f world_coords[3];
            for (int j = 0; j < 3; j++) {
                Vec3 v = m->vertices[m->faces[i][j]];
                world_coords[j] = (Vec3f) { .x = v.x, .y = v.y, .z = v.z };
                screen_coords[j].x = (v.x + 1.) * width/2;
                screen_coords[j].y = height - (v.y + 1.) * height/2;
                screen_coords[j].z = v.z;
            }

            // Get color of the face
            Vec3f side1 = sub_vec3f(world_coords[1], world_coords[0]);
            Vec3f side2 = sub_vec3f(world_coords[2], world_coords[0]);
            Vec3f normal = normalize(cross_prod(side1, side2));
            float intensity = dot_prod(normal, light_dir);
            if (intensity > 0) {
                gfx_color(intensity * 255, intensity * 255, intensity * 255);
                triangle(screen_coords[0], screen_coords[1], screen_coords[2], zbuffer);
            }
        }

        gfx_flush();
        int c = gfx_wait();
        if (c == 'q' || c == '\x1b')
            break;
        else if (c == 'i')
            light_dir.y += light_move_amount;
        else if (c == 'k')
            light_dir.y -= light_move_amount;
        else if (c == 'j')
            light_dir.x -= light_move_amount;
        else if (c == 'l')
            light_dir.x += light_move_amount;
        else if (c == 'u')
            light_dir.z -= light_move_amount;
        else if (c == 'o')
            light_dir.z += light_move_amount;
        else if (c == 'r')
            light_dir = starting_light_dir;
        gfx_clear();

        /* for (int y = 0; y < texture->height; y++) { */
        /*     for (int x = 0; x < texture->width; x++) { */
        /*         gfx_color( */
        /*             texture->data[(x + y*texture->width)*3 ], */
        /*             texture->data[(x + y*texture->width)*3 + 1], */
        /*             texture->data[(x + y*texture->width)*3 + 2]); */
        /*         gfx_point(x, texture->height - y); */
        /*     } */
        /* } */
        /* gfx_wait(); */
    }

    free(zbuffer);
    free(texture->data);
    free(texture);
    free_model(m);

    return 0;
}
