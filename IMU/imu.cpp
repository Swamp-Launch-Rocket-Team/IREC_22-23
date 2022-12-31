#include "imu.h"

static int file = 0; // File descriptor
static int data_len = 0; // Length of data message in bytes (determined by current configuration)
static int heading_byte_offset = 0; // Heading data offset from start of messages in bytes
static int accel_byte_offset = 0; // Acceleration data offset from start of messages in bytes
static int ang_v_byte_offset = 0; // Angular velocity data offset from start of messages in bytes
static int del_v_byte_offset = 0; // Delta V data offset from start of messages in bytes
static vector<unsigned char> buf; // Buffer where received data is stored

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

    find_byte_offset();

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
bool read_data()
{
    static const unsigned char READ_DATA = MEAS_PIPE;

    if (write(file, &READ_DATA, 1) != 1 || read(file, &buf[0], data_len) != data_len)
    {
        cout << "Error writing/reading from I2C device" << endl;
        return false;
    }
    return true;
}

// Finds the byte offsets for each set of data
void find_byte_offset()
{
    buf.resize(2);

    static const unsigned char READ_DATA = MEAS_PIPE;

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

    if (heading_byte_offset == 0)
    {
        cout << "Error finding heading byte offset" << endl;
    }
    if (accel_byte_offset == 0)
    {
        cout << "Error finding acceleration byte offset" << endl;
    }
    if (ang_v_byte_offset == 0)
    {
        cout << "Error finding angular velocity byte offset" << endl;
    }
    if (del_v_byte_offset == 0)
    {
        cout << "Error finding delta V byte offset" << endl;
    }
    return;
}

// Write data to imu_data struct, requires find_byte_offset to be run first \return imu_data_t object with parsed data
imu_data_t parse_msg()
{
    static imu_data_t imu_data;

    conv_to_float(&heading_byte_offset, &(imu_data.heading));
    conv_to_float(&accel_byte_offset, &(imu_data.accel));
    conv_to_float(&del_v_byte_offset, &(imu_data.del_v));
    conv_to_float(&ang_v_byte_offset, &(imu_data.ang_v));

    return imu_data;
}

// Converts set of data to floats to be stored in axes_t struct
void conv_to_float(const int *byte_offset, imu_data_t::axes_t *axes) 
{
    static unsigned char parse_array[4] = {0,0,0,0};

    parse_array[3] = buf[*byte_offset];
    parse_array[2] = buf[*byte_offset + 1];
    parse_array[1] = buf[*byte_offset + 2];
    parse_array[0] = buf[*byte_offset + 3];

    axes->x = *(float*)&parse_array;

    parse_array[2] = buf[*byte_offset + 5];
    parse_array[3] = buf[*byte_offset + 4];
    parse_array[1] = buf[*byte_offset + 6];
    parse_array[0] = buf[*byte_offset + 7];

    axes->y = *(float*)&parse_array;

    parse_array[3] = buf[*byte_offset + 8];
    parse_array[2] = buf[*byte_offset + 9];
    parse_array[1] = buf[*byte_offset + 10];
    parse_array[0] = buf[*byte_offset + 11];

    axes->z = *(float*)&parse_array;

    return;
}

// Takes cmd without checksum, calculates checksum and sends message to IMU
bool send_xbus_msg(vector<unsigned char> *cmd)
{
    unsigned char checksum = 0xFF;

    for (int i = 1; i < cmd->size(); ++i)
    {
        checksum += cmd->at(i);
    }

    checksum = (~checksum) + 1;

    cmd->push_back(checksum);

    if (write(file, &cmd[0], cmd->size()) != cmd->size())
    {
        cout << "Error writing xbus command to I2C device" << endl;
        return false;
    }

    return true;
}

// IN PROGRESS, MAY NOT WORK CORRECTLY!!! Reads continuously up to 1 second for a message from the specified opcode and writes the message of specified length to buf \return Time taken to receive message, -1 if no message received
int continuous_read(vector<unsigned char> *data, unsigned char opcode)
{
    vector<unsigned char> temp = *data;
    
    auto start = chrono::high_resolution_clock::now();
    auto current = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(current - start);

    while (duration.count() < 1000)
    {
        

    if ((write(file, &opcode, 1) == 1 || read(file, &data[0], data->size()) == data->size()) && temp != *data)
    {
        return duration.count();
    }

    current = chrono::high_resolution_clock::now();
    duration = chrono::duration_cast<chrono::milliseconds>(current - start);
    }

    return -1;
}
