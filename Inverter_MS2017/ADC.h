#ifndef _ADC_H_
#define _ADC_H_

typedef enum
{
	ADC_Ch0 = 0,
	ADC_Ch1,
	ADC_Ch2,
	ADC_Ch3,
	ADC_Ch4,
	ADC_Ch5,
	ADC_Ch6,
	ADC_Ch7,
	ADC_VeREFP,
	ADC_VeREFN,
	ADC_Temp,
	ADC_Power,
	ADC_Ch8,
	ADC_Ch9,
	ADC_Ch10,
	ADC_Ch11
}ADC_Channels;

typedef enum
{
	AVCC_AVSS = 0,
	VREFP_AVSS = 1,
	VeREFP_AVSS = 2,
	AVCC_VREFN = 4,
	VREFP_VREFN = 5,
	VeREFP_VeREFN = 6
}ADC_RefSel;

typedef struct
{
	ADC_Channels Source;
	ADC_RefSel Ref;
	uchar EOS;
}ADC_ChannelConfig;

typedef enum
{
	V15,
	V20,
	V25
}ADC_RefVolt;

typedef struct
{
	uint ClkDiv;
	uchar SlowMode;//clock divide by 4
	ADC_RefVolt RefVolt;
	uchar RefAlwaysOn;
	uchar RefOutput;
	uchar Repeated;
	ADC_ChannelConfig Channels[16];
}ADC_InitInfo;


void ADC_Init(ADC_InitInfo* InitInfo);
void ADC_Tigger(void);
uchar ADC_IsBusy(void);
ushort ADC_GetResult(uchar Index);

#endif