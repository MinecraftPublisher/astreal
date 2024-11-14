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
    while ((result = malloc(size + sizeof(byte))) == 0);
    result[ 0 ] = 243;

    return &result[ 1 ];
}

ptr betterrealloc(ptr original, u4 size) {
    byte *result;
    while ((result = realloc(&((byte *) original)[ -1 ], size + sizeof(byte))) == 0);

    return &result[ 1 ];
}

void __free(byte *x) {
    if (x[ -3 ] == 247) { // array
        free(&x[ -3 ]);
    } else if (x[ -1 ] == 243) {
        free(&x[ -1 ]);
    } else {
        perror("What the hell\n");
        exit(1);
    }
}

#define malloc(x)     bettermalloc(x)
#define realloc(x, s) betterrealloc(x, s)
#define free(x)       __free((ptr)x)

ptr betterarray(u4 unit, u4 _count) {
    u4  count     = _count + 1;
    u4 *pointer   = malloc(unit * count + 2 * sizeof(u4));
    pointer[ -1 ] = 247;
    pointer[ 0 ]  = unit;
    pointer[ 1 ]  = count;

    return &pointer[ 2 ];
}

ptr grow(ptr __array, u4 inflation) {
    u4 *_array = __array;
    u4 *array  = &_array[ -2 ];
    u4  unit   = array[ 0 ];
    u4  count  = array[ 1 ];

    u4 new_size = unit * (count + inflation) + 2 * sizeof(u4);

    u4 *arr2  = realloc(array, new_size);
    arr2[ 1 ] = count + inflation;

    return &arr2[ 2 ];
}

ptr shrink(ptr __array, u4 deflation) {
    u4 *_array = __array;
    u4 *array  = &_array[ -2 ];
    u4  unit   = array[ 0 ];
    u4  count  = array[ 1 ];

    u4 new_size = unit * (count - deflation) + 2 * sizeof(u4);

    u4 *arr2                           = realloc(array, new_size);
    ((byte *) arr2)[ new_size - unit ] = 0;
    arr2[ 1 ]                          = count - deflation;

    return &arr2[ 2 ];
}

#define newarray(unit)     ((unit *) betterarray(sizeof(unit), 0))
#define array(unit, count) ((unit *) betterarray(sizeof(unit), count))
#define count(arr)         (((u4 *) arr)[ -1 ] - 1)
#define unit_size(arr)     (((u4 *) arr)[ -1 ] - 1)
#define last(arr)          (arr[ count(arr) - 1 ])
#define push(arr, item)                                                                  \
    ({                                                                                   \
        grow(arr, 1);                                                                    \
        arr[ count(arr) - 1 ] = item;                                                    \
    })
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

void bettersend(pipeout fd, ptr data, u4 size) {
    if (fd.write == 0) {
        perror("Write not allowed on this pipe\n");
        exit(1);
    }

    byte mode = 0; // mode 0 is static data and mode 1 is an array
    write(fd.write, &mode, sizeof(byte));
    write(fd.write, &size, sizeof(u4)); // send size data through pipe
    write(fd.write, data, size);        // send actual data
}

// !! ALLOCATES MEMORY! FREE AFTERWARDS !!
ptr betterread(pipeout fd) {
    if (fd.read == 0) {
        perror("Read not allowed on this pipe\n");
        exit(1);
    }

    byte mode = 254;
    while (read(fd.read, &mode, sizeof(byte)) <= 0) { // data is not yet available
        printf("%i\n", mode);

        if (mode == (byte) -1) {
            perror("Failed to read from pipe?");
            exit(1);
        }
    }

    u4 size;
    read(fd.read, &size, sizeof(u4)); // read size

    ptr data = malloc(sizeof(byte) * size);
    read(fd.read, data, sizeof(byte) * size);

    return data;
}

#define pipe() betterpipe()
#define send(unit, fd, data)                                                             \
    ({                                                                                   \
        unit *__Data = data;                                                             \
        bettersend(fd, &__Data, sizeof(__Data));                                         \
    })
void __read(int fd, void *buf, size_t size) { read(fd, buf, size); }
#define read(unit, fd)                                                                   \
    ({                                                                                   \
        unit *__Data = betterread(fd);                                                   \
        unit  _Data  = *__Data;                                                          \
        free(__Data);                                                                    \
        _Data;                                                                           \
    })

// Does not work with nested pointers.
void betterarraysend(pipeout fd, ptr _array) {
    if (fd.write == 0) {
        perror("Write not allowed on this pipe\n");
        exit(1);
    }

    u4 *array  = _array;
    u4  amount = count(array);
    u4  unit   = unit_size(array);

    bettersend(fd, &unit, sizeof(u4));
    bettersend(fd, &amount, sizeof(u4));

    byte *send_array = _array;

    for (u8 i = 0; i < (amount * unit); i++) {
        // bettersend(fd, &send_array[i], sizeof(byte)); // inefficient. weak.
        write(fd.write, &send_array[ i ], sizeof(byte)); // efficient. strong.
    }
}

ptr betterarrayread(pipeout fd) {
    if (fd.read == 0) {
        perror("Read not allowed on this pipe\n");
        exit(1);
    }

    u4 unit   = read(u4, fd);
    u4 amount = read(u4, fd);
    u8 size   = unit * amount;

    byte *arr = betterarray(unit, amount);

    for (u8 i = 0; i < size; i++) { __read(fd.read, &arr[ i ], 1); }

    return arr;
}

#define send_array(fd, arr)  betterarraysend(fd, arr)
#define read_array(unit, fd) ((unit *) betterarrayread(fd))