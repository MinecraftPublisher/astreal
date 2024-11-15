#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
// #include <fnctl.h>

typedef void         *ptr;
typedef unsigned long u8;
typedef unsigned int  u4;
typedef unsigned char byte;
#define auto __auto_type
#define var  __auto_type
#include <unistd.h>

// 243 is static data
// 247 is array

#define cast_index(arr, type, index) ((type *) arr)[ index ]
#define cast_ptr(arr, type, index)   (&cast_index(arr, type, index))

ptr bettermalloc(u4 size) {
    ptr result;
    while ((result = malloc(size + sizeof(byte))) == 0);
    cast_index(result, byte, 0) = 243;

    return &result[ 1 ];
}

ptr betterrealloc(ptr original, u4 size) {
    byte *result;
    byte  main_flag = cast_index(original, byte, -1);
    while ((result = realloc(cast_ptr(original, byte, -1), size + sizeof(byte))) == 0);
    result[ 0 ] = main_flag;

    return &result[ 1 ];
}

void __free(ptr x) {
    ptr pre_x = cast_ptr(x, u4, -2);
    if (cast_index(pre_x, byte, -1) == 247) { // array
        free(cast_ptr(pre_x, byte, -1));
    } else if (cast_index(x, byte, -1) == 243) {
        free(cast_ptr(x, byte, -1));
    } else {
        printf("What the hell %p\n", x);
        exit(1);
    }
}

void std_free(ptr x) { free(x); }

#define malloc(x)     bettermalloc(x)
#define realloc(x, s) betterrealloc(x, s)
#define free(x)       __free((ptr) x)

ptr betterarray(u4 unit, u4 _count) {
    u4  count                     = _count + 1;
    ptr pointer                   = malloc(unit * count + 2 * sizeof(u4));
    cast_index(pointer, byte, -1) = 247;
    cast_index(pointer, u4, 0)    = unit;
    cast_index(pointer, u4, 1)    = count;

    return &cast_index(pointer, u4, 2);
}

ptr grow(ptr _array, u4 inflation) {
    ptr array = cast_ptr(_array, u4, -2);
    u4  unit  = cast_index(array, u4, 0);
    u4  count = cast_index(array, u4, 1);

    u4 new_size = unit * (count + inflation) + 2 * sizeof(u4);

    ptr arr2                = realloc(array, new_size);
    cast_index(arr2, u4, 1) = count + inflation;

    return &arr2[ 2 ];
}

#define shrink(array, deflation) grow(array, -(deflation))

#define newarray(unit)     cast_ptr(betterarray(sizeof(unit), 0), unit, 0)
#define array(unit, count) cast_ptr(betterarray(sizeof(unit), count), unit, 0)
#define count(arr)         (cast_index(arr, u4, -1) - 1)
#define unit_size(arr)     cast_index(arr, u4, -2)
#define last(arr)          arr[ count(arr) - 1 ]
#define push(arr, item)                                                                  \
    ({                                                                                   \
        grow(arr, 1);                                                                    \
        last(arr) = item;                                                                \
    })
#define pop(arr)                                                                         \
    ({                                                                                   \
        auto output = arr[ count(arr) - 1 ];                                             \
        shrink(arr, 1);                                                                  \
        output;                                                                          \
    })
#define __copy_string(text, size)                                                        \
    ({                                                                                   \
        auto __Data = array(char, size);                                                 \
        strcpy(__Data, text);                                                            \
        __Data;                                                                          \
    })
#define copy_string(text) __copy_string(text, strlen(text))

void memoryFailure(char *file, int line) {
    printf(
        "----------------------------\n"
        " Memory allocation failure!\n"
        " At file %s\n"
        " Line %i\n"
        "----------------------------\n",
        file,
        line);
    exit(6); // SIGABRT
}

typedef char *string;

#define format(fmts, ...)                                                                \
    ({                                                                                   \
        string buf = NULL;                                                               \
        while (asprintf(&buf, fmts, __VA_ARGS__) < 0);                                   \
        string output = copy_string(buf);                                                \
        std_free(buf);                                                                   \
        output;                                                                          \
    })

typedef int pipeout;

string betterpipe() {
    int    pipe_name = rand();
    string real_name = format("/tmp/FIFO_PIPE_%i", pipe_name);

    mkfifo(real_name, 0666);

    return real_name;
}

pipeout open_pipe(string pipe_id) { return open(pipe_id, O_RDWR); }

void bettersend(pipeout fd, ptr data, u4 size) {
    byte mode = 0; // mode 0 is static data and mode 1 is an array

    write(fd, &mode, sizeof(byte));
    write(fd, &size, sizeof(u4)); // send size data through pipe
    write(fd, data, size);        // send actual data
}

// !! ALLOCATES MEMORY! FREE AFTERWARDS !!
ptr betterread(pipeout fd) {
    byte mode = 254;
    while (read(fd, &mode, sizeof(byte)) <= 0) { // data is not yet available
        printf("%i\n", mode);

        if (mode == (byte) -1) {
            perror("Failed to read from pipe?");
            exit(1);
        }
    }

    printf("done!\n");

    u4 size;
    read(fd, &size, sizeof(u4)); // read size

    ptr data = malloc(sizeof(byte) * size);
    read(fd, data, sizeof(byte) * size);

    return data;
}

#define pipe() betterpipe()
#define send(unit, fd, data)                                                             \
    ({                                                                                   \
        unit __Data = data;                                                              \
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
void betterarraysend(pipeout fd, ptr array) {
    u4 amount = count(array);
    u4 unit   = unit_size(array);

    bettersend(fd, &unit, sizeof(u4));
    bettersend(fd, &amount, sizeof(u4));

    for (u8 i = 0; i < (amount * unit); i++) {
        // bettersend(fd, &send_array[i], sizeof(byte)); // inefficient. weak.
        write(fd, cast_ptr(array, byte, i), sizeof(byte)); // efficient. strong.
    }
}

ptr betterarrayread(pipeout fd) {
    u4 unit   = read(u4, fd);
    u4 amount = read(u4, fd);
    u8 size   = unit * amount;

    ptr arr = betterarray(unit, amount);

    for (u8 i = 0; i < size; i++) { __read(fd, cast_ptr(arr, byte, i), 1); }

    return arr;
}

#define send_array(fd, arr)  betterarraysend(fd, arr)
#define read_array(unit, fd) ((unit *) betterarrayread(fd))

#define pcat(a, b) a##b
#define cat(a, b)  pcat(a, b)

#define repeat(x, ...)                                                                   \
    for (int cat(_, __VA_ARGS__) = 0; cat(_, __VA_ARGS__) < (x); cat(_, __VA_ARGS__)++)

#define expand(x) x
#define __demon(code, ID)                                                                \
    ({                                                                                   \
        __pid_t cat(fork_, ID) = fork();                                                 \
        if (cat(fork_, ID) < 0) {                                                        \
            perror("Failed to fork!");                                                   \
            exit(1);                                                                     \
        } else if (cat(fork_, ID) == 0) {                                                \
            code;                                                                        \
            exit(0);                                                                     \
        };                                                                               \
        cat(fork_, ID);                                                                  \
    })
#define demon(code) __demon(code, expand(__COUNTER__))