#ifndef REND_MATH_H_
#define REND_MATH_H_

#define SWAP(a, b, T) do { T tmp = a; a = b; b = tmp; } while (0)

int randint(int n);
int abs(int n);
float absf(float f);

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

#endif // REND_MATH_H_
