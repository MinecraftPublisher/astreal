// For testing purposes!

unsigned char has_color = 1;

#include "headers/macros.h"
#include <sys/wait.h>

int main() {
    var new_baby = demon({
        print("I am a lil gremlin");
        sleep(0.5);
    });

    print("My lil gremlin is running");
    watch(new_baby);
    print("MY BABY IS DEAD");
}

