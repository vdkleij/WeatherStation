#include "weatherstation.h"

WeatherStation::WeatherStation(bool purge_database, bool debugmode)
{
    this->bmp085sensor = NULL;
    this->dht22sensor = NULL;
    this->weatherdatabase = NULL;

    this->purge_database = purge_database;
    this->debugmode = debugmode;
}

void WeatherStation::start_acquisition()
{
    float temperature, humidity, airpressure = 0;
    bool success = true;

    this->bmp085sensor = new BMP085();
    this->dht22sensor = new DHT22Sensor();
    this->weatherdatabase = new WeatherDatabase();

    this->weatherdatabase->OpenDatabase();

    if(this->purge_database)
        this->weatherdatabase->PurgeDatabase();

    this->bmp085sensor->initsensor();
    this->dht22sensor->InitSensor();

    while(success)
    {
        success = false;

        for(int i = 0; i < DHT22_MAX_ATTEMPTS && !success; i++)
            success = dht22sensor->readDHT(DHT22_PIN_NR, &temperature, &humidity);

        bmp085sensor->read_pressure(&airpressure);

        // Take picture
        system("raspistill -n -w 320 -h 240 -q 100 -o image.jpg");

        weatherdatabase->AddAirpressureData(airpressure);
        weatherdatabase->AddHumidityData(humidity);
        weatherdatabase->AddTemperatureData(temperature);
        weatherdatabase->AddImageData("image.jpg");

        sleep(ACQUISITION_INTERVAL);
    }

    dht22sensor->CloseSensor();
    weatherdatabase->CloseDatabase();
}

