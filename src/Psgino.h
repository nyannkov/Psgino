#ifndef PSGINO_H
#define PSGINO_H

#include "psg_ctrl.h"

class Psgino
{
public:
    Psgino(void (*write)(uint8_t addr, uint8_t data), float fs_clock);
    void SetMML(const char *mml, uint16_t mode = 0);
    void Play();
    void Stop();
    enum PlayStatus { PlayStop, Playing, PlayEnd };
    PlayStatus GetStatus();
    void Proc();

protected:
    PsgCtrl::SLOT           slot0;
    PsgCtrl::CHANNEL_INFO   ch0;
    PsgCtrl::CHANNEL_INFO   ch1;
    PsgCtrl::CHANNEL_INFO   ch2;
    void (*p_write)(uint8_t addr, uint8_t data);
};

class PsginoZ : public Psgino
{
public:
    PsginoZ(void (*write)(uint8_t addr, uint8_t data), float fs_clock);
    void SetSeMML(const char *mml, uint16_t mode = 0);
    void PlaySe();
    void StopSe();
    PlayStatus GetSeStatus();
    void Proc();

private:
    PsgCtrl::SLOT           slot1;
    PsgCtrl::CHANNEL_INFO   ch_se_0;
};

#endif/*PSGINO_H*/
