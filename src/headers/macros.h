#pragma once

// Ooh. Massive.
#include "color.h"
#include <dlfcn.h>
#include <execinfo.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

// void pointer my beloved!
typedef void *ptr;
// i prefer byte count instead of bit count in uX names.
typedef unsigned long u8;
// i prefer byte count instead of bit count in uX names.
typedef unsigned int u4;
// byte? in c?
typedef unsigned char byte;
// bool? in c?
typedef byte bool;
#define auto /* auto? in c? */ __auto_type
#define var  /* var? in c? */ __auto_type

// 243 is static data
// 247 is array

#define cast_index(arr, type, index)                                                     \
    /* This is an index-type-safety macro for my sanity. */ ((type *) arr)[ index ]
#define cast_ptr(arr, type, index)                                                       \
    /* This is an index-type-safety macro for my sanity. */ (                            \
        &cast_index(arr, type, index))

// libc malloc, for the vanilla folk.
ptr std_malloc(u4 size) { return malloc(size); }

#define expand(x) /* The use case of this macro is left as an exercise to the reader. */ x
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
#define demon(code)    /* Fork the current process and creates an async process that runs  \
                          in parallel, it also returns the PID of the child process to the \
                          parent, in case you need to wait until the child dies. */        \
    __demon(code, expand(__COUNTER__))

#define pcat(a, b) a##b
#define cat(a, b)  /* Meow? */ pcat(a, b)

struct leak_db {
    byte   leaked;
    ptr    memory;
    ptr    backtrace[ 32 ];
    char **symbols;
};

var leak_db      = (struct leak_db[ 4096 ]) {};
var leak_ptr     = 0;
var top_of_stack = NULL;

bool mine(ptr pointer) {
    bool *glob_var = mmap(
        NULL, sizeof(bool), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    var pid = demon({
        *glob_var  = 0;
        byte *x    = pointer;
        byte  data = *x;
        *glob_var  = 1;
    });

    wait(NULL);
    bool output = *glob_var;

    munmap(glob_var, sizeof(bool));

    return output;
}

ptr betterarray(u4 unit, u4 _count);

void final_free(ptr x) {
    // printf("Freeing memory " cHRED("%p") "\n", x);

    for (int i = 0; i < leak_ptr; i++) {
        if (leak_db[ i ].memory == x) leak_db[ i ].leaked = 0;
    }

    free(x);
}

// Better than libc free since 2024.
void betterfree(ptr x) {
    if (x == 0) return;

    var heap_check    = malloc(1);
    var __stack_check = 2;
    var stack_check   = (ptr) &__stack_check;
    var data_check    = (ptr) "";

    // This is a cheap hack. It's in no way reliable.
    var heap_dist = heap_check - x;
    if (heap_dist < 0) heap_dist = -heap_dist;
    var data_dist = data_check - x;
    if (data_dist < 0) data_dist = -data_dist;
    var stack_dist = stack_check - x;
    if (stack_dist < 0) stack_dist = -stack_dist;

    var is_heap = heap_dist < data_dist && heap_dist < stack_dist;

    if (!is_heap) {
        free(heap_check);
        return;
    }

    ptr pre_x = cast_ptr(x, u4, -2);
    if (cast_index(pre_x, byte, -1) == 247) { // array
        final_free(cast_ptr(pre_x, byte, -1));
    } else if (cast_index(x, byte, -1) == 243) {
        final_free(cast_ptr(x, byte, -1));
    } else {
        final_free(x);
    }

    free(heap_check);
}

#define SEC_TO_US(sec) ((sec) * 1000000)
#define NS_TO_US(ns)   ((ns) / 1000)
uint64_t micros() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    uint64_t us = SEC_TO_US((uint64_t) ts.tv_sec) + NS_TO_US((uint64_t) ts.tv_nsec);
    return us;
}

void check_for_leaks(bool silent) {
    var start_time = micros();

    var _bottom = 25;
    var bottom  = (ptr) &_bottom;

    var number_freed = 0;

    var start = bottom > top_of_stack ? top_of_stack : bottom;
    var end   = bottom > top_of_stack ? bottom : top_of_stack;

    var accessible = (bool *) malloc(sizeof(bool) * (leak_ptr + 1));

    for (long i = (long) start; i < (long) end; i++) {
        var _cur = (ptr) i;

        if (mine(_cur)) {
            var cur = *(ptr *) _cur;

            for (int j = 0; j < leak_ptr; j++) {
                var diff = leak_db[ j ].memory - cur;
                if (diff < 0) diff = -diff;

                // printf("%p %p %li\n", cur, leak_db[ j ].memory, diff);

                if (leak_db[ j ].leaked) {
                    if (diff == (sizeof(u4) * 2 + sizeof(byte)) || diff == 0
                        || diff == sizeof(byte))
                        accessible[ j ] = 1;
                    else
                        accessible[ j ] = 0;
                }
            }
        }
    }

    var accessible_tally = 0;
    for (long i = 0; i <= leak_ptr; i++) {
        if (accessible[ i ] != 0) accessible_tally++;
    }

    for (int i = 0; i <= leak_ptr; i++) number_freed += 1 - leak_db[ i ].leaked;

    if (!silent) {
        printf(
            "-- Memory leak summary:\nTop of stack: %p\nBottom of stack: %p\nStack size: "
            "%li\nTotal "
            "allocations: "
            "%i\nFreed: %i\nAccessible: %i\nLeaked: %i\n\n",
            top_of_stack,
            bottom,
            end - start,
            leak_ptr + 1,
            number_freed,
            accessible_tally,
            leak_ptr + 1 - number_freed - accessible_tally);
    }

    for (int i = 0; i < leak_ptr; i++) {
        if (leak_db[ i ].leaked && mine(leak_db[ i ].memory)) {
            var is_accessible = 0;

            for (long j = (long) start; j < (long) end; j++) {
                if ((ptr) j == leak_db[ i ].memory) {
                    is_accessible = 1;
                    break;
                }
            }

            if (!is_accessible) { betterfree(leak_db[ i ].memory); }
        }
    }

    if (!silent) {
        for (int i = 0; i < leak_ptr; i++) {
            if (leak_db[ i ].leaked && mine(leak_db[ i ].memory)) {
                var is_accessible = 0;

                for (long j = (long) start; j < (long) end; j++) {
                    if ((ptr) j == leak_db[ i ].memory) {
                        is_accessible = 1;
                        break;
                    }
                }

                if (!is_accessible) {
                    printf("Leaked memory: %p\n", leak_db[ i ].memory);

                    var strings = leak_db[ i ].symbols;

                    printf("Backtrace:\n");

                    if (strings != 0) {
                        for (int x = 0; x < 32; x++) {
                            if (strcmp("[(nil)]", strings[ x ]) != 0) {
                                printf(" - %s\n", strings[ x ]);
                            }
                        }
                    }

                    printf("\n");
                    // free(strings);
                }
            }
        }
    }

    free(accessible);
    var end_time = micros();

    if (!silent) { printf("Time elapsed: %lius\n", end_time - start_time); }
}

// Better than libc malloc since 2024.
ptr bettermalloc(u4 size) {
    ptr result;
    while ((result = malloc(size + sizeof(byte))) == 0);
    cast_index(result, byte, 0) = 243;

    var bt_array = malloc(sizeof(ptr) * 32);
    backtrace(bt_array, 32);
    leak_db[ leak_ptr++ ]
        = (struct leak_db) { .leaked    = 1,
                             .memory    = result,
                             .backtrace = bt_array,
                             .symbols   = backtrace_symbols(bt_array, 32) };

    return &result[ 1 ];
}

// Better than libc realloc since 2024.
ptr betterrealloc(ptr original, u4 size) {
    byte *result;
    byte  main_flag = cast_index(original, byte, -1);
    while ((result = realloc(cast_ptr(original, byte, -1), size + sizeof(byte))) == 0);
    result[ 0 ] = main_flag;

    for (int i = 0; i < leak_ptr; i++) {
        if (leak_db[ i ].memory == original) leak_db[ i ].leaked = 0;
    }

    var bt_array = malloc(sizeof(ptr) * 32);
    backtrace(bt_array, 32);
    leak_db[ leak_ptr++ ]
        = (struct leak_db) { .leaked    = 1,
                             .memory    = result,
                             .backtrace = bt_array,
                             .symbols   = backtrace_symbols(bt_array, 32) };

    return &result[ 1 ];
}

#define malloc(x)     /* Better than libc malloc since 2024. */ bettermalloc(x)
#define realloc(x, s) /* Better than libc realloc since 2024. */ betterrealloc(x, s)

// libc free, for the vanilla folk.
// void std_free(ptr x) { free(x); }

#define free(x)   /* Better than libc free since 2024. */ betterfree((ptr) x)
#define new(type) /* Allocates some empty space for you! */                              \
    ({                                                                                   \
        byte *__ptr__ = malloc(sizeof(type));                                            \
        __ptr__[ 0 ]  = 0;                                                               \
        (type *) __ptr__;                                                                \
    })

// Creates an array. UNIT is the element size and _COUNT is the element count.
ptr betterarray(u4 unit, u4 _count) {
    u4  count                     = _count + 1;
    ptr pointer                   = malloc(unit * count + 2 * sizeof(u4));
    cast_index(pointer, byte, -1) = 247;
    cast_index(pointer, u4, 0)    = unit;
    cast_index(pointer, u4, 1)    = count;

    return &cast_index(pointer, u4, 2);
}

// 'Eat me' cake.
ptr grow(ptr _array, u4 inflation) {
    ptr array = cast_ptr(_array, u4, -2);
    u4  unit  = cast_index(array, u4, 0);
    u4  count = cast_index(array, u4, 1);

    u4 new_size = unit * (count + inflation) + 2 * sizeof(u4);

    ptr arr2                = realloc(array, new_size);
    cast_index(arr2, u4, 1) = count + inflation;

    return &arr2[ 2 ];
}

#define shrink(array, deflation) /* 'Drink me' potion. */ grow(array, -(deflation))

#define get(type, array, index) /* Safety function with bounds checking. */              \
    (type)({                                                                             \
        if (index >= count(array)) {                                                     \
            printf("ERROR: Index %i out of bounds (%i).\n", index, count(array));        \
            exit(1);                                                                     \
        }                                                                                \
        array[ index ];                                                                  \
    })
#define set(type, array, index, value) /* Safety function with bounds-checking. */       \
    ({                                                                                   \
        if (index >= count(array)) {                                                     \
            printf("ERROR: Index %i out of bounds (%i).\n", index, count(array));        \
            exit(1);                                                                     \
        }                                                                                \
        cast_index(array, type, index) = value;                                          \
    })
#define newarray(unit) /* Creates a new empty dynamic array. */                          \
    cast_ptr(betterarray(sizeof(unit), 0), unit, 0)
#define array(unit, count)                                                               \
    /* Creates a new dynamic array of type UNIT with COUNT elements. */ cast_ptr(        \
        betterarray(sizeof(unit), count), unit, 0)
#define count(arr) /* Counts the number of elements in an array. */                      \
    (cast_index(arr, u4, -1) - 1)
#define unit_size(arr)  cast_index(arr, u4, -2)
#define last(arr)       /* Gets the last element of an array, */ arr[ count(arr) - 1 ]
#define push(arr, item) /* Pushes a value into an array. */                              \
    ({                                                                                   \
        grow(arr, 1);                                                                    \
        last(arr) = item;                                                                \
    })
#define pop(arr) /* Pops a value from an array and returns its value. */                 \
    ({                                                                                   \
        var output = last(arr);                                                          \
        shrink(arr, 1);                                                                  \
        output;                                                                          \
    })
#define __copy_string(text, size)                                                        \
    ({                                                                                   \
        var __Data = array(char, size);                                                  \
        strcpy(__Data, text);                                                            \
        __Data;                                                                          \
    })
#define copy_string(text) /* Copies a string into a separate heap memory slot. */        \
    __copy_string(text, strlen(text))
// #define frame()                                                                          \
//     var top_of_stack  = 25;                                                              \
//     var current_frame = newarray(ptr)

// void __unframe(ptr __top_of_stack, ptr *heap_frame) {
//     var __bottom_of_stack = 25;
//     var bottom_of_stack   = (ptr) &__bottom_of_stack;

//     var top_of_stack = __top_of_stack;

//     if (bottom_of_stack > top_of_stack) {
//         var inter       = bottom_of_stack;
//         bottom_of_stack = top_of_stack;
//         top_of_stack    = inter;
//     }

//     for (var j = 0; j < count(heap_frame); j++) {
//         if (heap_frame[ j ] == 0) continue;

//         var should_delete = 1;

//         for (var i = (long) bottom_of_stack; i < (long) top_of_stack; i++) {
//             if((ptr)(bottom_of_stack + i) == heap_frame[j]) {
//                 should_delete = 0;
//                 break;
//             }
//         }

//         if(should_delete) {
//             printf("Freed memory %p\n", heap_frame[j]);
//             free(heap_frame[j]);
//         }
//     }

//     free(heap_frame);
// }

// #define unframe() __unframe(&top_of_stack, current_frame)

// String ...? ...!
typedef char *string;

#define format(fmts, ...) /* Very handy! */                                              \
    ({                                                                                   \
        string buf = NULL;                                                               \
        while (asprintf(&buf, fmts __VA_OPT__(, ) __VA_ARGS__) < 0);                     \
        string output = copy_string(buf);                                                \
        free(buf);                                                                       \
        output;                                                                          \
    })

typedef long pipeout;

// Creates a pipe for you. Water is not included. Batteries also not included.
string betterpipe() {
    int    pipe_name = rand();
    string real_name = format("/tmp/FIFO_PIPE_%i", pipe_name);

    mkfifo(real_name, 0666);

    return real_name;
}

// Opens a water pipe to let the river stream flow!
pipeout open_pipe(string pipe_id) { return open(pipe_id, O_RDWR); }

typedef struct {
    string  read_id;
    string  write_id;
    pipeout read;
    pipeout write;
} stream;

// Creates a river stream for you.
stream create_stream() {
    string  read_pipe  = betterpipe();
    string  write_pipe = betterpipe();
    pipeout read_fd    = open_pipe(read_pipe);
    pipeout write_fd   = open_pipe(write_pipe);

    return (stream) {
        .read_id = read_pipe, .write_id = write_pipe, .read = read_fd, .write = write_fd
    };
}

// Connects your river stream to the central pond.
void stream_to_pond(stream input, string pond) {
    var config_file = fopen(pond, "a");
    fprintf(config_file, "%s\n%s\n", input.write_id, input.read_id);
    fclose(config_file);
}

#define stream() /* Just makes a stream. */ create_stream()

#define stream_and_send(filename) /* Creates and sends a stream for you. Handy! */       \
    ({                                                                                   \
        var __Output = stream();                                                         \
        stream_to_pond(__Output, filename);                                              \
        __Output;                                                                        \
    })

// Releases wild and feral data into the deadly stream.
void bettersend(pipeout fd, ptr data, u4 size) {
    byte mode = 0; // mode 0 is static data and mode 1 is an array

    write(fd, &mode, sizeof(byte));
    write(fd, &size, sizeof(u4)); // send size data through pipe
    write(fd, data, size);        // send actual data
}

// Reads something from the stream.
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

#define pipe()               /* Creates a stream pipe for you to use. */ betterpipe()
#define send(unit, fd, data) /* Sends DATA of type UNIT to a stream FD. */               \
    ({                                                                                   \
        unit __Data = data;                                                              \
        bettersend(fd.write, &__Data, sizeof(__Data));                                   \
    })
void __read(int fd, void *buf, size_t size) { read(fd, buf, size); }
#define read(                                                                            \
    unit,                                                                                \
    fd) /* Reads a variable with the type UNIT from the stream FD. No memory leaks. */   \
    ({                                                                                   \
        unit *__Data = betterread(fd.read);                                              \
        unit  _Data  = *__Data;                                                          \
        free(__Data);                                                                    \
        _Data;                                                                           \
    })

#define repeat(x, ...) /* Handy macro for repeating something. */                        \
    for (var cat(_, __VA_ARGS__) = 0; cat(_, __VA_ARGS__) < (x); cat(_, __VA_ARGS__)++)

// Sends an array through a stream.
// Does not work with nested pointers.
void betterarraysend(pipeout fd, ptr array) {
    u4 amount = count(array);
    u4 unit   = unit_size(array);

    bettersend(fd, &unit, sizeof(u4));
    bettersend(fd, &amount, sizeof(u4));

    repeat((amount * unit)) {
        // bettersend(fd, &send_array[i], sizeof(byte)); // inefficient. weak.
        write(fd, cast_ptr(array, byte, _), sizeof(byte)); // efficient. strong.
    }
}

// Reads an array thrown into a stream.
ptr betterarrayread(stream fd) {
    u4 unit   = read(u4, fd);
    u4 amount = read(u4, fd);
    u8 size   = unit * amount;

    ptr arr = betterarray(unit, amount);

    repeat(size) { __read(fd.read, cast_ptr(arr, byte, _), 1); }

    return arr;
}

// Just don't touch this. Thank you!
#define DONT_TOUCH_MESSAGE                                                               \
    "!DO NOT TOUCH THIS FILE!\n"                                                         \
    "This file is a communication stream, altering its contents will cause programs to " \
    "crash.\n\n"
#define DONT_TOUCH_MESSAGE_LENGTH 116

// Gets the file size from a file pointer.
int file_size(FILE *fp) {
    int last_pos = ftell(fp);
    fseek(fp, 0, SEEK_END);
    int end_pos = ftell(fp);

    fseek(fp, last_pos, SEEK_SET);

    return end_pos - last_pos;
}

#define watch(pid) /* Waits for your child to die. */ waitpid(pid, NULL, 0)

#define create_pond(filename, pipe_name, code)                                           \
    /* Huge macro. Creates a pond where the river streams will point to. Once a stream   \
     * connects with the pond, the pond forks itself and runs the code given to          \
     * it as a child process. */                                                         \
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

#define send_array(fd, arr) /* Drops an array into the flow of a given stream.*/         \
    betterarraysend(fd.write, arr)
#define read_array(                                                                      \
    unit, fd) /* Reads an array from a stream with the given element type. */            \
    ((unit *) betterarrayread(fd))

// yes = true = 1
const byte yes = 1;
// no = false = 0
const byte no = 0;
// null = ((ptr)0)
const ptr null = 0;

// Raw, controllable version of the print macro.
void __print(string input, byte has_color, byte new_line) {
    var len = strlen(input);

    var skip = no;
    repeat(len) {
        var cur = input[ _ ];

        if (cur == 1) skip = yes;
        if (cur == 2) skip = no;

        if (has_color || !skip) printf("%c", cur);
    }

    if (new_line) printf("\n");
}

// The raw, not pretty version of the sleep macro. The time in seconds can be a float with
// any precision.
void __sleep(float seconds) {
    var microseconds = (int) (seconds * 1000000);

    usleep(microseconds);
}
#define sleep(x)                                                                         \
    /* Hangs the process for the given amount of seconds. Can be a float. */             \
    __sleep(x)

// Raw, controllable version of the type macro. The time argument specifies time between
// each keystroke in seconds.
void __type(string input, byte has_color, byte new_line, float time) {
    var skip = no;

    repeat(strlen(input)) {
        var cur = input[ _ ];
        if (cur == 1) skip = yes;
        if (cur == 2) skip = no;

        if (has_color || !skip) {
            if (cur == '\5') {
                sleep(time * 3);
                continue;
            }
            printf("%c", cur);
            fflush(stdout);

            if (cur == '\n') sleep(time * 2);
            else { sleep(time); }
        }
    }

    if (new_line) printf("\n");

    free(input);
}

// Raw, controllable version of the fprint macro.
void __fprint(string input, byte has_color, byte new_line) {
    __print(input, has_color, new_line);
    free(input);
}

// #define print(text)    /* Prints text to the console, with a new line, and omits color if \
//                           has_color = no. */                                              \
//     __print(text, has_color, yes)
#define print(...)     /* Prints text to the console using a printf formatting, otherwise \
                           same as print(text). */                                        \
    __fprint(format(__VA_ARGS__), has_color, yes)
#define type(...)     /* Has a typewriter effect and utilizes default_time, otherwise the \
                          same as print(text). */                                         \
    __type(format(__VA_ARGS__), has_color, yes, default_time)

// Reads a line from stdin. Obviously allocates memory, so free it afterwards.
string readline() {
    var  input = newarray(char);
    char __init;

    while ((__init = getchar()) == EOF);

    if (__init <= 10 || __init == '\n') return input;
    push(input, __init);

    while (((__init = getchar()) != EOF) && __init != '\n' && __init > 10) {
        printf("%p %i\n", input, count(input));
        push(input, __init);
    }

    input[ count(input) ] = 0;
    return input;
}

#define equal(x, y) /* Checks the equality of two strings. */ (strcmp(x, y) == 0)

// Turns a STRING into lowercase by altering the original string.
void inplace_tolower(string input) {
    repeat(count(input), i) {
        var cur = get(char, input, _i);
        if (cur >= 'A' && cur <= 'Z') set(char, input, _i, cur - 'A' + 'a');
    }
}

// Turns a string into UPPERCASE by altering the original string.
void inplace_toupper(string input) {
    repeat(count(input), i) {
        var cur = get(char, input, _i);
        if (cur >= 'a' && cur <= 'z') set(char, input, _i, cur - 'a' + 'A');
    }
}

#define EXPAND(x, ...)   x __VA_ARGS__ __VA_OPT__("")
#define EXPAND1(x, ...)  x EXPAND(__VA_ARGS__)
#define EXPAND2(x, ...)  x EXPAND1(__VA_ARGS__)
#define EXPAND3(x, ...)  x EXPAND2(__VA_ARGS__)
#define EXPAND4(x, ...)  x EXPAND3(__VA_ARGS__)
#define EXPAND5(x, ...)  x EXPAND4(__VA_ARGS__)
#define EXPAND6(x, ...)  x EXPAND5(__VA_ARGS__)
#define EXPAND7(x, ...)  x EXPAND6(__VA_ARGS__)
#define EXPAND8(x, ...)  x EXPAND7(__VA_ARGS__)
#define EXPAND9(x, ...)  x EXPAND8(__VA_ARGS__)
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

ptr get_me() {
    ptr me;
    while ((me = dlopen(0, RTLD_LAZY)) == 0);

    return me;
}

ptr get_dylib(string path) {
    ptr dylib;
    while ((dylib = dlopen(path, RTLD_LAZY)) == 0);

    return dylib;
}

// returns a pointer to a function
ptr dynamic(ptr me, string name) {
    var result = dlsym(me, name);

    return result;
}

#define stage(name)                                                                      \
    void cat(__stage_, name)(game_data * user, bool has_color);                          \
    void cat(stage_, name)(game_data * user) {                                           \
        if (top_of_stack == NULL) top_of_stack = user;                                   \
        cat(__stage_, name)(user, user->has_color);                                      \
    }                                                                                    \
    void cat(__stage_, name)(game_data * user, bool has_color)
void sample_stage(ptr user) {}
// function pointer? in c?
typedef typeof(sample_stage) *stage;

struct game_data;

void transfer(string name, struct game_data *data) {
    var stages   = get_dylib("build/stages.so");
    var new_name = format("stage_%s", name);
    var function = (stage) dynamic(stages, new_name);
    free(new_name);

    if (function == null) {
        bool has_color = yes;
        print(cBHRED("ERROR!") " unknown transfer target: " cBHYEL("%s") ".", name);
        exit(1);
    }

    function(data);

    dlclose(stages);
}

// TODO: command(count, ...) macro
//  Example:
/*

        _ number of commands
           _ whether to show possible commands or not. 0 = no, 1 = in help menu, 2 = in
text command(2, 2, { "get up", stage_main }, { "stay", stage_bed }
    );

 */

struct command_choice {
    string name;
    string description;
    stage  func_ptr;
};

#define cmd(name, func, ...) /* less keystrokes! */                                      \
    (struct command_choice) { name, format(__VA_ARGS__), (stage) func }

enum command_func_options {
    hide_commands    = 0,
    commands_in_help = 1,
    commands_in_text = 2
};

float default_time = 0.002f;

#define command(options, help_text, ...)                                                     \
    ({                                                                                       \
        struct command_choice _options[] = { __VA_ARGS__ };                                  \
        u4  choice_length = sizeof(_options) / sizeof(struct command_choice);                \
        var output        = __command(                                                       \
            options, help_text, has_color, default_time, choice_length, __VA_ARGS__); \
        repeat(choice_length) { free(_options[ _ ].description); }                           \
        output;                                                                              \
    })

/*

0: number of commands, 1: whether to show possible commands or not.
command(2, 2, { "get up", stage_main }, { "stay", stage_bed }
    );

 */
stage __command(
    enum command_func_options commands_option,
    string                    help_text,
    bool                      has_color,
    float                     default_time,
    u4                        length,
    ...) {
    va_list cmd_list;
    va_start(cmd_list, length);

    var cmds = array(struct command_choice, length + 1);
    var available_commands
        = copy_string(cBGRN("Available commands:\n") cHYEL("[") "HELP" cHYEL("]"));

    repeat(length) {
        var cur = &cmds[ _ ];

        *cur      = va_arg(cmd_list, struct command_choice);
        cur->name = copy_string(cur->name);
        inplace_toupper(cur->name);

        var old_coms = available_commands;
        available_commands
            = format("%s\n" cHYEL("[") "%s" cHYEL("]"), available_commands, cur->name);
        free(old_coms);

        inplace_tolower(cur->name);
    }

    va_end(cmd_list);

    var choice = (ptr) -1;

    printf("\n");
    if (commands_option == commands_in_text) type("%s", available_commands);

    while (choice == (ptr) -1) {
        printf("\n> ");
        fflush(stdout);

        var input = readline();
        inplace_tolower(input);

        repeat(length) {
            var cur = cmds[ _ ];

            if (equal(input, cur.name)) {
                choice = cur.func_ptr;
                if (cur.description != null) { type("\n%s", cur.description); }
                break;
            }
        }

        if (choice != (ptr) -1) {
            free(input);
            break;
        }

        if (equal(input, "help")) {
            type(
                "%s",
                help_text == 0 || equal(help_text, "")
                    ? commands_option == hide_commands
                          ? "\nNo help text provided. You're on your own."
                          : "\nThere is no help text available right now."
                    : help_text);
            // type("\n");
            sleep(0.25);
            if (commands_option != hide_commands) { type("\n%s", available_commands); }
        } else if (equal(input, "memory leak check")) {
            check_for_leaks(no);
            printf("\n");
        }

        else {
            __print("I don't understand.", no, yes);
        }

        free(input);
    }

    free(cmds);
    free(available_commands);

    repeat(length) {
        var cur = &cmds[ _ ];

        if (cur->name != null) free(cur->name);
        free(cur->description);
    }

    demon(check_for_leaks(yes));

    return choice;
}

typedef struct game_data {
    float version;
    u4    time;
    byte  meow_times;
    bool  has_color;
    bool  passed_help;
    // bool  apartment_entered;
    u4   items[ 16 ];
    char location[ 128 ];
} game_data;

enum ITEMS { TORN_CLOTHES = 1 };

var item_names = (string[]) { "Torn clothes" };

void add_to_inventory(game_data *game, u4 item_index) {
    repeat(16) {
        var cur = game->items[ _ ];

        if (cur == 0) {
            cur = item_index;
            break;
        }
    }
}

// the MMAPed seed for the random number generator.
var *rng_seed = (u4 *) null;

#define $seed() /* seeds a child-safe random number generator. */                        \
    rng_seed = mmap(                                                                     \
        NULL, sizeof(u4), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0)

// taken off a github gist about mullbery32. the numbers are random enough for me, i don't
// need them for security or cryptography or anything.
int $rng() {
    var z = (*rng_seed += 0x9e3779b9);

    z ^= z >> 16;
    z *= 0x21f0aaad;
    z ^= z >> 15;
    z *= 0x735a2d97;
    z ^= z >> 15;

    return z;
}