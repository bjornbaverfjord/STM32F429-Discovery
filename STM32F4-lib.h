#include <stdarg.h>
#include <stdio.h>

//this contains STM32F4 hardware functions only
//for helper functions such as printf or math functions, see HelperFunctions.h
//for functions that use the external onboard hardware, see DiscoveryFunctions.h

//Defines
#define DAC_Mono1 0
#define DAC_Mono2 1
#define DAC_Stereo 2
#define DAC_Unbuffered 0
#define DAC_Buffered 1

#define ADC_12Bit 0UL
#define ADC_10Bit 1UL
#define ADC_8Bit 2UL
#define ADC_6Bit 3UL
#define ADC_SampleTime_3cycles 0
#define ADC_SampleTime_15cycles 1
#define ADC_SampleTime_28cycles 2
#define ADC_SampleTime_56cycles 3
#define ADC_SampleTime_84cycles 4
#define ADC_SampleTime_112cycles 5
#define ADC_SampleTime_144cycles 6
#define ADC_SampleTime_480cycles 7

#define Capture_Rising_Edge 0
#define Capture_Falling_Edge 1
#define Capture_Both_Edges 2
#define Capture_Channel1 1
#define Capture_Channel2 2
#define Capture_Channel3 3
#define Capture_Channel4 4
#define Capture_Every_Event 0
#define Capture_Every_2nd_Event 1
#define Capture_Every_4th_Event 2
#define Capture_Every_8th_Event 3
#define Capture_TIM2_CH1_PA0 0
#define Capture_TIM2_CH1_PA5 1
#define Capture_TIM2_CH1_PA15 2
#define Capture_TIM2_CH2_PA1 3
#define Capture_TIM2_CH2_PB3 4
#define Capture_TIM2_CH3_PA2 5
#define Capture_TIM2_CH3_PB10 6
#define Capture_TIM2_CH4_PA3 7
#define Capture_TIM2_CH4_PB11 8
#define Capture_TIM5_CH1_PA0 0
#define Capture_TIM5_CH2_PA1 1
#define Capture_TIM5_CH3_PA2 2
#define Capture_TIM5_CH4_PA3 3	
#define Capture_TIM3_CH1_PA6 0
#define Capture_TIM3_CH1_PB4 1
#define Capture_TIM3_CH1_PC6 2
#define Capture_TIM3_CH2_PA7 3
#define Capture_TIM3_CH2_PB5 4
#define Capture_TIM3_CH2_PC7 5
#define Capture_TIM3_CH3_PB0 6
#define Capture_TIM3_CH3_PC8 7
#define Capture_TIM3_CH4_PB1 8
#define Capture_TIM3_CH4_PC9 9
#define Capture_TIM4_CH1_PB6 0
#define Capture_TIM4_CH1_PD12 1
#define Capture_TIM4_CH2_PB7 2
#define Capture_TIM4_CH2_PD13 3
#define Capture_TIM4_CH3_PB8 4
#define Capture_TIM4_CH3_PD14 5
#define Capture_TIM4_CH4_PB9 6
#define Capture_TIM4_CH4_PD15 7

#define PWM_Edge_Aligned_mode 0
#define PWM_Center_Aligned_mode 1

#define PWM_Polarity_Active_High 0
#define PWM_Polarity_Active_Low 1

#define PWM_TIM1_CH1_PA8 0
#define PWM_TIM1_CH1_PE9 1
#define PWM_TIM1_CH2_PA9 2
#define PWM_TIM1_CH2_PE11 3
#define PWM_TIM1_CH3_PA10 4
#define PWM_TIM1_CH3_PE13 5
#define PWM_TIM1_CH4_PA11 6
#define PWM_TIM1_CH4_PE14 7
#define PWM_TIM2_CH1_PA0 0
#define PWM_TIM2_CH1_PA5 1
#define PWM_TIM2_CH2_PA1 2
#define PWM_TIM2_CH2_PB3 3
#define PWM_TIM2_CH3_PA2 4
#define PWM_TIM2_CH3_PB10 5
#define PWM_TIM2_CH4_PA3 6
#define PWM_TIM2_CH4_PB11 7
#define PWM_TIM3_CH1_PA6 0
#define PWM_TIM3_CH1_PB4 1
#define PWM_TIM3_CH1_PC6 2
#define PWM_TIM3_CH2_PA7 3
#define PWM_TIM3_CH2_PB5 4
#define PWM_TIM3_CH2_PC7 5
#define PWM_TIM3_CH3_PB0 6
#define PWM_TIM3_CH4_PB1 7
#define PWM_TIM4_CH1_PB6 0
#define PWM_TIM4_CH1_PD12 1
#define PWM_TIM4_CH2_PB7 2
#define PWM_TIM4_CH2_PD13 3
#define PWM_TIM4_CH3_PB8 4
#define PWM_TIM4_CH3_PD14 5
#define PWM_TIM4_CH4_PB9 6
#define PWM_TIM4_CH4_PD15 7
#define PWM_TIM5_CH1_PA0 0
#define PWM_TIM5_CH1_PH10 1
#define PWM_TIM5_CH2_PA1 2
#define PWM_TIM5_CH2_PH11 3
#define PWM_TIM5_CH3_PA2 4
#define PWM_TIM5_CH3_PH12 5
#define PWM_TIM5_CH4_PA4 6
#define PWM_TIM5_CH4_PI0 7
#define PWM_TIM9_CH1_PA2 0
#define PWM_TIM9_CH1_PE5 1
#define PWM_TIM9_CH2_PA3 2
#define PWM_TIM9_CH2_PE6 3

#define PWM_Update_Enable 1
#define PWM_Update_Disable 0


// Macros for setting and clearing single GPIO pins as fast as possible
// Any attempts at reliably inline these as functions have failed... Performance is critical in this case!
#define SetPin(portbase,pin) (*((volatile unsigned int *) (portbase + BSRR)) = (1 << pin))
#define ClearPin(portbase, pin) (*((volatile unsigned int *) (portbase + BSRR)) = (1 << (pin + 16)))

// Macros for translating a RAM or peripheral address and bit number to the bit-band alias address of the bit
#define BITBAND_SRAM_REF  0x20000000
#define BITBAND_SRAM_BASE 0x22000000
#define BITBAND_SRAM(a,b) ((BITBAND_SRAM_BASE + ((a - BITBAND_SRAM_REF) * 32) + (b * 4)))
#define BITBAND_PERI_REF  0x40000000
#define BITBAND_PERI_BASE 0x42000000
#define BITBAND_PERI(a,b) ((BITBAND_PERI_BASE + ((a - BITBAND_PERI_REF) * 32) + (b * 4)))

// Macros for accessing single bits
// Example: setBitP(PORTA + BSRR, 9);
// The write is done using read-modify-write and may be slower than other methods
#define readBitR(adr,bit) (*((volatile unsigned int *) BITBAND_SRAM(adr,bit)))
#define writeBitR(adr,bit,value) (*((volatile unsigned int *) BITBAND_SRAM(adr,bit)) = value)
#define setBitR(adr,bit) (writeBitR(adr,bit,1))
#define clearBitR(adr,bit) (writeBitR(adr,bit,0))

#define readBitP(adr,bit) (*((volatile unsigned int *) BITBAND_PERI(adr,bit)))
#define writeBitP(adr,bit,value) (*((volatile unsigned int *) BITBAND_PERI(adr,bit)) = value)
#define setBitP(adr,bit) (writeBitP(adr,bit,1))
#define clearBitP(adr,bit) (writeBitP(adr,bit,0))

//RTC
#define RTCAM 0
#define RTCPM 1
#define RTCMonday 1
#define RTCTuesday 2
#define RTCWednesday 3
#define RTCThursday 4
#define RTCFriday 5
#define RTCSaturday 6
#define RTCSunday 7

struct RTCType
{
	unsigned int AMPM;
	unsigned int Hours;
	unsigned int Minutes;
	unsigned int Seconds;
	unsigned int Year;
	unsigned int DayOfWeek;
	unsigned int Month;
	unsigned int Date;
};


//CLOCKS---------------------------------------------------------------
void EnableFPU(void);
void TestSysClock(void);	//testing SYSCLK/5 on PC9
unsigned int GetSYSCLKFreq(void);
unsigned int GetAHBFreq(void);
unsigned int GetAPB1Freq(void);	//low speed
unsigned int GetAPB2Freq(void);	//high speed

//GOIO---------------------------------------------------------------
void ResetIOToInput(void);	//turn off jtag and set everythnig as input except SWD pins (dissabled the port clocks when complete, should be done before port init)

void ConfigPinOnPort(unsigned int port, unsigned int pin, char c);

//Configure a given port from a 16 character string
//'a' - Analog function floating

//'i' - input floating
//'u' - input pull-up
//'d' - input pull-down

//'l' - output low speed, 2 MHz
//'m' - output medium speed, 25 MHz
//'f' - output fast speed, 50 MHz
//'h' - output high speed, 100 MHz
//'o' - output high speed, 100 MHz

//'L' - output low speed open-drain, 2 MHz
//'M' - output medium speed open-drain, 25 MHz
//'F' - output fast speed open-drain, 50 MHz
//'H' - output high speed open-drain, 100 MHz

//'A' - output low speed open-drain, pull up 2 MHz
//'D' - output medium speed open-drain, pull up 25 MHz
//'P' - output fast speed open-drain, pull up 50 MHz
//'p' - output high speed open-drain, pull up 100 MHz

//'n' - alternate function, 2 MHz
//'r' - alternate function, 25 MHz
//'e' - alternate function, 50 MHz
//'t' - alternate function, 100 MHz

//'N' - alternate function open-drain, 2 MHz
//'R' - alternate function open-drain, 25 MHz
//'E' - alternate function open-drain, 50 MHz
//'T' - alternate function open-drain, 100 MHz

//'I' - alternate function open-drain, pull up, 2 MHz
//'C' - alternate function open-drain, pull up, 25 MHz
//'c' - alternate function open-drain, pull up, 50 MHz
//'U' - alternate function open-drain, pull up, 100 MHz

//'-' - don't change
//' ' - ignore
//example: ConfigPortD("oooo ----  ---- ----");
void ConfigPortA(char *cfg);
void ConfigPortB(char *cfg);
void ConfigPortC(char *cfg);
void ConfigPortD(char *cfg);
void ConfigPortE(char *cfg);
void ConfigPortF(char *cfg);
void ConfigPortG(char *cfg);
void ConfigPortH(char *cfg);
void ConfigPortI(char *cfg);
void ConfigPort(char *cfg, volatile unsigned int *GPIO_MODER, volatile unsigned int *GPIO_OTYPER, volatile unsigned int *GPIO_OSPEEDR, volatile unsigned int *GPIO_PUPDR);
int ConfigPin(unsigned int pin, char c, volatile unsigned int *GPIO_MODER, volatile unsigned int *GPIO_OTYPER, volatile unsigned int *GPIO_OSPEEDR, volatile unsigned int *GPIO_PUPDR);
//void ConfigPinOnPort(char port, unsigned int pin, char c);	//port must be 'A' to 'I', pin is 0 to 15, c is the character for the pin mode, see ConfigPort


void TogglePin(char port, unsigned int pin);	//sets the pin low. port must be 'A' to 'I', pin is 0 to 15
unsigned int ReadPin(char port, unsigned int pin);	//returns 1 or 0. port must be 'A' to 'I', pin is 0 to 15

//AF---------------------------------------------------------------
void SetAFA(unsigned int pin, unsigned int AFnum);
void SetAFB(unsigned int pin, unsigned int AFnum);
void SetAFC(unsigned int pin, unsigned int AFnum);
void SetAFD(unsigned int pin, unsigned int AFnum);
void SetAFE(unsigned int pin, unsigned int AFnum);
void SetAFF(unsigned int pin, unsigned int AFnum);
void SetAFG(unsigned int pin, unsigned int AFnum);
void SetAFH(unsigned int pin, unsigned int AFnum);
void SetAFI(unsigned int pin, unsigned int AFnum);
void SetAF(volatile unsigned int *GPIO_AFRL, volatile unsigned int *GPIO_AFRH, unsigned int pin, unsigned int AFnum);

//USART/UART---------------------------------------------------------------
void InitUSART1(unsigned int baud);	//TX: PA9 RX: PA10
void InitUSART1Interrupts(unsigned int baud);
/*
example usage:
	InitUSART1Interrupts(9600);
	while(1);
}

void USART1_IRQHandler(void)
{
	DisableInterruptPosition(USART1_Interrupt_Position);
	
	ReadUSART1();	//RXNE flag is cleared by a read to the USART_DR register
	
	EnableInterruptPosition(USART1_Interrupt_Position);
}
*/
void WriteUSART1(unsigned char chrval);
unsigned char ReadUSART1(void);
unsigned int USART1DataIsAvailable(void);	// Return 1 if a character is in the buffer

void InitUSART2(unsigned int baud);	//TX: PA2 RX: PA3
void InitUSART2Interrupts(unsigned int baud);
void WriteUSART2(unsigned char chrval);
unsigned char ReadUSART2(void);
unsigned int USART2DataIsAvailable(void);	// Return 1 if a character is in the buffer

void InitUSART3(unsigned int baud);	//TX: PB10 RX: PB11
void InitUSART3Interrupts(unsigned int baud);
void WriteUSART3(unsigned char chrval);
unsigned char ReadUSART3(void);
unsigned int USART3DataIsAvailable(void);	// Return 1 if a character is in the buffer

void InitUART4(unsigned int baud);	//TX: PA0 RX: PA1
void InitUART4Interrupts(unsigned int baud);
void WriteUART4(unsigned char chrval);
unsigned char ReadUART4(void);
unsigned int UART4DataIsAvailable(void);	// Return 1 if a character is in the buffer

void InitUART5(unsigned int baud);	//TX: PC12 RX: PD2
void InitUART5Interrupts(unsigned int baud);
void WriteUART5(unsigned char chrval);
unsigned char ReadUART5(void);
unsigned int UART5DataIsAvailable(void);	// Return 1 if a character is in the buffer

void InitUSART6(unsigned int baud);	//TX: PC6 RX: PC7
void InitUSART6Interrupts(unsigned int baud);
void WriteUSART6(unsigned char chrval);
unsigned char ReadUSART6(void);
unsigned int USART6DataIsAvailable(void);	// Return 1 if a character is in the buffer

void InitUSART(unsigned int baud, volatile unsigned int *USART_BRR, volatile unsigned int *USART_CR1, volatile unsigned int *USART_CR2, volatile unsigned int *USART_CR3, volatile unsigned int *USART_SR);
void WriteUSART(unsigned char chrval, volatile unsigned int *USART_SR, volatile unsigned int *USART_DR);
unsigned char ReadUSART(volatile unsigned int *USART_SR, volatile unsigned int *USART_DR);
unsigned int USARTDataIsAvailable(volatile unsigned int *USART_SR);	// Return 1 if a byte is in the buffer

void PrintfUSART(volatile unsigned int *USART_SR, volatile unsigned int *USART_DR, const char * format, ... );
void Printf1(const char * format, ... );
void Printf2(const char * format, ... );
void Printf3(const char * format, ... );
void Printf4(const char * format, ... );
void Printf5(const char * format, ... );
void Printf6(const char * format, ... );

//RANDOM NUMBER GENERATOR---------------------------------------------------------------
void InitRandomNumberGenerator(void);
unsigned int ReadRandomNumberGenerator(void);	//returns a 32 bit random int

//DAC---------------------------------------------------------------
//Channel 1 on PA4, channel 2 on PA5
//modes: DAC_Mono1, DAC_Mono2, DAC_Stereo
//buffer: DAC_Unbuffered, DAC_Buffered
void InitDAC(unsigned int mode, unsigned int BufferEN);
void SetDAC1(unsigned int DACval);	//12 bit 0-4095
void SetDAC2(unsigned int DACval);	//12 bit 0-4095
void SetDACStereo(unsigned int DACval1, unsigned int DACval2);	//12 bit 0-4095

//ADC---------------------------------------------------------------
/*
Channel	ADC1	ADC2	ADC3
0				PA0		PA0		PA0
1				PA1		PA1		PA1
2				PA2		PA2		PA2
3				PA3		PA3		PA3
4				PA4		PA4	
5				PA5		PA5	
6				PA6		PA6	
7				PA7		PA7	
8				PB0		PB0	
9				PB1		PB1	
10			PC0		PC0		PC0
11			PC1		PC1		PC1
12			PC2		PC2		PC2
13			PC3		PC3		PC3
14			PC4		PC4	
15			PC5		PC5	
16			temperature sensor		
17			VREFINT		
18			VBAT/2

resolution is common to all ADCs
the less bits the faster the sampling time
must be one of:
ADC_12Bit
ADC_10Bit
ADC_8Bit
ADC_6Bit

SamplingTime must be one of:
ADC_SampleTime_3cycles
ADC_SampleTime_15cycles
ADC_SampleTime_28cycles
ADC_SampleTime_56cycles
ADC_SampleTime_84cycles
ADC_SampleTime_112cycles
ADC_SampleTime_144cycles
ADC_SampleTime_480cycles
*/

void InitADC1(unsigned int channels, unsigned int resolution, unsigned int SamplingTime);	//each bit represents a channel, LSB=channel 0
void InitADC2(unsigned int channels, unsigned int resolution, unsigned int SamplingTime);	//each bit represents a channel, LSB=channel 0
void InitADC3(unsigned int channels, unsigned int resolution, unsigned int SamplingTime);	//each bit represents a channel, LSB=channel 0

unsigned int ReadADC1(unsigned int channel);
unsigned int ReadADC2(unsigned int channel);
unsigned int ReadADC3(unsigned int channel);

//Interrupts---------------------------------------------------------------
void EnableInterruptPosition(unsigned int InterruptPosition);
void DisableInterruptPosition(unsigned int InterruptPosition);

//Timer---------------------------------------------------------------
//usually run at 84MHz (STM32F407) or 90Mhz (STM32F429)
//32 bit timers
void StartTimer2(unsigned int prescaler);
unsigned int ReadTimer2(void);
void ResetTimer2(void);
unsigned int ReadAndResetTimerOverflow2(void);
unsigned int GetTimer2InputClockFreq(void);
unsigned int GetTimer2Freq(void);

/*
interrupt frequency=(fCK_PSC/(TimerPrescaler+1))/(CountPeriod+1)
example 1Hz usage:

	StartTimer2InterruptPosition(10000-1, 8400-1);	//9000-1 for the STM32F429
	while(1);
}

void TIM2_IRQHandler(void)	//(replace 2 with timer number in use)
{
	DisableInterruptPosition(TIM2_Interrupt_Position);	//(replace 2 with timer number in use)
	
	//periodic code here
	
	TIM2_SR &= ~1;										//clear Update interrupt flag (replace 2 with timer number in use)
	EnableInterruptPosition(TIM2_Interrupt_Position);	//(replace 2 with timer number in use)
}
*/
void StartTimer2InterruptPosition(unsigned int TimerPrescaler, unsigned int CountPeriod);

void StartTimer5(unsigned int prescaler);
unsigned int ReadTimer5(void);
void ResetTimer5(void);
unsigned int ReadAndResetTimerOverflow5(void);
unsigned int GetTimer5InputClockFreq(void);
unsigned int GetTimer5Freq(void);
void StartTimer5InterruptPosition(unsigned int TimerPrescaler, unsigned int CountPeriod);

//16 bit timers
void StartTimer3(unsigned int prescaler);
unsigned int ReadTimer3(void);
void ResetTimer3(void);
unsigned int ReadAndResetTimerOverflow3(void);
unsigned int GetTimer3InputClockFreq(void);
unsigned int GetTimer3Freq(void);

void StartTimer4(unsigned int prescaler);
unsigned int ReadTimer4(void);
void ResetTimer4(void);
unsigned int ReadAndResetTimerOverflow4(void);
unsigned int GetTimer4InputClockFreq(void);
unsigned int GetTimer4Freq(void);

void StartTimer9(unsigned int prescaler);
unsigned int ReadTimer9(void);
void ResetTimer9(void);
unsigned int ReadAndResetTimerOverflow9(void);
unsigned int GetTimer9InputClockFreq(void);
unsigned int GetTimer9Freq(void);

void StartTimer10(unsigned int prescaler);
unsigned int ReadTimer10(void);
void ResetTimer10(void);
unsigned int ReadAndResetTimerOverflow10(void);
unsigned int GetTimer10InputClockFreq(void);
unsigned int GetTimer10Freq(void);

void StartTimer11(unsigned int prescaler);
unsigned int ReadTimer11(void);
void ResetTimer11(void);
unsigned int ReadAndResetTimerOverflow11(void);
unsigned int GetTimer11InputClockFreq(void);
unsigned int GetTimer11Freq(void);

void StartTimer12(unsigned int prescaler);
unsigned int ReadTimer12(void);
void ResetTimer12(void);
unsigned int ReadAndResetTimerOverflow12(void);
unsigned int GetTimer12InputClockFreq(void);
unsigned int GetTimer12Freq(void);

void StartTimer13(unsigned int prescaler);
unsigned int ReadTimer13(void);
void ResetTimer13(void);
unsigned int ReadAndResetTimerOverflow13(void);
unsigned int GetTimer13InputClockFreq(void);
unsigned int GetTimer13Freq(void);

void StartTimer14(unsigned int prescaler);
unsigned int ReadTimer14(void);
void ResetTimer14(void);
unsigned int ReadAndResetTimerOverflow14(void);
unsigned int GetTimer14InputClockFreq(void);
unsigned int GetTimer14Freq(void);

void StartTimer(unsigned int prescaler, volatile unsigned int *TIM_CR1, volatile unsigned int *TIM_PSC);	//timer runs at twice the APBx bus speed when bus divider>1. pescaler is 16 bit, and must be between 0 and 0xFFFF CK_CNT=fCK_PSC/(prescaler+1)
unsigned int ReadAndResetTimerOverflow(volatile unsigned int *TIM_SR);
void StartTimerInterrupt(unsigned int TimerPrescaler, unsigned int CountPeriod, volatile unsigned int *TIM_CR1, volatile unsigned int *TIM_PSC, volatile unsigned int *TIM_ARR, volatile unsigned int *TIM_DIER, volatile unsigned int *TIM_EGR, unsigned int InterruptPosition);

//capture---------------------------------------------------------------
/*
TimerPrescaler is 16 bit, timer clock is usually 84Mhz (STM32F407) or 90Mhz (STm32F429), see GetTimer1xFreq

CaptureChannel must be one of: Capture_Channel1		Capture_Channel2		Capture_Channel3		Capture_Channel4
No more than 1 channel per timer should be configured.

Note, PinMapping must use the same channel number as used in CaptureChannel
PinMapping must be one of:
channel 1:	Capture_TIM2_CH1_PA0
						Capture_TIM2_CH1_PA5
						Capture_TIM2_CH1_PA15
						
channel 2:	Capture_TIM2_CH2_PA1
						Capture_TIM2_CH2_PB3

channel 3:	Capture_TIM2_CH3_PA2
						Capture_TIM2_CH3_PB10
						
channel 4:	Capture_TIM2_CH4_PA3
						Capture_TIM2_CH4_PB11
						
filter:
0: No filter, sampling is done at fDTS	8: fSAMPLING=fDTS/8, N=6
1: fSAMPLING=fCK_INT, N=2 							9: fSAMPLING=fDTS/8, N=8
2: fSAMPLING=fCK_INT, N=4 							10: fSAMPLING=fDTS/16, N=5
3: fSAMPLING=fCK_INT, N=8 							11: fSAMPLING=fDTS/16, N=6
4: fSAMPLING=fDTS/2, N=6 								12: fSAMPLING=fDTS/16, N=8
5: fSAMPLING=fDTS/2, N=8 								13: fSAMPLING=fDTS/32, N=5
6: fSAMPLING=fDTS/4, N=6 								14: fSAMPLING=fDTS/32, N=6
7: fSAMPLING=fDTS/4, N=8 								15: fSAMPLING=fDTS/32, N=8

edge must be one of: Capture_Rising_Edge		Capture_Falling_Edge		Capture_Both_Edges

edgeprescaler must be one of: Capture_Every_Event		Capture_Every_2nd_Event		Capture_Every_4th_Event		Capture_Every_8th_Event
*/
void InitCaptureT2(unsigned int TimerPrescaler, unsigned int CaptureChannel, unsigned int PinMapping, unsigned int filter, unsigned int edge, unsigned int edgeprescaler);

/*
See InitCaptureT2 for parameter details
PinMapping must be one of:
channel 1:	Capture_TIM5_CH1_PA0
channel 2:	Capture_TIM5_CH2_PA1
channel 3:	Capture_TIM5_CH3_PA2
channel 4:	Capture_TIM5_CH4_PA3
*/
void InitCaptureT5(unsigned int TimerPrescaler, unsigned int CaptureChannel, unsigned int PinMapping, unsigned int filter, unsigned int edge, unsigned int edgeprescaler);

/*
See InitCaptureT2 for parameter details
PinMapping must be one of:
channel 1:	Capture_TIM3_CH1_PA6
						Capture_TIM3_CH1_PB4
						Capture_TIM3_CH1_PC6
						
channel 2:	Capture_TIM3_CH2_PA7
						Capture_TIM3_CH2_PB5
						Capture_TIM3_CH2_PC7
						
channel 3:	Capture_TIM3_CH3_PB0
						Capture_TIM3_CH3_PC8
						
channel 4:	Capture_TIM3_CH4_PB1
						Capture_TIM3_CH4_PC9
*/
void InitCaptureT3(unsigned int TimerPrescaler, unsigned int CaptureChannel, unsigned int PinMapping, unsigned int filter, unsigned int edge, unsigned int edgeprescaler);

/*
See InitCaptureT2 for parameter details
PinMapping must be one of:
channel 1:	Capture_TIM4_CH1_PB6
						Capture_TIM4_CH1_PD12
						
channel 2:	Capture_TIM4_CH2_PB7
						Capture_TIM4_CH2_PD13
						
channel 2:	Capture_TIM4_CH3_PB8
						Capture_TIM4_CH3_PD14
						
channel 3:	Capture_TIM4_CH4_PB9
						Capture_TIM4_CH4_PD15
*/
void InitCaptureT4(unsigned int TimerPrescaler, unsigned int CaptureChannel, unsigned int PinMapping, unsigned int filter, unsigned int edge, unsigned int edgeprescaler);

//CaptureChannel must be one of: Capture_Channel1		Capture_Channel2		Capture_Channel3		Capture_Channel4
unsigned int CheckCaptureT2(unsigned int CaptureChannel);
unsigned int CheckCaptureT5(unsigned int CaptureChannel);
unsigned int CheckCaptureT3(unsigned int CaptureChannel);
unsigned int CheckCaptureT4(unsigned int CaptureChannel);

//CaptureChannel must be one of: Capture_Channel1		Capture_Channel2		Capture_Channel3		Capture_Channel4
void WaitCaptureT2(unsigned int CaptureChannel);
void WaitCaptureT5(unsigned int CaptureChannel);
void WaitCaptureT3(unsigned int CaptureChannel);
void WaitCaptureT4(unsigned int CaptureChannel);

//CaptureChannel must be one of: Capture_Channel1		Capture_Channel2		Capture_Channel3		Capture_Channel4
void ClearCaptureT2(unsigned int CaptureChannel);
void ClearCaptureT5(unsigned int CaptureChannel);
void ClearCaptureT3(unsigned int CaptureChannel);
void ClearCaptureT4(unsigned int CaptureChannel);

//CaptureChannel must be one of: Capture_Channel1		Capture_Channel2		Capture_Channel3		Capture_Channel4
//these also clear the capture flag once read
unsigned int ReadCaptureT2(unsigned int CaptureChannel);
unsigned int ReadCaptureT5(unsigned int CaptureChannel);
unsigned int ReadCaptureT3(unsigned int CaptureChannel);
unsigned int ReadCaptureT4(unsigned int CaptureChannel);

void InitCapture(unsigned int CaptureChannel, volatile unsigned int *TIM_CCMR1, volatile unsigned int *TIM_CCMR2, volatile unsigned int *TIM_CCER, unsigned int filter, unsigned int edge, unsigned int edgeprescaler);
unsigned int CheckCapture(volatile unsigned int *TIM_SR, unsigned int CaptureChannel);
void ClearCapture(volatile unsigned int *TIM_SR, unsigned int CaptureChannel);

/*
prescaler is 16 bit
timers 2,3,4,6 are usually clocked at 84MHz (STM32F407) or 90MHz (STM32F429)
timers 1,9 are usually clocked at 160MHz (STM32F407) or 180MHz (STM32F429)
PWMMode is either PWM_Edge_Aligned_mode or PWM_Center_Aligned_mode
PWMPolarity is either PWM_Polarity_Active_High or PWM_Polarity_Active_Low
period counter counts from 0 to this preset and back to 0, giving n+1 counts
initduty is the number of counts that will be high (in active high mode). Value must not be larger than counts per period (period+1 because it counts from 0 to n)
PinMapping must be in the form of: PWM_TIM2_CH1_PA5, which sets the channel and pin. See table below

timer/channel	pin 1	pin 2	pin3
TIM1_CH1			PA8		PE9
TIM1_CH2			PA9		PE11
TIM1_CH3			PA10	PE13
TIM1_CH4			PA11	PE14

TIM2_CH1			PA0		PA5
TIM2_CH2			PA1		PB3
TIM2_CH3			PA2		PB10
TIM2_CH4			PA3		PB11

TIM3_CH1			PA6		PB4		PC6
TIM3_CH2			PA7		PB5		PC7
TIM3_CH3			PB0
TIM3_CH4			PB1

TIM4_CH1			PB6		PD12
TIM4_CH2			PB7		PD13
TIM4_CH3			PB8		PD14
TIM4_CH4			PB9		PD15

TIM5_CH1			PA0		PH10
TIM5_CH2			PA1		PH11
TIM5_CH3			PA2		PH12
TIM5_CH4			PA4		PI0

TIM9_CH1			PA2		PE5
TIM9_CH2			PA3		PE6

50Hz example: InitPWM2(9000, PWM_Edge_Aligned_mode, PWM_Polarity_Active_High, 200-1, 100-1,PWM_TIM2_CH1_PA5);
*/
void InitPWM1(unsigned int prescaler, unsigned int PWMMode, unsigned int PWMPolarity, unsigned int period, unsigned int initduty, unsigned int PinMapping);	//PWM channels on timer1 (16 bit)
void InitPWM2(unsigned int prescaler, unsigned int PWMMode, unsigned int PWMPolarity, unsigned int period, unsigned int initduty, unsigned int PinMapping);	//PWM channels on timer2 (32 bit)
void InitPWM3(unsigned int prescaler, unsigned int PWMMode, unsigned int PWMPolarity, unsigned int period, unsigned int initduty, unsigned int PinMapping);	//PWM channels on timer3 (16 bit)
void InitPWM4(unsigned int prescaler, unsigned int PWMMode, unsigned int PWMPolarity, unsigned int period, unsigned int initduty, unsigned int PinMapping);	//PWM channels on timer4 (32 bit)
void InitPWM5(unsigned int prescaler, unsigned int PWMMode, unsigned int PWMPolarity, unsigned int period, unsigned int initduty, unsigned int PinMapping);	//PWM channels on timer5 (16 bit)
void InitPWM9(unsigned int prescaler, unsigned int PWMMode, unsigned int PWMPolarity, unsigned int period, unsigned int initduty, unsigned int PinMapping);	//PWM channels on timer9 (16 bit)

void SetPWM1(unsigned int, unsigned int);	//channel, duty
void SetPWM2(unsigned int, unsigned int);	//channel, duty
void SetPWM3(unsigned int, unsigned int);	//channel, duty
void SetPWM4(unsigned int, unsigned int);	//channel, duty
void SetPWM5(unsigned int, unsigned int);	//channel, duty
void SetPWM9(unsigned int, unsigned int);	//channel, duty

/*
PWM_Update_Enable or PWM_Update_Disable

allows synchronous updating of multiple PWM channels on the same timer
PWM remains running while updates are disabled
using SetPWM settings will be applied when updates are re-enabled

example usage:
PWM_UpdateEnable2(PWM_Update_Disable);
SetPWM2(1,100);
SetPWM2(2,110);
SetPWM2(3,120);
PWM_UpdateEnable2(PWM_Update_Enable);
*/
void PWM_UpdateEnable1(unsigned int);
void PWM_UpdateEnable2(unsigned int);
void PWM_UpdateEnable3(unsigned int);
void PWM_UpdateEnable4(unsigned int);
void PWM_UpdateEnable5(unsigned int);
void PWM_UpdateEnable9(unsigned int);

void InitPWM(volatile unsigned int *TIM_CR1, volatile unsigned int *TIM_PSC, volatile unsigned int *TIM_ARR, volatile unsigned int *TIM_CCMR1, volatile unsigned int *TIM_CCER, volatile unsigned int *TIM_CCMR2, volatile unsigned int *TIM_EGR, unsigned int prescaler, unsigned int channel, unsigned int PWMMode, unsigned int PWMPolarity, unsigned int period);	//timer runs at twice the APBx bus speed when bus divider>1. pescaler is 16 bit, and must be between 0 and 0xFFFF CK_CNT=fCK_PSC/(prescaler+1)

//RTC----------------------------------------------------------------------------------
//dayofweek begins with 1 being monday, so you must type 1 or RTCMonday
//DaylightSavingsOffset must be -1, 0 or 1
//(output) format should be RTCAM or RTCPM 
void InitRTC(unsigned int year, unsigned int month, unsigned int date, unsigned int DayOfWeek, unsigned int hours, unsigned int minutes, unsigned int seconds, int DaylightSavingsOffset, unsigned int format);
unsigned int IsRTCUpdated(void);	//outputs 1 if updated since last read else 0
struct RTCType ReadRTC(void);	//outputs AMPM (RTCAM=AM or 24 hour mode, RTCPM=PM), Hours, Minutes, Seconds, Year, DayOfWeek (1=monday or RTCMonday), Month, Date. this also resets the read flag


unsigned int configSYSTICK(unsigned int timebase);	// Configure the systick timer with a timebase in ns, return the true timebase
void waitsys (unsigned int ticks);	// Wait for n ticks of SYSTICK timer, period set with configSYSTICK(unsigned int timebase)
