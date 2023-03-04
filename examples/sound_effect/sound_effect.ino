/*
 * MIT License, see the LICENSE file for details.
 *
 * Copyright (c) 2023 nyannkov
 */
#include <Arduino.h>
#include <Psgino.h>
#include <PsginoSampleDrivers.h>

/*
 * Please define one of the following macros depending on the PGS to be used.
 */
#define     USE_AY_3_8910
//#define     USE_YMZ294


#if defined(USE_AY_3_8910)
const auto& psg_driver = PsginoSampleDrivers::DriverAY_3_8910();
#elif defined(USE_YMZ294)
const auto& psg_driver = PsginoSampleDrivers::DriverYMZ294();
#else
#error Choose the PSG to use by defining the macro for it.
#endif

/*
 * This is an example when the system clock of PSG is 2.097152 MHz.
 */
PsginoZ psgino_z = PsginoZ(psg_driver.write, 2097152);

/*
 * Funiculì funiculà
 *
 * Music by Luigi Denza
 */
const char mml[] = 
    "T130"
    "[0"
        "$E1$A0$H100$D100$S80$F2300" "$M0$J6$L80$T0" "V15L4O5" 
        "[2" "ED8R8" "ED8R8" "F8.E16D8.F16|E2]C8.<"
        "$M0$E1$F600"
        "[4" "A16A8R16A16|A8.]>$E0$M1$T4F2"
        "$E1$F2300"
        "G8.F16D8.F16C8.<A16A8.A#16>C8.<A#16A8.G16F2"
    "],"
    "T130"
    "[0"  
        "$E1$A0$H100$D100$S80$F2000" "V13L4O4"
        "[7" "G8.G16G8.G16]F8.F16F8.F16"
        "$S100$F2000"
        "D2C#2C2A2"
        "$S80$F2000"
        "B-8.B-16G8.G16F8.C16C8.C16E8.C16C8.C16<A2"
    "],"
    "T130"
    "[0" 
        "$E1$A0$H100$D100$S80$F2300" "$M0$J4$L65$T0" "V15L4O5"
        "[2" "C<B8>R8" "C<B8R8" ">D8.C16<B8.>D16|C2]<A8."
        "$M0$E1$F600"
        "[4" "F16F8R16F16|F8.]>$E0$M1$T4D2"
        "$E1$F2300"
        "E8.D16<B-8.>D16<A8.F16F8.G16G8.G16F8.E16C2"
    "]"
    ;

const char mml_se1[] = "$E1$A0$H100$D100$S90$F2000" "I16H64I32H8";
const char mml_se2[] = "$E1$A0$H100$D100$S90$F1000" "O7L4$P-1800E";

unsigned long time0;

void pin_config() {

    /*
     * For the sound effect play buttons.
     */
    pinMode(14, INPUT);     /* Button 1 */
    pinMode(15, INPUT);     /* Button 2 */

    /*
     * PSG pin configuration.
     */
    psg_driver.pin_config();
}

void setup() {

    pin_config();

    psgino_z.Reset();
    
    psgino_z.SetMML(mml);

    psgino_z.Play();

    time0 = millis();
}

void loop() {
    
    /* Call the Proc() method in 10 ms cycles. */
    if ( (millis() - time0) >= 10 ) {

        time0 = millis();

        if ( digitalRead(14) == LOW ) {

            psgino_z.SetSeMML(mml_se1);
            psgino_z.PlaySe();

        } else if ( digitalRead(15) == LOW ) {

            psgino_z.SetSeMML(mml_se2);
            psgino_z.PlaySe();

        } else {
        }

        psgino_z.Proc();
    }
}
