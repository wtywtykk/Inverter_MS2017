#include "Global.h"
#include "Clock.h"

ushort FlybackPWMMax = 0;
short FlybackLoopMaxDuty = 0;

uchar FlybackReady = 0;

void FlybackPWMSetOutputDuty(ushort Duty)
{
	if (Duty > FlybackPWMMax)
	{
		Duty = FlybackPWMMax + 1;
	}
	TA2CCR1 = Duty;
}

void FlybackPWMGen_Init(void)
{
	FlybackPWMMax = QueryClock(SMCLK) / 34567 - 1;//34.567kHz
	assert(FlybackPWMMax <= 0xFFFF);
	TA2EX0 = 0;
	TA2CTL = TASSEL__SMCLK | ID__1 | MC__STOP;
	TA2CCTL0 = 0;
	TA2CCTL1 = CM_0 | CCIS_2 | OUTMOD_7;
	TA2CCR0 = FlybackPWMMax;

	FlybackLoopMaxDuty = FlybackPWMMax * 40 / 100;

	FlybackPWMSetOutputDuty(0);
	TA2CTL |= TACLR;
	TA2CTL |= MC__UP;
	__delay_cycles(10000);
	//P2.4
	P2DIR |= (1 << 4);
	P2SEL |= (1 << 4);
}

void FlybackDoFeedback(ushort ADCValue)
{
	ushort FlybackVolt = (ulong)ADCValue * 18000 / 0xFFF;
	const ushort StandardVolt = 12000;
	const ushort HighThreshold = 14000;
	const ushort LowThreshold = 10000;
	static ushort LastVolt = 0;
	short FeedbackDiff = FlybackVolt - StandardVolt + (FlybackVolt - LastVolt) * 2;
	static short FlybackLoopDuty = 0;
	FeedbackDiff /= 16;
	FlybackLoopDuty -= FeedbackDiff;
	LastVolt = FlybackVolt;
	if ((FlybackVolt < LowThreshold) | (FlybackVolt > HighThreshold))
	{
		FlybackReady = 0;
	}
	else
	{
		FlybackReady = 1;
	}
	if (FlybackLoopDuty > FlybackLoopMaxDuty)
	{
		FlybackLoopDuty = FlybackLoopMaxDuty;
	}
	if (FlybackLoopDuty < -FlybackLoopMaxDuty / 4)
	{
		FlybackLoopDuty = -FlybackLoopMaxDuty / 4;
	}
	if (FlybackLoopDuty > 0)
	{
		FlybackPWMSetOutputDuty(FlybackLoopDuty);
	}
	else
	{
		FlybackPWMSetOutputDuty(0);
	}
}

uchar FlybackIsReady(void)
{
	return FlybackReady;
}
