#include "color.c"
#include "comms.c"
#include "macros.c"
#include <stdio.h>
#include <stdlib.h>

int main() {
    srand(time(0));

    byte has_color = yes;

    print("Starting Astreal shell...");
    var cds_stream = stream();

    print("Attempting to communicate with Astreal CDS...");
    stream_to_station(cds_stream, "/tmp/cds_pipes.txt");

    print("Waiting for a connection signal from CDS...");
    var cds_response = read(byte, cds_stream);

    if (cds_response != MSG_CONNECT_SUCCESS) {
        printf("Failed to connect to CDS server: Invalid response. %i\n", cds_response);
        exit(1);
    }

    var cds_id = read(long, cds_stream);

    print("Connection succeeded! Welcome to Astreal.");
    printf("\x1b"
           "c");

    fprint(format(
        "Welcome to " BRED "Astreal" RESET ".\n"
        "Your client ID is: " YEL "%li" RESET "\n"
        "  -- Type " CYN "HELP" RESET " for a guide.\n"
        "  -- Type " CYN "QUIT" RESET " to exit at any time.\n"
        "  -- Type " GRN "COLOR" RESET " to toggle color display.\n",
        cds_id));

    while (1) {

    }
}

