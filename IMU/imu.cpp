#include "imu.h"

static int file = 0; // File descriptor
static int data_len = 0; // Length of data message in bytes (determined by current configuration)
static int heading_byte_offset = 0; // Heading data offset from start of messages in bytes
static int accel_byte_offset = 0; // Acceleration data offset from start of messages in bytes
static int ang_v_byte_offset = 0; // Angular velocity data offset from start of messages in bytes
static int del_v_byte_offset = 0; // Delta V data offset from start of messages in bytes
static vector<unsigned char> buf; // Buffer where received data is stored
static imu_data_t offset;

// Initializes IMU file and sets I2C slave
int imu_init(int address)    
{
    // Open the I2C device file
    if ((file = open("/dev/i2c-1", O_RDWR)) < 0)
    {
        cout << "Error opening I2C device file" << endl;
        return -1;
    }

    // Set the device address
    if (ioctl(file, I2C_SLAVE, address) < 0)
    {
        cout << "Error setting I2C device address" << endl;
        return -1;
    }

    byte_offset();

    return file;
}

// Sets IMU to configuration state
bool go_to_config()
{
    char cmd[4] = {CNTRL_PIPE,0x30,0x00,0xD1};

    if (write(file, cmd, 4) != 4)
    {
        std::cout << "Error writing to I2C device: device not set to config mode" << std::endl;
        return false;
    }
    return true;
}

// Sets IMU to measurement state
bool go_to_measurement()
{
    char cmd[4] = {CNTRL_PIPE,0x10,0x00,0xF1};

    if (write(file, cmd, 4) != 4)
    {
        std::cout << "Error writing to I2C device: device not set to measurement mode" << std::endl;
        return false;
    }
    return true;
}

// Reads data using the measurement pipe opcode and write data to buf
imu_data_t imu_read_data()
{
    imu_data_t imu_data;

    const unsigned char READ_DATA = MEAS_PIPE;

    if (write(file, &READ_DATA, 1) != 1 || read(file, &buf[0], data_len) != data_len)
    {
        cout << "Error writing/reading from I2C device" << endl;
        return imu_data;
    }

    parse_msg(imu_data);

    imu_data.accel.x -= offset.accel.x;
    imu_data.accel.y -= offset.accel.y;
    imu_data.accel.z -= offset.accel.z;
    imu_data.del_v.x -= offset.del_v.x;
    imu_data.del_v.y -= offset.del_v.y;
    imu_data.del_v.z -= offset.del_v.z;
    imu_data.ang_v.x -= offset.ang_v.x;
    imu_data.ang_v.y -= offset.ang_v.y;
    imu_data.ang_v.z -= offset.ang_v.z;
    
    // Prints out if data is NaN
    // if (isnan(imu_data.heading.x))
    // {
    //     cout << "Error: NaN found in message - ";
    //     for (int i = 0; i < data_len; ++i)
    //     {
    //         cout << hex << (int)buf[0] << " ";
    //     }
    //     cout << endl;
    // }

    return imu_data;
}

// Tries to find byte offsets for IMU data, if it cannot find offsets it tries again
void byte_offset()
{
    find_byte_offset();
    
    if (!check_byte_offset())
    {
        cout << "Trying to find byte offsets again..." << endl;

        sleep(1);

        find_byte_offset();

        if(!check_byte_offset())
        {
            cout << "Error finding byte offsets!" << endl;
            return;
        }
    }

    cout << "Success: IMU byte offsets found" << endl;

    return;
}

// Finds the byte offsets for each set of data
void find_byte_offset()
{
    buf.resize(2);

    const unsigned char READ_DATA = MEAS_PIPE;

    if (write(file, &READ_DATA, 1) != 1 || read(file, &buf[0], 2) != 2)
    {
        cout << "Error writing/reading from I2C device" << endl;
        return;
    }

    sleep(1);

    if (write(file, &READ_DATA, 1) != 1 || read(file, &buf[0], 2) != 2)
    {
        cout << "Error writing/reading from I2C device" << endl;
        return;
    }

    data_len = buf[1] + 3;

    if (data_len == 0)
    {
        cout << "Error reading data length" << endl;
        return;
    }

    sleep(1);

    buf.resize(data_len);

    if (write(file, &READ_DATA, 1) != 1 || read(file, &buf[0], data_len) != data_len)
    {
        cout << "Error writing/reading from I2C device" << endl;
        return;
    }

    for (int i = 2; i < data_len; ++i)
    {
        if (buf[i] == 0x20 && buf[i+1] == 0x30 && buf[i+2] == 0x0C)
        {
            heading_byte_offset = i + 3;
        }
        else if (buf[i] == 0x40 && buf[i+1] == 0x30 && buf[i+2] == 0x0C)
        {
            accel_byte_offset = i + 3;
        }
        else if (buf[i] == 0x80 && buf[i+1] == 0x20 && buf[i+2] == 0x0C)
        {
            ang_v_byte_offset = i + 3;
        }
        else if (buf[i] == 0x40 && buf[i+1] == 0x10 && buf[i+2] == 0x0C)
        {
            del_v_byte_offset = i + 3;
        }
    }
}

// Checks if all byte offsets were found
// \return true if all found, false if any are not found
bool check_byte_offset()
{
    bool byte_offsets_found = true;

    if (heading_byte_offset == 0)
    {
        cout << "Error finding heading byte offset" << endl;
        byte_offsets_found = false;
    }
    if (accel_byte_offset == 0)
    {
        cout << "Error finding acceleration byte offset" << endl;
        byte_offsets_found = false;
    }
    if (ang_v_byte_offset == 0)
    {
        cout << "Error finding angular velocity byte offset" << endl;
        byte_offsets_found = false;
    }
    if (del_v_byte_offset == 0)
    {
        cout << "Error finding delta V byte offset" << endl;
        byte_offsets_found = false;
    }

    return byte_offsets_found;
}

// Write data to imu_data struct, requires find_byte_offset to be run first \return imu_data_t object with parsed data
void parse_msg(imu_data_t &imu_data)
{
    conv_to_float(heading_byte_offset, imu_data.heading);
    conv_to_float(accel_byte_offset, imu_data.accel);
    conv_to_float(del_v_byte_offset, imu_data.del_v);
    conv_to_float(ang_v_byte_offset, imu_data.ang_v);

    return;
}

// Converts set of data to floats to be stored in axes_t struct
void conv_to_float(const int &byte_offset, axes_t &axes) 
{
    unsigned char parse_array[4] = {0,0,0,0};

    parse_array[3] = buf[byte_offset];
    parse_array[2] = buf[byte_offset + 1];
    parse_array[1] = buf[byte_offset + 2];
    parse_array[0] = buf[byte_offset + 3];

    axes.x = *(float*)&parse_array;

    parse_array[3] = buf[byte_offset + 4];
    parse_array[2] = buf[byte_offset + 5];
    parse_array[1] = buf[byte_offset + 6];
    parse_array[0] = buf[byte_offset + 7];

    axes.y = *(float*)&parse_array;

    parse_array[3] = buf[byte_offset + 8];
    parse_array[2] = buf[byte_offset + 9];
    parse_array[1] = buf[byte_offset + 10];
    parse_array[0] = buf[byte_offset + 11];

    axes.z = *(float*)&parse_array;

    return;
}

// Takes cmd without checksum, calculates checksum and sends message to IMU
bool send_xbus_msg(vector<unsigned char> cmd)
{
    unsigned char checksum = 0xFF;

    for (int i = 1; i < cmd.size(); ++i)
    {
        checksum += cmd[i];
    }

    checksum = (~checksum) + 1;

    cmd.push_back(checksum);

    if (write(file, &cmd[0], cmd.size()) != cmd.size())
    {
        cout << "Error writing xbus command to I2C device" << endl;
        return false;
    }

    return true;
}

// 
inline void set_offset(imu_data_t &_offset)
{
    offset = _offset;
    
    return;
}

// 
void imu_moving_avg_calibrate(map<int, imu_data_t> &data_set)
{
    imu_data_t imu_data;

    for (auto it = data_set.begin(); it != data_set.end(); ++it)
    {
        imu_data.accel.x += it->second.accel.x;
        imu_data.accel.y += it->second.accel.y;
        imu_data.accel.z += it->second.accel.z;
        imu_data.del_v.x += it->second.del_v.x;
        imu_data.del_v.y += it->second.del_v.y;
        imu_data.del_v.z += it->second.del_v.z;
        imu_data.ang_v.x += it->second.ang_v.x;
        imu_data.ang_v.y += it->second.ang_v.y;
        imu_data.ang_v.z += it->second.ang_v.z;
    }

    imu_data.accel.x /= data_set.size();
    imu_data.accel.y /= data_set.size();
    imu_data.accel.z /= data_set.size();
    imu_data.del_v.x /= data_set.size();
    imu_data.del_v.y /= data_set.size();
    imu_data.del_v.z /= data_set.size();
    imu_data.ang_v.x /= data_set.size();
    imu_data.ang_v.y /= data_set.size();
    imu_data.ang_v.z /= data_set.size();

    set_offset(imu_data);

    return;
}

// IN PROGRESS, MAY NOT WORK CORRECTLY!!! Reads continuously up to 1 second for a message from the specified opcode and writes the message of specified length to buf \return Time taken to receive message, -1 if no message received
// int continuous_read(vector<unsigned char> *data, unsigned char opcode)
// {
//     vector<unsigned char> temp = *data;
    
//     auto start = chrono::high_resolution_clock::now();
//     auto current = chrono::high_resolution_clock::now();
//     auto duration = chrono::duration_cast<chrono::milliseconds>(current - start);

//     while (duration.count() < 1000)
//     {
        

//     if ((write(file, &opcode, 1) == 1 || read(file, &data[0], data->size()) == data->size()) && temp != *data)
//     {
//         return duration.count();
//     }

//     current = chrono::high_resolution_clock::now();
//     duration = chrono::duration_cast<chrono::milliseconds>(current - start);
//     }

//     return -1;
// }
