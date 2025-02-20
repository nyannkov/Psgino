/*
 * MIT License, see the LICENSE file for details.
 *
 * Copyright (c) 2023 nyannkov
 */
#include <cstdlib>
#include "psg_ctrl.h"

namespace PsgCtrl {
namespace {

    bool parse_mml_header(SLOT &slot, const char **pp_text);

    int16_t decode_mml(SLOT &slot, uint8_t ch);
    void decode_dollar(
            CHANNEL_INFO *p_info,
            const char **pp_pos,
            const char *p_tail,
            uint16_t proc_freq
    );
    void decode_atsign(
            SLOT &slot,
            uint8_t ch,
            const char **pp_pos,
            const char *p_tail
    );

    const char * read_number_ex(
            const char *p_pos,
            const char *p_tail,
            int32_t min,
            int32_t max,
            int32_t default_value,
            int32_t *p_out,
            /*@null@*/bool *p_is_omitted
    );
    const char * read_number(
            const char *p_pos,
            const char *p_tail,
            int32_t min,
            int32_t max,
            int32_t default_value,
            int32_t *p_out
    );
    int32_t get_param(
            const char **pp_pos,
            const char *p_tail,
            int32_t min,
            int32_t max,
            int32_t default_value
    );


    uint16_t shift_tp(uint16_t tp, int16_t bias);
    uint16_t calc_tp(int16_t n, uint32_t s_clock);
    uint8_t get_tp_table_column_number(const char note_name);
    const char * shift_half_note_number(
            const char *p_pos,
            const char *p_tail,
            int16_t note_num,
            int16_t *p_out
    );
    const char * get_legato_end_note_num(
            const char *p_pos,
            const char *p_tail,
            int32_t default_octave,
            int32_t *p_out,
            int16_t start_note_num
    );
    const char * count_dot(
            const char *p_pos,
            const char *p_tail,
            uint8_t *p_out
    );
    uint32_t get_note_on_time(
            uint8_t note_len,
            uint16_t tempo,
            uint8_t dot_cnt,
            uint16_t proc_freq
    );
    void generate_tone(
            SLOT &slot,
            uint8_t ch,
            const char **pp_pos,
            const char *p_tail,
            uint32_t q12_exclude_note_len
    );

    void init_pitchbend(SLOT &slot, uint8_t ch);
    void proc_pitchbend(SLOT &slot, uint8_t ch);

    void init_noise_sweep(SLOT &slot, uint8_t ch);
    void proc_noise_sweep(SLOT &slot);

    int16_t get_lfo_speed(int16_t freq_value, uint16_t speed_unit, uint16_t tempo);
    void proc_lfo(SLOT &slot, uint8_t ch);

    uint16_t sw_env_time2tk(
            uint16_t env_time,
            uint16_t time_unit,
            uint16_t tempo,
            uint16_t proc_freq
    );
    uint16_t get_sus_volume(const SLOT &slot, uint8_t ch);
    void trans_sw_env_state(SLOT &slot, uint8_t ch);
    void update_sw_env_volume(SLOT &slot, uint8_t ch);
    void proc_sw_env_gen(SLOT &slot, uint8_t ch);

    void reset_ch_info(CHANNEL_INFO *p_ch_info);
    void reset_psg(PSG_REG &psg_reg);
    void rewind_mml(SLOT &slot);

    void skip_white_space(const char **pp_text);
    inline char to_upper_case(char c) {

        if ( ('a' <= c) && (c <= 'z') ) {
            c &= static_cast<uint8_t>(~0x20UL);
        }

        return c;
    }

    inline bool is_white_space(const char c) {

        switch(c) {
        case ' ': /*@fallthrough@*/
        case '\t':/*@fallthrough@*/
        case '\n':/*@fallthrough@*/
        case '\r':
            return true;
        default:
            return false;
        }
    }


    inline uint16_t U16(uint8_t h, uint8_t l) {

        return (static_cast<uint16_t>(h)<<8) | (l);
    }

    inline uint8_t U16_HI(uint16_t x) {

        return (((x)>>8)&0xFF);
    }

    inline uint8_t U16_LO(uint16_t x) {

        return (((x)>>0)&0xFF);
    }

    inline int32_t SAT(int32_t x, int32_t min, int32_t max) {

        return( (x <= min ) ? min
              : (x >= max ) ? max
              :  x
              );
    }

    inline uint8_t clamp_channel(uint8_t ch) {
        if ( ch >= NUM_CHANNEL ) {
            // Should never reach here.
            ch = NUM_CHANNEL-1;
        }
        return ch;
    }

    uint16_t sw_env_time2tk(
            uint16_t env_time,
            uint16_t time_unit,
            uint16_t tempo,
            uint16_t proc_freq
    ) {

        if ( time_unit == 0 ) {

            /* env_time unit is msec */
            return (static_cast<uint32_t>(env_time)*proc_freq+500)/1000;

        } else if ( tempo != 0 ) {

            /* env_time unit is note-unit */
            return (static_cast<uint32_t>(env_time)*proc_freq*4*60)/(static_cast<uint32_t>(tempo)*time_unit);

        } else {

            return 0;
        }
    }

    int16_t get_lfo_speed(int16_t freq_value, uint16_t speed_unit, uint16_t tempo) {

        if ( speed_unit == 0 ) {

            return freq_value;

        } else {

            int32_t result = static_cast<int32_t>(freq_value) * tempo * speed_unit;

            return static_cast<int16_t>(result/240);
        }
    }

    void skip_white_space(const char **pp_text) {

        size_t n = MAX_MML_TEXT_LEN;
        while ( is_white_space(**pp_text) && (n != 0) ) {

            if ( **pp_text == '\0' ) {

                break;
            }

            (*pp_text)++;
            n--;
        }
    }

    bool parse_mml_header(SLOT &slot, const char **pp_text) {

        const char *p_pos;
        bool parse_cont;
        long value;

        /* MML header sections must start with a colon (:). */
        if ( **pp_text != ':' ) {

            /* The header is considered as omitted and treated as normal in processing. */
            return true;
        }

        p_pos = *pp_text + 1;

        /* The MML version number must start immediately after the colon. */
        if ( to_upper_case(*p_pos) == 'V' ) {

            value = std::strtol(&p_pos[1], const_cast<char**>(&p_pos), 10);

            /* Here is a provisional implementation. This processing will change according to the MML version upgrade. */
            if ( value == 1 ) {

                slot.gl_info.mml_version = 1;
            }
        }

        parse_cont = true;
        while (parse_cont) {

            switch ( to_upper_case(*p_pos) ) {

            case 'M':
                value = std::strtol(&p_pos[1], const_cast<char**>(&p_pos), 10);
                slot.gl_info.sys_status.RH_LEN = (( value & 0x1 ) != 0) ? 1 : 0;
                break;

            case ';':
                /* End of MML header section. */
                p_pos++;
                parse_cont = false;
                break;

            case '\0':
                /* Parse failed. */
                parse_cont = false;
                break;

            default:
                /* Unknown settings will be ignored. */
                p_pos++;
                break;
            }
        }

        *pp_text = p_pos;

        return ( **pp_text != '\0' );
    }

    const char * count_dot(
            const char *p_pos,
            const char *p_tail,
            uint8_t *p_out
    ) {

        const char *p;
        uint8_t dot_cnt;

        dot_cnt = 0;
        for ( p = p_pos; p < p_tail; p++ ) {

            if ( *p == '.' ) {
                dot_cnt = (dot_cnt < MAX_REPEATING_DOT_LENGTH)
                        ? (dot_cnt+1)
                        : MAX_REPEATING_DOT_LENGTH;
            } else {

                break;
            }
        }

        *p_out = dot_cnt;

        return p;
    }

    uint16_t shift_tp(uint16_t tp, int16_t bias) {

        uint16_t q;
        uint16_t r;
        uint32_t q24_f;
        uint64_t lq24_tp;

        lq24_tp = static_cast<uint64_t>(tp)<<24;

        if ( bias >= 0 ) {

            q = bias/360;
            r = bias%360;
            lq24_tp >>= q;
            q24_f = Q_PITCHBEND_FACTOR_N;

        } else {

            bias *= -1;
            q = bias/360;
            r = bias%360;
            lq24_tp <<= q;
            q24_f= Q_PITCHBEND_FACTOR;
        }

        for (uint16_t i = 0; i < r; i++ ) {

            lq24_tp *= q24_f;
            lq24_tp >>= 24;
        }

        lq24_tp += 1<<23;
        tp = static_cast<uint16_t>(lq24_tp>>24);

        return ( (tp < MAX_TP) ? tp : MAX_TP );
    }

    uint16_t calc_tp(int16_t n, uint32_t s_clock) {

        int64_t lq24_hertz;
        int64_t lq24_clock;
        int32_t q24_f;
        int32_t r;
        int32_t q;
        uint32_t tp;

        n -= 45;
        q = n/12;

        lq24_hertz = static_cast<int64_t>(440)<<24;
        if ( n >= 0 ) {

            lq24_hertz <<= q;
            q24_f = Q_CALCTP_FACTOR;

        } else {

            lq24_hertz >>= (q*-1);
            q24_f = Q_CALCTP_FACTOR_N;
            n *= -1;
        }

        r = n%12;
        while ( r-- > 0 ) {

            lq24_hertz *= q24_f;
            lq24_hertz >>= 24;
        }

        lq24_clock = static_cast<int64_t>(s_clock)<<24;

        tp = (lq24_clock/(1600*lq24_hertz));

        if ( tp > MAX_TP ) {

            tp = MAX_TP;
        }

        return static_cast<uint16_t>(tp);
    }

    const char * read_number_ex(
            const char *p_pos,
            const char *p_tail,
            int32_t min,
            int32_t max,
            int32_t default_value,
            int32_t *p_out,
            /*@null@*/bool *p_is_omitted
    ) {

        int32_t n;
        bool is_omitted;
        const char *p_pos_next;

        if ( p_pos >= p_tail ) {
            *p_out = default_value;
            return p_tail;
        }

        n = std::strtol(p_pos, const_cast<char**>(&p_pos_next), 10);

        is_omitted = (p_pos == p_pos_next);

        if ( is_omitted ) {

            n = default_value;
        }

        *p_out = SAT(n, min, max);

        if ( p_is_omitted != nullptr ) {

            *p_is_omitted = is_omitted;
        }

        if ( p_pos_next > p_tail ) {

            p_pos_next = p_tail;
        }

        return p_pos_next;
    }

    const char * read_number(
            const char *p_pos,
            const char *p_tail,
            int32_t min,
            int32_t max,
            int32_t default_value,
            int32_t *p_out
    ) {

        return read_number_ex(
                p_pos,
                p_tail,
                min,
                max,
                default_value,
                p_out,
                nullptr
        );
    }

    int32_t get_param(
            const char **pp_pos,
            const char *p_tail,
            int32_t min,
            int32_t max,
            int32_t default_value
    ) {

        int32_t r = 0;

        *pp_pos = read_number(
                *pp_pos+1,
                p_tail,
                min,
                max,
                default_value,
                &r
        );

        return r;
    }

    uint8_t get_tp_table_column_number(const char note_name) {

        uint8_t col_num;
        switch ( to_upper_case(note_name) ) {
        case 'C':
            col_num = 0;
            break;
        case 'D':
            col_num = 2;
            break;
        case 'E':
            col_num = 4;
            break;
        case 'F':
            col_num = 5;
            break;
        case 'G':
            col_num = 7;
            break;
        case 'A':
            col_num = 9;
            break;
        case 'B':
            col_num = 11;
            break;
        default:
            col_num = 0;
            break;
        }
        return col_num;
    }

    const char * shift_half_note_number(
            const char *p_pos,
            const char *p_tail,
            int16_t note_num,
            int16_t *p_out
    ) {

        const char *p;
        int16_t n;

        n = note_num;

        for ( p = p_pos; p < p_tail; p++ ) {

            if ( ( *p == '+' ) || ( *p == '#' ) ) {

                n = ( n < MAX_NOTE_NUMBER )
                  ? (n+1)
                  : MAX_NOTE_NUMBER;

            } else if ( *p == '-' ) {

                n = ( n > MIN_NOTE_NUMBER )
                  ? (n-1)
                  : MIN_NOTE_NUMBER;

            } else {

                break;
            }
        }

        n = SAT(n, MIN_NOTE_NUMBER, MAX_NOTE_NUMBER);

        *p_out = n;

        return p;
    }

    uint16_t get_sus_volume(const SLOT &slot, uint8_t ch) {

        const CHANNEL_INFO *p_ch_info = slot.ch_info_list[clamp_channel(ch)];
        uint32_t sus_volume;

        sus_volume =  (static_cast<uint32_t>(p_ch_info->sw_env.sustain) * p_ch_info->tone.VOLUME + 50)/100;

        if ( sus_volume > MAX_VOLUME_LEVEL ) {

            sus_volume = MAX_VOLUME_LEVEL;
        }
        return static_cast<uint16_t>(sus_volume);
    }

    void trans_sw_env_state(SLOT &slot, uint8_t ch) {

        uint32_t q12_time_factor;
        CHANNEL_INFO *p_ch_info = slot.ch_info_list[ch];

        if ( p_ch_info->time.sw_env != 0 ) {

            /* NO CHANGE STATE */
            return;
        }

        q12_time_factor = (100 << 12) / slot.gl_info.speed_factor;

        switch ( p_ch_info->ch_status.SW_ENV_STAT ) {

        case SW_ENV_STAT_INIT_NOTE_ON:
            if ( p_ch_info->sw_env.attack_tk != 0 ) {

                p_ch_info->sw_env.VOL_INT = 0;
                p_ch_info->sw_env.VOL_FRAC = 0;
                p_ch_info->time.sw_env = (static_cast<uint32_t>(p_ch_info->sw_env.attack_tk) * q12_time_factor + (1<<11)) >> 12;
                p_ch_info->ch_status.SW_ENV_STAT = SW_ENV_STAT_ATTACK;
                break;
            }
            /*@fallthrough@*/

        case SW_ENV_STAT_ATTACK:
            if ( p_ch_info->sw_env.hold_tk != 0 ) {

                p_ch_info->sw_env.VOL_INT  = p_ch_info->tone.VOLUME;
                p_ch_info->sw_env.VOL_FRAC = 0;
                p_ch_info->time.sw_env = (static_cast<uint32_t>(p_ch_info->sw_env.hold_tk) * q12_time_factor + (1<<11)) >> 12;
                p_ch_info->ch_status.SW_ENV_STAT = SW_ENV_STAT_HOLD;
                break;
            }
            /*@fallthrough@*/

        case SW_ENV_STAT_HOLD:
            if ( p_ch_info->sw_env.decay_tk != 0 ) {

                p_ch_info->sw_env.VOL_INT  = p_ch_info->tone.VOLUME;
                p_ch_info->sw_env.VOL_FRAC = 0;
                p_ch_info->time.sw_env = (static_cast<uint32_t>(p_ch_info->sw_env.decay_tk) * q12_time_factor + (1<<11)) >> 12;
                p_ch_info->ch_status.SW_ENV_STAT= SW_ENV_STAT_DECAY;
                break;
            }
            /*@fallthrough@*/

        case SW_ENV_STAT_DECAY:

            p_ch_info->sw_env.VOL_INT  = get_sus_volume(slot, ch);
            p_ch_info->sw_env.VOL_FRAC = 0;
            p_ch_info->time.sw_env = (static_cast<uint32_t>(p_ch_info->sw_env.fade_tk) * q12_time_factor + (1<<11)) >> 12;
            p_ch_info->ch_status.SW_ENV_STAT = SW_ENV_STAT_FADE;
            break;


        case SW_ENV_STAT_INIT_NOTE_OFF:
            if ( p_ch_info->sw_env.release_tk != 0 ) {

                p_ch_info->sw_env.REL_VOL_INT = p_ch_info->sw_env.VOL_INT;
                p_ch_info->sw_env.REL_VOL_FRAC = p_ch_info->sw_env.VOL_FRAC;
                p_ch_info->time.sw_env = (static_cast<uint32_t>(p_ch_info->sw_env.release_tk) * q12_time_factor + (1<<11)) >> 12;
                p_ch_info->ch_status.SW_ENV_STAT = SW_ENV_STAT_RELEASE;
                break;
            }
            /*@fallthrough@*/

        case SW_ENV_STAT_RELEASE:
            p_ch_info->sw_env.VOL_INT = 0;
            p_ch_info->sw_env.VOL_FRAC = 0;
            p_ch_info->ch_status.SW_ENV_STAT = SW_ENV_STAT_END;
            break;

        default:
            /* NO CHANGE */
            break;
        }
    }

    void init_pitchbend(SLOT &slot, uint8_t ch) {

        uint16_t tp_end;
        uint16_t tp_base;
        uint32_t q6_tp_d;
        CHANNEL_INFO *p_ch_info = slot.ch_info_list[ch];

        // Multiplying by `speed_factor` is unnecessary here since it is already reflected in `note_on`.
        p_ch_info->time.pitchbend = p_ch_info->time.note_on;
        if ( p_ch_info->time.pitchbend == 0 ) {

            p_ch_info->ch_status.PBEND_STAT = PBEND_STAT_STOP;
            return;
        }

        tp_base = U16(slot.psg_reg.data[2*ch+1], slot.psg_reg.data[2*ch])&0xFFF;

        tp_end = p_ch_info->pitchbend.TP_END_H;
        tp_end = tp_end<<4 | p_ch_info->pitchbend.TP_END_L;

        if ( tp_base < tp_end ) {

            p_ch_info->ch_status.PBEND_STAT = PBEND_STAT_TP_UP;
            q6_tp_d = tp_end - tp_base;
            q6_tp_d <<= 6;
            q6_tp_d /= p_ch_info->time.pitchbend;

        } else if ( tp_base > tp_end ) {

            p_ch_info->ch_status.PBEND_STAT = PBEND_STAT_TP_DOWN;
            q6_tp_d = tp_base - tp_end;
            q6_tp_d <<= 6;
            q6_tp_d /= p_ch_info->time.pitchbend;

        } else {

            p_ch_info->ch_status.PBEND_STAT = PBEND_STAT_STOP;
            q6_tp_d = 0;
        }

        p_ch_info->pitchbend.TP_D_INT = (q6_tp_d >> 6) & 0xFFF;
        p_ch_info->pitchbend.TP_D_FRAC = q6_tp_d & 0x3F;
    }

    void proc_pitchbend(SLOT &slot, uint8_t ch) {

        uint32_t q6_tp;
        uint32_t q6_tp_d;
        uint32_t q6_tp_end;
        uint16_t tp_int;
        CHANNEL_INFO *p_ch_info;

        p_ch_info = slot.ch_info_list[clamp_channel(ch)];

        if ( p_ch_info->time.pitchbend > 0 ) {

            p_ch_info->time.pitchbend--;

        } else {

            switch ( p_ch_info->ch_status.PBEND_STAT ) {

            case PBEND_STAT_TP_UP:/*@fallthrough@*/
            case PBEND_STAT_TP_DOWN:/*@fallthrough@*/
            case PBEND_STAT_END:
                p_ch_info->ch_status.PBEND_STAT = PBEND_STAT_END;
                break;

            case PBEND_STAT_STOP:/*@fallthrough@*/
            default:
                p_ch_info->ch_status.PBEND_STAT = PBEND_STAT_STOP;
                break;
            }
        }
        if ( ( p_ch_info->ch_status.PBEND_STAT == PBEND_STAT_STOP ) ||
             ( p_ch_info->ch_status.PBEND_STAT == PBEND_STAT_END  )
        ) {

            return;
        }

        q6_tp      = p_ch_info->pitchbend.TP_INT;
        q6_tp      = (q6_tp<<6)|p_ch_info->pitchbend.TP_FRAC;
        q6_tp_d    = p_ch_info->pitchbend.TP_D_INT;
        q6_tp_d    = (q6_tp_d<<6)|p_ch_info->pitchbend.TP_D_FRAC;
        q6_tp_end  = p_ch_info->pitchbend.TP_END_H;
        q6_tp_end  = (q6_tp_end<<4)|p_ch_info->pitchbend.TP_END_L;
        q6_tp_end  = q6_tp_end<<6;

        if ( p_ch_info->ch_status.PBEND_STAT == PBEND_STAT_TP_UP ) {

            if ( (q6_tp_end-q6_tp) <= q6_tp_d ) {

                q6_tp = q6_tp_end;
                p_ch_info->ch_status.PBEND_STAT = PBEND_STAT_END;

            } else {

                q6_tp += q6_tp_d;
            }

        } else if ( p_ch_info->ch_status.PBEND_STAT == PBEND_STAT_TP_DOWN ) {

            if ( (q6_tp-q6_tp_end) <= q6_tp_d ) {

                q6_tp = q6_tp_end;
                p_ch_info->ch_status.PBEND_STAT = PBEND_STAT_END;

            } else {

                q6_tp -= q6_tp_d;
            }

        } else {
        }

        tp_int = q6_tp>>6;
        slot.psg_reg.data[0x0+2*ch] = U16_LO(tp_int);
        slot.psg_reg.data[0x1+2*ch] = U16_HI(tp_int);
        slot.psg_reg.flags_addr    |= 0x3<<(2*ch);

        p_ch_info->pitchbend.TP_INT  = tp_int;
        p_ch_info->pitchbend.TP_FRAC = q6_tp&0x3F;
    }

    void init_noise_sweep(SLOT &slot, uint8_t ch) {

        uint16_t np_end;
        uint16_t np_base;
        uint16_t q6_np_d;
        NOISE_INFO *p_noise_info = &slot.gl_info.noise_info;
        const CHANNEL_INFO *p_ch_info = slot.ch_info_list[ch];

        p_noise_info->sweep_time = p_ch_info->time.note_on;
        if ( p_noise_info->sweep_time == 0 ) {

            p_noise_info->SWEEP_STAT = NOISE_SWEEP_STAT_STOP;
            return;
        }

        np_base = slot.psg_reg.data[0x6];
        np_end = p_noise_info->NP_END;

        if ( np_base < np_end ) {

            p_noise_info->SWEEP_STAT = NOISE_SWEEP_STAT_NP_UP;
            q6_np_d = np_end - np_base;
            q6_np_d <<= 6;
            q6_np_d /= p_noise_info->sweep_time;

        } else if ( np_base > np_end ) {

            p_noise_info->SWEEP_STAT = NOISE_SWEEP_STAT_NP_DOWN;
            q6_np_d = np_base - np_end;
            q6_np_d <<= 6;
            q6_np_d /= p_noise_info->sweep_time;

        } else {

            p_noise_info->SWEEP_STAT = NOISE_SWEEP_STAT_STOP;
            q6_np_d = 0;
        }

        p_noise_info->NP_INT = np_base & 0x1F;
        p_noise_info->NP_FRAC = 0;
        p_noise_info->NP_D_INT = (q6_np_d >> 6) & 0x1F;
        p_noise_info->NP_D_FRAC = q6_np_d & 0x3F;
    }

    void proc_noise_sweep(SLOT &slot) {

        uint32_t q6_np;
        uint32_t q6_np_d;
        uint32_t q6_np_end;
        uint8_t np_int;
        NOISE_INFO *p_noise_info = &slot.gl_info.noise_info;

        if ( p_noise_info->sweep_time > 0 ) {

            p_noise_info->sweep_time--;

        } else {

            switch ( p_noise_info->SWEEP_STAT ) {

            case NOISE_SWEEP_STAT_NP_UP:/*@fallthrough@*/
            case NOISE_SWEEP_STAT_NP_DOWN:/*@fallthrough@*/
            case NOISE_SWEEP_STAT_END:
                p_noise_info->SWEEP_STAT = NOISE_SWEEP_STAT_END;
                break;

            case NOISE_SWEEP_STAT_STOP:/*@fallthrough@*/
            default:
                p_noise_info->SWEEP_STAT = NOISE_SWEEP_STAT_STOP;
                break;
            }
        }

        if ( ( p_noise_info->SWEEP_STAT == NOISE_SWEEP_STAT_STOP ) ||
             ( p_noise_info->SWEEP_STAT == NOISE_SWEEP_STAT_END  )
        ) {

            return;
        }

        q6_np      = p_noise_info->NP_INT;
        q6_np      = (q6_np<<6)|p_noise_info->NP_FRAC;
        q6_np_d    = p_noise_info->NP_D_INT;
        q6_np_d    = (q6_np_d<<6)|p_noise_info->NP_D_FRAC;
        q6_np_end  = (static_cast<uint32_t>(p_noise_info->NP_END) << 6);

        if ( p_noise_info->SWEEP_STAT == NOISE_SWEEP_STAT_NP_UP ) {

            if ( (q6_np_end-q6_np) <= q6_np_d ) {

                q6_np = q6_np_end;
                p_noise_info->SWEEP_STAT = NOISE_SWEEP_STAT_END;

            } else {

                q6_np += q6_np_d;
            }

        } else if ( p_noise_info->SWEEP_STAT == NOISE_SWEEP_STAT_NP_DOWN ) {

            if ( (q6_np-q6_np_end) <= q6_np_d ) {
                q6_np = q6_np_end;
                p_noise_info->SWEEP_STAT = NOISE_SWEEP_STAT_END;

            } else {

                q6_np -= q6_np_d;
            }

        } else {
        }

        np_int = (q6_np>>6)&0x1F;
        slot.psg_reg.data[0x6] = np_int;
        slot.psg_reg.flags_addr |= 1<<0x6;

        p_noise_info->NP_INT  = np_int;
        p_noise_info->NP_FRAC = q6_np&0x3F;
    }

    const char * get_legato_end_note_num(
            const char *p_pos,
            const char *p_tail,
            int32_t default_octave,
            int32_t *p_out,
            int16_t start_note_num
    ) {

        const char *p;
        int32_t octave;
        int16_t note_num;
        bool is_end_detected = false;

        /* Change octave */
        octave = SAT(default_octave, MIN_OCTAVE, MAX_OCTAVE);

        for ( p = p_pos; p < p_tail; p++ ) {

            if ( *p == 'O' ) {

                const char *p_tmp;
                p_tmp = read_number(
                        p+1,
                        p_tail,
                        MIN_OCTAVE,
                        MAX_OCTAVE,
                        DEFAULT_OCTAVE,
                        &octave
                );
                p = (p_tmp - 1);

            } else if ( *p == '<' ) {

                octave = (octave > MIN_OCTAVE) ? (octave-1) : MIN_OCTAVE;

            } else if ( *p == '>' ) {

                octave = (octave < MAX_OCTAVE) ? (octave+1) : MAX_OCTAVE;

            } else if ( is_white_space(*p) ) {

                continue;

            } else if ( ('A' <= *p ) && ( *p <= 'G' ) ) {

                is_end_detected = true;
                break;

            } else {

                break;
            }
        }

        if ( is_end_detected ) {

            uint8_t col_num;

            col_num = get_tp_table_column_number(*p);
            note_num = static_cast<int32_t>(col_num&0xFFu) + (octave-1)*12;
            /* Shift Note-Number */
            shift_half_note_number(
                    p+1,
                    p_tail,
                    note_num,
                    &note_num
            );

        } else {

            note_num = start_note_num;
        }

        *p_out = note_num;

        return p_pos;
    }

    uint32_t get_note_on_time(
            uint8_t note_len,
            uint16_t tempo,
            uint8_t dot_cnt,
            uint16_t proc_freq
    ) {

        uint32_t q12_time, q12_time_delta;

        if ( note_len == 0 ) {

            return 0;
        }

        q12_time_delta = ((static_cast<uint32_t>(proc_freq)*4*60)<<12)/(static_cast<uint32_t>(tempo)*note_len);
        q12_time = q12_time_delta;

        while ( dot_cnt > 0 ) {

            q12_time_delta >>= 1;
            q12_time += q12_time_delta;
            dot_cnt--;
        }

        return q12_time;
    }

    void decode_dollar(
            CHANNEL_INFO *p_info,
            const char **pp_pos,
            const char *p_tail,
            uint16_t proc_freq
    ) {

        int32_t param;
        uint8_t dot_cnt;
        uint32_t q12_delay_tk;

        (*pp_pos)++;

        switch ( to_upper_case(**pp_pos) ) {

        case 'A':
            param = get_param(
                pp_pos,
                p_tail,
                MIN_SOFT_ENVELOPE_ATTACK,
                MAX_SOFT_ENVELOPE_ATTACK,
                DEFAULT_SOFT_ENVELOPE_ATTACK
            );
            p_info->sw_env.attack_tk = sw_env_time2tk(param, p_info->sw_env.time_unit, p_info->tone.tempo, proc_freq);
            break;

        case 'D':
            param = get_param(
                pp_pos,
                p_tail,
                MIN_SOFT_ENVELOPE_DECAY,
                MAX_SOFT_ENVELOPE_DECAY,
                DEFAULT_SOFT_ENVELOPE_DECAY
            );
            p_info->sw_env.decay_tk = sw_env_time2tk(param, p_info->sw_env.time_unit, p_info->tone.tempo, proc_freq);
            break;

        case 'E':
            param = get_param(
                pp_pos,
                p_tail,
                MIN_SW_ENV_MODE,
                MAX_SW_ENV_MODE,
                DEFAULT_SW_ENV_MODE
            );
            p_info->ch_status.SW_ENV_MODE = param;
            break;

        case 'F':
            param = get_param(
                pp_pos,
                p_tail,
                MIN_SOFT_ENVELOPE_FADE,
                MAX_SOFT_ENVELOPE_FADE,
                DEFAULT_SOFT_ENVELOPE_FADE
            );
            p_info->sw_env.fade_tk = sw_env_time2tk(param, p_info->sw_env.time_unit, p_info->tone.tempo, proc_freq);
            break;

        case 'H':
            param = get_param(
                pp_pos,
                p_tail,
                MIN_SOFT_ENVELOPE_HOLD,
                MAX_SOFT_ENVELOPE_HOLD,
                DEFAULT_SOFT_ENVELOPE_HOLD
            );
            p_info->sw_env.hold_tk = sw_env_time2tk(param, p_info->sw_env.time_unit, p_info->tone.tempo, proc_freq);
            break;

        case 'R':
            param = get_param(
                pp_pos,
                p_tail,
                MIN_SOFT_ENVELOPE_RELEASE,
                MAX_SOFT_ENVELOPE_RELEASE,
                DEFAULT_SOFT_ENVELOPE_RELEASE
            );
            p_info->sw_env.release_tk = sw_env_time2tk(param, p_info->sw_env.time_unit, p_info->tone.tempo, proc_freq);
            break;

        case 'S':
            param = get_param(
                pp_pos,
                p_tail,
                MIN_SOFT_ENVELOPE_SUSTAIN,
                MAX_SOFT_ENVELOPE_SUSTAIN,
                DEFAULT_SOFT_ENVELOPE_SUSTAIN
            );
            p_info->sw_env.sustain = param;
            break;

        case 'U':
            param = get_param(
                pp_pos,
                p_tail,
                MIN_SW_ENV_TIME_UNIT,
                MAX_SW_ENV_TIME_UNIT,
                DEFAULT_SW_ENV_TIME_UNIT
            );
            p_info->sw_env.time_unit = param;
            break;

        case 'V':
            param = get_param(
                pp_pos,
                p_tail,
                MIN_LFO_SPEED_UNIT,
                MAX_LFO_SPEED_UNIT,
                DEFAULT_LFO_SPEED_UNIT
            );
            p_info->lfo.speed_unit = param;
            break;

        case 'M':
            param = get_param(
                pp_pos,
                p_tail,
                MIN_LFO_MODE,
                MAX_LFO_MODE,
                DEFAULT_LFO_MODE
            );
            p_info->ch_status.LFO_MODE = param;
            break;

        case 'B':
            param = get_param(
                pp_pos,
                p_tail,
                MIN_BIAS_LEVEL,
                MAX_BIAS_LEVEL,
                DEFAULT_BIAS_LEVEL
            );
            p_info->tone.BIAS = param + BIAS_LEVEL_OFS;
            break;

        case 'O':
            param = get_param(
                pp_pos,
                p_tail,
                MIN_TP_OFS,
                MAX_TP_OFS,
                DEFAULT_TP_OFS
            );
            p_info->tone.tp_ofs = param;
            break;

        case 'L':
            param = get_param(
                pp_pos,
                p_tail,
                MIN_LFO_SPEED,
                MAX_LFO_SPEED,
                DEFAULT_LFO_SPEED
            );

            p_info->lfo.speed = get_lfo_speed(
                param,
                p_info->lfo.speed_unit,
                p_info->tone.tempo
            );
            break;

        case 'J':
            param = get_param(
                pp_pos,
                p_tail,
                MIN_LFO_DEPTH,
                MAX_LFO_DEPTH,
                DEFAULT_LFO_DEPTH
            );
            p_info->lfo.depth = param;
            break;

        case 'T':
            param = get_param(
                pp_pos,
                p_tail,
                MIN_LFO_DELAY,
                MAX_LFO_DELAY,
                DEFAULT_LFO_DELAY
            );

            dot_cnt = 0;
            if ( **pp_pos == '.' ) {

                *pp_pos = count_dot(*pp_pos, p_tail, &dot_cnt);
            }

            q12_delay_tk = get_note_on_time(
                    param,
                    p_info->tone.tempo,
                    dot_cnt,
                    proc_freq
            );
            p_info->lfo.delay_tk = (q12_delay_tk>>12)&0xFFFF;
            break;

        case 'P':
            param = get_param(
                pp_pos,
                p_tail,
                MIN_PITCHBEND_LEVEL,
                MAX_PITCHBEND_LEVEL,
                DEFAULT_PITCHBEND_LEVEL
            );
            p_info->pitchbend.level = param;
            break;

        default:
            (*pp_pos)++;
            break;
        }
    }

    void generate_tone(
            SLOT &slot,
            uint8_t ch,
            const char **pp_pos,
            const char *p_tail,
            uint32_t q12_exclude_note_len
    ) {

        const char *p_pos;
        int16_t note_num;
        int32_t legato_end_note_num;
        int32_t note_len;
        char head;
        uint8_t dot_cnt;
        bool is_note_len_omitted;
        bool is_start_legato_effect;
        CHANNEL_INFO *p_ch_info = slot.ch_info_list[ch];
        enum {
            E_NOTE_TYPE_TONE = 0,
            E_NOTE_TYPE_NOISE,
            E_NOTE_TYPE_REST,
            E_NOTE_TYPE_OTHER
        } note_type;

        p_pos = *pp_pos;
        head = to_upper_case(p_pos[0]);

        note_num = 0;
        legato_end_note_num = 0;
        note_len = 0;
        dot_cnt = 0;
        is_start_legato_effect = false;

        if ( ('A' <= head) && (head <= 'G') ) {

            uint8_t col_num;

            /* Set Note-Type */
            note_type = E_NOTE_TYPE_TONE;

            /* Get Note-Number */
            col_num = get_tp_table_column_number(head);
            note_num = col_num + static_cast<uint16_t>(p_ch_info->tone.OCTAVE)*12;

            /* Shift Note-Number */
            p_pos = shift_half_note_number(
                    p_pos+1,
                    p_tail,
                    note_num,
                    &note_num
            );

            /* Get Note-Length */
            is_note_len_omitted = false;
            p_pos = read_number_ex(
                    p_pos,
                    p_tail,
                    0 /* special case */,
                    MAX_NOTE_LENGTH,
                    p_ch_info->tone.note_len,
                    &note_len,
                    &is_note_len_omitted
            );

            /* Count Dot-Repetition */
            dot_cnt = 0;
            p_pos = count_dot(p_pos, p_tail, &dot_cnt);
            if ( is_note_len_omitted ) {
                // Use global note length
                dot_cnt += p_ch_info->tone.LEN_DOTS;
            }

            /* Legato */
            if ( *p_pos == '&' ) {

                is_start_legato_effect = true;
                p_pos = get_legato_end_note_num(
                        p_pos+1,
                        p_tail,
                        static_cast<int32_t>(p_ch_info->tone.OCTAVE)+1,
                        &legato_end_note_num,
                        note_num
                );

            } else {

                is_start_legato_effect = false;
            }

        } else if ( head == 'N' ) {

            /* Set Note-Type */
            note_type = E_NOTE_TYPE_TONE;

            /* Get Note-Number */
            note_num = get_param(
                           &p_pos,
                           p_tail,
                           MIN_NOTE_NUMBER,
                           MAX_NOTE_NUMBER,
                           DEFAULT_NOTE_NUMBER
            );

            /* Count Dot-Repetition */
            dot_cnt = 0;
            p_pos = count_dot(p_pos, p_tail, &dot_cnt);

            note_len = p_ch_info->tone.note_len;
            dot_cnt += p_ch_info->tone.LEN_DOTS;

        } else if ( (head == 'J') || (head == 'H') || (head == 'R') ) {

            /* Set Note-Type */
            note_type = ( head == 'R' )
                      ? E_NOTE_TYPE_REST
                      : E_NOTE_TYPE_NOISE;

            if ( head == 'J' ) {

                int32_t np_base;
                int32_t np_end;
                np_base = slot.gl_info.noise_info.NP_I;

                np_end = np_base;

                /* Set Note-Type */
                note_type = E_NOTE_TYPE_NOISE;

                /* Get NP */
                p_pos = read_number(
                        p_pos+1,
                        p_tail,
                        MIN_NOISE_NP,
                        MAX_NOISE_NP,
                        np_base,
                        &np_base
                );
                /* Sweep */
                if ( *p_pos == '~' ) {

                    p_pos = read_number(
                            p_pos+1,
                            p_tail,
                            MIN_NOISE_NP,
                            MAX_NOISE_NP,
                            np_base,
                            &np_end
                    );

                } else {

                    np_end = np_base;
                }
                slot.psg_reg.data[0x6] = np_base&0x1F;
                slot.psg_reg.flags_addr |= 1<<0x6;

                slot.gl_info.noise_info.NP_END = np_end&0x1F;

                /* Count Dot-Repetition */
                dot_cnt = 0;
                p_pos = count_dot(p_pos, p_tail, &dot_cnt);

                note_len = p_ch_info->tone.note_len;
                dot_cnt += p_ch_info->tone.LEN_DOTS;

            } else {

                bool is_use_global_note_len;

                if ( note_type == E_NOTE_TYPE_NOISE ) {

                    slot.gl_info.noise_info.NP_END = slot.gl_info.noise_info.NP_I;
                    slot.psg_reg.data[0x6] = slot.gl_info.noise_info.NP_I;
                    slot.psg_reg.flags_addr |= 1<<0x6;
                }

                /* Get Note-Length */
                if ( slot.gl_info.sys_status.RH_LEN != 0 ) {

                    is_note_len_omitted = false;
                    p_pos = read_number_ex(
                            p_pos+1,
                            p_tail,
                            MIN_NOTE_LENGTH,
                            MAX_NOTE_LENGTH,
                            p_ch_info->tone.note_len,
                            &note_len,
                            &is_note_len_omitted
                    );

                    is_use_global_note_len = is_note_len_omitted;

                } else {

                    is_use_global_note_len = false;
                    p_pos = read_number(
                            p_pos+1,
                            p_tail,
                            MIN_NOTE_LENGTH,
                            MAX_NOTE_LENGTH,
                            DEFAULT_NOTE_LENGTH,
                            &note_len
                    );
                }

                /* Count Dot-Repetition */
                dot_cnt = 0;
                p_pos = count_dot(p_pos, p_tail, &dot_cnt);

                if ( is_use_global_note_len ) {

                    dot_cnt += p_ch_info->tone.LEN_DOTS;
                }
            }

        } else {

            note_type = E_NOTE_TYPE_OTHER;
        }

        /* Update current MML pos */
        *pp_pos = p_pos;

        if ( note_type == E_NOTE_TYPE_TONE ) {

            uint16_t tp;
            uint16_t tp_end;
            int16_t bias;
            bias = static_cast<int16_t>(p_ch_info->tone.BIAS) - BIAS_LEVEL_OFS;
            /* Apply bias-level to tp. */
            tp = shift_tp(calc_tp(note_num, slot.gl_info.s_clock), bias);

            /* Apply shift-degs to tp. */
            tp = shift_tp(tp, slot.gl_info.shift_degrees);

            /* Apply the TP offset to the BIAS calculation result for fine adjustments, such as detuning. */
            tp = static_cast<uint16_t>(SAT(static_cast<int16_t>(tp) + static_cast<int16_t>(p_ch_info->tone.tp_ofs), MIN_TP, MAX_TP));

            if ( is_start_legato_effect ) {

                tp_end = shift_tp(calc_tp(legato_end_note_num, slot.gl_info.s_clock), bias);

                /* Apply shift-degs to tp_end. */
                tp_end = shift_tp(tp_end, slot.gl_info.shift_degrees);

                /* Apply the TP offset to the BIAS calculation result for fine adjustments, such as detuning. */
                tp_end = static_cast<uint16_t>(SAT(static_cast<int16_t>(tp_end) + static_cast<int16_t>(p_ch_info->tone.tp_ofs), MIN_TP, MAX_TP));

            } else {

                tp_end = shift_tp(tp, p_ch_info->pitchbend.level);
            }

            slot.psg_reg.data[2*ch]     = U16_LO(tp);
            slot.psg_reg.data[2*ch+1]   = U16_HI(tp);
            slot.psg_reg.flags_addr    |= 0x3<<(2*ch);

            p_ch_info->pitchbend.TP_INT = tp;
            p_ch_info->pitchbend.TP_FRAC =0;
            p_ch_info->pitchbend.TP_END_L = tp_end&0xF;
            p_ch_info->pitchbend.TP_END_H = (tp_end>>4)&0xFF;

            if ( p_ch_info->ch_status.LFO_MODE != LFO_MODE_OFF ) {

                p_ch_info->ch_status.LFO_STAT = LFO_STAT_RUN;

                if ( p_ch_info->ch_status.LEGATO == 0 ) {

                    uint32_t q12_time_factor;
                    q12_time_factor = (100 << 12) / slot.gl_info.speed_factor;
                    p_ch_info->time.lfo_delay = (static_cast<uint32_t>(p_ch_info->lfo.delay_tk) * q12_time_factor + (1<<11)) >> 12;
                    p_ch_info->lfo.theta = 0;
                    p_ch_info->lfo.DELTA_FRAC = 0;
                    p_ch_info->lfo.TP_FRAC = 0;
                    p_ch_info->lfo.BASE_TP_H = (tp>>8)&0xF;
                    p_ch_info->lfo.BASE_TP_L = tp&0xFF;
                }

            } else {

                p_ch_info->ch_status.LFO_STAT = LFO_STAT_STOP;
            }
        }

        if ( ( note_type == E_NOTE_TYPE_TONE ) ||
             ( note_type == E_NOTE_TYPE_NOISE )
        ) {

            slot.psg_reg.data[0x8+ch] = p_ch_info->tone.VOLUME;
            slot.psg_reg.flags_addr  |= 1<<(0x8+ch);
            if ( p_ch_info->tone.HW_ENV != 0 ) {

                slot.psg_reg.data[0x8+ch] |= 1<<4;

                if ( p_ch_info->ch_status.LEGATO == 0 ) {

                    slot.psg_reg.flags_addr |= 0x7<<0xB;
                }

            } else {

                slot.psg_reg.data[0x8+ch] &= ~(1<<4);
            }
        }

        if ( p_ch_info->ch_status.SW_ENV_MODE != SW_ENV_MODE_OFF ) {

            if ( note_type == E_NOTE_TYPE_REST ) {

                if ( p_ch_info->ch_status.SW_ENV_STAT < SW_ENV_STAT_INIT_NOTE_OFF ) {

                    p_ch_info->time.sw_env = 0;
                    p_ch_info->ch_status.SW_ENV_STAT = SW_ENV_STAT_INIT_NOTE_OFF;
                    trans_sw_env_state(slot, ch);
                }

            } else {

                if ( ( p_ch_info->ch_status.LEGATO == 0 ) &&
                     ( note_len != 0 )
                ) {

                    p_ch_info->time.sw_env = 0;
                    p_ch_info->ch_status.SW_ENV_STAT = SW_ENV_STAT_INIT_NOTE_ON;
                    trans_sw_env_state(slot, ch);
                }
            }
        }

        /* Note-ON */
        if ( note_len != 0 ) {

            uint32_t q12_note_on_time;
            uint32_t q12_gate_time;
            uint8_t req_mixer;
            bool is_update_mixer;
            uint32_t tempo = (static_cast<uint32_t>(p_ch_info->tone.tempo) * slot.gl_info.speed_factor + 50)/ 100;

            q12_note_on_time  = get_note_on_time(
                    note_len,
                    tempo,
                    dot_cnt,
                    slot.gl_info.proc_freq
            );

            q12_note_on_time += p_ch_info->time.NOTE_ON_FRAC;
            if ( q12_note_on_time > q12_exclude_note_len ) {
                q12_note_on_time -= q12_exclude_note_len;
            } else {
                q12_note_on_time = p_ch_info->time.NOTE_ON_FRAC;
            }

            p_ch_info->time.NOTE_ON_FRAC = q12_note_on_time&0xFFF;
            p_ch_info->time.note_on = (q12_note_on_time>>12)&0xFFFF;

            q12_gate_time = (q12_note_on_time * (static_cast<uint32_t>(p_ch_info->tone.GATE_TIME)+1))/MAX_GATE_TIME;
            p_ch_info->time.gate = (q12_gate_time>>12)&0xFFFF;

            is_update_mixer = false;
            if ( note_type == E_NOTE_TYPE_TONE ) {

                req_mixer = 0x08;
                is_update_mixer = true;

            } else if ( note_type == E_NOTE_TYPE_NOISE ) {

                req_mixer = 0x01;
                is_update_mixer = true;
                init_noise_sweep(slot, ch);

            } else if ( note_type == E_NOTE_TYPE_REST ) {

                req_mixer = 0x09;
                if ( p_ch_info->ch_status.SW_ENV_MODE != SW_ENV_MODE_OFF ) {

                    if ( p_ch_info->ch_status.SW_ENV_STAT != SW_ENV_STAT_RELEASE ) {
                        is_update_mixer = true;
                    }

                } else {

                    is_update_mixer = true;
                }

            } else {

                req_mixer = 0x09;
                is_update_mixer = true;
            }

            if ( is_update_mixer ) {

                slot.psg_reg.data[0x7] &= ~(0x9<<ch);
                slot.psg_reg.data[0x7] |= req_mixer<<ch;
                slot.psg_reg.flags_addr  |= 1<<0x7;
                slot.psg_reg.flags_mixer |= 1<<ch;
            }
        }

        init_pitchbend(slot, ch);

        p_ch_info->ch_status.LEGATO = is_start_legato_effect ? 1 : 0;
    }

    int16_t decode_mml(SLOT &slot, uint8_t ch) {

        const char *p_pos;
        const char *p_head;
        const char *p_tail;
        int32_t param;
        uint32_t q12_exclude_note_len = static_cast<uint32_t>(DEFAULT_EXCLUDE_NOTE_LEN)<<12;
        bool loop_flag = false;
        bool decode_cont = true;
        CHANNEL_INFO *p_ch_info = slot.ch_info_list[ch];

        p_head =  p_ch_info->mml.p_mml_head;
        p_pos  = &p_ch_info->mml.p_mml_head[p_ch_info->mml.ofs_mml_pos];
        p_tail = &p_ch_info->mml.p_mml_head[p_ch_info->mml.mml_len];

        if ( p_pos >= p_tail ) {

            p_ch_info->ch_status.DECODE_END = 1;

            return 1;
        }

        while ( decode_cont ) {

            switch ( to_upper_case(p_pos[0]) ) {

            case 'A':/*@fallthrough@*/
            case 'B':/*@fallthrough@*/
            case 'C':/*@fallthrough@*/
            case 'D':/*@fallthrough@*/
            case 'E':/*@fallthrough@*/
            case 'F':/*@fallthrough@*/
            case 'G':/*@fallthrough@*/
            case 'H':/*@fallthrough@*/
            case 'J':/*@fallthrough@*/
            case 'N':/*@fallthrough@*/
            case 'R':
                generate_tone(slot, ch, &p_pos, p_tail, q12_exclude_note_len);
                q12_exclude_note_len = static_cast<uint32_t>(DEFAULT_EXCLUDE_NOTE_LEN)<<12;
                decode_cont = false;
                break;

            case '$':
                decode_dollar(p_ch_info, &p_pos, p_tail, slot.gl_info.proc_freq);
                break;
            case '@':
                decode_atsign(slot, ch, &p_pos, p_tail);
                break;

            case 'X':
            {
                uint8_t dot_cnt = 0;
                uint32_t tempo = (static_cast<uint32_t>(p_ch_info->tone.tempo) * slot.gl_info.speed_factor + 50)/ 100;
                param = get_param(
                    &p_pos,
                    p_tail,
                    MIN_EXCLUDE_NOTE_LEN,
                    MAX_EXCLUDE_NOTE_LEN,
                    DEFAULT_EXCLUDE_NOTE_LEN
                );
                if ( *p_pos == '.' ) {

                    p_pos = count_dot(p_pos, p_tail, &dot_cnt);
                }
                q12_exclude_note_len = get_note_on_time(
                        param,
                        tempo,
                        dot_cnt,
                        slot.gl_info.proc_freq
                );
                break;
            }

            case 'T':
                param = get_param(
                    &p_pos,
                    p_tail,
                    MIN_TEMPO,
                    MAX_TEMPO,
                    DEFAULT_TEMPO
                );
                p_ch_info->tone.tempo = param;
                break;

            case 'V':
                param = get_param(
                    &p_pos,
                    p_tail,
                    MIN_VOLUME_LEVEL,
                    MAX_VOLUME_LEVEL,
                    DEFAULT_VOLUME_LEVEL
                );
                p_ch_info->tone.HW_ENV = 0;
                p_ch_info->tone.VOLUME = param;
                break;

            case 'S':
                param = get_param(
                    &p_pos,
                    p_tail,
                    MIN_ENVELOP_SHAPE,
                    MAX_ENVELOP_SHAPE,
                    DEFAULT_ENVELOP_SHAPE
                );
                slot.psg_reg.data[0xD]    = param;
                slot.psg_reg.flags_addr  |= 1<<0xD;
                p_ch_info->tone.HW_ENV = 1;
                break;

            case 'M':
                param = get_param(
                    &p_pos,
                    p_tail,
                    MIN_ENVELOP_EP,
                    MAX_ENVELOP_EP,
                    DEFAULT_ENVELOP_EP
                );
                slot.psg_reg.data[0xB]    = U16_LO(param);
                slot.psg_reg.data[0xC]    = U16_HI(param);
                slot.psg_reg.flags_addr  |= (0x3<<0xB);
                break;

            case 'L':
                param = get_param(
                    &p_pos,
                    p_tail,
                    MIN_NOTE_LENGTH,
                    MAX_NOTE_LENGTH,
                    DEFAULT_NOTE_LENGTH
                );
                p_ch_info->tone.note_len = param;

                /* Clear global dot counter */
                p_ch_info->tone.LEN_DOTS = 0;
                if ( *p_pos == '.' ) {

                    uint8_t dot_cnt = 0;
                    p_pos = count_dot(p_pos, p_tail, &dot_cnt);
                    p_ch_info->tone.LEN_DOTS = dot_cnt;
                }
                break;

            case 'O':
                param = get_param(
                    &p_pos,
                    p_tail,
                    MIN_OCTAVE,
                    MAX_OCTAVE,
                    DEFAULT_OCTAVE
                );
                p_ch_info->tone.OCTAVE = param-1;
                break;

            case 'Q':
                param = get_param(
                    &p_pos,
                    p_tail,
                    MIN_GATE_TIME,
                    MAX_GATE_TIME,
                    DEFAULT_GATE_TIME
                );
                p_ch_info->tone.GATE_TIME = param-1;
                break;

            case 'I':
                param = get_param(
                    &p_pos,
                    p_tail,
                    MIN_NOISE_NP,
                    MAX_NOISE_NP,
                    DEFAULT_NOISE_NP
                );
                slot.gl_info.noise_info.NP_I = param;
                break;

            case '<':
                if ( p_ch_info->tone.OCTAVE > (MIN_OCTAVE-1) ) {

                    p_ch_info->tone.OCTAVE--;
                }
                p_pos++;
                break;

            case '>':
                if ( p_ch_info->tone.OCTAVE < (MAX_OCTAVE-1) ) {

                    p_ch_info->tone.OCTAVE++;
                }
                p_pos++;
                break;

            case '[':
                if ( p_ch_info->ch_status.LOOP_DEPTH < MAX_LOOP_NESTING_DEPTH ) {

                    uint16_t loop_index = 0;
                    loop_flag = true;
                    loop_index = p_ch_info->ch_status.LOOP_DEPTH;

                    param = get_param(
                              &p_pos,
                              p_tail,
                              MIN_LOOP_TIMES,
                              MAX_LOOP_TIMES,
                              DEFAULT_LOOP_TIMES
                    );
                    p_ch_info->mml.loop_times[loop_index] = param;
                    p_ch_info->mml.ofs_mml_loop_head[loop_index] = (p_pos-p_head);
                    p_ch_info->ch_status.LOOP_DEPTH = loop_index + 1;

                } else {

                    p_pos++;
                }
                break;

            case '|':
                if ( p_ch_info->ch_status.LOOP_DEPTH > 0 ) {
                    bool skip_flag = false;
                    skip_flag |= (p_ch_info->mml.loop_times[p_ch_info->ch_status.LOOP_DEPTH - 1] == 1);
                    skip_flag |= (
                            ( p_ch_info->ch_status.LOOP_DEPTH == 1 ) &&
                            ( p_ch_info->ch_status.END_PRI_LOOP == 1 ) &&
                            ( slot.gl_info.sys_request.FIN_PRI_LOOP_REQ == FIN_PRI_LOOP_REQ_FORCE )
                    );

                    if ( skip_flag ) {

                        uint16_t loop_depth = p_ch_info->ch_status.LOOP_DEPTH;
                        const uint16_t loop_end = loop_depth - 1;

                        for ( p_pos = p_pos+1 ; p_pos < p_tail; p_pos++ ) {

                            if ( p_pos[0] == '[' ) {

                                loop_depth++;

                            } else if ( p_pos[0] == ']' ) {

                                loop_depth--;

                            } else {
                            }

                            if ( loop_depth == loop_end ) {

                                break;
                            }
                        }

                    } else {

                        p_pos++;
                    }

                } else {

                    p_pos++;
                }
                break;

            case ']':
                if ( p_ch_info->ch_status.LOOP_DEPTH > 0 ) {

                    bool loop_exit_flag = false;
                    uint16_t loop_index = 0;
                    loop_index = p_ch_info->ch_status.LOOP_DEPTH - 1;

                    loop_exit_flag |= loop_flag; // Force exit from the loop because there is no message in this loop.
                    loop_exit_flag |= ( p_ch_info->mml.loop_times[loop_index] == 1 );
                    loop_exit_flag |= (
                            ( p_ch_info->ch_status.LOOP_DEPTH == 1 ) &&
                            ( p_ch_info->ch_status.END_PRI_LOOP == 1 ) &&
                            ( slot.gl_info.sys_request.FIN_PRI_LOOP_REQ == FIN_PRI_LOOP_REQ_FORCE )
                    );// Exit the primary loop with the force flag.

                    if ( loop_exit_flag ) {

                        if ( p_ch_info->ch_status.LOOP_DEPTH == 1 ) {

                            p_ch_info->mml.prim_loop_counter = 0;
                        }
                        p_ch_info->mml.loop_times[loop_index] = 0;
                        p_ch_info->ch_status.LOOP_DEPTH = loop_index;
                        p_pos++;

                    } else {

                        if ( p_ch_info->mml.loop_times[loop_index] > 1 ) {

                            p_ch_info->mml.loop_times[loop_index]--;

                        } else {

                            /* Infinite loop */
                        }

                        /* Exit the primary loop without using the force flag. */
                        if ( p_ch_info->ch_status.LOOP_DEPTH == 1 ) {

                            if ( p_ch_info->ch_status.END_PRI_LOOP == 1 ) {

                                p_ch_info->ch_status.END_PRI_LOOP = 0;
                                p_ch_info->mml.loop_times[loop_index] = 1;
                            }
                            p_ch_info->mml.prim_loop_counter++;
                        }

                        p_pos = p_head + p_ch_info->mml.ofs_mml_loop_head[loop_index];
                    }

                } else {

                    p_pos++;
                }
                break;

            default:
                p_pos++;
                break;
            }

            if ( p_pos >= p_tail ) {

                p_ch_info->mml.ofs_mml_pos = static_cast<uint16_t>(p_pos - p_head);

                p_ch_info->ch_status.DECODE_END = 1;

                return 1;
            }

            p_ch_info->mml.ofs_mml_pos = static_cast<uint16_t>(p_pos - p_head);
        }

        return 0;
    }

    void decode_atsign(
            SLOT &slot,
            uint8_t ch,
            const char **pp_pos,
            const char *p_tail
    ) {

        int32_t param;
        (*pp_pos)++;
        if ( *pp_pos >= p_tail ) {

            return;
        }

        switch ( to_upper_case(**pp_pos) ) {

        case 'C':
            if ( *(*pp_pos+1) == '(' ) {

                param = static_cast<int32_t>(std::strtol((*pp_pos+2), const_cast<char**>(pp_pos), 0));

            } else {

                param = static_cast<int32_t>(std::strtol((*pp_pos+1), const_cast<char**>(pp_pos), 10));
            }

            if ( slot.cb_info.user_callback ) {

                slot.cb_info.user_callback(ch, param);
            }
            break;

        default:
            (*pp_pos)++;
            break;
        }
    }

    void update_sw_env_volume(SLOT &slot, uint8_t ch) {

        uint16_t vol, rel_vol;
        uint16_t top;
        uint16_t sus;
        int32_t rate;
        CHANNEL_INFO *p_ch_info = slot.ch_info_list[ch];

        vol = p_ch_info->sw_env.VOL_INT;
        vol = (vol << 12)|p_ch_info->sw_env.VOL_FRAC;
        rel_vol = p_ch_info->sw_env.REL_VOL_INT;
        rel_vol = (rel_vol << 12)|p_ch_info->sw_env.REL_VOL_FRAC;
        top = p_ch_info->tone.VOLUME;
        top = top<<12;
        sus = get_sus_volume(slot, ch);
        sus = sus<<12;

        switch ( p_ch_info->ch_status.SW_ENV_STAT ) {

        case SW_ENV_STAT_ATTACK:
            rate = top/p_ch_info->sw_env.attack_tk;
            if ( rate != 0 ) {

                if ( static_cast<int32_t>(top - vol) <= rate ) {

                    vol = top;

                } else {

                    vol += rate;
                }

            } else {

                vol = top;
            }
            break;

        case SW_ENV_STAT_HOLD:
            vol = top;
            break;

        case SW_ENV_STAT_DECAY:
            rate = static_cast<int32_t>(top-sus)/p_ch_info->sw_env.decay_tk;
            if ( rate > 0 ) {

                if ( static_cast<int32_t>(vol-sus) <= rate ) {

                    vol = sus;

                } else {

                    vol -= rate;
                }
            }
            else if ( rate < 0 ) {

                if ( static_cast<int32_t>(vol-sus) >= rate ) {

                    vol = sus;

                } else {

                    vol -= rate;
                }

            } else {

                vol = sus;
            }
            break;

        case SW_ENV_STAT_FADE:
            if ( p_ch_info->sw_env.fade_tk != 0 ) {

                rate = sus/p_ch_info->sw_env.fade_tk;

            } else {

                rate = 0;
            }

            if ( rate != 0 ) {

                if ( vol <= rate ) {

                    vol = 0;

                } else {

                    vol -= rate;
                }

            } else {

                vol = sus;
            }
            break;

        case SW_ENV_STAT_RELEASE:
            if ( p_ch_info->sw_env.release_tk != 0 ) {

                rate = rel_vol/p_ch_info->sw_env.release_tk;

            } else {

                rate = 0;
            }

            if ( rate != 0 ) {

                if ( vol <= rate ) {

                    vol = 0;

                } else {

                    vol -= rate;
                }

            } else {

                vol = 0;
            }
            break;

        default:
            break;
        }

        p_ch_info->sw_env.VOL_INT  = (vol>>12)&0xF;
        p_ch_info->sw_env.VOL_FRAC = vol&0xFFF;
    }

    void proc_sw_env_gen(SLOT &slot, uint8_t ch) {

        uint8_t vol;
        CHANNEL_INFO *p_ch_info = slot.ch_info_list[ch];
        if ( p_ch_info->time.sw_env > 0 ) {

            p_ch_info->time.sw_env--;
        }
        vol = slot.psg_reg.data[0x08+ch]&0xF;
        if ( vol != p_ch_info->sw_env.VOL_INT ) {

            slot.psg_reg.data[0x8+ch] = p_ch_info->sw_env.VOL_INT;
            if ( ( (slot.psg_reg.data[0x7]>>ch) & 0x9 ) != 0x9 ) {

                /* Not muted. */
                slot.psg_reg.flags_addr |= 1<<(0x8+ch);
            }
        }

        trans_sw_env_state(slot, ch);

        update_sw_env_volume(slot, ch);
    }

    void proc_lfo(SLOT &slot, uint8_t ch) {

        uint8_t tp_hi;
        uint8_t tp_lo;
        uint16_t speed_abs;
        bool is_phase_inverted;
        uint16_t delta_int;
        uint32_t tp_next;
        uint32_t q6_omega;
        uint32_t q6_delta;
        CHANNEL_INFO *p_ch_info = slot.ch_info_list[ch];

        if ( p_ch_info->ch_status.LFO_STAT != LFO_STAT_RUN ) {

            return;
        }

        if ( p_ch_info->time.lfo_delay > 0 ) {

            p_ch_info->time.lfo_delay--;
            return;
        }

        tp_hi = slot.psg_reg.data[2*ch+1];
        tp_lo = slot.psg_reg.data[2*ch+0];
        tp_next = U16(tp_hi, tp_lo);

        if ( p_ch_info->lfo.speed > 0 ) {

            /* Inverted the phase when the speed is positive to maintain compatibility. */
            is_phase_inverted = true;
            speed_abs = p_ch_info->lfo.speed;

        } else {

            is_phase_inverted = false;
            speed_abs = p_ch_info->lfo.speed * -1;
        }
        speed_abs = (static_cast<uint32_t>(speed_abs) * slot.gl_info.speed_factor + 50)/100;

        q6_omega = 1<<6;
        q6_omega *= static_cast<uint32_t>(p_ch_info->lfo.depth)*4*speed_abs;
        q6_omega /= (static_cast<uint16_t>(slot.gl_info.proc_freq)*MAX_LFO_PERIOD);

        q6_delta = p_ch_info->lfo.DELTA_FRAC;

        q6_delta += q6_omega;
        delta_int = (q6_delta >> 6);

        if ( delta_int > 0 ) {

            uint16_t theta = p_ch_info->lfo.theta;
            uint16_t depth = p_ch_info->lfo.depth;

            for ( uint16_t i = 0; i < delta_int; i++ ) {

                uint64_t lq24_tp;

                lq24_tp = static_cast<uint64_t>(tp_next)<<6;
                lq24_tp += p_ch_info->lfo.TP_FRAC;
                lq24_tp = lq24_tp << 18;
                if ( (depth <= theta) && ( theta < (depth*3))) {

                    lq24_tp = (lq24_tp*(is_phase_inverted ? Q_PITCHBEND_FACTOR_N : Q_PITCHBEND_FACTOR))>>24;

                } else {

                    if ( ( theta == 0 ) &&
                         ( p_ch_info->ch_status.PBEND_STAT == PBEND_STAT_STOP )
                    ) {

                        uint16_t tp_base;
                        tp_base = p_ch_info->lfo.BASE_TP_H;
                        tp_base = (tp_base<<8)|p_ch_info->lfo.BASE_TP_L;
                        lq24_tp = static_cast<uint64_t>(tp_base)<<24;
                        q6_delta = 0;

                    } else {

                        lq24_tp = (lq24_tp*(is_phase_inverted ? Q_PITCHBEND_FACTOR : Q_PITCHBEND_FACTOR_N))>>24;
                    }
                }

                p_ch_info->lfo.TP_FRAC = (lq24_tp>>18)&0x3F;
                tp_next = (lq24_tp>>24)&0xFFF;

                theta++;
                if ( theta >= depth*4 ) {

                    theta = 0;
                }
            }

            p_ch_info->lfo.theta = theta;

            slot.psg_reg.data[2*ch+0] = U16_LO(tp_next);
            slot.psg_reg.data[2*ch+1] = U16_HI(tp_next);
            if ( ( (slot.psg_reg.data[0x7]>>ch) & 0x9 ) != 0x9 ) {

                /* Not muted. */
                slot.psg_reg.flags_addr  |= 0x3<<(2*ch);
            }
        }
        p_ch_info->lfo.DELTA_FRAC = q6_delta&0x3F;
    }

    void reset_ch_info(CHANNEL_INFO *p_ch_info) {

        *p_ch_info = (CHANNEL_INFO){};
        p_ch_info->tone.tempo = DEFAULT_TEMPO;
        p_ch_info->tone.note_len = DEFAULT_NOTE_LENGTH;
        p_ch_info->tone.OCTAVE = DEFAULT_OCTAVE-1;
        p_ch_info->tone.GATE_TIME = DEFAULT_GATE_TIME-1;
        p_ch_info->tone.VOLUME = DEFAULT_VOLUME_LEVEL;
        p_ch_info->tone.BIAS = DEFAULT_BIAS_LEVEL+BIAS_LEVEL_OFS;
        p_ch_info->tone.tp_ofs = DEFAULT_TP_OFS;
    }

    void reset_psg(PSG_REG &psg_reg) {

        for ( uint8_t i = 0; i < 16; i++ ) {

            psg_reg.data[i] = 0;
        }
        psg_reg.data[0x7]   = 0x3F;
    }

    void rewind_mml(SLOT &slot) {

        slot.psg_reg.data[0x7]   = 0x3F;
        slot.psg_reg.flags_addr  = 1<<0x7;
        slot.psg_reg.flags_mixer = 0;
        slot.gl_info.sys_request.FIN_PRI_LOOP_REQ_FLAG = 0;
        slot.gl_info.sys_request.FIN_PRI_LOOP_REQ = FIN_PRI_LOOP_REQ_NORMAL;

        for ( uint8_t i = 0; i < slot.gl_info.sys_status.NUM_CH_USED; i++ ) {

            const char *p_mml_head;
            uint8_t ch;
            uint16_t mml_len;

            CHANNEL_INFO *p_ch_info;

            ch = clamp_channel(
                    slot.gl_info.sys_status.REVERSE == 1 ?
                    (NUM_CHANNEL-(i+1)) : i
            );
            slot.psg_reg.flags_mixer |= (1<<ch);

            p_ch_info = slot.ch_info_list[ch];

            p_mml_head = p_ch_info->mml.p_mml_head;
            mml_len = p_ch_info->mml.mml_len;

            reset_ch_info(p_ch_info);

            p_ch_info->mml.p_mml_head = p_mml_head;
            p_ch_info->mml.mml_len = mml_len;
            p_ch_info->ch_status.END_PRI_LOOP = 0;
        }
    }
}

    void init_slot(
            SLOT &slot,
            uint32_t s_clock,
            uint16_t proc_freq,
            bool reverse,
            CHANNEL_INFO *p_ch0,
            CHANNEL_INFO *p_ch1,
            CHANNEL_INFO  *p_ch2
    ) {

        CHANNEL_INFO *p_list[NUM_CHANNEL] = { p_ch0, p_ch1, p_ch2 };

        slot = (SLOT){};

        slot.gl_info.s_clock = s_clock;
        slot.gl_info.sys_status.REVERSE = reverse ? 1 : 0;
        slot.gl_info.sys_status.NUM_CH_IMPL = 0;
        slot.gl_info.proc_freq = (proc_freq != 0) ? proc_freq : PsgCtrl::DEFAULT_PROC_FREQ;
        slot.gl_info.speed_factor = DEFAULT_SPEED_FACTOR;

        slot.cb_info.user_callback = nullptr;

        for ( uint8_t i = 0; i < NUM_CHANNEL; i++ ) {

            slot.ch_info_list[
                clamp_channel( reverse ? (NUM_CHANNEL-(i+1)) : i )
            ] = p_list[i];

            if ( p_list[i] != nullptr ) {

                reset_ch_info(p_list[i]);
                slot.gl_info.sys_status.NUM_CH_IMPL++;

            } else {

                break;
            }
        }

        reset_psg(slot.psg_reg);
    }

    int set_mml(SLOT &slot, const char *p_mml, uint16_t mode) {

        if ( p_mml == nullptr ) {

            return -1;
        }

        skip_white_space(&p_mml);

        /* Set default values. */
        slot.gl_info.mml_version = DEFAULT_MML_VERSION;
        slot.gl_info.sys_status.RH_LEN = (( mode & 0x1 ) != 0) ? 1 : 0;

        /* Parse MML header section. */
        if ( !parse_mml_header(slot, &p_mml) ) {

            return -2;
        }

        slot.gl_info.sys_status.NUM_CH_USED = 0;

        for ( uint8_t i = 0; i < slot.gl_info.sys_status.NUM_CH_IMPL; i++ ) {

            uint8_t ch;

            CHANNEL_INFO *p_ch_info;
            ch = clamp_channel(
                    ( slot.gl_info.sys_status.REVERSE == 1 ) ?
                    NUM_CHANNEL-(i+1) : i
            );

            p_ch_info = slot.ch_info_list[ch];

            p_ch_info->mml.p_mml_head = p_mml;
            p_ch_info->mml.ofs_mml_pos = 0;

            while ((*p_mml != ',') && (*p_mml != '\0')) p_mml++;

            p_ch_info->mml.mml_len = (p_mml - p_ch_info->mml.p_mml_head);

            p_ch_info->ch_status.DECODE_END = 0;
            slot.gl_info.sys_status.NUM_CH_USED++;

            if ( *p_mml == '\0' ) {

                break;
            }

            p_mml++;
        }

        slot.gl_info.sys_status.SET_MML = 1;

        return 0;
    }

    void set_user_callback(
            SLOT &slot,
            void (*callback)(uint8_t ch, int32_t param)
    ) {

        slot.cb_info.user_callback = callback;
    }

    void reset(SLOT &slot) {

        for ( uint8_t i = 0; i < NUM_CHANNEL; i++ ) {

            if ( slot.ch_info_list[i] != nullptr ) {

                reset_ch_info(slot.ch_info_list[i]);
            }
        }

        slot.gl_info.sys_status.SET_MML = 0;
        slot.gl_info.sys_status.NUM_CH_USED = 0;
        slot.gl_info.sys_status.CTRL_STAT = CTRL_STAT_STOP;
        slot.gl_info.sys_status.CTRL_STAT_PRE = CTRL_STAT_STOP;
        slot.gl_info.sys_request.CTRL_REQ = CTRL_REQ_STOP;
        slot.gl_info.sys_request.CTRL_REQ_FLAG = 0;
        slot.gl_info.sys_request.FIN_PRI_LOOP_REQ = FIN_PRI_LOOP_REQ_NORMAL;
        slot.gl_info.sys_request.FIN_PRI_LOOP_REQ_FLAG = 0;

        slot.gl_info.noise_info = (NOISE_INFO){};
        slot.gl_info.noise_info.SWEEP_STAT = NOISE_SWEEP_STAT_STOP;

        reset_psg(slot.psg_reg);
    }

    void set_speed_factor(SLOT &slot, uint16_t speed_factor) {

        uint16_t pre_speed_factor;
        uint32_t q12_alpha;

        pre_speed_factor = slot.gl_info.speed_factor;

        speed_factor = SAT(speed_factor, MIN_SPEED_FACTOR, MAX_SPEED_FACTOR);

        q12_alpha = (static_cast<uint32_t>(pre_speed_factor) << 12) / speed_factor;

        for ( uint8_t i = 0; i < NUM_CHANNEL; i++ ) {

            CHANNEL_INFO *p_ch_info;
            p_ch_info = slot.ch_info_list[i];

            if ( p_ch_info != nullptr ) {

                uint32_t q12_note_on_time;

                q12_note_on_time = (static_cast<uint64_t>(p_ch_info->time.note_on)<<12);
                q12_note_on_time += p_ch_info->time.NOTE_ON_FRAC;

                q12_note_on_time = (static_cast<uint64_t>(q12_note_on_time) * q12_alpha + (1<<11))>>12;

                p_ch_info->time.NOTE_ON_FRAC = q12_note_on_time&0xFFF;
                p_ch_info->time.note_on = (q12_note_on_time>>12)&0xFFFF;

                p_ch_info->time.gate = (static_cast<uint32_t>(p_ch_info->time.gate) * q12_alpha + (1<<11))>>12;
                p_ch_info->time.sw_env = (static_cast<uint32_t>(p_ch_info->time.sw_env) * q12_alpha + (1<<11))>>12;
                p_ch_info->time.lfo_delay = (static_cast<uint32_t>(p_ch_info->time.lfo_delay) * q12_alpha + (1<<11))>>12;
                p_ch_info->time.pitchbend = (static_cast<uint32_t>(p_ch_info->time.pitchbend) * q12_alpha + (1<<11))>>12;
            }
        }

        slot.gl_info.speed_factor = speed_factor;
    }

    void shift_frequency(SLOT &slot, int16_t shift_degrees) {

        slot.gl_info.shift_degrees = SAT(shift_degrees, MIN_FREQ_SHIFT_DEGREES, MAX_FREQ_SHIFT_DEGREES);
   }

    void control_psg(SLOT &slot) {

        uint8_t ch;
        uint8_t decode_end_cnt = 0;

        slot.gl_info.sys_status.CTRL_STAT_PRE = slot.gl_info.sys_status.CTRL_STAT;

        if ( slot.gl_info.sys_status.SET_MML == 0 ) {

            return;
        }

        if ( slot.gl_info.sys_request.CTRL_REQ_FLAG != 0 ) {

            slot.gl_info.sys_request.CTRL_REQ_FLAG = 0;

            if ( slot.gl_info.sys_request.CTRL_REQ == CTRL_REQ_PLAY ) {

                rewind_mml(slot);
                slot.gl_info.sys_status.CTRL_STAT = CTRL_STAT_PLAY;

            } else {

                slot.gl_info.sys_status.CTRL_STAT = CTRL_STAT_STOP;
                for ( uint8_t i = 0; i < slot.gl_info.sys_status.NUM_CH_USED; i++ ) {

                    ch = clamp_channel(
                            ( slot.gl_info.sys_status.REVERSE == 1 ) ?
                            NUM_CHANNEL-(i+1) : i
                    );
                    slot.psg_reg.data[0x7]   |= 0x9<<ch;
                    slot.psg_reg.flags_mixer |= 0x1<<ch;
                }
                slot.psg_reg.flags_addr  = 1<<0x7;
            }
        }

        if ( ( slot.gl_info.sys_status.CTRL_STAT == CTRL_STAT_STOP ) ||
             ( slot.gl_info.sys_status.CTRL_STAT == CTRL_STAT_END  )
        ) {

            return;
        }

        if ( slot.gl_info.sys_request.FIN_PRI_LOOP_REQ_FLAG != 0 ) {

            slot.gl_info.sys_status.FIN_PRI_LOOP_TRY = MAX_FIN_PRI_LOOP_TRY;
            slot.gl_info.sys_request.FIN_PRI_LOOP_REQ_FLAG = 0;
        }
        if ( slot.gl_info.sys_status.FIN_PRI_LOOP_TRY > 0 ) {

            bool fin_prim_loop;
            slot.gl_info.sys_status.FIN_PRI_LOOP_TRY--;

            if ( slot.gl_info.sys_status.FIN_PRI_LOOP_TRY > 0 ) {

                uint8_t prim_loop_counter = slot.ch_info_list[
                        (slot.gl_info.sys_status.REVERSE == 1 ) ? NUM_CHANNEL-1 : 0
                ]->mml.prim_loop_counter;

                fin_prim_loop = true;
                for ( uint8_t i = 1; i < slot.gl_info.sys_status.NUM_CH_USED; i++ ) {

                    ch = ( slot.gl_info.sys_status.REVERSE == 1 ) ? NUM_CHANNEL-(i+1) : i;
                    if ( prim_loop_counter != slot.ch_info_list[ch]->mml.prim_loop_counter ) {

                        fin_prim_loop = false;
                        break;
                    }
                }

            } else {

                fin_prim_loop = true;
            }

            if ( fin_prim_loop ) {

                for ( uint8_t i = 0; i < slot.gl_info.sys_status.NUM_CH_USED; i++ ) {

                    ch = clamp_channel(
                            ( slot.gl_info.sys_status.REVERSE == 1 ) ?
                            NUM_CHANNEL-(i+1) : i
                    );
                    slot.ch_info_list[ch]->ch_status.END_PRI_LOOP = 1;
                }
                slot.gl_info.sys_status.FIN_PRI_LOOP_TRY = 0;
            }
        }

        for ( uint8_t i = 0; i < slot.gl_info.sys_status.NUM_CH_USED; i++ ) {

            CHANNEL_INFO *p_ch_info;

            ch = clamp_channel(
                    ( slot.gl_info.sys_status.REVERSE == 1 ) ?
                    NUM_CHANNEL-(i+1) : i
            );
            p_ch_info = slot.ch_info_list[ch];

            if ( p_ch_info->time.note_on > 0 ) {

                p_ch_info->time.note_on--;
            }
            if ( p_ch_info->time.gate > 0 ) {

                p_ch_info->time.gate--;
            }
            if ( p_ch_info->time.note_on == 0 ) {

                if ( p_ch_info->ch_status.DECODE_END == 0 ) {

                    decode_mml(slot, ch);
                } else {

                    decode_end_cnt++;
                }
            }
            if ( p_ch_info->time.gate == 0 ) {

                if ( ( p_ch_info->tone.GATE_TIME < 7 ) ||
                     ( p_ch_info->ch_status.DECODE_END == 1 )
                ) {

                    /* Mute tone and noise */
                    if ( ((slot.psg_reg.data[0x7]>>ch)&0x9) != 0x9 ) {

                        slot.psg_reg.data[0x7]   |= (0x9<<ch);
                        slot.psg_reg.flags_addr  |= 1<<0x7;
                        slot.psg_reg.flags_mixer |= (1<<ch);
                    }
                }
            }

            /* PITCHBEND BLOCK */
            proc_pitchbend(slot, ch);

            /* SOFTWARE ENVELOPE GENERATOR BLOCK */
            if ( p_ch_info->ch_status.SW_ENV_MODE == 1 ) {

                proc_sw_env_gen(slot, ch);
            }
            /* LFO BLOCK */
            if ( p_ch_info->ch_status.LFO_MODE == 1 ) {

                proc_lfo(slot, ch);
            }
        }

        /* NOISE SWEEP BLOCK */
        proc_noise_sweep(slot);

        if ( decode_end_cnt >= slot.gl_info.sys_status.NUM_CH_USED ) {

            slot.gl_info.sys_status.CTRL_STAT = CTRL_STAT_END;
        }
    }
}
