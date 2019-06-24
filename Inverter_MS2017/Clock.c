#include "Global.h"
#include "Clock.h"
//#include <assert.h>
ulong fSMCLK, fMCLK, fACLK;
void SetVcoreUp(unsigned int level)
{
	// Open PMM registers for write
	PMMCTL0_H = PMMPW_H;
	// Set SVS/SVM high side new level
	SVSMHCTL = SVSHE + SVSHRVL0 * level + SVMHE + SVSMHRRL0 * level;
	// Set SVM low side to new level
	SVSMLCTL = SVSLE + SVMLE + SVSMLRRL0 * level;
	// Wait till SVM is settled
	while ((PMMIFG & SVSMLDLYIFG) == 0);
	// Clear already set flags
	PMMIFG &= ~(SVMLVLRIFG + SVMLIFG);
	// Set VCore to new level
	PMMCTL0_L = PMMCOREV0 * level;
	// Wait till new level reached
	if ((PMMIFG & SVMLIFG))
		while ((PMMIFG & SVMLVLRIFG) == 0);
	// Set SVS/SVM low side to new level
	SVSMLCTL = SVSLE + SVSLRVL0 * level + SVMLE + SVSMLRRL0 * level;
	// Lock PMM registers for write access
	PMMCTL0_H = 0x00;
}

void ClockInit(Clock* cctl)
{
	if (cctl->XT1Freq != 0)
	{
		P5SEL |= BIT4 | BIT5;
		UCSCTL6 |= XCAP_3;//12pF
		UCSCTL6 &= ~XT1OFF;
		while (SFRIFG1&OFIFG)
		{
			UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
			SFRIFG1 &= ~OFIFG;
		}
	}
	if (cctl->XT2Freq != 0)
	{
		P5SEL |= BIT2 | BIT3;
		UCSCTL6 &= ~XT2OFF;
		UCSCTL4 = UCSCTL4&(~(SELA_7)) | SELA_1;
		UCSCTL3 |= SELREF_2;
		while (SFRIFG1&OFIFG)
		{
			UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
			SFRIFG1 &= ~OFIFG;
		}
	}
	SetVcoreUp(1);
	SetVcoreUp(2);
	SetVcoreUp(3);
	if (cctl->DCOFreq != 0)
	{
		P5SEL |= BIT4 | BIT5;
		UCSCTL6 |= XCAP_3;
		UCSCTL6 &= ~XT1OFF;
		__bis_SR_register(SCG0);
		UCSCTL0 = 0;
		if (cctl->DCOFreq <= 630000)            //           DCOFreq < 0.63MHz
			UCSCTL1 = DCORSEL_0;
		else if (cctl->DCOFreq < 1250000)      // 0.63MHz < DCOFreq < 1.25MHz
			UCSCTL1 = DCORSEL_1;
		else if (cctl->DCOFreq < 2500000)      // 1.25MHz < DCOFreq <  2.5MHz
			UCSCTL1 = DCORSEL_2;
		else if (cctl->DCOFreq < 5000000)      // 2.5MHz  < DCOFreq <    5MHz
			UCSCTL1 = DCORSEL_3;
		else if (cctl->DCOFreq < 10000000)     // 5MHz    < DCOFreq <   10MHz
			UCSCTL1 = DCORSEL_4;
		else if (cctl->DCOFreq < 20000000)     // 10MHz   < DCOFreq <   20MHz
			UCSCTL1 = DCORSEL_5;
		else if (cctl->DCOFreq < 40000000)     // 20MHz   < DCOFreq <   40MHz
			UCSCTL1 = DCORSEL_6;
		else
			UCSCTL1 = DCORSEL_7;
		UCSCTL2 = FLLD_1 | ((cctl->DCOFreq / (cctl->XT1Freq)) / 2 - 1);
		__bic_SR_register(SCG0);
		//   __delay_cycles(782000);
		   //UCSCTL6 &= ~(XT2DRIVE0|XT2DRIVE1|XT2OFF);
		while (SFRIFG1 & OFIFG)                                // Check OFIFG fault flag
		{
			UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);         // Clear OSC flaut Flags
			SFRIFG1 &= ~OFIFG;                                  // Clear OFIFG fault flag
		}
	}
	switch (cctl->MCLKSource)
	{
	case REFO:
		UCSCTL4 = UCSCTL4&(~(SELM_7)) | SELM_2;
		fMCLK = 32768;
		break;
	case VLO:
		UCSCTL4 = UCSCTL4&(~(SELM_7)) | SELM_1;
		fMCLK = 10000;
		break;
	case XT1:
		UCSCTL4 = UCSCTL4&(~(SELM_7)) | SELM_0;
		fMCLK = cctl->XT1Freq;
		break;
	case XT2:
		UCSCTL4 = UCSCTL4&(~(SELM_7)) | SELM_5;
		fMCLK = cctl->XT2Freq;
		break;
	case DCO:
		UCSCTL4 = UCSCTL4&(~(SELM_7)) | SELM_3;
		fMCLK = cctl->DCOFreq;
		break;
	}
	switch (cctl->SMCLKSource)
	{
	case REFO:
		UCSCTL4 = UCSCTL4&(~(SELS_7)) | SELS_2;
		fSMCLK = 32768;
		break;
	case VLO:
		UCSCTL4 = UCSCTL4&(~(SELS_7)) | SELS_1;
		fSMCLK = 10000;
		break;
	case XT1:
		UCSCTL4 = UCSCTL4&(~(SELS_7)) | SELS_0;
		fSMCLK = cctl->XT1Freq;
		break;
	case XT2:
		UCSCTL4 = UCSCTL4&(~(SELS_7)) | SELS_5;
		fSMCLK = cctl->XT2Freq;
		break;
	case DCO:
		UCSCTL4 = UCSCTL4&(~(SELS_7)) | SELS_3;
		fSMCLK = cctl->DCOFreq;
		break;
	}
	switch (cctl->ACLKSource)
	{
	case REFO:
		UCSCTL4 = UCSCTL4&(~(SELA_7)) | SELA_2;
		fACLK = 32768;
		break;
	case VLO:
		UCSCTL4 = UCSCTL4&(~(SELA_7)) | SELA_1;
		fACLK = 10000;
		break;
	case XT1:
		UCSCTL4 = UCSCTL4&(~(SELA_7)) | SELA_0;
		fACLK = cctl->XT1Freq;
		break;
	case XT2:
		UCSCTL4 = UCSCTL4&(~(SELA_7)) | SELA_5;
		fACLK = cctl->XT2Freq;
		break;
	case DCO:
		UCSCTL4 = UCSCTL4&(~(SELA_7)) | SELA_3;
		fACLK = cctl->DCOFreq;
		break;
	}
	switch (cctl->ACLKDivide)
	{
	case 1:
		UCSCTL5 &= ~(DIVA0 | DIVA1 | DIVA2);
		break;
	case 2:
		UCSCTL5 = UCSCTL5&(~(DIVA0 | DIVA1 | DIVA2)) | DIVA0;
		fACLK = fACLK / 2;
		break;
	case 4:
		UCSCTL5 = UCSCTL5&(~(DIVA0 | DIVA1 | DIVA2)) | DIVA1;
		fACLK = fACLK / 4;
		break;
	case 8:
		UCSCTL5 = UCSCTL5&(~(DIVA0 | DIVA1 | DIVA2)) | DIVA0 | DIVA1;
		fACLK = fACLK / 8;
		break;
	case 16:
		UCSCTL5 = UCSCTL5&(~(DIVA0 | DIVA1 | DIVA2)) | DIVA2;
		fACLK = fACLK / 16;
		break;
	case 32:
		UCSCTL5 = UCSCTL5&(~(DIVA0 | DIVA1 | DIVA2)) | DIVA2 | DIVA0;
		fACLK = fACLK / 32;
		break;
	}
	switch (cctl->MCLKDivide)
	{
	case 1:
		UCSCTL5 &= ~(DIVM0 | DIVM1 | DIVM2);
		break;
	case 2:
		UCSCTL5 = UCSCTL5&(~(DIVM0 | DIVM1 | DIVM2)) | DIVM0;
		fMCLK = fMCLK / 2;
		break;
	case 4:
		UCSCTL5 = UCSCTL5&(~(DIVM0 | DIVM1 | DIVM2)) | DIVM1;
		fMCLK = fMCLK / 4;
		break;
	case 8:
		UCSCTL5 = UCSCTL5&(~(DIVM0 | DIVM1 | DIVM2)) | DIVM0 | DIVM1;
		fMCLK = fMCLK / 8;
		break;
	case 16:
		UCSCTL5 = UCSCTL5&(~(DIVM0 | DIVM1 | DIVM2)) | DIVM2;
		fMCLK = fMCLK / 16;
		break;
	case 32:
		UCSCTL5 = UCSCTL5&(~(DIVM0 | DIVM1 | DIVM2)) | DIVM2 | DIVM0;
		fMCLK = fMCLK / 32;
		break;
	}
	switch (cctl->SMCLKDivide)
	{
	case 1:
		UCSCTL5 &= ~(DIVS0 | DIVS1 | DIVS2);
		break;
	case 2:
		UCSCTL5 = UCSCTL5&(~(DIVS0 | DIVS1 | DIVS2)) | DIVS0;
		fSMCLK = fSMCLK / 2;
		break;
	case 4:
		UCSCTL5 = UCSCTL5&(~(DIVS0 | DIVS1 | DIVS2)) | DIVS1;
		fSMCLK = fSMCLK / 4;
		break;
	case 8:
		UCSCTL5 = UCSCTL5&(~(DIVS0 | DIVS1 | DIVS2)) | DIVS0 | DIVS1;
		fSMCLK = fSMCLK / 8;
		break;
	case 16:
		UCSCTL5 = UCSCTL5&(~(DIVS0 | DIVS1 | DIVS2)) | DIVS2;
		fSMCLK = fSMCLK / 16;
		break;
	case 32:
		UCSCTL5 = UCSCTL5&(~(DIVS0 | DIVS1 | DIVS2)) | DIVS2 | DIVS0;
		fSMCLK = fSMCLK / 32;
		break;
	}
}
ulong QueryClock(Clocks que)
{
	switch (que)
	{
	case SMCLK:return(fSMCLK); break;
	case MCLK:return(fMCLK); break;
	case ACLK:return(fACLK); break;
	default:return 0;
	}
}