#pragma once

#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <iostream>
#include <vector>

using namespace std;

struct imu_data_t
{
    struct axes_t
    {
        float x = 0;
        float y = 0;
        float z = 0;
    };

    axes_t accel;
    axes_t del_v;
    axes_t ang_v;
    axes_t heading;    
};

int imu_init(int address);
bool go_to_config();
bool go_to_measurement();
bool read_data(void *buf);
void find_byte_offset();
void parse_msg(const void *buf, imu_data_t *imu_data);
void conv_to_float(const void *buf, imu_data_t::axes_t *axes);
bool send_xbus_msg(vector<unsigned char> *cmd);
