#pragma once

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <iostream>
#include <vector>
// #include <chrono>
#include <cmath>
#include <map>
#include "axes.h"

// Opcodes
#define PRTCL_INFO 0x01
#define CONF_PRTCL 0x02
#define CNTRL_PIPE 0x03
#define PIPE_STATUS 0x04
#define NOTIF_PIPE 0x05
#define MEAS_PIPE 0x06

using namespace std;

struct imu_data_t
{
    axes_t accel;
    axes_t del_v;
    axes_t ang_v;
    axes_t heading;
};

int imu_init(int address);
bool go_to_config();
bool go_to_measurement();
imu_data_t imu_read_data();
void parse_msg(imu_data_t &imu_data);
void byte_offset();
void find_byte_offset();
bool check_byte_offset();
void conv_to_float(const int &byte_offset, axes_t &axes);
bool send_xbus_msg(vector<unsigned char> cmd);
// int continuous_read(vector<unsigned char> *buf, unsigned char opcode);
inline void set_offset(imu_data_t &offset);
void imu_moving_avg_calibrate(map<int, imu_data_t> &data_set);
