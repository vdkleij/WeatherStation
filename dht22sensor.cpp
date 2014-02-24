/*
 * Author:      Matthijs van der Kleij
 * Date:        24-01-2014
 * Description: This class offers functionality to be able to read
 *              the temperature and humidity data from the DHT22 sensor.
 *              The code relies on the bcm2835 library to be able to read / write
 *              the GPIO pins of the Raspberry Pi.
 *              Refer to the datasheet of the DHT22 for more information on the
 *              protocol. (http://www.humiditycn.com/cp22.html)
 */


#include "dht22sensor.h"

static void detect_high_pulses_duration(int pin, QLinkedList<long> *high_pulse_duration_lst);
static void detect_high_level(int pin, timespec *time_stamp, bool *timedout);
static void detect_low_level(int pin, timespec *time_stamp, bool *timedout);

DHT22Sensor::DHT22Sensor()
/*
 * Constructor.
 *
 * in:  none
 * out: none
 */
{
    this->sensor_initialized = false;
}

void DHT22Sensor::InitSensor()
/*
 * Initialize the bcm2835 library.
 *
 * in:  none
 * out: none
 */
{
    this->sensor_initialized = bcm2835_init();
}

void DHT22Sensor::CloseSensor()
/*
 * Close the bcm2835 library.
 *
 * in:  none
 * out: none
 */
{
    this->sensor_initialized = bcm2835_close();
}

bool DHT22Sensor::readDHT(int pin, float *temperature, float *humidity)
/*
 * Read the temperature and humidity data from the DHT22 sensor.
 * The MaxDetect 1-wire bus is being controlled to obtain the data.
 * Refer to the DHT22 datasheet (see top) for more information.
 *
 * in:  pin         Raspberry pi GPIO pin that is connected to the sensor.
 * out: temperature Temperature that is read back (degrees celcius)
 *      humidity    Humdity that is read back (relative (%))
 */
{
    QLinkedList<long> high_pulse_duration_lst;
    long duration = 0;
    int data[10];
    int j=0;
    bool success = false;

    if(this->sensor_initialized)
    {
        // Set GPIO pin to output
        bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);

        // Send start signal
        bcm2835_gpio_write(pin, LOW);
        usleep(1000); // 1 msec low level required
        bcm2835_gpio_write(pin, HIGH);

        // Set GPIO pin to input
        bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_INPT);

        // Obtain a linked list containing the duration of all
        // the high level pulses send by the sensor.
        detect_high_pulses_duration(pin, &high_pulse_duration_lst);

        // Set sensor data array to zero.
        data[0] = data[1] = data[2] = data[3] = data[4] = 0;

        // The first 2 high pulses consist of the host start signal
        // and sensor response signals and need to be stripped
        // from the linked list.
        if(!high_pulse_duration_lst.empty())
        {
            high_pulse_duration_lst.pop_front();
            high_pulse_duration_lst.pop_front();
        }

        foreach(duration, high_pulse_duration_lst)
        {
            // shove each bit into the storage bytes
            data[j/8] <<= 1;
            // duration > 50 usec indicates a "1"
            // (from spec: "0" = 26-28 us, "1" = ~70 us)
            if ((duration / 1000) > 50)
              data[j/8] |= 1;
            j++;
        }

        // Verify checksum
        if(data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF))
        {
            success = true;

            // First 16 bits contain the humidity
            *humidity = data[0] * 256 + data[1];

            // Decimal output looks like e.g. 323 --> 32,3%
            // So divide by 10
            *humidity /= 10;

            // bits 16-32 contain the temperature
            // Mask bit 7 of the first 8 bits, because it is
            // the signed bit for negative/positive temperature
            *temperature = (data[2] & 0x7F) * 256 + data[3];

            // Decimal output looks like e.g. 253 --> 25,3 degrees celcius
            // So divide by 10
            *temperature /= 10.0;

            // Read bit 7 to verify if the temperature is negative/positive.
            if (data[2] & 0x80)
                *temperature *= -1;
        }
    }

    return success;
}

static void detect_high_pulses_duration(int pin, QLinkedList<long> *high_pulse_duration_lst)
{
/*
 * Detects the duration of all the high level pulses send by the sensor and stores
 * these values in a linked list.
 *
 *                   <------->
 * -----\           /---------\
 *       \         /           \
 *        \-------/             \-----------
 *
 * The figure above indicated what is meant by the duration of a high level pulse.
 * This duration is stored in a linked list until the sensor stops sending data.
 *
 * in:  pin                     Raspberry pi GPIO pin that is connected to the sensor.
 * out: high_pulse_duration_lst Linked list containing the duration of all high level pulses
 *                              send by the sensor.
 */
    timespec high_level_start, high_level_end;
    long duration = 0;
    bool timedout = false;

    while(!timedout)
    {
        // Detect a high --> low level transition and store the
        // corresponding timestamp.
        detect_low_level(pin, &high_level_end, &timedout);

        if(!timedout)
        {
            // Calculate the duration of a high level pulse by
            // using the start and end timestamp of the
            // low --> high and high --> low transitions.
            // The duration of each pulse is stored in the linked list.

            // In case the timestamps are as follows: e.g. start = 11.89s and end = 12.01s
            if(high_level_end.tv_sec > high_level_start.tv_sec)
            {
                duration = (LONG_MAX - high_level_start.tv_nsec) + high_level_end.tv_nsec;
                (*high_pulse_duration_lst).append(duration);
            }
            // In case the timestamps are as follows: start = 10.45s and end = 10.81s
            else
            {
                duration = high_level_end.tv_nsec - high_level_start.tv_nsec;
                (*high_pulse_duration_lst).append(duration);
            }
        }

        if(!timedout)
        {
            // Detect a low --> high level transition and store the
            // corresponding timestamp.
            detect_high_level(pin, &high_level_start, &timedout);
        }
    }
}

static void detect_high_level(int pin, timespec *time_stamp, bool *timedout)
/*
 * Detect a low to high level transition of the signal and store
 * the corresponding timestamp of this event.
 *
 * in:  pin         Raspberry pi GPIO pin that is connected to the sensor.
 * out: time_stamp  Timestamp of the level transition.
 *      timedout    Timeout indicating the level transition never took place,
 *                  meaning that the sensor stopped sending data or is not
 *                  responding.
 */
{
    timespec tp1, tp2;

    *timedout = false;

    clock_gettime(CLOCK_REALTIME, &tp1);
    clock_gettime(CLOCK_REALTIME, &tp2);

    // Wait for the pin to become HIGH. If this takes
    // longer than LEVEL_TIMEOUT, a timeout occurs.
    while (bcm2835_gpio_lev(pin) == LOW &&
           *timedout == false)
    {
        if((tp2.tv_sec - tp1.tv_sec) > DHT22_LEVEL_TIMEOUT)
            *timedout = true;
        // Update tp2 timestamp
        clock_gettime(CLOCK_REALTIME, &tp2);
    }
    if(!(*timedout))
        *time_stamp = tp2;
}

static void detect_low_level(int pin, timespec *time_stamp, bool *timedout)
/*
 * Detect a high to low level transition of the signal and store
 * the corresponding timestamp of this event.
 *
 * in:  pin         Raspberry pi GPIO pin that is connected to the sensor.
 * out: time_stamp  Timestamp of the level transition.
 *      timedout    Timeout indicating the level transition never took place,
 *                  meaning that the sensor stopped sending data or is not
 *                  responding.
 */
{
    timespec tp1, tp2;

    *timedout = false;

    clock_gettime(CLOCK_REALTIME, &tp1);
    clock_gettime(CLOCK_REALTIME, &tp2);

    // Wait for the pin to become LOW. If this takes
    // longer than LEVEL_TIMEOUT, a timeout occurs.
    while (bcm2835_gpio_lev(pin) == HIGH &&
           *timedout == false)
    {
        if((tp2.tv_sec - tp1.tv_sec) > DHT22_LEVEL_TIMEOUT)
            *timedout = true;
        // Update tp2 timestamp
        clock_gettime(CLOCK_REALTIME, &tp2);
    }
    if(!(*timedout))
        *time_stamp = tp2;
}
