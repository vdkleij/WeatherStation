#ifndef WEATHERDATABASE_H
#define WEATHERDATABASE_H

/*
 * Author:      Matthijs van der Kleij
 * Date:        24-01-2014
 * Description: This class offers functionality add the temperature, humidity
 *              and air pressure to the mysql database.
 */

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <QDebug>

class WeatherDatabase
{
public:
    WeatherDatabase();

    void OpenDatabase();
    void CloseDatabase();

    void AddTemperatureData(float temperature);
    void AddHumidityData(float humidity);
    void AddAirpressureData(float airpressure);

    void PurgeDatabase();

private:
    QSqlDatabase db;
    bool database_opened;
};

#endif // WEATHERDATABASE_H
