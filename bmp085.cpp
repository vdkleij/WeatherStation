#include "bmp085.h"


BMP085::BMP085()
{
    this->cal_AC1 = 0;
    this->cal_AC2 = 0;
    this->cal_AC3 = 0;
    this->cal_AC4 = 0;
    this->cal_AC5 = 0;
    this->cal_AC6 = 0;
    this->cal_B1 = 0;
    this->cal_B2 = 0;
    this->cal_MB = 0;
    this->cal_MC = 0;
    this->cal_MD = 0;

    this->sensor_initialized = false;
    this->mode = 1;
    this->fd = -1;
}

void BMP085::initsensor()
{
    this->fd = wiringPiI2CSetup(BMP085_DEVID);

    if(this->fd != -1)
        this->sensor_initialized = true;

    if(this->sensor_initialized)
        this->read_calibration_data();
}

void BMP085::read_temperature(float *temperature)
    /* Gets the compensated temperature in degrees celcius */
{
  long UT = 0;
  long X1 = 0;
  long X2 = 0;
  long B5 = 0;

  // Read raw temp before aligning it with the calibration values
  this->read_raw_temp((int*)&UT);
  X1 = ((UT - this->cal_AC6) * this->cal_AC5) >> 15;
  X2 = (this->cal_MC << 11) / (X1 + this->cal_MD);
  B5 = X1 + X2;
  *temperature = ((B5 + 8) >> 4) / 10.0;
}

void BMP085::read_pressure(float *pressure)
  /* Gets the compensated pressure in pascal */
{
  long UT = 0;
  long UP = 0;
  long B3 = 0;
  long B5 = 0;
  long B6 = 0;
  long X1 = 0;
  long X2 = 0;
  long X3 = 0;
  long p = 0;
  unsigned long B4 = 0;
  unsigned long B7 = 0;

  this->read_raw_temp((int*)&UT);
  this->read_raw_pressure((int*)&UP);

  // True Temperature Calculations
  X1 = ((UT - this->cal_AC6) * this->cal_AC5) >> 15;
  X2 = (this->cal_MC << 11) / (X1 + this->cal_MD);
  B5 = X1 + X2;

  // Pressure Calculations
  B6 = B5 - 4000;
  X1 = (this->cal_B2 * (B6 * B6) >> 12) >> 11;
  X2 = (this->cal_AC2 * B6) >> 11;
  X3 = X1 + X2;
  B3 = (((this->cal_AC1 * 4 + X3) << this->mode) + 2) / 4;

  X1 = (this->cal_AC3 * B6) >> 13;
  X2 = (this->cal_B1 * ((B6 * B6) >> 12)) >> 16;
  X3 = ((X1 + X2) + 2) >> 2;
  B4 = (this->cal_AC4 * (X3 + 32768)) >> 15;
  B7 = (UP - B3) * (50000 >> this->mode);

  if (B7 < 0x80000000)
    p = (B7 * 2) / B4;
  else
    p = (B7 / B4) * 2;

  X1 = (p >> 8) * (p >> 8);
  X1 = (X1 * 3038) >> 16;
  X2 = (-7357 * p) >> 16;

  *pressure = p + ((X1 + X2 + 3791) >> 4);
  // Convert to hPa
  *pressure = *pressure / 100;
}

void BMP085::read_altitude(float *altitude)
  /* Calculates the altitude in meters */
{
  float pressure = 0;
  this->read_pressure(&pressure);
  *altitude = ALTITUDE_FORMULA_CONSTANT * (1.0 - pow(pressure / SEA_LEVEL_PRESSURE, 0.1903));
}


void BMP085::read_calibration_data()
    /* Reads the calibration data from the IC */
{
    readS16(BMP085_CAL_AC1, &(this->cal_AC1));   // INT16
    readS16(BMP085_CAL_AC2, &(this->cal_AC2));   // INT16
    readS16(BMP085_CAL_AC3, &(this->cal_AC3));   // INT16
    readU16(BMP085_CAL_AC4, &(this->cal_AC4));   // UINT16
    readU16(BMP085_CAL_AC5, &(this->cal_AC5));   // UINT16
    readU16(BMP085_CAL_AC6, &(this->cal_AC6));   // UINT16
    readS16(BMP085_CAL_B1, &(this->cal_B1));     // INT16
    readS16(BMP085_CAL_B2, &(this->cal_B2));     // INT16
    readS16(BMP085_CAL_MB, &(this->cal_MB));     // INT16
    readS16(BMP085_CAL_MC, &(this->cal_MC));     // INT16
    readS16(BMP085_CAL_MD, &(this->cal_MD));     // INT16
}

void BMP085::show_calibration_data()
    /* Displays the calibration values for debugging purposes */
{
    printf("DBG: AC1 = %6d\n", this->cal_AC1);
    printf("DBG: AC2 = %6d\n", this->cal_AC2);
    printf("DBG: AC3 = %6d\n", this->cal_AC3);
    printf("DBG: AC4 = %6d\n", this->cal_AC4);
    printf("DBG: AC5 = %6d\n", this->cal_AC5);
    printf("DBG: AC6 = %6d\n", this->cal_AC6);
    printf("DBG: B1  = %6d\n", this->cal_B1);
    printf("DBG: B2  = %6d\n", this->cal_B2);
    printf("DBG: MB  = %6d\n", this->cal_MB);
    printf("DBG: MC  = %6d\n", this->cal_MC);
    printf("DBG: MD  = %6d\n", this->cal_MD);
}

void BMP085::read_raw_temp(int *rawtemp)
    /* Reads the raw (uncompensated) temperature from the sensor */
{
    wiringPiI2CWriteReg8(this->fd, BMP085_CONTROL, BMP085_READTEMPCMD);
    usleep(5000);  // Wait 5ms
    readU16(BMP085_TEMPDATA, (unsigned short*)rawtemp);
}

void BMP085::read_raw_pressure(int *rawpressure)
    /* Reads the raw (uncompensated) pressure level from the sensor */
{
    uint8_t msb, lsb, xlsb;
    wiringPiI2CWriteReg8(this->fd, BMP085_CONTROL, BMP085_READPRESSURECMD + (this->mode << 6));
    if (this->mode == BMP085_ULTRALOWPOWER)
      usleep(5000);
    else if (this->mode == BMP085_HIGHRES)
      usleep(14000);
    else if (this->mode == BMP085_ULTRAHIGHRES)
      usleep(26000);
    else
      usleep(8000);
    msb = wiringPiI2CReadReg8(this->fd, BMP085_PRESSUREDATA);
    lsb = wiringPiI2CReadReg8(this->fd, BMP085_PRESSUREDATA+1);
    xlsb = wiringPiI2CReadReg8(this->fd, BMP085_PRESSUREDATA+2);
    *rawpressure = ((msb << 16) + (lsb << 8) + xlsb) >> (8 - this->mode);
}

void BMP085::readS16(int reg, short *value)
    /* Reads a signed 16-bit value */
{
    int8_t hi = 0;
    uint8_t lo = 0;
    hi = (int8_t)wiringPiI2CReadReg8(this->fd, reg);
    lo = (uint8_t)wiringPiI2CReadReg8(this->fd, reg+1);
    *value = (hi << 8) + lo;
}

void BMP085::readU16(int reg, unsigned short *value)
    /* Reads an unsigned 16-bit value */
{
    uint8_t hi = 0;
    uint8_t lo = 0;
    hi = (uint8_t)wiringPiI2CReadReg8(this->fd, reg);
    lo = (uint8_t)wiringPiI2CReadReg8(this->fd, reg+1);
    *value = (hi << 8) + lo;
}

