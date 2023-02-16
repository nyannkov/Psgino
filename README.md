# Psgino

Psgino is a library for controlling PSG with MML.
This document briefly describes the classes provided by this library and how to use them. 
For MML description rules, see [MML.md](/MML.md).

## Usage

### Psgino class

A sample of playing MML using Psgino is described below.

```c
/*
  * This is an example when the system clock of PSG is 2.097152 MHz.
  */
Psgino psgino = Psgino(psg_write, 2097152);

void setup() {

     pin_config();
  
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
```

Pass the pointer to the PSG write function and the PSG system clock frequency (unit: Hz) to the Psgino constructor.
The write function interface is:
```c
void (*p_write)(uint8_t addr, uint8_t data);
```
Here, "addr" represents the address of the PSG register, and "data" represents the value to be written to the register.
Psgino uses this function to write values to PSG registers.

The MML to be played can be set with the SetMML() method. Execute the Play() method to start playing. 
Conversely, you can stop playing with the Stop() method. Also, the current playing status can be obtained with the GetStatus() method. There are three playing states:
|State name|Description|
|--|--|
|PlayStop|MML playback is stopped.|
|Playing|Playing MML. |
|PlayEnd|The playback of MML is completed and stopped. |

These values are defined as enum PlayStatus types. In either of these states, when the Play() method is called, the MML is played from the beginning.

Note that this library requires the Proc() method to be called at regular intervals, as in the sample above. The default call frequency is 100 Hz. If you wish to change the frequency, enter the value you wish to set in Hz in the third argument ("proc_freq") of Psgino. However, if the value of the calling frequency is too large or too small, it may not work properly.

### PsginoZ class

PsginoZ class inherits the Psgino class and adds a function that can output sound effects at any time.
The PsginoZ class is suitable for playing sound effects during BGM playback, such as game programming.

The following code is an example of generating a sound effect when the signal logic of the pulled up pins 14 and 15 is set to LOW.

```c
PsginoZ psgino_z = PsginoZ(psg_write, 2097152);

unsigned long time0;

void setup() {

     pin_config();
  
     psgino_z.SetMML(mml);

     psgino_z.Play();

     time0 = millis();
}

void loop() {
    
     /* Call the Proc() method in 10 ms cycles. */
     if ( (millis() - time0) >= 10 ) {

         time0 = millis();

         psgino_z.Proc();
     }

     if ( digitalRead(14) == LOW ) {

         psgino_z.SetSeMML(mml_se1);
         psgino_z.PlaySe();

     } else if ( digitalRead(15) == LOW ) {

         psgino_z.SetSeMML(mml_se2);
         psgino_z.PlaySe();

     } else {
     }
}
```

The MML of the sound effect to be played can be set with the SetSeMML() method. The usage is the same as the SetMML() method, but please note that the MML that can be read is only single notes, not triads.
PlaySe() and StopSe() methods can be used to start and stop playing sound effects, respectively. The playing status of the sound effect can be obtained with GetSeStatus().This method, like the GetStatus() method, returns a value of type enum PlayStatus.

Note that PsginoZ uses the C channel of PSG (ch=2 in the source code) for sound effect generation. Therefore, if a sound effect is generated when three channels (A, B and C) are in use during BGM playback, the BGM will temporarily play on two channels (A and B).

## License

MIT License.
