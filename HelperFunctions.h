void printf1(const char * format, ... );
void printf2(const char * format, ... );
void printf3(const char * format, ... );
void printf4(const char * format, ... );
void printf5(const char * format, ... );
void printf6(const char * format, ... );

int iabs(int n);	//converts all numbers to the positive value of itself (whole numbers only)
double fAbs(double n);	//converts all numbers to the positive value of itself including floating point

double ToDeg(double angle);
double ToRad(double angle);

int imap(int value, int fromLow, int fromHigh, int toLow, int toHigh);
double fmap(double value, double fromLow, double fromHigh, double toLow, double toHigh);
int iconstrain(int value, int min, int max);
double fconstrain(double value, double min, double max);
int imin(int value, int min);
int imax(int value, int max);

unsigned int cmpstr(char str1[], char str2[]);
int rnd(void);
int RandInt(int min, int max);	//outputs a number between (and including) the min and max values, min value must be supplied first
int average(int data[], unsigned length);	//integer average (int array, number of entries)
int median(int data[], unsigned int length);	//integer median (int array, number of entries)
void sort(int data[], unsigned int length, int dir);	//integer sort, data is returned to source array (int array, length, direction 0 for lower to higher, 1 for higher to lower)

unsigned int IsDigit(char chr);	//outputs 1 if a character is neumeric(0 to 9) and 0 if not
unsigned int IsNumeric(char numberstr[]);	//outputs 1 is a string is number, number may contain a +/- at the start and , in the middle, and one decimal point (.)
unsigned int IsHexNibble(char chr);	//outputs 1 is a string is 0-F
unsigned int IsHex(char numberstr[]);	//outputs 1 is a string is hex, eg: FF 0x9C +0x9C -0xDEADBEEF -A
unsigned int IsBin(char numberstr[]);	//outputs 1 if a string in binary eg 1010 0b1010 -0101 -0b10110110

int StringToInt(char string[]);	//converts a number in a string (0x00 terminated) to a signed int
void IntToString(int val, char outputstr[]);	//destination will be 0 terminated (array must be large enough to fit this)
void FloatToString(double value, unsigned int decimals, char outputstr[]);	//value, decimals to show, output string. destination will be 0 terminated (array must be large enough to fit this)
double StringToDouble(char string[]);	//converts a number in a string (0x00 terminated) to a double (64 bit float)

unsigned short isWhiteSpace(char chr);	//checks if a character is 1 of ' '  '\t'  '\n'  '\r' and returns 1 or 0 accordingly
int IsWithinString(char inputstr[], char matchstr[]);	//outputs the start position if found else outputs -1
unsigned int StringLength(char string[]);
void CopyString(char source[], unsigned int sourcestart, char destination[], unsigned int destinationstart);	//source array, begin read from,, destination array, begin write from. destination will be 0 terminated (array must be large enough to fit this)

void swap(int *x, int *y);
int hi8(int n);
int lo8(int n);
void FailWith(char string[],int e);

// Max and min will evaluate inputs twice so beware of functions with side effects
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

// Return the length of an array, only works in the function where the array was decleared
#define ArrayLength(array) (sizeof(array)/sizeof((array)[0]))
