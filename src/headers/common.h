#pragma once

#include "color.h"
#include "comms.h"
#include "macros.h"

const var current_version = 0.12;

typedef struct game_data {
    float version;
    byte  meow_times;
    bool  has_color;
    bool  passed_help;
    bool  apartment_entered;
    char  location[ 128 ];
} game_data;

game_data new_game() {
    return (game_data) {
        .version = current_version, .meow_times = 0, .has_color = 1, .location = "main"
    };
}

stage get_stage(ptr me, string _name) {
    var name = format("stage_%s", _name);
    var stage_handle = dynamic(me, name);
    free(name);

    return stage_handle;
}

#define $run(name) get_stage(me, name)(&game)