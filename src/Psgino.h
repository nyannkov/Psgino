/*
 * MIT License, see the LICENSE file for details.
 *
 * Copyright (c) 2023 nyannkov
 */
#ifndef PSGINO_H
#define PSGINO_H

#include "./psg_ctrl/psg_ctrl.h"

class Psgino
{
public:
    Psgino(void (*write)(uint8_t addr, uint8_t data)
         , float fs_clock
         , uint8_t proc_freq = PsgCtrl::DEFAULT_PROC_FREQ
         , void (*reset)() = nullptr
         );
    void SetMML(const char *mml, uint16_t mode = 0);
    void Play();
    void Stop();
    enum PlayStatus { PlayStop, Playing, PlayEnd };
    PlayStatus GetStatus();
    virtual void Proc();
    virtual void Reset();

protected:
    PsgCtrl::SLOT           slot0;
    void (*p_write)(uint8_t addr, uint8_t data);
    void (*p_reset)();

private:
    PsgCtrl::CHANNEL_INFO   ch0;
    PsgCtrl::CHANNEL_INFO   ch1;
    PsgCtrl::CHANNEL_INFO   ch2;
};

class PsginoZ : public Psgino
{
public:
    PsginoZ(void (*write)(uint8_t addr, uint8_t data)
          , float fs_clock
          , uint8_t proc_freq = PsgCtrl::DEFAULT_PROC_FREQ
          , void (*reset)() = nullptr
          );
    void SetSeMML(const char *mml, uint16_t mode = 0);
    void PlaySe();
    void StopSe();
    PlayStatus GetSeStatus();
    void Proc() override;
    void Reset() override;

private:
    PsgCtrl::SLOT           slot1;
    PsgCtrl::CHANNEL_INFO   ch0_se;
    uint16_t reg_mask;
    uint8_t mixer_mask;
};

#endif/*PSGINO_H*/
