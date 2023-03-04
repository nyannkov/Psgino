/*
 * MIT License, see the LICENSE file for details.
 *
 * Copyright (c) 2023 nyannkov
 */
#ifndef PSGINO_SAMPLE_DRIVERS_DRIVER_YMZ294_H
#define PSGINO_SAMPLE_DRIVERS_DRIVER_YMZ294_H

#include <Arduino.h>
#include <stdint.h>

namespace PsginoSampleDrivers
{
    class DriverYMZ294
    {
    public:
        inline static void pin_config()
        {
            /*
             * This is an example of setup for YMZ294.
             */
            pinMode( 2, OUTPUT);   /* D0 : DATA(LSB) */
            pinMode( 3, OUTPUT);   /* D1 : DATA      */
            pinMode( 4, OUTPUT);   /* D2 : DATA      */
            pinMode( 5, OUTPUT);   /* D3 : DATA      */
            pinMode( 6, OUTPUT);   /* D4 : DATA      */
            pinMode( 7, OUTPUT);   /* D5 : DATA      */
            pinMode( 8, OUTPUT);   /* D6 : DATA      */
            pinMode( 9, OUTPUT);   /* D7 : DATA(MSB) */
            pinMode(10, OUTPUT);   /* /WR and /CS : Write enable and chip select. */
            pinMode(11, OUTPUT);   /* A0 : Select ADDR/DATA. */

            digitalWrite( 2, LOW);
            digitalWrite( 3, LOW);
            digitalWrite( 4, LOW);
            digitalWrite( 5, LOW);
            digitalWrite( 6, LOW);
            digitalWrite( 7, LOW);
            digitalWrite( 8, LOW);
            digitalWrite( 9, LOW);
            digitalWrite(10, HIGH);
            digitalWrite(11, HIGH);
        }

        inline static void write(uint8_t addr, uint8_t data)
        {
            /* ADDRESS */
            digitalWrite(11, LOW);
            digitalWrite(10, LOW);
            digitalWrite( 2, ( (addr & (1<<0) ) !=0 ) ? HIGH : LOW);
            digitalWrite( 3, ( (addr & (1<<1) ) !=0 ) ? HIGH : LOW);
            digitalWrite( 4, ( (addr & (1<<2) ) !=0 ) ? HIGH : LOW);
            digitalWrite( 5, ( (addr & (1<<3) ) !=0 ) ? HIGH : LOW);
            digitalWrite( 6, ( (addr & (1<<4) ) !=0 ) ? HIGH : LOW);
            digitalWrite( 7, ( (addr & (1<<5) ) !=0 ) ? HIGH : LOW);
            digitalWrite( 8, ( (addr & (1<<6) ) !=0 ) ? HIGH : LOW);
            digitalWrite( 9, ( (addr & (1<<7) ) !=0 ) ? HIGH : LOW);
            digitalWrite(10, HIGH);

            /* DATA */
            digitalWrite(11, HIGH);
            digitalWrite(10, LOW);
            digitalWrite( 2, ( (data & (1<<0) ) !=0 ) ? HIGH : LOW);
            digitalWrite( 3, ( (data & (1<<1) ) !=0 ) ? HIGH : LOW);
            digitalWrite( 4, ( (data & (1<<2) ) !=0 ) ? HIGH : LOW);
            digitalWrite( 5, ( (data & (1<<3) ) !=0 ) ? HIGH : LOW);
            digitalWrite( 6, ( (data & (1<<4) ) !=0 ) ? HIGH : LOW);
            digitalWrite( 7, ( (data & (1<<5) ) !=0 ) ? HIGH : LOW);
            digitalWrite( 8, ( (data & (1<<6) ) !=0 ) ? HIGH : LOW);
            digitalWrite( 9, ( (data & (1<<7) ) !=0 ) ? HIGH : LOW);
            digitalWrite(10, HIGH);
        }
    };
}

#endif/*PSGINO_SAMPLE_DRIVERS_DRIVER_YMZ294_H*/
