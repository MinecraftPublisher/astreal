#include "color.c"
#include "comms.c"
#include "macros.c"
#include <stdio.h>
#include <stdlib.h>

byte has_color = yes;

const var current_version = 0.11f;

struct game_data {
    float version;
    byte  meow_times;
    byte  has_color;
    byte passed_help;
    char  location[ 128 ];
};

struct game_data new_game() {
    return (struct game_data) {
        .version = current_version, .meow_times = 0, .has_color = 0, .location = { 0 }
    };
}

void save_data(u8 client_id, struct game_data data) {
    var file_name = format("astreal/%li.bin", client_id);
    var fd        = fopen(file_name, "wb");
    free(file_name);

    fwrite(&data, sizeof(struct game_data), 1, fd);
    fclose(fd);
}

struct game_data load_data(u8 client_id) {
    var data = new (struct game_data);

    var file_name = format("astreal/%li.bin", client_id);
    var fd        = fopen(file_name, "rb");
    free(file_name);

    fread(data, sizeof(struct game_data), 1, fd);

    var _data = *data;
    free(data);
    return _data;
}

bool exists(string fname) {
    FILE *file;
    if ((file = fopen(fname, "r"))) {
        fclose(file);
        return 1;
    }

    return 0;
}

int main() {
    print("Starting Astreal shell...");
    srand(time(0));

    print("Attempting to communicate with Astreal CDS...");
    var cds_stream = stream_and_send("/tmp/cds_pipes.txt");

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

    var game = new_game();

    while (1) {
        var session_id = "";
        __print(
            cBLK("Enter a session ID if you have one, or just press enter: "),
            has_color,
            no);
        fflush(stdout);

        session_id = readline();
        usleep(250000);

        if (count(session_id) == 0) {
            free(session_id);
            printf("\nUsing assigned session ID.\n");
            break;
        }

        // relative path exploits are maybe possible here...
        // ...but is that really a concern?
        var session_path = format("astreal/%s.bin", session_id);

        if (exists(session_path)) {
            var sid = atoll(session_id);
            game    = load_data(sid);
            cds_id  = sid;
            fprint("Loading game data from session ID " cBYEL("%lli") ".\n", sid);
            free(session_id);
            free(session_path);
            break;
        }

        print(cRED("Invalid session ID!"));
        free(session_id);
        free(session_path);
    }

    send(byte, cds_stream, MSG_CHANGE_SID);
    send(long, cds_stream, cds_id);

    fprint(
        reduce(
            "Welcome to " cBRED("Astreal") ".\n",
            "Your client ID is: " cYEL("%li") "\n",
            "  -- Type " cCYN("HELP") " for a guide.\n",
            "  -- Type " cCYN("QUIT") " to exit at any time.\n",
            "  -- Type " cGRN("COLOR") " to toggle color display.\n"),
        cds_id);

    while (1) {
        save_data(cds_id, game);

        var location = "main";

        __fprint(
            format(MAG "%li " BLK "%s " BGRN "$ " RESET, cds_id, location),
            has_color,
            no);
        fflush(stdout);
        var input = readline();
        var i     = 0;

        var original_command = newarray(char);

        char cur;
        while (i < count(input) && (cur = get(char, input, i)) != ' ') {
            push(original_command, cur);
            i++;
        }

        var command = copy_string(original_command);
        inplace_tolower(command);

        if (equal(command, "meow")) {
            if (game.meow_times == 0) {
                print("Hello there! Didn't know cats could play this game.");
            } else if (game.meow_times == 1) {
                print("Meow to you too.");
            } else if (game.meow_times == 2) {
                print("Quite a noisy cat, huh?");
            } else if (game.meow_times == 3) {
                print("Well, to be honest, I didn't expect you to know how to do "
                      "anything else. You're a cat.");
            } else if (game.meow_times >= 4) {
                print("You should at least try to play the game, don't you think?");
            }

            if (game.meow_times < 4) game.meow_times++;
        } else if (equal(command, "clear")) {
            print("\1\x1b"
                  "c\2");
        } else if (equal(command, "color")) {
            has_color      = !has_color;
            game.has_color = has_color;

            if (has_color) print("Color is now on.");
            else
                print("Color is now off.");
        } else if (equal(command, "help")) {
            sleep(0.4);
            print("This is " cGRN("Astreal") ".");
            sleep(1);
            print("An interactive game, based on communication "
                  "through SSH only.");
            sleep(1.4);
            print("The game will take place in Atrola, a modern-era "
                  "country.");
            sleep(2);
            print("\nOnce you see an object, you can know more about "
                  "it by " cCYNB("TOUCH") "ing it.");
            sleep(1);
            print("For example: " cBGRN(
                "touch table") " will give you information about a "
                               "table in your current surroundings.");
            sleep(2.5);
            print(reduce(
                "\nYou can move around using ",
                cCYNB("up (u)") " ",
                cCYNB("down (d)") "\n",
                cCYNB("north (n)") " ",
                cCYNB("south (s)") " ",
                cCYNB("east (e)") " and ",
                cCYNB("west (w)") "."));

            sleep(5);
            print("\nYou can look around using the " cGRNB("look") " command,");
            sleep(2);
            print(reduce(
                "Or you can specify details such as ",
                cBRED("look <object>") "\nor ",
                cBRED("look <north/east/west/south>") "."));
            sleep(2);
            print("Use the help command in different places when you're clueless on what to do.");
            sleep(2);
            print("Use the " cBYEL("begin") " command to start your journey!");
            sleep(1);
            game.passed_help = yes;
        }

        else if(equal(command, "begin") && game.passed_help) {
            print("Welcome to Atrola!");
            sleep(0.5);
            // TODO: scene("main_apartment");
        }

        else {
            fprint(cRED("Error!") " Unknown command \"%s\".", original_command);
        }

        send(byte, cds_stream, MSG_LOG);
        var log = format("Invoked command \"%s\".\n", input);

        send_array(cds_stream, log);

        free(log);
        free(input);
        free(command);
        free(original_command);
    }
}