#include <stdio.h>
#include <stdlib.h>

typedef void         *ptr;
typedef unsigned long u8;
typedef unsigned int  u4;
typedef unsigned char byte;
#define auto __auto_type
#define var  __auto_type
#include <unistd.h>

// 243 is static data
// 247 is array

ptr bettermalloc(u4 size) {
    byte *result;
    while ((result = malloc(size)) == 0);

    printf("MALLOC %p\n", result);
    return result;
}

ptr betterrealloc(ptr original, u4 size) {
    byte *result;
    printf("meow %p %lu\n", &((byte *) original)[ -1 ], size + sizeof(byte));
    while ((result = realloc(&((byte *) original)[ -1 ], size + sizeof(byte))) == 0);

    return result;
}

#define malloc(x)     bettermalloc(x)
#define realloc(x, s) betterrealloc(x, s)
#define free(x)       free(&((byte *) x)[ -1 ])

ptr betterarray(u4 unit, u4 _count) {
    u4  count     = _count + 1;
    u4 *pointer   = malloc(unit * count + 2 * sizeof(u4));
    pointer[ -1 ] = 247;
    pointer[ 0 ]  = unit;
    pointer[ 1 ]  = count;

    return &pointer[ 2 ];
}

ptr grow(ptr _array, u4 inflation) {
    u4 *array = _array;
    u4  count = array[ -1 ];
    u4  size  = array[ -2 ];

    printf("%i %i\n", size, count);
    u4 new_size = size * (count + inflation) + 2 * sizeof(u4);

    printf("%p\n", &array[ -2 ]);
    u4 *arr2  = realloc(&array[ -2 ], new_size);
    arr2[ 1 ] = size + inflation;

    return &arr2[ 2 ];
}

ptr shrink(ptr _array, u4 deflation) {
    u4 *array = _array;
    u4  count = array[ -1 ];
    u4  size  = array[ -2 ];

    u4 new_size = (size - deflation) * count + 2 * sizeof(u4);

    u4 *arr2  = realloc(&array[ -2 ], new_size);
    arr2[ 1 ] = size - deflation;

    return &arr2[ 2 ];
}

#define newarray(unit)     (unit *) betterarray(sizeof(unit), 0)
#define array(unit, count) (unit *) betterarray(sizeof(unit), count)
#define count(arr)         (((u4 *) arr)[ -1 ] - 1)
#define unit_size(arr)     (((u4 *) arr)[ -1 ])
#define push(arr, item)                                                                  \
    grow(arr, 1);                                                                        \
    arr[ count(arr) - 1 ] = item
#define pop(arr)                                                                         \
    ({                                                                                   \
        auto output = arr[ count(arr) - 1 ];                                             \
        shrink(arr, 1);                                                                  \
        output;                                                                          \
    })
#define __string(text, size)                                                             \
    ({                                                                                   \
        auto __Data = array(char, size);                                                 \
        strcpy(__Data, text);                                                            \
        __Data;                                                                          \
    })
#define string(text) __string(text, strlen(text))

typedef struct {
    int read;
    int write;
} pipeout;

pipeout betterpipe() {
    int fd[ 2 ];
    while (pipe(fd) != 0);

    return (pipeout) { .read = fd[ 0 ], .write = fd[ 1 ] };
}

void bettersend(int fd, ptr data, u4 size) {
    byte mode = 0; // mode 0 is static data and mode 1 is an array
    write(fd, &mode, sizeof(byte));
    write(fd, &size, sizeof(u4)); // send size data through pipe
    write(fd, data, size);        // send actual data
}

// !! ALLOCATES MEMORY! FREE AFTERWARDS !!
ptr betterread(int fd) {
    byte mode = 254;
    while (read(fd, &mode, sizeof(byte)) <= 0) { // data is not yet available
        printf("%i\n", mode);

        if (mode == (byte) -1) {
            perror("Failed to read from pipe?");
            exit(1);
        }
    }

    u4 size;
    read(fd, &size, sizeof(u4)); // read size

    ptr data = malloc(sizeof(byte) * size);
    read(fd, data, sizeof(byte) * size);

    return data;
}

#define pipe() betterpipe()
#define send(unit, fd, data)                                                             \
    ({                                                                                   \
        unit *__Data = data;                                                             \
        bettersend(fd.write, &__Data, sizeof(__Data));                                   \
    })
void __read(int fd, void *buf, size_t size) { read(fd, buf, size); }
#define read(unit, fd)                                                                   \
    ({                                                                                   \
        unit *__Data = betterread(fd.read);                                              \
        unit  _Data  = *__Data;                                                          \
        free(__Data);                                                                    \
        _Data;                                                                           \
    })

// Does not work with nested pointers.
void betterarraysend(pipeout fd, ptr _array) {
    u4 *array  = _array;
    u4  amount = count(array);
    u4  unit   = unit_size(array);

    bettersend(fd.write, &unit, sizeof(u4));
    bettersend(fd.write, &amount, sizeof(u4));

    byte *send_array = _array;

    for (u8 i = 0; i < (amount * unit); i++) {
        // bettersend(fd, &send_array[i], sizeof(byte)); // inefficient. weak.
        write(fd.write, &send_array[ i ], sizeof(byte)); // efficient. strong.
    }
}

ptr betterarrayread(pipeout fd) {
    printf("%p\n", &fd);
    u4 unit   = read(u4, fd);
    u4 amount = read(u4, fd);
    u8 size   = unit * amount;

    byte *arr = betterarray(unit, amount);

    for (u8 i = 0; i < size; i++) { __read(fd.read, &arr[ i ], 1); }

    return arr;
}

#define send_array(fd, arr)  betterarraysend(fd, arr)
#define read_array(unit, fd) (unit *) betterarrayread(fd)