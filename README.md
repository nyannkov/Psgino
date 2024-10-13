# Psgino

Psgino is a library for controlling PSG (AY-3-8910, YMZ294, etc.) with MML.
This document briefly describes the classes provided by this library and how to use them. 
For MML description rules, see [MML.md](/MML.md).

## Usage

### Psgino class

A sample of playing MML using Psgino is described below.

```c
/*
  * This is an example when the system clock of PSG is 2.097152 MHz.
  */
Psgino psgino;

unsigned long time0;

void setup() {

    pin_config();

    psgino.Initialize(psg_write, 2097152);

    psgino.Reset();

    psgino.SetMML(mml);

    psgino.Play();

    time0 = micros();
}

void loop() {

    unsigned long time_now = micros();

    /* Call the Proc() method in 10,000 us cycles. */

    if ( (time_now - time0) >= (10*1000) ) {

        time0 = time_now;

        psgino.Proc();
    }
}
```

Pass the pointer to the PSG write function and the PSG system clock frequency (unit: Hz) to the `Psgino` constructor.
The write function interface is:
```c
void (*p_write)(uint8_t addr, uint8_t data);
```
Here, `addr` represents the address of the PSG register, and `data` represents the value to be written to the register.
`Psgino` uses this function to write values to PSG registers.

The MML to be played can be set with the `SetMML()` method. Execute the `Play()` method to start playing. 
Conversely, you can stop playing with the `Stop()` method. Also, the current playing status can be obtained with the `GetStatus()` method. There are three playing states:
|State name|Description|
|--|--|
|PlayStop|MML playback is stopped.|
|Playing|Playing MML. |
|PlayEnd|The playback of MML is completed and stopped. |

These values are defined as enum `PlayStatus` types. In either of these states, when the `Play()` method is called, the MML is played from the beginning.

Note that this library requires periodic calls to the `Proc()` method, as in the sample above. The default frequency is 100 Hz. If you wish to change the frequency, enter the desired value in Hz in the third argument `proc_freq` of Psgino constructor. The resolution of note lengths is determined by this value. For example, if the tempo is 120 bpm, a 32nd note would be 120 bpm * 32 = 2 bps * 32 = 64 Hz, which is less than 100 Hz, so it can be played accurately. However, for a 64th note, it would be 128 Hz, which means it cannot be played accurately.
If you want to play 64th notes at a tempo of 120 bpm, you need to set `proc_freq` to 128 Hz or higher and call the `Proc()` method at that frequency. Similarly, if you want to play 128th notes at 120 bpm, you need to set `proc_freq` to 256 Hz or higher.

### PsginoZ class

`PsginoZ` class inherits the Psgino class and adds a function that can output sound effects at any time.
The `PsginoZ` class is suitable for playing sound effects during BGM playback, such as game programming.

The following code is an example of generating a sound effect when the signal logic of the pulled up pins 14 and 15 is set to LOW.

```c
PsginoZ psgino_z;

unsigned long time0;

void setup() {

    pin_config();

    psgino_z.Initialize(psg_write, 2097152);

    psgino_z.Reset();

    psgino_z.SetMML(mml);

    psgino_z.Play();

    time0 = micros();
}

void loop() {

    unsigned long time_now = micros();

    /* Call the Proc() method in 10,000 us cycles. */

    if ( (time_now - time0) >= (10*1000) ) {

        time0 = time_now;

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
```

The MML of the sound effect to be played can be set with the `SetSeMML()` method. The usage is the same as the `SetMML()` method, but please note that the MML that can be read is only single notes, not triads.
`PlaySe()` and `StopSe()` methods can be used to start and stop playing sound effects, respectively. The playing status of the sound effect can be obtained with `GetSeStatus()`.This method, like the `GetStatus()` method, returns a value of type enum `PlayStatus`.

Note that `PsginoZ` uses the C channel of PSG (ch=2 in the source code) for sound effect generation. Therefore, if a sound effect is generated when three channels (A, B and C) are in use during BGM playback, the BGM will temporarily play on two channels (A and B).

## Demonstration

### Sound effect generation

This URL is a link to a video of an example ([sound_effect.ino](/examples/sound_effect)) running with the YMZ294 and Arduino UNO R4 Minima.
(The song played in the video is Funiculì funiculà, composed by Luigi Denza.)

https://github.com/nyannkov/Psgino/assets/54160654/5bbb3513-4fbd-44b6-9d54-eeb37ceb350f

### Example using PSG emulator

Currently publishing examples using a PSG emulator in the following repository:

https://github.com/nyannkov/misc_arduino_sketches

PSG emulation is performed on the Arduino Uno R4. If you don't have ICs like PSG or SSG, these examples might be helpful as a reference.

## License

MIT License.
