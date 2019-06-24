#include "Global.h"
#include "Clock.h"
#include "ADC.h"
#include <math.h>
#include "FlyBackPWMGen.h"
#include "SPWMGen.h"

ushort SampleCount = 0;
ushort OutputSampleBuffer[SPWMGenFFTSize];

void InitADCAndPorts(void)
{
	ADC_InitInfo ADCInit;
	ADCInit.ClkDiv = 8;
	ADCInit.SlowMode = 1;
	ADCInit.RefVolt = V25;
	ADCInit.RefAlwaysOn = 1;
	ADCInit.RefOutput = 0;
	ADCInit.Repeated = 1;
	ADCInit.Channels[0].Source = ADC_Ch0;
	ADCInit.Channels[0].Ref = VREFP_VREFN;
	ADCInit.Channels[0].EOS = 0;
	ADCInit.Channels[1].Source = ADC_Ch1;
	ADCInit.Channels[1].Ref = VREFP_VREFN;
	ADCInit.Channels[1].EOS = 1;
	ADC_Init(&ADCInit);
	P6DIR &= (1 << 0);//ADC channels
	P6DIR &= (1 << 1);
	P6SEL |= (1 << 0);
	P6SEL |= (1 << 1);
}

void Feedback_Init(void)
{
	ulong FeedbackTimerMaxCount = QueryClock(SMCLK) / 4 / SPWMGenFFTSampleFrequency - 1;
	assert(FeedbackTimerMaxCount <= 0xFFFF);
	InitADCAndPorts();
	TA1CTL = MC__STOP | ID__4 | TASSEL__SMCLK | TAIE;
	TA1CCTL0 = 0;
	TA1CCTL1 = 0;
	TA1CCTL2 = 0;
	TA1CCR0 = FeedbackTimerMaxCount;
	TA1EX0 = TAIDEX_0;
	TA1CTL |= TACLR;
	SampleCount = 0;
	TA1CTL |= MC__UP;
}

void Feedback_DoOutputPredistortion(void)
{
	static ushort DiscardCount = 3;//discard unstable samples
	if (SampleCount == SPWMGenFFTSize)
	{
		if (DiscardCount)
		{
			DiscardCount--;
		}
		else
		{
			SPWM_DoOutputPredistortion(OutputSampleBuffer);
		}
		SampleCount = 0;
	}
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER1_A1_VECTOR
__interrupt void TIMERA1_ISR(void)
#elif defined(__GNUC__)
void __attribute__((interrupt(TIMER1_A1_VECTOR))) TIMERA1_ISR(void)
#else
#error Compiler not supported!
#endif
{
	ushort FlybackFeedbackVal;
	ushort OutFeedbackVal;
	FlybackFeedbackVal = ADC_GetResult(0);
	OutFeedbackVal = ADC_GetResult(1);
	FlybackDoFeedback(FlybackFeedbackVal);
	if (SampleCount < SPWMGenFFTSize)
	{
		OutputSampleBuffer[SampleCount] = OutFeedbackVal;
		SampleCount++;
	}
	TA1IV = 0;
}
