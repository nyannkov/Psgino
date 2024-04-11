/*
 * MIT License, see the LICENSE file for details.
 *
 * Copyright (c) 2023 nyannkov
 */
#include "Psgino.h"

Psgino::Psgino()
{
}

Psgino::Psgino(void (*write)(uint8_t addr, uint8_t data), float fs_clock, uint8_t proc_freq, void (*reset)())
{
    Psgino::Initialize(write, fs_clock, proc_freq, reset);
}

void Psgino::SetMML(const char *mml, uint16_t mode)
{
    PsgCtrl::set_mml(this->slot0, mml, mode);
}

void Psgino::Play()
{
    this->slot0.gl_info.sys_request.CTRL_REQ = PsgCtrl::CTRL_REQ_PLAY;
    this->slot0.gl_info.sys_request.CTRL_REQ_FLAG = 1;
}

void Psgino::Stop()
{
    this->slot0.gl_info.sys_request.CTRL_REQ = PsgCtrl::CTRL_REQ_STOP;
    this->slot0.gl_info.sys_request.CTRL_REQ_FLAG = 1;
}

Psgino::PlayStatus Psgino::GetStatus()
{
    switch ( this->slot0.gl_info.sys_status.CTRL_STAT )
    {
    case PsgCtrl::CTRL_STAT_STOP:
        return Psgino::PlayStop;

    case PsgCtrl::CTRL_STAT_PLAY:
        return Psgino::Playing;

    case PsgCtrl::CTRL_STAT_END:
        return Psgino::PlayEnd;

    default:
        this->Stop();
        return Psgino::PlayStop;
    }
}

void Psgino::SetUserCallback(void (*cb)(uint8_t ch, int32_t params))
{
    PsgCtrl::set_user_callback(this->slot0, cb);
}

void Psgino::Initialize(void (*write)(uint8_t addr, uint8_t data), float fs_clock, uint8_t proc_freq, void (*reset)())
{
    this->p_write = write;
    this->p_reset = reset;
    PsgCtrl::init_slot( this->slot0
                     , (uint32_t)(fs_clock*100+0.5F)
                     , proc_freq
                     , false
                     , &this->ch0
                     , &this->ch1
                     , &this->ch2
                     );
 }

void Psgino::Proc()
{
    uint8_t addr;

    if ( this->p_write == nullptr )
    {
        return;
    }

    PsgCtrl::control_psg(this->slot0);

    for ( addr = 0; addr <= 0xF; addr++ )
    {
        if ( ( (this->slot0.psg_reg.flags_addr >> addr) & 0x1 ) != 0 )
        {
            this->p_write(addr, this->slot0.psg_reg.data[addr]);
        }
    }

    this->slot0.psg_reg.flags_addr = 0;
    this->slot0.psg_reg.flags_mixer = 0;
}

void Psgino::Reset()
{
    PsgCtrl::reset(this->slot0);

    if ( this->p_reset != nullptr )
    {
        this->p_reset();
    }

    if ( this->p_write != nullptr )
    {
        uint8_t addr;
        for ( addr = 0; addr <= 0xF; addr++ )
        {
            this->p_write(addr, (addr == 0x7) ? 0x3F : 0x00);
        }
    }
}

void Psgino::FinishPrimaryLoop()
{
    this->slot0.gl_info.sys_request.FIN_PRI_LOOP_FLAG = 1;
}

PsginoZ::PsginoZ()
       : Psgino()
{
}

PsginoZ::PsginoZ(void (*write)(uint8_t addr, uint8_t data), float fs_clock, uint8_t proc_freq, void (*reset)())
       : Psgino(write, fs_clock, proc_freq, reset)
{
    PsgCtrl::init_slot( this->slot1
                     , (uint32_t)(fs_clock*100+0.5F)
                     , proc_freq
                     , true
                     , &this->ch0_se
                     );

    this->reg_mask = 0;
    this->mixer_mask = 0;
}

void PsginoZ::SetSeMML(const char *mml, uint16_t mode)
{
    PsgCtrl::set_mml(this->slot1, mml, mode);
}

PsginoZ::PlayStatus PsginoZ::GetSeStatus()
{
    switch ( this->slot1.gl_info.sys_status.CTRL_STAT )
    {
    case PsgCtrl::CTRL_STAT_STOP:
        return Psgino::PlayStop;

    case PsgCtrl::CTRL_STAT_PLAY:
        return Psgino::Playing;

    case PsgCtrl::CTRL_STAT_END:
        return Psgino::PlayEnd;

    default:
        this->StopSe();
        return Psgino::PlayStop;
    }
}

void PsginoZ::PlaySe()
{
    this->slot1.gl_info.sys_request.CTRL_REQ = PsgCtrl::CTRL_REQ_PLAY;
    this->slot1.gl_info.sys_request.CTRL_REQ_FLAG = 1;
}

void PsginoZ::StopSe()
{
    this->slot1.gl_info.sys_request.CTRL_REQ = PsgCtrl::CTRL_REQ_STOP;
    this->slot1.gl_info.sys_request.CTRL_REQ_FLAG = 1;
}

void PsginoZ::SetSeUserCallback(void (*cb)(uint8_t ch, int32_t params))
{
    PsgCtrl::set_user_callback(this->slot1, cb);
}

void PsginoZ::Initialize(void (*write)(uint8_t addr, uint8_t data), float fs_clock, uint8_t proc_freq, void (*reset)())
{
    Psgino::Initialize(write, fs_clock, proc_freq, reset);

    PsgCtrl::init_slot( this->slot1
                     , (uint32_t)(fs_clock*100+0.5F)
                     , proc_freq
                     , true
                     , &this->ch0_se
                     );

    this->reg_mask = 0;
    this->mixer_mask = 0;
}

void PsginoZ::Proc()
{
    uint8_t i;
    uint16_t masked_flags_addr;
    uint16_t mixer;

    if ( this->p_write == nullptr )
    {
        return;
    }

    PsgCtrl::control_psg(this->slot0);
    PsgCtrl::control_psg(this->slot1);

    for ( i = 0; i < PsgCtrl::NUM_CHANNEL; i++ )
    {
        /* MASK TP AND VOLUME CONTROL */
        if ( ( this->slot1.psg_reg.flags_mixer & (1 << i) ) != 0 )
        {
            if ( ( this->slot1.psg_reg.data[0x7] & (1 << i) ) == 0 )
            {
                this->mixer_mask |= (0x1 << i);
                this->reg_mask   |= (0x3 << (0x2*i));
                this->reg_mask   |= (0x1 << (0x8+i));
            }

            /* MASK NOISE SETTINGS */
            if ( ( this->slot1.psg_reg.data[0x7] & (1 << (0x3+i)) ) == 0 )
            {
                this->mixer_mask |= (0x1 << i);
                this->mixer_mask |= (0x7 << 0x3);
                this->reg_mask   |= (0x1 << 0x6);
                this->reg_mask   |= (0x1 << (0x8+i));
            }

            /* MASK HW ENV SETTINGS */
            if ( ( this->slot1.psg_reg.data[0x8+i] & 0x10 ) != 0 )
            {
                this->reg_mask   |= (0x7 << 0xB);
            }
        }
    }

    masked_flags_addr = this->slot0.psg_reg.flags_addr & ~this->reg_mask; 

    mixer  = this->slot0.psg_reg.data[0x7] & ~this->mixer_mask;
    mixer |= this->slot1.psg_reg.data[0x7] & this->mixer_mask;
    mixer &= 0x3F;

    for ( i = 0; i <= 0xF; i++ )
    {
        if ( ( this->slot1.psg_reg.flags_addr & (1<<i) ) != 0 )
        {
            this->p_write(i, (i==0x7) ? mixer : this->slot1.psg_reg.data[i]);
        }
        else if ( ( masked_flags_addr & (1<<i) ) != 0 )
        {
            this->p_write(i, (i==0x7) ? mixer : this->slot0.psg_reg.data[i]);
        }
        else
        {
        }
    }

    this->slot0.psg_reg.flags_addr = 0;
    this->slot0.psg_reg.flags_mixer = 0;
    this->slot1.psg_reg.flags_addr = 0;
    this->slot1.psg_reg.flags_mixer = 0;

    if ( this->slot1.gl_info.sys_status.CTRL_STAT != this->slot1.gl_info.sys_status.CTRL_STAT_PRE )
    {
        if ( this->slot1.gl_info.sys_status.CTRL_STAT != PsgCtrl::CTRL_STAT_PLAY )
        {
            this->slot0.psg_reg.data[0x7] &= ~this->mixer_mask;
            this->slot0.psg_reg.data[0x7] |= 0x3F&this->mixer_mask;
            this->reg_mask = 0;
            this->mixer_mask = 0;
        }
    }
}

void PsginoZ::Reset()
{
    PsgCtrl::reset(this->slot1);
    Psgino::Reset();
}
