#include "main.h"

#define portrait 0
#define landscape 1

#define FillBack 0
#define FillToEdge 1

#define CopyBuffers 1
#define ClearBufferAfterFlip 2
#define WaitForVerticalSync 4

//fonts
#define LCDFontVariableWidth 0
#define LCDFont7x11 1
#define LCDFont8x8 2
#define LCDFont8x12 3
#define LCDFont12x12 4
#define LCDFont16x24 5
#define LCDFontHebrew 6


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

//text
//these functions return the x of where the next character would go (assuming a 1 pixel gap)
//available fonts: LCDFontVariableWidth, LCDFont7x11, LCDFont8x8, LCDFont8x12, LCDFont12x12, LCDFont16x24, LCDFontHebrew
//use -1 for BackColour if you want a transparant background
unsigned int BmpCharacter(char character, int x1, int y1, unsigned int font, unsigned int TextColour, int BackColour);
unsigned short PrintfGLCD(int x1, int y1, unsigned int font, unsigned int TextColour, int BackColour, const char * format, ... );	//note: if using this with hebrew, create a string by char str[]={HEB_aleph, HEB_bet, HEB_gimel, HEB_dalet, 0}; it will display in reverse order
unsigned int PrintStringWrappedGLCD(char string[], unsigned short xpos, unsigned short ypos, unsigned short xmax, unsigned int font, unsigned int TextColour, int BackColour);	//string, x, y, xmax, font, text colour, back colour (xmax is the right hand limit for the text, returns (y<<16) | x of where the next character would go)
unsigned short PrintBinGLCD(unsigned int val, unsigned short xpos, unsigned short ypos, unsigned int font, unsigned int TextColour, int BackColour);			//value, x, y, 16 bit colour (prints a variable as binary)
unsigned int GetFontHeight(unsigned int font);
unsigned int GetFontWidth(unsigned int font);
unsigned int GetStringWidthGLCD(char string[], unsigned int font);
unsigned int GetCharacterWidthGLCD(char chr, unsigned int font);

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
