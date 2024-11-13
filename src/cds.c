#include "macros.c"
#include <string.h>

int main() {
    auto x = string("hello");

    grow(x, 1);
    // x[ (((u4 *) x)[ -1 ] - 1) - 1 ] = '!';

    printf("%s\n", x);
}