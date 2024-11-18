#include "color.c"
#include "comms.c"
#include "macros.c"
#include <stdio.h>
#include <unistd.h>

int main() {
    srand(time(0));

    // listen for new clients

    byte has_color = yes;

    printf("\x1b"
           "c");
    fflush(stdout);

    create_station("/tmp/cds_pipes.txt", client, {
        var client_id = random();

        printf(BLK "New client ID assigned as " BYEL "%li" BLK ".\n" RESET, client_id);

        send(byte, client, MSG_CONNECT_SUCCESS);
        send(long, client, client_id);

        while (1) {
            var msg_out = read(byte, client);

            if (msg_out == MSG_EXIT) {
                print("Client closed connection!");
                exit(0);
            } else if (msg_out == MSG_LOG) {
                var input_log = read_array(char, client);

                printf(
                    cBBLK("[LOG]") BLK " Client " GRN "#%li" BLK ": " cWHT("%s"),
                    client_id,
                    input_log);
                free(input_log);
            } else if (msg_out == MSG_CHANGE_SID) {
                var new_id = read(long, client);
                if (new_id != client_id) {
                    printf(
                        BLK "Client " BYEL "%li" BLK
                            "'s ID changed to " cBYEL("%li") ".\n",
                        client_id,
                        new_id);
                }
                client_id = new_id;
            }
        }
    });
}