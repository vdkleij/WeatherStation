#ifndef BMP085_H
#define BMP085_H

#include <wiringPiI2C.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>

#define BMP085_DEVID              0x77

// Operating Modes
#define BMP085_ULTRALOWPOWER      0
#define BMP085_STANDARD           1
#define BMP085_HIGHRES            2
#define BMP085_ULTRAHIGHRES       3

// BMP085 Registers
#define BMP085_CAL_AC1            0xAA  // R   Calibration data (16 bits)
#define BMP085_CAL_AC2            0xAC  // R   Calibration data (16 bits)
#define BMP085_CAL_AC3            0xAE  // R   Calibration data (16 bits)
#define BMP085_CAL_AC4            0xB0  // R   Calibration data (16 bits)
#define BMP085_CAL_AC5            0xB2  // R   Calibration data (16 bits)
#define BMP085_CAL_AC6            0xB4  // R   Calibration data (16 bits)
#define BMP085_CAL_B1             0xB6  // R   Calibration data (16 bits)
#define BMP085_CAL_B2             0xB8  // R   Calibration data (16 bits)
#define BMP085_CAL_MB             0xBA  // R   Calibration data (16 bits)
#define BMP085_CAL_MC             0xBC  // R   Calibration data (16 bits)
#define BMP085_CAL_MD             0xBE  // R   Calibration data (16 bits)
#define BMP085_CONTROL            0xF4
#define BMP085_TEMPDATA           0xF6
#define BMP085_PRESSUREDATA       0xF6
#define BMP085_READTEMPCMD        0x2E
#define BMP085_READPRESSURECMD    0x34

#define SEA_LEVEL_PRESSURE        991

#define ALTITUDE_FORMULA_CONSTANT 44330

class BMP085
{
public:
    BMP085();
    void initsensor();
    void read_temperature(float *temperature);
    void read_pressure(float *pressure);
    void read_altitude(float *altitude);
private:
    void show_calibration_data();
    void read_calibration_data();
    void read_raw_temp(int *rawtemp);
    void read_raw_pressure(int *rawpressure);
    void readS16(int reg, short *value);
    void readU16(int reg, unsigned short *value);

    short cal_AC1;
    short cal_AC2;
    short cal_AC3;
    unsigned short cal_AC4;
    unsigned short cal_AC5;
    unsigned short cal_AC6;
    short cal_B1;
    short cal_B2;
    short cal_MB;
    short cal_MC;
    short cal_MD;

    bool sensor_initialized;
    int mode;
    int fd;
};

#endif // BMP085_H
