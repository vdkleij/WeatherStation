#-------------------------------------------------
#
# Project created by QtCreator 2014-01-16T11:57:47
#
#-------------------------------------------------

QT       += core sql

QT       -= gui

TARGET = WeatherStation
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    weatherdatabase.cpp \
    dht22sensor.cpp \
    bmp085.cpp \
    weatherstation.cpp

target.path = /home/pi
INSTALLS += target

HEADERS += \
    weatherdatabase.h \
    dht22sensor.h \
    bmp085.h \
    weatherstation.h

unix:!macx: LIBS += -L$$PWD/../../../mnt/raspberry-rootfs/usr/local/lib/ -lbcm2835

INCLUDEPATH += $$PWD/../../../mnt/raspberry-rootfs/usr/local/include
DEPENDPATH += $$PWD/../../../mnt/raspberry-rootfs/usr/local/include

unix:!macx: PRE_TARGETDEPS += $$PWD/../../../mnt/raspberry-rootfs/usr/local/lib/libbcm2835.a

unix:!macx: LIBS += -L$$PWD/../../../mnt/raspberry-rootfs/usr/local/lib/ -lwiringPi

INCLUDEPATH += $$PWD/../../../mnt/raspberry-rootfs/usr/local/include
DEPENDPATH += $$PWD/../../../mnt/raspberry-rootfs/usr/local/include

unix:!macx: LIBS += -lrt
