#pragma once

#include "color.h"
#include "comms.h"
#include "macros.h"

const var current_version = 0.12;

game_data new_game() {
    return (game_data) { .version     = current_version,
                         .meow_times  = 0,
                         .has_color   = yes,
                         .location    = "main",
                         .passed_help = no,
                         .items       = {} };
}

stage get_stage(ptr me, string _name) {
    var name         = format("stage_%s", _name);
    var stage_handle = dynamic(me, name);
    free(name);

    return stage_handle;
}

#define $run(name) get_stage(me, name)(&game)