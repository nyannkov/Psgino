# Table of Contents

- [MML commands](#mml-commands)
- [Header section](#header-section)

## MML Commands

This document describes MML commands for Psgino.

| Command Name | Description |
|--------------|-------------|
| [A-G](#a-g-accidental-length-dot) | Outputs the sound of the specified note. |
| [N](#n-note-number-dot) | Specifies the note number and outputs the sound. |
| [R](#r-length-dot) | Inserts a rest. |
| [X](#x-length-dot) | Shortens the length of the following note or rest. |
| [T](#t-tempo) | Sets the tempo. |
| [L](#l-length-dot) | Sets the note length. |
| [V](#v-volume) | Sets the volume. |
| [S](#s-shape) | Switches to envelope generator volume control. |
| [M](#m-frequency) | Sets the envelope frequency. |
| [O](#o-octave) | Specifies the octave for notes A-G. |
| [Q](#q-gate-time) | Specifies the gate time of the note. |
| [H](#h-length-dot) | Outputs a noise sound. |
| [I](#i-period) | Sets the noise period. |
| [J](#j-np_start-np_end-dot) | Outputs a noise sound with the NP. This command can sweep the period of the noise. |
| [&lt;](#lt) | Decreases the octave value set by the O command. |
| [&gt;](#gt) | Increases the octave value set by the O command. |
| [,](#comma) | Concatenates MML into a chord. |
| [&](#-octave-settings) | Plays notes with slurs and ties. |
| [[],\|](#-loop-number----) | Specifies a loop section. |
| [$E](#e-enabled) | Toggles software envelope volume control on/off. |
| [$U](#u-time-unit) | Sets the time unit for the parameters specified by $A, $H, $D, $F, and $R. |
| [$A](#a-attack) | Sets the attack time of the software envelope. |
| [$H](#h-hold) | Sets the hold time of the software envelope. |
| [$D](#d-decay) | Sets the decay time of the software envelope. |
| [$S](#s-sustain) | Sets the sustain time of the software envelope. |
| [$F](#f-fade) | Sets the fade time of the software envelope. |
| [$R](#r-release) | Sets the release time of the software envelope. |
| [$M](#m-mode) | Sets the software LFO mode. |
| [$J](#j-depth) | Sets the modulation depth of the software LFO. |
| [$V](#v-speed-unit) | Sets the speed unit for the parameter specified by $L. |
| [$L](#l-low-frequency) | Sets the modulation frequency of the software LFO. |
| [$T](#t-delay-dot) | Specifies the delay time before the software LFO starts operating after sound output begins. |
| [$B](#b-bias) | Biases the frequency of the output sound. |
| [$O](#o-tp-ofs) | Sets the TP offset. |
| [$P](#p-pitchbend-level) | Smoothly increases or decreases the frequency of the output sound until it stops. |
| [@C](#c-data) | Invokes the user-defined callback function. |

### Basic command

#### A-G [&lt;accidental&gt;] [&lt;length&gt;] [&lt;dot&gt;]

Outputs the sound of the specified note.

| Values           | Description |
|------------------|-------------|
| &lt;accidental&gt; | Specify accidental symbols using `#`, `+`, or `-`. `#` and `+` raise the note by a semitone, while `-` lowers it by a semitone. |
| &lt;length&gt;     | Specify the length of the sound, ranging from 0 to 64. `1` represents a whole note, `4` represents a quarter note. If &lt;length&gt; is omitted, the note length will be set to the value specified by the L command. You can also specify `0` as the note length, in which case no sound is generated. This value is primarily used for the final note in a slur (&) sequence. |
| &lt;dot&gt;        | Specify a dot with `.`. One dot increases the note length by 1.5 times (= 1 + 0.5). Two dots increase it by 1.75 times (= 1 + 0.5 + 0.25). Up to three dots can be used. |

**Example:**
```
C#2.
```

#### N &lt;note-number&gt; [&lt;dot&gt;]

Specify the note number and output the sound. The note length is determined by the value set with the L command.

| Values             | Description |
|--------------------|-------------|
| &lt;note-number&gt; | Specify the note number within the range of 0 to 95. For example, N36 corresponds to O4C, and N52 corresponds to O5E. Note that N0 is treated as a rest, not O1C. |
| &lt;dot&gt;         | Specifies a dot. The dot effect works the same as for A-G notes. |

**Example:**
```
L4O4CEG.R8 N36N40N43.R8
```

#### R [&lt;length&gt;] [&lt;dot&gt;]

Insert a rest.

| Values           | Description |
|------------------|-------------|
| &lt;length&gt;     | Specify the length of the rest, ranging from 1 to 64. `1` represents a whole rest, `4` represents a quarter rest. If this value is omitted, the rest length is determined by the `mode` parameter of the SetMML/SetSeMML() method as follows:<br> - If the 0th bit of mode is 0, the rest length is 4 (quarter rest).<br> - If the 0th bit of mode is 1, the rest length will be the value specified by the L command. |
| &lt;dot&gt;        | Specifies a dot. The dot effect works the same as for A-G notes. |


**Example:**
```
L4
CR8 C16R16 CR
CR8 C16R16 CR
```

#### X &lt;length&gt; [&lt;dot&gt;]

Shortens the length of the sound or rest immediately following this command by the value specified in &lt;length&gt;. The minimum value is 0, and the maximum value is 128. The default value is 0. When 0 is specified, no shortening occurs.

For example, `X16R8` would change the `R8` rest to `R16`.

| Values           | Description |
|------------------|-------------|
| &lt;length&gt;     | Specifies the amount of length to subtract from the following note or rest. Values range from 0 to 128, where `1` represents a whole note, and `4` represents a quarter note. |
| &lt;dot&gt;        | Specifies a dot. The dot effect is the same as for A-G notes. |

**Example:**
```
$E1$A0$H0$D0$S100$F1000$R100      C  X    R  C R,
$E1$A0$H0$D0$S100$F1000$R100 R32  E  X32  R  E R,
$E1$A0$H0$D0$S100$F1000$R100 R16  G  X16  R  G R
```

#### T &lt;tempo&gt;

Sets the tempo. The default tempo is 120 bpm.

| Values       | Description |
|--------------|-------------|
| &lt;tempo&gt; | Specify a tempo value between 10 and 1000. When using multiple tone channels, tempo settings must be included in the MML for each channel. |

**Example:**
```
T120CDE2T100CDE2,
T120EGB2T100EGB2
```

#### L &lt;length&gt; [&lt;dot&gt;]

Sets the length of the sound when &lt;length&gt; is omitted in the A-G command. The default value is 4. This setting can also affect the R and H commands, depending on the `mode` parameter of the SetMML/SetSeMML() function.

| Values           | Description |
|------------------|-------------|
| &lt;length&gt;     | Specify the length of the sound from 1 to 128. `1` represents a whole note, and `4` represents a quarter note. |
| &lt;dot&gt;        | Specifies a dot. The dot effect is the same as for A-G notes. |


**Example:**
```
L4CDE2L16CDECDECDE
```

#### V &lt;volume&gt;

Sets the volume for tones and noises. The default volume is 15. Note that the V command and S command are mutually exclusive, and the volume setting will follow the command that is executed last.

| Values         | Description |
|----------------|-------------|
| &lt;volume&gt; | Specifies the volume level from 0 to 15. |


**Example:**
```
V15A V13A V10A
```

#### S &lt;shape&gt;

Switches to volume control using an envelope generator. The default setting is OFF. Executing this command enables volume control by the envelope generator. To turn off this volume control, use the V command.

| Values       | Description |
|--------------|-------------|
| &lt;shape&gt; | Specifies the envelope shape within the range of 0 to 15. Please refer to the PSG datasheet for details on the envelope shape for each value. |

**Example:**
```
V15CDER4
L4S0M3000CDER4
V15CDER4
```

#### M &lt;frequency&gt;

Specifies the envelope frequency. The default envelope frequency is 0.

| Values          | Description |
|-----------------|-------------|
| &lt;frequency&gt; | Specifies the envelope frequency (EP) in the range of 0 to 65535. |

**Example:**
```
L4
S0M5000CDER4
S0M1000CDER4
```

#### O &lt;octave&gt;

Specifies the octave for notes indicated by A-G. The default octave is 4.

| Values         | Description |
|----------------|-------------|
| &lt;octave&gt; | Specifies the octave, ranging from 1 to 8. |

**Example:**
```
L4
O4 CDE2
O5 CDE2
O3 CDE2
```

#### Q &lt;gate-time&gt;

Specifies the note gate time. The default value is 8 (which corresponds to 100%).

| Values           | Description |
|------------------|-------------|
| &lt;gate-time&gt; | Specifies the gate time, ranging from 1 to 8. For example, if the gate time is set to 3, the note duration will be 3/8, and the remaining 5/8 will be muted. |

**Example:**
```
L16CCCCCCCC
R4
Q4CCCCCCCC
```

#### H [&lt;length&gt;] [&lt;dot&gt;]

Outputs a noise sound with the frequency set by the I command.

| Values           | Description |
|------------------|-------------|
| &lt;length&gt;     | Specifies the length of the noise, ranging from 1 to 64. If this value is omitted, the length of the noise is determined by the `mode` parameter of the SetMML/SetSeMML() method, similar to the R command. |
| &lt;dot&gt;        | Specifies a dot. The dot effect is the same as for A-G notes. |

**Example:**
```
HR8H16R16HR HR8H16R16HR
```

#### I &lt;period&gt;

Sets the noise period (NP) for the noise generated by the H command. The default noise period is 16.

| Values          | Description |
|-----------------|-------------|
| &lt;period&gt; | Specifies the noise period, ranging from 0 to 31. |

**Example:**
```
I0H4 I8H4
```

#### J [&lt;np_start&gt;] [~&lt;np_end&gt;] [&lt;dot&gt;]

Outputs a noise sound with the NP. This command can sweep the period of the noise. The length of the sound follows the value set by the L command.

| Values          | Description |
|-----------------|-------------|
| &lt;np_start&gt; | Specifies the noise period, ranging from 0 to 31. If this value is omitted, the noise period will follow the value specified by the I command.|
| ~ &lt;np_end&gt; | Specifies the completion value of the noise period for the sweep, ranging from 0 to 31. If this value is omitted, the noise sweep will not be executed.|
| &lt;dot&gt;        | Specifies a dot. The dot effect is the same as for A-G notes. |

**Example1:**
```
I0
L4 J0 J15 J31
L8 J0 J15 J31
```

**Example2:**
```
L4
I0  J0~31 R J~31 R
I31 J0~31 R J~31 R
```

#### &lt;(LT)

Lowers the octave set by the O command.

**Example:**
```
L4O5C<BA
```

#### &gt;(GT)

Raises the octave set by the O command.

**Example:**
```
L4AB>C
```

#### ,(COMMA)

MML for each part can be concatenated using commas. Since PSG has 3 tone channels, up to 3 MMLs can be concatenated.

**Example:**
```
T120L4O4CEG,
T120L4O4EGB,
T120L4O4GB>D
```

#### & [&lt;octave-settings&gt;]

By connecting A-G commands with &, you can create slurs and ties.

| Values            | Description |
|-------------------|-------------|
| &lt;octave-settings&gt; | You can use the O command, along with &gt; and &lt;, to set the octave. The octave value set here will persist even after the performance of slurs and ties. |

**Example:**
```
A2R2 A4&A4R2 A2&>A0R2 A2&<A0R2
```

#### _[_ [&lt;loop-number&gt;] ... [|] ... _]_

Loop playback of MML is achieved using []. Loops can be nested up to 3 levels. Loop symbols beyond the 3rd level are ignored. The `|` symbol can also be inserted within the loop section to serve as a break statement. This symbol functions as a break statement only for the last loop.

**NOTE:**
The primary loop of MML registered with the SetMML() method can be reduced to two remaining loop iterations (including the current loop) by calling the FinishPrimaryLoop() method. This method is used to end background music such as end credits.

| Values            | Description |
|-------------------|-------------|
| &lt;loop-number&gt; | Specifies the number of loop iterations, ranging from 0 to 255. If 0 is specified, it creates an infinite loop. If this value is omitted, the loop count defaults to 1. |

**Example1:**
```
O3[3 C&>C0]

```
**Example2:**
```
[4 CD|ER4] CR4

```

## Software envelope generator

In addition to PSG's built-in envelope generator, Psgino supports volume control using software envelopes. This feature is only available when volume is controlled with the V command rather than the S command. The envelope shape is configured using six parameters (the AHDSFR method). Volume control via envelopes can be set independently for each channel.

#### $E &lt;enabled&gt;

Sets the ON/OFF state for volume control via software envelope. This value defaults to OFF.

| Values     | Description                           |
|------------|---------------------------------------|
| &lt;enabled&gt; | Set in the range from 0 to 1. 0 corresponds to OFF and 1 to ON. |

**Example:**
```
V15L4O4
$U0$A0$H100$D100$S90$F2000$R300

$E0
CDER

$E1
CDER
```

#### $U &lt;time-unit&gt;

Sets the time unit for the parameters specified by $A, $H, $D, $F, and $R.

| Values          | Description |
|-----------------|-------------|
| &lt;time-unit&gt; | Specifies the time unit, ranging from 0 to 8192. The default value is 0. <br>• `0`: The values specified in $A, $H, $D, $F, and $R are interpreted as milliseconds. <br>• `1-8192`: The time unit is set to note-length. For example, if $U4 is specified, the values correspond to the length of a quarter note. |

**NOTE:**
To scale the envelope shape proportionally with tempo changes, specify the time unit in note-length.

**Example:**

```
V15L4O4
$E1
T120 $U0    $A0$H100$D100$S90$F2000$R300 CDER
T360 $U0    $A0$H100$D100$S90$F2000$R300 CDER

T120 $U2048 $A0$H100$D100$S90$F2000$R300 CDER
T360 $U2048 $A0$H100$D100$S90$F2000$R300 CDER
```

#### $A &lt;attack&gt;

Specifies the rise time of the sound, which is the time it takes for the volume to increase from 0 to the level set by the V command.

| Values           | Description |
|------------------|-------------|
| &lt;attack&gt; | Specifies the rise time, ranging from 0 to 10000. If 0 is specified, no rise time processing is applied, and the sound is output directly at the volume set by the V command. |

#### $H &lt;hold&gt;

Sets the retention time of the volume level after the attack phase.

| Values        | Description |
|---------------|-------------|
| &lt;hold&gt; | Specifies the hold time, ranging from 0 to 10000. If 0 is specified, decay processing will start immediately. |

#### $D &lt;decay&gt;

Sets the time it takes for the volume to reach the sustain level after the hold phase.

| Values        | Description |
|---------------|-------------|
| &lt;decay&gt; | Specifies the decay time, ranging from 0 to 10000. If 0 is specified, the volume is immediately set to the sustain value, and fade processing begins. |

#### $S &lt;sustain&gt;

Sets the target volume level for decay processing.

| Values         | Description |
|----------------|-------------|
| &lt;sustain&gt; | Specifies the sustain volume level as a percentage, ranging from 0 to 1500%. This percentage corresponds to the volume set by the V command. |

#### $F &lt;fade&gt;

Specifies the time it takes for the volume to fade from the sustain level to 0 after the decay process is complete.

| Values        | Description |
|---------------|-------------|
| &lt;fade&gt; | Specifies the fade time, ranging from 0 to 10000. If 0 is specified, the volume will remain at the sustain level. |

#### $R &lt;release&gt;

Specifies the time it takes for the volume to decrease from the level at note-off to 0. This effect is only applied when the command following the note-off is a rest.

| Values        | Description |
|---------------|-------------|
| &lt;release&gt; | Specifies the release time, ranging from 0 to 10000. If 0 is specified, the volume is immediately set to the 0. |

## Software LFO

Psgino features a software LFO. By enabling this function, you can add vibrato to the sound output.

#### $M &lt;mode&gt;

Sets the ON/OFF state for the software LFO. The default value is OFF.

| Values       | Description |
|--------------|-------------|
| &lt;mode&gt; | Sets the software LFO mode, with 0 corresponding to OFF and 1 to ON (modulated by a triangular wave). |

**Example:**
```
V15L4O4
$V0$J4$L80$T8

$M0
CDE2

$M1
CDE2
```

#### $J &lt;depth&gt;

Sets the modulation depth. The default modulation depth is 0.

| Values         | Description |
|----------------|-------------|
| &lt;depth&gt; | Specifies the modulation depth, ranging from 0 to 255. If the modulation depth is n, the frequency of the sound varies from 2^(-n/360) times to 2^(n/360) times, depending on the modulation function. |

#### $V &lt;speed-unit&gt;

Sets the speed unit for the parameter specified by $L.

| Values          | Description |
|-----------------|-------------|
| &lt;speed-unit&gt; | Specifies the speed unit, ranging from 0 to 8192. The default value is 0. <br>• `0`: The values specified in $L is interpreted as 0.1 Hz. <br>• `1-8192`: The time unit is the reciprocal of the value obtained by multiplying the note-length by 10. For example, if $V4 is specified and $L10 is used, modulation will occur 1.0 times within the duration of a quarter note. Similarly, if $V8 is specified and $L25 is used, modulation will occur 2.5 times within the duration of an eighth note.|

**NOTE:**
To scale the LFO frequency proportionally with tempo changes, specify the time unit in note-length.


#### $L &lt;low-frequency&gt;

Sets the modulation frequency. The default modulation frequency is 40.

| Values            | Description |
|-------------------|-------------|
| &lt;low-frequency&gt; | Specifies the modulation frequency, ranging from -200 to 200. If a value of 0 or greater is specified, the phase of the modulation waveform will be 180 deg. If the value is less than 0, the phase will be 0 deg.|

#### $T &lt;delay&gt; [&lt;dot&gt;]

Specifies the time from the start of sound output until the LFO starts operating. The default value is 0.

| Values            | Description |
|-------------------|-------------|
| &lt;delay&gt;    | Specifies the delay time for the LFO, ranging from 0 to 128. If the delay is non-zero, it uses the same calculations as the L command to determine the delay time. For example, if the delay is set to 4, the LFO will start operating after the time of one quarter note has passed. If the delay is 0, the LFO starts immediately. |
| &lt;dot&gt;      | Specifies a dot. The dot effect is the same as for A-G notes. |


### Other commands

#### $B &lt;bias&gt;

Biases the frequency of the output sound. The default value for bias is 0.

| Values      | Description |
|-------------|-------------|
| &lt;bias&gt; | Specifies the bias, ranging from -500 to 500. If the bias value is n, the frequency of the output sound will be multiplied by 2^(n/360). |


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

#### $O &lt;tp-ofs&gt;

Sets the TP offset. The default value for tp-ofs is 0.

|Values|Description|
|--|--|
|&lt;tp-ofs&gt;|Specify the tp-ofs in the range (-100) to 100. |


**Example:**
```
CDEFGAB>C,
$O1CDEFGAB>C
```

#### $P &lt;pitchbend-level&gt;

Smoothly increases or decreases the frequency of the output sound to the value specified by pitchbend-level until the output stops. The default value is 0.

| Values              | Description |
|---------------------|-------------|
| &lt;pitchbend-level&gt; | Specifies the pitch bend level, ranging from -2880 to 2880. If the specified value is n, the frequency of the output sound changes smoothly to a final value multiplied by 2^(n/360). |

**Example:**
```
V15L16O4
$P-360 CDEFGAB>C<
$P360 CDEFGAB>C<
```

#### @C &lt;data&gt;

Invokes a user-defined callback function. When this command is decoded, it synchronously invokes the registered callback function. The interface for the callback function is as follows:

```
void (*user_callback)(uint8_t ch, int32_t param);
```
Here, `ch` represents the channel number of the MML where this command is written, and `param` represents the parameter of this command.

The function registered with `SetUserCallback()` is executed for MML registered with `SetMML()`, while for MML registered with `SetSeMML()`, the function registered with `SetSeUserCallback()` is executed.

| Values   | Description |
|----------|-------------|
| &lt;data&gt; | Specifies the value to be stored in the parameter of the callback function as an `int32_t` type. Enclosing data in parentheses allows for input in C-style octal and hexadecimal formats (e.g., 010, 0xF). |

**Example:**
```
T120L4O4CE@C1234G,
T120L4O4EGB@C-999,
T120L4O4@C(0xF)G@C(010)B>D
```

## Header Section

To add a header at the beginning of the MML, you can do the following:

:V &lt;version-number&gt; [ [_header-command_] ... ];

The header starts with a colon and ends with a semicolon. If the header section is omitted, the MML is treated as version 1.

**NOTE:** Currently, there is only one version of MML, which is version 1.

### Header command

#### M &lt;mode-flags&gt;

This command sets the mode-flags for the MML. Each field of the flags is as follows:

| Bit | Field  | Description |
|-----|--------|-------------|
| 15-1 | -      | Reserved.    |
| 0   | RH_LEN | Switches the default value when the R or H command omits the note length specification. <br> - 0: The default value for note length is 4. <br> - 1: The default note length is the value specified with the L command. |

If this command is not executed, the mode flags will be set to the value specified by the argument `mode`.

**Example:**
```
:V1M1;
L4CR L8CR L16CH L32CH L64CH
```
