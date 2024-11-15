#include "comms.c"
#include "macros.c"
#include <stdio.h>
#include <unistd.h>

#define FILENAME "/tmp/cds_pipes.txt"
#define DONT_TOUCH_MESSAGE                                                               \
    "!DO NOT TOUCH THIS FILE!\n"                                                         \
    "This file is a designated info-sharing log for the Astreal clients to connect to "  \
    "the server.\n"                                                                      \
    "Tampering with this file may crash the Astreal CDS server.\n\n"
#define DONT_TOUCH_MESSAGE_LENGTH 179

int file_size(FILE *fp) {
    int last_pos = ftell(fp);
    fseek(fp, 0, SEEK_END);
    int end_pos = ftell(fp);

    fseek(fp, last_pos, SEEK_SET);

    return end_pos - last_pos;
}

typedef char *string;

int main() {
    srand(0);

    // listen for new clients

    // wipe file
    var config_file = fopen(FILENAME, "w");
    fprintf(config_file, DONT_TOUCH_MESSAGE);
    fclose(config_file);

    config_file = fopen(FILENAME, "r");
    fseek(config_file, 0, SEEK_END);

    while (1) {
        int size = file_size(config_file);
        if (size > 0) { // new client!
            string shell_pipe_id = newarray(char);
            char   init          = fgetc(config_file);

            do push(shell_pipe_id, init);
            while ((init = fgetc(config_file)) != '\n' && init != EOF);

            printf("new client! %s\n", shell_pipe_id);

            demon({
                var shell_pipe = open_pipe(shell_pipe_id);

                send(byte, shell_pipe, 24);
                printf("wrote to client.\n");

                while (1) {
                    if (read(byte, shell_pipe) == MSG_EXIT) {
                        printf("Client closed connection!\n");
                        exit(0);
                    }
                }
            });
        }
    }
}
