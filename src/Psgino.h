/*
 * MIT License, see the LICENSE file for details.
 *
 * Copyright (c) 2023 nyannkov
 */
#ifndef PSGINO_H
#define PSGINO_H

#include "./psg_ctrl/psg_ctrl.h"

/**
 * @class Psgino
 * @brief A class for controlling PSG (Programmable Sound Generator) with MML (Music Macro Language) support.
 * 
 * This class provides methods to play and control sound using PSG, including setting MML commands,
 * adjusting playback speed, and handling sound generation.
 */
class Psgino {
public:
    /**
     * @brief Default constructor for Psgino.
     */
    Psgino();

    /**
     * @brief Parameterized constructor for Psgino.
     * 
     * @param write Function pointer for writing data to the PSG.
     * @param fs_clock The system clock frequency of the PSG.
     * @param proc_freq The processing frequency in Hertz (Hz), default is `PsgCtrl::DEFAULT_PROC_FREQ`.
     * @param reset Function pointer for resetting the PSG (default is nullptr).
     * 
     * The `reset` function pointer, if provided, will be called during the execution of the `Reset()` method.
     * Register this function if you want to perform a hardware reset of the PSG during `Reset()`.
     */
    Psgino(void (*write)(uint8_t addr, uint8_t data)
         , float fs_clock
         , uint16_t proc_freq = PsgCtrl::DEFAULT_PROC_FREQ
         , void (*reset)() = nullptr
    );

    /**
     * @brief Sets the MML string for playback.
     * 
     * @param mml The MML string to be processed.
     * @param mode Mode for MML processing (default is 0).
     * 
     * For details on the `mode` parameter, refer to the M command in the Header command section of MML.md.
     */
    void SetMML(const char *mml, uint16_t mode = 0);

    /**
     * @brief Starts playback of the MML string.
     */
    void Play();

    /**
     * @brief Stops playback of the MML string.
     */
    void Stop();

    /**
     * @brief Enumeration for the playback status.
     */
    enum PlayStatus {
        PlayStop, ///< Playback has stopped.
        Playing,  ///< Playback is in progress.
        PlayEnd   ///< Playback has ended.
    };

    /**
     * @brief Gets the current playback status.
     * 
     * @return Current playback status.
     */
    PlayStatus GetStatus();

    /**
     * @brief Sets a user-defined callback function.
     * 
     * This callback function is executed when the @C command within the MML is decoded.
     * 
     * @param cb Function pointer for the user callback.
     */
    void SetUserCallback(void (*cb)(uint8_t ch, int32_t param));

    /**
     * @brief Completes the primary loop of the PSG processing.
     * 
     * For example, if an MML sequence is being played in an infinite loop, calling this method will cause the playback to exit the loop 
     * when it reaches the loop's end, rather than returning to the beginning. The loop will terminate, and playback will continue decoding the subsequent MML.
     */
    void FinishPrimaryLoop();

    /**
     * @brief Sets the speed factor for playback.
     * 
     * The tempo can be adjusted using the `speed_factor`. The unit of `speed_factor` is percent (%).
     * The minimum value is 20% and the maximum value is 500%.
     * 
     * @param speed_factor The speed factor to be set.
     * 
     * @note If this function is called during playback and the `speed_factor` is set too high,
     * it may cause timing discrepancies between the channels.
     */
    void SetSpeedFactor(uint16_t speed_factor);

    /**
     * @brief Shifts the frequency of the sound.
     * 
     * The `shift_degrees` parameter can range from -1800 degrees to 1800 degrees. 
     * 
     * Setting `shift_degrees` to 360 degrees will raise the pitch by one octave, while setting it to -360 degrees will lower the pitch by one octave.
     * 
     * @param shift_degrees The amount to shift the frequency, in degrees.
     */
    void ShiftFrequency(int16_t shift_degrees);

    /**
     * @brief Initializes the PSG with the given parameters.
     * 
     * @param write Function pointer for writing data to the PSG.
     * @param fs_clock The system clock frequency of the PSG.
     * @param proc_freq The processing frequency in Hertz (Hz), default is `PsgCtrl::DEFAULT_PROC_FREQ`.
     * @param reset Function pointer for resetting the PSG (default is nullptr).
     * 
     * The `reset` function pointer, if provided, will be called during the execution of the `Reset()` method.
     * Register this function if you want to perform a hardware reset of the PSG during `Reset()`.
     */
    virtual void Initialize(void (*write)(uint8_t addr, uint8_t data)
         , float fs_clock
         , uint16_t proc_freq = PsgCtrl::DEFAULT_PROC_FREQ
         , void (*reset)() = nullptr
    );

    /**
     * @brief Processes the PSG operations.
     * 
     * This method should be called at the frequency specified by the `proc_freq` parameter
     * from the Psgino constructor or Initialize method, in Hertz (Hz).
     */
    virtual void Proc();

    /**
     * @brief Resets the PSG to its initial state.
     */
    virtual void Reset();

protected:
    /** 
     * @brief PSG control handler used for MML playback.
     */
    PsgCtrl::SLOT slot0;

    /** 
     * @brief Function pointer for writing data to the PSG.
     */
    void (*p_write)(uint8_t addr, uint8_t data);

    /** 
     * @brief Function pointer for resetting the PSG.
     */
    void (*p_reset)();

private:
    /** 
     * @brief Channel information for channel 0 (Channel A).
     */
    PsgCtrl::CHANNEL_INFO ch0;

    /** 
     * @brief Channel information for channel 1 (Channel B).
     */
    PsgCtrl::CHANNEL_INFO ch1;

    /** 
     * @brief Channel information for channel 2 (Channel C).
     */
    PsgCtrl::CHANNEL_INFO ch2;
};

/**
 * @class PsginoZ
 * @brief A derived class from Psgino that provides additional functionality for sound effects (SE).
 * 
 * This class extends the Psgino class to handle sound effects separately from the main MML playback,
 * offering additional controls for SE-specific playback and processing.
 */
class PsginoZ : public Psgino {
public:
    /**
     * @brief Default constructor for PsginoZ.
     */
    PsginoZ();

    /**
     * @brief Parameterized constructor for PsginoZ.
     * 
     * @param write Function pointer for writing data to the PSG.
     * @param fs_clock The system clock frequency of the PSG.
     * @param proc_freq The processing frequency in Hertz (Hz), default is `PsgCtrl::DEFAULT_PROC_FREQ`.
     * @param reset Function pointer for resetting the PSG (default is nullptr).
     * 
     * The `reset` function pointer, if provided, will be called during the execution of the `Reset()` method.
     * Register this function if you want to perform a hardware reset of the PSG during `Reset()`.
     */
    PsginoZ(void (*write)(uint8_t addr, uint8_t data)
          , float fs_clock
          , uint16_t proc_freq = PsgCtrl::DEFAULT_PROC_FREQ
          , void (*reset)() = nullptr
    );

    /**
     * @brief Sets the MML string specifically for sound effects (SE).
     * 
     * @param mml The MML string to be used for SE processing.
     * @param mode The mode for SE MML processing (default is 0).
     * 
     * @note Only one channel can be used for SE MML (single note only). 
     * SE playback utilizes Channel 2 (Channel C).
     */
    void SetSeMML(const char *mml, uint16_t mode = 0);

    /**
     * @brief Starts playback of the SE MML string.
     */
    void PlaySe();

    /**
     * @brief Stops playback of the SE MML string.
     */
    void StopSe();

    /**
     * @brief Gets the current playback status of the SE.
     * 
     * @return Current SE playback status.
     */
    PlayStatus GetSeStatus();

    /**
     * @brief Sets a user-defined callback function specifically for SE.
     * 
     * This callback function is executed when the @C command within the MML is decoded.
     * 
     * @param cb Function pointer for the user callback.
     */
    void SetSeUserCallback(void (*cb)(uint8_t ch, int32_t param));

    /**
     * @brief Sets the speed factor specifically for SE playback.
     * 
     * The tempo can be adjusted using the `speed_factor`. The unit of `speed_factor` is percent (%).
     * The minimum value is 20% and the maximum value is 500%.
     * 
     * @param speed_factor The speed factor to be set.
     * 
     * @note If this function is called during playback and the `speed_factor` is set too high,
     * it may cause timing discrepancies between the channels.
     */
    void SetSeSpeedFactor(uint16_t speed_factor);

    /**
     * @brief Shifts the frequency of the SE sound.
     * 
     * The `shift_degrees` parameter can range from -1800 degrees to 1800 degrees. 
     * 
     * Setting `shift_degrees` to 360 degrees will raise the pitch by one octave, while setting it to -360 degrees will lower the pitch by one octave.
     * 
     * @param shift_degrees The amount to shift the frequency, in degrees.
     */
    void ShiftSeFrequency(int16_t shift_degrees);

    /**
     * @brief Initializes the PSG with the given parameters, specifically for SE.
     * 
     * @param write Function pointer for writing data to the PSG.
     * @param fs_clock The system clock frequency of the PSG.
     * @param proc_freq The processing frequency in Hertz (Hz), default is `PsgCtrl::DEFAULT_PROC_FREQ`.
     * @param reset Function pointer for resetting the PSG (default is nullptr).
     * 
     * The `reset` function pointer, if provided, will be called during the execution of the `Reset()` method.
     * Register this function if you want to perform a hardware reset of the PSG during `Reset()`.
     */
    void Initialize(void (*write)(uint8_t addr, uint8_t data)
         , float fs_clock
         , uint16_t proc_freq = PsgCtrl::DEFAULT_PROC_FREQ
         , void (*reset)() = nullptr
    ) override;

    /**
     * @brief Processes the PSG operations, including SE processing.
     * 
     * This method should be called at the frequency specified by the `proc_freq` parameter
     * from the Psgino constructor or Initialize method, in Hertz (Hz).
     */
    void Proc() override;

    /**
     * @brief Resets the PSG to its initial state, including resetting SE-specific states.
     */
    void Reset() override;

private:
    /** 
     * @brief PSG control handler used for SE MML playback.
     */
    PsgCtrl::SLOT slot1;

    /** 
     * @brief Channel information for SE.
     * 
     * Although named `ch0_se`, this corresponds to Channel 2 (Channel C) of the PSG.
     */
    PsgCtrl::CHANNEL_INFO ch0_se;

    /** 
     * @brief Register mask for SE processing.
     */
    uint16_t reg_mask;

    /** 
     * @brief Mixer mask for SE processing.
     */
    uint8_t mixer_mask;
};

#endif/*PSGINO_H*/
