#pragma once

#include <config4cpp/Configuration.h>
#include <string>
#include <iostream>
#include <locale.h>

struct cfg_data_t
{
    struct gains_t
    {
        float kp;
        float ki;
        float kd;
    };

    gains_t throttle;
    gains_t x;
    gains_t y;
    gains_t z;
    gains_t x_pos;
    gains_t y_pos;
};

cfg_data_t parse_cfg(std::string filename);
cfg_data_t::gains_t parse_gains(config4cpp::Configuration *file, char *scope);
