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

#define LedMatrixWidth 64
#define LedMatrixHeight 32

//pin assignment
#define R1Pin 12
#define G1Pin 13
#define B1Pin 14

#define R0Pin 3
#define G0Pin 8
#define B0Pin 11

#define RowSelectAPin 2
#define RowSelectBPin 3
#define RowSelectCPin 4
#define RowSelectDPin 5

#define CLKPin 4
#define STBPin 5

#define OEPin 6


#define R1Port PORTC
#define G1Port PORTC
#define B1Port PORTC

#define R0Port PORTC
#define G0Port PORTC
#define B0Port PORTC

#define RowSelectAPort PORTE
#define RowSelectBPort PORTE
#define RowSelectCPort PORTE
#define RowSelectDPort PORTE

#define CLKPort PORTD
#define STBPort PORTD

#define OEPort PORTE

//display modes
#define DisplayMode1bit1px 0
#define DisplayMode1bit1Line 1
#define DisplayMode1bit2Lines 2
#define DisplayMode4bit 3
#define DisplayMode8bit 4

//fonts
#define LedFontVariableWidth 0
#define LedFont7x11 1

//1 bit/channel colours
#define LED1BITBLACK 0
#define LED1BITRED 4
#define LED1BITGREEN 2
#define LED1BITBLUE 1
#define LED1BITYELLOW 6
#define LED1BITCYAN 3
#define LED1BITPURPLE 5
#define LED1BITWHITE 7

//4 bit/channel colours
#define LED4BITBLACK 0x000
#define LED4BITRED 0xF00
#define LED4BITGREEN 0x0F0
#define LED4BITBLUE 0x00F
#define LED4BITYELLOW 0xFF0
#define LED4BITCYAN 0x0FF
#define LED4BITPURPLE 0xF0F
#define LED4BITWHITE 0xFFF
#define LED4BITORANGE 0xF70

//8 bit/channel colours
#define LED8BITBLACK 0x000000
#define LED8BITRED 0xFF0000
#define LED8BITGREEN 0x00FF00
#define LED8BITBLUE 0x0000FF
#define LED8BITYELLOW 0xFFFF 00
#define LED8BITCYAN 0x00FFFF
#define LED8BITPURPLE 0xFF00FF
#define LED8BITWHITE 0xFFFFFF
#define LED8BITORANGE 0xFF7F00

void InitRGBLEDMatrix(void);

//tests
void PixelTest(unsigned int colour, unsigned int delayMS);		//top to bottom, right to left
void BitmapTest(unsigned int mode);	//mode must be either: DisplayMode1bit1px, DisplayMode1bit1Line, DisplayMode1bit2Lines, DisplayMode4bit, DisplayMode8bit.	run this in a loop

//special functions that dont mux the entire display, bright, use 1 bit colours
void LightUpPixel(unsigned int x, unsigned int y, unsigned int colour);	//latched after run
void LightUpLine(unsigned int y, unsigned int colour);	//latched after run
void LightUpArea(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int colour);	//run in a loop
void LightLEDScreen(unsigned int colour);	//run in a loop, 64px at a time
void ClearLEDScreen(void);	//clears the upper and lower shift registers and then latches
void LightUpAreaSegmented(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int colour);	//used by LightUpArea

//loop these
void DispBitmap1bit1px(unsigned int bitmap[][32]);	// 00000000 00000000 00000000 00000RGB  1 pixel at a time
void DispBitmap1bit1Line(unsigned int bitmap[][32]);	// 00000000 00000000 00000000 00000RGB 32 pixel at a time 
void DispBitmap1bit2Lines(unsigned int bitmap[][32]);	// 00000000 00000000 00000000 00000RGB 64 pixel at a time 
void DispBitmap4bit(unsigned int bitmap[][32]);			// 00000000 00000000 0000RRRR GGGGBBBB 64 pixel at a time using 4 bit pwm
void DispBitmap8bit(unsigned int bitmap[][32]);       // 00000000 RRRRRRRR GGGGGGGG BBBBBBBB 64 pixel at a time using 8 bit weighted binary
void DisplayBitmap(unsigned int bitmap[][32], unsigned int mode);		//mode must be either: DisplayMode1bit1px, DisplayMode1bit1Line, DisplayMode1bit2Lines, DisplayMode4bit, DisplayMode8bit.	run this in a loop
void DisplayBitmapDuration(unsigned int bitmap[][32], unsigned int durationMS, unsigned int mode);		//mode must be either: DisplayMode1bit1px, DisplayMode1bit1Line, DisplayMode1bit2Lines, DisplayMode4bit, DisplayMode8bit.

void BmpClear(unsigned int bitmap[][32], unsigned int colour);
void BmpCopy(unsigned int sourcebitmap[][32], unsigned int destbitmap[][32]);
void BmpSetPixel(unsigned int bitmap[][32], int x, int y, unsigned int colour);	//1, 4, or 8 bit format
unsigned int BmpReadPixel(unsigned int bitmap[][32], int x, int y);
void BmpMixPixel(unsigned int bitmap[][32], int x, int y, unsigned int colour);	//values are ORed
void BmpMix(unsigned int sourcebitmap[][32], unsigned int destbitmap[][32]); //values are ORed
void BmpMultiply(unsigned int sourcebitmap[][32], unsigned int destbitmap[][32]);
void BmpInvertColours(unsigned int bitmap[][32]);
void BmpShiftLeft(unsigned int bitmap[][32], int shift);
void BmpShiftRight(unsigned int bitmap[][32], int shift);
void BmpShiftUp(unsigned int bitmap[][32], int shift);
void BmpShiftDown(unsigned int bitmap[][32], int shift);
void BmpRotate(unsigned int sourcebitmap[][32], unsigned int destbitmap[][32], double angle, double midx, double midy);
void BmpPlaceBitmap(const unsigned int sourcebitmap[][32], unsigned int destbitmap[][32], unsigned int width, unsigned int height, int xpos, int ypos);

void BmpReplaceColourWithColour(unsigned int bitmap[][32], unsigned int ColourToReplace, unsigned int ReplacementColour);
void BmpReplaceColourWithBmp(unsigned int bitmap[][32], unsigned int ColourToReplace, unsigned int backgroundbitmap[][32]);
void BmpReplaceColourWithBmpIntoBmp(unsigned int bitmap[][32], unsigned int ColourToReplace, unsigned int backgroundbitmap[][32], unsigned int destbitmap[][32]);
void BmpFadeDown(unsigned int bitmap[][32], unsigned int delayMS);	//8 bit mode only
void BmpFadeUp(unsigned int bitmap[][32], unsigned int delayMS);	//8 bit mode only

void BmpLine(unsigned int bitmap[][32], int x1, int y1, int x2, int y2,  unsigned int colour);
void BmpRectangle(unsigned int bitmap[][32], int x1, int y1, int x2, int y2,  unsigned int colour);
void BmpRectangleFilled(unsigned int bitmap[][32], int x1, int y1, int x2, int y2,  unsigned int colour);
void BmpCircle(unsigned int bitmap[][32], int x0, int y0, int radius, unsigned int colour);
void BmpCircleFilled(unsigned int bitmap[][32], int x0, int y0, int radius, unsigned int colour);

unsigned int BmpCharacter(unsigned int bitmap[][32], char character, int x1, int y1, unsigned int colour, unsigned int font);	//font must be LedFontVariableWidth or LedFont7x11. returns the x position for the next character. 
unsigned int BmpGetCharacterLength(char character);
void BmpShiftInCharacter(unsigned int bitmap[][32], char character, int xRight, int yTop, unsigned int fps, unsigned int colour, unsigned int font, unsigned int DisplayMode);
void BmpShiftInCharacterWithBG(unsigned int bitmap[][32], unsigned int bitmapBG[][32], char character, int xRight, int yTop, unsigned int fps, unsigned int colour, unsigned int font, unsigned int DisplayMode);
void BmpScrollText(unsigned int bitmap[][32], char characters[200], int xRight, int yTop, unsigned int fps, unsigned int colour, unsigned int font, unsigned int DisplayMode);
void BmpScrollTextWithBG(unsigned int bitmap[][32], unsigned int bitmapBG[][32], char characters[200], int xRight, int yTop, unsigned int fps, unsigned int colour, unsigned int font, unsigned int DisplayMode);
void BmpScrollValue(unsigned int bitmap[][32], unsigned int val, int xRight, int yTop, unsigned int fps, unsigned int colour, unsigned int font, unsigned int DisplayMode);
void BmpScrollValueWithBG(unsigned int bitmap[][32], unsigned int bitmapBG[][32], unsigned int val, int xRight, int yTop, unsigned int fps, unsigned int colour, unsigned int font, unsigned int DisplayMode);

void BmpShiftInBitmap(const unsigned int sourcebitmap[][32], unsigned int destbitmap[][32], unsigned int width, unsigned int height, int xRight, int yTop, unsigned int endgap, unsigned int fps, unsigned int DisplayMode);

void PrintfNx32LED(unsigned int bitmap[][32], int x1, int y1, unsigned int font, unsigned int colour, const char * format, ... );	//font must be LedFontVariableWidth or LedFont7x11

void BmpSevSegDigit(unsigned int bitmap[][32], unsigned int val, int xLeft, int yTop, unsigned int colour);
unsigned int BmpSevSegSegMultiDigits(unsigned int bitmap[][32], unsigned int val, int xLeft, int yTop, unsigned int ForceDigits, unsigned int colour);	//returns x position for next character. if ForceDigits= 0 or 1 then it will use as many digits as needed, 2 or more will left append 0s if needed
void BmpSevSegAMPM(unsigned int bitmap[][32], unsigned int AMPM, int xLeft, int yTop, unsigned int DispM, unsigned int colour);	//0 for AM, 1 for PM, DispM=0 will not show the M
void BmpShiftInDigitSevSeg(unsigned int bitmap[][32], unsigned int val, int xRight, int yTop, unsigned int fps, unsigned int colour, unsigned int DisplayMode);
void BmpShiftInDigitSevSegWithBG(unsigned int bitmap[][32], unsigned int bitmapBG[][32], unsigned int val, int xRight, int yTop, unsigned int fps, unsigned int colour, unsigned int DisplayMode);
void BmpScrollValueSevSeg(unsigned int bitmap[][32], unsigned int val, int xRight, int yTop, unsigned int fps, unsigned int colour, unsigned int DisplayMode);
void BmpScrollValueSevSegWithBG(unsigned int bitmap[][32], unsigned int bitmapBG[][32], unsigned int val, int xRight, int yTop, unsigned int fps, unsigned int colour, unsigned int DisplayMode);

void DrawSegmentLarge(unsigned int bitmap[][32], unsigned int x, unsigned int y, unsigned int length, unsigned int width, unsigned int type, unsigned int colour);
unsigned int SevSegDigitLarge(unsigned int bitmap[][32], unsigned int digit, unsigned int x, unsigned int y, unsigned int height, unsigned int thickness, unsigned int gap, unsigned int colour);
unsigned int PrintValueSevSegLarge(unsigned int bitmap[][32], unsigned int val, unsigned int xpos, unsigned int ypos, unsigned int height, unsigned int thickness, unsigned int gap, unsigned int colour);
unsigned int PrintValue2DigitsSevSegLarge(unsigned int bitmap[][32], unsigned int val, unsigned int xpos, unsigned int ypos, unsigned int height, unsigned int thickness, unsigned int gap, unsigned int colour);
