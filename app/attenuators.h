#ifndef ATTENUATORS_H
#define ATTENUATORS_H

#include "mbed.h"
#include "Drivers.h"
#include "pcbnAPI.h"
#include "util.h"

#define MIN_ATTENUATION    0.0
#define MAX_ATTENUATION    31.5

extern FeRAM feram;

double attenuators_update( double att );

#endif
