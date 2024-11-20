#include "../headers/common.h"

float default_time = 0.02f;

void stage_apartment_balcony(game_data *user) {}

void stage_apartment(game_data *user) {
    bool has_color = user->has_color;

    type("You are standing in your room. It's a small apartment, made even smaller by "
         "the mess you've made in it. You should probably LOOK around.");

    REWIND:;
    var cmd_out = null;
    while ((cmd_out = command(
               hide_commands,
               null,
               has_color,
               default_time,
               /* number of commands */ 6,
               cmd("look",
                   reduce(
                       "You look around your room.\nYou can spot " cBWHT("CLOTHES"),
                       ", a " cBWHT("COMPUTER"),
                       ", and a " cBWHT("LETTER") ", all of which you can EXAMINE."),
                   2),
               cmd("examine", "What do you want to examine exactly?", 2),
               cmd("examine letter",
                   "It's a notice of delayed payment, your rent was due two weeks ago "
                   "and you still haven't had the means to pay it.",
                   2),
               cmd("examine clothes",
                   "Your daily use pile of clothing. They're all messy and unclean. "
                   "Maybe this is why you keep bombing your every interview. Some are way too old and torn, so you coud probably PICK them up and sell them later.",
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
                cmd("pick", "What do you want to pick up?", 3)

                   ))
           == (ptr) 2);
    
    if(cmd_out == (ptr)3) {
        add_to_inventory(user, TORN_CLOTHES);
        goto REWIND;
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
        has_color,
        default_time,
        1,
        cmd("get up", "You get up, and switch off the TV.", stage_apartment))(user);
}