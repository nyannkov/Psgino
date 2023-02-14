# MML commands

This document describes MML commands for Psgino.

|Command name|Summary|
|--|--|
|[A-G](#a-g-accidental-length-dot)|Outputs the sound of the specified note name. |
|[N](#n-note-number-dot)|Specify the note number and output the sound. |
|[R](#r-length-dot)|Insert a rest. |
|[T](#t-tempo)|Sets the tempo. |
|[L](#l-length-dot)|Sets the length of the note. |
|[V](#v-volume)|Sets the volume. |
|[S](#s-shape)|Switch to envelope generator volume control. |
|[M](#m-frequency)|Specifies the envelope frequency. |
|[O](#o-octave)|Specifies the octave of the note specified by A-G. |
|[Q](#q-gate-time)|Specify the note gate time. |
|[H](#h-length-dot)| Outputs noise sound. |
|[I](#i-frequency)|Sets the frequency of the noise. |
|[&lt;](#lt)|Decrease the octave value set by O-command. |
|[&gt;](#gt)|Increase the octave value set by O-command. |
|[,](#comma)|MML is concatenated into a chord. |
|[&](#-octave-settings)|Plays like slurs and ties. |
|[[]](#-loop-number--)|Specify the loop section. |
|[$E](#e-enabled)|Set ON/OFF of volume control by software envelope. |
|[$A](#a-attack)|Specifies the attack time of the software envelope. |
|[$H](#h-hold)|Specifies the hold time of the software envelope. |
|[$D](#d-decay)|Specifies the software envelope decay time. |
|[$S](#s-sustain)|Specifies the sustain time of the software envelope. |
|[$F](#f-fade)|Specifies the fade time of the software envelope. |
|[$M](#m-mode)|Sets the software LFO mode. |
|[$J](#j-depth)|Specifies the modulation depth of the software LFO. |
|[$L](#l-low-frequency)|Sets the modulation frequency of the software LFO. |
|[$T](#t-delay-dot)|Specifies the time from the start of sound output until the software LFO operates. |
|[$B](#b-bias)|Bias the frequency of the output sound. |
|[$P](#p-pitchbend-level)|Smoothly increases or decreases the frequency of the output sound until output stops. |

## Basic command

### A-G [&lt;accidental&gt;] [&lt;length&gt;] [&lt;dot&gt;]

Outputs the sound of the specified note name.

|Values|Description|
|--|--|
|&lt;accidental&gt;|Specify the accidental symbols with "#", "+", and "-". "#" and "+" correspond to semitone up, and "-" correspond to semitone down.|
|&lt;length&gt;|Specify the length of the sound in the range from 0 to 64. 1 represents a whole note, 4 represents a quarter note. If you omit &lt;length&gt;, the note length will be the value specified by the L command. It is also possible to specify 0 as the length of the note. In this case, no sound is generated. This value is primarily used for the final tone of a slur (&) string.|
|&lt;dot&gt;|Specify the dot with ".". If you write one dot, the length of the sound will be 1.5 times (=1+0.5). If you write two, the length of the sound will be 1.75 times (= 1 + 0.5 + 0.25). Up to 3 dots can be described.|

**Example:**
```
C#2.
```

### N &lt;note-number&gt; [&lt;dot&gt;]

Specify the note number and output the sound. The note length follows the value specified with the L command.

|Values|Description|
|--|--|
|&lt;note-number&gt;|Specify the note number value in the range from 0 to 95. For example, N36 corresponds to O4C and N52 corresponds to O5E. However, N0 is treated as a rest, not O1C.|
|&lt;dot&gt;|Specifies a dot. The dot effect is the same as for A-G.|

**Example:**
```
L4O4CEG.R8 N36N40N43.R8
```

### R [&lt;length&gt;] [&lt;dot&gt;]

Insert a rest.

|Values|Description|
|--|--|
|&lt;length&gt;|Specify the length of the rest in the range of 1 to 64. 1 represents a whole rest, 4 represents a quarter note rest. If this value is omitted, the rest length is determined by the value of the parameter "mode" of the method SetMML/SetSeMML() as follows:<br> - If the 0th bit of mode is 0, the rest length is 4 (quarter rest).<br> - If the 0th bit of mode is 1, the length of the rest will be the value specified by the L command.|
|&lt;dot&gt;|Specifies a dot. The dot effect is the same as for A-G.|

**Example:**
```
L4
CR8 C16R16 CR
CR8 C16R16 CR
```

### H [&lt;length&gt;] [&lt;dot&gt;]

Outputs a noise sound with the frequency set by the I command.

|Values|Description|
|--|--|
|&lt;length&gt;|Specifies the length of the noise in the range 1 to 64. If this value is omitted, the length of the noise is determined by the value of the parameter "mode" of the method SetMML/SetSeMML(), just like the R command.|
|&lt;dot&gt;|Specifies a dot. The dot effect is the same as for A-G.|

**Example:**
```
HR8H16R16HR HR8H16R16HR
```

### T &lt;tempo&gt;

Sets the tempo. Tempo defaults to 120 bpm.

|Values|Description|
|--|--|
|&lt;tempo&gt;|Specify a tempo value in the range 32 to 255. When performing using multiple tone channels, tempo settings must be written in the MML for each channel.|

**Example:**
```
T120CDE2T100CDE2,
T120EGB2T100EGB2
```

### L &lt;length&gt; [&lt;dot&gt;]

Sets the length of the sound when &lt;length&gt; is omitted in the A-G command. This value defaults to 4.
This value can also be applied to the R and H commands depending on the value of the parameter "mode" of the SetMML/SetSeMML() function.

|Values|Description|
|--|--|
|&lt;length&gt;|Specify the length of the sound from 1-64. 1 represents a whole note, 4 represents a quarter note.|
|&lt;dot&gt;|Specifies a dot. The dot effect is the same as for A-G.|

**Example:**
```
L4CDE2L16CDECDECDE
```

### V &lt;volume&gt;

Sets the volume of tones and noises. Volume defaults to 15.
Note that the V command and S command operate exclusively, and the volume setting follows the command executed later.

|Values|Description|
|--|--|
|&lt;volume&gt;|Specifies the volume from 0 to 15.|

**Example:**
```
V15A V13A V10A
```

### S &lt;shape&gt;

Switch to volume control with an envelope generator. This value defaults to OFF.
Executing this command turns on the volume control by the envelope generator.
If you want to turn off this volume control, execute the V command.

|Values|Description|
|--|--|
|&lt;shape&gt;|Specifies the shape of the envelope in the range 0-15. Please refer to the PSG datasheet for the shape of the envelope for each value.|

**Example:**
```
V15CDER4
L4S0M3000CDER4
V15CDER4
```

### M &lt;frequency&gt;

Specifies the envelope frequency. Envelope frequency defaults to 0.

|Values|Description|
|--|--|
|&lt;frequency&gt;|Specifies the envelope frequency (EP) in the range 0 to 65535. |

**Example:**
```
L4
S0M5000CDER4
S0M1000CDER4
```

### O &lt;octave&gt;

Specifies the octave of the note specified by A-G. Octave defaults to 4.

|Values|Description|
|--|--|
|&lt;octave&gt;|Specify an octave in the range 1 to 8.|

**Example:**
```
L4
O4 CDE2
O5 CDE2
O3 CDE2
```

### Q &lt;gate-time&gt;

Specifies the note gate time. The default value is 8 (8/8=100%).

|Values|Description|
|--|--|
|&lt;gate-time&gt;|Specify the gate time in the range of 1 to 8. For example, if the gate time is set to 3, the note duration will be 3/8 and the remaining 5/8 will be muted.|

**Example:**
```
L16CCCCCCCC
R4
Q4CCCCCCCC
```

### I &lt;frequency&gt;

Sets the noise frequency (NP) generated by the H command. The noise frequency defaults to 16.

|Values|Description|
|--|--|
|&lt;frequency&gt;|Specifies the noise frequency in the range 0 to 31.|

**Example:**
```
I0H4 I8H4
```

### &lt;(LT)

Lowers the octave specified by the O command.

**Example:**
```
L4O5C<BA
```

### &gt;(GT)

Raises the octave specified by the O command.

**Example:**
```
L4AB>C
```

### ,(COMMA)

MML of each part can be concatenated with commas. Since PSG has 3 tone channels, up to 3 MMLs can be concatenated.

**Example:**
```
T120L4O4CEG,
T120L4O4EGB,
T120L4O4GB>D
```

### & [&lt;octave-settings&gt;]

By connecting A-G commands with &, you can perform slurs and ties.

|Values|Description|
|--|--|
|&lt;octave-settings&gt;|You can use the O command, &gt; and &lt; to set the octave. The octave value set here will continue even after the performance of slurs and ties.|

**Example:**
```
A2R2 A4&A4R2 A2&>A0R2 A2&<A0R2
```

### [ [&lt;loop-number&gt;] ... ]

Loop playback of MML in []. Loops can be nested up to 3 levels. Loop symbols after the 4th row are ignored.

|Values|Description|
|--|--|
|&lt;loop-number&gt;|Specify the loop count in the range from 0 to 255. However, if 0 is specified, it will be an infinite loop. If this value is omitted, the loop count will be 1.|

**Example:**
```
[3
  [2
    L4CDER4
  ]
  L4GEDCDEDR4
]
```

## Software envelope generator

In addition to PSG's built-in envelope generator, Psgino also supports volume control using software envelopes.
This feature can only be enabled if the volume is controlled with the V command rather than the S command.
The shape of the envelope is set with 5 parameters (AHDSF method).
This envelope volume control can be set independently for each channel.

### $E &lt;enabled&gt;

Sets ON/OFF of volume control by software envelope. This value defaults to OFF.

|Values|Description|
|--|--|
|&lt;enabled&gt;|Set in the range from 0 to 1. 0 corresponds to OFF and 1 to ON.|

**Example:**
```
V15L4O4
$A0$H100$D100$S90$F2000

$E0
CDE2

$E1
CDE2
```

### $A &lt;attack&gt;

Specifies the rise time of the sound (the arrival time from 0 to the volume set by the V command).

|Values|Description|
|--|--|
|&lt;attack&gt;|Specify in the range from 0 to 10000. The unit is ms. If 0 is specified, sound rise processing is not performed, and sound is output at the volume set with the V command.|

### $H &lt;hold&gt;

Sets the retention time of the volume level after attack.

|Values|Description|
|--|--|
|&lt;hold&gt;|Specify in the range from 0 to 10000. The unit is ms. If 0 is specified, Decay processing will start immediately.|

### $D &lt;decay&gt;

Sets the time it takes for the volume to reach the Sustain value after Hold.

|Values|Description|
|--|--|
|&lt;decay&gt;|Specify in the range from 0 to 10000. The unit is ms. If 0 is specified, the volume is immediately set to the Sustain value and fade processing is started.|

### $S &lt;sustain&gt;

Sets the target volume level for Decay processing.

|Values|Description|
|--|--|
|&lt;sustain&gt;|Specify in the range from 0 to 100. The unit is %. Corresponds to the percentage of the volume specified by the V command.|

### $F &lt;fade&gt;

Specifies the time from the Sustain value to 0 after the Decay process is complete.

|Values|Description|
|--|--|
|&lt;fade&gt;|Specify in the range from 0 to 10000. The unit is ms. However, if 0 is specified, the volume will keep the Sustain value.|

## Software LFO

Psgino has a software LFO. By enabling this function, you can output a sound with vibrato.

### $M &lt;mode&gt;

Sets ON/OFF for the software LFO. This value defaults to OFF.

|Values|Description|
|--|--|
|&lt;mode&gt;|Sets the software LFO mode from 0 to 1. 0 corresponds to OFF and 1 to ON (modulated by triangular wave).|

**Example:**
```
V15L4O4
$J4$L80$T8

$M0
CDE2

$M1
CDE2
```

### $J &lt;depth&gt;

Sets the modulation depth. Modulation depth defaults to 0.

|Values|Description|
|--|--|
|&lt;depth&gt;|Specifies the modulation depth in the range 0 to 360. If the modulation depth is n, the frequency of the sound varies from 2^(-n/360) times to 2^(n/360) times depending on the modulation function.|

### $L &lt;low-frequency&gt;

Sets the modulation frequency. The default modulation frequency is 40 ( = 4.0 Hz).

|Values|Description|
|--|--|
|&lt;low-frequency&gt;|Specifies the modulation frequency in the range 0 to 200. The unit is 0.1 Hz.|

### $T &lt;delay&gt; [&lt;dot&gt;]

Specifies the time from the start of sound output until the LFO operates. This value defaults to 0.

|Values|Description|
|--|--|
|&lt;delay&gt;|Specifies the delay time of the LFO in the range from 0 to 64. If delay is non-zero, it uses the same calculations as the L command to determine the delay time. For example, if the delay is set to 4, the LFO operation will start after the time of one quarter note has passed after the start of sounding. If delay is 0, the delay time will be 0.|
|&lt;dot&gt;|Specifies a dot. The dot effect is the same as for A-G.|


## Other commands

### $B &lt;bias&gt;

Bias the frequency of the output sound. The default value for bias is 0.

|Values|Description|
|--|--|
|&lt;bias&gt;|Specify the bias in the range (-500) to 500. If the bias value is n, the frequency of the output sound will be the value multiplied by 2^(n/360).|


**Example1:**
```
V15L4O4
CDE2
$B30 CDE2
$B60 CDE2
$B360 CDE2
```

**Example2:**
```
V15L1O4
AA
,
V15L1O4
A$B1A
```

### $P &lt;pitchbend-level&gt;

Smoothly increases or decreases the frequency of the output sound to the value specified by pitchbend-level until the output stops. This value defaults to 0.

|Values|Description|
|--|--|
|&lt;pitchbend-level&gt;|Specifies the pitch bend level in the range from (-2880) to 2880. If the specified value is n, the frequency of the output sound changes smoothly up to the final value multiplied by 2^(n/360).|

**Example:**
```
V15L16O4
$P-360 CDEFGAB>C<
$P360 CDEFGAB>C<
```

