#include "math.h"

Vec2i new_Vec2i(int x, int y)
{
    Vec2i v = {
        .x = x,
        .y = y
    };
    return v;
}

int abs(int n)
{
    return (n < 0) ? -n : n;
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
