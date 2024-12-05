#include "../headers/macros.h"

stage(apartment_hallway) {
    type("You can see other apartment units, and a HALLWAY leading to the cafeteria and "
         "the elevator.");
    var cmd_out = null;
MAIN_HALLWAY_REWIND:;
    while ((cmd_out = command(
                hide_commands,
                null,
                cmd("look", 2,
                    "You look around. You can see apartment units F1F, F2F and F3F to "
                    "the left of your own apartment, F4F. On the other side of your "
                    "unit, you can see units F9F to F16F. To the right of your "
                    "apartment, you can see units F5F to F8F. You can KNOCK on the door of all of these units. ")
                    
                    ))
           == (ptr) 2);
}