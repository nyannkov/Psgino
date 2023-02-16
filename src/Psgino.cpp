/*
 * MIT License, see the LICENSE file for details.
 *
 * Copyright (c) 2023 nyannkov
 */
#include "Psgino.h"

Psgino::Psgino(void (*write)(uint8_t addr, uint8_t data), float fs_clock, uint8_t proc_freq)
{
    this->p_write = write;
    PsgCtrl::init_slot( this->slot0
                     , (uint32_t)(fs_clock*100+0.5F)
                     , proc_freq
                     , false
                     , &this->ch0
                     , &this->ch1
                     , &this->ch2
                     );
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

void Psgino::Proc()
{
    uint16_t addr;
    PsgCtrl::control_psg(this->slot0);

    if ( this->p_write == nullptr )
    {
        return;
    }

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

PsginoZ::PsginoZ(void (*write)(uint8_t addr, uint8_t data), float fs_clock, uint8_t proc_freq)
       : Psgino(write, fs_clock, proc_freq)
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

void PsginoZ::Proc()
{
    int16_t i;
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
        /* MASK HW ENV SETTINGS */
        if ( ( this->slot1.psg_reg.data[0x8+i] & 0x10 ) != 0 )
        {
            this->reg_mask   |= (0x7 << 0xB);
        }

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
