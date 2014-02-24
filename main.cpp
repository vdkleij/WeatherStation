/*
 * Author:      Matthijs van der Kleij
 * Date:        24-01-2014
 * Description: main function implementation
 */


#include <QCoreApplication>
#include <QCommandLineParser>
#include <weatherstation.h>

int main(int argc, char *argv[])
{
    bool purge_database, debugmode = false;

    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("Raspberry Weatherstation");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Weatherstation based on RPi");
    parser.addHelpOption();
    parser.addVersionOption();

    // Boolean command line option with multiple names (-d, --debugmode)
    QCommandLineOption debugOption(QStringList() << "d" << "debugmode", "Enable debugmode");
    parser.addOption(debugOption);

    // Boolean command line option with multiple names (-p, --purge)
    QCommandLineOption purgeOption(QStringList() << "p" << "purge", "Purge weather database");
    parser.addOption(purgeOption);

    // Process the actual command line arguments given by the user
    parser.process(app);

    debugmode = parser.isSet(debugOption);
    purge_database = parser.isSet(purgeOption);

    WeatherStation *weatherstation = new WeatherStation(purge_database, debugmode);

    weatherstation->start_acquisition();

    return app.exec();
}
