/*
 * MIT License, see the LICENSE file for details.
 *
 * Copyright (c) 2023 nyannkov
 */
#ifndef PSGINO_SAMPLE_DRIVERS_H
#define PSGINO_SAMPLE_DRIVERS_H

#if defined(PSGINO_USE_SAMPLE_DRIVER_YMZ294)
#include "./sample_drivers/ymz294/driver_ymz294.h"
#endif

#if defined(PSGINO_USE_SAMPLE_DRIVER_AY_3_8910)
#include "./sample_drivers/ay_3_8910/driver_ay_3_8910.h"
#endif

#endif/*PSGINO_SAMPLE_DRIVERS_H*/
