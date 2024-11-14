#include "macros.c"
#include <stdio.h>
#include <unistd.h>

#define FILENAME "/tmp/cds_pipes.txt"
#define DONT_TOUCH_MESSAGE                                                               \
    "!DO NOT TOUCH THIS FILE!\n"                                                         \
    "This file is a designated info-sharing log for the Astreal clients to connect to "  \
    "the server.\n"                                                                      \
    "Tampering with this file may crash the Astreal CDS server.\n\n"

int file_size(FILE *fp) {
    int last_pos = ftell(fp);
    fseek(fp, 0, SEEK_END);
    int end_pos = ftell(fp);
    fseek(fp, 0, last_pos);

    return end_pos - last_pos;
}

typedef char* string;

int main() {
    // listen for new clients

    // wipe file
    auto config_file = fopen(FILENAME, "w");
    fprintf(config_file, DONT_TOUCH_MESSAGE);
    fclose(config_file);

    config_file = fopen(FILENAME, "r");
    fseek(config_file, 0, SEEK_END);

    int last_size = sizeof(DONT_TOUCH_MESSAGE);

    while (1) {
        int size = file_size(config_file);
        if (size > last_size) { // new client!
            last_size = size;
            printf("new client!\n");

            string pipe_id = array(char, 0);
            char init;
            while((init = fgetc(config_file)) != '\n') push(pipe_id, init);

            printf("%s\n", pipe_id);
        }
    }
}