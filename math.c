#include <stdlib.h>
#include <math.h>
#include "math.h"

// Returns unbiased random integer in range [0, n)
int randint(int n)
{
    if (n - 1 == RAND_MAX) {
        return rand();
    }

    int r;
    int rand_end = (RAND_MAX / n) * n;
    while ((r = rand()) >= rand_end);
    return r % n;
}

int abs(int n)
{
    return (n < 0) ? -n : n;
}

float absf(float f)
{
    return (f < 0) ? -f : f;
}

int max(int a, int b)
{
    return (a > b) ? a : b;
}

float maxf(float a, float b)
{
    return (a > b) ? a : b;
}

float minf(float a, float b)
{
    return (a < b) ? a : b;
}

// The same as ceil. Saxuravi ;)
int saxu(float n)
{
    if ((n - (int)n) > 0.f)
        return ((int)n) + 1;
    return (int) n;
}

int is_num(char c)
{
    return c >= '0' && c <= '9';
}

double power(double n, int p)
{
    if (n == 0)
        return 0;

    // Negative power
    if (p < 0) {
        p = -p;
        n = 1/n;
    }
    
    double ans = 1;
    while (p-- > 0)
        ans *= n;
    return ans;
}

float lerp(float a, float b, float amount)
{
    return a + (b - a) * amount;
}

Vec2i new_Vec2i(int x, int y)
{
    Vec2i v = {
        .x = x,
        .y = y
    };
    return v;
}

inline float magnitude(Vec3f a)
{
    return sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}

Vec3f normalize(Vec3f a)
{
    float m = magnitude(a);
    a.x = a.x / m;
    a.y = a.y / m;
    a.z = a.z / m;
    return a;
}

Vec3f add_vec3f(Vec3f a, Vec3f b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}

Vec3f sub_vec3f(Vec3f a, Vec3f b)
{
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    return a;
}

float dot_prod(Vec3f a, Vec3f b)
{
    Vec3f c = {
        .x = a.x - b.x,
        .y = a.y - b.y,
        .z = a.z - b.z
    };

    float am = magnitude(a);
    float bm = magnitude(b);
    float cm = magnitude(c);

    return (am*am + bm*bm - cm*cm) / 2.;
}

Vec3f cross_prod(Vec3f a, Vec3f b)
{
    Vec3f n = {
        .x = a.y * b.z - a.z * b.y,
        .y = a.z * b.x - a.x * b.z,
        .z = a.x * b.y - a.y * b.x
    };
    return n;
}
