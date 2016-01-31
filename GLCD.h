#include "main.h"

#define portrait 0
#define landscape 1

#define FillBack 0
#define FillToEdge 1

#define CopyBuffers 1
#define ClearBufferAfterFlip 2
#define WaitForVerticalSync 4


/*
colours
LCD_COLOR_WHITE
LCD_COLOR_BLACK
LCD_COLOR_GREY
LCD_COLOR_BLUE
LCD_COLOR_BLUE2
LCD_COLOR_RED
LCD_COLOR_MAGENTA
LCD_COLOR_GREEN
LCD_COLOR_CYAN
LCD_COLOR_YELLOW
*/


struct xyType
{
	int x;
	int y;
};

struct BufferType
{
	int Address;			// Address of point (0,0)
	int Width;        // Width in number of pixels
	int Height;       // Height in number of pixels
	int PixelType;    // Pixeltype (LTDC_Pixelformat_RGB565)
	int Next;         // Next buffer in the flip chain
};

struct GraphicsType
{
	int Address;					// Address of Color Frame Buffer accessible by LCD-TFT Controller (LTDC)
	int Width;
	int Height;
	int NumberOfBuffers;  // Number of buffers
	int AllocatedBitMap;  // Bitmap of allocated framebuffers, 1 = allocated, 0 = free, bit 0 = lowest address 
	int Layer1Buffer;			// Buffer currently mapped to layer 1, -1 if none;
	int Layer1PixelType;
	int Layer2Buffer;			// Buffer currently mapped to layer 2, -1 if none;
	int Layer2PixelType;
	struct BufferType Buffer[32];
};



struct ButtonType
{
	unsigned short x;
	unsigned short y;
	unsigned short width;
	unsigned short height;
	char text[50];
};

void InitGLCD(unsigned int direction);	//portrait or landscape
void LCD_Init2(void);
void LCD_LayerInit2(void);
void LCD_PowerOn2(void);
void LCD_Write16(uint16_t value);
void SetPixelGLCD(int x, int y, unsigned short colour);
void ClearGLCD(unsigned short colour);
unsigned short ColourWordGLCD(unsigned char r, unsigned char g, unsigned char b);	//8 bit each
unsigned int GetScreenSizeGLCD(void);	//returns hhhhhhhhhhhhhhhh wwwwwwwwwwwwwwww (16 bit height and width)
int isVisibleGLCD(int x, int y);

void InitGraphics(int layer1buffers,int layer2buffers);		// Set up graphics buffers and default values
																													// n1 = number of buffers in the layer 1 flipchain
																													// n2 = number of buffers in the layer 2 flipchain

int AllocateBufferGraphics(void);													// Allocate a single buffer if available
void FreeBufferGraphics(int b);														// Deallocate a single buffer
int GetPixelSizeGraphics(int pixeltype);									// Return the size of a pixel in bytes
int AllocateFlipChainGraphics(int n);											// Allocate n linked buffers that will be visible in turn by FlipBuffGraphics
void FreeFlipChainGraphics(int n);												// Deallocate buffers in a flipchain
void SetLayer1ToBufferGraphics(int buffer);								// Set layer1 to a buffer and make DrawAddress point to it (sets the curently onscreen buffer for layer1)
void SetLayer2ToBufferGraphics(int buffer);								// Set layer2 to a buffer (sets the curently onscreen buffer for layer2)
int isValidBufferGraphics(int buffer);										// Check if a buffer exists and is allocated

void FlipBuffGraphics(int flags);													// Make the next buffer in the flipchain visible or stay on same if only one buffer in the chain
																													// Flips both layer 1 & 2, and sets drawing buffer to next in layer 1 buffer chain
																													// Flags is a bitwise multiparameter value 
																													// Flags = CopyBuffers | ClearBufferAfterFlip | WaitForVerticalSync
																													
int GetAddressOfBufferGraphics(int n);										// Return the address of a buffer
void SetDrawBufferGraphics(int buffer);										// Set the DrawAddress to any buffer
void ResetDrawAddressGraphics(void);											// Set the DrawAddress to the current layer1 buffer (which is the next one in the flipchain from the currently onscreen buffer)

void SetTrasparencyGraphics(int t);												// Set mixture ration between layer1 and layer2
																													// 255 = only layer 1
																													// 128 = 50%
																													//   0 = only layer2
																													
int GetNextBufferGraphics(int n);													// Get the next buffer in a flipchain
int GetLayer1BufferGraphics(void);												// Return the buffer showing on layer1
int GetLayer2BufferGraphics(void);												// Return the buffer showing on layer2
int GetNextLayer1BufferGraphics(void);										// Return the next buffer showing on layer1
int GetNextLayer2BufferGraphics(void);										// Return the next buffer showing on layer2
int RGB888OfRGB565(int c);

void LineGLCD(int x1, int y1, int x2, int y2, unsigned short colour);
void RectangleGLCD(int x1, int y1, int x2, int y2,  unsigned short colour);	//x1, y1, x2, y2, 16 bit colour (also uses inclusive x2,y2)
void RectangleFilledGLCD(int x1, int y1, int x2, int y2,  unsigned short colour);	//x1, y1, x2, y2, 16 bit colour	(also uses inclusive x2,y2)
void CircleGLCD(int x0, int y0, int radius, unsigned short colour);	//mid x, mid y, radius, 16 bit colour
void CircleFilledGLCD(int x0, int y0, int radius, unsigned short colour);	//mid x, mid y, radius, 16 bit colour
void TriangleGLCD(int x1, int y1, int x2, int y2, int x3, int y3, unsigned short colour);	//x1,y1, x2,y2, x3,y3, 16 bit colour
void TriangleFilledGLCD(int x1, int y1, int x2, int y2, int x3, int y3, unsigned short colour);
void PolygonGLCD(struct xyType points[],int count,short colour);
void PolygonConvexFilledGLCD(struct xyType points[],int count,short colour);
void PolygonLineGLCD(int x1, int y1, int x2, int y2, short xLeft[], short xRight[]);
void RasterizePolygonConvexGLCD(short xLeft[], short xRight[], short colour);

void RectangleFilledWHGLCD(int x1, int y1, int Width, int Height, unsigned short colour);
void WaitForVSyncStart(void);
void WaitForVSyncGLCD(void);
void WaitForHSyncGLCD(void);
void TranslateXYGLCD(int *x, int *y);


//7x11 font functions
//these functions return the x of where the next character would go (assuming a 1 pixel gap)
unsigned short PrintCharGLCD(char chr, unsigned short xpos, unsigned short ypos, unsigned short colour);		//char, x, y, 16 bit colour (prints a character with its top left at x,y)
unsigned short PrintStringGLCD(char string[], unsigned short xpos, unsigned short ypos, unsigned short colour);	//string, x, y, 16 bit colour, returns finishing x+1 (30x29 chars/screen using 7x11 characters)
unsigned int PrintStringWrappedGLCD(char string[], unsigned short xpos, unsigned short ypos, unsigned short xmax, unsigned short colour);//string, x, y, xmax, 16 bit colour (xmax is the right hand limit for the text, returns (y<<16) | x of where the next character would go)
unsigned short PrintValueGLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short colour);		//value, x, y, 16 bit colour (prints a variable as decimal)
unsigned short PrintValue2DigitsGLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short colour);		//value, x, y, 16 bit colour (prints a variable as decimal)
unsigned short PrintFloatGLCD(double val, unsigned int decimals, unsigned short xpos, unsigned short ypos, unsigned short colour);	//value, digits to show, x, y, 16 bit colour (prints a value as floating point (xx.yy)) 
unsigned short PrintHexGLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short colour);			//value, x, y, 16 bit colour (prints a 32 bit variable as hex)
unsigned short PrintHexByteGLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short colour);		//value, x, y, 16 bit colour (prints a byte variable as hex)
unsigned short PrintBinGLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short colour);			//value, x, y, 16 bit colour (prints a variable as binary)
unsigned short PrintBinOf8GLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short colour);			//value, x, y, 16 bit colour (prints a variable as binary)
unsigned short GetStringWidthGLCD(char string[]);													//given a string, returns the number of pixels in width (without the ending space)

unsigned short PrintCharWithBGGLCD(char chr, unsigned short xpos, unsigned short ypos, unsigned short fillmode, unsigned short colour, unsigned short BGcolour);		//char, x, y, fillmode must be FillBack or FillToEdge. 16 bit text colour, background colour (prints a character with its top left at x,y)
unsigned short PrintStringWithBGGLCD(char string[], unsigned short xpos, unsigned short ypos, unsigned short fillmode, unsigned short colour, unsigned short BGcolour);	//string, x, y, fillmode must be FillBack or FillToEdge. 16 bit text colour, background colour. returns finishing x+1 (30x29 chars/screen using 7x11 characters)
unsigned short PrintValueWithBGGLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short fillmode, unsigned short colour, unsigned short BGcolour);		//value, x, y, fillmode must be FillBack or FillToEdge. 16 bit text colour, background colour (prints a variable as decimal)
unsigned short PrintValue2DigitsWithBGGLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short fillmode, unsigned short colour, unsigned short BGcolour);	//value, x, y, fillmode must be FillBack or FillToEdge. 16 bit colour, background colour (prints a variable as decimal)
unsigned short PrintFloatWithBGGLCD(double val, unsigned int decimals, unsigned short xpos, unsigned short ypos, unsigned short fillmode, unsigned short colour, unsigned short BGcolour);	//value, digits to show, x, y, fillmode must be FillBack or FillToEdge. background colour (prints a value as floating point (xx.yy)) 
unsigned short PrintHexWithBGGLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short fillmode, unsigned short colour, unsigned short BGcolour);			//value, x, y, fillmode must be FillBack or FillToEdge. 16 bit text colour, background colour (prints a 32 bit variable as hex)
unsigned short PrintHexByteWithBGGLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short fillmode, unsigned short colour, unsigned short BGcolour);		//value, x, y, fillmode must be FillBack or FillToEdge. 16 bit text colour, background colour (prints a byte variable as hex)
unsigned short PrintBinWithBGGLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short fillmode, unsigned short colour, unsigned short BGcolour);			//value, x, y, fillmode must be FillBack or FillToEdge. 16 bit text colour, background colour (prints a variable as binary)

//8x1-9 variable width font functions
//these functions return the x of where the next character would go (assuming a 1 pixel gap)
unsigned short PrintChar2GLCD(char chr, unsigned short xpos, unsigned short ypos, unsigned short colour);		//character, x, y, 16 bit colour (8 x 1-9 variable width font, returns the width of each character, this font is very small)
unsigned short GetStringWidth2GLCD(char string[]);													//given a string, returns the number of pixels in width (without the ending space)
unsigned short PrintString2GLCD(char string[], unsigned short xpos, unsigned short ypos, unsigned short colour);	//string, x, y, 16 bit colour, returns finishing x+1 (8 x 1-9 variable width, this font is very small)
unsigned int PrintStringWrapped2GLCD(char string[], unsigned short xpos, unsigned short ypos, unsigned short xmax, unsigned short colour);//string, x, y, xmax, 16 bit colour (xmax is the right hand limit for the text, returns (y<<16) | x of where the next character would go)
unsigned short PrintValue2GLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short colour);		//value, x, y, 16 bit colour (prints a variable as decimal)
unsigned short PrintFloat2GLCD(double val, unsigned int decimals, unsigned short xpos, unsigned short ypos, unsigned short colour);//value, digits to show, x, y, 16 bit colour (prints a value as floating point (xx.yy)) 
unsigned short PrintHex2GLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short colour);			//value, x, y, 16 bit colour (prints a 32 bit variable as hex)
unsigned short PrintHexByte2GLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short colour);		//value, x, y, 16 bit colour (prints a byte variable as hex)
unsigned short PrintBin2GLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short colour);			//value, x, y, 16 bit colour (prints a variable as binary

unsigned short PrintChar2WithBGGLCD(char chr, unsigned short xpos, unsigned short ypos, unsigned short fillmode, unsigned short colour, unsigned short BGcolour);		//char, x, y, fillmode must be FillBack or FillToEdge. 16 bit text colour, background colour (prints a character with its top left at x,y)
unsigned short PrintString2WithBGGLCD(char string[], unsigned short xpos, unsigned short ypos, unsigned short fillmode, unsigned short colour, unsigned short BGcolour);	//string, x, y, fillmode must be FillBack or FillToEdge. 16 bit text colour, background colour. returns finishing x+1 (30x29 chars/screen using 7x11 characters)
unsigned short PrintValue2WithBGGLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short fillmode, unsigned short colour, unsigned short BGcolour);		//value, x, y, fillmode must be FillBack or FillToEdge. 16 bit text colour, background colour (prints a variable as decimal)
unsigned short PrintFloat2WithBGGLCD(double val, unsigned int decimals, unsigned short xpos, unsigned short ypos, unsigned short fillmode, unsigned short colour, unsigned short BGcolour);	//value, digits to show, x, y, fillmode must be FillBack or FillToEdge. 16 bit text colour, background colour (prints a value as floating point (xx.yy)) 
unsigned short PrintHex2WithBGGLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short fillmode, unsigned short colour, unsigned short BGcolour);			//value, x, y, fillmode must be FillBack or FillToEdge. 16 bit text colour, background colour (prints a 32 bit variable as hex)
unsigned short PrintHexByte2WithBGGLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short fillmode, unsigned short colour, unsigned short BGcolour);		//value, x, y, fillmode must be FillBack or FillToEdge. 16 bit text colour, background colour (prints a byte variable as hex)
unsigned short PrintBin2WithBGGLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short fillmode, unsigned short colour, unsigned short BGcolour);			//value, x, y, fillmode must be FillBack or FillToEdge. 16 bit text colour, background colour (prints a variable as binary)

//7 segment
unsigned short PrintValueSevSegGLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short height, unsigned short thickness, unsigned short gap, unsigned short colour);	//value, x, y, height, thickness, gap, colour
unsigned short PrintValue2DigitsSevSegGLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned short height, unsigned short thickness, unsigned short gap, unsigned short colour);	//value, x, y, height, thickness, gap, colour
unsigned short SevSegDigitGLCD(unsigned short digit, unsigned short x, unsigned short y, unsigned short height, unsigned short thickness, unsigned short gap, unsigned short colour);	//digit, x, y, height, thickness, gap, colour
void DrawSegmentGLCD(unsigned short x, unsigned short y, unsigned short length, unsigned short width, unsigned short type, unsigned short colour);	//x, y, length, width, type, colour

/*
usage:
static TP_STATE* TP_State;

TP_State = ReadTouch();
if(TP_State->TouchDetected){}
TP_State->X
TP_State->Y
*/
TP_STATE* ReadTouch(void);


/*buttons-------
example:
struct ButtonType buttons[50];
unsigned int TotalButtons=0;
int PressedButton;

AddButtonGLCD(buttons, &TotalButtons, 0, 0, 50, 16, "text");
DrawButtonsGLCD(buttons, TotalButtons);
FlipBuffGLCD(NoCopyBuffs, ClearAfterFlip, WaitForVSync);

while(1)
{
	PressedButton=GetPressedButtonGLCD(buttons, TotalButtons);
	if(PressedButton==0){}	//action for first button
}
*/

void AddButtonGLCD(struct ButtonType buttons[], unsigned int *TotalButtons, unsigned short x, unsigned short y, unsigned short width, unsigned short height, char text[]);	//this will increment TotalButtons
void DrawButtonsGLCD(struct ButtonType buttons[], unsigned int TotalButtons);
//returns the button number pressed (0 is the 1st button) or -1 for none (non waiting if nothing pressed, but waits for de-press),
//also flips layers (to the currently visible one) if required (and back again) to display button pressed
int GetPressedButtonGLCD(struct ButtonType buttons[], unsigned int TotalButtons);	

void DrawButtonGLCD(unsigned short x, unsigned short y, unsigned short width, unsigned short height, char text[]);
void DrawButtonPressedGLCD(unsigned short x, unsigned short y, unsigned short width, unsigned short height, char text[]);
//--------------
