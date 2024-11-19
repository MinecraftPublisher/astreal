#include "../headers/common.h"

float default_time = 0.05f;

void stage_main(game_data *user) {
    //TODO
}

void stage_intro(game_data *user) {
    bool has_color = user->has_color;

    type(reduce(
        cUBLU("\"Based on recent reports, the possible migration of humans to our moon "
             "Ethet is now closer than ever. Planetary reform specialist Era Ahgo will "
             "come on live TV for a public announcement soon, Stay tuned!\"\n"
             "\"A zoo reports the escape of five camels.....\"\n"),
        "The sound of the TV news wakes you up.\n"
        "Today is a Saturday, a window of peace before you have to go to work again on Sunday."));
    
    stage_main(user);
}