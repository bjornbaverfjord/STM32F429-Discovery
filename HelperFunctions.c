#include "HelperFunctions.h"
#include "STM32-lib.h"
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <stdlib.h>

#define PI 3.1415926535897932384626433832795

/*
void printf1(const char * format, ... )
{
	char buff[256];
	unsigned int i=0;
	va_list args;
	
	va_start(args,format);
	vsnprintf(buff,256,format,args);
	
  //print the string
	do
	{
		WriteLPUART1(buff[i]);
		i+=1;
	}while(buff[i]!=0);
	
	va_end(args);
}
*/

void printf2(const char * format, ... )
{
	char buff[256];
	unsigned int i=0;
	va_list args;
	
	va_start(args,format);
	vsnprintf(buff,256,format,args);
	
  //print the string
	do
	{
		WriteUSART2(buff[i]);
		i+=1;
	}while(buff[i]!=0);
	
	va_end(args);
}

// Check if int is a power of two (contains a single 1)
int IsPow2 (int x) {
	return ((x != 0) && !(x & (x - 1)));
}

// Absolute value of an int
int iabs(int n) {
	if (n >= 0 ) return n;
	return 0 - n;
};

double fAbs(double n) {
	if (n >= 0 ) return n;
	return 0 - n;
};

double ToDeg(double angle)
{
	return angle * (180 / PI);
}

double ToRad(double angle)
{
	return angle * (PI / 180);
}

int imap(int value, int fromLow, int fromHigh, int toLow, int toHigh)
{
	double ratio;
	int range=toHigh-toLow;
	
	ratio=(double)(value-fromLow)/(double)(fromHigh-fromLow);
	
	return toLow+(int)((double)range*ratio);
}

double fmap(double value, double fromLow, double fromHigh, double toLow, double toHigh)
{
	double ratio;
	double range=toHigh-toLow;
	
	ratio=(value-fromLow)/(fromHigh-fromLow);
	
	return toLow+(range*ratio);
}

int iconstrain(int value, int min, int max)
{
	if(value<min){ value=min; }
	if(value>max){ value=max; }
	
	return value;
}

double fconstrain(double value, double min, double max)
{
	if(value<min){ value=min; }
	if(value>max){ value=max; }
	
	return value;
}

int imin(int value, int min)
{
	if(min<value){ value=min; }
	
	return value;
}

int imax(int value, int max)
{
	if(max>value){ value=max; }
	
	return value;
}

//compare 2 strings or strings in arrays
unsigned int cmpstr(char str1[], char str2[])
{
	unsigned int eq=1;
	unsigned int i=0;

	do
	{
		if(str1[i]!=str2[i]){ eq=0; break; }
		i++;
	}while(str1[i-1]!='\0');
	return eq;
}

// Fast pseudorandom integer
// Do not change the seed without extremely good reason
// The seed 0xfb000 is carefully selected to load in one ARM instruction, giving a period of 1 703 271
int rnd(void) {
	static unsigned int r = 0xfb000;
	return r = (((r >> 21) | (r << (32 - 21))) - r);
}

// random integer
int RandInt(int min, int max)
{
	return ((int)(rand() / (((double)RAND_MAX + 1)/ ((max-min)+1))))+min;
}

int average(int data[], unsigned length)
{
  int total=0;
  unsigned int i;
    
  if (length == 0) return 0;
  for(i=0;i<length;i++){ total += data[i]; }
  return total/length;
}

int median(int data[], unsigned int length)
{
	int median;

  if (length == 0) return 0;
	sort(data, length, 0);
	median=data[length>>1];
	if((length & 1)==0){ median+=data[(length>>1)-1]; median>>=1; }
	
  return median;
}


// Combsort11, uses no extra memory and is fast on almost all input data
void sort(int data[], unsigned int length, int dir) {
	unsigned int i, gap,f;
    
  if (length ==0) return;
	if (dir) dir = -1;

	gap = length;
	while(f || (gap > 1)) {
		gap = (gap * 10) / 13;
		if (gap == 0) gap = 1;
		if ((gap == 9) || (gap == 10)) gap = 11;

		f = 0;
		for (i = 0;i < (length - gap);i += 1) {
			if ((data[i] ^ dir) > ((data[i + gap] ^ dir))) {
				swap(&data[i],&data[i + gap]);
				f = 1;
			}
		}
	} 
}


unsigned int IsDigit(char chr)
{
	if((chr>='0') && (chr<='9')){ return 1; }else{ return 0; }
}

unsigned int IsNumeric(char numberstr[])
{
	unsigned int i=0, pass=1, dpcounter=0, sign=0, digitpass=0;

	do
	{
		if(i==0)
		{
			if((numberstr[0]!='+') && (numberstr[0]!='-') && (IsDigit(numberstr[0])!=1)){ pass=0; break; }
			if((numberstr[0]=='+') || (numberstr[0]=='-')){ sign=1; }else{ digitpass=1; }
		}else
		{
			if((sign==1) && (i==1))
			{
				if(IsDigit(numberstr[1])==1){ digitpass=1; }
			}
			if((IsDigit(numberstr[i])!=1) && (numberstr[i]!=',') && (numberstr[i]!='.')){ pass=0; break; }
			if(numberstr[i]=='.'){ dpcounter++; }
			if(dpcounter>1){ pass=0; break; }
			if((dpcounter>=1) && (numberstr[i]==',')){ pass=0; break; }
		}
		i++;
	}while(numberstr[i]!=0);
	if(digitpass==0){ pass=0; }	//if sign was present but no number, mark as invalid

	return pass;
}

unsigned int IsHexNibble(char chr)
{
	if(((chr>='0') && (chr<='9')) || ((chr>='a') && (chr<='f')) || ((chr>='A') && (chr<='F'))){ return 1; }else{ return 0; }
}

unsigned int IsHex(char numberstr[])
{
	unsigned int i=0, pass=1;

	do
	{
		if(i==0)
		{
			if((numberstr[0]!='+') && (numberstr[0]!='-') && (IsHexNibble(numberstr[0])!=1)){ pass=0; break; }
		}
		if((i==1) || (i==2))
		{
			if((numberstr[i]!='x') && (IsHexNibble(numberstr[i])!=1)){ pass=0; break; }
		}
		if(i>=3)
		{
			if(IsHexNibble(numberstr[i])==0){ pass=0; break; }
		}
		i++;
	}while(numberstr[i]!=0);

	return pass;
}

unsigned int IsBin(char numberstr[])
{
	unsigned int i=0, pass=1;

	do
	{
		if(i==0)
		{
			if((numberstr[0]!='+') && (numberstr[0]!='-') && (numberstr[0]!='1') && (numberstr[0]!='0')){ pass=0; break; }
		}
		if((i==1) || (i==2))
		{
			if((numberstr[i]!='x') && (numberstr[0]!='1') && (numberstr[0]!='0')){ pass=0; break; }
		}
		if(i>=3)
		{
			if((numberstr[0]!='1') && (numberstr[0]!='0')){ pass=0; break; }
		}
		i++;
	}while(numberstr[i]!=0);

	return pass;
}

int StringToInt(char string[])
{
	int sign = 1;
	int i = 0;
	int value = 0;

	while(isWhiteSpace(string[i])) i += 1;

	if (string[i] == '-') {
		sign = -1;
		i += 1; 
	}

	while(IsDigit(string[i]))	{
		value *= 10;
		value += (string[i] - '0');
		i += 1;
	}

	return value * sign;
}

unsigned short isWhiteSpace(char chr)
{
	if((chr==' ') | (chr=='\t') | (chr=='\n') | (chr=='\r') | (chr==0)){ return 1; }else{ return 0; }
}

double StringToDouble(char string[])
{
	char ipart[10]={0,0,0,0,0,0,0,0,0,0};
	char fpart[10]={0,0,0,0,0,0,0,0,0,0};
	unsigned int i=0;
	unsigned int ii=0;
	double multiplier=1;
	double value;

	while((string[i]!='.') && (string[i]!=0))
	{
		ipart[i]=string[i];
		i++;
	}
	
	if(string[i]=='.')
	{
		i++;
		while(string[i]!=0)
		{
			fpart[ii]=string[i];
			i++;
			ii++;
			multiplier *= 10;
		}
	}

	value = (double)(StringToInt(ipart));
	value += ((double)StringToInt(fpart))/multiplier;

	return value;
}

unsigned int StringLength(char string[])
{
	unsigned int i=0;
	
	while(string[i]!=0){ i++; }
	return i;
}

void IntToString(int val, char outputstr[])
{
	int unit;
	unsigned int digit;
	int digitval;
	unsigned int print;
	unsigned int arraypos=0;
	
	unit=1000000000;
	print=0;

	if(val<0){ outputstr[arraypos]='-'; val*=-1;	arraypos+=1; }
	for(digit=10;digit>=1;digit--)
	{
		digitval=val/unit;
		val -= digitval*unit;
		if((digitval>0) || (digit==1)){ print=1; }
		if(print==1){ outputstr[arraypos]=digitval+'0';	arraypos+=1; }
		unit /= 10;
	}
	outputstr[arraypos]=0;
}

void FloatToString(double value, unsigned int decimals, char outputstr[])
{
	double value2;
	int ipart;
	double fpart;
	unsigned int i;
	int decimal;
	unsigned int arraypos=0;

	int unit;
	int digit;
	int digitval;
	unsigned int print;
	
	if(value<0){ outputstr[arraypos]='-'; arraypos+=1; value*=-1; }
	value2=value;
	ipart=(int)value;

	unit=1000000000;
	print=0;

	for(digit=10;digit>=1;digit--)
	{
		digitval=value/unit;
		value -= digitval*unit;
		if((digitval>0) || (digit==1)){ print=1; }
		if(print==1){ outputstr[arraypos]=digitval+'0'; arraypos+=1; }
		unit /= 10;
	}

	outputstr[arraypos]='.'; arraypos+=1;

	fpart=value2-(float)ipart;
	for(i=0;i<decimals;i++)
	{
		fpart *= 10.0;
		decimal=(int)fpart;
		if(decimal>9){ decimal=9; }
		outputstr[arraypos]='0'+(unsigned char)decimal; arraypos+=1;
		fpart -= (float)decimal;
	}
	outputstr[arraypos]=0;
}

void CopyString(char source[], unsigned int sourcestart, char destination[], unsigned int destinationstart)
{
	do
	{
		destination[destinationstart]=source[sourcestart];
		sourcestart+=1;
		destinationstart+=1;
	}while(source[sourcestart-1]!=0);
}

int IsWithinString(char inputstr[], char matchstr[])
{
	unsigned int i=0, j=0, startpos=0;
	int returnval=-1;
	
	while(inputstr[i]!=0)
	{
		if(matchstr[j]==inputstr[i])
		{
			if(j==0){ startpos=i; }
			if(matchstr[j+1]==0){ returnval=startpos;	break; }
			j+=1;
		}else{ j=0; }
		i+=1;
	}
	
	return returnval;
}


void swap(int *x, int *y)
{
	int tmp = *x;
	*x = *y;
	*y = tmp;
}


int hi8(int n)
{
		return (n >> 8) & 0xff;
}

int lo8(int n)
{
		return n & 0xff;
}


// Signal fatal error
// TODO: Add rs-232 and other outputs
void FailWith(char string[],int e)
{
}

