/*
plug pinout at end with holes facing you: https://learn.adafruit.com/system/assets/assets/000/023/826/medium800/led_matrix_plug2.png?1426392350
____________________________
|GND	STB	D	B	GND	G1	GND	GO|
|/OE	CLK	C	A	B1	R1	B0	R0|
¯¯¯¯¯¯¯¯¯¯¯¯¯|_|¯¯¯¯¯¯¯¯¯¯¯¯

place board with silkscreen upright facing you, then flip board horizontally (left side to right side)
pixels shift in from right to left

top 16 rows (clock in 32 pixels)
R0	PC3
G0	PC8
B0	PC11

bottom 16 rows (clock in 32 pixels)
R1	PC12
G1	PC13
B1	PC14

4 bit selector selects the 2 active rows for the top and bottom (eg 0 selects rows 0 and 16). These are asynchronous and alters the row without a clock or latch
A		PE2
B		PE3
C		PE4
D		PE5

CLK	PD4		clock the data into the latches for both top and bottom sections
STB	PD5	latches the data into the leds for both top and bottom sections

output enable turns the currently latched LEDs on or off and is active low
/OE PE6 for both top and bottom sections

unlabled pins on this port connect to ground

free pins:
PA0 (if the user button is not used), PA5, PA9, PA10
PB4, PB7
PC15
PD2, PD7
PF6
PG2, PG3, PG13 (green led), PG14 (red led)
*/

#include "main.h"
#include "HelperFunctions.h"
#include "registers.h"
#include "STM32F4-lib.h"
#include "Nx32_RGB_LED.h"
#include <math.h>
#include <stdarg.h>

extern unsigned short ascii_7x11[95][14];
extern unsigned int asciivbw[559];
extern uint16_t ascii_16x24[];
extern uint16_t ascii_12x12[];
extern uint16_t ascii_8x12[];
extern uint16_t ascii_8x8[];
extern unsigned int hebrewfontvbw[205];

const unsigned int sevsegasciivbw[30]=
{
	31,17,31,	//0
	0,0,31,	  	//right aligned 1
	//0,31,0,	//centre aligned 1
	29,21,23, 	//2
	21,21,31, 	//3
	7,4,31,	  	//4
	23,21,29,	//5
	31,21,29, 	//6
	1,1,31,	  	//7
	31,21,31, 	//8
	23,21,31	//9
};

void InitRGBLEDMatrix(void)
{
	ConfigPinOnPort(R0Port, R0Pin, 'l');
	ConfigPinOnPort(G0Port, G0Pin, 'l');
	ConfigPinOnPort(B0Port, B0Pin, 'l');
	ConfigPinOnPort(R1Port, R1Pin, 'l');
	ConfigPinOnPort(G1Port, G1Pin, 'l');
	ConfigPinOnPort(B1Port, B1Pin, 'l');
	ConfigPinOnPort(RowSelectAPort, RowSelectAPin, 'l');
	ConfigPinOnPort(RowSelectBPort, RowSelectBPin, 'l');
	ConfigPinOnPort(RowSelectCPort, RowSelectCPin, 'l');
	ConfigPinOnPort(RowSelectDPort, RowSelectDPin, 'l');
	ConfigPinOnPort(CLKPort, CLKPin, 'l');
	ConfigPinOnPort(STBPort, STBPin, 'l');
	ConfigPinOnPort(OEPort, OEPin, 'l');

	StartTimer14(4500-1);	//10Khz
}

void RGBLEDMatrixPixelTest(unsigned int colour, unsigned int delayMS)	//top to bottom, right to left
{
	unsigned int x;
	unsigned int y;

	delayMS*=1000;
	
	//top half of screen
	ClearPin(R1Port, R1Pin);
	ClearPin(G1Port, G1Pin);
	ClearPin(B1Port, B1Pin);

	for(y=0;y<16;y++)
	{		
		for(x=0;x<LedMatrixWidth;x++)
		{
			if(x==0)	//put the data into the start of the shift register
			{
				if(colour & 1){ SetPin(B0Port, B0Pin); }else{ ClearPin(B0Port, B0Pin); }
				if(colour & 2){ SetPin(G0Port, G0Pin); }else{ ClearPin(G0Port, G0Pin); }
				if(colour & 4){ SetPin(R0Port, R0Pin); }else{ ClearPin(R0Port, R0Pin); }
			}else	//shift in 0 for the rest
			{
				ClearPin(B0Port, B0Pin);
				ClearPin(G0Port, G0Pin);
				ClearPin(R0Port, R0Pin);
			}
			__asm{ NOP }
			
			SetPin(CLKPort, CLKPin);	//clock high
			waitsys(1);
			ClearPin(CLKPort, CLKPin);	//clock low
			__asm{ NOP }
			
			//display every shift
			SetPin(STBPort, STBPin);	//latch high
			waitsys(1);
			ClearPin(STBPort, STBPin);	//latch low
			__asm{ NOP }
			
			//select the row
			SetPin(OEPort,OEPin);	//active low. turn off the leds while changing the active row	such that the current pixel(s) are not shifted down
			__asm{ NOP }
			if(y & 1){ SetPin(RowSelectAPort, RowSelectAPin); }else{ ClearPin(RowSelectAPort, RowSelectAPin); }
			if(y & 2){ SetPin(RowSelectBPort, RowSelectBPin); }else{ ClearPin(RowSelectBPort, RowSelectBPin); }
			if(y & 4){ SetPin(RowSelectCPort, RowSelectCPin); }else{ ClearPin(RowSelectCPort, RowSelectCPin); }
			if(y & 8){ SetPin(RowSelectDPort, RowSelectDPin); }else{ ClearPin(RowSelectDPort, RowSelectDPin); }
			__asm{ NOP }
			ClearPin(OEPort,OEPin);	//active low. leds are ready for display, turn on
			__asm{ NOP }
			
			waitsys(delayMS);
		}
	}
	
	//bottom half of screen
	ClearPin(R0Port, R0Pin);
	ClearPin(G0Port, G0Pin);
	ClearPin(B0Port, B0Pin);
	
	for(y=0;y<16;y++)
	{
		for(x=0;x<LedMatrixWidth;x++)
		{
			if(x==0)	//put the data into the start of the shift register
			{
				if(colour & 1){ SetPin(B1Port, B1Pin); }else{ ClearPin(B1Port, B1Pin); }
				if(colour & 2){ SetPin(G1Port, G1Pin); }else{ ClearPin(G1Port, G1Pin); }
				if(colour & 4){ SetPin(R1Port, R1Pin); }else{ ClearPin(R1Port, R1Pin); }
			}else	//shift in 0 for the rest
			{
				ClearPin(B1Port, B1Pin);
				ClearPin(G1Port, G1Pin);
				ClearPin(R1Port, R1Pin);
			}
			__asm{ NOP }
			__asm{ NOP }
				
			SetPin(CLKPort, CLKPin);	//clock high
			__asm{ NOP }
			ClearPin(CLKPort, CLKPin);	//clock low
			__asm{ NOP }
			
			//display every shift
			SetPin(STBPort, STBPin);	//latch high
			__asm{ NOP }
			ClearPin(STBPort, STBPin);	//latch low
			__asm{ NOP }
			
			//select the row
			SetPin(OEPort,OEPin);	//active low. turn off the leds while changing the active row	such that the current pixel(s) are not shifted down
			__asm{ NOP }
			if(y & 1){ SetPin(RowSelectAPort, RowSelectAPin); }else{ ClearPin(RowSelectAPort, RowSelectAPin); }
			if(y & 2){ SetPin(RowSelectBPort, RowSelectBPin); }else{ ClearPin(RowSelectBPort, RowSelectBPin); }
			if(y & 4){ SetPin(RowSelectCPort, RowSelectCPin); }else{ ClearPin(RowSelectCPort, RowSelectCPin); }
			if(y & 8){ SetPin(RowSelectDPort, RowSelectDPin); }else{ ClearPin(RowSelectDPort, RowSelectDPin); }
			__asm{ NOP }
			ClearPin(OEPort,OEPin);	//active low. leds are ready for display, turn on
			__asm{ NOP }
			
			waitsys(delayMS);
		}
	}
}

void RGBLEDMatrixLightUpPixel(unsigned int x, unsigned int y, unsigned int colour)
{
	unsigned int i;
	
	x=(LedMatrixWidth-1)-x;
	
	SetPin(OEPort,OEPin);	//active low. leds are ready for display, turn off
	__asm{ NOP }
	
	if(y & 1){ SetPin(RowSelectAPort, RowSelectAPin); }else{ ClearPin(RowSelectAPort, RowSelectAPin); }
	if(y & 2){ SetPin(RowSelectBPort, RowSelectBPin); }else{ ClearPin(RowSelectBPort, RowSelectBPin); }
	if(y & 4){ SetPin(RowSelectCPort, RowSelectCPin); }else{ ClearPin(RowSelectCPort, RowSelectCPin); }
	if(y & 8){ SetPin(RowSelectDPort, RowSelectDPin); }else{ ClearPin(RowSelectDPort, RowSelectDPin); }
	__asm{ NOP }
	
	if(y<16)
	{
		if(colour & 1){ SetPin(B0Port, B0Pin); }else{ ClearPin(B0Port, B0Pin); }
		if(colour & 2){ SetPin(G0Port, G0Pin); }else{ ClearPin(G0Port, G0Pin); }
		if(colour & 4){ SetPin(R0Port, R0Pin); }else{ ClearPin(R0Port, R0Pin); }
	}else
	{
		if(colour & 1){ SetPin(B1Port, B1Pin); }else{ ClearPin(B1Port, B1Pin); }
		if(colour & 2){ SetPin(G1Port, G1Pin); }else{ ClearPin(G1Port, G1Pin); }
		if(colour & 4){ SetPin(R1Port, R1Pin); }else{ ClearPin(R1Port, R1Pin); }
	}
	
	SetPin(CLKPort, CLKPin);	//clock high
	waitsys(1);
	ClearPin(CLKPort, CLKPin);	//clock low
	__asm{ NOP }
	
	if(y<16)
	{
		ClearPin(B0Port, B0Pin);
		ClearPin(G0Port, G0Pin);
		ClearPin(R0Port, R0Pin);
	}else
	{
		ClearPin(B1Port, B1Pin);
		ClearPin(G1Port, G1Pin);
		ClearPin(R1Port, R1Pin);
	}
	
	for(i=0;i<LedMatrixWidth;i++)
	{
		if(i==x)
		{
			SetPin(STBPort, STBPin);	//latch high
			waitsys(1);
			ClearPin(STBPort, STBPin);	//latch low
			__asm{ NOP }
			
			ClearPin(OEPort,OEPin);	//active low. leds are ready for display, turn on
			__asm{ NOP }
		}
		
		SetPin(CLKPort, CLKPin);	//clock high
		waitsys(1);
		ClearPin(CLKPort, CLKPin);	//clock low
		__asm{ NOP }
	}
}

void RGBLEDMatrixLightUpLine(unsigned int y, unsigned int colour)
{
	unsigned int i;
	
	SetPin(OEPort,OEPin);	//active low. leds are ready for display, turn off
	__asm{ NOP }
	
	if(y & 1){ SetPin(RowSelectAPort, RowSelectAPin); }else{ ClearPin(RowSelectAPort, RowSelectAPin); }
	if(y & 2){ SetPin(RowSelectBPort, RowSelectBPin); }else{ ClearPin(RowSelectBPort, RowSelectBPin); }
	if(y & 4){ SetPin(RowSelectCPort, RowSelectCPin); }else{ ClearPin(RowSelectCPort, RowSelectCPin); }
	if(y & 8){ SetPin(RowSelectDPort, RowSelectDPin); }else{ ClearPin(RowSelectDPort, RowSelectDPin); }
	__asm{ NOP }
	
	if(y<16)
	{
		ClearPin(B1Port, B1Pin);
		ClearPin(G1Port, G1Pin);
		ClearPin(R1Port, R1Pin);
		if(colour & 1){ SetPin(B0Port, B0Pin); }else{ ClearPin(B0Port, B0Pin); }
		if(colour & 2){ SetPin(G0Port, G0Pin); }else{ ClearPin(G0Port, G0Pin); }
		if(colour & 4){ SetPin(R0Port, R0Pin); }else{ ClearPin(R0Port, R0Pin); }

	}else
	{
		ClearPin(B0Port, B0Pin);
		ClearPin(G0Port, G0Pin);
		ClearPin(R0Port, R0Pin);
		if(colour & 1){ SetPin(B1Port, B1Pin); }else{ ClearPin(B1Port, B1Pin); }
		if(colour & 2){ SetPin(G1Port, G1Pin); }else{ ClearPin(G1Port, G1Pin); }
		if(colour & 4){ SetPin(R1Port, R1Pin); }else{ ClearPin(R1Port, R1Pin); }

	}
	__asm{ NOP }
	
	for(i=0;i<LedMatrixWidth;i++)
	{
		SetPin(CLKPort, CLKPin);	//clock high
		waitsys(1);
		ClearPin(CLKPort, CLKPin);	//clock low
		__asm{ NOP }
	}
	
	SetPin(STBPort, STBPin);	//latch high
	__asm{ NOP }
	ClearPin(STBPort, STBPin);	//latch low
	__asm{ NOP }
	
	ClearPin(OEPort,OEPin);	//active low. leds are ready for display, turn on
	__asm{ NOP }
}

void RGBLEDMatrixLightUpAreaSegmented(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int colour)
{
	unsigned int x;
	unsigned int y;
	
	SetPin(OEPort,OEPin);	//active low. leds are ready for display, turn off
	__asm{ NOP }
	
	//limit to top or bottom
	if((y1<16) && (y2>15)){ y2=15; }
	
	//clear the shift registers (top and bottom)
	for(x=0;x<LedMatrixWidth;x++)
	{
		SetPin(CLKPort, CLKPin);	//clock high
		waitsys(1);
		ClearPin(CLKPort, CLKPin);	//clock low
		__asm{ NOP }
	}
	
	if(y1<16)
	{
		if(colour & 1){ SetPin(B0Port, B0Pin); }else{ ClearPin(B0Port, B0Pin); }
		if(colour & 2){ SetPin(G0Port, G0Pin); }else{ ClearPin(G0Port, G0Pin); }
		if(colour & 4){ SetPin(R0Port, R0Pin); }else{ ClearPin(R0Port, R0Pin); }
	}else
	{
		if(colour & 1){ SetPin(B1Port, B1Pin); }else{ ClearPin(B1Port, B1Pin); }
		if(colour & 2){ SetPin(G1Port, G1Pin); }else{ ClearPin(G1Port, G1Pin); }
		if(colour & 4){ SetPin(R1Port, R1Pin); }else{ ClearPin(R1Port, R1Pin); }
	}
	
	//shift in data
	for(x=x1;x<=x2;x++)
	{
		SetPin(CLKPort, CLKPin);	//clock high
		waitsys(1);
		ClearPin(CLKPort, CLKPin);	//clock low
		__asm{ NOP }
	}
	
	if(y1<16)
	{
		ClearPin(B0Port, B0Pin);
		ClearPin(G0Port, G0Pin);
		ClearPin(R0Port, R0Pin);
	}else
	{
		ClearPin(B1Port, B1Pin);
		ClearPin(G1Port, G1Pin);
		ClearPin(R1Port, R1Pin);
	}
	
	//offset to xstart
		for(x=0;x<x1;x++)
		{
			SetPin(CLKPort, CLKPin);	//clock high
			waitsys(1);
			ClearPin(CLKPort, CLKPin);	//clock low
			__asm{ NOP }
		}
	
	//cycle the line over all y lines
	for(y=y1;y<=y2;y++)
	{
		if(y & 1){ SetPin(RowSelectAPort, RowSelectAPin); }else{ ClearPin(RowSelectAPort, RowSelectAPin); }
		if(y & 2){ SetPin(RowSelectBPort, RowSelectBPin); }else{ ClearPin(RowSelectBPort, RowSelectBPin); }
		if(y & 4){ SetPin(RowSelectCPort, RowSelectCPin); }else{ ClearPin(RowSelectCPort, RowSelectCPin); }
		if(y & 8){ SetPin(RowSelectDPort, RowSelectDPin); }else{ ClearPin(RowSelectDPort, RowSelectDPin); }
		__asm{ NOP }
		
		if(y==y1)
		{
			SetPin(STBPort, STBPin);	//latch high
			waitsys(1);
			ClearPin(STBPort, STBPin);	//latch low
			__asm{ NOP }
			
			ClearPin(OEPort,OEPin);	//active low. leds are ready for display, turn on
			__asm{ NOP }
		}
		
		waitsys(100);
	}
}

void RGBLEDMatrixLightUpArea(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int colour)
{
	unsigned int tmp;
	
	x1=(LedMatrixWidth-1)-x1;
	x2=(LedMatrixWidth-1)-x2;
	
	//may sure p2 coords are below p1 coords
	if(x1>x2){ tmp=x1; x1=x2; x2=tmp; }
	if(y1>y2){ tmp=y1; y1=y2; y2=tmp; }

	if((y1 & 0x10)==(y2 & 0x10))
	{
		RGBLEDMatrixLightUpAreaSegmented(x1, y1, x2, y2, colour);
	}else
	{
		RGBLEDMatrixLightUpAreaSegmented(x1, y1, x2, 15, colour);
		RGBLEDMatrixLightUpAreaSegmented(x1, 16, x2, y2, colour);
	}
}

void RGBLEDMatrixLightScreen(unsigned int colour)
{
	unsigned int i;
	
	SetPin(OEPort,OEPin);	//active low. leds are ready for display, turn off
	__asm{ NOP }
	
	if(colour & 1){ SetPin(B0Port, B0Pin); SetPin(B1Port, B1Pin); }else{ ClearPin(B0Port, B0Pin); ClearPin(B1Port, B1Pin); }
	if(colour & 2){ SetPin(G0Port, G0Pin); SetPin(G1Port, G1Pin); }else{ ClearPin(G0Port, G0Pin); ClearPin(G1Port, G1Pin); }
	if(colour & 4){ SetPin(R0Port, R0Pin); SetPin(R1Port, R1Pin); }else{ ClearPin(R0Port, R0Pin); ClearPin(R1Port, R1Pin); }
	__asm{ NOP }
	
	for(i=0;i<LedMatrixWidth;i++)
	{
		SetPin(CLKPort, CLKPin);	//clock high
		waitsys(1);
		ClearPin(CLKPort, CLKPin);	//clock low
		__asm{ NOP }
	}
	
	SetPin(STBPort, STBPin);	//latch high
	__asm{ NOP }
	ClearPin(STBPort, STBPin);	//latch low
	__asm{ NOP }
	
	for(i=0;i<16;i++)
	{
		if(i & 1){ SetPin(RowSelectAPort, RowSelectAPin); }else{ ClearPin(RowSelectAPort, RowSelectAPin); }
		if(i & 2){ SetPin(RowSelectBPort, RowSelectBPin); }else{ ClearPin(RowSelectBPort, RowSelectBPin); }
		if(i & 4){ SetPin(RowSelectCPort, RowSelectCPin); }else{ ClearPin(RowSelectCPort, RowSelectCPin); }
		if(i & 8){ SetPin(RowSelectDPort, RowSelectDPin); }else{ ClearPin(RowSelectDPort, RowSelectDPin); }
		__asm{ NOP }
		
		ClearPin(OEPort,OEPin);	//active low. leds are ready for display, turn on
		__asm{ NOP }
		
		waitsys(10);
	}
}

void RGBLEDMatrixClearScreen(void)
{
	unsigned int i;
	
	SetPin(OEPort,OEPin);	//active low. leds are ready for display, turn off
	__asm{ NOP }
	
	ClearPin(B0Port, B0Pin);
	ClearPin(G0Port, G0Pin);
	ClearPin(R0Port, R0Pin);
	ClearPin(B1Port, B1Pin);
	ClearPin(G1Port, G1Pin);
	ClearPin(R1Port, R1Pin);
	__asm{ NOP }
	__asm{ NOP }
	
	for(i=0;i<LedMatrixWidth;i++)
	{
		SetPin(CLKPort, CLKPin);	//clock high
		waitsys(1);
		ClearPin(CLKPort, CLKPin);	//clock low
		__asm{ NOP }
		waitsys(1);
	}
	
	SetPin(STBPort, STBPin);	//latch high
	waitsys(1);
	ClearPin(STBPort, STBPin);	//latch low
	__asm{ NOP }
	
	ClearPin(OEPort,OEPin);	//active low. leds are ready for display, turn on
	__asm{ NOP }
}

void RGBLEDMatrixDispBitmap1bit1px(unsigned int bitmap[][32])	//n*32 pixel at a time
{
	//00000000 00000000 00000000 00000000 00000RGB
	unsigned int i, x, y,Rpin,Gpin,Bpin;
	int Rport,Gport,Bport;

	for(y=0;y<32;y++)
	{
		if(y<16)
		{
			ClearPin(R1Port, R1Pin);
			ClearPin(G1Port, G1Pin);
			ClearPin(B1Port, B1Pin);
			Rport=R0Port;
			Gport=G0Port;
			Bport=B0Port;
			Rpin=R0Pin;
			Gpin=G0Pin;
			Bpin=B0Pin;
		}else
		{
			ClearPin(R0Port, R0Pin);
			ClearPin(G0Port, G0Pin);
			ClearPin(B0Port, B0Pin);
			Rport=R1Port;
			Gport=G1Port;
			Bport=B1Port;
			Rpin=R1Pin;
			Gpin=G1Pin;
			Bpin=B1Pin;
		}
		__asm{ NOP }
		
		for(x=0;x<LedMatrixWidth;x++)
		{
			for(i=0;i<LedMatrixWidth;i++)
			{
				if(i==x)
				{
					if(bitmap[x][y] & 1){ SetPin(Bport, Bpin); }else{ ClearPin(Bport, Bpin); }
					if(bitmap[x][y] & 2){ SetPin(Gport, Gpin); }else{ ClearPin(Gport, Gpin); }
					if(bitmap[x][y] & 4){ SetPin(Rport, Rpin); }else{ ClearPin(Rport, Rpin); }
				}else
				{
					ClearPin(Bport, Bpin);
					ClearPin(Gport, Gpin);
					ClearPin(Rport, Rpin);
				}
				waitsys(1);
				
				SetPin(CLKPort, CLKPin);	//clock high
				__asm{ NOP }
				ClearPin(CLKPort, CLKPin);	//clock low
				__asm{ NOP }
			}
			
			//select the row
			SetPin(OEPort,OEPin);	//active low. turn off the leds while changing the active row	such that the current pixel(s) are not shifted down
			__asm{ NOP }
			if(y & 1){ SetPin(RowSelectAPort, RowSelectAPin); }else{ ClearPin(RowSelectAPort, RowSelectAPin); }
			if(y & 2){ SetPin(RowSelectBPort, RowSelectBPin); }else{ ClearPin(RowSelectBPort, RowSelectBPin); }
			if(y & 4){ SetPin(RowSelectCPort, RowSelectCPin); }else{ ClearPin(RowSelectCPort, RowSelectCPin); }
			if(y & 8){ SetPin(RowSelectDPort, RowSelectDPin); }else{ ClearPin(RowSelectDPort, RowSelectDPin); }
			__asm{ NOP }
			SetPin(STBPort, STBPin);	//latch high
			__asm{ NOP }
			ClearPin(STBPort, STBPin);	//latch low
			__asm{ NOP }
			ClearPin(OEPort,OEPin);	//active low. leds are ready for display, turn on
			__asm{ NOP }
		}
	}
	waitsys(30);
	SetPin(OEPort,OEPin);	//make the last pixel not remain on for the length of all the above code
	__asm{ NOP }
}

void RGBLEDMatrixDispBitmap1bit1Line(unsigned int bitmap[][32])	//1 line at a time
{
	//00000000 00000000 00000000 00000000 00000RGB
	unsigned int x, y,Rpin,Gpin,Bpin;
	int Rport,Gport,Bport;

	for(y=0;y<32;y++)
	{
		if(y<16)
		{
			ClearPin(R1Port, R1Pin);
			ClearPin(G1Port, G1Pin);
			ClearPin(B1Port, B1Pin);
			Rport=R0Port;
			Gport=G0Port;
			Bport=B0Port;
			Rpin=R0Pin;
			Gpin=G0Pin;
			Bpin=B0Pin;
		}else
		{
			ClearPin(R0Port, R0Pin);
			ClearPin(G0Port, G0Pin);
			ClearPin(B0Port, B0Pin);
			Rport=R1Port;
			Gport=G1Port;
			Bport=B1Port;
			Rpin=R1Pin;
			Gpin=G1Pin;
			Bpin=B1Pin;
		}
		__asm{ NOP }
		
		for(x=0;x<LedMatrixWidth;x++)
		{
			if(bitmap[x][y] & 1){ SetPin(Bport, Bpin); }else{ ClearPin(Bport, Bpin); }
			if(bitmap[x][y] & 2){ SetPin(Gport, Gpin); }else{ ClearPin(Gport, Gpin); }
			if(bitmap[x][y] & 4){ SetPin(Rport, Rpin); }else{ ClearPin(Rport, Rpin); }
			waitsys(1);
				
			SetPin(CLKPort, CLKPin);	//clock high
			__asm{ NOP }
			ClearPin(CLKPort, CLKPin);	//clock low
			__asm{ NOP }
		}
		
		//select the row
		SetPin(OEPort,OEPin);	//active low. turn off the leds while changing the active row	such that the current pixel(s) are not shifted down
		__asm{ NOP }
		if(y & 1){ SetPin(RowSelectAPort, RowSelectAPin); }else{ ClearPin(RowSelectAPort, RowSelectAPin); }
		if(y & 2){ SetPin(RowSelectBPort, RowSelectBPin); }else{ ClearPin(RowSelectBPort, RowSelectBPin); }
		if(y & 4){ SetPin(RowSelectCPort, RowSelectCPin); }else{ ClearPin(RowSelectCPort, RowSelectCPin); }
		if(y & 8){ SetPin(RowSelectDPort, RowSelectDPin); }else{ ClearPin(RowSelectDPort, RowSelectDPin); }
		__asm{ NOP }
		SetPin(STBPort, STBPin);	//latch high
		__asm{ NOP }
		ClearPin(STBPort, STBPin);	//latch low
		__asm{ NOP }
		ClearPin(OEPort,OEPin);	//active low. leds are ready for display, turn on
		__asm{ NOP }
	
		waitsys(100);
	}
}

void RGBLEDMatrixDispBitmap1bit2Lines(unsigned int bitmap[][32])	//2 lines at a time
{
	//00000000 00000000 00000000 00000000 00000RGB
	unsigned int x, y, y2;

	for(y=0;y<16;y++)
	{
		y2=y+16;
		
		for(x=0;x<LedMatrixWidth;x++)
		{
			if(bitmap[x][y] & 1){ SetPin(B0Port, B0Pin); }else{ ClearPin(B0Port, B0Pin); }
			if(bitmap[x][y] & 2){ SetPin(G0Port, G0Pin); }else{ ClearPin(G0Port, G0Pin); }
			if(bitmap[x][y] & 4){ SetPin(R0Port, R0Pin); }else{ ClearPin(R0Port, R0Pin); }
			
			if(bitmap[x][y2] & 1){ SetPin(B1Port, B1Pin); }else{ ClearPin(B1Port, B1Pin); }
			if(bitmap[x][y2] & 2){ SetPin(G1Port, G1Pin); }else{ ClearPin(G1Port, G1Pin); }
			if(bitmap[x][y2] & 4){ SetPin(R1Port, R1Pin); }else{ ClearPin(R1Port, R1Pin); }
			__asm{ NOP }
				
			SetPin(CLKPort, CLKPin);	//clock high
			__asm{ NOP }
			ClearPin(CLKPort, CLKPin);	//clock low
			__asm{ NOP }
		}
		
		//select the row
		SetPin(OEPort,OEPin);	//active low. turn off the leds while changing the active row	such that the current pixel(s) are not shifted down
		__asm{ NOP }
		if(y & 1){ SetPin(RowSelectAPort, RowSelectAPin); }else{ ClearPin(RowSelectAPort, RowSelectAPin); }
		if(y & 2){ SetPin(RowSelectBPort, RowSelectBPin); }else{ ClearPin(RowSelectBPort, RowSelectBPin); }
		if(y & 4){ SetPin(RowSelectCPort, RowSelectCPin); }else{ ClearPin(RowSelectCPort, RowSelectCPin); }
		if(y & 8){ SetPin(RowSelectDPort, RowSelectDPin); }else{ ClearPin(RowSelectDPort, RowSelectDPin); }
		__asm{ NOP }
		SetPin(STBPort, STBPin);	//latch high
		__asm{ NOP }
		ClearPin(STBPort, STBPin);	//latch low
		__asm{ NOP }
		ClearPin(OEPort,OEPin);	//active low. leds are ready for display, turn on
		__asm{ NOP }
	
		waitsys(100);
	}
}


void RGBLEDMatrixDispBitmap4bit(unsigned int bitmap[][32])	//	//3 4-bit values 00000000 00000000 0000RRRR GGGG BBBB. 2 lines at a time with 4 bit pwm
{
	unsigned int x, y, y2, i;
	for(i=0;i<16;i++)	//display the frame 16 times to generate software PWM
	{
		if (i != 15 ) {
			for(y=0;y<16;y++)
			{
				y2=y+16;
				
				for(x=0;x<LedMatrixWidth;x++)
				{
					if(i < (bitmap[x][y] & 0xF)){ SetPin(B0Port, B0Pin); }else{ ClearPin(B0Port, B0Pin); }
					if(i < ((bitmap[x][y]>>4) & 0xF)){ SetPin(G0Port, G0Pin); }else{ ClearPin(G0Port, G0Pin); }
					if(i < ((bitmap[x][y]>>8) & 0xF)){ SetPin(R0Port, R0Pin); }else{ ClearPin(R0Port, R0Pin); }
					
					if(i < (bitmap[x][y2] & 0xF)){ SetPin(B1Port, B1Pin); }else{ ClearPin(B1Port, B1Pin); }
					if(i < ((bitmap[x][y2]>>4) & 0xF)){ SetPin(G1Port, G1Pin); }else{ ClearPin(G1Port, G1Pin); }
					if(i < ((bitmap[x][y2]>>8) & 0xF)){ SetPin(R1Port, R1Pin); }else{ ClearPin(R1Port, R1Pin); }
						
					SetPin(CLKPort, CLKPin);	//clock high
					__asm{ NOP }
					ClearPin(CLKPort, CLKPin);	//clock low
				}
				
				//select the row
				SetPin(OEPort,OEPin);	//active low. turn off the leds while changing the active row	such that the current pixel(s) are not shifted down
				__asm{ NOP }
				if(y & 1){ SetPin(RowSelectAPort, RowSelectAPin); }else{ ClearPin(RowSelectAPort, RowSelectAPin); }
				if(y & 2){ SetPin(RowSelectBPort, RowSelectBPin); }else{ ClearPin(RowSelectBPort, RowSelectBPin); }
				if(y & 4){ SetPin(RowSelectCPort, RowSelectCPin); }else{ ClearPin(RowSelectCPort, RowSelectCPin); }
				if(y & 8){ SetPin(RowSelectDPort, RowSelectDPin); }else{ ClearPin(RowSelectDPort, RowSelectDPin); }
				__asm{ NOP }
				SetPin(STBPort, STBPin);	//latch high
				__asm{ NOP }
				ClearPin(STBPort, STBPin);	//latch low
				__asm{ NOP }
				ClearPin(OEPort,OEPin);	//active low. leds are ready for display, turn on
			}
		} else SetPin(OEPort,OEPin);
	}
}


// 00000000 RRRRRRRR GGGGGGGG BBBBBBBB. 64 pixel at a time using 8 bit weighted binary
void RGBLEDMatrixDispBitmap8bit(unsigned int bitmap[][32])
{
    unsigned int x, y, y2, i,b;
	int d1,d2;
    
	for(i=0;i<8;i++)
	{
		  b = 1<<i;
			for(y=0;y<16;y++)
			{
				y2=y+16;
		
				for(x=0;x<LedMatrixWidth;x++)
				{
  				ClearPin(CLKPort, CLKPin);  // clock low
					__asm{ NOP }
	
					d1 = bitmap[x][y];
					if (b & d1)        { SetPin(B0Port, B0Pin); }else{ ClearPin(B0Port, B0Pin); }
					if (b & (d1>>8))   { SetPin(G0Port, G0Pin); }else{ ClearPin(G0Port, G0Pin); }
					if (b & (d1>>16))  { SetPin(R0Port, R0Pin); }else{ ClearPin(R0Port, R0Pin); }
					
   				d2 = bitmap[x][y2];
					if (b & d2)       { SetPin(B1Port, B1Pin); }else{ ClearPin(B1Port, B1Pin); }
					if (b & (d2>>8))  { SetPin(G1Port, G1Pin); }else{ ClearPin(G1Port, G1Pin); }
					if (b & (d2>>16)) { SetPin(R1Port, R1Pin); }else{ ClearPin(R1Port, R1Pin); }
					__asm{ NOP }
					__asm{ NOP }
						
					SetPin(CLKPort, CLKPin);    // clock high
					__asm{ NOP }
				}
				ClearPin(CLKPort, CLKPin);  // clock low
				__asm{ NOP }
				
				// Select the row
				if (y & 1){ SetPin(RowSelectAPort, RowSelectAPin); }else{ ClearPin(RowSelectAPort, RowSelectAPin); }
				if (y & 2){ SetPin(RowSelectBPort, RowSelectBPin); }else{ ClearPin(RowSelectBPort, RowSelectBPin); }
				if (y & 4){ SetPin(RowSelectCPort, RowSelectCPin); }else{ ClearPin(RowSelectCPort, RowSelectCPin); }
				if (y & 8){ SetPin(RowSelectDPort, RowSelectDPin); }else{ ClearPin(RowSelectDPort, RowSelectDPin); }
				__asm{ NOP }

				SetPin(STBPort, STBPin);   // latch high
				__asm{ NOP }
				ClearPin(STBPort, STBPin); // latch low
				__asm{ NOP }
				
				ClearPin(OEPort,OEPin);	// Enable output
				waitsys(b<<1);
				SetPin(OEPort,OEPin);
				__asm{ NOP }
			}
	}
}


void RGBLEDMatrixDisplayBitmap(unsigned int bitmap[][32], unsigned int mode)
{
	switch(mode)
	{
		case DisplayMode1bit1px:
			RGBLEDMatrixDispBitmap1bit1px(bitmap);
		break;
		case DisplayMode1bit1Line:
			RGBLEDMatrixDispBitmap1bit1Line(bitmap);
		break;
		case DisplayMode1bit2Lines:
			RGBLEDMatrixDispBitmap1bit2Lines(bitmap);
		break;
		case DisplayMode4bit:
			RGBLEDMatrixDispBitmap4bit(bitmap);
		break;
		case DisplayMode8bit:
			RGBLEDMatrixDispBitmap8bit(bitmap);
		break;
	}	
}

void RGBLEDMatrixDisplayBitmapDuration(unsigned int bitmap[][32], unsigned int durationMS, unsigned int mode)
{
	durationMS*=10;
	ResetTimer14();
	while(ReadTimer14()<durationMS)
	{
		RGBLEDMatrixDisplayBitmap(bitmap,mode);
	}	
}

void RGBLEDMatrixBitmapTest(unsigned int mode)	//run this in a loop
{
	unsigned int x,y,c,r,g,b,yy;
	unsigned int bitmap[LedMatrixWidth][32]={0};
	
	if(mode<=DisplayMode1bit2Lines)	//1 bit test pattern (checker board)
	{
		for(y=0;y<31;y+=4)
		{
			for(x=0;x<LedMatrixWidth-1;x+=4)
			{
				c=((x>>2)+(y>>2)) & 7;
				for(yy=0;yy<4;yy++)
				{
					bitmap[x][y+yy]=c;
					bitmap[x+1][y+yy]=c;
					bitmap[x+2][y+yy]=c;
					bitmap[x+3][y+yy]=c;
				}
			}
		}
	}else
	{
		for(x=0;x<LedMatrixWidth;x++)
		{
			if(mode==DisplayMode4bit)	//4 bit gradient bars
			{
				r=(x*15)/LedMatrixWidth;
				g=r<<4;
				b=r<<8;
			}else	//8 bit gradient bars
			{
				r=(x*255)/LedMatrixWidth;
				g=r<<8;
				b=r<<16;
			}
			
			//black
			bitmap[x][0]=0;
			bitmap[x][1]=0;
			bitmap[x][2]=0;
			bitmap[x][3]=0;
			
			//r
			bitmap[x][4]=r;
			bitmap[x][5]=r;
			bitmap[x][6]=r;
			bitmap[x][7]=r;
			
			//rg
			bitmap[x][8]=r | g;
			bitmap[x][9]=r | g;
			bitmap[x][10]=r | g;
			bitmap[x][11]=r | g;
			
			//g
			bitmap[x][12]=g;
			bitmap[x][13]=g;
			bitmap[x][14]=g;
			bitmap[x][15]=g;
			
			//gb
			bitmap[x][16]=g | b;
			bitmap[x][17]=g | b;
			bitmap[x][18]=g | b;
			bitmap[x][19]=g | b;
			
			//b
			bitmap[x][20]=b;
			bitmap[x][21]=b;
			bitmap[x][22]=b;
			bitmap[x][23]=b;
			
			//br
			bitmap[x][24]=b | r;
			bitmap[x][25]=b | r;
			bitmap[x][26]=b | r;
			bitmap[x][27]=b | r;
			
			bitmap[x][28]=r | g | b;
			bitmap[x][29]=r | g | b;
			bitmap[x][30]=r | g | b;
			bitmap[x][31]=r | g | b;
		}
	}
	
	RGBLEDMatrixDisplayBitmap(bitmap,mode);
}

void RGBLEDMatrixClear(unsigned int bitmap[][32], unsigned int colour)
{
	unsigned int x, y;

	for(y=0;y<32;y++)
	{
		for(x=0;x<LedMatrixWidth;x++)
		{
			bitmap[x][y]=colour;
		}
	}
}

void RGBLEDMatrixCopy(unsigned int sourcebitmap[][32], unsigned int destbitmap[][32])
{
	unsigned int x, y;

	for(y=0;y<32;y++)
	{
		for(x=0;x<LedMatrixWidth;x++)
		{
			destbitmap[x][y]=sourcebitmap[x][y];
		}
	}
}

void RGBLEDMatrixSetPixel(unsigned int bitmap[][32], int x, int y, unsigned int colour)	//1 or 4 bit format
{
	if((x>=0) && (x<=(LedMatrixWidth-1)) && (y>=0) && (y<=(LedMatrixHeight-1)))
	{
		bitmap[x][y]=colour;
	}
}

void RGBLEDMatrixMix(unsigned int sourcebitmap[][32], unsigned int destbitmap[][32])
{
	unsigned int x, y;

	for(y=0;y<32;y++)
	{
		for(x=0;x<LedMatrixWidth;x++)
		{
			destbitmap[x][y]=sourcebitmap[x][y] | destbitmap[x][y];
		}
	}
}

void RGBLEDMatrixMultiply(unsigned int sourcebitmap[][32], unsigned int destbitmap[][32])
{
	unsigned int x, y;

	for(y=0;y<32;y++)
	{
		for(x=0;x<LedMatrixWidth;x++)
		{
			destbitmap[x][y]=(sourcebitmap[x][y] * destbitmap[x][y])>>5;
		}
	}
}

unsigned int RGBLEDMatrixReadPixel(unsigned int bitmap[][32], int x, int y)
{
	if((x>=0) && (x<=(LedMatrixWidth-1)) && (y>=0) && (y<=31))
	{
		return bitmap[x][y];
	}else
	{
		return 0;
	}
}

void RGBLEDMatrixMixPixel(unsigned int bitmap[][32], int x, int y, unsigned int colour)
{
	RGBLEDMatrixSetPixel(bitmap,x,y,RGBLEDMatrixReadPixel(bitmap,x,y) | colour);
}

void RGBLEDMatrixReplaceColourWithColour(unsigned int bitmap[][32], unsigned int ColourToReplace, unsigned int ReplacementColour)
{
	int x, y;

	for(y=0;y<8;y++)
	{
		for(x=0;x<8;x++)
		{
			if(RGBLEDMatrixReadPixel(bitmap,x,y)==ReplacementColour){ RGBLEDMatrixSetPixel(bitmap,x,y,ColourToReplace); }
		}
	}
}

void RGBLEDMatrixReplaceColourWithBmp(unsigned int bitmap[][32], unsigned int ColourToReplace, unsigned int backgroundbitmap[][32])
{
	int x, y;

	for(y=0;y<32;y++)
	{
		for(x=0;x<LedMatrixWidth;x++)
		{
			if(bitmap[x][y]==ColourToReplace){ bitmap[x][y]=backgroundbitmap[x][y]; }
		}
	}
}

void RGBLEDMatrixReplaceColourWithBmpIntoBmp(unsigned int bitmap[][32], unsigned int ColourToReplace, unsigned int backgroundbitmap[][32], unsigned int destbitmap[][32])
{
	int x, y;

	for(y=0;y<32;y++)
	{
		for(x=0;x<LedMatrixWidth;x++)
		{
			if(bitmap[x][y]==ColourToReplace){ destbitmap[x][y]=backgroundbitmap[x][y]; }else{ destbitmap[x][y]=bitmap[x][y]; }
		}
	}
}

void RGBLEDMatrixFadeDown(unsigned int bitmap[][32], unsigned int delayMS)
{	
	int x, y, r, g, b;
	unsigned int changed;

	do
	{
		changed=0;
		for(y=0;y<32;y++)
		{
			for(x=0;x<LedMatrixWidth;x++)
			{
				b=bitmap[x][y] & 0xF;
				g=((bitmap[x][y])>>4) & 0xF;
				r=((bitmap[x][y])>>8) & 0xF;
				if(r!=0){ r-=1; changed=1; }
				if(g!=0){ g-=1; changed=1;  }
				if(b!=0){ b-=1; changed=1;  }
				
				bitmap[x][y]=(r<<8) | (g<<4) | b;
			}
		}
		RGBLEDMatrixDisplayBitmapDuration(bitmap, delayMS, DisplayMode8bit);
	}while(changed==1);
}

void RGBLEDMatrixFadeUp(unsigned int bitmap[][32], unsigned int delayMS)
{	
	int x, y, r1, g1, b1, r2, g2, b2;
	unsigned int changed;
	unsigned int bmp[LedMatrixWidth][32]={0};

	do
	{
		changed=0;
		for(y=0;y<32;y++)
		{
			for(x=0;x<LedMatrixWidth;x++)
			{
				b1=bitmap[x][y] & 0xF;
				g1=((bitmap[x][y])>>4) & 0xF;
				r1=((bitmap[x][y])>>8) & 0xF;
				
				b2=bmp[x][y] & 0xF;
				g2=((bmp[x][y])>>4) & 0xF;
				r2=((bmp[x][y])>>8) & 0xF;
				
				if(r2!=r1){ r2+=1; changed=1; }
				if(g2!=g1){ g2+=1; changed=1;  }
				if(b2!=b1){ b2+=1; changed=1;  }
				
				bmp[x][y]=(r2<<8) | (g2<<4) | b2;
			}
		}
		RGBLEDMatrixDisplayBitmapDuration(bmp, delayMS, DisplayMode8bit);
	}while(changed==1);
}

void RGBLEDMatrixFadeBetween(unsigned int bitmap1[][32], unsigned int bitmap2[][32])
{	
	int x, y, i, r1, g1, b1, r2, g2, b2;
	unsigned int bmp[LedMatrixWidth][32]={0};

	for(i=0;i<256;i+=2)
	{
		for(y=0;y<32;y++)
		{
			for(x=0;x<LedMatrixWidth;x++)
			{
				r1=bitmap1[x][y]>>16;
				g1=(bitmap1[x][y]>>8) & 0xFF;
				b1=bitmap1[x][y] & 0xFF;

				r2=bitmap2[x][y]>>16;
				g2=(bitmap2[x][y]>>8) & 0xFF;
				b2=bitmap2[x][y] & 0xFF;
				
				r1=r1+(((r2-r1)*i)/255);
				g1=g1+(((g2-g1)*i)/255);
				b1=b1+(((b2-b1)*i)/255);
				bmp[x][y]=(r1<<16) | (g1<<8)  | b1;
			}
		}
		RGBLEDMatrixDisplayBitmap(bmp, DisplayMode8bit);
	}
}

void RGBLEDMatrixLine(unsigned int bitmap[][32], int x1, int y1, int x2, int y2,  unsigned int colour)
{
	int x, y, xdelta, ydelta, width, height, i, count;
	
	if(y1==y2)
	{
		if((y1>=0) && (y1<=31))
		{
			if(x1>x2){ x=x2; x2=x1; x1=x; }
			for(x=x1;x<=x2;x++)
			{
				RGBLEDMatrixSetPixel(bitmap,x,y1,colour);
			}
		}
	  return;
	}

	if(x1==x2)
	{
		if((x1>=0) && (x1<=(LedMatrixWidth-1)))
		{
			if(y1>y2){ y=y2; y2=y1; y1=y; }
			for(y=y1;y<=y2;y++)
			{
				RGBLEDMatrixSetPixel(bitmap,x1,y,colour);
			}
		}
	  return;
	}
	
	width = x2 - x1;
	height = y2 - y1;
	
	count = max(iabs(width),iabs(height));

	xdelta = (width << 16) / count;
	ydelta = (height << 16) / count;

	x = x1 << 16;
	y = y1 << 16;

	for (i = 0;i <= count;i++)
	{
		RGBLEDMatrixSetPixel(bitmap,x>>16,y>>16,colour);
		x += xdelta;
		y += ydelta;
	}	
}

void RGBLEDMatrixRectangle(unsigned int bitmap[][32], int x1, int y1, int x2, int y2,  unsigned int colour)
{
	int tmp;
	int x;
	int y;
	
	if(x1>x2){ tmp=x2; x2=x1; x1=tmp; }
	if(y1>y2){ tmp=y2; y2=y1; y1=tmp; }

	for(x=x1;x<=x2;x++)
	{
		RGBLEDMatrixSetPixel(bitmap,x,y1,colour);
		RGBLEDMatrixSetPixel(bitmap,x,y2,colour);
	}
	for(y=y1;y<=y2;y++)
	{
		RGBLEDMatrixSetPixel(bitmap,x1,y,colour);
		RGBLEDMatrixSetPixel(bitmap,x2,y,colour);
	}
}

void RGBLEDMatrixRectangleFilled(unsigned int bitmap[][32], int x1, int y1, int x2, int y2,  unsigned int colour)
{
	int tmp;
	int x;
	int y;
	
	if(x1>x2){ tmp=x2; x2=x1; x1=tmp; }
	if(y1>y2){ tmp=y2; y2=y1; y1=tmp; }

	for(x=x1;x<=x2;x++)
	{
		for(y=y1;y<=y2;y++)
		{
			RGBLEDMatrixSetPixel(bitmap,x,y,colour);
		}
	}
}

void RGBLEDMatrixCircle(unsigned int bitmap[][32], int x0, int y0, int radius, unsigned int colour)
{
	int f = 1 - radius;
	int ddF_x = 1;
	int ddF_y = -2 * radius;
	int x = 0;
	int y = radius;
	
	RGBLEDMatrixSetPixel(bitmap,x0, y0 + radius,colour);
	RGBLEDMatrixSetPixel(bitmap,x0, y0 - radius,colour);
	RGBLEDMatrixSetPixel(bitmap,x0 + radius, y0,colour);
	RGBLEDMatrixSetPixel(bitmap,x0 - radius, y0,colour);
	
	while(x < y)
	{
		if(f >= 0) 
		{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;
		    
		RGBLEDMatrixSetPixel(bitmap,x0 + x, y0 + y,colour);	//segment 4
		RGBLEDMatrixSetPixel(bitmap,x0 - x, y0 + y,colour);	//segment 5
		RGBLEDMatrixSetPixel(bitmap,x0 + x, y0 - y,colour);	//segment 1
		RGBLEDMatrixSetPixel(bitmap,x0 - x, y0 - y,colour);	//segment 8
		RGBLEDMatrixSetPixel(bitmap,x0 + y, y0 + x,colour);	//segment 3
		RGBLEDMatrixSetPixel(bitmap,x0 - y, y0 + x,colour);	//segment 6
		RGBLEDMatrixSetPixel(bitmap,x0 + y, y0 - x,colour);	//segment 2
		RGBLEDMatrixSetPixel(bitmap,x0 - y, y0 - x,colour);	//segment 7
	}
}

void RGBLEDMatrixCircleFilled(unsigned int bitmap[][32], int x0, int y0, int radius, unsigned int colour)
{
	int f = 1 - radius;
	int ddF_x = 1;
	int ddF_y = -2 * radius;
	int x = 0;
	int y = radius;
	
	RGBLEDMatrixLine(bitmap, x0 - radius,y0,x0 + radius+1,y0,colour);
	
	while(x < y)
	{
		if(f >= 0) 
		{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;
		    
		RGBLEDMatrixLine(bitmap, x0 + x, y0 - y,x0 - x, y0 - y,colour);	//segment 1 to 8
		RGBLEDMatrixLine(bitmap, x0 + y, y0 - x,x0 - y, y0 - x,colour);	//segment 2 to 7
		RGBLEDMatrixLine(bitmap, x0 + y, y0 + x,x0 - y, y0 + x,colour);	//segment 3 to 6
		RGBLEDMatrixLine(bitmap, x0 + x, y0 + y,x0 - x, y0 + y,colour);	//segment 4 to 5
	}
}

void RGBLEDMatrixInvertColours(unsigned int bitmap[][32])
{
	int x, y;

	for(y=0;y<32;y++)
	{
		for(x=0;x<LedMatrixWidth;x++)
		{
			bitmap[x][y] ^= 0xFFF;
		}
	}
}

void RGBLEDMatrixShiftLeft(unsigned int bitmap[][32], int shift)
{
	int x;
	int y;

	for(y=0;y<32;y++)
	{
		for(x=0;x<LedMatrixWidth;x++)
		{
			RGBLEDMatrixSetPixel(bitmap,x,y,RGBLEDMatrixReadPixel(bitmap,x+shift,y));
		}
	}
}

void RGBLEDMatrixShiftRight(unsigned int bitmap[][32], int shift)
{
	int x;
	int y;

	for(y=0;y<32;y++)
	{
		for(x=(LedMatrixWidth-1);x>=0;x--)
		{
			RGBLEDMatrixSetPixel(bitmap,x,y,RGBLEDMatrixReadPixel(bitmap,x-shift,y));
		}
	}
}

void RGBLEDMatrixShiftUp(unsigned int bitmap[][32], int shift)
{
	int x;
	int y;

	for(x=0;x<LedMatrixWidth;x++)
	{
		for(y=0;y<32;y++)
		{
			RGBLEDMatrixSetPixel(bitmap,x,y,RGBLEDMatrixReadPixel(bitmap,x,y+shift));
		}
	}
}

void RGBLEDMatrixShiftDown(unsigned int bitmap[][32], int shift)
{
	int x;
	int y;

	for(x=0;x<LedMatrixWidth;x++)
	{
		for(y=31;y>=0;y--)
		{
			RGBLEDMatrixSetPixel(bitmap,x,y,RGBLEDMatrixReadPixel(bitmap,x,y-shift));
		}
	}
}

void BmpRotate(unsigned int sourcebitmap[][32], unsigned int destbitmap[][32], double angle, double midx, double midy)
{
  double x;
	double y;
	double xold;
	double yold;
	double xnew;
	double ynew;
	double s,c,yoldS,yoldC;
	
	s=sin((angle * 3.14159) / 180.0);
	c=cos((angle * 3.14159) / 180.0);

	for(y=0;y<32;y++)
	{
	  yold = y - midy;
		yoldS=yold * s;
		yoldC=yold * c;
		for(x=0;x<LedMatrixWidth;x++)
		{
			xold = x - midx;
			xnew = (xold * c) - yoldS;
			ynew = (xold * s) + yoldC;
			RGBLEDMatrixSetPixel(destbitmap,rint(xnew + midx),rint(ynew + midy),RGBLEDMatrixReadPixel(sourcebitmap,x,y));
	  }
	}
}

unsigned int RGBLEDMatrixCharacter(unsigned int bitmap[][32], char character, int x1, int y1, unsigned int font, unsigned int TextColour, int BackColour)
{
	int x;
	int y;
	int bytenum;
	int nextx;
	int prelineX;
	
	//background before the character
	prelineX=x1-1;
	if((BackColour>-1) && (prelineX>-1)){ RGBLEDMatrixLine(bitmap,x1-1,y1,x1-1,y1+RGBLEDMatrixGetFontHeight(font)-1,BackColour); }

	switch(font)
	{
		case LEDFontVariableWidth:
			x=0;
			for(bytenum=asciivbw[character-' '];bytenum<(asciivbw[character-' ']+asciivbw[95+(character-' ')]);bytenum++)
			{
				for(y=0;y<8;y++)	//add byte
				{
					if(((asciivbw[bytenum]>>y) & 1)==1)
					{
						RGBLEDMatrixSetPixel(bitmap,x+x1,y+y1,TextColour);
					}else
					{
						if(BackColour>-1){ RGBLEDMatrixSetPixel(bitmap,x+x1,y+y1,BackColour); }
					}
				}
				x+=1;
			}
			break;
			
		case LEDFont7x11:
			for(x=0;x<7;x++)
			{
				for(y=5;y<16;y++)
				{
					if((ascii_7x11[character-0x20][x]>>y) & 1)
					{
						RGBLEDMatrixSetPixel(bitmap,x+x1,y+y1-5,TextColour);
					}else
					{
						if(BackColour>-1){ RGBLEDMatrixSetPixel(bitmap,x+x1,y+y1-5,BackColour); }
					}
				}
			}
			break;
			
		case LEDFont8x8:
			for(y=0;y<8;y++)
			{
				for(x=0;x<8;x++)
				{
					if(ascii_8x8[y+((character-' ')*8)] & (1<<(7-x)))
					{
						RGBLEDMatrixSetPixel(bitmap,x+x1,y+y1,TextColour);
					}else
					{
						if(BackColour>-1){ RGBLEDMatrixSetPixel(bitmap,x+x1,y+y1,BackColour); }
					}
				}
			}
			break;
			
		case LEDFont8x12:
			for(y=0;y<12;y++)
			{
				for(x=0;x<8;x++)
				{
					if(ascii_8x12[y+((character-' ')*12)] & (1<<(7-x)))
					{
						RGBLEDMatrixSetPixel(bitmap,x+x1,y+y1,TextColour);
					}else
					{
						if(BackColour>-1){ RGBLEDMatrixSetPixel(bitmap,x+x1,y+y1,BackColour); }
					}
				}
			}
			break;
			
		case LEDFont12x12:
			for(y=0;y<12;y++)
			{
				for(x=0;x<12;x++)
				{
					if(ascii_12x12[y+((character-' ')*12)] & (1<<(15-x)))
					{
						RGBLEDMatrixSetPixel(bitmap,x+x1,y+y1,TextColour);
					}else
					{
						if(BackColour>-1){ RGBLEDMatrixSetPixel(bitmap,x+x1,y+y1,BackColour); }
					}
				}
			}
			break;
			
		case LEDFont16x24:
			for(y=0;y<24;y++)
			{
				for(x=0;x<16;x++)
				{
					if(ascii_16x24[y+((character-' ')*24)] & (1<<x))
					{
						RGBLEDMatrixSetPixel(bitmap,x+x1,y+y1,TextColour);
					}else
					{
						if(BackColour>-1){ RGBLEDMatrixSetPixel(bitmap,x+x1,y+y1,BackColour); }
					}
				}
			}
			break;

		case LEDFontHebrew:
			x=0;
			for(bytenum=hebrewfontvbw[character-1]+hebrewfontvbw[28+(character-1)]-1;bytenum>=hebrewfontvbw[character-1];bytenum--)
			{
				for(y=0;y<16;y++)	//add byte
				{
					if(((hebrewfontvbw[bytenum]>>y) & 1)==1)
					{
						RGBLEDMatrixSetPixel(bitmap,x1-x,y+y1,TextColour);
					}else
					{
						if(BackColour>-1){ RGBLEDMatrixSetPixel(bitmap,x1-x,y+y1,BackColour); }
					}
				}
				x+=1;
			}
			break;
	}
	
	
	if(font!=LEDFontHebrew)
	{
		nextx=x1+x+1;
		if(BackColour>-1){ RGBLEDMatrixLine(bitmap,x1+x,y1,x1+x,y1+RGBLEDMatrixGetFontHeight(font)-1,BackColour); }	//Colour inter letter gap
	}else
	{
		nextx=x1-x-1;
		if(BackColour>-1){ RGBLEDMatrixLine(bitmap,x1-x,y1,x1-x,y1+RGBLEDMatrixGetFontHeight(font)-1,BackColour); }	//Colour inter letter gap
	}
	
	return nextx;
}

unsigned int RGBLEDMatrixGetFontHeight(unsigned int font)
{
	switch(font)
	{
		case LEDFontVariableWidth:
			return 8;
		case LEDFont7x11:
			return 11;
		case LEDFont8x8:
			return 8;
		case LEDFont8x12:
			return 12;
		case LEDFont12x12:
			return 12;
		case LEDFont16x24:
			return 24;
		case LEDFontHebrew:
			return 16;
	}
	
	return 0;
}

unsigned int RGBLEDMatrixGetFontWidth(unsigned int font)
{
	switch(font)
	{
		case LEDFontVariableWidth:
			return 9;	//widest letter
		case LEDFont7x11:
			return 7;
		case LEDFont8x8:
			return 8;
		case LEDFont8x12:
			return 8;
		case LEDFont12x12:
			return 12;
		case LEDFont16x24:
			return 16;
		case LEDFontHebrew:
			return 8;	//widest letter
	}

	return 0;
}

void RGBLEDMatrixShiftInCharacter(unsigned int bitmap[][32], char character, int x1, int y1, unsigned int fps, unsigned int TextColour, int BackColour, unsigned int font, unsigned int DisplayMode, unsigned int flipAxis)
{
	int x;
	int y;
	int bytenum;
	unsigned int mS;
	
	mS=1000/fps;

	switch(font)
	{
		case LEDFontVariableWidth:
			for(bytenum=asciivbw[character-' '];bytenum<(asciivbw[character-' ']+asciivbw[95+(character-' ')]);bytenum++)
			{
				for(y=0;y<8;y++)	//add byte
				{
					if(((asciivbw[bytenum]>>y) & 1)==1)
					{
						RGBLEDMatrixSetPixel(bitmap,x1,y+y1,TextColour);
					}else
					{
						if(BackColour>-1){ RGBLEDMatrixSetPixel(bitmap,x1,y+y1,BackColour); }
					}
				}
				
				if(flipAxis!=LEDNoFlip){ RGBLEDMatrixFlip(bitmap,flipAxis); }	//flip
				RGBLEDMatrixDisplayBitmapDuration(bitmap, mS, DisplayMode);		//display frame
				if(flipAxis!=LEDNoFlip){ RGBLEDMatrixFlip(bitmap,flipAxis); }	//unflip
				RGBLEDMatrixShiftLeft(bitmap,1);
			}
			break;
			
		case LEDFont7x11:
			for(x=0;x<7;x++)
			{
				for(y=5;y<16;y++)
				{
					if((ascii_7x11[character-0x20][x]>>y) & 1)
					{
						RGBLEDMatrixSetPixel(bitmap,x1,y+y1-5,TextColour);
					}else
					{
						if(BackColour>-1){ RGBLEDMatrixSetPixel(bitmap,x1,y+y1-5,BackColour); }
					}
				}
				
				if(flipAxis!=LEDNoFlip){ RGBLEDMatrixFlip(bitmap,flipAxis); }	//flip
				RGBLEDMatrixDisplayBitmapDuration(bitmap, mS, DisplayMode);		//display frame
				if(flipAxis!=LEDNoFlip){ RGBLEDMatrixFlip(bitmap,flipAxis); }	//unflip
				RGBLEDMatrixShiftLeft(bitmap,1);
			}
			break;
		
		case LEDFont8x8:
			for(x=0;x<8;x++)
			{
				for(y=0;y<8;y++)
				{
					if(ascii_8x8[y+((character-' ')*8)] & (1<<(7-x)))
					{
						RGBLEDMatrixSetPixel(bitmap,x1,y+y1,TextColour);
					}else
					{
						if(BackColour>-1){ RGBLEDMatrixSetPixel(bitmap,x1,y+y1,BackColour); }
					}
				}
				
				if(flipAxis!=LEDNoFlip){ RGBLEDMatrixFlip(bitmap,flipAxis); }	//flip
				RGBLEDMatrixDisplayBitmapDuration(bitmap, mS, DisplayMode);		//display frame
				if(flipAxis!=LEDNoFlip){ RGBLEDMatrixFlip(bitmap,flipAxis); }	//unflip
				RGBLEDMatrixShiftLeft(bitmap,1);
			}
			break;
			
		case LEDFont8x12:
			for(x=0;x<8;x++)
			{
				for(y=0;y<12;y++)
				{
					if(ascii_8x12[y+((character-' ')*12)] & (1<<(7-x)))
					{
						RGBLEDMatrixSetPixel(bitmap,x1,y+y1,TextColour);
					}else
					{
						if(BackColour>-1){ RGBLEDMatrixSetPixel(bitmap,x1,y+y1,BackColour); }
					}
				}
				
				if(flipAxis!=LEDNoFlip){ RGBLEDMatrixFlip(bitmap,flipAxis); }	//flip
				RGBLEDMatrixDisplayBitmapDuration(bitmap, mS, DisplayMode);		//display frame
				if(flipAxis!=LEDNoFlip){ RGBLEDMatrixFlip(bitmap,flipAxis); }	//unflip
				RGBLEDMatrixShiftLeft(bitmap,1);
			}
			break;
			
		case LEDFont12x12:
			for(x=0;x<12;x++)
			{
				for(y=0;y<12;y++)
				{
					if(ascii_12x12[y+((character-' ')*12)] & (1<<(15-x)))
					{
						RGBLEDMatrixSetPixel(bitmap,x1,y+y1,TextColour);
					}else
					{
						if(BackColour>-1){ RGBLEDMatrixSetPixel(bitmap,x1,y+y1,BackColour); }
					}
				}
				
				if(flipAxis!=LEDNoFlip){ RGBLEDMatrixFlip(bitmap,flipAxis); }	//flip
				RGBLEDMatrixDisplayBitmapDuration(bitmap, mS, DisplayMode);		//display frame
				if(flipAxis!=LEDNoFlip){ RGBLEDMatrixFlip(bitmap,flipAxis); }	//unflip
				RGBLEDMatrixShiftLeft(bitmap,1);
			}
			break;
			
		case LEDFont16x24:
			for(x=0;x<16;x++)
			{
				for(y=0;y<24;y++)
				{
					if(ascii_16x24[y+((character-' ')*24)] & (1<<x))
					{
						RGBLEDMatrixSetPixel(bitmap,x1,y+y1,TextColour);
					}else
					{
						if(BackColour>-1){ RGBLEDMatrixSetPixel(bitmap,x1,y+y1,BackColour); }
					}
				}

				if(flipAxis!=LEDNoFlip){ RGBLEDMatrixFlip(bitmap,flipAxis); }	//flip
				RGBLEDMatrixDisplayBitmapDuration(bitmap, mS, DisplayMode);		//display frame
				if(flipAxis!=LEDNoFlip){ RGBLEDMatrixFlip(bitmap,flipAxis); }	//unflip
				RGBLEDMatrixShiftLeft(bitmap,1);
			}
			break;
			
		case LEDFontHebrew:
			for(bytenum=hebrewfontvbw[character-1]+hebrewfontvbw[28+(character-1)]-1;bytenum>=hebrewfontvbw[character-1];bytenum--)
			{
				for(y=0;y<16;y++)	//add word
				{
					if(((hebrewfontvbw[bytenum]>>y) & 1)==1)
					{
						RGBLEDMatrixSetPixel(bitmap,x1,y+y1,TextColour);
					}else
					{
						if(BackColour>-1){ RGBLEDMatrixSetPixel(bitmap,x1,y+y1,BackColour); }
					}
				}

				if(flipAxis!=LEDNoFlip){ RGBLEDMatrixFlip(bitmap,flipAxis); }	//flip
				RGBLEDMatrixDisplayBitmapDuration(bitmap, mS, DisplayMode);		//display frame
				if(flipAxis!=LEDNoFlip){ RGBLEDMatrixFlip(bitmap,flipAxis); }	//unflip
				RGBLEDMatrixShiftRight(bitmap,1);
			}
			break;
	}
	
	if(BackColour>-1){ RGBLEDMatrixLine(bitmap,x1,y1,x1,y1+RGBLEDMatrixGetFontHeight(font)-1,BackColour); }	//Colour inter letter gap

	if(flipAxis!=LEDNoFlip){ RGBLEDMatrixFlip(bitmap,flipAxis); }	//flip
	RGBLEDMatrixDisplayBitmapDuration(bitmap, mS, DisplayMode);		//display frame
	if(flipAxis!=LEDNoFlip){ RGBLEDMatrixFlip(bitmap,flipAxis); }	//unflip	
	
	if(font!=LEDFontHebrew)	//extra 1px space after the character
	{
		RGBLEDMatrixShiftLeft(bitmap,1);
	}else
	{
		RGBLEDMatrixShiftRight(bitmap,1);
	}


}

void printfRGBLEDMatrixScrollText(unsigned int bitmap[][32], int x1, int y1, unsigned int fps, unsigned int TextColour, int BackColour, unsigned int font, unsigned int DisplayMode, unsigned int flipAxis, const char * format, ...)
{
	char characters[256];
	unsigned int i=0;
	va_list args;
	
	va_start(args,format);
	vsnprintf(characters,256,format,args);

	while(characters[i]!=0)
	{
		RGBLEDMatrixShiftInCharacter(bitmap, characters[i], x1, y1, fps, TextColour, BackColour, font, DisplayMode, flipAxis);
		i++;
	}
}

unsigned int RGBLEDMatrixGetStringWidth(char string[], unsigned int font)
{
	int x=0;
	int i=0;

	while(string[i]!=0)
	{
		x+=RGBLEDMatrixGetCharacterWidth(string[i], font)+1;
		i+=1;
	}
	
	return x-1;
}

unsigned int RGBLEDMatrixGetCharacterWidth(char chr, unsigned int font)
{
	unsigned int w=0;
	
	switch(font)
	{
		case LEDFontVariableWidth:
			w=asciivbw[95+(chr-' ')];
			break;
			
		case LEDFont7x11:
			w=7;
			break;
			
		case LEDFont8x8:
			w=8;
			break;
			
		case LEDFont8x12:
			w=8;
			break;
			
		case LEDFont12x12:
			w=12;
			break;
			
		case LEDFont16x24:
			w=16;
			break;
			
		case LEDFontHebrew:
			w=hebrewfontvbw[27+(chr-1)];
			break;
	}
	
	return w;
}

void RGBLEDMatrixSevSegDigit(unsigned int bitmap[][32], unsigned int val, int xLeft, int yTop, unsigned int colour)
{
	int x;
	int y;
	unsigned int segments;
	unsigned int digits[10]={0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
	
	segments=digits[val];
	
	if(segments & 1)		//seg A
	{
		for(x=xLeft;x<=(xLeft+2);x++){ RGBLEDMatrixSetPixel(bitmap,x,yTop,colour); }
	}

	if(segments & 2)		//seg B
	{
		for(y=yTop;y<=(yTop+2);y++){ RGBLEDMatrixSetPixel(bitmap,xLeft+2,y,colour); }
	}

	if(segments & 4)		//seg C
	{
		for(y=yTop+2;y<=(yTop+4);y++){ RGBLEDMatrixSetPixel(bitmap,xLeft+2,y,colour); }
	}

	if(segments & 8)		//seg D
	{
		for(x=xLeft;x<=(xLeft+2);x++){ RGBLEDMatrixSetPixel(bitmap,x,yTop+4,colour); }
	}

	if(segments & 16)		//seg E
	{
		for(y=yTop+2;y<=(yTop+4);y++){ RGBLEDMatrixSetPixel(bitmap,xLeft,y,colour); }
	}

	if(segments & 32)		//seg F
	{
		for(y=yTop;y<=(yTop+2);y++){ RGBLEDMatrixSetPixel(bitmap,xLeft,y,colour); }
	}

	if(segments & 64)		//seg G
	{
		for(x=xLeft;x<=(xLeft+2);x++){ RGBLEDMatrixSetPixel(bitmap,x,yTop+2,colour); }
	}
}

unsigned int RGBLEDMatrixSevSegSegMultiDigits(unsigned int bitmap[][32], unsigned int val, int xLeft, int yTop, unsigned int ForceDigits, unsigned int colour)
{
	unsigned int unit;
	unsigned int digit;
	unsigned int digitval;
	unsigned int print;
	unsigned int x1=0;

	unit=1000000000;
	print=0;
	for(digit=10;digit>=1;digit--)
	{
		digitval=val/unit;
		val -= digitval*unit;
		if((digitval>0) || (digit==1) || (digit<=ForceDigits)){ print=1; }
		if(print==1){ RGBLEDMatrixSevSegDigit(bitmap, digitval, xLeft+x1, yTop, colour); x1+=4; }
		unit /= 10;
	}
	
	return xLeft+x1;
}

void RGBLEDMatrixSevSegAMPM(unsigned int bitmap[][32], unsigned int AMPM, int xLeft, int yTop, unsigned int DispM, unsigned int colour)
{
	int x;
	int y;
	unsigned int digits[3]={127^8, 2 | 4 | 16 | 32, 127^(4 | 8)};	//A M P
	char strAMPM[2];
	unsigned int i;
	
	if(AMPM==0){ strAMPM[0]=0; }else{ strAMPM[0]=2; }
	strAMPM[1]=1;
	
	for(i=0;i<2;i++)
	{
		if(digits[strAMPM[i]] & 1)		//seg A
		{
			for(x=xLeft;x<=(xLeft+2);x++){ RGBLEDMatrixSetPixel(bitmap,x,yTop,colour); }
		}

		if(digits[strAMPM[i]] & 2)		//seg B
		{
			for(y=yTop;y<=(yTop+2);y++){ RGBLEDMatrixSetPixel(bitmap,xLeft+2,y,colour); }
		}

		if(digits[strAMPM[i]] & 4)		//seg C
		{
			for(y=yTop+2;y<=(yTop+4);y++){ RGBLEDMatrixSetPixel(bitmap,xLeft+2,y,colour); }
		}

		if(digits[strAMPM[i]] & 8)		//seg D
		{
			for(x=xLeft;x<=(xLeft+2);x++){ RGBLEDMatrixSetPixel(bitmap,x,yTop+4,colour); }
		}

		if(digits[strAMPM[i]] & 16)		//seg E
		{
			for(y=yTop+2;y<=(yTop+4);y++){ RGBLEDMatrixSetPixel(bitmap,xLeft,y,colour); }
		}

		if(digits[strAMPM[i]] & 32)		//seg F
		{
			for(y=yTop;y<=(yTop+2);y++){ RGBLEDMatrixSetPixel(bitmap,xLeft,y,colour); }
		}

		if(digits[strAMPM[i]] & 64)		//seg G
		{
			for(x=xLeft;x<=(xLeft+2);x++){ RGBLEDMatrixSetPixel(bitmap,x,yTop+2,colour); }
		}
		
		if(DispM==0){ break; }
		
		if(i==1){ RGBLEDMatrixSetPixel(bitmap,xLeft+1,yTop+1,colour); } //if M, add the middle dot
		
		xLeft+=4;
	}
}

void RGBLEDMatrixShiftInDigitSevSeg(unsigned int bitmap[][32], unsigned int val, int xRight, int yTop, unsigned int fps, unsigned int colour, unsigned int DisplayMode, unsigned int flipAxis)
{
	unsigned int y;
	unsigned int bytenum;
	
	for(bytenum=val*3;bytenum<=((val*3)+2);bytenum++)
	{
		ResetTimer14();
		for(y=0;y<5;y++)	//add byte
		{
			if(((sevsegasciivbw[bytenum]>>y) & 1)==1){ RGBLEDMatrixSetPixel(bitmap,xRight,y+yTop,colour); }
		}
		if(flipAxis!=LEDNoFlip){ RGBLEDMatrixFlip(bitmap,flipAxis); }	//flip
		RGBLEDMatrixDisplayBitmapDuration(bitmap, 1000/fps, DisplayMode);		//display frame
		if(flipAxis!=LEDNoFlip){ RGBLEDMatrixFlip(bitmap,flipAxis); }	//unflip
		RGBLEDMatrixShiftLeft(bitmap,1);	//scroll
	}

	ResetTimer14();
	if(flipAxis!=LEDNoFlip){ RGBLEDMatrixFlip(bitmap,flipAxis); }	//flip
	RGBLEDMatrixDisplayBitmapDuration(bitmap, 1000/fps, DisplayMode);		//display frame
	if(flipAxis!=LEDNoFlip){ RGBLEDMatrixFlip(bitmap,flipAxis); }	//unflip
	RGBLEDMatrixShiftLeft(bitmap,1);	//extra 1px space after the character
}

void RGBLEDMatrixShiftInDigitSevSegWithBG(unsigned int bitmap[][32], unsigned int bitmapBG[][32], unsigned int val, int xRight, int yTop, unsigned int fps, unsigned int colour, unsigned int DisplayMode, unsigned int flipAxis)
{
	unsigned int y;
	unsigned int bytenum;
	unsigned int outputbitmap[32][32];
	
	for(bytenum=val*3;bytenum<=((val*3)+2);bytenum++)
	{
		ResetTimer14();
		for(y=0;y<5;y++)	//add byte
		{
			if(((sevsegasciivbw[bytenum]>>y) & 1)==1){ RGBLEDMatrixSetPixel(bitmap,xRight,y+yTop,colour); }
		}
		RGBLEDMatrixReplaceColourWithBmpIntoBmp(bitmap,0,bitmapBG,outputbitmap);
		if(flipAxis!=LEDNoFlip){ RGBLEDMatrixFlip(bitmap,flipAxis); }	//flip
		RGBLEDMatrixDisplayBitmapDuration(bitmap, 1000/fps, DisplayMode);		//display frame
		if(flipAxis!=LEDNoFlip){ RGBLEDMatrixFlip(bitmap,flipAxis); }	//unflip
		RGBLEDMatrixShiftLeft(bitmap,1);	//scroll
	}

	ResetTimer14();
	if(flipAxis!=LEDNoFlip){ RGBLEDMatrixFlip(bitmap,flipAxis); }	//flip
	RGBLEDMatrixDisplayBitmapDuration(bitmap, 1000/fps, DisplayMode);		//display frame
	if(flipAxis!=LEDNoFlip){ RGBLEDMatrixFlip(bitmap,flipAxis); }	//unflip
	RGBLEDMatrixShiftLeft(bitmap,1);	//extra 1px space after the character
}

void RGBLEDMatrixScrollValueSevSeg(unsigned int bitmap[][32], unsigned int val, int xRight, int yTop, unsigned int fps, unsigned int colour, unsigned int DisplayMode, unsigned int flipAxis)
{
	unsigned int unit;
	unsigned int digit;
	unsigned int digitval;
	unsigned int print;

	unit=1000000000;
	print=0;
	for(digit=10;digit>=1;digit--)
	{
		digitval=val/unit;
		val -= digitval*unit;
		if((digitval>0) || (digit==1)){ print=1; }
		if(print==1){ RGBLEDMatrixShiftInDigitSevSeg(bitmap, digitval, xRight, yTop, fps, colour, DisplayMode, flipAxis); }
		unit /= 10;
	}
}

void RGBLEDMatrixScrollValueSevSegWithBG(unsigned int bitmap[][32], unsigned int bitmapBG[][32], unsigned int val, int xRight, int yTop, unsigned int fps, unsigned int colour, unsigned int DisplayMode, unsigned int flipAxis)
{
	unsigned int unit;
	unsigned int digit;
	unsigned int digitval;
	unsigned int print;

	unit=1000000000;
	print=0;
	for(digit=10;digit>=1;digit--)
	{
		digitval=val/unit;
		val -= digitval*unit;
		if((digitval>0) || (digit==1)){ print=1; }
		if(print==1){ RGBLEDMatrixShiftInDigitSevSegWithBG(bitmap, bitmapBG, digitval, xRight, yTop, fps, colour, DisplayMode, flipAxis); }
		unit /= 10;
	}
}

void RGBLEDMatrixDrawSegmentLarge(unsigned int bitmap[][32], unsigned int x, unsigned int y, unsigned int length, unsigned int width, unsigned int type, unsigned int colour)
{
	int i;
	
	switch(type)
	{
		case 0:	//left
			for(i=0;i<width;i++)
			{
				RGBLEDMatrixLine(bitmap, x+i,y+i,x+i,y+(length-1)-i,colour);
			}
		break;
		case 1:	//right
			for(i=0;i<width;i++)
			{
				RGBLEDMatrixLine(bitmap, x+((width-1)-i),y+i,x+((width-1)-i),y+(length-1)-i,colour);
			}
		break;
		case 2:	//top
			for(i=0;i<width;i++)
			{
				RGBLEDMatrixLine(bitmap, x+i,y+i,x+(length-1)-i,y+i,colour);
			}
		break;
		case 3:	//bottom
			for(i=0;i<width;i++)
			{
				RGBLEDMatrixLine(bitmap, x+((width-1)-i),y+i,x+(length-1)-((width-1)-i),y+i,colour);
			}
		break;
		case 4:	//middle
			width>>=1;
			for(i=0;i<width;i++)
			{
				RGBLEDMatrixLine(bitmap, x+((width-1)-i),y+i,x+(length-1)-((width-1)-i),y+i,colour);
			}
			y+=width-1;
			for(i=1;i<width;i++)
			{
				RGBLEDMatrixLine(bitmap, x+i,y+i,x+(length-1)-i,y+i,colour);
			}
		break;
	}
}


unsigned int RGBLEDMatrixSevSegDigitLarge(unsigned int bitmap[][32], unsigned int digit, unsigned int x, unsigned int y, unsigned int height, unsigned int thickness, unsigned int gap, unsigned int colour)
{
	unsigned int length;

	length=(height>>1)-(gap>>1);
	
	switch(digit)
	{
		case 0:
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+gap, y, length, thickness, 2, colour);	//top
			RGBLEDMatrixDrawSegmentLarge(bitmap,x, y+gap, length, thickness, 0, colour);	//left top
			RGBLEDMatrixDrawSegmentLarge(bitmap,x, y+length+(gap<<1), length, thickness, 0, colour);	//left bottom
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+gap, y+(length<<1)+(gap<<1)+gap-thickness, length, thickness, 3, colour);	//bottom
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+length+(gap<<1)-thickness, y+length+(gap<<1), length, thickness, 1, colour);	//right bottom
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+length+(gap<<1)-thickness, y+gap, length, thickness, 1, colour);	//right top
		break;
		case 1:
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+length+(gap<<1)-thickness, y+length+(gap<<1), length, thickness, 1, colour);	//right bottom
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+length+(gap<<1)-thickness, y+gap, length, thickness, 1, colour);	//right top
		break;
		case 2:
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+gap, y, length, thickness, 2, colour);	//top
			RGBLEDMatrixDrawSegmentLarge(bitmap,x, y+length+(gap<<1), length, thickness, 0, colour);	//left bottom
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+gap, y+(length<<1)+(gap<<1)+gap-thickness, length, thickness, 3, colour);	//bottom
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+length+(gap<<1)-thickness, y+gap, length, thickness, 1, colour);	//right top
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+gap, y+length+gap+(gap>>1)-((unsigned int)(thickness*1.5)>>1), length, thickness*1.5, 4, colour);	//middle
		break;
		case 3:
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+gap, y, length, thickness, 2, colour);	//top
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+gap, y+(length<<1)+(gap<<1)+gap-thickness, length, thickness, 3, colour);	//bottom
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+length+(gap<<1)-thickness, y+length+(gap<<1), length, thickness, 1, colour);	//right bottom
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+length+(gap<<1)-thickness, y+gap, length, thickness, 1, colour);	//right top
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+gap, y+length+gap+(gap>>1)-((unsigned int)(thickness*1.5)>>1), length, thickness*1.5, 4, colour);	//middle
		break;
		case 4:
			RGBLEDMatrixDrawSegmentLarge(bitmap,x, y+gap, length, thickness, 0, colour);	//left top
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+length+(gap<<1)-thickness, y+length+(gap<<1), length, thickness, 1, colour);	//right bottom
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+length+(gap<<1)-thickness, y+gap, length, thickness, 1, colour);	//right top
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+gap, y+length+gap+(gap>>1)-((unsigned int)(thickness*1.5)>>1), length, thickness*1.5, 4, colour);	//middle
		break;
		case 5:
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+gap, y, length, thickness, 2, colour);	//top
			RGBLEDMatrixDrawSegmentLarge(bitmap,x, y+gap, length, thickness, 0, colour);	//left top
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+gap, y+(length<<1)+(gap<<1)+gap-thickness, length, thickness, 3, colour);	//bottom
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+length+(gap<<1)-thickness, y+length+(gap<<1), length, thickness, 1, colour);	//right bottom
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+gap, y+length+gap+(gap>>1)-((unsigned int)(thickness*1.5)>>1), length, thickness*1.5, 4, colour);	//middle
		break;
		case 6:
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+gap, y, length, thickness, 2, colour);	//top
			RGBLEDMatrixDrawSegmentLarge(bitmap,x, y+gap, length, thickness, 0, colour);	//left top
			RGBLEDMatrixDrawSegmentLarge(bitmap,x, y+length+(gap<<1), length, thickness, 0, colour);	//left bottom
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+gap, y+(length<<1)+(gap<<1)+gap-thickness, length, thickness, 3, colour);	//bottom
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+length+(gap<<1)-thickness, y+length+(gap<<1), length, thickness, 1, colour);	//right bottom
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+gap, y+length+gap+(gap>>1)-((unsigned int)(thickness*1.5)>>1), length, thickness*1.5, 4, colour);	//middle
		break;
		case 7:
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+gap, y, length, thickness, 2, colour);	//top
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+length+(gap<<1)-thickness, y+length+(gap<<1), length, thickness, 1, colour);	//right bottom
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+length+(gap<<1)-thickness, y+gap, length, thickness, 1, colour);	//right top
		break;
		case 8:
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+gap, y, length, thickness, 2, colour);	//top
			RGBLEDMatrixDrawSegmentLarge(bitmap,x, y+gap, length, thickness, 0, colour);	//left top
			RGBLEDMatrixDrawSegmentLarge(bitmap,x, y+length+(gap<<1), length, thickness, 0, colour);	//left bottom
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+gap, y+(length<<1)+(gap<<1)+gap-thickness, length, thickness, 3, colour);	//bottom
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+length+(gap<<1)-thickness, y+length+(gap<<1), length, thickness, 1, colour);	//right bottom
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+length+(gap<<1)-thickness, y+gap, length, thickness, 1, colour);	//right top
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+gap, y+length+gap+(gap>>1)-((unsigned int)(thickness*1.5)>>1), length, thickness*1.5, 4, colour);	//middle
		break;
		case 9:
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+gap, y, length, thickness, 2, colour);	//top
			RGBLEDMatrixDrawSegmentLarge(bitmap,x, y+gap, length, thickness, 0, colour);	//left top
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+gap, y+(length<<1)+(gap<<1)+gap-thickness, length, thickness, 3, colour);	//bottom
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+length+(gap<<1)-thickness, y+length+(gap<<1), length, thickness, 1, colour);	//right bottom
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+length+(gap<<1)-thickness, y+gap, length, thickness, 1, colour);	//right top
			RGBLEDMatrixDrawSegmentLarge(bitmap,x+gap, y+length+gap+(gap>>1)-((unsigned int)(thickness*1.5)>>1), length, thickness*1.5, 4, colour);	//middle
		break;
	}

	return x+length+(gap<<1)+(length>>2);
}


unsigned int RGBLEDMatrixPrintValueSevSegLarge(unsigned int bitmap[][32], unsigned int val, unsigned int xpos, unsigned int ypos, unsigned int height, unsigned int thickness, unsigned int gap, unsigned int colour)
{
	unsigned int unit;
	unsigned int digit;
	unsigned int digitval;
	unsigned int print;

	unit=1000000000;
	print=0;

	for(digit=10;digit>=1;digit--)
	{
		digitval=val/unit;
		val -= digitval*unit;
		if((digitval>0) || (digit==1)){ print=1; }
		if(print==1){ xpos=RGBLEDMatrixSevSegDigitLarge(bitmap, digitval, xpos, ypos, height, thickness, gap, colour); }
		unit /= 10;
	}

	return xpos;
}


unsigned int RGBLEDMatrixPrintValue2DigitsSevSegLarge(unsigned int bitmap[][32], unsigned int val, unsigned int xpos, unsigned int ypos, unsigned int height, unsigned int thickness, unsigned int gap, unsigned int colour)
{
	unsigned int tens, units;

	tens=val/10;
	units=val-(tens*10);
	xpos=RGBLEDMatrixSevSegDigitLarge(bitmap, tens, xpos, ypos, height, thickness, gap, colour);
	xpos=RGBLEDMatrixSevSegDigitLarge(bitmap, units, xpos, ypos, height, thickness, gap, colour);

	return xpos;
}

void printfRGBLEDMatrix(unsigned int bitmap[][32], int x1, int y1, unsigned int font, unsigned int TextColour, int BackColour, const char * format, ... )
{
	char buff[256];
	unsigned int i=0;
	va_list args;
	
	va_start(args,format);
	vsnprintf(buff,256,format,args);
	
  //print the string
	do
	{
		x1=RGBLEDMatrixCharacter(bitmap, buff[i], x1, y1, font, TextColour, BackColour);
		i+=1;
	}while(buff[i]!=0);
	
	va_end(args);
}

void RGBLEDMatrixPlaceBitmap(const unsigned int sourcebitmap[][32], unsigned int destbitmap[][32], unsigned int width, unsigned int height, int xpos, int ypos)
{
	unsigned int x, y;
	
	for(x=0;x<width;x++)
	{
		for(y=0;y<height;y++)
		{
			RGBLEDMatrixSetPixel(destbitmap,x+xpos,y+ypos,sourcebitmap[x][y]);
		}
	}
}

void RGBLEDMatrixShiftInBitmap(const unsigned int sourcebitmap[][32], unsigned int destbitmap[][32], unsigned int width, unsigned int height, int xRight, int yTop, unsigned int endgap, unsigned int fps, unsigned int DisplayMode, unsigned int flipAxis)
{
	unsigned int x, y, mS;
	
	mS=1000/fps;
	
	for(x=0;x<width;x++)
	{
		for(y=0;y<height;y++)
		{
			RGBLEDMatrixSetPixel(destbitmap,xRight,yTop+y,sourcebitmap[x][y]);
		}
		if(flipAxis!=LEDNoFlip){ RGBLEDMatrixFlip(destbitmap,flipAxis); }	//flip
		RGBLEDMatrixDisplayBitmapDuration(destbitmap, mS, DisplayMode);		//display frame
		if(flipAxis!=LEDNoFlip){ RGBLEDMatrixFlip(destbitmap,flipAxis); }	//unflip
		RGBLEDMatrixShiftLeft(destbitmap,1);	//scroll
	}

	for(x=0;x<endgap;x++)
	{
		if(flipAxis!=LEDNoFlip){ RGBLEDMatrixFlip(destbitmap,flipAxis); }	//flip
		RGBLEDMatrixDisplayBitmapDuration(destbitmap, mS, DisplayMode);		//display frame
		if(flipAxis!=LEDNoFlip){ RGBLEDMatrixFlip(destbitmap,flipAxis); }	//unflip
		RGBLEDMatrixShiftLeft(destbitmap,1);
	}
}

void RGBLEDMatrixFlip(unsigned int bitmap[][32], unsigned int flipAxis)
{
	unsigned int bmp[LedMatrixWidth][32], x, y;
	
	if(flipAxis & 1)	//flip X
	{
		for(y=0;y<32;y++)
		{
			for(x=0;x<LedMatrixWidth;x++)
			{
				bmp[x][y]=bitmap[(LedMatrixWidth-1)-x][y];
			}
		}
		
		for(y=0;y<32;y++)	//write back
		{
			for(x=0;x<LedMatrixWidth;x++)
			{
				bitmap[x][y]=bmp[x][y];
			}
		}
	}
	
	if(flipAxis & 2)	//flip Y
	{
		for(y=0;y<32;y++)
		{
			for(x=0;x<LedMatrixWidth;x++)
			{
				bmp[x][y]=bitmap[x][31-y];
			}
		}
		
		for(y=0;y<32;y++)	//write back
		{
			for(x=0;x<LedMatrixWidth;x++)
			{
				bitmap[x][y]=bmp[x][y];
			}
		}
	}
}
