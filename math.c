#include "math.h"
#include <stdlib.h>

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

Vec2i new_Vec2i(int x, int y)
{
    Vec2i v = {
        .x = x,
        .y = y
    };
    return v;
}
