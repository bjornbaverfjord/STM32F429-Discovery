#include "glcd.h"
#include "main.h"
#include "HelperFunctions.h"
#include "registers.h"
#include <limits.h>
#include "STM32F4-lib.h"

struct GraphicsType Graphics;

/******************************************************
 *              Font 7x11(English)                    *
 * -ASCII fonts from 0x20 ~ 0x7F(DEC 32 ~ 126)     	  *
 *   bits 5 (bottom) to 15 (top) in the y direction   *
 *	      7 words=7 lines in the x direction		  *
 ******************************************************/
const unsigned short ascii_7x11[95][14] = {                                                       
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
	{0x0060, 0x0020, 0x0060, 0x00C0, 0x0080, 0x00C0, 0x005F}  //0x007E ~
};

//details on pxldata table:
//0 to 94 (character start index, 32d to 126d ascii)
//95 to 189 (bytes per character)
//190 to 558 (character graphics bytes)
const unsigned int ascii2[559]={190, 191, 192, 195, 201, 205, 211, 215, 216, 218, 220, 223, 228, 230, 233, 234, 237, 241, 243, 247, 251,
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


static unsigned int DrawAddress;
static unsigned int LCDDirection, GLCDWidth, GLCDHeight;

void InitGLCD(unsigned int direction)
{
	
	LCDDirection = direction;
	
	if(LCDDirection==portrait)
	{
		GLCDWidth = LCD_PIXEL_WIDTH;
		GLCDHeight = LCD_PIXEL_HEIGHT;
	}else
	{
		GLCDWidth = LCD_PIXEL_HEIGHT;
		GLCDHeight = LCD_PIXEL_WIDTH;
	}

	LCD_Init2();	/* LCD initialization */
  LCD_LayerInit2();	/* LCD Layer initialization */
  LTDC_Cmd(ENABLE);	/* Enable the LTDC */
  LCD_SetLayer(LCD_FOREGROUND_LAYER);	/* Set LCD foreground layer */
	DrawAddress=LCD_FRAME_BUFFER + BUFFER_OFFSET;
	IOE_Config();	/* Touch Panel configuration */
	ClearGLCD(LCD_COLOR_BLACK);
	
	LCD_SetColors(LCD_COLOR_WHITE, LCD_COLOR_BLACK);
	LCD_SetTextColor(LCD_COLOR_WHITE);	//sets colour of shapes too
	
}

void SetPixelGLCD(int x, int y, unsigned short colour)
{
	TranslateXYGLCD(&x, &y);

	if (x >= LCD_PIXEL_WIDTH) return;
	if (x < 0) return;
	if (y >= LCD_PIXEL_HEIGHT) return;
	if (y < 0) return;
	*(unsigned short *)(DrawAddress+(((y*LCD_PIXEL_WIDTH)+x)<<1))=colour;
}


void ClearGLCD(unsigned short colour)
{
	RectangleFilledWHGLCD(0,0,GLCDWidth,GLCDHeight,colour);
}

unsigned short ColourWordGLCD(unsigned char r, unsigned char g, unsigned char b)
{
	return ((r>>3)<<11) | ((g>>2)<<5) | (b>>3);
}

unsigned int GetScreenSizeGLCD(void)
{
	return (GLCDHeight<<16) | GLCDWidth;
}


void RectangleGLCD(int x1, int y1, int x2, int y2,  unsigned short colour)
{
	LineGLCD(x1,y1,x2,y1,colour);
	LineGLCD(x1,y2,x2,y2,colour);
	
	LineGLCD(x1,y1,x1,y2,colour);
	LineGLCD(x2,y1,x2,y2,colour);
}


void CircleGLCD(int x0, int y0, int radius, unsigned short colour)
{
	int f = 1 - radius;
	int ddF_x = 1;
	int ddF_y = -2 * radius;
	int x = 0;
	int y = radius;
	
	SetPixelGLCD(x0, y0 + radius,colour);
	SetPixelGLCD(x0, y0 - radius,colour);
	SetPixelGLCD(x0 + radius, y0,colour);
	SetPixelGLCD(x0 - radius, y0,colour);
	
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
		    
		SetPixelGLCD(x0 + x, y0 + y,colour);	//segment 4
		SetPixelGLCD(x0 - x, y0 + y,colour);	//segment 5
		SetPixelGLCD(x0 + x, y0 - y,colour);	//segment 1
		SetPixelGLCD(x0 - x, y0 - y,colour);	//segment 8
		SetPixelGLCD(x0 + y, y0 + x,colour);	//segment 3
		SetPixelGLCD(x0 - y, y0 + x,colour);	//segment 6
		SetPixelGLCD(x0 + y, y0 - x,colour);	//segment 2
		SetPixelGLCD(x0 - y, y0 - x,colour);	//segment 7
	}
}


void CircleFilledGLCD(int x0, int y0, int radius, unsigned short colour)
{
	int f = 1 - radius;
	int ddF_x = 1;
	int ddF_y = -2 * radius;
	int x = 0;
	int y = radius;
	
	LineGLCD(x0 - radius,y0,x0 + radius+1,y0,colour);
	
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
		    
		LineGLCD(x0 + x, y0 - y,x0 - x, y0 - y,colour);	//segment 1 to 8
		LineGLCD(x0 + y, y0 - x,x0 - y, y0 - x,colour);	//segment 2 to 7
		LineGLCD(x0 + y, y0 + x,x0 - y, y0 + x,colour);	//segment 3 to 6
		LineGLCD(x0 + x, y0 + y,x0 - x, y0 + y,colour);	//segment 4 to 5
	}
}


void TriangleGLCD(int x1, int y1, int x2, int y2, int x3, int y3, unsigned short colour)
{
	LineGLCD(x1,y1, x2,y2, colour);
	LineGLCD(x2,y2, x3,y3, colour);
	LineGLCD(x3,y3, x1,y1, colour);
}


void TriangleFilledGLCD(int x1, int y1, int x2, int y2, int x3, int y3, unsigned short colour)
{
	struct xyType points[3];

	points[0].x = x1;
	points[0].y = y1;
	points[1].x = x2;
	points[1].y = y2;
	points[2].x = x3;
	points[2].y = y3;
	PolygonConvexFilledGLCD(points,3,colour);
}


unsigned short PrintCharGLCD(char chr, unsigned short xpos, unsigned short ypos, unsigned short colour)
{
	unsigned int x;
	unsigned int y;
	
	for(x=0;x<7;x++)
	{
		for(y=5;y<16;y++)
		{
			if((ascii_7x11[chr-0x20][x]>>y) & 1)
			{
				SetPixelGLCD(xpos+x,ypos+y-5,colour);
			}
		}
	}
	
	return xpos+8;
}


unsigned short PrintStringGLCD(char string[], unsigned short xpos, unsigned short ypos, unsigned short colour)	//30x29 chars/screen 7x11 font
{
	unsigned int i = 0;

	while (string[i] != 0)
	{
		PrintCharGLCD(string[i],xpos,ypos,colour);
		xpos += 8;	//1 pixel gap between chars
		i += 1;
	}

	return xpos;
}


unsigned short GetStringWidthGLCD(char string[])
{
	unsigned int i=0;	

	do
	{
		i++;
	}while(string[i]!=0);

	return (i*8)-1;
}


unsigned int PrintStringWrappedGLCD(char string[], unsigned short xpos, unsigned short ypos, unsigned short xmax, unsigned short colour)	//30x29 chars/screen 7x11 font
{
	unsigned short i = 0, substrcnt=0, stringwidth, printcntr, xpos2, atend, spacecounter=0;
	char substring[512];

	xpos2=xpos;
	
	atend=0;
	while(atend==0)
	{
		if(isWhiteSpace(string[i])==0)	//if a word
		{
			spacecounter=0;

			substrcnt=0;
			while(isWhiteSpace(string[i])==0)	//copy the word
			{
				substring[substrcnt]=string[i];
				i++;
				substrcnt++;
			}
			substring[substrcnt]=0;	//add terminating character
			if(string[i]==0){ atend=1; }
			stringwidth=GetStringWidthGLCD(substring);

			if((xpos+stringwidth)<=xmax)	//if the word fits on the current line
			{
				xpos=PrintStringGLCD(substring,xpos,ypos,colour);	//print the word
			}else
			{
				ypos+=12;	//word doesn't fit, print it on a new line
				xpos=xpos2;
				//print the word character by character incase its longer than 1 line
				printcntr=0;
				while (substring[printcntr] != 0)
				{
					if((xpos+7)>xmax){ ypos+=12; xpos=xpos2; }	//word doesnt fit, continue remainder of word on new line
					xpos=PrintCharGLCD(substring[printcntr],xpos,ypos,colour);
					printcntr++;
				}
			}
		}else	//if a space
		{
			if((xpos+7)>xmax){ ypos+=12; xpos=xpos2; }	//if previous word ended at edge of screen, go to next line
			if(xpos==xpos2)	//if at the start of a line, ignore the first space
			{
				spacecounter++;
				if(spacecounter>1)	//if more then 1 space
				{
					xpos=PrintCharGLCD(' ',xpos, ypos, colour);	//print space (at the start of the line)
				}
			}else	//not at the start of the line
			{
				//print space (while not at the start of the line)
				if((xpos+7)>xmax){ ypos+=12; xpos=xpos2; }	//space will go over edge of screen, put it on the next line
				xpos=PrintCharGLCD(' ',xpos, ypos, colour);
			}
			
			i++;
			if(string[i]==0){ atend=1; }
		}
	}

	if((xpos+7)>xmax){ ypos+=12; xpos=xpos2; }	//make sure return for next character fits
	return (ypos<<16) | xpos;
}


unsigned short PrintValueGLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short colour)
{
	unsigned int unit;
	unsigned int digit;
	unsigned int digitval;
	unsigned int print;

	unit = 1000000000;
	print = 0;

	for(digit=10;digit>=1;digit--)
	{
		digitval = val/unit;
		val -= digitval*unit;
		if ((digitval > 0) || (digit==1)) print = 1;
		if (print == 1) xpos = PrintCharGLCD(digitval + '0', xpos ,ypos, colour);
		unit /= 10;
	}

	return xpos;
}

unsigned short PrintValueOf3GLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short colour)
{
	unsigned int unit;
	unsigned int digit;
	unsigned int digitval;
	unsigned int print;

	unit = 1000000000;
	print = 0;

	for(digit=10;digit>=1;digit--)
	{
		digitval = val/unit;
		val -= digitval*unit;
		if ((digitval > 0) || (digit==1)) print = 1;
		if (print == 1) {
			xpos = PrintCharGLCD(digitval + '0', xpos ,ypos, colour);
			if ((digit % 3) == 1) xpos += 4;
		}
		unit /= 10;
	}

	return xpos;
}




unsigned short PrintValue2DigitsGLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short colour)
{
	unsigned int tens, units;

	tens=val/10;
	units=val-(tens*10);
	xpos=PrintCharGLCD(tens+'0', xpos ,ypos, colour);
	xpos=PrintCharGLCD(units+'0', xpos ,ypos, colour);

	return xpos;
}

unsigned short PrintFloatGLCD(double val, unsigned int decimals, unsigned short xpos, unsigned short ypos, unsigned short colour)
{
	unsigned char str[50], len[2], format[6];
	
	sprintf((char *)len, "%d", decimals);
	format[0]='%'; format[1]='.'; format[2]=len[0];
	if(decimals<10)
	{
		format[3]='f';
		format[4]=0;	//string terminator
	}else
	{
		format[3]=len[1];
		format[4]='f';
		format[5]=0;	//string terminator
	}
	snprintf((char *)str,50,(const char *)format,val);
	xpos=PrintStringGLCD((char *)str, xpos, ypos, colour);

	return xpos;
}


unsigned short PrintHexGLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short colour)
{
	int i;
	
	xpos=PrintCharGLCD('0', xpos, ypos, colour);
	xpos=PrintCharGLCD('x', xpos, ypos, colour);
	
	for(i=28;i>=0;i-=4)
	{
		xpos=PrintCharGLCD("0123456789ABCDEF"[(val>>i) & 0xF], xpos, ypos, colour);
	}

	return xpos;
}

// Print hex in groups of 2 digits
unsigned short PrintHexOf2GLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short colour)
{
	int i;
	
	xpos = PrintCharGLCD('0', xpos, ypos, colour);
	xpos = PrintCharGLCD('x', xpos, ypos, colour);
	xpos += 4;
	
	for(i=28;i>=0;i-=4)
	{
		xpos=PrintCharGLCD("0123456789ABCDEF"[(val>>i) & 0xF], xpos, ypos, colour);
		if ((i & 7) == 0) xpos += 4;
	}

	return xpos;
}


unsigned short PrintHexByteGLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short colour)
{
	int i;
	
	for(i=4;i>=0;i-=4)
	{
		xpos=PrintCharGLCD("0123456789ABCDEF"[(val>>i) & 0xF], xpos, ypos, colour);
	}

	return xpos;
}


unsigned short PrintBinGLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short colour)
{
	int i;
	for(i=31;i>=0;i--)
	{
		if(((val>>i) & 1)==1){ xpos=PrintCharGLCD('1', xpos, ypos, colour); }else{ xpos=PrintCharGLCD('0', xpos, ypos, colour); }
	}

	return xpos;
}

// Print int as binary in groups of 4 digits
unsigned short PrintBinOf4GLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short colour)
{
	int i;
	for(i=31;i>=0;i--)
	{
		if(((val>>i) & 1)==1){ xpos=PrintCharGLCD('1', xpos, ypos, colour); }else{ xpos=PrintCharGLCD('0', xpos, ypos, colour); }
		if ((i & 3) == 0)  xpos += 4;
	}

	return xpos;
}


unsigned short PrintCharWithBGGLCD(char chr, unsigned short xpos, unsigned short ypos, unsigned short fillmode, unsigned short colour, unsigned short BGcolour)
{
	unsigned int x;
	unsigned int y;
	
	if(fillmode!=0){ RectangleFilledGLCD(xpos+7,ypos,GLCDWidth-1,ypos+10,BGcolour); }
	for(x=0;x<7;x++)
	{
		for(y=5;y<16;y++)
		{
			if((ascii_7x11[chr-0x20][x]>>y) & 1){ SetPixelGLCD(xpos+x,ypos+y-5,colour); }else{ SetPixelGLCD(xpos+x,ypos+y-5,BGcolour); }
		}
	}

	return xpos+8;
}


unsigned short PrintStringWithBGGLCD(char string[], unsigned short xpos, unsigned short ypos, unsigned short fillmode, unsigned short colour, unsigned short BGcolour)	//30x29 chars/screen 7x11 font
{
	unsigned int i = 0;

	if(fillmode==0)
	{
		while (string[i] != 0)
		{
			PrintCharWithBGGLCD(string[i],xpos,ypos,0,colour,BGcolour);
			xpos += 8;	//1 pixel gap between chars
			LineGLCD(xpos-1,ypos,xpos-1,ypos+10,BGcolour);
			i += 1;
		}
	}else
	{
		RectangleFilledGLCD(xpos,ypos,GLCDWidth-1,ypos+10,BGcolour);
		xpos=PrintStringGLCD(string, xpos,ypos,colour);
	}

	return xpos;
}


unsigned short PrintValueWithBGGLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short fillmode, unsigned short colour, unsigned short BGcolour)
{
	unsigned int unit;
	unsigned int digit;
	unsigned int digitval;
	unsigned int print;

	if(fillmode==0)
	{
		unit=1000000000;
		print=0;
	
		for(digit=10;digit>=1;digit--)
		{
			digitval=val/unit;
			val -= digitval*unit;
			if((digitval>0) || (digit==1)){ print=1; }
			if(print==1){ xpos=PrintCharWithBGGLCD(digitval+'0', xpos ,ypos, 0, colour, BGcolour); }
			unit /= 10;
		}
	
	}else
	{
		RectangleFilledGLCD(xpos,ypos,GLCDWidth-1,ypos+10,BGcolour);
		xpos=PrintValueGLCD(val, xpos, ypos, colour);
	}

	return xpos;
}


unsigned short PrintValue2DigitsWithBGGLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short fillmode, unsigned short colour, unsigned short BGcolour)
{
	unsigned int tens, units;

	tens=val/10;
	units=val-(tens*10);
	xpos=PrintCharWithBGGLCD(tens+'0', xpos ,ypos, fillmode, colour, BGcolour);
	if(fillmode==0)
	{
		xpos=PrintCharWithBGGLCD(units+'0', xpos ,ypos, 0, colour, BGcolour);
	}else
	{
		xpos=PrintCharGLCD(units+'0', xpos ,ypos, colour);
	}

	return xpos;
}


unsigned short PrintFloatWithBGGLCD(double val, unsigned int decimals, unsigned short xpos, unsigned short ypos, unsigned short fillmode, unsigned short colour, unsigned short BGcolour)
{
	unsigned int ipart;
	double fpart;
	unsigned int i;
	unsigned int decimal;
	
	if(fillmode==0)
	{
		ipart=(unsigned int)val;
		xpos=PrintValueWithBGGLCD(ipart, xpos, ypos, 0, colour, BGcolour);
		xpos=PrintCharWithBGGLCD('.', xpos, ypos, 0, colour, BGcolour);
	
		fpart=val-(float)ipart;
		for(i=0;i<decimals;i++)
		{
			fpart *= 10.0;
			decimal=(unsigned int)fpart;
			if(decimal>9){ decimal=9; }
			xpos=PrintCharWithBGGLCD('0'+decimal, xpos, ypos, 0, colour, BGcolour);
			fpart -= (float)decimal;
		}
	}else
	{
		RectangleFilledGLCD(xpos,ypos,GLCDWidth-1,ypos+10,BGcolour);
		xpos=PrintFloatGLCD(val, decimals, xpos, ypos, colour);
	}

	return xpos;
}


unsigned short PrintHexWithBGGLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short fillmode, unsigned short colour, unsigned short BGcolour)
{
	int i;

	if(fillmode==0)
	{
		xpos=PrintCharWithBGGLCD('0', xpos, ypos, 0, colour, BGcolour);
		xpos=PrintCharWithBGGLCD('x', xpos, ypos, 0, colour, BGcolour);
		
		for(i=28;i>=0;i-=4)
		{
			xpos=PrintCharWithBGGLCD("0123456789ABCDEF"[(val>>i) & 0xF], xpos, ypos, 0, colour, BGcolour);
		}
	}else
	{
		RectangleFilledGLCD(xpos,ypos,GLCDWidth-1,ypos+10,BGcolour);
		xpos=PrintHexGLCD(val, xpos, ypos, colour);
	}

	return xpos;
}


unsigned short PrintHexByteWithBGGLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short fillmode, unsigned short colour, unsigned short BGcolour)
{
	int i;
	
	if(fillmode==0)
	{
		for(i=4;i>=0;i-=4)
		{
			xpos=PrintCharWithBGGLCD("0123456789ABCDEF"[(val>>i) & 0xF], xpos, ypos, 0, colour, BGcolour);
		}
	}else
	{
		RectangleFilledGLCD(xpos,ypos,GLCDWidth-1,ypos+10,BGcolour);
		xpos=PrintHexByteGLCD(val, xpos, ypos, colour);
	}

	return xpos;
}


unsigned short PrintBinWithBGGLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short fillmode, unsigned short colour, unsigned short BGcolour)
{
	int i;

	if(fillmode==0)
	{
		for(i=31;i>=0;i--)
		{
			if(((val>>i) & 1)==1){ xpos=PrintCharWithBGGLCD('1', xpos, ypos, 0, colour, BGcolour); }else{ xpos=PrintCharWithBGGLCD('0', xpos, ypos, 0, colour, BGcolour); }
		}
	}else
	{
		RectangleFilledGLCD(xpos,ypos,GLCDWidth-1,ypos+10,BGcolour);
		xpos=PrintBinGLCD(val, xpos, ypos, colour);
	}

	return xpos;
}


unsigned short PrintChar2GLCD(char chr, unsigned short xpos, unsigned short ypos, unsigned short colour)	//1-9 x 8 variable width font
{
	int x;
	int y;
	int bytenum;
	
	x=0;
	for(bytenum=ascii2[chr-' '];bytenum<(ascii2[chr-' ']+ascii2[95+(chr-' ')]);bytenum++)
	{
		for(y=0;y<8;y++)	//add byte
		{
			if(((ascii2[bytenum]>>y) & 1)==1)
			{
				SetPixelGLCD(xpos+x,ypos+y,colour);
			}
		}
		x+=1;
	}
	
	return xpos+ascii2[95+(chr-' ')]+1;
}


unsigned short PrintString2GLCD(char string[], unsigned short xpos, unsigned short ypos, unsigned short colour)	//1-9 x 8 variable width font
{
	unsigned int i = 0;

	while (string[i] != 0)
	{
		xpos=PrintChar2GLCD(string[i],xpos, ypos, colour);
		i += 1;
	}

	return xpos;
}


unsigned short PrintChar2WithBGGLCD(char chr, unsigned short xpos, unsigned short ypos, unsigned short fillmode, unsigned short colour, unsigned short BGcolour)	//1-9 x 8 variable width font
{
	int x;
	int y;
	int bytenum;

	x=0;
	for(bytenum=ascii2[chr-' '];bytenum<(ascii2[chr-' ']+ascii2[95+(chr-' ')]);bytenum++)
	{
		for(y=0;y<8;y++)	//add byte
		{
			if(((ascii2[bytenum]>>y) & 1)==1){ SetPixelGLCD(xpos+x,ypos+y,colour); }else{ SetPixelGLCD(xpos+x,ypos+y,BGcolour); }
		}
		x+=1;
	}
	if(fillmode!=0){ RectangleFilledGLCD(xpos+x,ypos,GLCDWidth-1,ypos+7,BGcolour); }

	return xpos+x+1;
}


unsigned short PrintString2WithBGGLCD(char string[], unsigned short xpos, unsigned short ypos, unsigned short fillmode, unsigned short colour, unsigned short BGcolour)	//30x29 chars/screen 7x11 font
{	
	unsigned int i = 0;
	
	if(fillmode==0)
	{
		while (string[i] != 0)
		{
			xpos=PrintChar2WithBGGLCD(string[i],xpos,ypos,0,colour,BGcolour);
			//xpos += 8;	//1 pixel gap between chars
			LineGLCD(xpos-1,ypos,xpos-1,ypos+7,BGcolour);
			i += 1;
		}
	}else
	{
		RectangleFilledGLCD(xpos,ypos,GLCDWidth-1,ypos+7,BGcolour);
		xpos=PrintString2GLCD(string, xpos,ypos,colour);
	}
	
	return xpos;
}


unsigned short GetStringWidth2GLCD(char string[])
{
	unsigned int i=0, length=0;	

	do
	{
		length+=ascii2[95+(string[i]-' ')];
		length+=1;
		i++;
	}while(string[i]!=0);

	return length-1;
}


unsigned int PrintStringWrapped2GLCD(char string[], unsigned short xpos, unsigned short ypos, unsigned short xmax, unsigned short colour)
{
	unsigned short i = 0, substrcnt=0, stringwidth, printcntr, xpos2, atend, spacecounter=0, charlength;
	char substring[512];

	xpos2=xpos;
	
	atend=0;
	while(atend==0)
	{
		if(isWhiteSpace(string[i])==0)	//if a word
		{
			spacecounter=0;

			substrcnt=0;
			while(isWhiteSpace(string[i])==0)	//copy the word
			{
				substring[substrcnt]=string[i];
				i++;
				substrcnt++;
			}
			substring[substrcnt]=0;	//add terminating character
			if(string[i]==0){ atend=1; }
			stringwidth=GetStringWidth2GLCD(substring);

			if((xpos+stringwidth)<=xmax)	//if the word fits on the current line
			{
				xpos=PrintString2GLCD(substring,xpos,ypos,colour);	//print the word
			}else
			{
				ypos+=12;	//word doesn't fit, print it on a new line
				xpos=xpos2;
				//print the word character by character incase its longer than 1 line
				printcntr=0;
				while (substring[printcntr] != 0)
				{
					charlength=ascii2[95+(substring[printcntr]-' ')];
					if((xpos+charlength)>xmax){ ypos+=9; xpos=xpos2; }	//word doesnt fit, continue remainder of word on new line
					xpos=PrintChar2GLCD(substring[printcntr],xpos,ypos,colour);
					printcntr++;
				}
			}
		}else	//if a space
		{
			if((xpos+ascii2[95])>xmax){ ypos+=9; xpos=xpos2; }	//if previous word ended at edge of screen, go to next line
			if(xpos==xpos2)	//if at the start of a line, ignore the first space
			{
				spacecounter++;
				if(spacecounter>1)	//if more then 1 space
				{
					xpos=PrintChar2GLCD(' ',xpos, ypos, colour);	//print space (at the start of the line)
				}
			}else	//not at the start of the line
			{
				//print space (while not at the start of the line)
				if((xpos+ascii2[95])>xmax){ ypos+=9; xpos=xpos2; }	//space will go over edge of screen, put it on the next line
				xpos=PrintChar2GLCD(' ',xpos, ypos, colour);
			}
			
			i++;
			if(string[i]==0){ atend=1; }
		}
	}

	if((xpos+7)>xmax){ ypos+=12; xpos=xpos2; }	//make sure return for next character fits
	return (ypos<<16) | xpos;
}


unsigned short PrintValue2GLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short colour)
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
		if(print==1){ xpos=PrintChar2GLCD(digitval+'0', xpos ,ypos, colour); }
		unit /= 10;
	}

	return xpos;
}


unsigned short PrintFloat2GLCD(double val, unsigned int decimals, unsigned short xpos, unsigned short ypos, unsigned short colour)
{
	unsigned int ipart;
	double fpart;
	unsigned int i;
	unsigned int decimal;
	
	ipart=(unsigned int)val;
	xpos=PrintValue2GLCD(ipart, xpos, ypos, colour);
	xpos=PrintChar2GLCD('.', xpos, ypos, colour);

	fpart=val-(float)ipart;
	for(i=0;i<decimals;i++)
	{
		fpart *= 10.0;
		decimal=(unsigned int)fpart;
		if(decimal>9){ decimal=9; }
		xpos=PrintChar2GLCD('0'+decimal, xpos, ypos, colour);
		fpart -= (float)decimal;
	}

	return xpos;
}


unsigned short PrintHex2GLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short colour)
{
	int i;
	
	xpos=PrintChar2GLCD('0', xpos, ypos, colour);
	xpos=PrintChar2GLCD('x', xpos, ypos, colour);
	
	for(i=28;i>=0;i-=4)
	{
		xpos=PrintChar2GLCD("0123456789ABCDEF"[(val>>i) & 0xF], xpos, ypos, colour);
	}

	return xpos;
}


unsigned short PrintHexByte2GLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short colour)
{
	int i;
	
	for(i=4;i>=0;i-=4)
	{
		xpos=PrintChar2GLCD("0123456789ABCDEF"[(val>>i) & 0xF], xpos, ypos, colour);
	}

	return xpos;
}


unsigned short PrintBin2GLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short colour)
{
	int i;
	for(i=31;i>=0;i--)
	{
		if(((val>>i) & 1)==1){ xpos=PrintChar2GLCD('1', xpos, ypos, colour); }else{ xpos=PrintChar2GLCD('0', xpos, ypos, colour); }
	}

	return xpos;
}


unsigned short PrintValue2WithBGGLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short fillmode, unsigned short colour, unsigned short BGcolour)
{
	unsigned int unit;
	unsigned int digit;
	unsigned int digitval;
	unsigned int print;

	if(fillmode==0)
	{
		unit=1000000000;
		print=0;
	
		for(digit=10;digit>=1;digit--)
		{
			digitval=val/unit;
			val -= digitval*unit;
			if((digitval>0) || (digit==1)){ print=1; }
			if(print==1){ xpos=PrintChar2WithBGGLCD(digitval+'0', xpos ,ypos, 0, colour, BGcolour); }
			unit /= 10;
		}
	
	}else
	{
		RectangleFilledGLCD(xpos,ypos,GLCDWidth-1,ypos+7,BGcolour);
		xpos=PrintValueGLCD(val, xpos, ypos, colour);
	}

	return xpos;
}


unsigned short PrintFloat2WithBGGLCD(double val, unsigned int decimals, unsigned short xpos, unsigned short ypos, unsigned short fillmode, unsigned short colour, unsigned short BGcolour)
{
	unsigned int ipart;
	double fpart;
	unsigned int i;
	unsigned int decimal;
	
	if(fillmode==0)
	{
		ipart=(unsigned int)val;
		xpos=PrintValue2WithBGGLCD(ipart, xpos, ypos, 0, colour, BGcolour);
		xpos=PrintChar2WithBGGLCD('.', xpos, ypos, 0, colour, BGcolour);
	
		fpart=val-(float)ipart;
		for(i=0;i<decimals;i++)
		{
			fpart *= 10.0;
			decimal=(unsigned int)fpart;
			if(decimal>9){ decimal=9; }
			xpos=PrintChar2WithBGGLCD('0'+decimal, xpos, ypos, 0, colour, BGcolour);
			fpart -= (float)decimal;
		}
	}else
	{
		RectangleFilledGLCD(xpos,ypos,GLCDWidth-1,ypos+7,BGcolour);
		xpos=PrintFloat2GLCD(val, decimals, xpos, ypos, colour);
	}

	return xpos;
}


unsigned short PrintHex2WithBGGLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short fillmode, unsigned short colour, unsigned short BGcolour)
{
	int i;

	if(fillmode==0)
	{
		xpos=PrintChar2WithBGGLCD('0', xpos, ypos, 0, colour, BGcolour);
		xpos=PrintChar2WithBGGLCD('x', xpos, ypos, 0, colour, BGcolour);
		
		for(i=28;i>=0;i-=4)
		{
			xpos=PrintChar2WithBGGLCD("0123456789ABCDEF"[(val>>i) & 0xF], xpos, ypos, 0, colour, BGcolour);
		}
	}else
	{
		RectangleFilledGLCD(xpos,ypos,GLCDWidth-1,ypos+7,BGcolour);
		xpos=PrintHex2GLCD(val, xpos, ypos, colour);
	}

	return xpos;
}


unsigned short PrintHexByte2WithBGGLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short fillmode, unsigned short colour, unsigned short BGcolour)
{
	int i;
	
	if(fillmode==0)
	{
		for(i=4;i>=0;i-=4)
		{
			xpos=PrintChar2WithBGGLCD("0123456789ABCDEF"[(val>>i) & 0xF], xpos, ypos, 0, colour, BGcolour);
		}
	}else
	{
		RectangleFilledGLCD(xpos,ypos,GLCDWidth-1,ypos+7,BGcolour);
		xpos=PrintHexByte2GLCD(val, xpos, ypos, colour);
	}

	return xpos;
}


unsigned short PrintBin2WithBGGLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short fillmode, unsigned short colour, unsigned short BGcolour)
{
	int i;

	if(fillmode==0)
	{
		for(i=31;i>=0;i--)
		{
			if(((val>>i) & 1)==1){ xpos=PrintCharWithBGGLCD('1', xpos, ypos, 0, colour, BGcolour); }else{ xpos=PrintChar2WithBGGLCD('0', xpos, ypos, 0, colour, BGcolour); }
		}
	}else
	{
		RectangleFilledGLCD(xpos,ypos,GLCDWidth-1,ypos+10,BGcolour);
		xpos=PrintBin2GLCD(val, xpos, ypos, colour);
	}

	return xpos;
}


void DrawSegmentGLCD(unsigned short x, unsigned short y, unsigned short length, unsigned short width, unsigned short type, unsigned short colour)
{
	int i;
	
	switch(type)
	{
		case 0:	//left
			for(i=0;i<width;i++)
			{
				LineGLCD(x+i,y+i,x+i,y+(length-1)-i,colour);
			}
		break;
		case 1:	//right
			for(i=0;i<width;i++)
			{
				LineGLCD(x+((width-1)-i),y+i,x+((width-1)-i),y+(length-1)-i,colour);
			}
		break;
		case 2:	//top
			for(i=0;i<width;i++)
			{
				LineGLCD(x+i,y+i,x+(length-1)-i,y+i,colour);
			}
		break;
		case 3:	//bottom
			for(i=0;i<width;i++)
			{
				LineGLCD(x+((width-1)-i),y+i,x+(length-1)-((width-1)-i),y+i,colour);
			}
		break;
		case 4:	//middle
			width>>=1;
			for(i=0;i<width;i++)
			{
				LineGLCD(x+((width-1)-i),y+i,x+(length-1)-((width-1)-i),y+i,colour);
			}
			y+=width-1;
			for(i=1;i<width;i++)
			{
				LineGLCD(x+i,y+i,x+(length-1)-i,y+i,colour);
			}
		break;
	}
}


unsigned short SevSegDigitGLCD(unsigned short digit, unsigned short x, unsigned short y, unsigned short height, unsigned short thickness, unsigned short gap, unsigned short colour)
{
	unsigned int length;

	length=(height>>1)-(gap>>1);
	
	switch(digit)
	{
		case 0:
			DrawSegmentGLCD(x+gap, y, length, thickness, 2, colour);	//top
			DrawSegmentGLCD(x, y+gap, length, thickness, 0, colour);	//left top
			DrawSegmentGLCD(x, y+length+(gap<<1), length, thickness, 0, colour);	//left bottom
			DrawSegmentGLCD(x+gap, y+(length<<1)+(gap<<1)+gap-thickness, length, thickness, 3, colour);	//bottom
			DrawSegmentGLCD(x+length+(gap<<1)-thickness, y+length+(gap<<1), length, thickness, 1, colour);	//right bottom
			DrawSegmentGLCD(x+length+(gap<<1)-thickness, y+gap, length, thickness, 1, colour);	//right top
		break;
		case 1:
			DrawSegmentGLCD(x+length+(gap<<1)-thickness, y+length+(gap<<1), length, thickness, 1, colour);	//right bottom
			DrawSegmentGLCD(x+length+(gap<<1)-thickness, y+gap, length, thickness, 1, colour);	//right top
		break;
		case 2:
			DrawSegmentGLCD(x+gap, y, length, thickness, 2, colour);	//top
			DrawSegmentGLCD(x, y+length+(gap<<1), length, thickness, 0, colour);	//left bottom
			DrawSegmentGLCD(x+gap, y+(length<<1)+(gap<<1)+gap-thickness, length, thickness, 3, colour);	//bottom
			DrawSegmentGLCD(x+length+(gap<<1)-thickness, y+gap, length, thickness, 1, colour);	//right top
			DrawSegmentGLCD(x+gap, y+length+gap+(gap>>1)-((unsigned short)(thickness*1.5)>>1), length, thickness*1.5, 4, colour);	//middle
		break;
		case 3:
			DrawSegmentGLCD(x+gap, y, length, thickness, 2, colour);	//top
			DrawSegmentGLCD(x+gap, y+(length<<1)+(gap<<1)+gap-thickness, length, thickness, 3, colour);	//bottom
			DrawSegmentGLCD(x+length+(gap<<1)-thickness, y+length+(gap<<1), length, thickness, 1, colour);	//right bottom
			DrawSegmentGLCD(x+length+(gap<<1)-thickness, y+gap, length, thickness, 1, colour);	//right top
			DrawSegmentGLCD(x+gap, y+length+gap+(gap>>1)-((unsigned short)(thickness*1.5)>>1), length, thickness*1.5, 4, colour);	//middle
		break;
		case 4:
			DrawSegmentGLCD(x, y+gap, length, thickness, 0, colour);	//left top
			DrawSegmentGLCD(x+length+(gap<<1)-thickness, y+length+(gap<<1), length, thickness, 1, colour);	//right bottom
			DrawSegmentGLCD(x+length+(gap<<1)-thickness, y+gap, length, thickness, 1, colour);	//right top
			DrawSegmentGLCD(x+gap, y+length+gap+(gap>>1)-((unsigned short)(thickness*1.5)>>1), length, thickness*1.5, 4, colour);	//middle
		break;
		case 5:
			DrawSegmentGLCD(x+gap, y, length, thickness, 2, colour);	//top
			DrawSegmentGLCD(x, y+gap, length, thickness, 0, colour);	//left top
			DrawSegmentGLCD(x+gap, y+(length<<1)+(gap<<1)+gap-thickness, length, thickness, 3, colour);	//bottom
			DrawSegmentGLCD(x+length+(gap<<1)-thickness, y+length+(gap<<1), length, thickness, 1, colour);	//right bottom
			DrawSegmentGLCD(x+gap, y+length+gap+(gap>>1)-((unsigned short)(thickness*1.5)>>1), length, thickness*1.5, 4, colour);	//middle
		break;
		case 6:
			DrawSegmentGLCD(x+gap, y, length, thickness, 2, colour);	//top
			DrawSegmentGLCD(x, y+gap, length, thickness, 0, colour);	//left top
			DrawSegmentGLCD(x, y+length+(gap<<1), length, thickness, 0, colour);	//left bottom
			DrawSegmentGLCD(x+gap, y+(length<<1)+(gap<<1)+gap-thickness, length, thickness, 3, colour);	//bottom
			DrawSegmentGLCD(x+length+(gap<<1)-thickness, y+length+(gap<<1), length, thickness, 1, colour);	//right bottom
			DrawSegmentGLCD(x+gap, y+length+gap+(gap>>1)-((unsigned short)(thickness*1.5)>>1), length, thickness*1.5, 4, colour);	//middle
		break;
		case 7:
			DrawSegmentGLCD(x+gap, y, length, thickness, 2, colour);	//top
			DrawSegmentGLCD(x+length+(gap<<1)-thickness, y+length+(gap<<1), length, thickness, 1, colour);	//right bottom
			DrawSegmentGLCD(x+length+(gap<<1)-thickness, y+gap, length, thickness, 1, colour);	//right top
		break;
		case 8:
			DrawSegmentGLCD(x+gap, y, length, thickness, 2, colour);	//top
			DrawSegmentGLCD(x, y+gap, length, thickness, 0, colour);	//left top
			DrawSegmentGLCD(x, y+length+(gap<<1), length, thickness, 0, colour);	//left bottom
			DrawSegmentGLCD(x+gap, y+(length<<1)+(gap<<1)+gap-thickness, length, thickness, 3, colour);	//bottom
			DrawSegmentGLCD(x+length+(gap<<1)-thickness, y+length+(gap<<1), length, thickness, 1, colour);	//right bottom
			DrawSegmentGLCD(x+length+(gap<<1)-thickness, y+gap, length, thickness, 1, colour);	//right top
			DrawSegmentGLCD(x+gap, y+length+gap+(gap>>1)-((unsigned short)(thickness*1.5)>>1), length, thickness*1.5, 4, colour);	//middle
		break;
		case 9:
			DrawSegmentGLCD(x+gap, y, length, thickness, 2, colour);	//top
			DrawSegmentGLCD(x, y+gap, length, thickness, 0, colour);	//left top
			DrawSegmentGLCD(x+gap, y+(length<<1)+(gap<<1)+gap-thickness, length, thickness, 3, colour);	//bottom
			DrawSegmentGLCD(x+length+(gap<<1)-thickness, y+length+(gap<<1), length, thickness, 1, colour);	//right bottom
			DrawSegmentGLCD(x+length+(gap<<1)-thickness, y+gap, length, thickness, 1, colour);	//right top
			DrawSegmentGLCD(x+gap, y+length+gap+(gap>>1)-((unsigned short)(thickness*1.5)>>1), length, thickness*1.5, 4, colour);	//middle
		break;
	}

	return x+length+(gap<<1)+(length>>2);
}


unsigned short PrintValueSevSegGLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short height, unsigned short thickness, unsigned short gap, unsigned short colour)
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
		if(print==1){ xpos=SevSegDigitGLCD(digitval, xpos, ypos, height, thickness, gap, colour); }
		unit /= 10;
	}

	return xpos;
}


unsigned short PrintValue2DigitsSevSegGLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short height, unsigned short thickness, unsigned short gap, unsigned short colour)
{
	unsigned int tens, units;

	tens=val/10;
	units=val-(tens*10);
	xpos=SevSegDigitGLCD(tens, xpos, ypos, height, thickness, gap, colour);
	xpos=SevSegDigitGLCD(units, xpos, ypos, height, thickness, gap, colour);

	return xpos;
}



TP_STATE* ReadTouch(void)
{
	TP_STATE* touch;
	unsigned int t, tmp, i;
	
	t=0;
	touch = IOE_TP_GetState();
	
	if(touch->TouchDetected)
	{
		for(i=0; i<40; i++)	//try to get a valid reading
		{
			if((touch->Y < 317) && (touch->Y >= 3) && (touch->X < 237) && (touch->X > 3))
			{
				t=1;
				
				if(LCDDirection!=portrait)
				{
					tmp=touch->Y;
					touch->Y=239-touch->X;
					touch->X=tmp;
				}
				
				break;
			}
			touch = IOE_TP_GetState();
		}
	}
	
	touch->TouchDetected=t;
	
	return touch;
}


// Translate coordinates for portrait and landscape orientation of the LCD
// so (0,0) is in the top left corner and (319,239) is in bottom right corner
void TranslateXYGLCD(int *x, int *y)
{
	if (LCDDirection == landscape) {
  		*y = (LCD_PIXEL_WIDTH - 1) - *y;
		  swap (x,y);
	}
}

void TranslateXYWHGLCD(int *x, int *y,int *w, int *h)
{
	if (LCDDirection == landscape) {
  		*y = (LCD_PIXEL_WIDTH - 1) - *y;
		  *y = *y - (*h - 1);
		  swap (x,y);
			swap (w,h);	  
	}
}


void RectangleFilledGLCD(int x1, int y1, int x2, int y2,  unsigned short colour)
{
		int width, height;
	
		// Make (x1,y1) top left and (x2,y2) bottom right
		if(x1>x2){ swap(&x1,&x2); }
		if(y1>y2){ swap(&y1,&y2); }
		
		width = (x2 - x1) + 1;
		height = (y2 - y1) + 1;
		
		RectangleFilledWHGLCD(x1,y1,width,height,colour);
	
	return;
}


void RectangleFilledWHGLCD(int Xpos, int Ypos, int Width, int Height, unsigned short colour)
{
  DMA2D_InitTypeDef      DMA2D_InitStruct;
  
  uint32_t  Xaddress = 0; 
  uint16_t Red_Value = 0, Green_Value = 0, Blue_Value = 0;
 
  Red_Value = (0xF800 & colour) >> 11;
  Blue_Value = 0x001F & colour;
  Green_Value = (0x07E0 & colour) >> 5;
  
  TranslateXYWHGLCD(&Xpos,&Ypos,&Width,&Height);

  if (Xpos < 0) {
		Width -= -Xpos;
		Xpos = 0;
	}
	
  if (Ypos < 0) {
		Height -= -Ypos;
		Ypos = 0;
	}
  
  if (Xpos >= LCD_PIXEL_WIDTH) return;
  if (Width <= 0) return;

  if (Ypos >= LCD_PIXEL_HEIGHT) return;
  if (Height <= 0) return;

  if ((Xpos + Width) >= LCD_PIXEL_WIDTH) Width = (LCD_PIXEL_WIDTH - Xpos);
  if ((Ypos + Height) >= LCD_PIXEL_HEIGHT) Height = (LCD_PIXEL_HEIGHT - Ypos);

  Xaddress = DrawAddress + 2 * (LCD_PIXEL_WIDTH * Ypos + Xpos);
  
  // configure DMA2D 
  DMA2D_DeInit();
  DMA2D_InitStruct.DMA2D_Mode = DMA2D_R2M;       
  DMA2D_InitStruct.DMA2D_CMode = DMA2D_RGB565;      
  DMA2D_InitStruct.DMA2D_OutputGreen = Green_Value;      
  DMA2D_InitStruct.DMA2D_OutputBlue = Blue_Value;     
  DMA2D_InitStruct.DMA2D_OutputRed = Red_Value;                
  DMA2D_InitStruct.DMA2D_OutputAlpha = 0x0F;                  
  DMA2D_InitStruct.DMA2D_OutputMemoryAdd = Xaddress;                
  DMA2D_InitStruct.DMA2D_OutputOffset = (LCD_PIXEL_WIDTH - Width);                
  DMA2D_InitStruct.DMA2D_NumberOfLine = Height;            
  DMA2D_InitStruct.DMA2D_PixelPerLine = Width;
  DMA2D_Init(&DMA2D_InitStruct); 
  
  DMA2D_StartTransfer();
  
  // Wait for transfer complete
  while(DMA2D_GetFlagStatus(DMA2D_FLAG_TC) == RESET);
}


void WaitForVSyncStart(void) {
			while((GPIOA_IDR & (1 << 4)) != 0);
}

void WaitForVSyncGLCD(void) {
			while((GPIOA_IDR & (1 << 4)) != 0);
			while((GPIOA_IDR & (1 << 4)) == 0);
}


void WaitForHSyncGLCD(void) {
			while((GPIOC_IDR & (1 << 6)) != 0);
			while((GPIOC_IDR & (1 << 6)) == 0);
}


void LineGLCD(int x1, int y1, int x2, int y2, unsigned short colour)
{
	int x, y, xdelta, ydelta, width, height, i, count;
	
	if (y1 == y2) {
		if (y1 < 0) return;
		if (y1 >= GLCDHeight) return;
		if (x1 > x2) swap(&x1,&x2);
		if (x1 < 0) x1 = 0;
		if (x2 >= GLCDWidth) x2 = GLCDWidth - 1;
		RectangleFilledWHGLCD(x1,y1,(x2 - x1) + 1,1,colour);
	  return;
	}

	if (x1 == x2) {
		if (x1 < 0) return;
		if (x1 >= GLCDWidth) return;
		if (y1 > y2) swap(&y1,&y2);
		if (y1 < 0) y1 = 0;
		if (y2 >= GLCDHeight) y2 = GLCDHeight - 1;
		RectangleFilledWHGLCD(x1,y1,1,(y2 - y1) + 1,colour);
		return;
	}
	
	width = x2 - x1;
	height = y2 - y1;
	
	count = max(iabs(width),iabs(height));

	xdelta = (width << 16) / count;
	ydelta = (height << 16) / count;

	x = x1 << 16;
	y = y1 << 16;

	for (i = 0;i <= count;i++)	{
		SetPixelGLCD(x >> 16,y >> 16,colour);
		x += xdelta;
		y += ydelta;
	}		
}


void PolygonGLCD(struct xyType points[],int count,short colour) 
{
	int i;
	
	if (count < 3) return;
	
	LineGLCD(points[0].x, points[0].y,points[count - 1 ].x, points[count - 1].y, colour);
	for (i = 0;i < count;i++)	{
		LineGLCD(points[i - 1].x, points[i - 1].y,points[i].x, points[i].y, colour);
	}
}


void PolygonConvexFilledGLCD(struct xyType points[],int count,short colour) 
{
  const int yCount = 320;
	int i;
	short xLeft[yCount], xRight[yCount];
	
	if (count < 3) return;
	
	for (i = 0;i < yCount;i++)	{
		xLeft[i] = SHRT_MAX;
		xRight[i] = SHRT_MIN;
	}

	PolygonLineGLCD(points[0].x, points[0].y,points[count -1 ].x, points[count - 1].y, xLeft, xRight);
	for (i = 1;i < count;i++)	{
		PolygonLineGLCD(points[i - 1].x, points[i - 1].y,points[i].x, points[i].y, xLeft, xRight);
	}
	
	RasterizePolygonConvexGLCD(xLeft, xRight, colour);
}


void PolygonLineGLCD(int x1, int y1, int x2, int y2, short xLeft[], short xRight[])
{
	int x, y, xdelta, ydelta, width, height, i, count;
	
	width = x2 - x1;
	height = y2 - y1;
	
	count = max(iabs(width),iabs(height));

	xdelta = (width << 16) / count;
	ydelta = (height << 16) / count;

	x = x1 << 16;
	y = y1 << 16;

	for (i = 0;i <= count;i++)	{
		if (isVisibleGLCD(x >> 16,y >> 16)) {
			xLeft[y >> 16] = min(x >> 16, xLeft[y >> 16]);
			xRight[y >> 16] = max(x >> 16, xRight[y >> 16]);
		}
		x += xdelta;
		y += ydelta;
	}		
}


void RasterizePolygonConvexGLCD(short xLeft[], short xRight[], short colour)
{
	int y;
  const int yCount = 320;

	for (y = 0;y < yCount;y++)	{
		if (xLeft[y] == SHRT_MAX) continue;
		if (xRight[y] == SHRT_MIN) xRight[y] = xLeft[y];
		LineGLCD(xLeft[y],y,xRight[y],y,colour);
	}		
}

int isVisibleGLCD(int x, int y)
{
  if (x < 0) return 0;
  if (y < 0) return 0;
  if (x >= GLCDWidth) return 0;
  if (y >= GLCDHeight) return 0;
  return 1;
}

//buttons-------
void AddButtonGLCD(struct ButtonType buttons[], unsigned int *TotalButtons, unsigned short x, unsigned short y, unsigned short width, unsigned short height, char text[])
{
	unsigned int i=0;
	
	buttons[*TotalButtons].x=x;
	buttons[*TotalButtons].y=y;
	buttons[*TotalButtons].width=width;
	buttons[*TotalButtons].height=height;
	
	while(text[i]!=0)
	{
		buttons[*TotalButtons].text[i]=text[i];
		i+=1;
	}
	buttons[*TotalButtons].text[i]=0;

	*TotalButtons+=1;
}

void DrawButtonGLCD(unsigned short x, unsigned short y, unsigned short width, unsigned short height, char text[])
{
	RectangleFilledGLCD(x+1,y+1,x+(width-1)-2,y+(height-1)-2,ColourWordGLCD(128,128,128));
	LineGLCD(x,y,x,y+(height-1),LCD_COLOR_WHITE);
	LineGLCD(x,y,x+(width-1),y,LCD_COLOR_WHITE);
	LineGLCD(x,y+(height-1)-1,x+(width-1),y+(height-1)-1,ColourWordGLCD(80,80,80));
	LineGLCD(x,y+(height-1),x+(width-1),y+(height-1),ColourWordGLCD(80,80,80));
	LineGLCD(x+(width-1)-1,y,x+(width-1)-1,y+(height-1),ColourWordGLCD(80,80,80));
	LineGLCD(x+(width-1),y,x+(width-1),y+(height-1),ColourWordGLCD(80,80,80));
	PrintStringGLCD(text,x+(width>>1)-(GetStringWidthGLCD(text)>>1),y+(height>>1)-6,LCD_COLOR_BLACK);
}

void DrawButtonPressedGLCD(unsigned short x, unsigned short y, unsigned short width, unsigned short height, char text[])
{
	RectangleFilledGLCD(x+1,y+1,x+(width-1)-1,y+(height-1)-1,ColourWordGLCD(128,128,128));
	
	LineGLCD(x,y,x,y+(height-1),ColourWordGLCD(80,80,80));
	LineGLCD(x+1,y,x+1,y+(height-1),ColourWordGLCD(80,80,80));
	LineGLCD(x,y,x+(width-1),y,ColourWordGLCD(80,80,80));
	LineGLCD(x,y+1,x+(width-1),y+1,ColourWordGLCD(80,80,80));
	
	LineGLCD(x,y+(height-1),x+(width-1),y+(height-1),ColourWordGLCD(80,80,80));
	LineGLCD(x+(width-1),y,x+(width-1),y+(height-1),ColourWordGLCD(80,80,80));
	PrintStringGLCD(text,x+(width>>1)-(GetStringWidthGLCD(text)>>1)+1,y+(height>>1)-6+1,LCD_COLOR_BLACK);
}

void DrawButtonsGLCD(struct ButtonType buttons[], unsigned int TotalButtons)
{
	unsigned int i;
	
	for(i=0;i<TotalButtons;i++)
	{
		DrawButtonGLCD(buttons[i].x, buttons[i].y, buttons[i].width, buttons[i].height, buttons[i].text);
	}
}

int GetPressedButtonGLCD(struct ButtonType buttons[], unsigned int TotalButtons)
{
	unsigned int i;
	volatile TP_STATE* TP_State;
	int PressedButton=-1;
	
	TP_State = ReadTouch();
	if(TP_State->TouchDetected)
	{
		for(i=0;i<TotalButtons;i++)
		{
			if((TP_State->X > buttons[i].x) && (TP_State->X < (buttons[i].x+(buttons[i].width-1))) && (TP_State->Y > buttons[i].y) && (TP_State->Y < (buttons[i].y+(buttons[i].height-1))))
			{
				PressedButton=i;
				
				SetDrawBufferGraphics(GetLayer1BufferGraphics());	//set drawing to 
				DrawButtonPressedGLCD(buttons[i].x, buttons[i].y, buttons[i].width, buttons[i].height, buttons[i].text);
				waitsys(20000);	//debounce
				do	//wait while pressed
				{
					TP_State = ReadTouch();
				}while(TP_State->TouchDetected);
				DrawButtonGLCD(buttons[i].x, buttons[i].y, buttons[i].width, buttons[i].height, buttons[i].text);
				//SetLayer1ToBufferGraphics(GetNextLayer1BufferGraphics());
				ResetDrawAddressGraphics();
				break;
			}
		}
	}
	
	return PressedButton;
}
//--------------


static void LCD_AF_GPIOConfig(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  
  /* Enable GPIOA, GPIOB, GPIOC, GPIOD, GPIOF, GPIOG AHB Clocks */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | \
                         RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | \
                         RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOG, ENABLE);

/* GPIOs Configuration */
/*
 +------------------------+-----------------------+----------------------------+
 +                       LCD pins assignment                                   +
 +------------------------+-----------------------+----------------------------+
 |  LCD_TFT R2 <-> PC.10  |  LCD_TFT G2 <-> PA.06 |  LCD_TFT B2 <-> PD.06      |
 |  LCD_TFT R3 <-> PB.00  |  LCD_TFT G3 <-> PG.10 |  LCD_TFT B3 <-> PG.11      |
 |  LCD_TFT R4 <-> PA.11  |  LCD_TFT G4 <-> PB.10 |  LCD_TFT B4 <-> PG.12      |
 |  LCD_TFT R5 <-> PA.12  |  LCD_TFT G5 <-> PB.11 |  LCD_TFT B5 <-> PA.03      |
 |  LCD_TFT R6 <-> PB.01  |  LCD_TFT G6 <-> PC.07 |  LCD_TFT B6 <-> PB.08      |
 |  LCD_TFT R7 <-> PG.06  |  LCD_TFT G7 <-> PD.03 |  LCD_TFT B7 <-> PB.09      |
 -------------------------------------------------------------------------------
          |  LCD_TFT HSYNC <-> PC.06  | LCDTFT VSYNC <->  PA.04 |
          |  LCD_TFT CLK   <-> PG.07  | LCD_TFT DE   <->  PF.10 |
           -----------------------------------------------------

*/

 /* GPIOA configuration */
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource4, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_LTDC);

  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_6 | \
                             GPIO_Pin_11 | GPIO_Pin_12;
                             
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStruct);
  
 /* GPIOB configuration */  
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource0, 0x09);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource1, 0x09);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_LTDC);

  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_8 | \
                             GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
  
  GPIO_Init(GPIOB, &GPIO_InitStruct);

 /* GPIOC configuration */
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_LTDC);
  
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_10;
                             
  GPIO_Init(GPIOC, &GPIO_InitStruct);

 /* GPIOD configuration */
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource3, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource6, GPIO_AF_LTDC);
  
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_6;
                             
  GPIO_Init(GPIOD, &GPIO_InitStruct);
  
 /* GPIOF configuration */
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource10, GPIO_AF_LTDC);
  
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
                             
  GPIO_Init(GPIOF, &GPIO_InitStruct);     

 /* GPIOG configuration */  
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource6, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource7, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource10, 0x09);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource11, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource12, 0x09);

  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_10 | \
                             GPIO_Pin_11 | GPIO_Pin_12;
  
  GPIO_Init(GPIOG, &GPIO_InitStruct);
 
}


// Sets up both layers for the STM32 LTDC controller
// The controller loads both layers and merges them into 
// one video stream using alpha and/or transparency
void LCD_LayerInit2(void)
{
  LTDC_Layer_InitTypeDef LTDC_Layer_InitStruct; 
  
  // Windowing configuration 
  // In this case all the active display area is used to display a picture then:
  // Horizontal start = horizontal synchronization + Horizontal back porch = 30 
  // Horizontal stop = Horizontal start + window width -1 = 30 + 240 -1
  // Vertical start   = vertical synchronization + vertical back porch     = 4
  // Vertical stop   = Vertical start + window height -1  = 4 + 320 -1
  LTDC_Layer_InitStruct.LTDC_HorizontalStart = 30;
  LTDC_Layer_InitStruct.LTDC_HorizontalStop = (LCD_PIXEL_WIDTH + 30 - 1); // Verified to be horizontal
  LTDC_Layer_InitStruct.LTDC_VerticalStart = 4;
  LTDC_Layer_InitStruct.LTDC_VerticalStop = (LCD_PIXEL_HEIGHT + 4 - 1); // Verified to be vertical
  
  // Pixel Format configuration
  LTDC_Layer_InitStruct.LTDC_PixelFormat = LTDC_Pixelformat_RGB565;
  // Alpha constant (255 totally opaque)
  LTDC_Layer_InitStruct.LTDC_ConstantAlpha = 255; 
  // Default Color configuration (configure A,R,G,B component values)
  LTDC_Layer_InitStruct.LTDC_DefaultColorBlue = 0;        
  LTDC_Layer_InitStruct.LTDC_DefaultColorGreen = 0;       
  LTDC_Layer_InitStruct.LTDC_DefaultColorRed = 0;         
  LTDC_Layer_InitStruct.LTDC_DefaultColorAlpha = 0;
  /* Configure blending factors */       
  LTDC_Layer_InitStruct.LTDC_BlendingFactor_1 = LTDC_BlendingFactor1_CA;    
  LTDC_Layer_InitStruct.LTDC_BlendingFactor_2 = LTDC_BlendingFactor2_CA;
  
  // the length of one line of pixels in bytes + 3 then :
  // Line Lenth         = Active high width x number of bytes per pixel + 3 
  // Bytes per pixel    = 2    (pixel_format : RGB565) 
  LTDC_Layer_InitStruct.LTDC_CFBLineLength = ((LCD_PIXEL_WIDTH * 2) + 3);

  // the pitch is the increment from the start of one line of pixels to the 
  // start of the next line in bytes, then :
  // Pitch = Active high width times number of bytes per pixel 
  LTDC_Layer_InitStruct.LTDC_CFBPitch = LCD_PIXEL_WIDTH * 2;
  
  // Configure the number of lines
  LTDC_Layer_InitStruct.LTDC_CFBLineNumber = LCD_PIXEL_HEIGHT; // Verified to be number of lines on Y axis
  
  // Start Address configuration : the LCD Frame buffer is defined on SDRAM
  LTDC_Layer_InitStruct.LTDC_CFBStartAdress = LCD_FRAME_BUFFER;
  
  // Initialize LTDC layer 1
  LTDC_LayerInit(LTDC_Layer1, &LTDC_Layer_InitStruct);
 



  // Configure Layer2
  // Start Address configuration : the LCD Frame buffer is defined on SDRAM w/ Offset
  LTDC_Layer_InitStruct.LTDC_CFBStartAdress = LCD_FRAME_BUFFER + BUFFER_OFFSET;
  
  // Configure blending factors
  LTDC_Layer_InitStruct.LTDC_BlendingFactor_1 = LTDC_BlendingFactor1_PAxCA;    
  LTDC_Layer_InitStruct.LTDC_BlendingFactor_2 = LTDC_BlendingFactor2_PAxCA;
 
  // Initialize LTDC layer 2 
  LTDC_LayerInit(LTDC_Layer2, &LTDC_Layer_InitStruct);
  
  // LTDC configuration reload
  LTDC_ReloadConfig(LTDC_IMReload);
  
  // Enable foreground & background Layers
  LTDC_LayerCmd(LTDC_Layer1, ENABLE); 
  LTDC_LayerCmd(LTDC_Layer2, ENABLE);
  
  // LTDC configuration reload
  LTDC_ReloadConfig(LTDC_IMReload);
  
  // Set default font
  LCD_SetFont(&LCD_DEFAULT_FONT); 
  
  // dithering activation
  LTDC_DitherCmd(ENABLE);
}


void LCD_Init2(void)
{ 
  LTDC_InitTypeDef LTDC_InitStruct;
  
  // Configure the LCD Control pins
  LCD_CtrlLinesConfig();
  LCD_ChipSelect(DISABLE);
  LCD_ChipSelect(ENABLE);
  
  // Configure the LCD_SPI interface
  LCD_SPIConfig(); 
  
  // Power on the ILI9341 LCD controller
  LCD_PowerOn2();
  
  // Enable the LTDC Clock
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_LTDC, ENABLE);
  
  /* Enable the DMA2D Clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2D, ENABLE); 
  
  /* Configure the LCD Control pins */
  LCD_AF_GPIOConfig();  
  
  /* Configure the FMC Parallel interface : SDRAM is used as Frame Buffer for LCD */
  SDRAM_Init();
  
  /* LTDC Configuration *********************************************************/  
  /* Polarity configuration */
  /* Initialize the horizontal synchronization polarity as active low */
  LTDC_InitStruct.LTDC_HSPolarity = LTDC_HSPolarity_AL;     
  /* Initialize the vertical synchronization polarity as active low */  
  LTDC_InitStruct.LTDC_VSPolarity = LTDC_VSPolarity_AL;     
  /* Initialize the data enable polarity as active low */
  LTDC_InitStruct.LTDC_DEPolarity = LTDC_DEPolarity_AL;     
  /* Initialize the pixel clock polarity as input pixel clock */ 
  LTDC_InitStruct.LTDC_PCPolarity = LTDC_PCPolarity_IPC;
  
  /* Configure R,G,B component values for LCD background color */                   
  LTDC_InitStruct.LTDC_BackgroundRedValue = 0;            
  LTDC_InitStruct.LTDC_BackgroundGreenValue = 0;          
  LTDC_InitStruct.LTDC_BackgroundBlueValue = 0;  
  
  /* Configure PLLSAI prescalers for LCD */
  /* Enable Pixel Clock */
  /* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
  /* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAI_N = 192 Mhz */
  /* PLLLCDCLK = PLLSAI_VCO Output/PLLSAI_R = 192/4 = 48 Mhz */
  /* LTDC clock frequency = PLLLCDCLK / RCC_PLLSAIDivR = 48/8 = 6 Mhz */
  RCC_PLLSAIConfig(192, 7, 4);
  RCC_LTDCCLKDivConfig(RCC_PLLSAIDivR_Div8);
  
  /* Enable PLLSAI Clock */
  RCC_PLLSAICmd(ENABLE);
  /* Wait for PLLSAI activation */
  while(RCC_GetFlagStatus(RCC_FLAG_PLLSAIRDY) == RESET)
  {
  }
  
  // Timing configuration
  // Configure horizontal synchronization width
  LTDC_InitStruct.LTDC_HorizontalSync = 9;

  // Configure vertical synchronization height 
  LTDC_InitStruct.LTDC_VerticalSync = 1;

  // Configure accumulated horizontal back porch 
  LTDC_InitStruct.LTDC_AccumulatedHBP = 29; 

  // Configure accumulated vertical back porch 
  LTDC_InitStruct.LTDC_AccumulatedVBP = 3;  

  // Configure accumulated active width 
  LTDC_InitStruct.LTDC_AccumulatedActiveW = LCD_PIXEL_WIDTH + 29;

  // Configure accumulated active height
  LTDC_InitStruct.LTDC_AccumulatedActiveH = LCD_PIXEL_HEIGHT + 3;

  // Configure total width
  LTDC_InitStruct.LTDC_TotalWidth = LCD_PIXEL_WIDTH + 39; 

  // Configure total height
  LTDC_InitStruct.LTDC_TotalHeigh = LCD_PIXEL_HEIGHT + 7;

  LTDC_Init(&LTDC_InitStruct);
}  



//         Hsync    HBP          HAdr         HFP
//  -----+<-----><-------><---------------><------->
//       |
// Vsync | 
//       |
//  -----+ Vertical interval when no valid display data is transferred from host to display
//  VBp  |
//  -----+                +---------------+
//       |                |               | 
//  VAdr |                |  Period when valid display data are transferred from host to display module
//       |                |               |
//  -----+                +---------------+
//  VFP  | Vertical interval when no valid display data is transferred from host to display
//  -----+<---------------------------------------->

// (Hsync + HBP) – Horizontal interval when no valid display data is sent from host to display
// HFP - Horizontal interval when no valid display data is sent from host to display 

// Parameter                  Symbol Min  Typ Max Unit
// Horizontal Synchronization Hsync    2   10  16 DOTCLK
// Horizontal Back Porch      HBP      2   20  24 DOTCLK
// Horizontal Address         HAdr     -  240   - DOTCLK
// Horizontal Front Porch     HFP      2   10  16 DOTCLK
// Vertical   Synchronization Vsync    1    2   4 Line
// Vertical   Back Porch      VBP      1    2   - Line
// Vertical   Address         VAdr     -  320   - Line
// Vertical   Front Porch     VFP      3    4   - Line

// Typical clock frequency 6.35MHz and frame frequency 70Hz.


// Writes a 16 bit value to the ILI9341
void LCD_Write16(uint16_t value)
{
		LCD_WriteData(hi8(value));  // high byte
		LCD_WriteData(lo8(value));  // low byte
}


// Sets up and enables the ILI9341 LCD controller
void LCD_PowerOn2(void)
{
  LCD_WriteCommand(0xCA);
  LCD_WriteData(0xC3);
  LCD_WriteData(0x08);
  LCD_WriteData(0x50);
  LCD_WriteCommand(LCD_POWERB);    // Power control B (CFh)
  LCD_WriteData(0x00);
  LCD_WriteData(0xC1);
  LCD_WriteData(0x30);
  LCD_WriteCommand(LCD_POWER_SEQ); // Power on sequence control (EDh)
  LCD_WriteData(0x64);
  LCD_WriteData(0x03);
  LCD_WriteData(0x12);
  LCD_WriteData(0x81);
  LCD_WriteCommand(LCD_DTCA);      // Driver timing control A (E8h)
  LCD_WriteData(0x85);
  LCD_WriteData(0x00);
  LCD_WriteData(0x78);
  LCD_WriteCommand(LCD_POWERA);   // Power control A (CBh)
  LCD_WriteData(0x39);
  LCD_WriteData(0x2C);
  LCD_WriteData(0x00);
  LCD_WriteData(0x34);
  LCD_WriteData(0x02);
  LCD_WriteCommand(LCD_PRC);      // Pump ratio control (F7h)
  LCD_WriteData(0x20);
  LCD_WriteCommand(LCD_DTCB);     // Driver timing control B (EAh)
  LCD_WriteData(0x00);
  LCD_WriteData(0x00);
  LCD_WriteCommand(LCD_FRC);      // Frame Rate Control (In Normal Mode/Full Colour) (B1h)
  LCD_WriteData(0x00);
  LCD_WriteData(0x1B);
  LCD_WriteCommand(LCD_DFC);      // Display Function Control (B6h)
  LCD_WriteData(0x0A);
  LCD_WriteData(0xA2);
  LCD_WriteCommand(LCD_POWER1);   // Power Control 1 (C0h)
  LCD_WriteData(0x10);
  LCD_WriteCommand(LCD_POWER2);   // Power Control 2 (C1h)
  LCD_WriteData(0x10);
  LCD_WriteCommand(LCD_VCOM1);    // VCOM Control 1(C5h)
  LCD_WriteData(0x45);
  LCD_WriteData(0x15);
  LCD_WriteCommand(LCD_VCOM2);   // VCOM Control 2(C7h)
  LCD_WriteData(0x90);

	// Portrait
	LCD_WriteCommand(LCD_MAC);     // Memory Access Control (36h)
  LCD_WriteData(0xC8);

  LCD_WriteCommand(LCD_3GAMMA_EN);     // Enable 3G (F2h) (gamma control)
  LCD_WriteData(0x00);
  LCD_WriteCommand(LCD_RGB_INTERFACE); // RGB Interface Signal Control (B0h)
  LCD_WriteData(0xC2);
  LCD_WriteCommand(LCD_DFC);           // Display Function Control (B6h)
  LCD_WriteData(0x0A);
  LCD_WriteData(0xA7);
  LCD_WriteData(0x27);
  LCD_WriteData(0x04);

  // Portrait
  // Column address set 
	LCD_WriteCommand(LCD_COLUMN_ADDR); // Column Address Set (2Ah)
	LCD_Write16(0);
	LCD_Write16(LCD_PIXEL_WIDTH);

	// Page Address Set 
	LCD_WriteCommand(LCD_PAGE_ADDR);  // Page Address Set (2Bh)
	LCD_Write16(0);
	LCD_Write16(LCD_PIXEL_HEIGHT);
	
  LCD_WriteCommand(LCD_INTERFACE);  // Interface Control (F6h)
  LCD_WriteData(0x01);
  LCD_WriteData(0x00);
  LCD_WriteData(0x06);
  
  LCD_WriteCommand(LCD_GRAM);       // Memory Write (2Ch)
	
	waitsys(5000);	                  // Wait 5 ms for unknown reason
  
  LCD_WriteCommand(LCD_GAMMA);      // Gamma Set (26h)
  LCD_WriteData(0x01);
  
  LCD_WriteCommand(LCD_PGAMMA);     // Positive Gamma Correction (E0h)
  LCD_WriteData(0x0F);
  LCD_WriteData(0x29);
  LCD_WriteData(0x24);
  LCD_WriteData(0x0C);
  LCD_WriteData(0x0E);
  LCD_WriteData(0x09);
  LCD_WriteData(0x4E);
  LCD_WriteData(0x78);
  LCD_WriteData(0x3C);
  LCD_WriteData(0x09);
  LCD_WriteData(0x13);
  LCD_WriteData(0x05);
  LCD_WriteData(0x17);
  LCD_WriteData(0x11);
  LCD_WriteData(0x00);

  LCD_WriteCommand(LCD_NGAMMA);    // Negative Gamma Correction (E1h)
  LCD_WriteData(0x00);
  LCD_WriteData(0x16);
  LCD_WriteData(0x1B);
  LCD_WriteData(0x04);
  LCD_WriteData(0x11);
  LCD_WriteData(0x07);
  LCD_WriteData(0x31);
  LCD_WriteData(0x33);
  LCD_WriteData(0x42);
  LCD_WriteData(0x05);
  LCD_WriteData(0x0C);
  LCD_WriteData(0x0A);
  LCD_WriteData(0x28);
  LCD_WriteData(0x2F);
  LCD_WriteData(0x0F);
  
  LCD_WriteCommand(LCD_SLEEP_OUT);  // Sleep Out (11h) This command turns off sleep mode. 
	                                  // In this mode e.g. the DC/DC converter is enabled, 
																		// Internal oscillator is started, and panel scanning is started.

	waitsys(5000);                    // Wait 5 ms before sending next command, allow time for the supply voltages and clock circuits stabilize.

  LCD_WriteCommand(LCD_DISPLAY_ON); // Display ON (29h) Output from the Frame Memory is enabled.

  LCD_WriteCommand(LCD_GRAM);       // Memory Write (2Ch)
 }



// Return the size of a pixel in bytes
int GetPixelSizeGraphics(int pixeltype) {
	switch (pixeltype) {
		case LTDC_Pixelformat_ARGB8888:
			return 4;
		case LTDC_Pixelformat_RGB888:
			return 3;
		case LTDC_Pixelformat_RGB565:
		case LTDC_Pixelformat_ARGB1555:
		case LTDC_Pixelformat_ARGB4444:
			return 2;
		case LTDC_Pixelformat_L8:
		case LTDC_Pixelformat_AL44:
		case LTDC_Pixelformat_AL88:
			return 1;
	}
	FailWith("Unknown PixelType in GetPixelSize.",pixeltype);
	return 0;
}


// Deallocate a single buffer
void FreeBufferGraphics(int b) 
{
	if (!isValidBufferGraphics(b)) FailWith("Trying to free unallocated graphics buffer.",b);
	Graphics.AllocatedBitMap &= ~(1 << b);
}	


// Allocate a single buffer if available
int AllocateBufferGraphics(void)
{
	int i;
	for (i=0;i<32;i+=1) {
		if (((Graphics.AllocatedBitMap >> i) & 1) == 0) {
			Graphics.AllocatedBitMap |= 1 << i;
			Graphics.Buffer[i].Next = -1;
			return i;
		}
  }
	FailWith("Out of graphics memory.",i);
	return -1;
}


// Deallocate buffers in a flipchain
void FreeFlipChainGraphics(int n) 
{
	int i,prev;
	prev = n;
	for (i=0;i<32;i+=1) {
		FreeBufferGraphics(prev);
		prev = Graphics.Buffer[prev].Next;
		if (prev == n) return;
	}
}


// Allocate n linked buffers that will be visible in turn by FlipBuffGraphics
int AllocateFlipChainGraphics(int n) 
{
	int i,x,prev,first;

	if (n < 1) return -1;
	first = AllocateBufferGraphics();
	prev = first;
	for (i=0;i<(n-1);i+=1) {
		x = AllocateBufferGraphics();
		Graphics.Buffer[prev].Next = x;
		prev = x;
	}
	Graphics.Buffer[prev].Next = first;
	
	return first;
}


// Set up graphics buffers and default values
// n1 = number of buffers in the layer 1 flipchain
// n2 = number of buffers in the layer 2 flipchain
void InitGraphics(int n1,int n2)
{
	int i,adr;
	Graphics.Address = LCD_FRAME_BUFFER;
	Graphics.Width = LCD_PIXEL_WIDTH;
	Graphics.Height = LCD_PIXEL_HEIGHT;
	Graphics.NumberOfBuffers = 8;
	Graphics.AllocatedBitMap = -1 << Graphics.NumberOfBuffers;
	Graphics.Layer1Buffer = -1;
	Graphics.Layer2Buffer = -1;
	Graphics.Layer1PixelType = LTDC_Pixelformat_RGB565;
	Graphics.Layer2PixelType = LTDC_Pixelformat_RGB565;
	
	adr = Graphics.Address;
	for (i=0;i<Graphics.NumberOfBuffers;i+=1) {
		Graphics.Buffer[i].Address = adr;
		Graphics.Buffer[i].PixelType = LTDC_Pixelformat_RGB565;
		Graphics.Buffer[i].Width = Graphics.Width;
		Graphics.Buffer[i].Height = Graphics.Height;
		adr += ((Graphics.Width * Graphics.Height) * GetPixelSizeGraphics(Graphics.Buffer[i].PixelType));
		Graphics.Buffer[i].Next = -1;
	}
	SetTrasparencyGraphics(255);
	
	if (n1 > 0) SetLayer1ToBufferGraphics(AllocateFlipChainGraphics(n1));
	if (n2 > 0) SetLayer2ToBufferGraphics(AllocateFlipChainGraphics(n2));
}


// Set mixture ration between layer1 and layer2
// 255 = only layer 1
// 128 = 50%
//   0 = only layer2
void SetTrasparencyGraphics(int t)
{
	WaitForVSyncStart();
	LTDC_LayerAlpha(LTDC_Layer2, 255 - t);
	LTDC_LayerAlpha(LTDC_Layer1, 255);
	LTDC_SRCR = 1;
}


// Make the next buffer in the flipchain visible or stay on same if only one buffer in the chain
// Flips both layer 1 & 2, and sets drawing buffer to next in layer 1 buffer chain
// Flags is a bitwise multiparameter value 
// Flags = CopyBuffers | ClearBufferAfterFlip | WaitForVerticalSync
void FlipBuffGraphics(int flags)
{
	int i;
	
	if (flags & WaitForVerticalSync) WaitForVSyncStart();

	// Handle layer 1
	if (Graphics.Layer1Buffer != -1) {
		if (flags & CopyBuffers) {	// TODO: DMA this (make a blit function)
			for(i=0; i<(Graphics.Height * Graphics.Width); i++)
			{
				*(unsigned short *)(Graphics.Buffer[Graphics.Layer1Buffer].Address+(i<<1))=*(unsigned short *)(Graphics.Buffer[Graphics.Buffer[Graphics.Layer1Buffer].Next].Address+(i<<1));
			}
		}
		SetLayer1ToBufferGraphics(GetNextLayer1BufferGraphics());
		if (flags & ClearBufferAfterFlip) ClearGLCD(LCD_COLOR_BLACK);
	}

  // Handle layer 2
	if (Graphics.Layer2Buffer != -1) {
		if (flags & CopyBuffers) {	// TODO: DMA this (make a blit function)
			for(i=0; i<(Graphics.Height * Graphics.Width); i++)
			{
				*(unsigned short *)(Graphics.Buffer[Graphics.Layer2Buffer].Address+(i<<1))=*(unsigned short *)(Graphics.Buffer[Graphics.Buffer[Graphics.Layer2Buffer].Next].Address+(i<<1));
			}
		}
		SetLayer2ToBufferGraphics(GetNextLayer2BufferGraphics());
		DrawAddress = Graphics.Buffer[Graphics.Buffer[Graphics.Layer2Buffer].Next].Address;
		if (flags & ClearBufferAfterFlip) ClearGLCD(LCD_COLOR_BLACK);
		ResetDrawAddressGraphics();
	}

	if (flags & WaitForVerticalSync) WaitForVSyncGLCD();
}


// Check if a buffer exists and is allocated
int isValidBufferGraphics(int buffer) 
{
	if (buffer < 0) return 0;
	if (buffer >= Graphics.NumberOfBuffers) return 0;
	if (((Graphics.AllocatedBitMap >> buffer) & 1) == 0) return 0;
	return -1;
}


// Set layer1 to a buffer and make DrawAddress point to it (sets the curently onscreen buffer for layer1)
void SetLayer1ToBufferGraphics(int buffer)
{
	if (!isValidBufferGraphics(buffer)) FailWith("Invalid buffer",buffer);
  Graphics.Layer1Buffer = buffer;
  Graphics.Layer1PixelType = Graphics.Buffer[buffer].PixelType;

	if (Graphics.Buffer[buffer].Next != -1) {
		DrawAddress = Graphics.Buffer[Graphics.Buffer[buffer].Next].Address;
	} else {
		DrawAddress = Graphics.Buffer[buffer].Address;
	}

  LTDC_L1CFBAR = Graphics.Buffer[buffer].Address;
  LTDC_SRCR = 1;
}


// Set layer2 to a buffer (sets the curently onscreen buffer for layer1)
void SetLayer2ToBufferGraphics(int buffer)
{
	if (!isValidBufferGraphics(buffer)) FailWith("Invalid buffer",buffer);
  Graphics.Layer2Buffer = buffer;
  Graphics.Layer2PixelType = Graphics.Buffer[buffer].PixelType;
  LTDC_L2CFBAR = Graphics.Buffer[buffer].Address;
  LTDC_SRCR = 1;
}


// Return the address of a buffer
int GetAddressOfBufferGraphics(int n) 
{
	return Graphics.Buffer[n].Address;
}


// Return the buffer showing on layer1
int GetLayer1BufferGraphics() 
{
	return Graphics.Layer1Buffer;
}

// Return the buffer showing on layer2
int GetLayer2BufferGraphics() 
{
	return Graphics.Layer2Buffer;
}

// Return the next buffer showing on layer1
int GetNextLayer1BufferGraphics(void) 
{
	return Graphics.Buffer[Graphics.Layer1Buffer].Next;
}

// Return the next buffer showing on layer2
int GetNextLayer2BufferGraphics(void) 
{
	return Graphics.Buffer[Graphics.Layer2Buffer].Next;
}


// Set the DrawAddress to any buffer
void SetDrawBufferGraphics(int buffer) 
{
	DrawAddress = Graphics.Buffer[buffer].Address;
}


// Set the DrawAddress to the current layer1 buffer (which is the next one in the flipchain from the currently onscreen buffer)
void ResetDrawAddressGraphics() 
{
	DrawAddress = Graphics.Buffer[Graphics.Buffer[Graphics.Layer1Buffer].Next].Address;
}


// Get the next buffer in a flipchain
int GetNextBufferGraphics(int n)
{
	if (!isValidBufferGraphics(n)) return n; 
	if (Graphics.Buffer[n].Next == -1) return n;
	return Graphics.Buffer[n].Next;
}


int RGB888OfRGB565(int c) {
	int r = c >> 11;
	int g = (c >> 5) & 63;
	int b = c & 31;
	
	r = (r << 3) | (r >> 2); 
	g = (r << 2) | (g >> 4); 
	b = (b << 3) | (b >> 2); 
	
	return (r << 16) | (g << 8) | b;
}

