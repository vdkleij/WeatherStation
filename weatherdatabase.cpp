/*
 * Author:      Matthijs van der Kleij
 * Date:        24-01-2014
 * Description: This class offers functionality add the temperature, humidity
 *              and air pressure to the mysql database.
 */

#include "weatherdatabase.h"
#include <QSqlError>

WeatherDatabase::WeatherDatabase()
/*
 * Constructor.
 *
 * in:  none
 * out: none
 */
{
    // Use generic "QMYSQL" database for connection setup
    this->db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("eu-cdbr-azure-west-b.cloudapp.net");
    db.setDatabaseName("ictweataqjswbn7u");
    db.setUserName("bc156d741860ea");
    db.setPassword("0b704a62");

    this->database_opened = false;
}

void WeatherDatabase::OpenDatabase()
/*
 * Open the weatherdatabase. A new database with corresponsing tables will be
 * created if these do not exist.
 *
 * in:  none
 * out: none
 */
{
    // Open the "QMYSQL" database, this makes sure a initial database object
    // can be opened to check for the existence of the weatherdatabase.
    bool ok = db.open();

    if(ok)
    {
        QSqlQuery query;

        // Create required tables if not existing.
        query.exec("CREATE TABLE IF NOT EXISTS temperaturedata (datetime DATETIME, temperature FLOAT)");
        query.exec("CREATE TABLE IF NOT EXISTS humiditydata (datetime DATETIME, humidity FLOAT)");
        query.exec("CREATE TABLE IF NOT EXISTS airpressuredata (datetime DATETIME, airpressure FLOAT)");
        query.exec("CREATE TABLE IF NOT EXISTS imagedata (id SMALLINT, image LONGBLOB, PRIMARY KEY (id))");

        this->database_opened = true;
    }
}

void WeatherDatabase::CloseDatabase()
/*
 * Close the weatherdatabase.
 *
 * in:  none
 * out: none
 */
{
    db.close();
    this->database_opened = false;
}

void WeatherDatabase::AddTemperatureData(float temperature)
/*
 * Add temperature data to the weatherdatabase.
 * Also stores the date and time
 *
 * in:  temperature  Temperature collected from sensor.
 * out: none
 */
{
    QSqlQuery query;

    if(this->database_opened)
    {
        query.prepare("INSERT INTO temperaturedata "
                      "VALUES (NOW(), :temperature)");
        query.bindValue(":temperature", temperature);
        query.exec();
    }
}

void WeatherDatabase::AddHumidityData(float humidity)
/*
 * Add humidity data to the weatherdatabase.
 * Also stores the date and time
 *
 * in:  humidity  Humidity collected from sensor.
 * out: none
 */
{
    QSqlQuery query;

    if(this->database_opened)
    {
        query.prepare("INSERT INTO humiditydata "
                      "VALUES (NOW(), :humidity)");
        query.bindValue(":humidity", humidity);
        query.exec();
    }
}

void WeatherDatabase::AddAirpressureData(float airpressure)
/*
 * Add air pressure data to the weatherdatabase.
 * Also stores the date and time
 *
 * in:  airpressure  Airpressure collected from sensor.
 * out: none
 */
{
    QSqlQuery query;

    if(this->database_opened)
    {
        query.prepare("INSERT INTO airpressuredata "
                      "VALUES (NOW(), :airpressure)");
        query.bindValue(":airpressure", airpressure);
        query.exec();
    }
}

void WeatherDatabase::AddImageData(char* imagepath)
/*
 * Add image data to the weatherdatabase.
 *
 * in:  imagepath  Path to image.
 * out: none
 */
{
    QSqlQuery query;

    QByteArray bytearray;
    QFile f(imagepath);

    if(f.open(QIODevice::ReadOnly))
    {
        bytearray = f.readAll();
        f.close();
    }

    if(this->database_opened)
    {
        query.prepare("REPLACE INTO imagedata "
                      "VALUES (0, :image)");
        query.bindValue(":image", bytearray);
        query.exec();
    }
}

void WeatherDatabase::PurgeDatabase()
/*
 * Empty the entire weatherdatabase
 *
 * in:  none
 * out: none
 */
{
    QSqlQuery query;

    if(this->database_opened)
    {
        query.exec("DROP DATABASE ictweataqjswbn7u");

        // Recreate weatherdatabase.
        query.exec("CREATE DATABASE IF NOT EXISTS ictweataqjswbn7u");

        // Reopen the weatherdatabase
        db.setDatabaseName("ictweataqjswbn7u");
        bool ok = db.open();

        if(ok)
        {
            // Recreate required tables.
            query.exec("CREATE TABLE IF NOT EXISTS temperaturedata (datetime DATETIME, temperature FLOAT)");
            query.exec("CREATE TABLE IF NOT EXISTS humiditydata (datetime DATETIME, humidity FLOAT)");
            query.exec("CREATE TABLE IF NOT EXISTS airpressuredata (datetime DATETIME, airpressure FLOAT)");
            query.exec("CREATE TABLE IF NOT EXISTS imagedata (id SMALLINT, image LONGBLOB, PRIMARY KEY (id))");
        }
    }
}
