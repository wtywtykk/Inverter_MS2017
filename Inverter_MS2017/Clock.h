#include "Global.h"
typedef enum
{
	DCO,
	XT1,
	XT2,
	REFO,
    VLO
}ClockSource;
typedef enum
{
	SMCLK,
	MCLK,
	ACLK
}Clocks;
typedef struct
{
	uint DCOExtRes;
	ulong DCOFreq;
	ulong XT1Freq;
	ulong XT2Freq;
	ClockSource MCLKSource;
	uint MCLKDivide;
	ClockSource SMCLKSource;
	uint SMCLKDivide;
	ClockSource ACLKSource;
	uint ACLKDivide;

}Clock;
void ClockInit(Clock* cctl);
ulong QueryClock(Clocks que);