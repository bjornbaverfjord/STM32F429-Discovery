#include "glcd.h"
#include "main.h"
#include "HelperFunctions.h"
#include "registers.h"
#include <limits.h>
#include "STM32F4-lib.h"
#include <stdarg.h>
#include "fonttables.h"

struct GraphicsType Graphics;
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

unsigned int GetFontHeight(unsigned int font)
{
	switch(font)
	{
		case LCDFontVariableWidth:
			return 8;
		case LCDFont7x11:
			return 11;
		case LCDFont8x8:
			return 8;
		case LCDFont8x12:
			return 12;
		case LCDFont12x12:
			return 12;
		case LCDFont16x24:
			return 24;
		case LCDFontHebrew:
			return 16;
	}
	
	return 0;
}

unsigned int GetFontWidth(unsigned int font)
{
	switch(font)
	{
		case LCDFontVariableWidth:
			return 9;	//widest letter
		case LCDFont7x11:
			return 7;
		case LCDFont8x8:
			return 8;
		case LCDFont8x12:
			return 8;
		case LCDFont12x12:
			return 12;
		case LCDFont16x24:
			return 16;
		case LCDFontHebrew:
			return 8;	//widest letter
	}

	return 0;
}

unsigned int PrintStringWrappedGLCD(char string[], unsigned short xpos, unsigned short ypos, unsigned short xmax, unsigned int font, unsigned int TextColour, int BackColour)	//30x29 chars/screen 7x11 font
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
			stringwidth=GetStringWidthGLCD(substring,font);

			if((xpos+stringwidth)<=xmax)	//if the word fits on the current line
			{
				xpos=PrintfGLCD(xpos,ypos,TextColour,BackColour,font,substring);	//print the word
			}else
			{
				ypos+=GetFontHeight(font)+1;	//word doesn't fit, print it on a new line
				xpos=xpos2;
				//print the word character by character incase its longer than 1 line
				printcntr=0;
				while (substring[printcntr] != 0)
				{
					if((xpos+GetCharacterWidthGLCD(substring[printcntr],font))>xmax){ ypos+=GetFontHeight(font)+1; xpos=xpos2; }	//word doesnt fit, continue remainder of word on new line
					xpos=BmpCharacter(substring[printcntr],xpos,ypos,font,TextColour,BackColour);
					printcntr++;
				}
			}
		}else	//if a space
		{
			if((xpos+GetCharacterWidthGLCD(' ',font))>xmax){ ypos+=GetFontHeight(font)+1; xpos=xpos2; }	//if previous word ended at edge of screen, go to next line
			if(xpos==xpos2)	//if at the start of a line, ignore the first space
			{
				spacecounter++;
				if(spacecounter>1)	//if more then 1 space
				{
					xpos=BmpCharacter(' ',xpos,ypos,font,TextColour,BackColour);	//print space (at the start of the line)
				}
			}else	//not at the start of the line
			{
				//print space (while not at the start of the line)
				if((xpos+GetCharacterWidthGLCD(' ',font))>xmax){ ypos+=GetFontHeight(font)+1; xpos=xpos2; }	//space will go over edge of screen, put it on the next line
				xpos=BmpCharacter(' ',xpos,ypos,font,TextColour,BackColour);
			}
			
			i++;
			if(string[i]==0){ atend=1; }
		}
	}

	if((xpos+7)>xmax){ ypos+=12; xpos=xpos2; }	//make sure return for next character fits
	return (ypos<<16) | xpos;
}

unsigned short PrintBinGLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned int font, unsigned int TextColour, int BackColour)
{
	int i;
	for(i=31;i>=0;i--)
	{
		if(((val>>i) & 1)==1){ xpos=BmpCharacter(xpos, ypos, font, TextColour, BackColour, '1'); }else{ xpos=BmpCharacter(xpos, ypos, font, TextColour, BackColour, '0'); }
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
	PrintfGLCD(x+(width>>1)-(GetStringWidthGLCD(text,LCDFont7x11)>>1),y+(height>>1)-6,LCDFont7x11,LCD_COLOR_BLACK,ColourWordGLCD(128,128,128),text);
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
	PrintfGLCD(x+(width>>1)-(GetStringWidthGLCD(text,LCDFont7x11)>>1)+1,y+(height>>1)-6+1,LCDFont7x11,LCD_COLOR_BLACK,ColourWordGLCD(128,128,128),text);
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

unsigned int BmpCharacter(char character, int x1, int y1, unsigned int font, unsigned int TextColour, int BackColour)
{
	int x;
	int y;
	int bytenum;
	int nextx;
	int prelineX;
	
	//background before the character
	prelineX=x1-1;
	if((BackColour>-1) && (prelineX>-1)){ LineGLCD(x1-1,y1,x1-1,y1+GetFontHeight(font)-1,BackColour); }

	switch(font)
	{
		case LCDFontVariableWidth:
			x=0;
			for(bytenum=asciivbw[character-' '];bytenum<(asciivbw[character-' ']+asciivbw[95+(character-' ')]);bytenum++)
			{
				for(y=0;y<8;y++)	//add byte
				{
					if(((asciivbw[bytenum]>>y) & 1)==1)
					{
						SetPixelGLCD(x+x1,y+y1,TextColour);
					}else
					{
						if(BackColour>-1){ SetPixelGLCD(x+x1,y+y1,BackColour); }
					}
				}
				x+=1;
			}
			break;
			
		case LCDFont7x11:
			for(x=0;x<7;x++)
			{
				for(y=5;y<16;y++)
				{
					if((ascii_7x11[character-0x20][x]>>y) & 1)
					{
						SetPixelGLCD(x+x1,y+y1-5,TextColour);
					}else
					{
						if(BackColour>-1){ SetPixelGLCD(x+x1,y+y1-5,BackColour); }
					}
				}
			}
			break;
			
		case LCDFont8x8:
			for(y=0;y<8;y++)
			{
				for(x=0;x<8;x++)
				{
					if(ascii_8x8[y+((character-' ')*8)] & (1<<(7-x)))
					{
						SetPixelGLCD(x+x1,y+y1,TextColour);
					}else
					{
						if(BackColour>-1){ SetPixelGLCD(x+x1,y+y1,BackColour); }
					}
				}
			}
			break;
			
		case LCDFont8x12:
			for(y=0;y<12;y++)
			{
				for(x=0;x<8;x++)
				{
					if(ascii_8x12[y+((character-' ')*12)] & (1<<(7-x)))
					{
						SetPixelGLCD(x+x1,y+y1,TextColour);
					}else
					{
						if(BackColour>-1){ SetPixelGLCD(x+x1,y+y1,BackColour); }
					}
				}
			}
			break;
			
		case LCDFont12x12:
			for(y=0;y<12;y++)
			{
				for(x=0;x<12;x++)
				{
					if(ascii_12x12[y+((character-' ')*12)] & (1<<(15-x)))
					{
						SetPixelGLCD(x+x1,y+y1,TextColour);
					}else
					{
						if(BackColour>-1){ SetPixelGLCD(x+x1,y+y1,BackColour); }
					}
				}
			}
			break;
			
		case LCDFont16x24:
			for(y=0;y<24;y++)
			{
				for(x=0;x<16;x++)
				{
					if(ascii_16x24[y+((character-' ')*24)] & (1<<x))
					{
						SetPixelGLCD(x+x1,y+y1,TextColour);
					}else
					{
						if(BackColour>-1){ SetPixelGLCD(x+x1,y+y1,BackColour); }
					}
				}
			}
			break;
			
		case LCDFontHebrew:
			x=0;
			for(bytenum=hebrewfontvbw[character-1]+hebrewfontvbw[27+(character-1)]-1;bytenum>=hebrewfontvbw[character-1];bytenum--)
			{
				for(y=0;y<16;y++)	//add byte
				{
					if(((hebrewfontvbw[bytenum]>>y) & 1)==1)
					{
						SetPixelGLCD(x1-x,y+y1,TextColour);
					}else
					{
						if(BackColour>-1){ SetPixelGLCD(x1-x,y+y1,BackColour); }
					}
				}
				x+=1;
			}
			break;
	}
	
	
	if(font!=LCDFontHebrew)
	{
		nextx=x1+x+1;
		if(BackColour>-1){ LineGLCD(x1+x,y1,x1+x,y1+GetFontHeight(font)-1,BackColour); }	//Colour inter letter gap
	}else
	{
		nextx=x1-x-1;
		if(BackColour>-1){ LineGLCD(x1-x,y1,x1-x,y1+GetFontHeight(font)-1,BackColour); }	//Colour inter letter gap
	}
	
	return nextx;
}

unsigned int GetStringWidthGLCD(char string[], unsigned int font)
{
	int x=0;
	int i=0;

	while(string[i]!=0)
	{
		x+=GetCharacterWidthGLCD(string[i], font)+1;
		i+=1;
	}
	
	return x-1;
}

unsigned int GetCharacterWidthGLCD(char chr, unsigned int font)
{
	unsigned int w=0;
	
	switch(font)
	{
		case LCDFontVariableWidth:
			w=asciivbw[95+(chr-' ')];
			break;
			
		case LCDFont7x11:
			w=7;
			break;
			
		case LCDFont8x8:
			w=8;
			break;
			
		case LCDFont8x12:
			w=8;
			break;
			
		case LCDFont12x12:
			w=12;
			break;
			
		case LCDFont16x24:
			w=16;
			break;
			
		case LCDFontHebrew:
			w=hebrewfontvbw[27+(chr-1)];
			break;
	}
	
	return w;
}

unsigned short PrintfGLCD(int x1, int y1, unsigned int font, unsigned int TextColour, int BackColour, const char * format, ... )
{
	char buff[256];
	unsigned int i=0;
	va_list args;
	
	va_start(args,format);
	vsnprintf(buff,256,format,args);
	
  //print the string
	do
	{
		x1=BmpCharacter(buff[i], x1, y1, font, TextColour, BackColour);
		i+=1;
	}while(buff[i]!=0);
	
	va_end(args);
	
	return x1;
}

