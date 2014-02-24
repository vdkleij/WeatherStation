#ifndef WEATHERSTATION_H
#define WEATHERSTATION_H

#include <bmp085.h>
#include <dht22sensor.h>
#include <weatherdatabase.h>

#define ACQUISITION_INTERVAL (60) //seconds

class WeatherStation
{
public:
    WeatherStation(bool purge_database, bool debugmode);
    void start_acquisition();
private:
    BMP085 *bmp085sensor;
    DHT22Sensor *dht22sensor;
    WeatherDatabase *weatherdatabase;
    bool purge_database;
    bool debugmode;
};

#endif // WEATHERSTATION_H
