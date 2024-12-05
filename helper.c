// Helper application for determining sizes etc.

#include <stdio.h>
#include <string.h>

#define streq(x, y) (strcmp(x, y) == 0)
#define _mode       (argv[ 1 ])
#define mode(x, code) ({ if(streq(_mode, x)) code; return 0; })
#define val (argv[2])

struct array {
    unsigned int size;
    void* items;
};

int main(int argc, char** argv) {
    if (argc == 1) {
        printf("Give me something to work with!\n");
        return 1;
    }

    mode("size", {
        // byte, pointer, int, short, long, long long
        printf("byte ptr int short long long_long double array\n");
        printf("%lu %lu %lu %lu %lu %lu %lu %lu\n", sizeof(char), sizeof(void*), sizeof(int), sizeof(short), sizeof(long), sizeof(long long), sizeof(double), sizeof(struct array));
    });
}