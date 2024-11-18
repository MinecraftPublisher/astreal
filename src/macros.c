#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

typedef void         *ptr;
typedef unsigned long u8;
typedef unsigned int  u4;
typedef unsigned char byte;
typedef byte bool;
#define auto __auto_type
#define var  __auto_type
#include <unistd.h>

// 243 is static data
// 247 is array

#define cast_index(arr, type, index) ((type *) arr)[ index ]
#define cast_ptr(arr, type, index)   (&cast_index(arr, type, index))

ptr __malloc(u4 size) { return malloc(size); }

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
        printf("Warn: not freeing normal memory %p\n", x);
        // exit(1);
    }
}

void std_free(ptr x) { free(x); }

#define malloc(x)     bettermalloc(x)
#define realloc(x, s) betterrealloc(x, s)
#define free(x)       __free((ptr) x)
#define new(type)                                                                        \
    ({                                                                                   \
        byte *__ptr__ = malloc(sizeof(type));                                            \
        __ptr__[ 0 ]  = 0;                                                               \
        (type *) __ptr__;                                                                \
    })

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

#define get(type, array, index)                                                          \
    (type)({                                                                             \
        if (index >= count(array)) {                                                     \
            printf("ERROR: Index %i out of bounds (%i).\n", index, count(array));        \
            exit(1);                                                                     \
        }                                                                                \
        array[ index ];                                                                  \
    })
#define set(type, array, index, value)                                                   \
    ({                                                                                   \
        if (index >= count(array)) {                                                     \
            printf("ERROR: Index %i out of bounds (%i).\n", index, count(array));        \
            exit(1);                                                                     \
        }                                                                                \
        cast_index(array, type, index) = value;                                          \
    })
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
        var output = arr[ count(arr) - 1 ];                                              \
        shrink(arr, 1);                                                                  \
        output;                                                                          \
    })
#define __copy_string(text, size)                                                        \
    ({                                                                                   \
        var __Data = array(char, size);                                                  \
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

// #define print(text) printf(text "\n")
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

typedef struct {
    string  read_id;
    string  write_id;
    pipeout read;
    pipeout write;
} stream;

stream create_stream() {
    string  read_pipe  = betterpipe();
    string  write_pipe = betterpipe();
    pipeout read_fd    = open_pipe(read_pipe);
    pipeout write_fd   = open_pipe(write_pipe);

    return (stream) {
        .read_id = read_pipe, .write_id = write_pipe, .read = read_fd, .write = write_fd
    };
}

void stream_to_station(stream input, string station) {
    var config_file = fopen(station, "a");
    fprintf(config_file, "%s\n%s\n", input.write_id, input.read_id);
    fclose(config_file);
}

#define stream() create_stream()

#define stream_and_send(filename)                                                        \
    ({                                                                                   \
        var __Output = stream();                                                         \
        stream_to_station(__Output, filename);                                           \
        __Output;                                                                        \
    })

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

ptr betterarrayread(stream fd) {
    u4 unit   = read(u4, fd);
    u4 amount = read(u4, fd);
    u8 size   = unit * amount;

    ptr arr = betterarray(unit, amount);

    for (u8 i = 0; i < size; i++) { __read(fd.read, cast_ptr(arr, byte, i), 1); }

    return arr;
}

#define DONT_TOUCH_MESSAGE                                                               \
    "!DO NOT TOUCH THIS FILE!\n"                                                         \
    "This file is a communication stream, altering its contents will cause programs to " \
    "crash.\n\n"
#define DONT_TOUCH_MESSAGE_LENGTH 116

int file_size(FILE *fp) {
    int last_pos = ftell(fp);
    fseek(fp, 0, SEEK_END);
    int end_pos = ftell(fp);

    fseek(fp, last_pos, SEEK_SET);

    return end_pos - last_pos;
}

#define create_station(filename, pipe_name, code)                                        \
    ({                                                                                   \
        var config_file = fopen(filename, "w");                                          \
        fprintf(config_file, DONT_TOUCH_MESSAGE);                                        \
        fclose(config_file);                                                             \
                                                                                         \
        config_file = fopen(filename, "r");                                              \
        fseek(config_file, 0, SEEK_END);                                                 \
                                                                                         \
        while (1) {                                                                      \
            int size = file_size(config_file);                                           \
            if (size > 0) {                                                              \
                string read_pipe_id = newarray(char);                                    \
                char   init         = fgetc(config_file);                                \
                                                                                         \
                do push(read_pipe_id, init);                                             \
                while ((init = fgetc(config_file)) != '\n' && init != EOF);              \
                                                                                         \
                string write_pipe_id = newarray(char);                                   \
                init                 = fgetc(config_file);                               \
                                                                                         \
                do push(write_pipe_id, init);                                            \
                while ((init = fgetc(config_file)) != '\n' && init != EOF);              \
                                                                                         \
                stream pipe_name = { .read_id  = read_pipe_id,                           \
                                     .write_id = write_pipe_id,                          \
                                     .write    = open_pipe(write_pipe_id),               \
                                     .read     = open_pipe(read_pipe_id) };                  \
                                                                                         \
                demon({ code });                                                         \
            }                                                                            \
        }                                                                                \
    })

#define send_array(fd, arr)  betterarraysend(fd.write, arr)
#define read_array(unit, fd) ((unit *) betterarrayread(fd))

#define pcat(a, b) a##b
#define cat(a, b)  pcat(a, b)

#define repeat(x, ...)                                                                   \
    for (var cat(_, __VA_ARGS__) = 0; cat(_, __VA_ARGS__) < (x); cat(_, __VA_ARGS__)++)

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

const byte yes = 1;
const byte no  = 0;

void __print(string input, byte has_color, byte new_line) {
    var len = strlen(input);

    var skip = no;
    for (u4 i = 0; i < len; i++) {
        if (input[ i ] == 1) skip = yes;
        if (input[ i ] == 2) skip = no;

        if (has_color || !skip) printf("%c", input[ i ]);
    }

    if (new_line) printf("\n");
}

void __fprint(string input, byte has_color, byte new_line) {
    __print(input, has_color, new_line);
    free(input);
}

#define print(x)    __print(x, has_color, yes)
#define fprint(...) __fprint(format(__VA_ARGS__), has_color, yes)

string readline() {
    var  input = newarray(char);
    char __init;

    while ((__init = getchar()) == EOF);

    if (__init <= 10 || __init == '\n') return input;
    push(input, __init);

    while (((__init = getchar()) != EOF) && __init != '\n' && __init > 10) {
        push(input, __init);
    }

    input[ count(input) ] = 0;
    return input;
}

#define equal(x, y) (strcmp(x, y) == 0)

void inplace_tolower(string input) {
    repeat(count(input), i) {
        var cur = get(char, input, _i);
        if (cur >= 'A' && cur <= 'Z') set(char, input, _i, cur - 'A' + 'a');
    }
}

void __sleep(float seconds) {
    var microseconds = (int) (seconds * 1000000);

    usleep(microseconds);
}
#define sleep(x) __sleep(x)

#define EXPAND(x, ...) x __VA_ARGS__ __VA_OPT__("")
#define EXPAND1(x, ...) x EXPAND(__VA_ARGS__)
#define EXPAND2(x, ...) x EXPAND1(__VA_ARGS__)
#define EXPAND3(x, ...) x EXPAND2(__VA_ARGS__)
#define EXPAND4(x, ...) x EXPAND3(__VA_ARGS__)
#define EXPAND5(x, ...) x EXPAND4(__VA_ARGS__)
#define EXPAND6(x, ...) x EXPAND5(__VA_ARGS__)
#define EXPAND7(x, ...) x EXPAND6(__VA_ARGS__)
#define EXPAND8(x, ...) x EXPAND7(__VA_ARGS__)
#define EXPAND9(x, ...) x EXPAND8(__VA_ARGS__)
#define EXPAND10(x, ...) x EXPAND9(__VA_ARGS__)
#define EXPAND11(x, ...) x EXPAND10(__VA_ARGS__)
#define EXPAND12(x, ...) x EXPAND11(__VA_ARGS__)
#define EXPAND13(x, ...) x EXPAND12(__VA_ARGS__)
#define EXPAND14(x, ...) x EXPAND13(__VA_ARGS__)
#define EXPAND15(x, ...) x EXPAND14(__VA_ARGS__)
#define EXPAND16(x, ...) x EXPAND15(__VA_ARGS__)
#define EXPAND17(x, ...) x EXPAND16(__VA_ARGS__)
#define EXPAND18(x, ...) x EXPAND17(__VA_ARGS__)
#define EXPAND19(x, ...) x EXPAND18(__VA_ARGS__)
#define EXPAND20(x, ...) x EXPAND19(__VA_ARGS__)
#define EXPAND21(x, ...) x EXPAND20(__VA_ARGS__)
#define EXPAND22(x, ...) x EXPAND21(__VA_ARGS__)
#define EXPAND23(x, ...) x EXPAND22(__VA_ARGS__)
#define EXPAND24(x, ...) x EXPAND23(__VA_ARGS__)
#define EXPAND25(x, ...) x EXPAND24(__VA_ARGS__)
#define EXPAND26(x, ...) x EXPAND25(__VA_ARGS__)
#define EXPAND27(x, ...) x EXPAND26(__VA_ARGS__)
#define EXPAND28(x, ...) x EXPAND27(__VA_ARGS__)
#define EXPAND29(x, ...) x EXPAND28(__VA_ARGS__)
#define EXPAND30(x, ...) x EXPAND29(__VA_ARGS__)
#define EXPAND31(x, ...) x EXPAND30(__VA_ARGS__)
#define EXPAND32(x, ...) x EXPAND31(__VA_ARGS__)

#define reduce(...) EXPAND32(__VA_ARGS__)