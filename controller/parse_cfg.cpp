#include "parse_cfg.h"

cfg_data_t parse_cfg(std::string filename)
{
    setlocale(LC_ALL,"");

    cfg_data_t cfg_data;
    
    config4cpp::Configuration *file = config4cpp::Configuration::create();
    
    const std::string gains_scope = "PID_gains";

    try
    {
        file->parse(&filename[0]);
        cfg_data.throttle = parse_gains(file,&(gains_scope + ".throttle")[0]);
        cfg_data.x = parse_gains(file,&(gains_scope + ".x")[0]);
        cfg_data.y = parse_gains(file,&(gains_scope + ".y")[0]);
        cfg_data.z = parse_gains(file,&(gains_scope + ".z")[0]);
        cfg_data.x_pos = parse_gains(file,&(gains_scope + ".x_pos")[0]); // make sure this doesnt conflict with .x
        cfg_data.y_pos = parse_gains(file,&(gains_scope + ".y_pos")[0]);
    }
    catch(const config4cpp::ConfigurationException &e)
    {
        std::cout << e.c_str() << std::endl;
    }
    
    file->destroy();

    return cfg_data;
}

cfg_data_t::gains_t parse_gains(config4cpp::Configuration *file, char *scope)
{
    cfg_data_t::gains_t gains;

    gains.kp = file->lookupFloat(scope,"kp");
    gains.ki = file->lookupFloat(scope,"ki");
    gains.kd = file->lookupFloat(scope,"kd");

    return gains;
}