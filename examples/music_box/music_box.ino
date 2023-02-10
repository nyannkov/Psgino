#include <arduino.h>
#include <Psgino.h>

const char mml[] = 
    "T93" "[2" "$E1$A0$H100$D100$S90$F2500" "$M1$J1$L40$T8."       "V15L4O5" "A#>C8<A#8GD#F.G8F2" "CD#8C8<A#>D#GA#8G8F2" "G.G8GF8G8A#.A#8>C<A#G.A#8GFD#1" "],"
    "T93" "[2" "$E1$A0$H100$D100$S90$F4000" "$M1$J1$L40$T8."       "V12L4O5" "D#2<A#2>D1<G#2G2A#1" ">D#2D2C2<G#2A#2A#2G1" "],"
    "T93" "[2" "$E1$A0$H100$D100$S90$F4000" "$M1$J1$L40$T8." "$B2" "V12L4O5" "D#2<A#2>D1<G#2G2A#1" ">D#2D2C2<G#2A#2A#2G1" "]";

void pin_config();
void psg_write(uint8_t addr, uint8_t data);

/* In this example, the PSG system clock is 2.097152 MHz. */
PsginoZ psgino = PsginoZ(psg_write, 2097152);
unsigned long time0;

void setup() {

    pin_config();
  
    psgino.SetMML(mml);

    psgino.Play();

    time0 = millis();
}

void loop() {
    
    if ( (millis() - time0) >= 10 )
    {
        time0 = millis();
        /* Call the Proc() method in 10 ms cycles. */
        psgino.Proc();
    }
}

void pin_config() {

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
