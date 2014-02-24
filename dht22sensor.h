#ifndef DHT22SENSOR_H
#define DHT22SENSOR_H

/*
 * Author:      Matthijs van der Kleij
 * Date:        24-01-2014
 * Description: This class offers functionality to be able to read
 *              the temperature and humidity data from the DHT22 sensor
 */


#include <QLinkedList>
#include <unistd.h>
#include <stdio.h>
#include <bcm2835.h>
#include <time.h>
#include <limits.h>

#define DHT22_LEVEL_TIMEOUT (2)
#define DHT22_MAX_ATTEMPTS  (10)
#define DHT22_PIN_NR        (4)

class DHT22Sensor
{
public:
    DHT22Sensor();
    void InitSensor();
    void CloseSensor();
    bool readDHT(int pin, float *temperature, float *humidity);

private:
    bool sensor_initialized;
};

#endif // DHT22SENSOR_H
