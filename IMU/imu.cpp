#include "imu.h"

static int file = 0; // File descriptor
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

    buf.resize(100);

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

    if (write(file, &READ_DATA, 1) != 1 || read(file, &buf[0], buf.size()) != buf.size())
    {
        cout << "Error writing/reading from I2C device" << endl;
        return imu_data;
    }

    parse_msg(imu_data);

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

// Write data to imu_data struct
void parse_msg(imu_data_t &imu_data)
{
    if(!check_sum())
    {
        cout << "check sum fail" << endl;
    }

    int data_len = buf[1];
    cout << "data length: " << data_len << "\t";

    for (int i = 2; i < data_len; ++i)
    {
        if (buf[i] == 0x50 && buf[i+1] == 0x40 && buf[i+2] == 0x08) // XDI_LatLon 100 Hz
        {
            parse_float(imu_data.gps.lat, i + 3);
            parse_float(imu_data.gps.lon, i + 7);
        }
        else if (buf[i] == 0x50 && buf[i+1] == 0x20 && buf[i+2] == 0x04) // XDI_AltitudeEllipsoid 100 Hz
        {
            parse_float(imu_data.alt, i + 3);
        }
        else if (buf[i] == 0xD0 && buf[i+1] == 0x10 && buf[i+2] == 0x0C) // XDI_VelocityXYZ 100 Hz
        {
            parse_float(imu_data.velocity.x, i + 3);
            parse_float(imu_data.velocity.y, i + 7);
            parse_float(imu_data.velocity.z, i + 11);
        }
        else if (buf[i] == 0x80 && buf[i+1] == 0x20 && buf[i+2] == 0x0C) //XDI_RateOfTurn 100 Hz
        {
            parse_float(imu_data.ang_v.x, i + 3);
            parse_float(imu_data.ang_v.y, i + 7);
            parse_float(imu_data.ang_v.z, i + 11);
        }
        else if (buf[i] == 0x20 && buf[i+1] == 0x30 && buf[i+2] == 0x0C) // XDI_EulerAngles 100 Hz
        {
            parse_float(imu_data.heading.x, i + 3);
            parse_float(imu_data.heading.y, i + 7);
            parse_float(imu_data.heading.z, i + 11);
        }
    }

    return;
}

// parses buf[] for the num at the num_offset
void parse_float(float &num, int num_offset)
{
    unsigned char arr[4] = {0,0,0,0};

    arr[3] = buf[num_offset];
    arr[2] = buf[num_offset + 1];
    arr[1] = buf[num_offset + 2];
    arr[0] = buf[num_offset + 3];

    num = *(float*)&arr;
}

// checks the checksum byte to confirm valid message
bool check_sum()
{
    unsigned char sum = 0xFF;
    int msg_len = buf[1] + 3;
    for (int i = 0; i < msg_len; ++i)
    {
        sum += buf[i];
    }

    if (sum != 0x00)
    {
        for(int x : buf)
        {
            cout << hex << x << " ";
        }
        cout << "\nCalc sum: " << hex << (int)(sum) << " ";
	    return false;
    }
    return true;
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
    vector<unsigned char> ack;
    ack.resize(60);
    unsigned char temp = 0x05;
    unsigned char meas = 0x06;
    // if (write(file, &temp, 1) != 1 || read(file,&ack[0],ack.size()) != ack.size())
    // {
	//     cout << "Clear buf fail" << endl;
    // }
    // int count = 0;
    // while (ack[0] != 0x00)
    // {
	//     if (write(file,&temp,1) != 1);
    //     {
	//         cout << "write" << endl;
	//         break;
	//     }
    //     if (read(file,&ack[0],ack.size()) != ack.size())
    //     {
    //         cout << "read" << endl;
    //         break;
    //     }
    //     if (count > 1000)
    //     {
    //         cout << "count too big" << endl;
    //         break;
    //     }
    // }
    // cout << "Count: " << count << endl;
    if (write(file, &cmd[0], cmd.size()) != cmd.size())// || read(file,&ack[0],ack.size()) != ack.size())
    {
        cout << "Error writing xbus command to I2C device" << endl;
        return false;
    }
    
    if (write(file, &temp, 1) != 1 || read(file,&ack[0],ack.size()) != ack.size())
    {
	cout << "Error reading notif pipe" << endl;
    }

    for (int i = 0; i < ack.size(); ++i)
    {
	cout << hex << (int)ack[i] << " ";
    }

    temp = 0x04;
    if (write(file, &temp, 1) != 1 || read(file,&ack[0],ack.size()) != ack.size())
    {
	cout << "Error reading pipe status" << endl;
    }
    cout << endl;
    for (int i = 0; i < ack.size(); ++i)
    {
	cout << hex << (int)ack[i] << " ";
    }

    return true;
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
