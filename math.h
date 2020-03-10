#ifndef REND_MATH_H_
#define REND_MATH_H_

#define SWAP(a, b, T) do { T tmp = a; a = b; b = tmp; } while (0)

int randint(int n);
int abs(int n);
int max(int a, int b);
float maxf(float a, float b);
float absf(float f);
float lerp(float a, float b, float amount);
//float lerpi(int a, int b, float amount);

typedef struct Vec3 {
    double x, y, z;
} Vec3;

typedef struct Vec2 {
    double x, y;
} Vec2;

typedef struct Vec3i {
    int x, y, z;
} Vec3i;

typedef struct Vec3f {
    float x, y, z;
} Vec3f;

typedef struct Vec2f {
    float x, y;
} Vec2f;

typedef struct Vec2i {
    int x, y;
} Vec2i;

Vec2i new_Vec2i(int x, int y);

Vec3f add_vec3f(Vec3f a, Vec3f b);
Vec3f sub_vec3f(Vec3f a, Vec3f b);
float magnitude(Vec3f a);
Vec3f normalize(Vec3f a);
float dot_prod(Vec3f a, Vec3f b);
Vec3f cross_prod(Vec3f a, Vec3f b);

typedef struct Face {
    int vertex_index[3];
    int texture_vertex_index[3];
    /* int normal_vertex_index[3]; */
} Face;

typedef struct Model {
    Vec3* vertices;
    int vertex_count;
    Face* faces;
    int face_count;
    Vec2* texture_vertices;
    int texture_vertex_count;
} Model;

typedef struct Bitmap {
    unsigned char* data;
    int width, height;
} Bitmap;

int is_num(char c);
double power(double n, int p);

#endif /* REND_MATH_H_ */
