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

namespace PsgCtrl {

    constexpr uint8_t DEFAULT_MML_VERSION           = (1);
    constexpr int16_t NUM_CHANNEL                   = (3);

    constexpr int16_t CTRL_STAT_STOP                = (0);
    constexpr int16_t CTRL_STAT_PLAY                = (1);
    constexpr int16_t CTRL_STAT_END                 = (2);

    constexpr int16_t CTRL_REQ_STOP                 = (0);
    constexpr int16_t CTRL_REQ_PLAY                 = (1);

    constexpr int16_t PBEND_STAT_STOP               = (0);
    constexpr int16_t PBEND_STAT_TP_UP              = (1);
    constexpr int16_t PBEND_STAT_TP_DOWN            = (2);
    constexpr int16_t PBEND_STAT_END                = (3);

    constexpr uint16_t MIN_TP                       = (0);
    constexpr uint16_t MAX_TP                       = (4095);

    constexpr int8_t MIN_TP_OFS                     = (100*-1);
    constexpr int8_t MAX_TP_OFS                     = (100);
    constexpr int8_t DEFAULT_TP_OFS                 = (0);

    constexpr int16_t MIN_EXCLUDE_NOTE_LEN          = (0);
    constexpr int16_t MAX_EXCLUDE_NOTE_LEN          = (128);
    constexpr int16_t DEFAULT_EXCLUDE_NOTE_LEN      = (0);

    constexpr uint16_t NOISE_SWEEP_STAT_STOP        = (0);
    constexpr uint16_t NOISE_SWEEP_STAT_NP_UP       = (1);
    constexpr uint16_t NOISE_SWEEP_STAT_NP_DOWN     = (2);
    constexpr uint16_t NOISE_SWEEP_STAT_END         = (3);

    constexpr int16_t SW_ENV_STAT_INIT_NOTE_ON      = (0);
    constexpr int16_t SW_ENV_STAT_ATTACK            = (1);
    constexpr int16_t SW_ENV_STAT_HOLD              = (2);
    constexpr int16_t SW_ENV_STAT_DECAY             = (3);
    constexpr int16_t SW_ENV_STAT_FADE              = (4);
    constexpr int16_t SW_ENV_STAT_INIT_NOTE_OFF     = (5);
    constexpr int16_t SW_ENV_STAT_RELEASE           = (6);
    constexpr int16_t SW_ENV_STAT_END               = (7);

    constexpr int16_t LFO_STAT_STOP                 = (0);
    constexpr int16_t LFO_STAT_RUN                  = (1);

    constexpr int16_t MAX_LFO_PERIOD                = (10);       /* unit: sec. */
    constexpr uint16_t DEFAULT_PROC_FREQ            = (100);      /* Hz */

    constexpr int16_t MIN_NOTE_NUMBER               = (0);
    constexpr int16_t MAX_NOTE_NUMBER               = (95);
    constexpr int16_t DEFAULT_NOTE_NUMBER           = (0);

    constexpr int16_t MAX_REPEATING_DOT_LENGTH      = (3);

    constexpr int16_t MIN_OCTAVE                    = (1);
    constexpr int16_t MAX_OCTAVE                    = (8);
    constexpr int16_t DEFAULT_OCTAVE                = (4);

    constexpr int16_t MIN_GATE_TIME                 = (1);
    constexpr int16_t MAX_GATE_TIME                 = (8);
    constexpr int16_t DEFAULT_GATE_TIME             = MAX_GATE_TIME;

    constexpr int16_t MIN_NOISE_NP                  = (0);
    constexpr int16_t MAX_NOISE_NP                  = (31);
    constexpr int16_t DEFAULT_NOISE_NP              = (16);

    constexpr int16_t MAX_LOOP_NESTING_DEPTH        = (3);

    constexpr int16_t MAX_FIN_PRI_LOOP_TRY          = (15);

    constexpr int16_t MIN_LOOP_TIMES                = (0);
    constexpr int16_t MAX_LOOP_TIMES                = (255);
    constexpr int16_t DEFAULT_LOOP_TIMES            = (1);

    constexpr uint16_t MIN_ENVELOP_EP               = (0x0000);
    constexpr uint16_t MAX_ENVELOP_EP               = (0xFFFF);
    constexpr uint16_t DEFAULT_ENVELOP_EP           = (0x0000);

    constexpr int16_t MIN_ENVELOP_SHAPE             = (0x0);
    constexpr int16_t MAX_ENVELOP_SHAPE             = (0xF);
    constexpr int16_t DEFAULT_ENVELOP_SHAPE         = (0x0);

    constexpr int16_t MIN_VOLUME_LEVEL              = (0);
    constexpr int16_t MAX_VOLUME_LEVEL              = (15);
    constexpr int16_t DEFAULT_VOLUME_LEVEL          = (15);

    constexpr uint16_t MIN_TEMPO                    = (10);
    constexpr uint16_t MAX_TEMPO                    = (1000);
    constexpr uint16_t DEFAULT_TEMPO                = (120);

    constexpr uint16_t MIN_SPEED_FACTOR             = (20);     // 20%
    constexpr uint16_t MAX_SPEED_FACTOR             = (500);    // 500%
    constexpr uint16_t DEFAULT_SPEED_FACTOR         = (100);    // 100%

    constexpr int16_t MIN_FREQ_SHIFT_DEGREES        = (-360*5);
    constexpr int16_t MAX_FREQ_SHIFT_DEGREES        = (360*5);
    constexpr int16_t DEFAULT_FREQ_SHIFT_DEGREES    = (0);

    constexpr int16_t MIN_NOTE_LENGTH               = (1);
    constexpr int16_t MAX_NOTE_LENGTH               = (128);
    constexpr int16_t DEFAULT_NOTE_LENGTH           = (4);

    constexpr uint16_t MAX_MML_TEXT_LEN             = (0xFFFE);

    constexpr int16_t SW_ENV_MODE_OFF               = (0);
    constexpr int16_t SW_ENV_MODE_ON                = (1);
    constexpr int16_t MIN_SW_ENV_MODE               = SW_ENV_MODE_OFF;
    constexpr int16_t MAX_SW_ENV_MODE               = SW_ENV_MODE_ON;
    constexpr int16_t DEFAULT_SW_ENV_MODE           = SW_ENV_MODE_OFF;

    constexpr uint16_t MIN_SW_ENV_TIME_UNIT         = (0);
    constexpr uint16_t MAX_SW_ENV_TIME_UNIT         = (8192);
    constexpr uint16_t DEFAULT_SW_ENV_TIME_UNIT     = (0);

    constexpr int16_t MIN_SOFT_ENVELOPE_ATTACK      = (0);
    constexpr int16_t MAX_SOFT_ENVELOPE_ATTACK      = (10000);
    constexpr int16_t DEFAULT_SOFT_ENVELOPE_ATTACK  = (0);

    constexpr int16_t MIN_SOFT_ENVELOPE_HOLD        = (0);
    constexpr int16_t MAX_SOFT_ENVELOPE_HOLD        = (10000);
    constexpr int16_t DEFAULT_SOFT_ENVELOPE_HOLD    = (0);

    constexpr int16_t MIN_SOFT_ENVELOPE_DECAY       = (0);
    constexpr int16_t MAX_SOFT_ENVELOPE_DECAY       = (10000);
    constexpr int16_t DEFAULT_SOFT_ENVELOPE_DECAY   = (0);

    constexpr uint16_t MIN_SOFT_ENVELOPE_SUSTAIN     = (0);
    constexpr uint16_t MAX_SOFT_ENVELOPE_SUSTAIN     = (1500);
    constexpr uint16_t DEFAULT_SOFT_ENVELOPE_SUSTAIN = (100);

    constexpr int16_t MIN_SOFT_ENVELOPE_FADE        = (0);
    constexpr int16_t MAX_SOFT_ENVELOPE_FADE        = (10000);
    constexpr int16_t DEFAULT_SOFT_ENVELOPE_FADE    = (0);

    constexpr int16_t MIN_SOFT_ENVELOPE_RELEASE     = (0);
    constexpr int16_t MAX_SOFT_ENVELOPE_RELEASE     = (10000);
    constexpr int16_t DEFAULT_SOFT_ENVELOPE_RELEASE = (0);

    constexpr int16_t MIN_BIAS_LEVEL                = (-500);
    constexpr int16_t MAX_BIAS_LEVEL                = (500);
    constexpr int16_t DEFAULT_BIAS_LEVEL            = (0);
    constexpr int16_t BIAS_LEVEL_OFS                = (500);

    constexpr uint16_t MIN_LFO_SPEED_UNIT           = (0);
    constexpr uint16_t MAX_LFO_SPEED_UNIT           = (8192);
    constexpr uint16_t DEFAULT_LFO_SPEED_UNIT       = (0);

    constexpr int16_t MIN_LFO_SPEED                 = (-200);
    constexpr int16_t MAX_LFO_SPEED                 = (200);
    constexpr int16_t DEFAULT_LFO_SPEED             = (40);

    constexpr int16_t MIN_LFO_DEPTH                 = (0);
    constexpr int16_t MAX_LFO_DEPTH                 = (255);
    constexpr int16_t DEFAULT_LFO_DEPTH             = (0);

    constexpr int16_t MIN_LFO_DELAY                 = (0);
    constexpr int16_t MAX_LFO_DELAY                 = (128);
    constexpr int16_t DEFAULT_LFO_DELAY             = (0);

    constexpr int16_t MIN_PITCHBEND_LEVEL           = (-360*8);
    constexpr int16_t MAX_PITCHBEND_LEVEL           = (360*8);
    constexpr int16_t DEFAULT_PITCHBEND_LEVEL       = (0);

    constexpr int16_t LFO_MODE_OFF                  = (0);
    constexpr int16_t LFO_MODE_TRIANGLE             = (1);
    constexpr int16_t MIN_LFO_MODE                  = LFO_MODE_OFF;
    constexpr int16_t MAX_LFO_MODE                  = LFO_MODE_TRIANGLE;
    constexpr int16_t DEFAULT_LFO_MODE              = LFO_MODE_OFF;

    constexpr int32_t Q_PITCHBEND_FACTOR            = (16809550);   /* POW(2, 1/360) << 24 */
    constexpr int32_t Q_PITCHBEND_FACTOR_N          = (16744944);   /* POW(2,-1/360) << 24 */

    constexpr int32_t Q_CALCTP_FACTOR               = (17774841);   /* POW(2, 1/12)  << 24 */
    constexpr int32_t Q_CALCTP_FACTOR_N             = (15835583);   /* POW(2,-1/12)  << 24 */

    struct SYS_STATUS {
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

    struct SYS_REQUEST {
        uint8_t    CTRL_REQ        : 2;
        uint8_t                    : 6;
        uint8_t    CTRL_REQ_FLAG   : 1;
        uint8_t    FIN_PRI_LOOP_FLAG : 1;
        uint8_t                    : 6;
    };

    struct CH_STATUS {
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

    struct NOISE_INFO {
        uint16_t    sweep_time;
        uint16_t    NP_INT         : 5;
        uint16_t    NP_FRAC        : 6;
        uint16_t    NP_END         : 5;
        uint16_t    NP_D_INT       : 5;
        uint16_t    NP_D_FRAC      : 6;
        uint16_t    SWEEP_STAT     : 2;
        uint16_t                   : 3;
        uint8_t     NP_I           : 5;
        uint8_t                    : 3;
    };

    struct GLOBAL_INFO {
        SYS_STATUS  sys_status;
        SYS_REQUEST sys_request;
        uint32_t    s_clock;
        uint16_t    proc_freq;
        uint16_t    speed_factor;
        int16_t     shift_degrees;
        uint8_t     mml_version;
        NOISE_INFO  noise_info;
    };

    struct CALLBACK_INFO {
        void (*user_callback)(uint8_t ch, int32_t param);
    };

    struct MML_INFO {
        const char *p_mml_head;
        uint16_t    mml_len;
        uint16_t    ofs_mml_pos;
        uint16_t    ofs_mml_loop_head[MAX_LOOP_NESTING_DEPTH];
        uint8_t     loop_times[MAX_LOOP_NESTING_DEPTH];
        uint8_t     prim_loop_counter;
    };

    struct TONE_INFO {
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

    struct TIME_INFO {
        uint16_t    note_on;
        uint16_t    gate;
        uint16_t    sw_env;
        uint16_t    lfo_delay;
        uint16_t    pitchbend;
        uint16_t    NOTE_ON_FRAC   :12;
        uint16_t                   : 4;
    };

    struct LFO_INFO {
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

    struct SW_ENV_INFO {
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

    struct PITCHBEND_INFO {
        int16_t  level;
        uint16_t TP_FRAC    : 6;
        uint16_t TP_D_FRAC  : 6;
        uint16_t TP_END_L   : 4;
        uint32_t TP_INT     :12;
        uint32_t TP_D_INT   :12;
        uint32_t TP_END_H   : 8;
    };

    struct CHANNEL_INFO {
        CH_STATUS       ch_status;
        MML_INFO        mml;
        TONE_INFO       tone;
        TIME_INFO       time;
        LFO_INFO        lfo;
        SW_ENV_INFO     sw_env;
        PITCHBEND_INFO  pitchbend;
    };

    struct PSG_REG {
        uint16_t   flags_addr;
        uint8_t    flags_mixer;
        uint8_t    data[16];
    };

    struct SLOT {
        GLOBAL_INFO     gl_info;
        CALLBACK_INFO   cb_info;
        CHANNEL_INFO   *ch_info_list[NUM_CHANNEL];
        PSG_REG         psg_reg;
    };

    /**
     * @brief Initializes a SLOT structure.
     *
     * @param slot Reference to the SLOT structure to be initialized.
     * @param s_clock System clock frequency.The unit of this parameter is 0.01 Hz.
     * @param proc_freq Processing frequency.The unit of this parameter is 1 Hz.
     * @param reverse Set to true if the slot should process in reverse channel mode.
     * @param p_ch0 Pointer to the first CHANNEL_INFO structure.
     * @param p_ch1 Pointer to the second CHANNEL_INFO structure (optional).
     * @param p_ch2 Pointer to the third CHANNEL_INFO structure (optional).
     */
    void init_slot(
            SLOT &slot,
            uint32_t s_clock,
            uint16_t proc_freq,
            bool reverse,
            CHANNEL_INFO *p_ch0,
            CHANNEL_INFO *p_ch1 = nullptr,
            CHANNEL_INFO *p_ch2 = nullptr
    );

    /**
     * @brief Sets the MML string for a SLOT.
     *
     * @param slot Reference to the SLOT structure.
     * @param p_mml Pointer to the MML string.
     * @param mode Mode setting for the MML.
     * @return Returns an integer status code.
     * @retval 0 Success.
     * @retval Negative value Error.
     */
    int set_mml(SLOT &slot, const char *p_mml, uint16_t mode);

    /**
     * @brief Sets a user-defined callback function for a SLOT.
     *
     * @param slot Reference to the SLOT structure.
     * @param callback Pointer to the callback function that takes a channel and a parameter.
     *
     * This callback is invoked when the @C command is decoded.
     */
    void set_user_callback(
            SLOT &slot,
            void (*callback)(uint8_t ch, int32_t param)
    );

    /**
     * @brief Controls the PSG (Programmable Sound Generator) for a SLOT.
     *
     * @param slot Reference to the SLOT structure.
     *
     * This function must be executed at the frequency specified by the `proc_freq` argument in `init_slot`.
     */
    void control_psg(SLOT &slot);

    /**
     * @brief Resets a SLOT to its initial state.
     *
     * @param slot Reference to the SLOT structure.
     */
    void reset(SLOT &slot);

    /**
     * @brief Sets the speed factor for a SLOT.
     *
     * @param slot Reference to the SLOT structure.
     * @param speed_factor Speed factor to be set.
     */
    void set_speed_factor(SLOT &slot, uint16_t speed_factor);

    /**
     * @brief Shifts the frequency of a SLOT by a certain number of degrees.
     *
     * @param slot Reference to the SLOT structure.
     * @param shift_degrees The amount by which to shift the frequency, in degrees.
     */
    void shift_frequency(SLOT &slot, int16_t shift_degrees);

}
#pragma pack()


#endif/*PSG_CTRL_H*/
