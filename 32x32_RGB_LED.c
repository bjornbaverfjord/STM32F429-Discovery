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
#include "32x32_RGB_LED.h"
#include <math.h>
#include <stdarg.h>

/******************************************************
 *              Font 7x11(English)                    *
 * -ASCII fonts from 0x20 ~ 0x7F(DEC 32 ~ 126)     	  *
 *   bits 5 (bottom) to 15 (top) in the y direction   *
 *	      7 words=7 lines in the x direction		  *
 ******************************************************/
const unsigned short LEDascii_7x11[95][14] = {                                                       
	{0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x001F}, //0x0020 space
	{0x0000, 0x07C0, 0xDFE0, 0xDFE0, 0x07C0, 0x0000, 0x001F}, //0x0021 !
	{0x0000, 0x00E0, 0x00E0, 0x0000, 0x00E0, 0x00E0, 0x001F}, //0x0022 "
	{0x0000, 0x1B00, 0x7FC0, 0x1B00, 0x7FC0, 0x1B00, 0x001F}, //0x0023 #
	{0x1300, 0x2780, 0x2480, 0x7FC0, 0x2480, 0x3880, 0x191F}, //0x0024 $
	{0x3080, 0x1940, 0x2D40, 0x5680, 0x5300, 0x2180, 0x001F}, //0x0025 %
	{0x3800, 0x7D80, 0x47C0, 0x5E40, 0x7BC0, 0x2180, 0x581F}, //0x0026 &
	{0x0000, 0x0000, 0x00E0, 0x00E0, 0x0000, 0x0000, 0x001F}, //0x0027 '
	{0x0000, 0x0000, 0x3F80, 0x7FC0, 0xC060, 0x8020, 0x001F}, //0x0028 (
	{0x0000, 0x8020, 0xC060, 0x7FC0, 0x3F80, 0x0000, 0x001F}, //0x0029 )
	{0x0000, 0x2480, 0x3F80, 0x0E00, 0x3F80, 0x2480, 0x001F}, //0x002A *
	{0x0400, 0x0400, 0x1F00, 0x1F00, 0x0400, 0x0400, 0x001F}, //0x002B +
	{0x0000, 0x8000, 0xE000, 0x6000, 0x0000, 0x0000, 0x001F}, //0x002C ,
	{0x0400, 0x0400, 0x0400, 0x0400, 0x0400, 0x0400, 0x001F}, //0x002D -
	{0x0000, 0x0000, 0xC000, 0xC000, 0x0000, 0x0000, 0x001F}, //0x002E .
	{0xC000, 0xF000, 0x3C00, 0x0F00, 0x03C0, 0x00E0, 0x003F}, //0x002F /
	{0x7FC0, 0xFFE0, 0x8020, 0x8020, 0x8020, 0xFFE0, 0x7FDF}, //0x0030 0
	{0x0000, 0x8080, 0x80C0, 0xFFE0, 0xFFE0, 0x8000, 0x001F}, //0x0031 1
	{0xE0C0, 0xF0E0, 0x9820, 0x8C20, 0x8420, 0x87E0, 0x83DF}, //0x0032 2
	{0x60C0, 0xE0E0, 0x8420, 0x8420, 0x8420, 0xFFE0, 0x7BDF}, //0x0033 3
	{0x3800, 0x3E00, 0x2780, 0x21E0, 0xFFE0, 0xFFE0, 0x201F}, //0x0034 4
	{0x83E0, 0x83E0, 0x8220, 0x8220, 0xC220, 0x7E20, 0x3C3F}, //0x0035 5
	{0x7F00, 0xFFC0, 0x8260, 0x8220, 0x8220, 0xFE20, 0x7C3F}, //0x0036 6
	{0x0020, 0x0020, 0xE020, 0xFC20, 0x1F20, 0x03E0, 0x00FF}, //0x0037 7
	{0x7BC0, 0xFFE0, 0x8620, 0x8620, 0x8C20, 0xFFE0, 0x7BDF}, //0x0038 8
	{0x03C0, 0x87E0, 0x8420, 0xC420, 0x6420, 0x3FE0, 0x1FDF}, //0x0039 9
	{0x0000, 0x0000, 0x3180, 0x3180, 0x3180, 0x0000, 0x001F}, //0x003A :
	{0x0000, 0x0000, 0xB180, 0xF180, 0x7180, 0x0000, 0x001F}, //0x003B ;
	{0x0400, 0x0E00, 0x1B00, 0x3180, 0x60C0, 0xC060, 0x803F}, //0x003C <
	{0x0A00, 0x0A00, 0x0A00, 0x0A00, 0x0A00, 0x0A00, 0x0A1F}, //0x003D =
	{0x8020, 0xC060, 0x60C0, 0x3180, 0x1B00, 0x0E00, 0x041F}, //0x003E >
	{0x01C0, 0x01E0, 0x0020, 0xD820, 0xDC20, 0x07E0, 0x03DF}, //0x003F ?
	{0x7F40, 0x8320, 0x8320, 0xFF20, 0x8060, 0xFFE0, 0x7FDF}, //0x0040 @
	{0xFF00, 0xFF80, 0x08C0, 0x0860, 0x08C0, 0xFF80, 0xFF1F}, //0x0041 A
	{0xFFE0, 0xFFE0, 0x8420, 0x8420, 0x8420, 0xFFE0, 0x7BDF}, //0x0042 B
	{0x7FC0, 0xFFE0, 0x8020, 0x8020, 0x8020, 0xE0E0, 0x60DF}, //0x0043 C
	{0xFFE0, 0xFFE0, 0x8020, 0x8020, 0xC060, 0x7FC0, 0x3F9F}, //0x0044 D
	{0xFFE0, 0xFFE0, 0x8420, 0x8420, 0x8420, 0x8420, 0x803F}, //0x0045 E
	{0xFFE0, 0xFFE0, 0x0420, 0x0420, 0x0420, 0x0420, 0x003F}, //0x0046 F
	{0x7FC0, 0xFFE0, 0x8020, 0x8020, 0x8820, 0xF8E0, 0xF8DF}, //0x0047 G
	{0xFFE0, 0xFFE0, 0x0400, 0x0400, 0x0400, 0xFFE0, 0xFFFF}, //0x0048 H
	{0x0000, 0x8020, 0xFFE0, 0xFFE0, 0x8020, 0x0000, 0x001F}, //0x0049 I
	{0x6000, 0xE000, 0x8000, 0x8000, 0x8000, 0xFFE0, 0x7FFF}, //0x004A J
	{0xFFE0, 0xFFE0, 0x0400, 0x1F00, 0x7BC0, 0xE0E0, 0x803F}, //0x004B K
	{0xFFE0, 0xFFE0, 0x8000, 0x8000, 0x8000, 0x8000, 0x801F}, //0x004C L
	{0xFFE0, 0xFFE0, 0x0080, 0x0700, 0x0080, 0xFFE0, 0xFFFF}, //0x004D M
	{0xFFE0, 0xFFE0, 0x0380, 0x0E00, 0x3800, 0xFFE0, 0xFFFF}, //0x004E N
	{0x7FC0, 0xFFE0, 0x8020, 0x8020, 0x8020, 0xFFE0, 0x7FDF}, //0x004F O
	{0xFFE0, 0xFFE0, 0x0420, 0x0420, 0x0420, 0x07E0, 0x03DF}, //0x0050 P
	{0x7FC0, 0xFFE0, 0x9020, 0xB020, 0xE020, 0x7FE0, 0xFFDF}, //0x0051 Q
	{0xFFE0, 0xFFE0, 0x0420, 0x0420, 0x0C20, 0xFFE0, 0xF3DF}, //0x0052 R
	{0x43C0, 0xC7E0, 0x8620, 0x8420, 0x8C20, 0xFC60, 0x785F}, //0x0053 S
	{0x0020, 0x0020, 0xFFE0, 0xFFE0, 0x0020, 0x0020, 0x001F}, //0x0054 T
	{0x7FE0, 0xFFE0, 0x8000, 0x8000, 0x8000, 0xFFE0, 0x7FFF}, //0x0055 U
	{0x1FE0, 0x3FE0, 0x6000, 0xC000, 0x6000, 0x3FE0, 0x1FFF}, //0x0056 V
	{0x1FE0, 0xFFE0, 0xE000, 0x1C00, 0xE000, 0xFFE0, 0x1FFF}, //0x0057 W
	{0xC060, 0xF1E0, 0x3F80, 0x0E00, 0x3F80, 0xF1E0, 0xC07F}, //0x0058 X
	{0x03E0, 0x07E0, 0xFC00, 0xFC00, 0x07E0, 0x03E0, 0x001F}, //0x0059 Y
	{0xC020, 0xF020, 0xBC20, 0x8F20, 0x83A0, 0x80E0, 0x807F}, //0x005A Z
	{0x0000, 0xFFE0, 0xFFE0, 0x8020, 0x8020, 0x8020, 0x001F}, //0x005B [
	{0x0020, 0x00E0, 0x03C0, 0x0F00, 0x3C00, 0xF000, 0xC01F}, //0x005C '\'
	{0x0000, 0x8020, 0x8020, 0x8020, 0xFFE0, 0xFFE0, 0x001F}, //0x005D ]
	{0x0080, 0x00C0, 0x0060, 0x0060, 0x00C0, 0x0080, 0x001F}, //0x005E ^
	{0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x801F}, //0x005F _
	{0x0000, 0x0000, 0x0020, 0x0060, 0x00E0, 0x0080, 0x001F}, //0x0060 `
	{0x7000, 0xF900, 0x8900, 0x8900, 0x8900, 0xFF00, 0xFE1F}, //0x0061 a
	{0xFFE0, 0xFFE0, 0x8100, 0x8100, 0x8100, 0xFF00, 0x7E1F}, //0x0062 b
	{0x7E00, 0xFF00, 0x8100, 0x8100, 0x8100, 0xC300, 0x421F}, //0x0063 c
	{0x7E00, 0xFF00, 0x8100, 0x8100, 0x8100, 0xFFE0, 0xFFFF}, //0x0064 d
	{0x7E00, 0xFF00, 0x9100, 0x9100, 0x9100, 0x9F00, 0x5E1F}, //0x0065 e
	{0x0400, 0xFFC0, 0xFFE0, 0x0420, 0x0420, 0x0420, 0x001F}, //0x0066 f
	{0x9E00, 0xBF00, 0xA100, 0xA100, 0xA100, 0xFF00, 0x7F1F}, //0x0067 g
	{0xFFE0, 0xFFE0, 0x0100, 0x0100, 0x0100, 0xFF00, 0xFE1F}, //0x0068 h
	{0x8100, 0x8100, 0xFF60, 0xFF60, 0x8000, 0x8000, 0x001F}, //0x0069 i
	{0x8000, 0x8100, 0x8100, 0x8100, 0xFF60, 0x7F60, 0x001F}, //0x006A j
	{0xFFE0, 0xFFE0, 0x1800, 0x3C00, 0x6600, 0xC300, 0xC11F}, //0x006B k
	{0x8020, 0x8020, 0xFFE0, 0xFFE0, 0x8000, 0x8000, 0x001F}, //0x006C l
	{0xFF00, 0xFF00, 0x0100, 0x3F00, 0x0100, 0xFF00, 0xFE1F}, //0x006D m
	{0xFF00, 0xFF00, 0x0100, 0x0100, 0x0100, 0xFF00, 0xFE1F}, //0x006E n
	{0x7E00, 0xFF00, 0x8100, 0x8100, 0x8100, 0xFF00, 0x7E1F}, //0x006F o
	{0xFF00, 0xFF00, 0x2100, 0x2100, 0x2100, 0x3F00, 0x1E1F}, //0x0070 p
	{0x1E00, 0x3F00, 0x2100, 0x2100, 0x2100, 0xFF00, 0xFE1F}, //0x0071 q
	{0x0100, 0xFF00, 0xFF00, 0x0400, 0x0200, 0x0300, 0x031F}, //0x0072 r
	{0x8E00, 0x9F00, 0x9900, 0x9900, 0x9900, 0xF900, 0x711F}, //0x0073 s
	{0x0100, 0x0100, 0x7FC0, 0xFFC0, 0x8100, 0x8100, 0x811F}, //0x0074 t
	{0x7F00, 0xFF00, 0x8000, 0x8000, 0x8000, 0xFF00, 0xFF1F}, //0x0075 u
	{0x1F00, 0x3F00, 0x6000, 0xC000, 0x6000, 0x3F00, 0x1F1F}, //0x0076 v
	{0x3F00, 0xFF00, 0xC000, 0x3C00, 0xC000, 0xFF00, 0x3F1F}, //0x0077 w
	{0xE300, 0xF700, 0x1C00, 0x1C00, 0x1C00, 0xF700, 0xE31F}, //0x0078 x
	{0x8F00, 0x9F00, 0x9000, 0x9000, 0x9000, 0xFF00, 0x7F1F}, //0x0079 y
	{0xC100, 0xE100, 0xB100, 0x9900, 0x8D00, 0x8700, 0x831F}, //0x007A z
	{0x0000, 0x0400, 0x7FC0, 0xE0E0, 0x8020, 0x8000, 0x001F}, //0x007B {
	{0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x001F}, //0x007C |
	{0x0000, 0x8020, 0xE0E0, 0x7FC0, 0x0400, 0x0000, 0x005F}, //0x007D }
	{0x0060, 0x0020, 0x0060, 0x00C0, 0x0080, 0x00C0, 0x005F}};//0x007E ~
	
//details on pxldata table:
//0 to 94 (character start index, 32d to 126d ascii)
//95 to 189 (bytes per character)
//190 to 558 (character graphics bytes)

const unsigned int pxldata[559]={190, 191, 192, 195, 201, 205, 211, 215, 216, 218, 220, 223, 228, 230, 233, 234, 237, 241, 243, 247, 251,
256, 260, 264, 268, 272, 276, 277, 279, 283, 287, 291, 295, 303, 310, 314, 319, 324, 328, 332, 337, 342,
343, 347, 352, 356, 363, 368, 373, 378, 383, 388, 392, 397, 402, 409, 418, 424, 429, 433, 435, 438, 440,
445, 450, 452, 456, 460, 464, 468, 472, 474, 478, 482, 483, 484, 488, 489, 496, 500, 504, 508, 512, 514,
518, 520, 524, 529, 536, 540, 544, 548, 551, 552, 555, 1, 1, 3, 6, 4, 6, 4, 1, 2, 2, 3, 5, 2, 3, 1, 3, 4,
2, 4, 4, 5, 4, 4, 4, 4, 4, 1, 2, 4, 4, 4, 4, 8, 7, 4, 5, 5, 4, 4, 5, 5, 1, 4, 5, 4, 7, 5, 5, 5, 5, 5, 4,
5, 5, 7, 9, 6, 5, 4, 2, 3, 2, 5, 5, 2, 4, 4, 4, 4, 4, 2, 4, 4, 1, 1, 4, 1, 7, 4, 4, 4, 4, 2, 4, 2, 4, 5,
7, 4, 4, 4, 3, 1, 3, 4, 0, 95, 3, 0, 3, 20, 124, 23, 116, 31, 20, 36, 122, 47, 18, 66, 37, 26, 36, 82, 33,
54, 73, 118, 80, 3, 126, 129, 129, 126, 5, 2, 5, 4, 4, 31, 4, 4, 64, 32, 8, 8, 8, 64, 96, 28, 3, 62, 65,
65, 62, 2, 127, 98, 81, 73, 70, 34, 65, 73, 54, 24, 20, 18, 127, 16, 39, 69, 69, 57, 62, 73, 73, 50, 1, 97,
29, 3, 54, 73, 73, 54, 38, 73, 73, 62, 34, 64, 34, 8, 20, 20, 34, 20, 20, 20, 20, 34, 20, 20, 8, 2, 1, 89,
6, 60, 66, 153, 165, 165, 189, 162, 28, 96, 24, 22, 17, 22, 24, 96, 127, 73, 73, 54, 62, 65, 65, 65, 34,
127, 65, 65, 34, 28, 127, 73, 73, 73, 127, 9, 9, 9, 62, 65, 65, 73, 122, 127, 8, 8, 8, 127, 127, 32, 64,
64, 63, 127, 8, 20, 34, 65, 127, 64, 64, 64, 127, 4, 24, 32, 24, 4, 127, 127, 6, 8, 48, 127, 62, 65, 65,
65, 62, 127, 9, 9, 9, 6, 62, 65, 65, 97, 190, 127, 9, 9, 25, 102, 38, 73, 73, 50, 1, 1, 127, 1, 1, 63, 64,
64, 64, 63, 3, 12, 48, 64, 48, 12, 3, 3, 12, 112, 12, 3, 12, 112, 12, 3, 65, 34, 28, 28, 34, 65, 3, 4, 120,
4, 3, 97, 89, 69, 67, 255, 129, 3, 28, 96, 129, 255, 4, 2, 1, 2, 4, 64, 64, 64, 64, 64, 1, 2, 36, 84, 84,
120, 127, 68, 68, 56, 56, 68, 68, 40, 56, 68, 68, 127, 56, 84, 84, 88, 126, 5, 156, 162, 162, 126, 127, 4,
4, 120, 125, 253, 127, 16, 40, 68, 127, 124, 4, 4, 124, 4, 4, 120, 124, 4, 4, 120, 56, 68, 68, 56, 254, 34,
34, 28, 28, 34, 34, 254, 124, 4, 72, 84, 84, 36, 126, 68, 60, 64, 64, 124, 12, 48, 64, 48, 12, 28, 96, 16,
12, 16, 96, 28, 108, 16, 16, 108, 142, 112, 16, 14, 100, 84, 84, 76, 8, 54, 65, 255, 65, 54, 8, 2, 1, 2, 1};
	
//0 to 26 character start index
//27 to 53 words per character
//54 to 204 words per character
const unsigned int hebrewfont[205]={0x36, 0x3C, 0x43, 0x49, 0x4E, 0x54, 0x57, 0x5A, 0x60, 0x66, 0x69, 0x6E, 0x74, 0x7A, 0x80, 0x87, 0x8B, 0x8E, 0x94, 0x9A, 0x9F, 0xA5, 0xAB, 0xB1, 0xB7, 0xBD, 0xC5, 0x06, 0x07, 0x06, 0x05, 0x06, 0x03, 0x03, 0x06, 0x06, 0x03, 0x05, 0x06, 0x06, 0x06, 0x07, 0x04, 0x03, 0x06, 0x06, 0x05, 0x06, 0x06, 0x06, 0x06, 0x06, 0x08, 0x08, 0x1738, 0x18E0, 0x1C0, 0x398, 0x6F0, 0x1C30, 0x1838, 0x1830, 0x1830, 0x1830, 0x1C30, 0x1BE0, 0x1800, 0x1800, 0x1838, 0x1830, 0x430, 0x7F0, 0x1C00, 0x38, 0x30, 0x30, 0x1FF0, 0x30, 0x1E38, 0x30, 0x30, 0x30, 0x30, 0x1FF0, 0x38, 0x30, 0x1FF0, 0x38, 0x1FF0, 0x30, 0x1FF8, 0x30, 0x30, 0x30, 0x30, 0x1FF0, 0x38, 0x7F0, 0x1800, 0x1860, 0x1830, 0x7F0, 0x38, 0x130, 0xF0, 0x1838, 0x1830, 0x1830, 0x1830, 0xFE0, 0x38, 0x30, 0x30, 0x30, 0x30, 0xFFF0, 0x01, 0x3F, 0x30, 0x1C30, 0x230, 0x1F0, 0x1E38, 0x1B0, 0x1840, 0x1820, 0x1830, 0x7E0, 0x1FB8, 0x1870, 0x1830, 0x1830, 0x1830, 0x1830, 0x1FE0, 0x1800, 0x1838, 0x1830, 0x1FF0, 0x38, 0x30, 0xFFF0, 0xFB8, 0x1870, 0x1830, 0x1830, 0x1830, 0xFE0, 0x1838, 0x19F0, 0x1F30, 0x800, 0x7B8, 0x70, 0x19F8, 0x1930, 0x1830, 0x1830, 0xFE0, 0x398, 0x370, 0x230, 0x30, 0x30, 0xFFE0, 0x1838, 0x19F0, 0x1B00, 0x1BB8, 0x1E70, 0x1C30, 0x38, 0xFFF0, 0x100, 0xB8, 0x70, 0x30, 0xFFB8, 0x30, 0x1830, 0x430, 0x230, 0x1F0, 0x38, 0x30, 0x30, 0x30, 0x30, 0x1FE0, 0x38, 0x7F0, 0x1C00, 0x1E38, 0x19F0, 0x1800, 0x1C38, 0x3F0, 0x1800, 0x1FB8, 0x70, 0x30, 0x30, 0x30, 0x30, 0x1FE0};

const unsigned int sevsegpxldata[30]=
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

void PixelTest(unsigned int colour, unsigned int delayMS)	//top to bottom, right to left
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

void LightUpPixel(unsigned int x, unsigned int y, unsigned int colour)
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

void LightUpLine(unsigned int y, unsigned int colour)
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

void LightUpAreaSegmented(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int colour)
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

void LightUpArea(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int colour)
{
	unsigned int tmp;
	
	x1=(LedMatrixWidth-1)-x1;
	x2=(LedMatrixWidth-1)-x2;
	
	//may sure p2 coords are below p1 coords
	if(x1>x2){ tmp=x1; x1=x2; x2=tmp; }
	if(y1>y2){ tmp=y1; y1=y2; y2=tmp; }

	if((y1 & 0x10)==(y2 & 0x10))
	{
		LightUpAreaSegmented(x1, y1, x2, y2, colour);
	}else
	{
		LightUpAreaSegmented(x1, y1, x2, 15, colour);
		LightUpAreaSegmented(x1, 16, x2, y2, colour);
	}
}

void LightLEDScreen(unsigned int colour)
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

void ClearLEDScreen(void)
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

void DispBitmap1bit1px(unsigned int bitmap[][32])	//n*32 pixel at a time
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

void DispBitmap1bit1Line(unsigned int bitmap[][32])	//1 line at a time
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

void DispBitmap1bit2Lines(unsigned int bitmap[][32])	//2 lines at a time
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


void DispBitmap4bit(unsigned int bitmap[][32])	//	//3 4-bit values 00000000 00000000 0000RRRR GGGG BBBB. 2 lines at a time with 4 bit pwm
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
void DispBitmap8bit(unsigned int bitmap[][32])
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


void DisplayBitmap(unsigned int bitmap[][32], unsigned int mode)
{
	switch(mode)
	{
		case DisplayMode1bit1px:
			DispBitmap1bit1px(bitmap);
		break;
		case DisplayMode1bit1Line:
			DispBitmap1bit1Line(bitmap);
		break;
		case DisplayMode1bit2Lines:
			DispBitmap1bit2Lines(bitmap);
		break;
		case DisplayMode4bit:
			DispBitmap4bit(bitmap);
		break;
		case DisplayMode8bit:
			DispBitmap8bit(bitmap);
		break;
	}	
}

void DisplayBitmapDuration(unsigned int bitmap[][32], unsigned int durationMS, unsigned int mode)
{
	durationMS*=10;
	ResetTimer14();
	while(ReadTimer14()<durationMS)
	{
		DisplayBitmap(bitmap,mode);
	}	
}

void BitmapTest(unsigned int mode)	//run this in a loop
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
	
	DisplayBitmap(bitmap,mode);
}

void BmpClear(unsigned int bitmap[][32], unsigned int colour)
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

void BmpCopy(unsigned int sourcebitmap[][32], unsigned int destbitmap[][32])
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

void BmpSetPixel(unsigned int bitmap[][32], int x, int y, unsigned int colour)	//1 or 4 bit format
{
	if((x>=0) && (x<=(LedMatrixWidth-1)) && (y>=0) && (y<=(LedMatrixHeight-1)))
	{
		bitmap[x][y]=colour;
	}
}

void BmpMix(unsigned int sourcebitmap[][32], unsigned int destbitmap[][32])
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

void BmpMultiply(unsigned int sourcebitmap[][32], unsigned int destbitmap[][32])
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

unsigned int BmpReadPixel(unsigned int bitmap[][32], int x, int y)
{
	if((x>=0) && (x<=(LedMatrixWidth-1)) && (y>=0) && (y<=31))
	{
		return bitmap[x][y];
	}else
	{
		return 0;
	}
}

void BmpMixPixel(unsigned int bitmap[][32], int x, int y, unsigned int colour)
{
	BmpSetPixel(bitmap,x,y,BmpReadPixel(bitmap,x,y) | colour);
}

void BmpReplaceColourWithColour(unsigned int bitmap[][32], unsigned int ColourToReplace, unsigned int ReplacementColour)
{
	int x, y;

	for(y=0;y<8;y++)
	{
		for(x=0;x<8;x++)
		{
			if(BmpReadPixel(bitmap,x,y)==ReplacementColour){ BmpSetPixel(bitmap,x,y,ColourToReplace); }
		}
	}
}

void BmpReplaceColourWithBmp(unsigned int bitmap[][32], unsigned int ColourToReplace, unsigned int backgroundbitmap[][32])
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

void BmpReplaceColourWithBmpIntoBmp(unsigned int bitmap[][32], unsigned int ColourToReplace, unsigned int backgroundbitmap[][32], unsigned int destbitmap[][32])
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

void BmpFadeDown(unsigned int bitmap[][32], unsigned int delayMS)
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
		DisplayBitmapDuration(bitmap, delayMS, DisplayMode8bit);
	}while(changed==1);
}

void BmpFadeUp(unsigned int bitmap[][32], unsigned int delayMS)
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
		DisplayBitmapDuration(bmp, delayMS, DisplayMode8bit);
	}while(changed==1);
}

void BmpFadeBetween(unsigned int bitmap1[][32], unsigned int bitmap2[][32])
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
		DisplayBitmap(bmp, DisplayMode8bit);
	}
}

void BmpLine(unsigned int bitmap[][32], int x1, int y1, int x2, int y2,  unsigned int colour)
{
	int x, y, xdelta, ydelta, width, height, i, count;
	
	if(y1==y2)
	{
		if((y1>=0) && (y1<=31))
		{
			if(x1>x2){ x=x2; x2=x1; x1=x; }
			for(x=x1;x<=x2;x++)
			{
				BmpSetPixel(bitmap,x,y1,colour);
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
				BmpSetPixel(bitmap,x1,y,colour);
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
		BmpSetPixel(bitmap,x>>16,y>>16,colour);
		x += xdelta;
		y += ydelta;
	}	
}

void BmpRectangle(unsigned int bitmap[][32], int x1, int y1, int x2, int y2,  unsigned int colour)
{
	int tmp;
	int x;
	int y;
	
	if(x1>x2){ tmp=x2; x2=x1; x1=tmp; }
	if(y1>y2){ tmp=y2; y2=y1; y1=tmp; }

	for(x=x1;x<=x2;x++)
	{
		BmpSetPixel(bitmap,x,y1,colour);
		BmpSetPixel(bitmap,x,y2,colour);
	}
	for(y=y1;y<=y2;y++)
	{
		BmpSetPixel(bitmap,x1,y,colour);
		BmpSetPixel(bitmap,x2,y,colour);
	}
}

void BmpRectangleFilled(unsigned int bitmap[][32], int x1, int y1, int x2, int y2,  unsigned int colour)
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
			BmpSetPixel(bitmap,x,y,colour);
		}
	}
}

void BmpCircle(unsigned int bitmap[][32], int x0, int y0, int radius, unsigned int colour)
{
	int f = 1 - radius;
	int ddF_x = 1;
	int ddF_y = -2 * radius;
	int x = 0;
	int y = radius;
	
	BmpSetPixel(bitmap,x0, y0 + radius,colour);
	BmpSetPixel(bitmap,x0, y0 - radius,colour);
	BmpSetPixel(bitmap,x0 + radius, y0,colour);
	BmpSetPixel(bitmap,x0 - radius, y0,colour);
	
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
		    
		BmpSetPixel(bitmap,x0 + x, y0 + y,colour);	//segment 4
		BmpSetPixel(bitmap,x0 - x, y0 + y,colour);	//segment 5
		BmpSetPixel(bitmap,x0 + x, y0 - y,colour);	//segment 1
		BmpSetPixel(bitmap,x0 - x, y0 - y,colour);	//segment 8
		BmpSetPixel(bitmap,x0 + y, y0 + x,colour);	//segment 3
		BmpSetPixel(bitmap,x0 - y, y0 + x,colour);	//segment 6
		BmpSetPixel(bitmap,x0 + y, y0 - x,colour);	//segment 2
		BmpSetPixel(bitmap,x0 - y, y0 - x,colour);	//segment 7
	}
}

void BmpCircleFilled(unsigned int bitmap[][32], int x0, int y0, int radius, unsigned int colour)
{
	int f = 1 - radius;
	int ddF_x = 1;
	int ddF_y = -2 * radius;
	int x = 0;
	int y = radius;
	
	BmpLine(bitmap, x0 - radius,y0,x0 + radius+1,y0,colour);
	
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
		    
		BmpLine(bitmap, x0 + x, y0 - y,x0 - x, y0 - y,colour);	//segment 1 to 8
		BmpLine(bitmap, x0 + y, y0 - x,x0 - y, y0 - x,colour);	//segment 2 to 7
		BmpLine(bitmap, x0 + y, y0 + x,x0 - y, y0 + x,colour);	//segment 3 to 6
		BmpLine(bitmap, x0 + x, y0 + y,x0 - x, y0 + y,colour);	//segment 4 to 5
	}
}

void BmpInvertColours(unsigned int bitmap[][32])
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

void BmpShiftLeft(unsigned int bitmap[][32], int shift)
{
	int x;
	int y;

	for(y=0;y<32;y++)
	{
		for(x=0;x<LedMatrixWidth;x++)
		{
			BmpSetPixel(bitmap,x,y,BmpReadPixel(bitmap,x+shift,y));
		}
	}
}

void BmpShiftRight(unsigned int bitmap[][32], int shift)
{
	int x;
	int y;

	for(y=0;y<32;y++)
	{
		for(x=(LedMatrixWidth-1);x>=0;x--)
		{
			BmpSetPixel(bitmap,x,y,BmpReadPixel(bitmap,x-shift,y));
		}
	}
}

void BmpShiftUp(unsigned int bitmap[][32], int shift)
{
	int x;
	int y;

	for(x=0;x<LedMatrixWidth;x++)
	{
		for(y=0;y<32;y++)
		{
			BmpSetPixel(bitmap,x,y,BmpReadPixel(bitmap,x,y+shift));
		}
	}
}

void BmpShiftDown(unsigned int bitmap[][32], int shift)
{
	int x;
	int y;

	for(x=0;x<LedMatrixWidth;x++)
	{
		for(y=31;y>=0;y--)
		{
			BmpSetPixel(bitmap,x,y,BmpReadPixel(bitmap,x,y-shift));
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
			BmpSetPixel(destbitmap,rint(xnew + midx),rint(ynew + midy),BmpReadPixel(sourcebitmap,x,y));
	  }
	}
}

unsigned int BmpCharacter(unsigned int bitmap[][32], char character, int x1, int y1, unsigned int colour, unsigned int font)
{
	int x;
	int y;
	int bytenum;

	if(font==LedFontVariableWidth)
	{
		x=0;
		for(bytenum=pxldata[character-' '];bytenum<(pxldata[character-' ']+pxldata[95+(character-' ')]);bytenum++)
		{
			for(y=0;y<8;y++)	//add byte
			{
				if(((pxldata[bytenum]>>y) & 1)==1){ BmpSetPixel(bitmap,x+x1,y+y1,colour); }
			}
			x+=1;
		}
	}
	
	if(font==LedFont7x11)
	{
		for(x=0;x<7;x++)
		{
			for(y=5;y<16;y++)
			{
				if((LEDascii_7x11[character-0x20][x]>>y) & 1)
				{
					BmpSetPixel(bitmap,x+x1,y+y1-5,colour);
				}
			}
		}
	}
	
	return x1+x+1;
}

unsigned int BmpGetCharacterLength(char character)
{
	return pxldata[95+(character-' ')];
}

void BmpShiftInCharacter(unsigned int bitmap[][32], char character, int xRight, int yTop, unsigned int fps, unsigned int colour, unsigned int font, unsigned int DisplayMode, unsigned int flipAxis)
{
	unsigned int x, y;
	unsigned int bytenum, mS;
	
	mS=1000/fps;

	if(font==LedFontVariableWidth)
	{
		for(bytenum=pxldata[character-' '];bytenum<(pxldata[character-' ']+pxldata[95+(character-' ')]);bytenum++)
		{
			ResetTimer14();
			for(y=0;y<8;y++)	//add byte
			{
				if(((pxldata[bytenum]>>y) & 1)==1){ BmpSetPixel(bitmap,xRight,yTop+y,colour); }
			}
			if(flipAxis!=LEDNoFlip){ BmpFlip(bitmap,flipAxis); }	//flip
			DisplayBitmapDuration(bitmap, mS, DisplayMode);		//display frame
			if(flipAxis!=LEDNoFlip){ BmpFlip(bitmap,flipAxis); }	//unflip
			BmpShiftLeft(bitmap,1);	//scroll
		}
	}
	
	if(font==LedFont7x11)
	{
		for(x=0;x<7;x++)
		{
			for(y=5;y<16;y++)
			{
				if((LEDascii_7x11[character-0x20][x]>>y) & 1)
				{
					BmpSetPixel(bitmap,xRight,yTop+y-5,colour);
				}
			}
			if(flipAxis!=LEDNoFlip){ BmpFlip(bitmap,flipAxis); }	//flip
			DisplayBitmapDuration(bitmap, mS, DisplayMode);		//display frame
			if(flipAxis!=LEDNoFlip){ BmpFlip(bitmap,flipAxis); }	//unflip
			BmpShiftLeft(bitmap,1);	//scroll
		}
	}

	if(flipAxis!=LEDNoFlip){ BmpFlip(bitmap,flipAxis); }	//flip
	DisplayBitmapDuration(bitmap, mS, DisplayMode);		//display frame
	if(flipAxis!=LEDNoFlip){ BmpFlip(bitmap,flipAxis); }	//unflip
	BmpShiftLeft(bitmap,1);	//extra 1px space after the character
}

//0 to 26 character start index
//27 to 53 words per character
//54 to 204 words per character
void BmpShiftInCharacterHebrew(unsigned int bitmap[][32], char character, int xLeft, int yTop, unsigned int fps, unsigned int colour, unsigned int DisplayMode, unsigned int flipAxis)
{
	unsigned int y, mS, i;
	int wordnum;
	
	mS=1000/fps;

	if(character!=' ')
	{
		for(wordnum=hebrewfont[character-1]+hebrewfont[27+(character-1)]-1;wordnum>=hebrewfont[character-1];wordnum--)
		{
			ResetTimer14();
			for(y=0;y<16;y++)	//add word
			{
				if(((hebrewfont[wordnum]>>y) & 1)==1){ BmpSetPixel(bitmap,xLeft,yTop+y,colour); }
			}
			if(flipAxis!=LEDNoFlip){ BmpFlip(bitmap,flipAxis); }	//flip
			DisplayBitmapDuration(bitmap, mS, DisplayMode);		//display frame
			if(flipAxis!=LEDNoFlip){ BmpFlip(bitmap,flipAxis); }	//unflip
			BmpShiftRight(bitmap,1);	//scroll
		}
	}else
	{
			for(i=0;i<5;i++)
			{
				ResetTimer14();
				if(flipAxis!=LEDNoFlip){ BmpFlip(bitmap,flipAxis); }	//flip
				DisplayBitmapDuration(bitmap, mS, DisplayMode);		//display frame
				if(flipAxis!=LEDNoFlip){ BmpFlip(bitmap,flipAxis); }	//unflip
				BmpShiftRight(bitmap,1);	//scroll
			}
	}

	if(flipAxis!=LEDNoFlip){ BmpFlip(bitmap,flipAxis); }	//flip
	DisplayBitmapDuration(bitmap, mS, DisplayMode);		//display frame
	if(flipAxis!=LEDNoFlip){ BmpFlip(bitmap,flipAxis); }	//unflip
	BmpShiftRight(bitmap,1);	//extra 1px space after the character
}

void BmpShiftInCharacterWithBG(unsigned int bitmap[][32], unsigned int bitmapBG[][32], char character, int xRight, int yTop, unsigned int fps, unsigned int colour, unsigned int font, unsigned int DisplayMode, unsigned int flipAxis)
{
	unsigned int x, y;
	unsigned int bytenum, mS;
	unsigned int outputbitmap[32][32];
	
	mS=1000/fps;

	if(font==LedFontVariableWidth)
	{
		for(bytenum=pxldata[character-' '];bytenum<(pxldata[character-' ']+pxldata[95+(character-' ')]);bytenum++)
		{
			ResetTimer14();
			for(y=0;y<8;y++)	//add byte
			{
				if(((pxldata[bytenum]>>y) & 1)==1){ BmpSetPixel(bitmap,xRight,yTop+y,colour); }
			}
			
			BmpReplaceColourWithBmpIntoBmp(bitmap,0,bitmapBG,outputbitmap);
			if(flipAxis!=LEDNoFlip){ BmpFlip(bitmap,flipAxis); }	//flip
			DisplayBitmapDuration(outputbitmap, mS, DisplayMode);		//display frame
			if(flipAxis!=LEDNoFlip){ BmpFlip(bitmap,flipAxis); }	//unflip
			BmpShiftLeft(bitmap,1);	//scroll
		}
	}
	
	if(font==LedFont7x11)
	{
		for(x=0;x<7;x++)
		{
			for(y=5;y<16;y++)
			{
				if((LEDascii_7x11[character-0x20][x]>>y) & 1)
				{
					BmpSetPixel(bitmap,xRight,yTop+y-5,colour);
				}
			}
			BmpReplaceColourWithBmpIntoBmp(bitmap,0,bitmapBG,outputbitmap);
			if(flipAxis!=LEDNoFlip){ BmpFlip(bitmap,flipAxis); }	//flip
			DisplayBitmapDuration(outputbitmap, mS, DisplayMode);		//display frame
			if(flipAxis!=LEDNoFlip){ BmpFlip(bitmap,flipAxis); }	//unflip
			BmpShiftLeft(bitmap,1);	//scroll
		}
	}

	BmpReplaceColourWithBmpIntoBmp(bitmap,0,bitmapBG,outputbitmap);
	if(flipAxis!=LEDNoFlip){ BmpFlip(bitmap,flipAxis); }	//flip
	DisplayBitmapDuration(outputbitmap, mS, DisplayMode);		//display frame
	if(flipAxis!=LEDNoFlip){ BmpFlip(bitmap,flipAxis); }	//unflip
	BmpShiftLeft(bitmap,1);	//extra 1px space after the character
}

void BmpScrollText(unsigned int bitmap[][32], char characters[200], int xRight, int yTop, unsigned int fps, unsigned int colour, unsigned int font, unsigned int DisplayMode, unsigned int flipAxis)
{
	unsigned int i=0;

	while(characters[i]!=0)
	{
		BmpShiftInCharacter(bitmap, characters[i], xRight, yTop, fps, colour, font, DisplayMode, flipAxis);
		i++;
	}
}

void BmpScrollTextHebrew(unsigned int bitmap[][32], char characters[200], int xLeft, int yTop, unsigned int fps, unsigned int colour, unsigned int DisplayMode, unsigned int flipAxis)
{
	unsigned int i=0;

	while(characters[i]!=0)
	{
		BmpShiftInCharacterHebrew(bitmap, characters[i], xLeft, yTop, fps, colour, DisplayMode, flipAxis);
		i++;
	}
}

void BmpScrollTextWithBG(unsigned int bitmap[][32], unsigned int bitmapBG[][32], char characters[200], int xRight, int yTop, unsigned int fps, unsigned int colour, unsigned int font, unsigned int DisplayMode, unsigned int flipAxis)
{
	unsigned int i=0;

	while(characters[i]!=0)
	{
		BmpShiftInCharacterWithBG(bitmap, bitmapBG, characters[i], xRight, yTop, fps, colour, font, DisplayMode, flipAxis);
		i++;
	}
}

void BmpScrollValue(unsigned int bitmap[][32], unsigned int val, int xRight, int yTop, unsigned int fps, unsigned int colour, unsigned int font, unsigned int DisplayMode, unsigned int flipAxis)
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
		if(print==1){ BmpShiftInCharacter(bitmap, digitval+'0', xRight, yTop, fps, colour, font, DisplayMode, flipAxis); }
		unit /= 10;
	}
}

void BmpScrollValueWithBG(unsigned int bitmap[][32], unsigned int bitmapBG[][32], unsigned int val, int xRight, int yTop, unsigned int fps, unsigned int colour, unsigned int font, unsigned int DisplayMode, unsigned int flipAxis)
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
		if(print==1){ BmpShiftInCharacterWithBG(bitmap, bitmapBG, digitval+'0', xRight, yTop, fps, colour, font, DisplayMode, flipAxis); }
		unit /= 10;
	}
}

void BmpSevSegDigit(unsigned int bitmap[][32], unsigned int val, int xLeft, int yTop, unsigned int colour)
{
	int x;
	int y;
	unsigned int segments;
	unsigned int digits[10]={0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
	
	segments=digits[val];
	
	if(segments & 1)		//seg A
	{
		for(x=xLeft;x<=(xLeft+2);x++){ BmpSetPixel(bitmap,x,yTop,colour); }
	}

	if(segments & 2)		//seg B
	{
		for(y=yTop;y<=(yTop+2);y++){ BmpSetPixel(bitmap,xLeft+2,y,colour); }
	}

	if(segments & 4)		//seg C
	{
		for(y=yTop+2;y<=(yTop+4);y++){ BmpSetPixel(bitmap,xLeft+2,y,colour); }
	}

	if(segments & 8)		//seg D
	{
		for(x=xLeft;x<=(xLeft+2);x++){ BmpSetPixel(bitmap,x,yTop+4,colour); }
	}

	if(segments & 16)		//seg E
	{
		for(y=yTop+2;y<=(yTop+4);y++){ BmpSetPixel(bitmap,xLeft,y,colour); }
	}

	if(segments & 32)		//seg F
	{
		for(y=yTop;y<=(yTop+2);y++){ BmpSetPixel(bitmap,xLeft,y,colour); }
	}

	if(segments & 64)		//seg G
	{
		for(x=xLeft;x<=(xLeft+2);x++){ BmpSetPixel(bitmap,x,yTop+2,colour); }
	}
}

unsigned int BmpSevSegSegMultiDigits(unsigned int bitmap[][32], unsigned int val, int xLeft, int yTop, unsigned int ForceDigits, unsigned int colour)
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
		if(print==1){ BmpSevSegDigit(bitmap, digitval, xLeft+x1, yTop, colour); x1+=4; }
		unit /= 10;
	}
	
	return xLeft+x1;
}

void BmpSevSegAMPM(unsigned int bitmap[][32], unsigned int AMPM, int xLeft, int yTop, unsigned int DispM, unsigned int colour)
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
			for(x=xLeft;x<=(xLeft+2);x++){ BmpSetPixel(bitmap,x,yTop,colour); }
		}

		if(digits[strAMPM[i]] & 2)		//seg B
		{
			for(y=yTop;y<=(yTop+2);y++){ BmpSetPixel(bitmap,xLeft+2,y,colour); }
		}

		if(digits[strAMPM[i]] & 4)		//seg C
		{
			for(y=yTop+2;y<=(yTop+4);y++){ BmpSetPixel(bitmap,xLeft+2,y,colour); }
		}

		if(digits[strAMPM[i]] & 8)		//seg D
		{
			for(x=xLeft;x<=(xLeft+2);x++){ BmpSetPixel(bitmap,x,yTop+4,colour); }
		}

		if(digits[strAMPM[i]] & 16)		//seg E
		{
			for(y=yTop+2;y<=(yTop+4);y++){ BmpSetPixel(bitmap,xLeft,y,colour); }
		}

		if(digits[strAMPM[i]] & 32)		//seg F
		{
			for(y=yTop;y<=(yTop+2);y++){ BmpSetPixel(bitmap,xLeft,y,colour); }
		}

		if(digits[strAMPM[i]] & 64)		//seg G
		{
			for(x=xLeft;x<=(xLeft+2);x++){ BmpSetPixel(bitmap,x,yTop+2,colour); }
		}
		
		if(DispM==0){ break; }
		
		if(i==1){ BmpSetPixel(bitmap,xLeft+1,yTop+1,colour); } //if M, add the middle dot
		
		xLeft+=4;
	}
}

void BmpShiftInDigitSevSeg(unsigned int bitmap[][32], unsigned int val, int xRight, int yTop, unsigned int fps, unsigned int colour, unsigned int DisplayMode, unsigned int flipAxis)
{
	unsigned int y;
	unsigned int bytenum;
	
	for(bytenum=val*3;bytenum<=((val*3)+2);bytenum++)
	{
		ResetTimer14();
		for(y=0;y<5;y++)	//add byte
		{
			if(((sevsegpxldata[bytenum]>>y) & 1)==1){ BmpSetPixel(bitmap,xRight,y+yTop,colour); }
		}
		if(flipAxis!=LEDNoFlip){ BmpFlip(bitmap,flipAxis); }	//flip
		DisplayBitmapDuration(bitmap, 1000/fps, DisplayMode);		//display frame
		if(flipAxis!=LEDNoFlip){ BmpFlip(bitmap,flipAxis); }	//unflip
		BmpShiftLeft(bitmap,1);	//scroll
	}

	ResetTimer14();
	if(flipAxis!=LEDNoFlip){ BmpFlip(bitmap,flipAxis); }	//flip
	DisplayBitmapDuration(bitmap, 1000/fps, DisplayMode);		//display frame
	if(flipAxis!=LEDNoFlip){ BmpFlip(bitmap,flipAxis); }	//unflip
	BmpShiftLeft(bitmap,1);	//extra 1px space after the character
}

void BmpShiftInDigitSevSegWithBG(unsigned int bitmap[][32], unsigned int bitmapBG[][32], unsigned int val, int xRight, int yTop, unsigned int fps, unsigned int colour, unsigned int DisplayMode, unsigned int flipAxis)
{
	unsigned int y;
	unsigned int bytenum;
	unsigned int outputbitmap[32][32];
	
	for(bytenum=val*3;bytenum<=((val*3)+2);bytenum++)
	{
		ResetTimer14();
		for(y=0;y<5;y++)	//add byte
		{
			if(((sevsegpxldata[bytenum]>>y) & 1)==1){ BmpSetPixel(bitmap,xRight,y+yTop,colour); }
		}
		BmpReplaceColourWithBmpIntoBmp(bitmap,0,bitmapBG,outputbitmap);
		if(flipAxis!=LEDNoFlip){ BmpFlip(bitmap,flipAxis); }	//flip
		DisplayBitmapDuration(bitmap, 1000/fps, DisplayMode);		//display frame
		if(flipAxis!=LEDNoFlip){ BmpFlip(bitmap,flipAxis); }	//unflip
		BmpShiftLeft(bitmap,1);	//scroll
	}

	ResetTimer14();
	if(flipAxis!=LEDNoFlip){ BmpFlip(bitmap,flipAxis); }	//flip
	DisplayBitmapDuration(bitmap, 1000/fps, DisplayMode);		//display frame
	if(flipAxis!=LEDNoFlip){ BmpFlip(bitmap,flipAxis); }	//unflip
	BmpShiftLeft(bitmap,1);	//extra 1px space after the character
}

void BmpScrollValueSevSeg(unsigned int bitmap[][32], unsigned int val, int xRight, int yTop, unsigned int fps, unsigned int colour, unsigned int DisplayMode, unsigned int flipAxis)
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
		if(print==1){ BmpShiftInDigitSevSeg(bitmap, digitval, xRight, yTop, fps, colour, DisplayMode, flipAxis); }
		unit /= 10;
	}
}

void BmpScrollValueSevSegWithBG(unsigned int bitmap[][32], unsigned int bitmapBG[][32], unsigned int val, int xRight, int yTop, unsigned int fps, unsigned int colour, unsigned int DisplayMode, unsigned int flipAxis)
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
		if(print==1){ BmpShiftInDigitSevSegWithBG(bitmap, bitmapBG, digitval, xRight, yTop, fps, colour, DisplayMode, flipAxis); }
		unit /= 10;
	}
}

void DrawSegmentLarge(unsigned int bitmap[][32], unsigned int x, unsigned int y, unsigned int length, unsigned int width, unsigned int type, unsigned int colour)
{
	int i;
	
	switch(type)
	{
		case 0:	//left
			for(i=0;i<width;i++)
			{
				BmpLine(bitmap, x+i,y+i,x+i,y+(length-1)-i,colour);
			}
		break;
		case 1:	//right
			for(i=0;i<width;i++)
			{
				BmpLine(bitmap, x+((width-1)-i),y+i,x+((width-1)-i),y+(length-1)-i,colour);
			}
		break;
		case 2:	//top
			for(i=0;i<width;i++)
			{
				BmpLine(bitmap, x+i,y+i,x+(length-1)-i,y+i,colour);
			}
		break;
		case 3:	//bottom
			for(i=0;i<width;i++)
			{
				BmpLine(bitmap, x+((width-1)-i),y+i,x+(length-1)-((width-1)-i),y+i,colour);
			}
		break;
		case 4:	//middle
			width>>=1;
			for(i=0;i<width;i++)
			{
				BmpLine(bitmap, x+((width-1)-i),y+i,x+(length-1)-((width-1)-i),y+i,colour);
			}
			y+=width-1;
			for(i=1;i<width;i++)
			{
				BmpLine(bitmap, x+i,y+i,x+(length-1)-i,y+i,colour);
			}
		break;
	}
}


unsigned int SevSegDigitLarge(unsigned int bitmap[][32], unsigned int digit, unsigned int x, unsigned int y, unsigned int height, unsigned int thickness, unsigned int gap, unsigned int colour)
{
	unsigned int length;

	length=(height>>1)-(gap>>1);
	
	switch(digit)
	{
		case 0:
			DrawSegmentLarge(bitmap,x+gap, y, length, thickness, 2, colour);	//top
			DrawSegmentLarge(bitmap,x, y+gap, length, thickness, 0, colour);	//left top
			DrawSegmentLarge(bitmap,x, y+length+(gap<<1), length, thickness, 0, colour);	//left bottom
			DrawSegmentLarge(bitmap,x+gap, y+(length<<1)+(gap<<1)+gap-thickness, length, thickness, 3, colour);	//bottom
			DrawSegmentLarge(bitmap,x+length+(gap<<1)-thickness, y+length+(gap<<1), length, thickness, 1, colour);	//right bottom
			DrawSegmentLarge(bitmap,x+length+(gap<<1)-thickness, y+gap, length, thickness, 1, colour);	//right top
		break;
		case 1:
			DrawSegmentLarge(bitmap,x+length+(gap<<1)-thickness, y+length+(gap<<1), length, thickness, 1, colour);	//right bottom
			DrawSegmentLarge(bitmap,x+length+(gap<<1)-thickness, y+gap, length, thickness, 1, colour);	//right top
		break;
		case 2:
			DrawSegmentLarge(bitmap,x+gap, y, length, thickness, 2, colour);	//top
			DrawSegmentLarge(bitmap,x, y+length+(gap<<1), length, thickness, 0, colour);	//left bottom
			DrawSegmentLarge(bitmap,x+gap, y+(length<<1)+(gap<<1)+gap-thickness, length, thickness, 3, colour);	//bottom
			DrawSegmentLarge(bitmap,x+length+(gap<<1)-thickness, y+gap, length, thickness, 1, colour);	//right top
			DrawSegmentLarge(bitmap,x+gap, y+length+gap+(gap>>1)-((unsigned int)(thickness*1.5)>>1), length, thickness*1.5, 4, colour);	//middle
		break;
		case 3:
			DrawSegmentLarge(bitmap,x+gap, y, length, thickness, 2, colour);	//top
			DrawSegmentLarge(bitmap,x+gap, y+(length<<1)+(gap<<1)+gap-thickness, length, thickness, 3, colour);	//bottom
			DrawSegmentLarge(bitmap,x+length+(gap<<1)-thickness, y+length+(gap<<1), length, thickness, 1, colour);	//right bottom
			DrawSegmentLarge(bitmap,x+length+(gap<<1)-thickness, y+gap, length, thickness, 1, colour);	//right top
			DrawSegmentLarge(bitmap,x+gap, y+length+gap+(gap>>1)-((unsigned int)(thickness*1.5)>>1), length, thickness*1.5, 4, colour);	//middle
		break;
		case 4:
			DrawSegmentLarge(bitmap,x, y+gap, length, thickness, 0, colour);	//left top
			DrawSegmentLarge(bitmap,x+length+(gap<<1)-thickness, y+length+(gap<<1), length, thickness, 1, colour);	//right bottom
			DrawSegmentLarge(bitmap,x+length+(gap<<1)-thickness, y+gap, length, thickness, 1, colour);	//right top
			DrawSegmentLarge(bitmap,x+gap, y+length+gap+(gap>>1)-((unsigned int)(thickness*1.5)>>1), length, thickness*1.5, 4, colour);	//middle
		break;
		case 5:
			DrawSegmentLarge(bitmap,x+gap, y, length, thickness, 2, colour);	//top
			DrawSegmentLarge(bitmap,x, y+gap, length, thickness, 0, colour);	//left top
			DrawSegmentLarge(bitmap,x+gap, y+(length<<1)+(gap<<1)+gap-thickness, length, thickness, 3, colour);	//bottom
			DrawSegmentLarge(bitmap,x+length+(gap<<1)-thickness, y+length+(gap<<1), length, thickness, 1, colour);	//right bottom
			DrawSegmentLarge(bitmap,x+gap, y+length+gap+(gap>>1)-((unsigned int)(thickness*1.5)>>1), length, thickness*1.5, 4, colour);	//middle
		break;
		case 6:
			DrawSegmentLarge(bitmap,x+gap, y, length, thickness, 2, colour);	//top
			DrawSegmentLarge(bitmap,x, y+gap, length, thickness, 0, colour);	//left top
			DrawSegmentLarge(bitmap,x, y+length+(gap<<1), length, thickness, 0, colour);	//left bottom
			DrawSegmentLarge(bitmap,x+gap, y+(length<<1)+(gap<<1)+gap-thickness, length, thickness, 3, colour);	//bottom
			DrawSegmentLarge(bitmap,x+length+(gap<<1)-thickness, y+length+(gap<<1), length, thickness, 1, colour);	//right bottom
			DrawSegmentLarge(bitmap,x+gap, y+length+gap+(gap>>1)-((unsigned int)(thickness*1.5)>>1), length, thickness*1.5, 4, colour);	//middle
		break;
		case 7:
			DrawSegmentLarge(bitmap,x+gap, y, length, thickness, 2, colour);	//top
			DrawSegmentLarge(bitmap,x+length+(gap<<1)-thickness, y+length+(gap<<1), length, thickness, 1, colour);	//right bottom
			DrawSegmentLarge(bitmap,x+length+(gap<<1)-thickness, y+gap, length, thickness, 1, colour);	//right top
		break;
		case 8:
			DrawSegmentLarge(bitmap,x+gap, y, length, thickness, 2, colour);	//top
			DrawSegmentLarge(bitmap,x, y+gap, length, thickness, 0, colour);	//left top
			DrawSegmentLarge(bitmap,x, y+length+(gap<<1), length, thickness, 0, colour);	//left bottom
			DrawSegmentLarge(bitmap,x+gap, y+(length<<1)+(gap<<1)+gap-thickness, length, thickness, 3, colour);	//bottom
			DrawSegmentLarge(bitmap,x+length+(gap<<1)-thickness, y+length+(gap<<1), length, thickness, 1, colour);	//right bottom
			DrawSegmentLarge(bitmap,x+length+(gap<<1)-thickness, y+gap, length, thickness, 1, colour);	//right top
			DrawSegmentLarge(bitmap,x+gap, y+length+gap+(gap>>1)-((unsigned int)(thickness*1.5)>>1), length, thickness*1.5, 4, colour);	//middle
		break;
		case 9:
			DrawSegmentLarge(bitmap,x+gap, y, length, thickness, 2, colour);	//top
			DrawSegmentLarge(bitmap,x, y+gap, length, thickness, 0, colour);	//left top
			DrawSegmentLarge(bitmap,x+gap, y+(length<<1)+(gap<<1)+gap-thickness, length, thickness, 3, colour);	//bottom
			DrawSegmentLarge(bitmap,x+length+(gap<<1)-thickness, y+length+(gap<<1), length, thickness, 1, colour);	//right bottom
			DrawSegmentLarge(bitmap,x+length+(gap<<1)-thickness, y+gap, length, thickness, 1, colour);	//right top
			DrawSegmentLarge(bitmap,x+gap, y+length+gap+(gap>>1)-((unsigned int)(thickness*1.5)>>1), length, thickness*1.5, 4, colour);	//middle
		break;
	}

	return x+length+(gap<<1)+(length>>2);
}


unsigned int PrintValueSevSegLarge(unsigned int bitmap[][32], unsigned int val, unsigned int xpos, unsigned int ypos, unsigned int height, unsigned int thickness, unsigned int gap, unsigned int colour)
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
		if(print==1){ xpos=SevSegDigitLarge(bitmap, digitval, xpos, ypos, height, thickness, gap, colour); }
		unit /= 10;
	}

	return xpos;
}


unsigned int PrintValue2DigitsSevSegLarge(unsigned int bitmap[][32], unsigned int val, unsigned int xpos, unsigned int ypos, unsigned int height, unsigned int thickness, unsigned int gap, unsigned int colour)
{
	unsigned int tens, units;

	tens=val/10;
	units=val-(tens*10);
	xpos=SevSegDigitLarge(bitmap, tens, xpos, ypos, height, thickness, gap, colour);
	xpos=SevSegDigitLarge(bitmap, units, xpos, ypos, height, thickness, gap, colour);

	return xpos;
}

void printfNx32LED(unsigned int bitmap[][32], int x1, int y1, unsigned int font, unsigned int colour, const char * format, ... )
{
	char buff[256];
	unsigned int i=0;
	va_list args;
	
	va_start(args,format);
	vsnprintf(buff,256,format,args);
	
  //print the string
	do
	{
		x1=BmpCharacter(bitmap, buff[i], x1, y1, colour, font);
		i+=1;
	}while(buff[i]!=0);
	
	va_end(args);
}

void BmpPlaceBitmap(const unsigned int sourcebitmap[][32], unsigned int destbitmap[][32], unsigned int width, unsigned int height, int xpos, int ypos)
{
	unsigned int x, y;
	
	for(x=0;x<width;x++)
	{
		for(y=0;y<height;y++)
		{
			BmpSetPixel(destbitmap,x+xpos,y+ypos,sourcebitmap[x][y]);
		}
	}
}

void BmpShiftInBitmap(const unsigned int sourcebitmap[][32], unsigned int destbitmap[][32], unsigned int width, unsigned int height, int xRight, int yTop, unsigned int endgap, unsigned int fps, unsigned int DisplayMode, unsigned int flipAxis)
{
	unsigned int x, y, mS;
	
	mS=1000/fps;
	
	for(x=0;x<width;x++)
	{
		for(y=0;y<height;y++)
		{
			BmpSetPixel(destbitmap,xRight,yTop+y,sourcebitmap[x][y]);
		}
		if(flipAxis!=LEDNoFlip){ BmpFlip(destbitmap,flipAxis); }	//flip
		DisplayBitmapDuration(destbitmap, mS, DisplayMode);		//display frame
		if(flipAxis!=LEDNoFlip){ BmpFlip(destbitmap,flipAxis); }	//unflip
		BmpShiftLeft(destbitmap,1);	//scroll
	}

	for(x=0;x<endgap;x++)
	{
		if(flipAxis!=LEDNoFlip){ BmpFlip(destbitmap,flipAxis); }	//flip
		DisplayBitmapDuration(destbitmap, mS, DisplayMode);		//display frame
		if(flipAxis!=LEDNoFlip){ BmpFlip(destbitmap,flipAxis); }	//unflip
		BmpShiftLeft(destbitmap,1);
	}
}

void BmpFlip(unsigned int bitmap[][32], unsigned int flipAxis)
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
