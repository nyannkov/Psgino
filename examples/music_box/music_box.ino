/*
 * MIT License, see the LICENSE file for details.
 *
 * Copyright (c) 2023 nyannkov
 */
#include <Arduino.h>
#include <Psgino.h>

/*
 * Please define one of the following macros depending on the PGS to be used.
 */
//#define     AY_3_8910
#define     YMZ294

/*
 * Funiculì funiculà
 *
 * Music by Luigi Denza
 */
const char mml[] = 
    "T130"
    "[2"
        "$E1$A0$H100$D100$S90$F2500"
        "$M0$J6$L80$T0"
        "V15L4O5" 
        "E" "D8R8" "E" "$M0D8R8" "F8.E16D8.F16E2"
        "E" "D8R8" "E" "$M0D8R8" "F8.E16D8.F16C8.<"
        "$M0$E1$F600A16A8R16A16A8.A16A8R16A16A8.A16A8R16A16A8.A16A8R16A16>$M1$T4$E0F2"
        "$E1$F2500G8.F16D8.F16C8.<A16A8.A#16>C8.<A#16A8.G16F2"
    "],"
    "T130"
    "[2"  
        "S0M15000L4O4"
        "G8.G16G8.G16 G8.G16G8.G16 G8.G16G8.G16 G8.G16G8.G16"
        "G8.G16G8.G16 G8.G16G8.G16 G8.G16G8.G16 F8.F16F8.F16"
        "M30000D2C#2C2A2"
        "M15000B-8.B-16G8.G16 F8.C16C8.C16 E8.C16C8.C16 <A2"
    "],"
    "T130"
    "[2" 
        "$E1$A0$H100$D100$S90$F2500"
        "$J4$L65$T0"
        "V15L4O5"
        "C" "<B8>R8" "C" "<B8R8" ">D8.C16<B8.>D16C2"
        "C" "<B8>R8" "C" "<B8R8" ">D8.C16<B8.>D16<A8."
        "$M0$E1$F600F16F8R16F16F8.F16F8R16F16F8.F16F8R16F16F8.F16F8R16F16>$M1$T4$E0D2"
        "$E1$F2500E8.D16<B-8.>D16<A8.F16F8.G16G8.G16F8.E16C2"
    "]"
    ;

void pin_config();
void psg_write(uint8_t addr, uint8_t data);

/*
 * This is an example when the system clock of PSG is 2.097152 MHz.
 */
Psgino psgino = Psgino(psg_write, 2097152);

unsigned long time0;

void setup() {

    pin_config();

    psgino.Reset();
  
    psgino.SetMML(mml);

    psgino.Play();

    time0 = millis();
}

void loop() {
    
    /* Call the Proc() method in 10 ms cycles. */
    if ( (millis() - time0) >= 10 ) {
        time0 = millis();

        psgino.Proc();
    }
}

#if defined(AY_3_8910)
void pin_config() {

    /*
     * For the sound effect play buttons.
     */
    pinMode(14, INPUT);     /* Button 1 */
    pinMode(15, INPUT);     /* Button 2 */

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

void psg_write(uint8_t addr, uint8_t data) {

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

#elif defined(YMZ294)
void pin_config() {

    /*
     * For the sound effect play buttons.
     */
    pinMode(14, INPUT);     /* Button 1 */
    pinMode(15, INPUT);     /* Button 2 */

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

void psg_write(uint8_t addr, uint8_t data) {

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
#else
#error Choose the PSG to use by defining the macro for it.
#endif
