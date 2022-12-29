#include "imu.h"
#include <chrono>

#define PRTCL_INFO 0x01
#define CONF_PRTCL 0x02
#define CNTRL_PIPE 0x03
#define PIPE_STATUS 0x04
#define NOTIF_PIPE 0x05
#define MEAS_PIPE 0x06

void print_data(unsigned char data[], int len);
void print_all(imu_data_t *imu_data);

int main()
{
    // Set up variables for the I2C device address, file descriptor, and data buffer
    int address = 0x6B;
    int file;
    unsigned char data[40];
    // char cmd[5] = {0xFA,0xFF,0x00,0x00,0x01};
    // char cmd[4] = {CNTRL_PIPE,0x10,0x00,0xF1}; // go to measurement state
    // char cmd[4] = {CNTRL_PIPE,0x30,0x00,0xD1}; // go to config
    // char cmd[20] = {CNTRL_PIPE,0xC0,0x10,0x20,0x30,0x00,0x64,0x40,0x30,0xFF,0xFF,0x80,0x20,0xFF,0xFF,0x40,0x10,0xFF,0xFF,0x23}; // set euler angles thingy
    // char cmd[1] = {MEAS_PIPE};
    // char cmd[] = {CNTRL_PIPE,0x0E,0x00,}

    file = imu_init(address);

    // if (write(file, cmd, 4) != 4 || read(file, data, 40) != 40)
    // {
    //     cout << "Error writing/reading from I2C device" << endl;
    // }

    // print_data(data,40);

    // // Read 20 bytes from the I2C device starting at register 0x1C (Acceleration)
    // read_data(&data);


    imu_data_t imu_data;

    while (true)
    {
        read_data(&data);

        parse_msg(&data,&imu_data);

        print_all(&imu_data);
    }


    // Close the I2C device file
    close(file);

    return 0;
}

void print_data(unsigned char data[], int len)
{
    for (int i = 0; i < len; ++i)
    {
        std::cout << std::hex << (int)data[i];
        std::cout << " ";
    }
    std::cout << std::endl;
}

void print_all(imu_data_t *imu_data)
{
    printf("Heading: %.4f | %.4f | %.4f ||\t Accel: %.4f | %.4f | %.4f ||\t Ang v: %.4f | %.4f | %.4f ||\t Del v: %.4f | %.4f | %.4f ||\n",
        imu_data->heading.x,imu_data->heading.y,imu_data->heading.z,
        imu_data->accel.x,imu_data->accel.y,imu_data->accel.z,
        imu_data->ang_v.x,imu_data->ang_v.y,imu_data->ang_v.z,
        imu_data->del_v.x,imu_data->del_v.y,imu_data->del_v.z);

    usleep(250000);
}

