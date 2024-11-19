// For testing purposes!

#include "macros.h"

bool has_color = yes;

int test_me() {
    return 4;
}

int main() {
    var me = get_me();
    printf("Hello world! %p\n", me);
    typeof(test_me)* ptr = dynamic(me, "test_me");

    printf("%p %i\n", ptr, ptr());

    dlclose(me);
}