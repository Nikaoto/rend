#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include "rend.h"
#include "gfx.h"
#include "objparser.h"
#include "tgaparser.h"
#include "math.h"

int wireframe = 1;

#define SCALE 1

void point(int x, int y)
{

    gfx_point(x, y);
    return;
    for (int i = x*SCALE; i < x*(SCALE + 1); i++) {
        for (int j = y*SCALE; j < y*(SCALE + 1); j++) {
            gfx_point(i, j);
        }
    }
}

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
            point(y, x);
        else
            point(x, y);

        yjump += yinc;

        if (yjump >= dx) {
            y += y0 < y1 ? 1 : -1;
            yjump -= dx * 2;
        }
    }
}

inline void triangle_wireframe(Vec2i v[3])
{
    line(v[0].x, v[0].y, v[1].x, v[1].y);
    line(v[1].x, v[1].y, v[2].x, v[2].y);
    line(v[2].x, v[2].y, v[0].x, v[0].y);
}

void quad(int x0, int y0, int x1, int y1)
{
    line(x0, y0, x1, y0);
    line(x1, y0, x1, y1);
    line(x1, y1, x0, y1);
    line(x0, y1, x0, y0);
}

void triangle_(Vec3f a, Vec3f b, Vec3f c, float* zbuf)
{
    if (a.y == b.y && a.y == c.y) return;

    // Sort by y coord ascending (a.y < b.y < c.y)
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
                    point(x, y);
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
                    point(x, y);
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

/*void triangle_full(Vec3f v[3], Vec2i vt[3], Bitmap* tex, float* zbuf, float intensity)
{
    Vec3f* a = v;
    Vec3f* b = v + 1;
    Vec3f* c = v + 2;

    if (a->y == b->y && a->y == c->y) return;
    if (vt[0].y == vt[1].y && vt[0].y == vt[2].y) return;

    // Sort by y coord ascending (a->y < b->y < c->y)
    if (a->y > b->y) {
        SWAP(a, b, Vec3f*);
        SWAP(vt[0], vt[1], Vec2i);
    }
    if (a->y > c->y) {
        SWAP(a, c, Vec3f*);
        SWAP(vt[0], vt[2], Vec2i);
    }
    if (b->y > c->y) {
        SWAP(b, c, Vec3f*);
        SWAP(vt[1], vt[2], Vec2i);
    }

    float ac_dx = c->x - a->x;
    float ac_dy = c->y - a->y;
    float ac_dz = c->z - a->z;
    float ac_k = (float) ac_dx / (float) ac_dy;
    float ac_zk = (float) ac_dz / (float) ac_dy;
    // For texture vertex interpolation
    int ac_t_dx = vt[2].x - vt[0].x;
    int ac_t_dy = vt[2].y - vt[0].y;
    if (ac_t_dy == 0) ac_t_dy = 1;
    float ac_t_k = (float) ac_t_dx / (float) ac_t_dy;

    float ab_dx = b->x - a->x;
    float ab_dy = b->y - a->y;
    float ab_dz = b->z - a->z;

    // Can't sweep region from AB to AC if it doesn't exist
    if (ab_dy != 0) {
        float ab_k = (float) ab_dx / (float) ab_dy;
        float ab_zk = (float) ab_dz / (float) ab_dy;
        int ab_t_dx = vt[1].x - vt[0].x;
        int ab_t_dy = vt[1].y - vt[0].y;
        if (ab_t_dy == 0) ab_t_dy = 1;
        float ab_t_k = (float) ab_t_dx / (float) ab_t_dy;

        // Swap so startx <= endx
        int swapped = 0;
        if (ac_k > ab_k) {
            SWAP(ac_k, ab_k, float);
            swapped = 1;
        }

        // Swap so startz <= endz
        int swapped_zk = 0;
        if (ac_zk > ab_zk) {
            SWAP(ac_zk, ab_zk, float);
            swapped_zk = 1;
        }

        // Swap so start_tx <= end_tx
        int swapped_tk = 0;
        if (ac_t_k > ab_t_k) {
            SWAP(ac_t_k, ab_t_k, float);
            swapped_tk = 1;
        }

        for (int y = a->y; y <= b->y; y++) {
            int dy = y - a->y;
            float startz = a->z + dy * ac_zk;
            float endz = a->z + dy * ab_zk;
            float dz = endz - startz;
            int startx = a->x + dy * ac_k;
            int endx = a->x + dy * ab_k;
            int ty = lerp(vt[0].y, vt[1].y, (float) dy / (float) ab_dy);
            int start_tx = vt[0].x + dy * ac_t_k;
            int end_tx = vt[0].x + dy * ab_t_k;
            int x_dist = endx - startx + 1;
            for (int x = startx; x <= endx; x += 1) {
                int dx = x - startx;
                float z = startz + dz * (float) ((x - startx) / (float) x_dist);
                int zi = (int)(x + y * ZBUFFER_WIDTH);
                int tx = lerp(start_tx, end_tx, (float) dx / (float) x_dist);
                if (z > zbuf[zi]) {
                    int tex_xi = tx;
                    int tex_yi = ty;
                    int tex_i = (tex_xi + tex_yi * tex->width) * 3;
                    gfx_color(
                        intensity * tex->data[tex_i],
                        intensity * tex->data[tex_i + 1],
                        intensity * tex->data[tex_i + 2]);
                    point(x, y);
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
        if (swapped_tk) {
            SWAP(ab_t_k, ac_t_k, float);
        }
    }

    float bc_dx = c->x - b->x;
    float bc_dy = c->y - b->y;
    float bc_dz = c->z - b->z;

    // Can't sweep region from AC to BC if it doesn't exist
    if (bc_dy != 0) {
        float bc_k = (float) bc_dx / (float) bc_dy;
        float bc_zk = (float) bc_dz / (float) bc_dy;
        int bc_t_dx = vt[2].x - vt[1].x;
        int bc_t_dy = vt[2].y - vt[1].y;
        if (bc_t_dy == 0) bc_t_dy = 1;
        float bc_t_k = (float) bc_t_dx / (float) bc_t_dy;

        // If AC is to the right and BC is to the left
        if (ac_k < bc_k) {
            SWAP(ac_k, bc_k, float);
        }
        // Swap so that startz <= endz
        if (ac_zk < bc_zk) {
            SWAP(ac_zk, bc_zk, float);
        }
        // Swap so that start_tx <= end_tx
        if (ac_t_k < bc_t_k) {
            SWAP(ac_t_k, bc_t_k, float);
        }

        int y_dist = abs(bc_dy);
        for (int y = c->y; y >= b->y; y--) {
            int dy = y - c->y;
            float startz = c->z + dy * ac_zk;
            float endz = c->z + dy * bc_zk;
            float z_dist = endz - startz;
            int startx = c->x + dy * ac_k;
            int endx = c->x + dy * bc_k;
            int x_dist = endx - startx + 1;
            int ty = lerp(vt[1].y, vt[2].y, 1.f - (-dy / (float) y_dist));
            int start_tx = vt[1].x + dy * ac_t_k;
            int end_tx = vt[1].x + dy * bc_t_k;
            for (int x = startx; x <= endx; x++) {
                int dx = x - startx;
                int tx = lerp(start_tx, end_tx, (float) dx / (float) x_dist);
                float z = startz + z_dist * dx / (float) x_dist;
                int zi = (int)(x + y * ZBUFFER_WIDTH);
                if (z > zbuf[zi]) {
                    int tex_i = (tx + ty * tex->width) * 3;
                    gfx_color(
                        intensity * tex->data[tex_i],
                        intensity * tex->data[tex_i + 1],
                        intensity * tex->data[tex_i + 2]);
                    point(x, y);
                    zbuf[zi] = z;
                }
            }
        }
    }

    if (wireframe) {
        gfx_color(255, 0, 0);
        line(a->x, a->y, c->x, c->y);
        line(a->x, a->y, b->x, b->y);
        line(b->x, b->y, c->x, c->y);
    }
}*/

void triangle(Vec3f v[3], Vec2f vt[3], Bitmap* tex, float* zbuf, float intensity)
{
    Vec3f* a = v;
    Vec3f* b = v + 1;
    Vec3f* c = v + 2;

    // Skip if triangle parallel to view (invisible)
    if (a->y == b->y && a->y == c->y) return;

    // Skip if texture coords are wrong
    if (vt[0].y == vt[1].y && vt[0].y == vt[2].y) return;

    // Sort vertices by y coord ascending (a->y < b->y < c->y)
    if (a->y > b->y) SWAP(a, b, Vec3f*);
    if (a->y > c->y) SWAP(a, c, Vec3f*);
    if (b->y > c->y) SWAP(b, c, Vec3f*);

    // Sort texture vertices by y coord ascending
    if (vt[0].y > vt[1].y) SWAP(vt[0], vt[1], Vec2f);
    if (vt[0].y > vt[2].y) SWAP(vt[0], vt[2], Vec2f);
    if (vt[1].y > vt[2].y) SWAP(vt[1], vt[2], Vec2f);

    // AC slope
    float ac_dx = c->x - a->x;
    float ac_dy = c->y - a->y;
    float ac_dz = c->z - a->z;
    float ac_k = (ac_dy == 0.f) ? 0.f : ac_dx / ac_dy;
    float ac_z_k = (ac_dy == 0.f) ? 0.f : ac_dz / ac_dy;

    // AB slope
    float ab_dx = b->x - a->x;
    float ab_dy = b->y - a->y;
    float ab_dz = b->z - a->z;
    float ab_k = (ab_dy == 0.f) ? 0.f : ab_dx / ab_dy;
    float ab_z_k = (ab_dy == 0.f) ? 0.f : ab_dz / ab_dy;

    // BC slope
    float bc_dx = c->x - b->x;
    float bc_dy = c->y - b->y;
    float bc_dz = c->z - b->z;
    float bc_k = (bc_dy == 0.f) ? 0.f : bc_dx / bc_dy;
    float bc_z_k = (bc_dy == 0.f) ? 0.f : bc_dz / bc_dy;

    /* Offset by .5f because we're using pixel centers for the vectors.
       I ceil (saxu) the coordinates to comply with the top-left rule. */
    int y_start = saxu(a->y - .5f);
    int y_end = saxu(c->y - .5f);
    for (int y = y_start; y < y_end; y++) {
        int left_x = saxu((a->x + (y + .5f - a->y) * ac_k) - .5f);
        int right_x;
        float z_start = a->z + (y + .5f - a->y) * ac_z_k;
        float z_end;
        if (y < saxu(b->y - .5f) || (int)b->y == (int)c->y) {
            // First half
            right_x = saxu((a->x + (y + .5f - a->y) * ab_k) - .5f);
            z_end = a->z + (y + .5f - a->y) * ab_z_k;
        } else {
            // Second half
            right_x = saxu((b->x + (y + .5f - b->y) * bc_k) - .5f);
            z_end = b->z + (y + .5f - a->y) * bc_z_k;
        }
        int x_start, x_end;
        if (left_x > right_x) {
            x_start = right_x;
            x_end = left_x;
        } else {
            x_start = left_x;
            x_end = right_x;
        }

        // Interpolate texture coordinates
        float y_progress = (y + .5f - a->y) / (float) (y_end - y_start);
        float ty = lerp(vt[0].y, vt[2].y, y_progress);
        float tx_start = lerp(vt[0].x, vt[2].x, y_progress);
        float tx_end;
        if (ty < vt[1].y || vt[2].y == vt[3].y) {
            tx_end = lerp(vt[0].x, vt[1].x, y_progress);
        } else {
            tx_end = lerp(vt[1].x, vt[2].x, y_progress);
        }

        for (int x = x_start; x < x_end; x++) {
            float x_progress = (x - x_start) / (float) (x_end - x_start);
            float z = lerp(z_start, z_end, x_progress);
            if (zbuf[x + y * ZBUFFER_WIDTH] < z) {
                float tx = lerp(tx_start, tx_end, x_progress);
                int tex_i = ((int) (tx * tex->width) + (int) (ty * tex->height) * tex->width) * 3;
                gfx_color(
                    intensity * tex->data[tex_i],
                    intensity * tex->data[tex_i + 1],
                    intensity * tex->data[tex_i + 2]);
                point(x, y);
                zbuf[x + y * ZBUFFER_WIDTH] = z;
            }
        }
    }

    if (wireframe) {
        gfx_color(255, 0, 0);
        line(a->x, a->y, c->x, c->y);
        line(a->x, a->y, b->x, b->y);
        line(b->x, b->y, c->x, c->y);
    }
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

    if (argc > 3 && argv[3][0] == 'f')
        flip_vertices_vertically(m);

    if (argc > 4 && argv[3][0] == 'f')
        flip_texture_vertices_vertically(m);

    Bitmap* texture = parse_tga_file(texture_file_name, 1);

    Vec3f starting_light_dir = { .x = 0, .y = 0, .z = -1 };
    Vec3f light_dir = starting_light_dir;
    float *zbuffer = malloc(sizeof(float) * width * height);
    float light_move_amount = 0.05f;

    while(1) {
        // Clear zbuffer
        for (int i = 0; i < width*height; i++) {
            zbuffer[i] = -FLT_MAX;
        }

        gfx_color(255, 255, 255);

        // Draw texture bitmap
        /* for (int y = 0; y < texture->height; y++) { */
        /*     for (int x = 0; x < texture->width; x++) { */
        /*         gfx_color( */
        /*             texture->data[(x + y*texture->width)*3 ], */
        /*             texture->data[(x + y*texture->width)*3 + 1], */
        /*             texture->data[(x + y*texture->width)*3 + 2]); */
        /*         point(x, y); */
        /*     } */
        /* } */

        // Draw texturemap wireframe
        /* for (int i = 0; i < m->face_count; i++) { */
        /*     Vec2i texture_coords[3]; */
        /*     for (int j = 0; j < 3; j++) { */
        /*         Vec2 vt = m->texture_vertices[m->faces[i].texture_vertex_index[j]]; */
        /*         texture_coords[j].x = vt.x * texture->width; */
        /*         texture_coords[j].y = vt.y * texture->height; */
        /*     } */
        /*     gfx_color(255, 255, 255); */
        /*     triangle_wireframe(texture_coords); */
        /* } */

        for (int i = 0; i < m->face_count; i++) {
            // Get screen_coords
            Vec3f screen_coords[3];
            Vec3f world_coords[3];
            Vec2f tex_coords[3];
            for (int j = 0; j < 3; j++) {
                Vec3 v = m->vertices[m->faces[i].vertex_index[j]];
                Vec2 vt = m->texture_vertices[m->faces[i].texture_vertex_index[j]];
                world_coords[j] = (Vec3f) { .x = v.x, .y = v.y, .z = v.z };
                screen_coords[j].x = (v.x + 1.f) * width/2;
                screen_coords[j].y = (v.y + 1.f) * height/2;
                screen_coords[j].z = v.z + 1.f;
                tex_coords[j].x = vt.x;
                tex_coords[j].y = vt.y;
            }

            // Get light intensity depending on triangle orientation
            Vec3f side1 = sub_vec3f(world_coords[1], world_coords[0]);
            Vec3f side2 = sub_vec3f(world_coords[2], world_coords[0]);
            Vec3f normal = normalize(cross_prod(side1, side2));
            float intensity = dot_prod(normal, light_dir);

            // Only draw triangle if not back-facing (effectively culling back-faces)
            if (intensity > 0) {
                triangle(screen_coords, tex_coords, texture, zbuffer, intensity);
            }
        }

        /* Vec3f screen_coords[3] = { */
        /*     {.x = 20, .y = 20, .z = 0.1}, */
        /*     {.x = 100, .y = 40, .z = 0.1}, */
        /*     {.x = 40, .y = 100, .z = 0.1} */
        /* }; */
        /* Vec2f tex_coords[3] = { */
        /*     {.x = 20, .y = 20}, */
        /*     {.x = 100, .y = 40}, */
        /*     {.x = 40, .y = 100} */
        /* }; */
        /* triangle(screen_coords, tex_coords, texture, zbuffer, 1.0f); */

        /* for (int i = 0; i < width; i+=SCALE) { */
        /*     gfx_line() */

        gfx_flush();
        int c, quit = 0;
        while ((c = gfx_wait())) {
            quit = 0;
            if (c == 'q' || c == '\x1b') {
                quit = 1;
                break;
            } else if (c == 'w') {
                wireframe = !wireframe;
                break;
            } else if (c == 'i') {
                light_dir.y += light_move_amount;
                break;
            } else if (c == 'k') {
                light_dir.y -= light_move_amount;
                break;
            } else if (c == 'j') {
                light_dir.x -= light_move_amount;
                break;
            } else if (c == 'l') {
                light_dir.x += light_move_amount;
                break;
            } else if (c == 'u') {
                light_dir.z -= light_move_amount;
                break;
            } else if (c == 'o') {
                light_dir.z += light_move_amount;
                break;
            } else if (c == 'r') {
                light_dir = starting_light_dir;
                break;
            }
        }

        if (quit) break;
        gfx_clear();
    }

    free(zbuffer);
    free(texture->data);
    free(texture);
    free_model(m);

    return 0;
}
