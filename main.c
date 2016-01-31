#include "main.h"
#include "glcd.h"
#include "registers.h"
#include "STM32F4-lib.h"
#include "InterruptPositions.h"
#include "AlternateFunctions.h"
#include "InterruptPositions.h"
#include "HelperFunctions.h"
#include "math.h"

int main(void)
{
	struct ButtonType buttons[1];
	unsigned int TotalButtons=0;
	int PressedButton,x;
	
	configSYSTICK(1000);	// 1 us timer resolution
	
	InitGLCD(landscape);
	
	// Create two layer1 buffers and one layer2 buffer
	InitGraphics(2,1);
  
	ConfigPortG("-oo- ----  ---- ----");

	
	// Draw a picture on the visible layer2 buffer
	SetDrawBufferGraphics(GetLayer2BufferGraphics());
	ClearGLCD(LCD_COLOR_BLACK);
	PrintStringWithBGGLCD("                  Text!",0,0,1,LCD_COLOR_BLACK,LCD_COLOR_WHITE);
	CircleGLCD(159,119,100,LCD_COLOR_BLUE);
	PrintValueSevSegGLCD(1234, 160-50+2+10, 120-5-13-30, 20, 3, 2, LCD_COLOR_YELLOW);
	RectangleGLCD(160-50+2+14+5, 120-5-13-31, 160-50+2+10+58, 120-5-13-30+25, LCD_COLOR_RED);
	PrintStringWithBGGLCD("Hello World!",160-50,120-5-13,0,LCD_COLOR_WHITE,LCD_COLOR_GREEN);
	PrintStringGLCD("Hello World!",160-50,120-5,LCD_COLOR_WHITE);
	PrintFloatGLCD(3.14159,5,160-50+20,120-5+14,LCD_COLOR_GREEN);
	PrintString2GLCD("Hello World!",160-50+23,120-5+28,LCD_COLOR_WHITE);
	PrintString2WithBGGLCD("Text!", 160-50+23+14, 120-5+28+10, 0, LCD_COLOR_BLUE,LCD_COLOR_RED);
	
	// This is buggy now
	AddButtonGLCD(buttons, &TotalButtons, 0, 15, GetStringWidthGLCD("blink")+8, 32, "blink");
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
