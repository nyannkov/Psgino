#include "Psgino.h"

Psgino::Psgino(void (*write)(uint8_t addr, uint8_t data), float fs_clock)
{
    this->p_write = write;

    PsgCtrl::init_slot( this->slot0
                     , (uint32_t)(fs_clock*100+0.5F)
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
    this->slot0.gl_info.sys_request.CTRL_REQ = PsgCtrl::CTRL_STAT_PLAY;
}

void Psgino::Stop()
{
    this->slot0.gl_info.sys_request.CTRL_REQ = PsgCtrl::CTRL_STAT_STOP;
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

PsginoZ::PsginoZ(void (*write)(uint8_t addr, uint8_t data), float fs_clock)
       : Psgino(write, fs_clock)
{
    PsgCtrl::init_slot( this->slot1
                     , (uint32_t)(fs_clock*100+0.5F)
                     , true
                     , &this->ch_se_0
                     );
}

void PsginoZ::SetSeMML(const char *mml, uint16_t mode)
{
    PsgCtrl::set_mml(this->slot1, mml, mode);
}

void PsginoZ::PlaySe()
{
    this->slot1.gl_info.sys_request.CTRL_REQ = PsgCtrl::CTRL_STAT_PLAY;
}

void PsginoZ::StopSe()
{
    this->slot1.gl_info.sys_request.CTRL_REQ = PsgCtrl::CTRL_STAT_STOP;
}
