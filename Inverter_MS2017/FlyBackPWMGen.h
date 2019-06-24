#pragma once

#include "Global.h"

void FlybackPWMGen_Init(void);
void FlybackDoFeedback(ushort ADCValue);
uchar FlybackIsReady(void);
