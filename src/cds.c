#include "comms.c"
#include "macros.c"
#include "color.c"
#include <unistd.h>

int main() {
    srand(time(0));

    // listen for new clients

    byte has_color = yes;

    create_station("/tmp/cds_pipes.txt", client, {
        var client_id  = random();

        printf("New client ID assigned as " BYEL "%li" RESET ".\n", client_id);

        send(byte, client, MSG_CONNECT_SUCCESS);
        send(long, client, client_id);

        while (1) {
            var msg_out = read(byte, client);

            if (msg_out == MSG_EXIT) {
                print("Client closed connection!");
                exit(0);
            }
        }
    });
}