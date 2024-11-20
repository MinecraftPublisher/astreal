#include "../headers/macros.h"

stage(apartment_balcony) {
    bool has_color = user->has_color;

    var text = format(
        "You step into the balcony.\5\5\5 You can see %s. You wonder if you should GO "
        "BACK inside your apartment, and then GO OUTSIDE.",
        user->time > 5
            ? "the deserted world outside, with most of the population having fled to "
              "Ethet"
            : "the lively city in front of you, full of sounds and lights and life");
    type(text);
    free(text);

    while (command(
               commands_in_help,
               null,
               cmd("go back",
                   "You sigh, and open the balcony door, heading back inside.",
                   1))
           != (ptr) 1);
}

stage(apartment) {
    bool has_color = user->has_color;

    type("You are standing in your room. It's a small apartment, made even smaller by "
         "the mess you've made in it. You should probably LOOK around.");

REWIND:;
    var cmd_out = null;

    while (
        (cmd_out = command(
             hide_commands,
             null,
             cmd("look",
                 reduce(
                     "You look around your room.\nYou can spot " cBWHT("CLOTHES"),
                     ", a " cBWHT("COMPUTER"),
                     ", and a " cBWHT("LETTER"),
                     ", all of which you can EXAMINE. You also see your BALCONY that you "
                     "can MOVE to."),
                 2),
             cmd("examine", "What do you want to examine exactly?", 2),
             cmd("examine letter",
                 "It's a notice of delayed payment, your rent was due two weeks ago "
                 "and you still haven't had the means to pay it.",
                 2),
             cmd("examine clothes",
                 "Your daily use pile of clothing. They're all messy and unclean. "
                 "Maybe this is why you keep bombing your every interview. Some of your "
                 "CLOTHES are way too old and torn, so you coud probably PICK them up "
                 "and sell them later.",
                 2),
             cmd("examine computer",
                 reduce(
                     "You unlock your computer. \5\5Inside is an advertisement for the "
                     "Ethet immigration program: ",
                     cCYN("THE LAND OF DREAMS THAT BECOME REAL! PURCHASE A -LIFETIME- "
                          "CITIZENSHIP IN THE UPCOMING HABITATION-O1 COLONY ON ETHET "
                          "FOR JUST 56,000 CREDITS! REPLY WITH \"IAMIN\" FOR A -FREE- "
                          "CONSULTATION CALL! (terms and conditions apply.)") "\n",
                     "\5And a message from your friend Aspet: ",
                     cHCYN("Hey, you free today?") "\n",
                     "But you haven't renewed your outbound transmit, so you can't "
                     "answer her message."),
                 2),
             cmd("pick", "What do you want to pick up?", 2),
             cmd("pick clothes",
                 "You pick up a few of your torn up clothes. They were added to your "
                 "inventory.",
                 3),

             cmd("move", "You can't move to nowhere.", 2),
             cmd("move balcony", null, 4),
             cmd("go", "Go where exactly?", 2),
             cmd("go outside", null, 5)))
        == (ptr) 2);

    if (cmd_out == (ptr) 3) {
        add_to_inventory(user, TORN_CLOTHES);
        goto REWIND;
    } else if (cmd_out == (ptr) 4) {
        stage_apartment_balcony(user);
        goto REWIND;
    } else if (cmd_out == (ptr) 5) {
        transfer("apartment_hallway", user);
    }
}

void stage_intro(game_data *user) {
    bool has_color = user->has_color;

    type(reduce(
        cUBLU("\"Based on recent reports, the possible migration of humans to our moon "
              "Ethet is now closer than ever. Planetary reform specialist Era Ahgo will "
              "come on live TV for a public announcement soon, Stay tuned!\"\n"
              "\"A zoo reports the escape of five camels.....\"\n\n"),
        "The sound of the TV news wakes you up.\n",
        "Today is a Saturday, a window of peace before you have to go to work again on "
        "Sunday."));

    command(
        commands_in_text,
        null,
        cmd("get up", "You get up, and switch off the TV.", stage_apartment))(user);
}