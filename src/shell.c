#include "comms.c"
#include "macros.c"
#include <stdio.h>
#include <stdlib.h>

int main() {
    srand(0);

    printf("Starting Astreal shell...\n");
    auto cds_pipe_id = pipe();

    printf("Attempting to communicate with Astreal CDS... %s\n", cds_pipe_id);
    auto config_file = fopen("/tmp/cds_pipes.txt", "a");
    fprintf(config_file, "%s\n", cds_pipe_id);
    fclose(config_file);

    auto cds_pipe = open_pipe(cds_pipe_id);

    printf("Waiting for a connection signal from CDS...\n");
    auto cds_response = read(byte, cds_pipe);

    printf("%i\n", cds_response);

    send(byte, cds_pipe, MSG_EXIT);
}