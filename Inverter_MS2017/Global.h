#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <msp430.h>

typedef unsigned long ulong;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;
typedef uchar byte;

#define Stringlize1(Str) #Str
#define Stringlize(Str) Stringlize1(Str)
//Make a token into a string

#define MergeTokens1(Token1,Token2) Token1##Token2
#define MergeTokens(Token1,Token2) MergeTokens1(Token1,Token2)
//Combine Token1 and Token2 to create a new token
//Used to create function names with prefix.
//Example:void MergeTokens(Prefix,Func)(uint param) is void PrefixFunc(uint param)

#define assert(x) if((x)==0)while(1);

#endif
