#include "headers/common.h"
#include <time.h>

int main() {
    $seed();

    // listen for new clients

    bool has_color = yes;

    printf("\x1b"
           "c");
    fflush(stdout);

    create_pond("/tmp/cds_pipes.txt", client, {
        var client_id = $rng();
        printf("%i\n", rng_seed);

        printf("New client ID assigned as " cBYEL("%li") ".\n" RESET, client_id);

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
                    cBYEL("[LOG]") " Client " GRN "#%li" RESET ": " cWHT("%s"),
                    client_id,
                    input_log);
                free(input_log);
            } else if (msg_out == MSG_CHANGE_SID) {
                var new_id = read(long, client);
                if (new_id != client_id) {
                    printf(
                        YEL "Client " BYEL "%li" YEL
                            "'s ID changed to " cBYEL("%li") ".\n",
                        client_id,
                        new_id);
                }
                client_id = new_id;
            }
        }
    });
}