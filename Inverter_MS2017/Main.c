#include "Global.h"
#include "Clock.h"
#include "SPWMGen.h"
#include "FlyBackPWMGen.h"
#include "Feedback.h"

void InitPortsForLowPower(void)
{
	P1DIR = 0;
	P1OUT = 0;
	P1REN = 0xFF;
	P2DIR = 0;
	P2OUT = 0;
	P2REN = 0xFF;
	P3DIR = 0;
	P3OUT = 0;
	P3REN = 0xFF;
	P4DIR = 0;
	P4OUT = 0;
	P4REN = 0xFF;
	P5DIR = 0;
	P5OUT = 0;
	P5REN = 0xFF;
	P6DIR = 0;
	P6OUT = 0;
	P6REN = 0xFF;
	P7DIR = 0;
	P7OUT = 0;
	P7REN = 0xFF;
	P8DIR = 0;
	P8OUT = 0;
	P8REN = 0xFF;
	PJDIR = 0;
	PJOUT = 0;
	PJREN = 0xFF;
}

void InitSysClock(void)
{
	Clock clk;
	clk.ACLKDivide = 4;
	clk.ACLKSource = DCO;
	clk.DCOExtRes = 0;
	clk.DCOFreq = 40000000;
	clk.MCLKDivide = 1;
	clk.MCLKSource = DCO;
	clk.SMCLKDivide = 1;
	clk.SMCLKSource = DCO;
	clk.XT1Freq = 32768;
	clk.XT2Freq = 0;
	ClockInit(&clk);
}

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;
	InitPortsForLowPower();
	P8OUT = 0x0;
	P8DIR = 0xFF;
	InitSysClock();
	FlybackPWMGen_Init();
	Feedback_Init();
	__enable_interrupt();

	while (!FlybackIsReady());

	P8OUT |= (1 << 0);

	SPWMGen_Init();

	for (;;)
	{
		P8OUT = FlybackIsReady();
		P8OUT &= ~(1 << 1);
		Feedback_DoOutputPredistortion();
		P8OUT |= (1 << 1);
	}
}
