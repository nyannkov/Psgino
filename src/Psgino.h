#ifndef PSGINO_H
#define PSGINO_H

#include "psg_ctrl.h"

class Psgino
{
public:
    enum PlayStatus { PlayStop, Playing, PlayEnd };
    Psgino(void (*p_write)(uint8_t addr, uint8_t data), float fs_clock);
    void SetMML(const char *mml, uint16_t mode = 0);
    void Play();
    void Stop();
    PlayStatus GetStatus();
    void Proc();
protected:
    PsgCtrl::SLOT slot0;
    void (*p_write)(uint8_t addr, uint8_t data);
};


#endif/*PSGINO_H*/
