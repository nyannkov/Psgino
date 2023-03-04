/*
 * MIT License, see the LICENSE file for details.
 *
 * Copyright (c) 2023 nyannkov
 */
#ifndef PSGINO_SAMPLE_DRIVERS_AY_3_8910_H
#define PSGINO_SAMPLE_DRIVERS_AY_3_8910_H

#include <Arduino.h>
#include <stdint.h>

namespace PsginoSampleDrivers
{
    class DriverAY_3_8910
    {
    public:
        inline static void pin_config()
        {
            /*
             * This is an example of setup for AY-3-8910.
             */
            pinMode( 2, OUTPUT);   /* D0 : DATA(LSB) */
            pinMode( 3, OUTPUT);   /* D1 : DATA      */
            pinMode( 4, OUTPUT);   /* D2 : DATA      */
            pinMode( 5, OUTPUT);   /* D3 : DATA      */
            pinMode( 6, OUTPUT);   /* D4 : DATA      */
            pinMode( 7, OUTPUT);   /* D5 : DATA      */
            pinMode( 8, OUTPUT);   /* D6 : DATA      */
            pinMode( 9, OUTPUT);   /* D7 : DATA(MSB) */
            pinMode(10, OUTPUT);   /* BDIR */
            pinMode(11, OUTPUT);   /* BC2  */
                                   /* NOTE: Fix BC1 to LOW. */

            digitalWrite( 2, LOW);
            digitalWrite( 3, LOW);
            digitalWrite( 4, LOW);
            digitalWrite( 5, LOW);
            digitalWrite( 6, LOW);
            digitalWrite( 7, LOW);
            digitalWrite( 8, LOW);
            digitalWrite( 9, LOW);
            digitalWrite(10, LOW);
            digitalWrite(11, LOW);
        }

        inline static void write(uint8_t addr, uint8_t data)
        {
            /*
             * NOTE:
             *
             * The associative delay time of the bus signals (BDIR, BC1, BC2) 
             * must be less than 50 ns. Therefore, when controlling these logics 
             * one by one with digitalWrite(), care must be taken in the order of
             * the signals whose logic is to be changed.
             */

            /* 
             * BDIR BC2 BC1
             *    0   0   0   NACT (INACTIVE)
             */
            digitalWrite(10, LOW);  // BDIR
            digitalWrite(11, LOW);  // BC2

            /* SET ADDRESS */
            digitalWrite( 2, ( (addr & (1<<0) ) !=0 ) ? HIGH : LOW);
            digitalWrite( 3, ( (addr & (1<<1) ) !=0 ) ? HIGH : LOW);
            digitalWrite( 4, ( (addr & (1<<2) ) !=0 ) ? HIGH : LOW);
            digitalWrite( 5, ( (addr & (1<<3) ) !=0 ) ? HIGH : LOW);
            digitalWrite( 6, LOW);
            digitalWrite( 7, LOW);
            digitalWrite( 8, LOW);
            digitalWrite( 9, LOW);

            /* 
             * BDIR BC2 BC1
             *    1   0   0   ADAR (LATCH ADDRESS)
             */
            digitalWrite(10, HIGH); // BDIR
            delayMicroseconds(1);   // t_AS = 300 ns < 1 us

            /* 
             * BDIR BC2 BC1
             *    0   0   0   NACT (INACTIVE)
             */
            digitalWrite(10, LOW);  // BC1
            delayMicroseconds(1);   // t_AH = 50 ns < 1 us

            /* SET DATA */
            digitalWrite( 2, ( (data & (1<<0) ) !=0 ) ? HIGH : LOW);
            digitalWrite( 3, ( (data & (1<<1) ) !=0 ) ? HIGH : LOW);
            digitalWrite( 4, ( (data & (1<<2) ) !=0 ) ? HIGH : LOW);
            digitalWrite( 5, ( (data & (1<<3) ) !=0 ) ? HIGH : LOW);
            digitalWrite( 6, ( (data & (1<<4) ) !=0 ) ? HIGH : LOW);
            digitalWrite( 7, ( (data & (1<<5) ) !=0 ) ? HIGH : LOW);
            digitalWrite( 8, ( (data & (1<<6) ) !=0 ) ? HIGH : LOW);
            digitalWrite( 9, ( (data & (1<<7) ) !=0 ) ? HIGH : LOW);

            /* 
             * BDIR BC2 BC1
             *    0   1   0   IAB (INACTIVE)
             */
            digitalWrite(11, HIGH); // BC2
            delayMicroseconds(1);   // t_DS = 50 ns < 1 us

            /* 
             * BDIR BC2 BC1
             *    1   1   0   DWS (WRITE TO PSG)
             */
            digitalWrite(10, HIGH); // BDIR
            delayMicroseconds(2);   // T_DW = 1800 ns < 2 us

            /* 
             * BDIR BC2 BC1
             *    0   1   0   IAB (INACTIVE)
             */
            digitalWrite(10, LOW);  // BDIR
            delayMicroseconds(1);   // T_DH = 100 ns < 1 us

            /* 
             * BDIR BC2 BC1
             *    0   0   0   NACT (INACTIVE)
             */
            digitalWrite(11, LOW);  //BC2
        }
    };
}

#endif/*PSGINO_SAMPLE_DRIVERS_AY_3_8910_H*/
