#define STD__STRING_HEAP(x) x

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef char* string;
typedef unsigned char u1;

typedef struct person {
    string name;
    u1 age;
} person;

string builtin__toString(int i) {
    int n = snprintf(NULL, 0, "%d", i) + 1;
    char *s = malloc(n);

    if (s != NULL)
        snprintf(s, n, "%d", i);
    return s;
}


string PROPERTY__person_ageToString(person me) {
    return builtin__toString(me.age);
}

int main() {
    person main = {
        .name = STD__STRING_HEAP("Mark"),
        .age = 200
    };

    printf("%s\n", PROPERTY__person_ageToString(main));

    free(main.name);

    return 0;
}