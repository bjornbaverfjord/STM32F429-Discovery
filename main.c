#include "main.h"
#include "glcd.h"
#include "hebletters.h"
#include "registers.h"
#include "STM32F4-lib.h"
#include "InterruptPositions.h"
#include "AlternateFunctions.h"
#include "InterruptPositions.h"
#include "HelperFunctions.h"
#include "math.h"
#include "Nx32_RGB_LED.h"	//uses timer14, LedMatrixWidth is defined here

int main(void)
{
	struct ButtonType buttons[1];
	unsigned int TotalButtons=0;
	int PressedButton,x;
	
	//unsigned int bmp[64][32]={0};
	
	configSYSTICK(1000);	// 1 us timer resolution
	
	//InitRGBLEDMatrix();
	//RGBLEDMatrixScrollText(bmp, "Hello World!    ", 63, 15-6, 10, LED8BITWHITE, LedFont7x11, DisplayMode8bit,LEDNoFlip);
	
	InitGLCD(landscape);
	
	// Create two layer1 buffers and one layer2 buffer
	InitGraphics(2,1);
  
	ConfigPortG("-oo- ----  ---- ----");

	
	// Draw a picture on the visible layer2 buffer
	SetDrawBufferGraphics(GetLayer2BufferGraphics());
	ClearGLCD(LCD_COLOR_BLACK);
	PrintfGLCD(0,0,LCDFont7x11,LCD_COLOR_BLACK,LCD_COLOR_WHITE,"                  Text!                  ");
	CircleGLCD(159,119,100,LCD_COLOR_BLUE);
	PrintValueSevSegGLCD(1234, 160-50+2+10, 120-5-13-30, 20, 3, 2, LCD_COLOR_YELLOW);
	RectangleGLCD(160-50+2+14+5, 120-5-13-31, 160-50+2+10+58, 120-5-13-30+25, LCD_COLOR_RED);
	PrintfGLCD(160-50,120-5-13,LCDFont7x11,LCD_COLOR_BLACK,LCD_COLOR_WHITE,"Hello World!");
	PrintfGLCD(160-50,120-5,LCDFont7x11,LCD_COLOR_WHITE,-1,"Hello World!");
	PrintfGLCD(160-50+20,120-5+14,LCDFont7x11,LCD_COLOR_GREEN,-1,"%.5f",3.14159);
	PrintfGLCD(160-50+23,120-5+28,LCDFontVariableWidth,LCD_COLOR_WHITE,-1,"Hello World!");
	PrintfGLCD(160-50+23+14,120-5+28+10,LCDFontVariableWidth,LCD_COLOR_BLUE,LCD_COLOR_RED,"Text!");
	
	// This is buggy now
	AddButtonGLCD(buttons, &TotalButtons, 0, 15, GetStringWidthGLCD("blink",LCDFont7x11)+8, 32, "blink");
	DrawButtonsGLCD(buttons, TotalButtons);

  // Set the draw address back to the layer1 backbuffer
	ResetDrawAddressGraphics();
	
	// Mix layer1 and layer2 equally
	SetTrasparencyGraphics(128);
	FailWith("failure",1103);
  x = 0;
  while(1)
	{
    ClearGLCD(LCD_COLOR_BLACK);		
		CircleFilledGLCD(x%400,119,100,LCD_COLOR_GREEN);
		
		PressedButton = GetPressedButtonGLCD(buttons, TotalButtons);
		if (PressedButton == 0) GPIOG_ODR^=1<<13;
		
		// Flash red LED to make sure it is possible to detect a crash
		if ((x & 31) == 0) GPIOG_ODR^=1<<14;

		x += 1;
		FlipBuffGraphics(WaitForVerticalSync);
	}
}
