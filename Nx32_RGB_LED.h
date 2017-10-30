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
#define LED8BITYELLOW 0xFFFF00
#define LED8BITCYAN 0x00FFFF
#define LED8BITPURPLE 0xFF00FF
#define LED8BITWHITE 0xFFFFFF
#define LED8BITORANGE 0xFF7F00

#define LEDNoFlip 0
#define LEDXFlip 1
#define LEDYFlip 2
#define LEDXYFlip 3

void InitRGBLEDMatrix(void);

//tests
void RGBLEDMatrixPixelTest(unsigned int colour, unsigned int delayMS);		//top to bottom, right to left. colour is 1 bit per channel
void RGBLEDMatrixBitmapTest(unsigned int mode);	//mode must be either: DisplayMode1bit1px, DisplayMode1bit1Line, DisplayMode1bit2Lines, DisplayMode4bit, DisplayMode8bit.	run this in a loop

//special functions that dont mux the entire display, bright, use 1 bit colours
void RGBLEDMatrixLightUpPixel(unsigned int x, unsigned int y, unsigned int colour);	//latched after run
void RGBLEDMatrixLightUpLine(unsigned int y, unsigned int colour);	//latched after run
void RGBLEDMatrixLightUpArea(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int colour);	//run in a loop
void RGBLEDMatrixLightScreen(unsigned int colour);	//run in a loop, 64px at a time
void RGBLEDMatrixClearScreen(void);	//clears the upper and lower shift registers and then latches
void RGBLEDMatrixLightUpAreaSegmented(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int colour);	//used by LightUpArea

//loop these
void RGBLEDMatrixDispBitmap1bit1px(unsigned int bitmap[][32]);	// 00000000 00000000 00000000 00000RGB  1 pixel at a time
void RGBLEDMatrixDispBitmap1bit1Line(unsigned int bitmap[][32]);	// 00000000 00000000 00000000 00000RGB 32 pixel at a time 
void RGBLEDMatrixDispBitmap1bit2Lines(unsigned int bitmap[][32]);	// 00000000 00000000 00000000 00000RGB 64 pixel at a time 
void RGBLEDMatrixDispBitmap4bit(unsigned int bitmap[][32]);			// 00000000 00000000 0000RRRR GGGGBBBB 64 pixel at a time using 4 bit pwm
void RGBLEDMatrixDispBitmap8bit(unsigned int bitmap[][32]);       // 00000000 RRRRRRRR GGGGGGGG BBBBBBBB 64 pixel at a time using 8 bit weighted binary
void RGBLEDMatrixDisplayBitmap(unsigned int bitmap[][32], unsigned int mode);		//mode must be either: DisplayMode1bit1px, DisplayMode1bit1Line, DisplayMode1bit2Lines, DisplayMode4bit, DisplayMode8bit.	run this in a loop
void RGBLEDMatrixDisplayBitmapDuration(unsigned int bitmap[][32], unsigned int durationMS, unsigned int mode);		//mode must be either: DisplayMode1bit1px, DisplayMode1bit1Line, DisplayMode1bit2Lines, DisplayMode4bit, DisplayMode8bit.

void RGBLEDMatrixClear(unsigned int bitmap[][32], unsigned int colour);
void RGBLEDMatrixCopy(unsigned int sourcebitmap[][32], unsigned int destbitmap[][32]);
void RGBLEDMatrixSetPixel(unsigned int bitmap[][32], int x, int y, unsigned int colour);	//1, 4, or 8 bit format
unsigned int RGBLEDMatrixReadPixel(unsigned int bitmap[][32], int x, int y);
void RGBLEDMatrixMixPixel(unsigned int bitmap[][32], int x, int y, unsigned int colour);	//values are ORed
void RGBLEDMatrixMix(unsigned int sourcebitmap[][32], unsigned int destbitmap[][32]); //values are ORed
void RGBLEDMatrixMultiply(unsigned int sourcebitmap[][32], unsigned int destbitmap[][32]);
void RGBLEDMatrixInvertColours(unsigned int bitmap[][32]);
void RGBLEDMatrixShiftLeft(unsigned int bitmap[][32], int shift);
void RGBLEDMatrixShiftRight(unsigned int bitmap[][32], int shift);
void RGBLEDMatrixShiftUp(unsigned int bitmap[][32], int shift);
void RGBLEDMatrixShiftDown(unsigned int bitmap[][32], int shift);
void RGBLEDMatrixRotate(unsigned int sourcebitmap[][32], unsigned int destbitmap[][32], double angle, double midx, double midy);
void RGBLEDMatrixFlip(unsigned int bitmap[][32], unsigned int flipAxis);	//use with LEDXFlip/LEDYFlip/LEDXYFlip
void RGBLEDMatrixPlaceBitmap(const unsigned int sourcebitmap[][32], unsigned int destbitmap[][32], unsigned int width, unsigned int height, int xpos, int ypos);

void RGBLEDMatrixReplaceColourWithColour(unsigned int bitmap[][32], unsigned int ColourToReplace, unsigned int ReplacementColour);
void RGBLEDMatrixReplaceColourWithBmp(unsigned int bitmap[][32], unsigned int ColourToReplace, unsigned int backgroundbitmap[][32]);
void RGBLEDMatrixReplaceColourWithBmpIntoBmp(unsigned int bitmap[][32], unsigned int ColourToReplace, unsigned int backgroundbitmap[][32], unsigned int destbitmap[][32]);
void RGBLEDMatrixFadeDown(unsigned int bitmap[][32], unsigned int delayMS);	//8 bit mode only
void RGBLEDMatrixFadeUp(unsigned int bitmap[][32], unsigned int delayMS);	//8 bit mode only
void RGBLEDMatrixFadeBetween(unsigned int bitmap1[][32], unsigned int bitmap2[][32]);	//8 bit mode only

void RGBLEDMatrixLine(unsigned int bitmap[][32], int x1, int y1, int x2, int y2,  unsigned int colour);
void RGBLEDMatrixRectangle(unsigned int bitmap[][32], int x1, int y1, int x2, int y2,  unsigned int colour);
void RGBLEDMatrixRectangleFilled(unsigned int bitmap[][32], int x1, int y1, int x2, int y2,  unsigned int colour);
void RGBLEDMatrixCircle(unsigned int bitmap[][32], int x0, int y0, int radius, unsigned int colour);
void RGBLEDMatrixCircleFilled(unsigned int bitmap[][32], int x0, int y0, int radius, unsigned int colour);

//for functions using flipaxis, supply one of these: LEDNoFlip/LEDXFlip/LEDYFlip/LEDXYFlip
unsigned int RGBLEDMatrixCharacter(unsigned int bitmap[][32], char character, int x1, int y1, unsigned int colour, unsigned int font);	//font must be LedFontVariableWidth or LedFont7x11. returns the x position for the next character. 
unsigned int RGBLEDMatrixGetCharacterLength(char character);
void RGBLEDMatrixShiftInCharacter(unsigned int bitmap[][32], char character, int xRight, int yTop, unsigned int fps, unsigned int colour, unsigned int font, unsigned int DisplayMode, unsigned int flipAxis);
void RGBLEDMatrixShiftInCharacterWithBG(unsigned int bitmap[][32], unsigned int bitmapBG[][32], char character, int xRight, int yTop, unsigned int fps, unsigned int colour, unsigned int font, unsigned int DisplayMode, unsigned int flipAxis);
void RGBLEDMatrixScrollText(unsigned int bitmap[][32], char characters[200], int xRight, int yTop, unsigned int fps, unsigned int colour, unsigned int font, unsigned int DisplayMode, unsigned int flipAxis);
void RGBLEDMatrixScrollTextWithBG(unsigned int bitmap[][32], unsigned int bitmapBG[][32], char characters[200], int xRight, int yTop, unsigned int fps, unsigned int colour, unsigned int font, unsigned int DisplayMode, unsigned int flipAxis);
void RGBLEDMatrixScrollValue(unsigned int bitmap[][32], unsigned int val, int xRight, int yTop, unsigned int fps, unsigned int colour, unsigned int font, unsigned int DisplayMode, unsigned int flipAxis);
void RGBLEDMatrixScrollValueWithBG(unsigned int bitmap[][32], unsigned int bitmapBG[][32], unsigned int val, int xRight, int yTop, unsigned int fps, unsigned int colour, unsigned int font, unsigned int DisplayMode, unsigned int flipAxis);

void RGBLEDMatrixShiftInCharacterHebrew(unsigned int bitmap[][32], char character, int xLeft, int yTop, unsigned int fps, unsigned int colour, unsigned int DisplayMode, unsigned int flipAxis);
void RGBLEDMatrixScrollTextHebrew(unsigned int bitmap[][32], char characters[200], int xLeft, int yTop, unsigned int fps, unsigned int colour, unsigned int DisplayMode, unsigned int flipAxis);

void RGBLEDMatrixShiftInBitmap(const unsigned int sourcebitmap[][32], unsigned int destbitmap[][32], unsigned int width, unsigned int height, int xRight, int yTop, unsigned int endgap, unsigned int fps, unsigned int DisplayMode, unsigned int flipAxis);

void printfRGBLEDMatrix(unsigned int bitmap[][32], int x1, int y1, unsigned int font, unsigned int colour, const char * format, ... );	//font must be LedFontVariableWidth or LedFont7x11

void RGBLEDMatrixSevSegDigit(unsigned int bitmap[][32], unsigned int val, int xLeft, int yTop, unsigned int colour);
unsigned int RGBLEDMatrixSevSegSegMultiDigits(unsigned int bitmap[][32], unsigned int val, int xLeft, int yTop, unsigned int ForceDigits, unsigned int colour);	//returns x position for next character. if ForceDigits= 0 or 1 then it will use as many digits as needed, 2 or more will left append 0s if needed
void RGBLEDMatrixSevSegAMPM(unsigned int bitmap[][32], unsigned int AMPM, int xLeft, int yTop, unsigned int DispM, unsigned int colour);	//0 for AM, 1 for PM, DispM=0 will not show the M
void RGBLEDMatrixShiftInDigitSevSeg(unsigned int bitmap[][32], unsigned int val, int xRight, int yTop, unsigned int fps, unsigned int colour, unsigned int DisplayMode, unsigned int flipAxis);
void RGBLEDMatrixShiftInDigitSevSegWithBG(unsigned int bitmap[][32], unsigned int bitmapBG[][32], unsigned int val, int xRight, int yTop, unsigned int fps, unsigned int colour, unsigned int DisplayMode, unsigned int flipAxis);
void RGBLEDMatrixScrollValueSevSeg(unsigned int bitmap[][32], unsigned int val, int xRight, int yTop, unsigned int fps, unsigned int colour, unsigned int DisplayMode, unsigned int flipAxis);
void RGBLEDMatrixScrollValueSevSegWithBG(unsigned int bitmap[][32], unsigned int bitmapBG[][32], unsigned int val, int xRight, int yTop, unsigned int fps, unsigned int colour, unsigned int DisplayMode, unsigned int flipAxis);

void RGBLEDMatrixDrawSegmentLarge(unsigned int bitmap[][32], unsigned int x, unsigned int y, unsigned int length, unsigned int width, unsigned int type, unsigned int colour);
unsigned int RGBLEDMatrixSevSegDigitLarge(unsigned int bitmap[][32], unsigned int digit, unsigned int x, unsigned int y, unsigned int height, unsigned int thickness, unsigned int gap, unsigned int colour);
unsigned int RGBLEDMatrixPrintValueSevSegLarge(unsigned int bitmap[][32], unsigned int val, unsigned int xpos, unsigned int ypos, unsigned int height, unsigned int thickness, unsigned int gap, unsigned int colour);
unsigned int RGBLEDMatrixPrintValue2DigitsSevSegLarge(unsigned int bitmap[][32], unsigned int val, unsigned int xpos, unsigned int ypos, unsigned int height, unsigned int thickness, unsigned int gap, unsigned int colour);
