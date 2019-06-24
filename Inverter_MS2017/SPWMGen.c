#include "Global.h"
#include "Clock.h"
#include <math.h>
#include "SPWMGen.h"
#include "DSP\include\DSPLib.h"

#define DeadTime 5
ushort PWMMax = 0;

#define SinTableSize 512
#define SampleFrequency (SinTableSize * 50L)
#define MaxSensibleHarmonicOrder 13

ulong Ticks = 0;
ushort SinTable[SinTableSize];
int16_t FFTBuffer[SPWMGenFFTSize * 2];

float FundamentalAmplitude;
float CompensateTable[SinTableSize];

void SPWMReGenerateSinTable(void);

void SPWMSetOutputDuty(ushort Duty)
{
	const ushort DeadTimeHi = DeadTime / 2;
	const ushort DeadTimeLo = DeadTime - DeadTimeHi;
	if (Duty < DeadTime)
	{
		Duty = DeadTime;
	}
	if (Duty > PWMMax - DeadTime)
	{
		Duty = PWMMax - DeadTime;
	}
	TB0CCR1 = Duty - DeadTimeHi;
	TB0CCR2 = Duty + DeadTimeLo;
}

void SPWMInitPWMGen(void)
{
	PWMMax = QueryClock(SMCLK) / 2 / 21345 - 1;//21.345kHz
	assert(PWMMax <= 0xFFFF);
	TB0EX0 = 0;
	TB0CTL = TBCLGRP_1 | CNTL__16 | TBSSEL__SMCLK | ID__1 | MC__STOP;
	TB0CCTL0 = 0;
	TB0CCTL1 = CM_0 | CCIS_2 | CLLD_2 | OUTMOD_2;//Hi
	TB0CCTL2 = CM_0 | CCIS_2 | CLLD_2 | OUTMOD_6;//Lo
	TB0CCR0 = PWMMax;
	SPWMSetOutputDuty(0);
	TB0CTL |= TBCLR;
	TB0CTL |= MC__UPDOWN;
	__delay_cycles(10000);
	//P5.7,P7.4
	P5DIR |= (1 << 7);
	P5SEL |= (1 << 7);
	P7DIR |= (1 << 4);
	P7SEL |= (1 << 4);
}

void SPWMInitSinGen(void)
{
	ushort i;
	ulong SinGenTimerMaxCount = QueryClock(SMCLK) / SampleFrequency - 1;
	assert(SinGenTimerMaxCount <= 0xFFFF);

	FundamentalAmplitude = 0.5;
	for (i = 0; i < SinTableSize; i++)
	{
		CompensateTable[i] = 0;
	}
	SPWMReGenerateSinTable();

	TA0CTL = MC__STOP | ID__1 | TASSEL__SMCLK | TAIE;
	TA0CCTL0 = 0;
	TA0CCTL1 = 0;
	TA0CCTL2 = 0;
	TA0CCTL3 = 0;
	TA0CCTL4 = 0;
	TA0CCR0 = SinGenTimerMaxCount;
	TA0EX0 = TAIDEX_0;
	TA0CTL |= TACLR;
	Ticks = 0;
	TA0CTL |= MC__UP;
}

void SPWMGen_Init(void)
{
	SPWMInitPWMGen();
	SPWMInitSinGen();
}

void SPWMCompensateAmplitude(float CurAmplitude)
{
	const float TargetAmplitude = 194500.0;
	FundamentalAmplitude -= ((CurAmplitude - TargetAmplitude) / TargetAmplitude) * FundamentalAmplitude;
	if ((FundamentalAmplitude < 0.05) | isnanf(FundamentalAmplitude))
	{
		FundamentalAmplitude = 0.05;
	}
	if (FundamentalAmplitude > 1.2)
	{
		FundamentalAmplitude = 1.2;
	}
}

void SPWMCompensateHarmonic(ushort Order, float Amplitude, float PhaseOffset)//assume fundamental amplitude is 1
{
	ushort i;
	for (i = 0; i < SinTableSize; i++)
	{
		CompensateTable[i] += Amplitude * cosf(Order * 2 * M_PI * i / SinTableSize + PhaseOffset);
		if (CompensateTable[i] > 1)
		{
			CompensateTable[i] = 1;
		}
		if (CompensateTable[i] < -1)
		{
			CompensateTable[i] = -1;
		}
	}
}

void SPWMReGenerateSinTable(void)
{
	ushort i;
	for (i = 0; i < SinTableSize; i++)
	{
		float Val = (float)PWMMax * FundamentalAmplitude * (cosf(2 * M_PI * i / SinTableSize) - CompensateTable[i]) / 2 + PWMMax / 2;
		if (Val < 0)
		{
			Val = 0;
		}
		if (Val > PWMMax)
		{
			Val = PWMMax;
		}
		SinTable[i] = Val;
	}
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER0_A1_VECTOR
__interrupt void TIMERA0_ISR(void)
#elif defined(__GNUC__)
void __attribute__((interrupt(TIMER0_A1_VECTOR))) TIMERA0_ISR(void)
#else
#error Compiler not supported!
#endif
{
	SPWMSetOutputDuty(SinTable[Ticks]);
	Ticks++;
	if (Ticks == SinTableSize)
	{
		Ticks = 0;
	}
	TA0IV = 0;
}

ushort sqrt_bitwise(ulong x)
{
	ulong temp = 0;
	ushort v_bit = 15;
	ushort n = 0;
	ushort b = 0x8000;

	if (x <= 1)
	{
		return x;
	}

	do
	{
		temp = ((n << 1) + b) << (v_bit--);
		if (x >= temp)
		{
			n += b;
			x -= temp;
		}
	} while (b >>= 1);

	return n;
}

void SPWM_DoOutputPredistortion(ushort OutputVal[SPWMGenFFTSize])
{
	msp_cmplx_fft_q15_params fftParams;
	ushort i;
	uint16_t shift;
	int16_t Harmonics[MaxSensibleHarmonicOrder + 1][2];
	float HarmonicsAmp[MaxSensibleHarmonicOrder + 1];
	float HarmonicsAng[MaxSensibleHarmonicOrder + 1];
	for (i = 0; i < SPWMGenFFTSize; i++)
	{
		FFTBuffer[i * 2] = (int16_t)OutputVal[i] - 0x7FF;
		FFTBuffer[i * 2 + 1] = 0;
	}
	fftParams.length = SPWMGenFFTSize;
	fftParams.bitReverse = 1;
	fftParams.twiddleTable = msp_cmplx_twiddle_table_512_q15;
	msp_cmplx_fft_auto_q15(&fftParams, FFTBuffer, &shift);
	for (i = 0; i <= MaxSensibleHarmonicOrder; i++)
	{
		ushort Freq = 50 * i;
		ushort FreqPos = (ulong)Freq * SPWMGenFFTSize / SPWMGenFFTSampleFrequency;
		ushort FreqPosArr = FreqPos;
		while (FreqPosArr >= SPWMGenFFTSize)
		{
			FreqPosArr -= SPWMGenFFTSize;
		}
		Harmonics[i][0] = FFTBuffer[FreqPosArr * 2];
		Harmonics[i][1] = FFTBuffer[FreqPosArr * 2 + 1];
		HarmonicsAmp[i] = sqrtf((float)Harmonics[i][0] * Harmonics[i][0] + (float)Harmonics[i][1] * Harmonics[i][1]) * (1 << shift);
		HarmonicsAng[i] = atan2f((float)Harmonics[i][1], (float)Harmonics[i][0]);
	}
	SPWMCompensateAmplitude(HarmonicsAmp[1]);
	for (i = 3; i <= MaxSensibleHarmonicOrder; i++)
	{
		if (i % 2 != 0)
		{
			float Weigh = 1.0;
			if (i > 7)
			{
				Weigh = 7.0 / i;
			}
			SPWMCompensateHarmonic(i, Weigh * HarmonicsAmp[i] / HarmonicsAmp[1], HarmonicsAng[i] - HarmonicsAng[1] * i);
		}
	}
	SPWMReGenerateSinTable();
}
