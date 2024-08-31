/*
 * MIT License, see the LICENSE file for details.
 *
 * Copyright (c) 2023 nyannkov
 */
#ifndef PSG_CTRL_H
#define PSG_CTRL_H

#include <stdint.h>
#include <stddef.h>

#pragma pack(1)
namespace PsgCtrl
{
    const uint8_t DEFAULT_MML_VERSION           = (1);
    const int16_t NUM_CHANNEL                   = (3);

    const int16_t CTRL_STAT_STOP                = (0);
    const int16_t CTRL_STAT_PLAY                = (1);
    const int16_t CTRL_STAT_END                 = (2);

    const int16_t CTRL_REQ_STOP                 = (0);
    const int16_t CTRL_REQ_PLAY                 = (1);

    const int16_t PBEND_STAT_STOP               = (0);
    const int16_t PBEND_STAT_TP_UP              = (1);
    const int16_t PBEND_STAT_TP_DOWN            = (2);
    const int16_t PBEND_STAT_END                = (3);

    const uint16_t MIN_TP                       = (0);
    const uint16_t MAX_TP                       = (4095);

    const int8_t MIN_TP_OFS                    = (100*-1);
    const int8_t MAX_TP_OFS                    = (100);
    const int8_t DEFAULT_TP_OFS                = (0);

    const int16_t SW_ENV_STAT_INIT_NOTE_ON      = (0);
    const int16_t SW_ENV_STAT_ATTACK            = (1);
    const int16_t SW_ENV_STAT_HOLD              = (2);
    const int16_t SW_ENV_STAT_DECAY             = (3);
    const int16_t SW_ENV_STAT_FADE              = (4);
    const int16_t SW_ENV_STAT_INIT_NOTE_OFF     = (5);
    const int16_t SW_ENV_STAT_RELEASE           = (6);
    const int16_t SW_ENV_STAT_END               = (7);

    const int16_t LFO_STAT_STOP                 = (0);
    const int16_t LFO_STAT_RUN                  = (1);

    const int16_t MAX_LFO_PERIOD                = (10);       /* unit: sec. */
    const uint16_t DEFAULT_PROC_FREQ            = (100);      /* Hz */

    const int16_t MIN_NOTE_NUMBER               = (0);
    const int16_t MAX_NOTE_NUMBER               = (95);
    const int16_t DEFAULT_NOTE_NUMBER           = (0);

    const int16_t MAX_REPEATING_DOT_LENGTH      = (3);

    const int16_t MIN_OCTAVE                    = (1);
    const int16_t MAX_OCTAVE                    = (8);
    const int16_t DEFAULT_OCTAVE                = (4);

    const int16_t MIN_GATE_TIME                 = (1);
    const int16_t MAX_GATE_TIME                 = (8);
    const int16_t DEFAULT_GATE_TIME             = MAX_GATE_TIME;

    const int16_t MIN_NOISE_NP                  = (0);
    const int16_t MAX_NOISE_NP                  = (31);
    const int16_t DEFAULT_NOISE_NP              = (16);

    const int16_t MAX_LOOP_NESTING_DEPTH        = (3);

    const int16_t MAX_FIN_PRI_LOOP_TRY          = (15);

    const int16_t MIN_LOOP_TIMES                = (0);
    const int16_t MAX_LOOP_TIMES                = (255);
    const int16_t DEFAULT_LOOP_TIMES            = (1);

    const uint16_t MIN_ENVELOP_EP               = (0x0000);
    const uint16_t MAX_ENVELOP_EP               = (0xFFFF);
    const uint16_t DEFAULT_ENVELOP_EP           = (0x0000);

    const int16_t MIN_ENVELOP_SHAPE             = (0x0);
    const int16_t MAX_ENVELOP_SHAPE             = (0xF);
    const int16_t DEFAULT_ENVELOP_SHAPE         = (0x0);

    const int16_t MIN_VOLUME_LEVEL              = (0);
    const int16_t MAX_VOLUME_LEVEL              = (15);
    const int16_t DEFAULT_VOLUME_LEVEL          = (15);

    const uint16_t MIN_TEMPO                    = (10);
    const uint16_t MAX_TEMPO                    = (1000);
    const uint16_t DEFAULT_TEMPO                = (120);

    const uint16_t MIN_SPEED_FACTOR             = (20);     // 20%
    const uint16_t MAX_SPEED_FACTOR             = (500);    // 500%
    const uint16_t DEFAULT_SPEED_FACTOR         = (100);    // 100%

    const int16_t MIN_FREQ_SHIFT_DEGREES        = (-360*5);
    const int16_t MAX_FREQ_SHIFT_DEGREES        = (360*5);
    const int16_t DEFAULT_FREQ_SHIFT_DEGREES    = (0);

    const int16_t MIN_NOTE_LENGTH               = (1);
    const int16_t MAX_NOTE_LENGTH               = (128);
    const int16_t DEFAULT_NOTE_LENGTH           = (4);

    const uint16_t MAX_MML_TEXT_LEN             = (0xFFFE);

    const int16_t SW_ENV_MODE_OFF               = (0);
    const int16_t SW_ENV_MODE_ON                = (1);
    const int16_t MIN_SW_ENV_MODE               = SW_ENV_MODE_OFF;
    const int16_t MAX_SW_ENV_MODE               = SW_ENV_MODE_ON;
    const int16_t DEFAULT_SW_ENV_MODE           = SW_ENV_MODE_OFF;

    const uint16_t MIN_SW_ENV_TIME_UNIT         = (0);
    const uint16_t MAX_SW_ENV_TIME_UNIT         = (8192);
    const uint16_t DEFAULT_SW_ENV_TIME_UNIT     = (0);

    const int16_t MIN_SOFT_ENVELOPE_ATTACK      = (0);
    const int16_t MAX_SOFT_ENVELOPE_ATTACK      = (10000);
    const int16_t DEFAULT_SOFT_ENVELOPE_ATTACK  = (0);

    const int16_t MIN_SOFT_ENVELOPE_HOLD        = (0);
    const int16_t MAX_SOFT_ENVELOPE_HOLD        = (10000);
    const int16_t DEFAULT_SOFT_ENVELOPE_HOLD    = (0);

    const int16_t MIN_SOFT_ENVELOPE_DECAY       = (0);
    const int16_t MAX_SOFT_ENVELOPE_DECAY       = (10000);
    const int16_t DEFAULT_SOFT_ENVELOPE_DECAY   = (0);

    const uint16_t MIN_SOFT_ENVELOPE_SUSTAIN     = (0);
    const uint16_t MAX_SOFT_ENVELOPE_SUSTAIN     = (1500);
    const uint16_t DEFAULT_SOFT_ENVELOPE_SUSTAIN = (100);

    const int16_t MIN_SOFT_ENVELOPE_FADE        = (0);
    const int16_t MAX_SOFT_ENVELOPE_FADE        = (10000);
    const int16_t DEFAULT_SOFT_ENVELOPE_FADE    = (0);

    const int16_t MIN_SOFT_ENVELOPE_RELEASE     = (0);
    const int16_t MAX_SOFT_ENVELOPE_RELEASE     = (10000);
    const int16_t DEFAULT_SOFT_ENVELOPE_RELEASE = (0);

    const int16_t MIN_BIAS_LEVEL                = (-500);
    const int16_t MAX_BIAS_LEVEL                = (500);
    const int16_t DEFAULT_BIAS_LEVEL            = (0);
    const int16_t BIAS_LEVEL_OFS                = (500);

    const uint16_t MIN_LFO_SPEED_UNIT           = (0);
    const uint16_t MAX_LFO_SPEED_UNIT           = (8192);
    const uint16_t DEFAULT_LFO_SPEED_UNIT       = (0);

    const int16_t MIN_LFO_SPEED                 = (-200);
    const int16_t MAX_LFO_SPEED                 = (200);
    const int16_t DEFAULT_LFO_SPEED             = (40);

    const int16_t MIN_LFO_DEPTH                 = (0);
    const int16_t MAX_LFO_DEPTH                 = (255);
    const int16_t DEFAULT_LFO_DEPTH             = (0);

    const int16_t MIN_LFO_DELAY                 = (0);
    const int16_t MAX_LFO_DELAY                 = (128);
    const int16_t DEFAULT_LFO_DELAY             = (0);

    const int16_t MIN_PITCHBEND_LEVEL           = (-360*8);
    const int16_t MAX_PITCHBEND_LEVEL           = (360*8);
    const int16_t DEFAULT_PITCHBEND_LEVEL       = (0);

    const int16_t LFO_MODE_OFF                  = (0);
    const int16_t LFO_MODE_TRIANGLE             = (1);
    const int16_t MIN_LFO_MODE                  = LFO_MODE_OFF;
    const int16_t MAX_LFO_MODE                  = LFO_MODE_TRIANGLE;
    const int16_t DEFAULT_LFO_MODE              = LFO_MODE_OFF;

    const int32_t Q_PITCHBEND_FACTOR            = (16809550);   /* POW(2, 1/360) << 24 */
    const int32_t Q_PITCHBEND_FACTOR_N          = (16744944);   /* POW(2,-1/360) << 24 */

    const int32_t Q_CALCTP_FACTOR               = (17774841);   /* POW(2, 1/12)  << 24 */ 
    const int32_t Q_CALCTP_FACTOR_N             = (15835583);   /* POW(2,-1/12)  << 24 */ 

    struct SYS_STATUS
    {
        uint16_t    SET_MML        : 1;
        uint16_t    REVERSE        : 1;
        uint16_t    NUM_CH_IMPL    : 2;
        uint16_t    NUM_CH_USED    : 2;
        uint16_t    RH_LEN         : 1;
        uint16_t    CTRL_STAT      : 2;
        uint16_t    CTRL_STAT_PRE  : 2;
        uint16_t    FIN_PRI_LOOP_TRY : 4;
        uint16_t                   : 1;
    };

    struct SYS_REQUEST
    {
        uint8_t    CTRL_REQ        : 2;
        uint8_t                    : 6;
        uint8_t    CTRL_REQ_FLAG   : 1;
        uint8_t    FIN_PRI_LOOP_FLAG : 1;
        uint8_t                    : 6;
    };

    struct CH_STATUS
    {
        uint16_t    DECODE_END     : 1;
        uint16_t    LEGATO         : 1;
        uint16_t    LFO_MODE       : 3;
        uint16_t    LFO_STAT       : 1;
        uint16_t    SW_ENV_MODE    : 1;
        uint16_t    SW_ENV_STAT    : 3;
        uint16_t    PBEND_STAT     : 2;
        uint16_t    LOOP_DEPTH     : 2;
        uint16_t    END_PRI_LOOP   : 1;
        uint16_t                   : 1;
    };

    struct GLOBAL_INFO
    {
        SYS_STATUS  sys_status;
        SYS_REQUEST sys_request;
        uint32_t    s_clock;
        uint16_t    proc_freq;
        uint16_t    speed_factor;
        int16_t     shift_degrees;
        uint8_t     mml_version;
    };

    struct CALLBACK_INFO
    {
        void (*user_callback)(uint8_t ch, int32_t param);
    };

    struct MML_INFO
    {
        const char *p_mml_head;
        uint16_t    mml_len;
        uint16_t    ofs_mml_pos;
        uint16_t    ofs_mml_loop_head[MAX_LOOP_NESTING_DEPTH];
        uint8_t     loop_times[MAX_LOOP_NESTING_DEPTH];
        uint8_t     prim_loop_counter;
    };

    struct TONE_INFO
    {
        uint16_t    tempo;
        uint8_t     note_len;
        int8_t      tp_ofs;
        uint8_t     OCTAVE     : 3;
        uint8_t     GATE_TIME  : 3;
        uint8_t     LEN_DOTS   : 2;
        uint16_t    HW_ENV     : 1;
        uint16_t    VOLUME     : 4;
        uint16_t    BIAS       :10;
        uint16_t               : 1; 
    };

    struct TIME_INFO
    {
        uint16_t    note_on;
        uint16_t    gate; 
        uint16_t    sw_env;
        uint16_t    lfo_delay;
        uint16_t    pitchbend;
        uint16_t    NOTE_ON_FRAC   :12;
        uint16_t                   : 4;
    };

    struct LFO_INFO
    {
        int16_t     speed;
        uint8_t     depth;
        uint8_t     BASE_TP_L  : 8;
        uint16_t    delay_tk;
        uint16_t    theta;
        uint16_t    DELTA_FRAC : 6;
        uint16_t    TP_FRAC    : 6;
        uint16_t    BASE_TP_H  : 4;
        uint16_t    speed_unit;
    };

    struct SW_ENV_INFO
    {
        uint16_t    attack_tk;
        uint16_t    hold_tk;
        uint16_t    decay_tk;
        uint16_t    fade_tk;
        uint16_t    release_tk;
        uint16_t    VOL_INT   : 4;
        uint16_t    VOL_FRAC  :12;
        uint16_t    REL_VOL_INT   : 4;
        uint16_t    REL_VOL_FRAC  :12;
        uint16_t    sustain;
        uint16_t    time_unit;
    };

    struct PITCHBEND_INFO
    {
        int16_t  level;
        uint16_t TP_FRAC    : 6;
        uint16_t TP_D_FRAC  : 6;
        uint16_t TP_END_L   : 4;
        uint32_t TP_INT     :12;
        uint32_t TP_D_INT   :12;
        uint32_t TP_END_H   : 8;
    };

    struct CHANNEL_INFO
    {
        CH_STATUS       ch_status;
        MML_INFO        mml;
        TONE_INFO       tone;
        TIME_INFO       time;
        LFO_INFO        lfo;
        SW_ENV_INFO     sw_env;
        PITCHBEND_INFO  pitchbend;
    };

    struct PSG_REG
    {
        uint16_t   flags_addr;
        uint8_t    flags_mixer;
        uint8_t    data[16];
    };

    struct SLOT
    {
        GLOBAL_INFO     gl_info;
        CALLBACK_INFO   cb_info;
        CHANNEL_INFO   *ch_info_list[NUM_CHANNEL];
        PSG_REG         psg_reg;
    };

    void init_slot( SLOT    &slot
            , uint32_t      s_clock
            , uint16_t      proc_freq
            , bool          reverse
            , CHANNEL_INFO  *p_ch0
            , CHANNEL_INFO  *p_ch1 = nullptr
            , CHANNEL_INFO  *p_ch2 = nullptr
            );
    int set_mml(SLOT &slot, const char *p_mml, uint16_t mode);
    void set_user_callback(SLOT &slot, void (*callback)(uint8_t ch, int32_t param));
    void control_psg(SLOT &slot);
    void reset(SLOT &slot);
    void set_speed_factor(SLOT &slot, uint16_t speed_factor);
    void shift_frequency(SLOT &slot, int16_t shift_degrees);
}
#pragma pack()


#endif/*PSG_CTRL_H*/
