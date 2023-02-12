/*
 * MIT License, see the LICENSE file for details.
 *
 * Copyright (c) 2023 nyannkov
 */
#include "psg_ctrl.h"

namespace
{
    using namespace PsgCtrl;

    uint16_t U16(uint8_t h, uint8_t l)
    {
        return (((uint16_t)(h)<<8)|(l));
    }

    uint8_t U16_HI(uint16_t x)
    {
        return (((x)>>8)&0xFF);
    }

    uint8_t U16_LO(uint16_t x)
    {
        return (((x)>>0)&0xFF);
    }

    uint16_t ms2tk(uint32_t time_ms)
    {
        uint16_t time_tk;

        time_tk = (time_ms*TICK_HZ+500)/1000;
        
        return time_tk;
    }

    inline char to_upper_case(char c)
    {
        if ( ('a' <= c) && (c <= 'z') )
        {
            c &= (uint8_t)~0x20UL;
        }
        return c;
    }

    int16_t sat(int32_t x, int32_t min, int32_t max)
    {
        return( (x <= min ) ? min
              : (x >= max ) ? max
              :  x
              );
    }

    inline bool is_white_space(const char c)
    {
        switch(c)
        {
        case ' ': /*@fallthrough@*/
        case '\t':/*@fallthrough@*/
        case '\n':/*@fallthrough@*/
        case '\r':
            return true;
        default:
            return false;
        }
    }

    void skip_white_space(const char **pp_text)
    {
        size_t n = MAX_MML_TEXT_LEN;
        while ( is_white_space(**pp_text) && (n != 0) )
        {
            if ( **pp_text == '\0' )
            {
                break;
            }

            (*pp_text)++;
            n--;
        }
    }

    const char * count_dot(const char *p_pos, const char *p_tail, uint8_t *p_out)
    {
        const char *p;
        uint8_t dot_cnt;

        dot_cnt = 0;
        for ( p = p_pos; p < p_tail; p++ )
        {
            if ( *p == '.' )
            {
                dot_cnt = (dot_cnt < MAX_REPEATING_DOT_LENGTH)
                        ? (dot_cnt+1)
                        : MAX_REPEATING_DOT_LENGTH;
            }
            else
            {
                break;
            }
        }

        *p_out = dot_cnt;

        return p;
    }

    uint16_t shift_tp(uint16_t tp, int16_t bias)
    {
        uint16_t i;
        uint16_t q;
        uint16_t r;
        uint32_t q24_f;
        uint64_t lq24_tp;

        lq24_tp = (uint64_t)tp<<24;

        if ( bias >= 0 )
        {
            q = bias/360;
            r = bias%360;
            lq24_tp >>= q;
            q24_f = Q_PITCHBEND_FACTOR_N;
        }
        else
        {
            bias *= -1;
            q = bias/360;
            r = bias%360;
            lq24_tp <<= q;
            q24_f= Q_PITCHBEND_FACTOR;
        }

        for ( i = 0; i < r; i++ )
        {
            lq24_tp *= q24_f;
            lq24_tp >>= 24;
        }
        lq24_tp += ((uint32_t)1)<<23;
        tp = (uint16_t)(lq24_tp>>24);
        
        return ( (tp < MAX_TP) ? tp : MAX_TP );
    }

    uint16_t calc_tp(int16_t n, uint32_t s_clock)
    {
        int64_t lq24_hertz;
        int64_t lq24_clock;
        int32_t q24_f;
        int32_t r;
        int32_t q;
        uint32_t tp;

        n -= 45;
        q = n/12;

        lq24_hertz = (int64_t)440<<24;
        if ( n >= 0 )
        {
            lq24_hertz <<= q;
            q24_f = Q_CALCTP_FACTOR;
        }
        else
        {
            lq24_hertz >>= (q*-1);
            q24_f = Q_CALCTP_FACTOR_N;
            n *= -1;
        }

        r = n%12;
        while ( r-- > 0 )
        {
            lq24_hertz *= q24_f;
            lq24_hertz >>= 24;
        }

        lq24_clock = (int64_t)s_clock<<24;

        tp = (lq24_clock/(1600*lq24_hertz));

        if ( tp > MAX_TP )
        {
            tp = MAX_TP;
        }
        
        return (uint16_t)tp;
    }

    const char * read_number_ex(const char *p_pos, const char *p_tail, int32_t min, int32_t max, int32_t default_value, int32_t *p_out, /*@null@*/bool *p_is_omitted)
    {
        int32_t n;
        int16_t sign;
        bool is_omitted;

        if ( p_pos >= p_tail )
        {
            *p_out = default_value;
            return p_tail;
        }

        is_omitted = true;
        switch ( *p_pos )
        {
        case '+':
            sign = 1;
            p_pos++;
            is_omitted = false;
            break;

        case '-':
            sign = (-1);
            p_pos++;
            is_omitted = false;
            break;

        default:
            sign = 1;
            /*Leave p_pos as is.*/
            break;
        }

        n = 0;
        for (/*DO NOTHING*/; p_pos < p_tail ; p_pos++ )
        {
            if ( ('0' <= *p_pos) && (*p_pos <= '9') )
            {
                is_omitted = false;
                n = sat( 
                    10*n + (int32_t)(*p_pos-'0')*sign
                  , min
                  , max
                  );
            }
            else
            {
                break;
            }
        }

        if ( is_omitted )
        {
            n = default_value;
        }

        *p_out = sat(n, min, max);

        if ( p_is_omitted != nullptr )
        {
            *p_is_omitted = is_omitted;
        }

        return p_pos;
    }

    const char * read_number(const char *p_pos, const char *p_tail, int32_t min, int32_t max, int32_t default_value, int32_t *p_out)
    {
        return read_number_ex(p_pos, p_tail, min, max, default_value, p_out, nullptr);
    }

    int32_t get_param(const char **pp_pos, const char *p_tail, int32_t min, int32_t max, int32_t default_value)
    {
        int32_t r = 0;

        *pp_pos = read_number(*pp_pos+1, p_tail, min, max, default_value, &r);

        return r;
    }

    int16_t get_tp_table_column_number(const char note_name)
    {
        int16_t col_num = -1;
        switch ( to_upper_case(note_name) )
        {
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

    const char * shift_half_note_number(const char *p_pos, const char *p_tail, int16_t note_num, int16_t *p_out)
    {
        const char *p;
        int16_t n;

        n = note_num;

        for ( p = p_pos; p < p_tail; p++ )
        {
            if ( ( *p == '+' ) || ( *p == '#' ) )
            {
                n = ( n < MAX_NOTE_NUMBER )
                  ? (n+1)
                  : MAX_NOTE_NUMBER;
            }
            else if ( *p == '-' )
            {
                n = ( n > MIN_NOTE_NUMBER )
                  ? (n-1)
                  : MIN_NOTE_NUMBER;
            }
            else
            {
                break;
            }
        }

        n = sat(n, MIN_NOTE_NUMBER, MAX_NOTE_NUMBER);
        
        *p_out = n;

        return p;
    }

    uint16_t get_sus_volume(SLOT &slot, int16_t ch)
    {
        return ( (uint16_t)slot.ch_info_list[ch]->sw_env.sustain * slot.ch_info_list[ch]->tone.VOLUME+50 )/100;
    }

    void trans_sw_env_state(SLOT &slot, int16_t ch)
    {
        if ( slot.ch_info_list[ch]->time.sw_env != 0 )
        {
            /* NO CHANGE STATE */
            return;
        }

        switch ( slot.ch_info_list[ch]->ch_status.SW_ENV_STAT )
        {
        case SW_ENV_STAT_INIT_NOTE_ON:
            if ( slot.ch_info_list[ch]->sw_env.attack_tk != 0 )
            {
                slot.ch_info_list[ch]->sw_env.VOL_INT = 0;
                slot.ch_info_list[ch]->sw_env.VOL_FRAC = 0;
                slot.ch_info_list[ch]->time.sw_env = slot.ch_info_list[ch]->sw_env.attack_tk;
                slot.ch_info_list[ch]->ch_status.SW_ENV_STAT = SW_ENV_STAT_ATTACK;
                break;
            }
            /*@fallthrough@*/

        case SW_ENV_STAT_ATTACK:
            if ( slot.ch_info_list[ch]->sw_env.hold_tk != 0 )
            {
                slot.ch_info_list[ch]->sw_env.VOL_INT  = slot.ch_info_list[ch]->tone.VOLUME;
                slot.ch_info_list[ch]->sw_env.VOL_FRAC = 0;
                slot.ch_info_list[ch]->time.sw_env = slot.ch_info_list[ch]->sw_env.hold_tk;
                slot.ch_info_list[ch]->ch_status.SW_ENV_STAT = SW_ENV_STAT_HOLD;
                break;
            }
            /*@fallthrough@*/

        case SW_ENV_STAT_HOLD:
            if ( slot.ch_info_list[ch]->sw_env.decay_tk != 0 )
            {
                slot.ch_info_list[ch]->sw_env.VOL_INT  = slot.ch_info_list[ch]->tone.VOLUME;
                slot.ch_info_list[ch]->sw_env.VOL_FRAC = 0;
                slot.ch_info_list[ch]->time.sw_env = slot.ch_info_list[ch]->sw_env.decay_tk;
                slot.ch_info_list[ch]->ch_status.SW_ENV_STAT= SW_ENV_STAT_DECAY;
                break;
            }
            /*@fallthrough@*/

        case SW_ENV_STAT_DECAY:

            slot.ch_info_list[ch]->sw_env.VOL_INT  = get_sus_volume(slot, ch);
            slot.ch_info_list[ch]->sw_env.VOL_FRAC = 0;
            slot.ch_info_list[ch]->time.sw_env = slot.ch_info_list[ch]->sw_env.fade_tk;
            slot.ch_info_list[ch]->ch_status.SW_ENV_STAT = SW_ENV_STAT_FADE;
            break;

        default:
            /* NO CHANGE */
            break;
        }
    }

    void init_pitchbend(SLOT &slot, int16_t ch)
    {
        uint16_t tp_end;
        uint16_t tp_base;
        uint32_t q6_tp_d;

        slot.ch_info_list[ch]->time.pitchbend = slot.ch_info_list[ch]->time.note_on;
        if ( slot.ch_info_list[ch]->time.pitchbend == 0 )
        {
            slot.ch_info_list[ch]->ch_status.PBEND_STAT = PBEND_STAT_STOP;
            return;
        }

        tp_base = U16(slot.psg_reg.data[2*ch+1], slot.psg_reg.data[2*ch])&0xFFF;

        tp_end = slot.ch_info_list[ch]->pitchbend.TP_END_H;
        tp_end = tp_end<<4 | slot.ch_info_list[ch]->pitchbend.TP_END_L;

        if ( tp_base < tp_end )
        {
            slot.ch_info_list[ch]->ch_status.PBEND_STAT = PBEND_STAT_TP_UP;
            q6_tp_d = tp_end - tp_base;
            q6_tp_d <<= 6;
            q6_tp_d /= slot.ch_info_list[ch]->time.pitchbend;
        }
        else if ( tp_base > tp_end )
        {
            slot.ch_info_list[ch]->ch_status.PBEND_STAT = PBEND_STAT_TP_DOWN;
            q6_tp_d = tp_base - tp_end;
            q6_tp_d <<= 6;
            q6_tp_d /= slot.ch_info_list[ch]->time.pitchbend;
        }
        else
        {
            slot.ch_info_list[ch]->ch_status.PBEND_STAT = PBEND_STAT_STOP;
            q6_tp_d = 0;
        }

        slot.ch_info_list[ch]->pitchbend.TP_D_INT = (q6_tp_d >> 6) & 0xFFF;
        slot.ch_info_list[ch]->pitchbend.TP_D_FRAC = q6_tp_d & 0x3F;
    }

    void proc_pitchbend(SLOT &slot, int16_t ch)
    {
        uint32_t q6_tp;
        uint32_t q6_tp_d;
        uint32_t q6_tp_end;
        uint16_t tp_int;

        if ( slot.ch_info_list[ch]->time.pitchbend > 0 )
        {
            slot.ch_info_list[ch]->time.pitchbend--;
        }
        else
        {
            slot.ch_info_list[ch]->ch_status.PBEND_STAT = PBEND_STAT_STOP;
        }
        
        if ( slot.ch_info_list[ch]->ch_status.PBEND_STAT == PBEND_STAT_STOP )
        {
            return;
        }

        q6_tp      = slot.ch_info_list[ch]->pitchbend.TP_INT;
        q6_tp      = (q6_tp<<6)|slot.ch_info_list[ch]->pitchbend.TP_FRAC;
        q6_tp_d    = slot.ch_info_list[ch]->pitchbend.TP_D_INT;
        q6_tp_d    = (q6_tp_d<<6)|slot.ch_info_list[ch]->pitchbend.TP_D_FRAC;
        q6_tp_end  = slot.ch_info_list[ch]->pitchbend.TP_END_H;
        q6_tp_end  = (q6_tp_end<<4)|slot.ch_info_list[ch]->pitchbend.TP_END_L;
        q6_tp_end  = q6_tp_end<<6;

        if ( slot.ch_info_list[ch]->ch_status.PBEND_STAT == PBEND_STAT_TP_UP )
        {
            if ( (q6_tp_end-q6_tp) <= q6_tp_d )
            {
                q6_tp = q6_tp_end;
                slot.ch_info_list[ch]->ch_status.PBEND_STAT = PBEND_STAT_STOP;
            }
            else
            {
                q6_tp += q6_tp_d;
            }
        }
        else
        {
            if ( (q6_tp-q6_tp_end) <= q6_tp_d )
            {
                q6_tp = q6_tp_end;
                slot.ch_info_list[ch]->ch_status.PBEND_STAT = PBEND_STAT_STOP;
            }
            else
            {
                q6_tp -= q6_tp_d;
            }
        }

        tp_int = q6_tp>>6;
        slot.psg_reg.data[0x0+2*ch] = U16_LO(tp_int);
        slot.psg_reg.data[0x1+2*ch] = U16_HI(tp_int);
        slot.psg_reg.flags_addr    |= 0x3<<(2*ch);

        slot.ch_info_list[ch]->pitchbend.TP_INT  = tp_int;
        slot.ch_info_list[ch]->pitchbend.TP_FRAC = q6_tp&0x3F;
    }

    const char * get_legato_end_note_num(const char *p_pos, const char *p_tail, int32_t default_octave, int32_t *p_out)
    {
        const char *p;
        int32_t octave;
        uint8_t col_num;
        int16_t note_num;

        /* Change octave */
        octave = sat(default_octave, MIN_OCTAVE, MAX_OCTAVE);
        for ( p = p_pos; p < p_tail; p++ )
        {
            if ( *p == 'O' )
            {
                const char *p_tmp;
                p_tmp = read_number(
                        p+1
                      , p_tail
                      , MIN_OCTAVE
                      , MAX_OCTAVE
                      , DEFAULT_OCTAVE
                      , &octave
                      );
                p = (p_tmp - 1);
            }
            else if ( *p == '<' )
            {
                octave = (octave > MIN_OCTAVE) ? (octave-1) : MIN_OCTAVE;
            }
            else if ( *p == '>' )
            {
                octave = (octave < MAX_OCTAVE) ? (octave+1) : MAX_OCTAVE;
            }
            else if ( is_white_space(*p) )
            {
                continue;
            }
            else
            {
                break;
            }
        }

        col_num = get_tp_table_column_number(*p);
        note_num = (int32_t)((uint32_t)col_num&0xFFu) + (octave-1)*12;
        /* Shift Note-Number */
        p = shift_half_note_number( p+1
                                  , p_tail
                                  , note_num
                                  , &note_num);

        *p_out = note_num;

        return p_pos;
    }
    
    uint32_t get_note_on_time(uint8_t note_len, uint8_t tempo, uint8_t dot_cnt)
    {
        uint32_t q12_time, q12_time_delta;

        if ( note_len == 0 )
        {
            return 0;
        }

        q12_time_delta = (((uint32_t)TICK_HZ*4*60)<<12) / ((uint16_t)tempo*note_len);
        q12_time = q12_time_delta;

        while ( dot_cnt > 0 )
        {
            q12_time_delta >>= 1;
            q12_time += q12_time_delta;
            dot_cnt--;
        }

        return q12_time;
    }

    void decode_dollar(CHANNEL_INFO *p_info, const char **pp_pos, const char *p_tail)
    {
        int32_t param;

        (*pp_pos)++;
        switch ( to_upper_case(**pp_pos) )
        {
            case 'A':
                param = get_param(
                    pp_pos
                  , p_tail
                  , MIN_SOFT_ENVELOPE_ATTACK
                  , MAX_SOFT_ENVELOPE_ATTACK
                  , DEFAULT_SOFT_ENVELOPE_ATTACK
                );
                p_info->sw_env.attack_tk = ms2tk(param);
                break;

            case 'D':
                param = get_param(
                    pp_pos
                  , p_tail
                  , MIN_SOFT_ENVELOPE_DECAY
                  , MAX_SOFT_ENVELOPE_DECAY
                  , DEFAULT_SOFT_ENVELOPE_DECAY
                );
                p_info->sw_env.decay_tk = ms2tk(param);
                break;

            case 'E':
                param = get_param(
                    pp_pos
                  , p_tail
                  , MIN_SW_ENV_MODE
                  , MAX_SW_ENV_MODE
                  , DEFAULT_SW_ENV_MODE
                );
                p_info->ch_status.SW_ENV_MODE = param;
                break;

            case 'F':
                param = get_param(
                    pp_pos
                  , p_tail
                  , MIN_SOFT_ENVELOPE_FADE
                  , MAX_SOFT_ENVELOPE_FADE
                  , DEFAULT_SOFT_ENVELOPE_FADE
                );
                p_info->sw_env.fade_tk = ms2tk(param);
                break;

            case 'H':
                param = get_param(
                    pp_pos
                  , p_tail
                  , MIN_SOFT_ENVELOPE_HOLD
                  , MAX_SOFT_ENVELOPE_HOLD
                  , DEFAULT_SOFT_ENVELOPE_HOLD
                );
                p_info->sw_env.hold_tk = ms2tk(param);
                break;

            case 'S':
                param = get_param(
                    pp_pos
                  , p_tail
                  , MIN_SOFT_ENVELOPE_SUSTAIN
                  , MAX_SOFT_ENVELOPE_SUSTAIN
                  , DEFAULT_SOFT_ENVELOPE_SUSTAIN
                );
                p_info->sw_env.sustain = param;
                break;

            case 'M':
                param = get_param(
                    pp_pos
                  , p_tail
                  , MIN_LFO_MODE
                  , MAX_LFO_MODE
                  , DEFAULT_LFO_MODE
                );
                p_info->ch_status.LFO_MODE = param;
                break;

            case 'B':
                param = get_param(
                    pp_pos
                  , p_tail
                  , MIN_BIAS_LEVEL
                  , MAX_BIAS_LEVEL
                  , DEFAULT_BIAS_LEVEL
                );
                p_info->tone.BIAS = param + BIAS_LEVEL_OFS;
                break;

            case 'L':
                param = get_param(
                    pp_pos
                  , p_tail
                  , MIN_LFO_SPEED
                  , MAX_LFO_SPEED
                  , DEFAULT_LFO_SPEED
                );
                p_info->lfo.speed = param;
                break;

            case 'J':
                param = get_param(
                    pp_pos
                  , p_tail
                  , MIN_LFO_DEPTH
                  , MAX_LFO_DEPTH
                  , DEFAULT_LFO_DEPTH
                );
                p_info->lfo.depth = param;
                break;

            case 'T':
                param = get_param(
                    pp_pos
                  , p_tail
                  , MIN_LFO_DELAY
                  , MAX_LFO_DELAY
                  , DEFAULT_LFO_DELAY
                );
                p_info->lfo.delay_tk = param;
                break;

            case 'P':
                param = get_param(
                    pp_pos
                  , p_tail
                  , MIN_PITCHBEND_LEVEL
                  , MAX_PITCHBEND_LEVEL
                  , DEFAULT_PITCHBEND_LEVEL
                );
                p_info->pitchbend.level = param;
                break;

            default:
                (*pp_pos)++;
                break;
        }
    }

    void generate_tone(SLOT &slot, int16_t ch, const char **pp_pos, const char *p_tail)
    {
        const char *p_pos;
        int16_t note_num;
        int32_t legato_end_note_num;
        int32_t note_len;
        uint16_t tp;
        uint16_t tp_end;
        char head;
        uint8_t col_num;
        uint8_t dot_cnt;
        bool is_note_len_omitted;
        bool is_use_global_note_len;
        bool is_start_legato_effect;
        enum
        {
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

        if ( ('A' <= head) && (head <= 'G') )
        {
            /* Set Note-Type */
            note_type = E_NOTE_TYPE_TONE;

            /* Get Note-Number */
            col_num = get_tp_table_column_number(head);
            note_num = col_num + ((uint16_t)slot.ch_info_list[ch]->tone.OCTAVE)*12;

            /* Shift Note-Number */
            p_pos = shift_half_note_number( p_pos+1
                                          , p_tail
                                          , note_num
                                          , &note_num);

            /* Get Note-Length */
            is_note_len_omitted = false;
            p_pos = read_number_ex( p_pos
                               , p_tail
                               , 0 /* special case */
                               , MAX_NOTE_LENGTH
                               , slot.ch_info_list[ch]->tone.note_len
                               , &note_len
                               , &is_note_len_omitted);

            is_use_global_note_len = is_note_len_omitted;

            /* Count Dot-Repetition */
            dot_cnt = 0;
            p_pos = count_dot(p_pos, p_tail, &dot_cnt);
            if ( is_use_global_note_len )
            {
                dot_cnt += slot.ch_info_list[ch]->tone.LEN_DOTS;
            }

            /* Legato */
            if ( *p_pos == '&' )
            {
                is_start_legato_effect = true;
                p_pos = get_legato_end_note_num( p_pos+1
                                               , p_tail
                                               , (int32_t)slot.ch_info_list[ch]->tone.OCTAVE+1
                                               , &legato_end_note_num );
            }
            else
            {
                is_start_legato_effect = false;
            }
        }
        else if ( head == 'N' )
        {
            /* Set Note-Type */
            note_type = E_NOTE_TYPE_TONE;

            /* Get Note-Number */
            note_num = get_param(
                           &p_pos
                         , p_tail
                         , MIN_NOTE_NUMBER
                         , MAX_NOTE_NUMBER
                         , DEFAULT_NOTE_NUMBER
                       );

            /* Count Dot-Repetition */
            dot_cnt = 0;
            p_pos = count_dot(p_pos, p_tail, &dot_cnt);

            note_len = slot.ch_info_list[ch]->tone.note_len;
            dot_cnt += slot.ch_info_list[ch]->tone.LEN_DOTS;
        }
        else if ( (head == 'H') || (head == 'R') )
        {
            /* Set Note-Type */
            note_type = ( head == 'R' ) 
                      ? E_NOTE_TYPE_REST 
                      : E_NOTE_TYPE_NOISE;

            /* Get Note-Length */
            if ( slot.gl_info.sys_status.RH_LEN != 0 )
            {
                is_note_len_omitted = false;
                p_pos = read_number_ex( p_pos+1
                                   , p_tail
                                   , MIN_NOTE_LENGTH
                                   , MAX_NOTE_LENGTH
                                   , slot.ch_info_list[ch]->tone.note_len 
                                   , &note_len
                                   , &is_note_len_omitted
                                   );

                is_use_global_note_len = is_note_len_omitted;
            }
            else
            {
                is_use_global_note_len = false;
                p_pos = read_number( p_pos+1
                                   , p_tail
                                   , MIN_NOTE_LENGTH
                                   , MAX_NOTE_LENGTH
                                   , DEFAULT_NOTE_LENGTH
                                   , &note_len
                                   );
            }

            /* Count Dot-Repetition */
            dot_cnt = 0;
            p_pos = count_dot(p_pos, p_tail, &dot_cnt);

            if ( is_use_global_note_len )
            {
                dot_cnt += slot.ch_info_list[ch]->tone.LEN_DOTS;
            }
        }
        else
        {
            note_type = E_NOTE_TYPE_OTHER;
        }

        /* Update current MML pos */
        *pp_pos = p_pos;

        if ( note_type == E_NOTE_TYPE_TONE ) 
        {
            int16_t bias;
            bias = (int16_t)slot.ch_info_list[ch]->tone.BIAS - BIAS_LEVEL_OFS;
            /* Apply detune-level to tp. */
            tp = shift_tp(calc_tp(note_num, slot.gl_info.s_clock), bias);
            if ( is_start_legato_effect )
            {
                tp_end = shift_tp(calc_tp(legato_end_note_num, slot.gl_info.s_clock), bias);
            }
            else
            {
                tp_end = shift_tp(tp, slot.ch_info_list[ch]->pitchbend.level);
            }

            slot.psg_reg.data[2*ch]     = U16_LO(tp);
            slot.psg_reg.data[2*ch+1]   = U16_HI(tp);
            slot.psg_reg.flags_addr    |= 0x3<<(2*ch);
        }
        else
        {
            tp = 0;
            tp_end = 0;
        }

        slot.ch_info_list[ch]->pitchbend.TP_INT = tp;
        slot.ch_info_list[ch]->pitchbend.TP_FRAC =0;
        slot.ch_info_list[ch]->pitchbend.TP_END_L = tp_end&0xF;
        slot.ch_info_list[ch]->pitchbend.TP_END_H = (tp_end>>4)&0xFF;

        if ( ( note_type == E_NOTE_TYPE_TONE ) 
          || ( note_type == E_NOTE_TYPE_NOISE ) )
        {
            slot.psg_reg.data[0x8+ch] = slot.ch_info_list[ch]->tone.VOLUME;
            slot.psg_reg.flags_addr  |= 1<<(0x8+ch);
            if ( slot.ch_info_list[ch]->tone.HW_ENV != 0 )
            {
                slot.psg_reg.data[0x8+ch] |= 1<<4;
                if ( slot.ch_info_list[ch]->ch_status.LEGATO == 0 )
                {
                    /* To reload 5bit counter of the envelope generator. */
                    slot.psg_reg.flags_addr |= 1<<0xD;
                }
            }
            else
            {
                slot.psg_reg.data[0x8+ch] &= ~(1<<4);
            }
        }

        /* Note-ON */
        if ( note_len != 0 )
        {
            uint32_t q12_note_on_time;
            uint32_t q12_gate_time;
            uint8_t req_mixer;

            q12_note_on_time  = get_note_on_time(
                                        note_len
                                      , slot.ch_info_list[ch]->tone.tempo
                                      , dot_cnt
                                      );
            q12_note_on_time += slot.ch_info_list[ch]->time.NOTE_ON_FRAC;
            slot.ch_info_list[ch]->time.NOTE_ON_FRAC = q12_note_on_time&0xFFF;
            slot.ch_info_list[ch]->time.note_on = (q12_note_on_time>>12)&0xFFFF;

            q12_gate_time = (q12_note_on_time * ((uint32_t)slot.ch_info_list[ch]->tone.GATE_TIME+1))/MAX_GATE_TIME;
            slot.ch_info_list[ch]->time.gate = (q12_gate_time>>12)&0xFFFF;

            req_mixer = ( note_type == E_NOTE_TYPE_TONE  ) ? (0x08)
                      : ( note_type == E_NOTE_TYPE_NOISE ) ? (0x01)
                      : (0x09);

            slot.psg_reg.data[0x7] &= ~(0x9<<ch);
            slot.psg_reg.data[0x7] |= req_mixer<<ch;
            slot.psg_reg.flags_addr  |= 1<<0x7;
            slot.psg_reg.flags_mixer |= 1<<ch;
        }

        if ( slot.ch_info_list[ch]->ch_status.SW_ENV_MODE != SW_ENV_MODE_OFF )
        {
            if ( slot.ch_info_list[ch]->ch_status.LEGATO == 0 )
            {
                slot.ch_info_list[ch]->time.sw_env = 0;
                slot.ch_info_list[ch]->ch_status.SW_ENV_STAT = SW_ENV_STAT_INIT_NOTE_ON;
                trans_sw_env_state(slot, ch);
            }
        }

        if ( slot.ch_info_list[ch]->ch_status.LFO_MODE != LFO_MODE_OFF )
        {
            slot.ch_info_list[ch]->ch_status.LFO_STAT = LFO_STAT_RUN;
            if ( slot.ch_info_list[ch]->ch_status.LEGATO == 0 )
            {
                slot.ch_info_list[ch]->time.lfo_delay = slot.ch_info_list[ch]->lfo.delay_tk;
                slot.ch_info_list[ch]->lfo.theta = 0;
            }
        }
        else
        {
            slot.ch_info_list[ch]->ch_status.LFO_STAT = LFO_STAT_STOP;
        }

        init_pitchbend(slot, ch);

        slot.ch_info_list[ch]->ch_status.LEGATO = is_start_legato_effect ? 1 : 0;
    }

    int16_t decode_mml(SLOT &slot, int16_t ch)
    {
        const char *p_pos;
        const char *p_head;
        const char *p_tail;
        int32_t param;
        bool loop_flag = false;
        bool decode_cont = true;

        p_head =  slot.ch_info_list[ch]->mml.p_mml_head;
        p_pos  = &slot.ch_info_list[ch]->mml.p_mml_head[slot.ch_info_list[ch]->mml.ofs_mml_pos];
        p_tail = &slot.ch_info_list[ch]->mml.p_mml_head[slot.ch_info_list[ch]->mml.mml_len];

        if ( p_pos >= p_tail )
        {
            if ( slot.ch_info_list[ch]->ch_status.DECODE_END != 1 )
            {
                slot.ch_info_list[ch]->ch_status.DECODE_END = 1;
            }

            return 1;
        }

        while ( decode_cont )
        {
            switch ( to_upper_case(p_pos[0]) )
            {
            case 'A':/*@fallthrough@*/
            case 'B':/*@fallthrough@*/
            case 'C':/*@fallthrough@*/
            case 'D':/*@fallthrough@*/
            case 'E':/*@fallthrough@*/
            case 'F':/*@fallthrough@*/
            case 'G':/*@fallthrough@*/
            case 'H':/*@fallthrough@*/
            case 'N':/*@fallthrough@*/
            case 'R':
                generate_tone(slot, ch, &p_pos, p_tail);
                decode_cont = false;
                break;

            case '$':
                decode_dollar(slot.ch_info_list[ch], &p_pos, p_tail);
                break;

            case 'T':
                param = get_param(
                    &p_pos
                  , p_tail
                  , MIN_TEMPO
                  , MAX_TEMPO
                  , DEFAULT_TEMPO
                );
                slot.ch_info_list[ch]->tone.tempo = param;
                break;

            case 'V':
                param = get_param(
                    &p_pos
                  , p_tail
                  , MIN_VOLUME_LEVEL
                  , MAX_VOLUME_LEVEL
                  , DEFAULT_VOLUME_LEVEL
                );
                slot.ch_info_list[ch]->tone.HW_ENV = 0;
                slot.ch_info_list[ch]->tone.VOLUME = param;
                break;

            case 'S':
                param = get_param(
                    &p_pos
                  , p_tail
                  , MIN_ENVELOP_SHAPE
                  , MAX_ENVELOP_SHAPE
                  , DEFAULT_ENVELOP_SHAPE
                );
                slot.psg_reg.data[0xD]    = param;
                slot.psg_reg.flags_addr  |= 1<<0xD;
                slot.ch_info_list[ch]->tone.HW_ENV = 1;
                break;

            case 'M':
                param = get_param(
                    &p_pos
                  , p_tail
                  , MIN_ENVELOP_EP
                  , MAX_ENVELOP_EP
                  , DEFAULT_ENVELOP_EP
                );
                slot.psg_reg.data[0xB]    = U16_LO(param);
                slot.psg_reg.data[0xC]    = U16_HI(param);
                slot.psg_reg.flags_addr  |= (0x3<<0xB);
                break;

            case 'L':
                param = get_param(
                    &p_pos
                  , p_tail
                  , MIN_NOTE_LENGTH
                  , MAX_NOTE_LENGTH
                  , DEFAULT_NOTE_LENGTH
                );
                slot.ch_info_list[ch]->tone.note_len = param;

                /* Clear global dot counter */
                slot.ch_info_list[ch]->tone.LEN_DOTS = 0;
                if ( *p_pos == '.' )
                {
                    uint8_t dot_cnt = 0;
                    p_pos = count_dot(p_pos, p_tail, &dot_cnt);
                    slot.ch_info_list[ch]->tone.LEN_DOTS = dot_cnt;
                }
                break;

            case 'O':
                param = get_param(
                    &p_pos
                  , p_tail
                  , MIN_OCTAVE
                  , MAX_OCTAVE
                  , DEFAULT_OCTAVE
                );
                slot.ch_info_list[ch]->tone.OCTAVE = param-1;
                break;

            case 'Q':
                param = get_param(
                    &p_pos
                  , p_tail
                  , MIN_GATE_TIME
                  , MAX_GATE_TIME 
                  , DEFAULT_GATE_TIME
                );
                slot.ch_info_list[ch]->tone.GATE_TIME = param-1;
                break;

            case 'I':
                param = get_param(
                    &p_pos
                  , p_tail 
                  , MIN_NOISE_NP
                  , MAX_NOISE_NP
                  , DEFAULT_NOISE_NP
                );
                slot.psg_reg.data[0x6]   = param;
                slot.psg_reg.flags_addr |= 1<<0x6;
                break;

            case '<':
                if ( slot.ch_info_list[ch]->tone.OCTAVE > (MIN_OCTAVE-1) )
                {
                    slot.ch_info_list[ch]->tone.OCTAVE--;
                }
                p_pos++;
                break;

            case '>':
                if ( slot.ch_info_list[ch]->tone.OCTAVE < (MAX_OCTAVE-1) )
                {
                    slot.ch_info_list[ch]->tone.OCTAVE++;
                }
                p_pos++;
                break;

            case '[':
                if ( slot.ch_info_list[ch]->ch_status.LOOP_DEPTH < MAX_LOOP_NESTING_DEPTH )
                {
                    uint16_t loop_index = 0;
                    loop_flag = true;
                    loop_index = slot.ch_info_list[ch]->ch_status.LOOP_DEPTH;

                    param = get_param(
                              &p_pos
                            , p_tail
                            , MIN_LOOP_TIMES
                            , MAX_LOOP_TIMES
                            , DEFAULT_LOOP_TIMES
                            );
                    slot.ch_info_list[ch]->mml.loop_times[loop_index] = param;
                    slot.ch_info_list[ch]->mml.ofs_mml_loop_head[loop_index] = (p_pos-p_head);
                    slot.ch_info_list[ch]->ch_status.LOOP_DEPTH = loop_index + 1;
                }
                else
                {
                    p_pos++;
                }
                break;

            case ']':
                if ( slot.ch_info_list[ch]->ch_status.LOOP_DEPTH > 0 )
                {
                    uint16_t loop_index = 0;
                    loop_index = slot.ch_info_list[ch]->ch_status.LOOP_DEPTH - 1;

                    if ( loop_flag )
                    {
                        /* A loop with no message. */
                        /* Force exit from the loop. */
                        slot.ch_info_list[ch]->mml.loop_times[loop_index] = 0;
                        slot.ch_info_list[ch]->ch_status.LOOP_DEPTH = loop_index;
                        p_pos++;
                    }
                    else if ( slot.ch_info_list[ch]->mml.loop_times[loop_index] == 1 )
                    {
                        /* Loop end */
                        slot.ch_info_list[ch]->mml.loop_times[loop_index] = 0;
                        slot.ch_info_list[ch]->ch_status.LOOP_DEPTH = loop_index;
                        p_pos++;
                    }
                    else
                    {
                        if ( slot.ch_info_list[ch]->mml.loop_times[loop_index] > 1 )
                        {
                            slot.ch_info_list[ch]->mml.loop_times[loop_index]--;
                        }
                        else
                        {
                            /* Infinite loop */
                        }
                        p_pos = p_head + slot.ch_info_list[ch]->mml.ofs_mml_loop_head[loop_index];
                    }
                }
                else
                {
                    p_pos++;
                }
                break;

            default:
                p_pos++;
                break;
            }

            if ( p_pos >= p_tail )
            {
                slot.ch_info_list[ch]->mml.ofs_mml_pos = (uint16_t)(p_pos - p_head);
                if ( slot.ch_info_list[ch]->ch_status.DECODE_END != 1 )
                {
                    slot.ch_info_list[ch]->ch_status.DECODE_END = 1;
                }

                return 1;
            }

            slot.ch_info_list[ch]->mml.ofs_mml_pos = (uint16_t)(p_pos - p_head);
        }

        return 0;
    }

    void update_sw_env_volume(SLOT &slot, int16_t ch)
    {
        uint16_t vol;
        uint16_t top;
        uint16_t sus;
        uint16_t rate;

        vol = slot.ch_info_list[ch]->sw_env.VOL_INT;
        vol = (vol << 12)|slot.ch_info_list[ch]->sw_env.VOL_FRAC;
        top = slot.ch_info_list[ch]->tone.VOLUME;
        top = top<<12;
        sus = get_sus_volume(slot, ch);
        sus = sus<<12;

        switch ( slot.ch_info_list[ch]->ch_status.SW_ENV_STAT )
        {
        case SW_ENV_STAT_ATTACK:
            rate = top/slot.ch_info_list[ch]->sw_env.attack_tk;
            if ( rate != 0 )
            {
                if ( (top - vol) <= rate )
                {
                    vol = top;
                }
                else
                {
                    vol += rate;
                }
            }
            else
            {
                vol = top;
            }
            break;

        case SW_ENV_STAT_HOLD:
            vol = top;
            break;

        case SW_ENV_STAT_DECAY:
            rate = (top-sus)/slot.ch_info_list[ch]->sw_env.decay_tk;
            if ( rate != 0 )
            {
                if ( (vol-sus) <= rate )
                {
                    vol = sus;
                }
                else
                {
                    vol -= rate;
                }
            }
            else
            {
                vol = sus;
            }
            break;

        case SW_ENV_STAT_FADE:
            if ( slot.ch_info_list[ch]->sw_env.fade_tk != 0 )
            {
                rate = sus/slot.ch_info_list[ch]->sw_env.fade_tk;
            }
            else
            {
                rate = 0;
            }

            if ( rate != 0 )
            {
                if ( vol <= rate )
                {
                    vol = 0;
                }
                else
                {
                    vol -= rate;
                }
            }
            else
            {
                vol = sus;
            }
            break;

        default:
            break;
        }

        slot.ch_info_list[ch]->sw_env.VOL_INT  = (vol>>12)&0xF;
        slot.ch_info_list[ch]->sw_env.VOL_FRAC = vol&0xFFF;
    }

    void proc_sw_env_gen(SLOT &slot, int16_t ch)
    {
        uint8_t vol;
        if ( slot.ch_info_list[ch]->time.sw_env > 0 )
        {
            slot.ch_info_list[ch]->time.sw_env--;
        }
        vol = slot.psg_reg.data[0x08+ch]&0xF;
        if ( vol != slot.ch_info_list[ch]->sw_env.VOL_INT )
        {
            slot.psg_reg.data[0x8+ch] = slot.ch_info_list[ch]->sw_env.VOL_INT;
            if ( ( (slot.psg_reg.data[0x7]>>ch) & 0x9 ) != 0x9 )
            {/* Not muted. */
                slot.psg_reg.flags_addr |= 1<<(0x8+ch);
            }
        }

        trans_sw_env_state(slot, ch);

        update_sw_env_volume(slot, ch);
    }

    void proc_lfo(SLOT &slot, int16_t ch)
    {
        uint8_t tp_hi;
        uint8_t tp_lo;
        uint16_t delta_int;
        uint32_t tp_next;
        uint32_t q6_omega;
        uint32_t q6_delta;

        if ( slot.ch_info_list[ch]->ch_status.LFO_STAT != LFO_STAT_RUN )
        {
            return;
        }

        if ( slot.ch_info_list[ch]->time.lfo_delay > 0 )
        {
            slot.ch_info_list[ch]->time.lfo_delay--;
            return;
        }

        tp_hi = slot.psg_reg.data[2*ch+1];
        tp_lo = slot.psg_reg.data[2*ch+0];
        tp_next = U16(tp_hi, tp_lo);

        q6_omega = 1<<6;
        q6_omega *= (uint32_t)slot.ch_info_list[ch]->lfo.depth*4*slot.ch_info_list[ch]->lfo.speed;
        q6_omega /= (TICK_HZ*MAX_LFO_PERIOD);

        q6_delta = slot.ch_info_list[ch]->lfo.DELTA_FRAC;

        q6_delta += q6_omega;
        delta_int = (q6_delta >> 6);

        if ( delta_int > 0 )
        {
            uint16_t theta = slot.ch_info_list[ch]->lfo.theta;
            uint16_t depth = slot.ch_info_list[ch]->lfo.depth;
            uint16_t i;
            uint64_t lq24_tp;

            for ( i = 0; i < delta_int; i++ )
            {
                lq24_tp = tp_next<<6;
                lq24_tp += slot.ch_info_list[ch]->lfo.TP_FRAC;
                lq24_tp = lq24_tp << 18;
                if ( (depth <= theta) && ( theta < (depth*3)))
                {
                    lq24_tp = (lq24_tp*Q_PITCHBEND_FACTOR_N)>>24;
                }
                else
                {
                    lq24_tp = (lq24_tp*Q_PITCHBEND_FACTOR)>>24;
                }

                slot.ch_info_list[ch]->lfo.TP_FRAC = ((lq24_tp+((uint32_t)1<<17))>>18)&0x3F;
                tp_next = (lq24_tp>>24)&0xFFF;

                theta++;
                if ( theta >= depth*4 )
                {
                    theta = 0;
                }
            }

            slot.ch_info_list[ch]->lfo.theta = theta;
            slot.ch_info_list[ch]->lfo.DELTA_FRAC = q6_delta&0x3F;

            slot.psg_reg.data[2*ch+0] = U16_LO(tp_next);
            slot.psg_reg.data[2*ch+1] = U16_HI(tp_next);
            if ( ( (slot.psg_reg.data[0x7]>>ch) & 0x9 ) != 0x9 )
            {/* Not muted. */
                slot.psg_reg.flags_addr  |= 0x3<<(2*ch);
            }
        }
    }

    void init_ch_info(CHANNEL_INFO *p_ch_info)
    {
        *p_ch_info = (CHANNEL_INFO){};
        p_ch_info->tone.tempo = DEFAULT_TEMPO;
        p_ch_info->tone.note_len = DEFAULT_NOTE_LENGTH;
        p_ch_info->tone.OCTAVE = DEFAULT_OCTAVE-1;
        p_ch_info->tone.GATE_TIME = DEFAULT_GATE_TIME-1;
        p_ch_info->tone.VOLUME = DEFAULT_VOLUME_LEVEL;
        p_ch_info->tone.BIAS = DEFAULT_BIAS_LEVEL+BIAS_LEVEL_OFS;
    }

    void reset_mml(SLOT &slot)
    {
        int16_t i;
        int16_t ch;
        const char *p_mml_head;
        uint16_t mml_len;

        slot.psg_reg.data[0x7]   = 0x3F;
        slot.psg_reg.flags_addr  = 1<<0x7;
        slot.psg_reg.flags_mixer = 0;
        for ( i = 0; i < slot.gl_info.sys_status.NUM_CH_USED; i++ )
        {
            ch = slot.gl_info.sys_status.REVERSE == 1 ? (NUM_CHANNEL-(i+1)) : i;
            slot.psg_reg.flags_mixer |= (1<<ch);

            p_mml_head = slot.ch_info_list[ch]->mml.p_mml_head;
            mml_len = slot.ch_info_list[ch]->mml.mml_len;

            init_ch_info(slot.ch_info_list[ch]);

            slot.ch_info_list[ch]->mml.p_mml_head = p_mml_head;
            slot.ch_info_list[ch]->mml.mml_len = mml_len;
        }
    }
}

namespace PsgCtrl
{
    void init_slot( SLOT    &slot
            , uint32_t      s_clock
            , bool          reverse
            , CHANNEL_INFO  *p_ch0
            , CHANNEL_INFO  *p_ch1
            , CHANNEL_INFO  *p_ch2
            )
    {
        int16_t i;
        CHANNEL_INFO *p_list[NUM_CHANNEL] = { p_ch0, p_ch1, p_ch2 };

        slot = (SLOT){};

        slot.gl_info.s_clock = s_clock;
        slot.gl_info.sys_status.REVERSE = reverse ? 1 : 0;
        slot.gl_info.sys_status.NUM_CH_IMPL = 0;

        for ( i = 0; i < NUM_CHANNEL; i++ )
        {
            slot.ch_info_list[
                reverse ? (NUM_CHANNEL-(i+1)) : i
            ] = p_list[i];

            if ( p_list[i] != nullptr )
            {
                init_ch_info(p_list[i]);
                slot.gl_info.sys_status.NUM_CH_IMPL++;
            }
            else
            {
                break;
            }
        }

        slot.psg_reg.data[0x7]   = 0x3F;
        slot.psg_reg.flags_addr  = 1<<0x7;
        slot.psg_reg.flags_mixer = 0x7;
    }

    int set_mml(SLOT &slot, const char *p_mml, uint16_t mode)
    {
        int16_t i;
        int16_t ch;

        if ( p_mml == nullptr )
        {
            return -1;
        }

        slot.gl_info.sys_status.RH_LEN = (( mode & 0x1 ) != 0) ? 1 : 0;

        skip_white_space(&p_mml);

        slot.gl_info.p_mml_text = p_mml;
        slot.gl_info.sys_status.NUM_CH_USED = 0;

        for ( i = 0; i < slot.gl_info.sys_status.NUM_CH_IMPL; i++ )
        {
            ch = ( slot.gl_info.sys_status.REVERSE == 1 ) ? NUM_CHANNEL-(i+1) : i;

            slot.ch_info_list[ch]->mml.p_mml_head = p_mml;
            slot.ch_info_list[ch]->mml.ofs_mml_pos = 0;
            while ((*p_mml != ',') && (*p_mml != '\0')) p_mml++;

            slot.ch_info_list[ch]->mml.mml_len = (p_mml - slot.ch_info_list[ch]->mml.p_mml_head);

            slot.ch_info_list[ch]->ch_status.DECODE_END = 0;
            slot.gl_info.sys_status.NUM_CH_USED++;
            if ( *p_mml == '\0' )
            {
                break;
            }
            p_mml++;
        }

        slot.gl_info.sys_status.SET_MML = 1;

        return 0;
    }

    void control_psg(SLOT &slot)
    {
        int16_t i;
        int16_t ch;
        int16_t decode_end_cnt = 0;

        slot.gl_info.sys_status.CTRL_STAT_PRE = slot.gl_info.sys_status.CTRL_STAT;

        if ( slot.gl_info.sys_status.SET_MML == 0 )
        {
            return;
        }

        if ( slot.gl_info.sys_request.CTRL_REQ_FLAG != 0 )
        {
            slot.gl_info.sys_request.CTRL_REQ_FLAG = 0;

            if ( slot.gl_info.sys_request.CTRL_REQ == CTRL_REQ_PLAY )
            {
                reset_mml(slot);
                slot.gl_info.sys_status.CTRL_STAT = CTRL_STAT_PLAY;
            }
            else
            {
                slot.gl_info.sys_status.CTRL_STAT = CTRL_STAT_STOP;
            }
        }

        if ( ( slot.gl_info.sys_status.CTRL_STAT == CTRL_STAT_STOP )
          || ( slot.gl_info.sys_status.CTRL_STAT == CTRL_STAT_END  ) )
        {
            return;
        }

        for ( i = 0; i < slot.gl_info.sys_status.NUM_CH_USED; i++ )
        {
            ch = ( slot.gl_info.sys_status.REVERSE == 1 ) ? NUM_CHANNEL-(i+1) : i;

            if ( slot.ch_info_list[ch]->time.note_on > 0 )
            {
                slot.ch_info_list[ch]->time.note_on--;
            }
            if ( slot.ch_info_list[ch]->time.gate > 0 )
            {
                slot.ch_info_list[ch]->time.gate--;
            }
            if ( slot.ch_info_list[ch]->time.note_on == 0 )
            {
                if ( slot.ch_info_list[ch]->ch_status.DECODE_END == 0 )
                {
                    decode_mml(slot, ch);
                }
                else
                {
                    decode_end_cnt++;
                }
            }
            if ( slot.ch_info_list[ch]->time.gate == 0 )
            {
                /* Mute tone and noise */
                if ( ((slot.psg_reg.data[0x7]>>ch)&0x9) != 0x9 )
                {
                    slot.psg_reg.data[0x7]   |= (0x9<<ch);    
                    slot.psg_reg.flags_addr  |= 1<<0x7;
                    slot.psg_reg.flags_mixer |= (1<<ch);
                }
            }

            /* PITCHBEND BLOCK */
            proc_pitchbend(slot, ch);

            /* SOFTWARE ENVELOPE GENERATOR BLOCK */
            if ( slot.ch_info_list[ch]->ch_status.SW_ENV_MODE == 1 )
            {
                proc_sw_env_gen(slot, ch);
            }
            /* LFO BLOCK */
            if ( slot.ch_info_list[ch]->ch_status.LFO_MODE == 1 )
            {
                proc_lfo(slot, ch);
            }
        }

        if ( decode_end_cnt >= slot.gl_info.sys_status.NUM_CH_USED )
        {
            slot.gl_info.sys_status.CTRL_STAT = CTRL_STAT_END;
        }
    }
}
