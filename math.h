#pragma once

#define SWAP(a, b, t) { t tmp = a; a = b; b = tmp; }

int abs(int n);

typedef struct Vec3 {
    double x, y, z;
} Vec3;

typedef struct Vec3i {
    int x, y, z;
} Vec3i;

typedef struct Vec3f {
    float x, y, z;
} Vec3f;

typedef struct Vec2i {
    int x, y;
} Vec2i;

Vec2i new_Vec2i(int x, int y);

typedef struct Model {
    Vec3* vertices;
    int vertex_count;
    int** faces;
    int face_count;
} Model;

int is_num(char c);
double power(double n, int p);

