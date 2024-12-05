// For testing purposes!

unsigned char has_color = 1;

#include "headers/macros.h"
#include <sys/wait.h>

int main() {
    printf("%p\n", "Hello");
    printf("%p\n", copy_string("Hello"));
    var i = 2;
    printf("%p\n", &i);
}

