#include "Psgino.h"

Psgino::Psgino(void (*p_write)(uint8_t addr, uint8_t data), float fs_clock)
{
    this->p_write = p_write;
    PsgCtrl::init_slot(this->slot0, (uint32_t)(fs_clock*100+0.5F));
}

void Psgino::SetMML(const char *mml, uint16_t mode)
{
    PsgCtrl::set_mml(this->slot0, mml, mode, 0);
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

