#pragma once

#include "Global.h"

#define SPWMGenFFTSize 256
#define SPWMGenFFTSampleFrequency 512

void SPWMGen_Init(void);
void SPWM_DoOutputPredistortion(ushort OutputVal[SPWMGenFFTSize]);
