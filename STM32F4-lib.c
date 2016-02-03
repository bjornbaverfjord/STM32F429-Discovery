#include "registers.h"
#include "STM32F4-lib.h"
#include "AlternateFunctions.h"
#include "InterruptPositions.h"
#include "HelperFunctions.h"

#define HSEfreq 8000000

unsigned int RTCyear=0;	//global variable

void EnableFPU(void)
{
		CPACR |= 0xFUL<<20;
}

void TestSysClock(void)		//testing SYSCLK/5 on PC9
{
	RCC_CFGR |= 7UL<<27;	//MCO2 prescaler division by 5
	RCC_CFGR &= ~(3UL<<30);	//System clock (SYSCLK) selected on Microcontroller clock output2 (PC9)
	RCC_AHB1ENR |= 1UL<<2; //IO port C clock enable
	GPIOC_OSPEEDR |= 3UL<<(9*2);	//PC9 100 MHz High speed on 30 pF (80 MHz Output max speed on 15 pF)
	GPIOC_MODER |= 2UL<<(9*2);	//PC9 in Alternate function mode
	GPIOC_AFRH &= ~(0xFUL<<4);	//set PC9 to AF function 0
}

void ResetIOToInput(void) //turn off jtag and set everythnig as input except SWD pins
{
	RCC_AHB1ENR |= 3;	//enable clocks for GIPOA and GPIOB
	GPIOA_MODER &= 0xFUL<<26;	//PA13 and PA14 needed for SWD debug
	GPIOA_PUPDR &= 0xFUL<<26;
	
	GPIOB_MODER=0;
	GPIOB_OSPEEDR=0;
	GPIOB_PUPDR=0;
	RCC_AHB1ENR &= ~3;	//disable clocks for GIPOA and GPIOB
}


/*
void SetPin(unsigned int portbase,int pin)
{
	(*((volatile unsigned int *) (portbase + BSRR)) = (1 << pin));
}

void ClearPin(unsigned int portbase, int pin)
{
	(*((volatile unsigned int *) (portbase + BSRR)) = (1 << (pin + 16)));
}
*/

//Configure a given port from a 16 character string
//'a' - Analog function floating

//'i' - input floating
//'u' - input pull-up
//'d' - input pull-down

//'l' - output low speed, 2 MHz
//'m' - output medium speed, 25 MHz
//'f' - output fast speed, 50 MHz
//'h' - output high speed, 100
//'o' - output high speed, 100

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
void ConfigPort(char *cfg, volatile unsigned int *GPIO_MODER, volatile unsigned int *GPIO_OTYPER, volatile unsigned int *GPIO_OSPEEDR, volatile unsigned int *GPIO_PUPDR)
{
	char c;
	unsigned int n=0;
	unsigned int i=0;
	
	while(cfg[n] != 0)
	{
		c=cfg[n];
		n += 1;
				
		if (c == ' ') continue;			// Ignore spaces that are inserted for readability
		
		i += 1;
		if (i > 16) break;								// There is only 16 pins to configure, error condition
		
		if (c == '-') continue;			// Dashes are no-operation
		
		ConfigPin(15-(i-1),c,GPIO_MODER, GPIO_OTYPER, GPIO_OSPEEDR, GPIO_PUPDR);
	}
}

//configures an an I/O pin
int ConfigPin(unsigned int pin, char c, volatile unsigned int *GPIO_MODER, volatile unsigned int *GPIO_OTYPER, volatile unsigned int *GPIO_OSPEEDR, volatile unsigned int *GPIO_PUPDR)
{
		switch (c)
		{
			case 'a': //Analog function floating
				*GPIO_MODER |= 3<<(pin<<1);	//Analog mode
				
				*GPIO_PUPDR &= ~(3<<(pin<<1));	//No pull-up, pull-down
			break;
			
			case 'i': //input floating
				*GPIO_MODER &= ~(3<<(pin<<1));	//Input
				
				*GPIO_PUPDR &= ~(3<<(pin<<1));	//No pull-up, pull-down
			break;
			case 'u': //input pull-up
				*GPIO_MODER &= ~(3<<(pin<<1));	//Input
				
				*GPIO_PUPDR |= 1<<(pin<<1);	//Pull-up
			break;
			
			case 'd': //input pull-down
				*GPIO_MODER &= ~(3<<(pin<<1));	//Input
				
				*GPIO_PUPDR |= 2<<(pin<<1);	//Pull-down
			break;
			
			case 'l': //output low speed, 2 MHz
				*GPIO_MODER &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_MODER |= 1<<(pin<<1);	//General purpose output mode
			
				*GPIO_OTYPER &= ~(1<<pin);	//Output push-pull
			
				*GPIO_OSPEEDR |= 3<<(pin<<1);	//2 MHz Low speed
				
				*GPIO_PUPDR &= ~(3<<(pin<<1));	//No pull-up, pull-down
			break;
			
			case 'm': //output medium speed, 25 MHz
				*GPIO_MODER &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_MODER |= 1<<(pin<<1);	//General purpose output mode
			
				*GPIO_OTYPER &= ~(1<<pin);	//Output push-pull
			
				*GPIO_OSPEEDR &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_OSPEEDR |= 1<<(pin<<1);	//25 MHz Medium speed
				
				*GPIO_PUPDR &= ~(3<<(pin<<1));	//No pull-up, pull-down
			break;
			
			case 'f': //output fast speed, 50 MHz
				*GPIO_MODER &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_MODER |= 1<<(pin<<1);	//General purpose output mode
			
				*GPIO_OTYPER &= ~(1<<pin);	//Output push-pull
			
				*GPIO_OSPEEDR &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_OSPEEDR |= 2<<(pin<<1);	//50 MHz Fast speed
				
				*GPIO_PUPDR &= ~(3<<(pin<<1));	//No pull-up, pull-down
			break;
			
			case 'h': //output high speed, 100
				*GPIO_MODER &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_MODER |= 1<<(pin<<1);	//General purpose output mode
			
				*GPIO_OTYPER &= ~(1<<pin);	//Output push-pull
			
				*GPIO_OSPEEDR &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_OSPEEDR |= 3<<(pin<<1);	//100 MHz High speed on 30 pF (80 MHz Output max speed on 15 pF)
				
				*GPIO_PUPDR &= ~(3<<(pin<<1));	//No pull-up, pull-down
			break;
			
			case 'o': //output high speed, 100
				*GPIO_MODER &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_MODER |= 1<<(pin<<1);	//General purpose output mode
			
				*GPIO_OTYPER &= ~(1<<pin);	//Output push-pull
			
				*GPIO_OSPEEDR &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_OSPEEDR |= 3<<(pin<<1);	//100 MHz High speed on 30 pF (80 MHz Output max speed on 15 pF)
				
				*GPIO_PUPDR &= ~(3<<(pin<<1));	//No pull-up, pull-down
			break;
			
			case 'L': //output low speed open-drain, 2 MHz
				*GPIO_MODER &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_MODER |= 1<<(pin<<1);	//General purpose output mode
			
				*GPIO_OTYPER |= 1<<pin;;	//Output open-drain
			
				*GPIO_OSPEEDR |= 3<<(pin<<1);	//2 MHz Low speed
				
				*GPIO_PUPDR &= ~(3<<(pin<<1));	//No pull-up, pull-down
			break;
			
			case 'M': //output medium speed open-drain, 25 MHz
				*GPIO_MODER &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_MODER |= 1<<(pin<<1);	//General purpose output mode
			
				*GPIO_OTYPER |= 1<<pin;;	//Output open-drain
			
				*GPIO_OSPEEDR &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_OSPEEDR |= 1<<(pin<<1);	//25 MHz Medium speed
				
				*GPIO_PUPDR &= ~(3<<(pin<<1));	//No pull-up, pull-down
			break;
			
			case 'F': //output fast speed open-drain, 50 MHz
				*GPIO_MODER &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_MODER |= 1<<(pin<<1);	//General purpose output mode
			
				*GPIO_OTYPER |= 1<<pin;;	//Output open-drain
			
				*GPIO_OSPEEDR &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_OSPEEDR |= 2<<(pin<<1);	//50 MHz Fast speed
				
				*GPIO_PUPDR &= ~(3<<(pin<<1));	//No pull-up, pull-down
			break;
			
			case 'H': //output high speed open-drain, 100 MHz
				*GPIO_MODER &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_MODER |= 1<<(pin<<1);	//General purpose output mode
			
				*GPIO_OTYPER |= 1<<pin;;	//Output open-drain
			
				*GPIO_OSPEEDR &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_OSPEEDR |= 3<<(pin<<1);	//100 MHz High speed on 30 pF (80 MHz Output max speed on 15 pF)
				
				*GPIO_PUPDR &= ~(3<<(pin<<1));	//No pull-up, pull-down
			break;
			
			case 'A': //output low speed open-drain, pull up 2 MHz
				*GPIO_MODER &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_MODER |= 1<<(pin<<1);	//General purpose output mode
			
				*GPIO_OTYPER |= 1<<pin;;	//Output open-drain
			
				*GPIO_OSPEEDR |= 3<<(pin<<1);	//2 MHz Low speed
				
				*GPIO_PUPDR |= 1<<(pin<<1);	//Pull-up
			break;
			
			case 'D': //output medium speed open-drain, pull up 25 MHz
				*GPIO_MODER &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_MODER |= 1<<(pin<<1);	//General purpose output mode
			
				*GPIO_OTYPER |= 1<<pin;;	//Output open-drain
			
				*GPIO_OSPEEDR &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_OSPEEDR |= 1<<(pin<<1);	//25 MHz Medium speed
				
				*GPIO_PUPDR |= 1<<(pin<<1);	//Pull-up
			break;
			
			case 'P': //output fast speed open-drain, pull up 50 MHz
				*GPIO_MODER &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_MODER |= 1<<(pin<<1);	//General purpose output mode
			
				*GPIO_OTYPER |= 1<<pin;;	//Output open-drain
			
				*GPIO_OSPEEDR &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_OSPEEDR |= 2<<(pin<<1);	//50 MHz Fast speed
				
				*GPIO_PUPDR |= 1<<(pin<<1);	//Pull-up
			break;
			
			case 'p': //output high speed open-drain, pull up 100 MHz
				*GPIO_MODER &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_MODER |= 1<<(pin<<1);	//General purpose output mode
			
				*GPIO_OTYPER |= 1<<pin;;	//Output open-drain
			
				*GPIO_OSPEEDR &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_OSPEEDR |= 3<<(pin<<1);	//100 MHz High speed on 30 pF (80 MHz Output max speed on 15 pF)
				
				*GPIO_PUPDR |= 1<<(pin<<1);	//Pull-up
			break;
			
			case 'n': //alternate function, 2 MHz
				*GPIO_MODER &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_MODER |= 2<<(pin<<1);	//Alternate function mode
			
				*GPIO_OTYPER &= ~(1<<pin);	//Output push-pull
			
				*GPIO_OSPEEDR |= 3<<(pin<<1);	//2 MHz Low speed
				
				*GPIO_PUPDR &= ~(3<<(pin<<1));	//No pull-up, pull-down
			break;
			
			case 'r': //alternate function, 25 MHz
				*GPIO_MODER &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_MODER |= 2<<(pin<<1);	//Alternate function mode
			
				*GPIO_OTYPER &= ~(1<<pin);	//Output push-pull
			
				*GPIO_OSPEEDR &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_OSPEEDR |= 1<<(pin<<1);	//25 MHz Medium speed
				
				*GPIO_PUPDR &= ~(3<<(pin<<1));	//No pull-up, pull-down
			break;
			
			case 'e': //alternate function, 50 MHz
				*GPIO_MODER &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_MODER |= 2<<(pin<<1);	//Alternate function mode
			
				*GPIO_OTYPER &= ~(1<<pin);	//Output push-pull
			
				*GPIO_OSPEEDR &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_OSPEEDR |= 2<<(pin<<1);	//50 MHz Fast speed
				
				*GPIO_PUPDR &= ~(3<<(pin<<1));	//No pull-up, pull-down
			break;
			
			case 't': //alternate function, 100 MHz
				*GPIO_MODER &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_MODER |= 2<<(pin<<1);	//Alternate function mode
			
				*GPIO_OTYPER &= ~(1<<pin);	//Output push-pull
			
				*GPIO_OSPEEDR &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_OSPEEDR |= 3<<(pin<<1);	//100 MHz High speed on 30 pF (80 MHz Output max speed on 15 pF)
				
				*GPIO_PUPDR &= ~(3<<(pin<<1));	//No pull-up, pull-down
			break;
			
			case 'N': //alternate function open-drain, 2 MHz
				*GPIO_MODER &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_MODER |= 2<<(pin<<1);	//Alternate function mode
			
				*GPIO_OTYPER |= 1<<pin;;	//Output open-drain
			
				*GPIO_OSPEEDR |= 3<<(pin<<1);	//2 MHz Low speed
				
				*GPIO_PUPDR &= ~(3<<(pin<<1));	//No pull-up, pull-down
			break;
			
			case 'R': //alternate function open-drain, 25 MHz
				*GPIO_MODER &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_MODER |= 2<<(pin<<1);	//Alternate function mode
			
				*GPIO_OTYPER |= 1<<pin;;	//Output open-drain
			
				*GPIO_OSPEEDR &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_OSPEEDR |= 1<<(pin<<1);	//25 MHz Medium speed
				
				*GPIO_PUPDR &= ~(3<<(pin<<1));	//No pull-up, pull-down
			break;
			
			case 'E': //alternate function open-drain, 50 MHz
				*GPIO_MODER &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_MODER |= 2<<(pin<<1);	//Alternate function mode
			
				*GPIO_OTYPER |= 1<<pin;;	//Output open-drain
			
				*GPIO_OSPEEDR &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_OSPEEDR |= 2<<(pin<<1);	//50 MHz Fast speed
				
				*GPIO_PUPDR &= ~(3<<(pin<<1));	//No pull-up, pull-down
			break;
			
			case 'T': //alternate function open-drain, 100 MHz
				*GPIO_MODER &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_MODER |= 2<<(pin<<1);	//Alternate function mode
			
				*GPIO_OTYPER |= 1<<pin;;	//Output open-drain
			
				*GPIO_OSPEEDR &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_OSPEEDR |= 3<<(pin<<1);	//100 MHz High speed on 30 pF (80 MHz Output max speed on 15 pF)
				
				*GPIO_PUPDR &= ~(3<<(pin<<1));	//No pull-up, pull-down
			break;
			
			case 'I': //alternate function open-drain, pull up, 2 MHz
				*GPIO_MODER &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_MODER |= 2<<(pin<<1);	//Alternate function mode
			
				*GPIO_OTYPER |= 1<<pin;;	//Output open-drain
			
				*GPIO_OSPEEDR |= 3<<(pin<<1);	//2 MHz Low speed
				
				*GPIO_PUPDR |= 1<<(pin<<1);	//Pull-up
			break;
			
			case 'C': //alternate function open-drain, pull up, 25 MHz
				*GPIO_MODER &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_MODER |= 2<<(pin<<1);	//Alternate function mode
			
				*GPIO_OTYPER |= 1<<pin;;	//Output open-drain
			
				*GPIO_OSPEEDR &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_OSPEEDR |= 1<<(pin<<1);	//25 MHz Medium speed
				
				*GPIO_PUPDR |= 1<<(pin<<1);	//Pull-up
			break;
			
			case 'c': //alternate function open-drain, pull up, 50 MHz
				*GPIO_MODER &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_MODER |= 2<<(pin<<1);	//Alternate function mode
			
				*GPIO_OTYPER |= 1<<pin;;	//Output open-drain
			
				*GPIO_OSPEEDR &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_OSPEEDR |= 2<<(pin<<1);	//50 MHz Fast speed
				
				*GPIO_PUPDR |= 1<<(pin<<1);	//Pull-up
			break;
			
			case 'U': //alternate function open-drain, pull up, 100 MHz
				*GPIO_MODER &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_MODER |= 2<<(pin<<1);	//Alternate function mode
			
				*GPIO_OTYPER |= 1<<pin;;	//Output open-drain
			
				*GPIO_OSPEEDR &= ~(3<<(pin<<1));	//clear config bits
				*GPIO_OSPEEDR |= 3<<(pin<<1);	//100 MHz High speed on 30 pF (80 MHz Output max speed on 15 pF)
				
				*GPIO_PUPDR |= 1<<(pin<<1);	//Pull-up
			break;
			
			default:
				return -1;		// Error condition
		}
		
		return 1;
}

void ConfigPortA(char *cfg)
{
	RCC_AHB1ENR |= 1;													//GPIOAEN clock enable, allows port configuration registers to be edited
	ConfigPort(cfg, (unsigned int *)&GPIOA_MODER, (unsigned int *)&GPIOA_OTYPER, (unsigned int *)&GPIOA_OSPEEDR, (unsigned int *)&GPIOA_PUPDR);
}

void ConfigPortB(char *cfg)
{
	RCC_AHB1ENR |= 1<<1;													//GPIOBEN clock enable, allows port configuration registers to be edited
	ConfigPort(cfg, (unsigned int *)&GPIOB_MODER, (unsigned int *)&GPIOB_OTYPER, (unsigned int *)&GPIOB_OSPEEDR, (unsigned int *)&GPIOB_PUPDR);
}

void ConfigPortC(char *cfg)
{
	RCC_AHB1ENR |= 1<<2;													//GPIOCEN clock enable, allows port configuration registers to be edited
	ConfigPort(cfg, (unsigned int *)&GPIOC_MODER, (unsigned int *)&GPIOC_OTYPER, (unsigned int *)&GPIOC_OSPEEDR, (unsigned int *)&GPIOC_PUPDR);
}

void ConfigPortD(char *cfg)
{
	RCC_AHB1ENR |= 1<<3;													//GPIODEN clock enable, allows port configuration registers to be edited
	ConfigPort(cfg, (unsigned int *)&GPIOD_MODER, (unsigned int *)&GPIOD_OTYPER, (unsigned int *)&GPIOD_OSPEEDR, (unsigned int *)&GPIOD_PUPDR);
}

void ConfigPortE(char *cfg)
{
	RCC_AHB1ENR |= 1<<4;													//GPIOEEN clock enable, allows port configuration registers to be edited
	ConfigPort(cfg, (unsigned int *)&GPIOE_MODER, (unsigned int *)&GPIOE_OTYPER, (unsigned int *)&GPIOE_OSPEEDR, (unsigned int *)&GPIOE_PUPDR);
}

void ConfigPortF(char *cfg)
{
	RCC_AHB1ENR |= 1<<5;													//GPIOFEN clock enable, allows port configuration registers to be edited
	ConfigPort(cfg, (unsigned int *)&GPIOF_MODER, (unsigned int *)&GPIOF_OTYPER, (unsigned int *)&GPIOF_OSPEEDR, (unsigned int *)&GPIOF_PUPDR);
}

void ConfigPortG(char *cfg)
{
	RCC_AHB1ENR |= 1<<6;													//GPIOGEN clock enable, allows port configuration registers to be edited
	ConfigPort(cfg, (unsigned int *)&GPIOG_MODER, (unsigned int *)&GPIOG_OTYPER, (unsigned int *)&GPIOG_OSPEEDR, (unsigned int *)&GPIOG_PUPDR);
}

void ConfigPortH(char *cfg)
{
	RCC_AHB1ENR |= 1<<7;													//GPIOHEN clock enable, allows port configuration registers to be edited
	ConfigPort(cfg, (unsigned int *)&GPIOH_MODER, (unsigned int *)&GPIOH_OTYPER, (unsigned int *)&GPIOH_OSPEEDR, (unsigned int *)&GPIOH_PUPDR);
}

void ConfigPortI(char *cfg)
{
	RCC_AHB1ENR |= 1<<8;													//GPIOIEN clock enable, allows port configuration registers to be edited
	ConfigPort(cfg, (unsigned int *)&GPIOI_MODER, (unsigned int *)&GPIOI_OTYPER, (unsigned int *)&GPIOI_OSPEEDR, (unsigned int *)&GPIOI_PUPDR);
}



void ConfigPinOnPort(unsigned int port, unsigned int pin, char c)
{
	int portnum;
	portnum = (port - PORTA) / (PORTB - PORTA);
	RCC_AHB1ENR |= 1 << portnum; //AHB1 peripheral clock enable register
	ConfigPin(pin, c, (unsigned int *)(port + MODER), (unsigned int *)(port + OTYPER), (unsigned int *)(port + OSPEEDR), (unsigned int *)(port + PUPDR));
	
}
void TogglePin(char port, unsigned int pin)
{
	switch(port)
	{
		case 'A':
			GPIOA_ODR^=(1<<pin);
			break;
		case 'B':
			GPIOB_ODR^=(1<<pin);
			break;
		case 'C':
			GPIOC_ODR^=(1<<pin);
			break;
		case 'D':
			GPIOD_ODR^=(1<<pin);
			break;
		case 'E':
			GPIOE_ODR^=(1<<pin);
			break;
		case 'F':
			GPIOF_ODR^=(1<<pin);
			break;
		case 'G':
			GPIOG_ODR^=(1<<pin);
			break;
		case 'H':
			GPIOH_ODR^=(1<<pin);
			break;
		case 'I':
			GPIOI_ODR^=(1<<pin);
			break;
		default:
			break;
	}
}

unsigned int ReadPin(char port, unsigned int pin)
{
	switch(port)
	{
		case 'A': return (GPIOA_IDR>>pin) & 1;
		case 'B': return (GPIOB_IDR>>pin) & 1;
		case 'C': return (GPIOC_IDR>>pin) & 1;
		case 'D': return (GPIOD_IDR>>pin) & 1;
		case 'E': return (GPIOE_IDR>>pin) & 1;
		case 'F': return (GPIOF_IDR>>pin) & 1;
		case 'G': return (GPIOG_IDR>>pin) & 1;
		case 'H': return (GPIOH_IDR>>pin) & 1;
		case 'I': return (GPIOI_IDR>>pin) & 1;
		default: return 0;
	}
}

unsigned int GetSYSCLKFreq(void)
{
	unsigned int freq;

	switch((RCC_CFGR>>2) & 3)
	{
		case 0:	//HSI
			freq=16000000;
		break;
		case 1:	//HSE
			freq=HSEfreq;
		break;
		case 2:	//PLL
			if(((RCC_PLLCFGR>>22) & 1)==0){ freq=16000000; }else{ freq=HSEfreq; }	//PLL input=HSI or HSE
			freq /= RCC_PLLCFGR & 0x3F;	//M
			freq *= (RCC_PLLCFGR>>6) & 0x1FF;	//N
			freq /= (((RCC_PLLCFGR>>16) & 3)+1)<<1;	//P
		break;
	}
	
	return freq;
}

unsigned int GetAHBFreq(void)
{
	unsigned int freq,div;
	
	freq=GetSYSCLKFreq();
	div=(RCC_CFGR>>4) & 0xF;
	if((div & 8)!=0)	//if SYSCLK divided
	{
		div &= 7;
		if(div<=3){ div=1<<(div+1); }else{ div=1<<(div+2); }
		freq /= div;
	}
	
	return freq;
}

unsigned int GetAPB1Freq(void)
{
	unsigned int freq,div;
	
	freq=GetAHBFreq();
	div=(RCC_CFGR>>10) & 7;
	if((div & 4)!=0)	//if AHBCLK divided
	{
		div &= 3;
		div=1<<(div+1);
		freq /= div;
	}
	
	return freq;
}

unsigned int GetAPB2Freq(void)
{
	unsigned int freq,div;
	
	freq=GetAHBFreq();
	div=(RCC_CFGR>>13) & 7;
	if((div & 4)!=0)	//if AHBCLK divided
	{
		div &= 3;
		div=1<<(div+1);
		freq /= div;
	}
	
	return freq;
}

void SetAF(volatile unsigned int *GPIO_AFRL, volatile unsigned int *GPIO_AFRH, unsigned int pin, unsigned int AFnum)
{
	if(pin<=7)
	{
		*GPIO_AFRL &= ~(0xFUL<<(pin<<2));	//clear configuration bits
		*GPIO_AFRL |= AFnum<<(pin<<2);
	}else
	{
		*GPIO_AFRH &= ~(0xFUL<<((pin-8)<<2));	//clear configuration bits
		*GPIO_AFRH |= AFnum<<((pin-8)<<2);
	}
}

void SetAFA(unsigned int pin, unsigned int AFnum)
{
	RCC_AHB1ENR |= 1;													//GPIOAEN clock enable, allows port configuration registers to be edited
	SetAF((unsigned int *)&GPIOA_AFRL,(unsigned int *)&GPIOA_AFRH, pin, AFnum);
}

void SetAFB(unsigned int pin, unsigned int AFnum)
{
	RCC_AHB1ENR |= 1UL<<1;													//GPIOBEN clock enable, allows port configuration registers to be edited
	SetAF((unsigned int *)&GPIOB_AFRL,(unsigned int *)&GPIOB_AFRH, pin, AFnum);
}

void SetAFC(unsigned int pin, unsigned int AFnum)
{
	RCC_AHB1ENR |= 1UL<<2;													//GPIOCEN clock enable, allows port configuration registers to be edited
	SetAF((unsigned int *)&GPIOC_AFRL,(unsigned int *)&GPIOC_AFRH, pin, AFnum);
}

void SetAFD(unsigned int pin, unsigned int AFnum)
{
	RCC_AHB1ENR |= 1UL<<3;													//GPIODEN clock enable, allows port configuration registers to be edited
	SetAF((unsigned int *)&GPIOD_AFRL,(unsigned int *)&GPIOD_AFRH, pin, AFnum);
}

void SetAFE(unsigned int pin, unsigned int AFnum)
{
	RCC_AHB1ENR |= 1UL<<4;													//GPIOEEN clock enable, allows port configuration registers to be edited
	SetAF((unsigned int *)&GPIOE_AFRL,(unsigned int *)&GPIOE_AFRH, pin, AFnum);
}

void SetAFF(unsigned int pin, unsigned int AFnum)
{
	RCC_AHB1ENR |= 1UL<<5;													//GPIOFEN clock enable, allows port configuration registers to be edited
	SetAF((unsigned int *)&GPIOF_AFRL,(unsigned int *)&GPIOF_AFRH, pin, AFnum);
}

void SetAFG(unsigned int pin, unsigned int AFnum)
{
	RCC_AHB1ENR |= 1UL<<6;													//GPIOGEN clock enable, allows port configuration registers to be edited
	SetAF((unsigned int *)&GPIOG_AFRL,(unsigned int *)&GPIOG_AFRH, pin, AFnum);
}

void SetAFH(unsigned int pin, unsigned int AFnum)
{
	RCC_AHB1ENR |= 1UL<<7;													//GPIOHEN clock enable, allows port configuration registers to be edited
	SetAF((unsigned int *)&GPIOH_AFRL,(unsigned int *)&GPIOH_AFRH, pin, AFnum);
}

void SetAFI(unsigned int pin, unsigned int AFnum)
{
	RCC_AHB1ENR |= 1UL<<8;													//GPIOIEN clock enable, allows port configuration registers to be edited
	SetAF((unsigned int *)&GPIOI_AFRL,(unsigned int *)&GPIOI_AFRH, pin, AFnum);
}

void InitUSART(unsigned int baud, volatile unsigned int *USART_BRR, volatile unsigned int *USART_CR1, volatile unsigned int *USART_CR2, volatile unsigned int *USART_CR3, volatile unsigned int *USART_SR)
{
	*USART_CR1 = 1UL<<13;											// Enable USART
	
	if(((unsigned int)USART_BRR==(unsigned int)&USART1_BRR) || ((unsigned int)USART_BRR==(unsigned int)&USART6_BRR))		//check if it is USART1 or USART6, if it is use APB2 frequency, else APB1
	{
		*USART_BRR = (GetAPB2Freq() / baud);
	}else
	{
		*USART_BRR = (GetAPB1Freq() / baud);
	}
	
	//8N1 is default
	*USART_CR1 |= 1UL<<3;											// Enable transmitter
	*USART_CR1 |= 1UL<<2;											// Enable receiver
	*USART_CR1 |= 1UL<<1;											// Receiver in active mode
	
	*USART_CR2=0;
	*USART_CR3=0;
	
	while((*USART_SR & (1UL<<6)) == 0);				// Wait for USART to be ready
}

void WriteUSART(unsigned char chrval, volatile unsigned int *USART_SR, volatile unsigned int *USART_DR)
{
	//a read from the USART_SR register followed by a write to the USART_DR register clears the transmission complete bit (6)
	while((*USART_SR & (1UL<<6)) == 0);							// Wait for transmission to complete
	*USART_DR = chrval;
	while((*USART_SR & (1UL<<7)) == 0);							// Wait for data to enter the shift register (cleared by a write to DR)
}

unsigned char ReadUSART(volatile unsigned int *USART_SR, volatile unsigned int *USART_DR)
{
	while((*USART_SR & (1UL<<5)) == 0);							// wait for Received data is ready to be read (cleared by reading data register)
	return *USART_DR;
}

unsigned int USARTDataIsAvailable(volatile unsigned int *USART_SR)	// Return 1 if a byte is in the buffer
{
	return (*USART_SR>>5) & 1;
}

void InitUSART1(unsigned int baud)
{
	RCC_APB2ENR |= 1<<4;										// USART1 clock enable
	//configure AF first so RX and TX dont both end up as output
	SetAFA(9,AF_USART1_TX);	//PA9 AF USART1_TX
	SetAFA(10,AF_USART1_RX);	//PA10 AF USART1_RX
	ConfigPortA("---- -rr- ---- ----");			// sets PA9 and PA10 to alternate function 25Mhz push pull

	InitUSART(baud, (unsigned int *)&USART1_BRR, (unsigned int *)&USART1_CR1, (unsigned int *)&USART1_CR2, (unsigned int *)&USART1_CR3, (unsigned int *)&USART1_SR);
}

void InitUSART1Interrupts(unsigned int baud)
{
	InitUSART1(baud);
	EnableInterruptPosition(USART1_Interrupt_Position);
	USART1_CR1 |= 1<<5;	//interrupt enable for when read data register not empty
}

void WriteUSART1(unsigned char chrval)
{
	WriteUSART(chrval, (unsigned int *)&USART1_SR, (unsigned int *)&USART1_DR);
}

unsigned char ReadUSART1(void)
{
	return ReadUSART((unsigned int *)&USART1_SR, (unsigned int *)&USART1_DR);
}

unsigned int USART1DataIsAvailable(void)	// Return 1 if a character is in the buffer
{
	return USARTDataIsAvailable((unsigned int *)&USART1_SR);
}

void InitUSART2(unsigned int baud)
{
	RCC_APB1ENR |= 1<<17;										// USART2 clock enable
	//configure AF first so RX and TX dont both end up as output
	SetAFA(2,AF_USART2_TX);	//PA2 AF USART2_TX
	SetAFA(3,AF_USART2_RX);	//PA3 AF USART2_RX
	ConfigPortA("---- ---- ---- rr--");			// sets PA2 and PA3 to alternate function 25Mhz push pull

	InitUSART(baud, (unsigned int *)&USART2_BRR, (unsigned int *)&USART2_CR1, (unsigned int *)&USART2_CR2, (unsigned int *)&USART2_CR3, (unsigned int *)&USART2_SR);
}

void InitUSART2Interrupts(unsigned int baud)
{
	InitUSART2(baud);
	EnableInterruptPosition(USART2_Interrupt_Position);
	USART2_CR1 |= 1<<5;	//interrupt enable for when read data register not empty
}

void WriteUSART2(unsigned char chrval)
{
	WriteUSART(chrval, (unsigned int *)&USART2_SR, (unsigned int *)&USART2_DR);
}

unsigned char ReadUSART2(void)
{
	return ReadUSART((unsigned int *)&USART2_SR, (unsigned int *)&USART2_DR);
}

unsigned int USART2DataIsAvailable(void)	// Return 1 if a character is in the buffer
{
	return USARTDataIsAvailable((unsigned int *)&USART2_SR);
}

void InitUSART3(unsigned int baud)
{
	RCC_APB1ENR |= 1<<18;										// USART3 clock enable
	//configure AF first so RX and TX dont both end up as output
	SetAFB(10,AF_USART3_TX);	//PB3 AF USART3_TX
	SetAFB(11,AF_USART3_RX);	//PB3 AF USART3_RX
	ConfigPortB("---- rr-- ---- ----");			// sets PB10 and PB11 to alternate function 25Mhz push pull

	InitUSART(baud, (unsigned int *)&USART3_BRR, (unsigned int *)&USART3_CR1, (unsigned int *)&USART3_CR2, (unsigned int *)&USART3_CR3, (unsigned int *)&USART3_SR);
}

void InitUSART3Interrupts(unsigned int baud)
{
	InitUSART3(baud);
	EnableInterruptPosition(USART3_Interrupt_Position);
	USART3_CR1 |= 1<<5;	//interrupt enable for when read data register not empty
}

void WriteUSART3(unsigned char chrval)
{
	WriteUSART(chrval, (unsigned int *)&USART3_SR, (unsigned int *)&USART3_DR);
}

unsigned char ReadUSART3(void)
{
	return ReadUSART((unsigned int *)&USART3_SR, (unsigned int *)&USART3_DR);
}

unsigned int USART3DataIsAvailable(void)	// Return 1 if a character is in the buffer
{
	return USARTDataIsAvailable((unsigned int *)&USART3_SR);
}

void InitUART4(unsigned int baud)
{
	RCC_APB1ENR |= 1<<19;										// UART4 clock enable
	//configure AF first so RX and TX dont both end up as output
	SetAFA(0,AF_UART4_TX);	//PA0 AF UART4_TX
	SetAFA(1,AF_UART4_RX);	//PA1 AF UART4_RX
	ConfigPortA("---- ---- ---- --rr");			// sets PA0 and PA1 to alternate function 25Mhz push pull

	InitUSART(baud, (unsigned int *)&UART4_BRR, (unsigned int *)&UART4_CR1, (unsigned int *)&UART4_CR2, (unsigned int *)&UART4_CR3, (unsigned int *)&UART4_SR);
}

void InitUART4Interrupts(unsigned int baud)
{
	InitUART4(baud);
	EnableInterruptPosition(UART4_Interrupt_Position);
	UART4_CR1 |= 1<<5;	//interrupt enable for when read data register not empty
}

void WriteUART4(unsigned char chrval)
{
	WriteUSART(chrval, (unsigned int *)&UART4_SR, (unsigned int *)&UART4_DR);
}

unsigned char ReadUART4(void)
{
	return ReadUSART((unsigned int *)&UART4_SR, (unsigned int *)&UART4_DR);
}

unsigned int UART4DataIsAvailable(void)	// Return 1 if a character is in the buffer
{
	return USARTDataIsAvailable((unsigned int *)&UART4_SR);
}

void InitUART5(unsigned int baud)
{
	RCC_APB1ENR |= 1<<20;										// UART5 clock enable
	//configure AF first so RX and TX dont both end up as output
	SetAFC(12,AF_UART5_TX);	//PC12 AF UART5_TX
	SetAFD(2,AF_UART5_RX);	//PD2 AF UART5_RX
	ConfigPortC("---r ---- ---- ----");			// sets PC12 alternate function 25Mhz push pull
	ConfigPortD("---- ---- ---- -r--");			// sets PD2 alternate function 25Mhz push pull

	InitUSART(baud, (unsigned int *)&UART5_BRR, (unsigned int *)&UART5_CR1, (unsigned int *)&UART5_CR2, (unsigned int *)&UART5_CR3, (unsigned int *)&UART5_SR);
}

void InitUART5Interrupts(unsigned int baud)
{
	InitUART5(baud);
	EnableInterruptPosition(UART5_Interrupt_Position);
	UART5_CR1 |= 1<<5;	//interrupt enable for when read data register not empty
}

void WriteUART5(unsigned char chrval)
{
	WriteUSART(chrval, (unsigned int *)&UART5_SR, (unsigned int *)&UART5_DR);
}

unsigned char ReadUART5(void)
{
	return ReadUSART((unsigned int *)&UART5_SR, (unsigned int *)&UART5_DR);
}

unsigned int UART5DataIsAvailable(void)	// Return 1 if a character is in the buffer
{
	return USARTDataIsAvailable((unsigned int *)&UART5_SR);
}

void InitUSART6(unsigned int baud)
{
	RCC_APB2ENR |= 1<<5;										// USART6 clock enable
	//configure AF first so RX and TX dont both end up as output
	SetAFC(6,AF_USART6_TX);	//PC6 AF USART3_TX
	SetAFC(7,AF_USART6_RX);	//PC7 AF USART3_RX
	ConfigPortC("---- ---- rr-- ----");			// sets PC6 and PC7 to alternate function 25Mhz push pull

	InitUSART(baud, (unsigned int *)&USART6_BRR, (unsigned int *)&USART6_CR1, (unsigned int *)&USART6_CR2, (unsigned int *)&USART6_CR3, (unsigned int *)&USART6_SR);
}

void InitUSART6Interrupts(unsigned int baud)
{
	InitUSART6(baud);
	EnableInterruptPosition(USART6_Interrupt_Position);
	USART6_CR1 |= 1<<5;	//interrupt enable for when read data register not empty
}

void WriteUSART6(unsigned char chrval)
{
	WriteUSART(chrval, (unsigned int *)&USART6_SR, (unsigned int *)&USART6_DR);
}

unsigned char ReadUSART6(void)
{
	return ReadUSART((unsigned int *)&USART6_SR, (unsigned int *)&USART6_DR);
}

unsigned int USART6DataIsAvailable(void)	// Return 1 if a character is in the buffer
{
	return USARTDataIsAvailable((unsigned int *)&USART6_SR);
}

void PrintfUSART(volatile unsigned int *USART_SR, volatile unsigned int *USART_DR, const char * format, ... )
{
	char buff[256];
	unsigned int i=0;
	va_list args;
	
	va_start(args,format);
	vsnprintf(buff,256,format,args);
	
  //print the string
	do
	{
		WriteUSART(buff[i], (unsigned int *)&USART_SR, (unsigned int *)&USART_DR);
	}while(buff[i]!=0);
	
	va_end(args);
}

void Printf1(const char * format, ... )
{
	va_list args;
	va_start(args,format);
	PrintfUSART((unsigned int *)&USART1_SR, (unsigned int *)&USART1_DR, format, args );
}

void Printf2(const char * format, ... )
{
	va_list args;
	va_start(args,format);
	PrintfUSART((unsigned int *)&USART2_SR, (unsigned int *)&USART2_DR, format, args );
}

void Printf3(const char * format, ... )
{
	va_list args;
	va_start(args,format);
	PrintfUSART((unsigned int *)&USART3_SR, (unsigned int *)&USART3_DR, format, args );
}

void Printf4(const char * format, ... )
{
	va_list args;
	va_start(args,format);
	PrintfUSART((unsigned int *)&UART4_SR, (unsigned int *)&UART4_DR, format, args );
}

void Printf5(const char * format, ... )
{
	va_list args;
	va_start(args,format);
	PrintfUSART((unsigned int *)&UART5_SR, (unsigned int *)&UART5_DR, format, args );
}

void Printf6(const char * format, ... )
{
	va_list args;
	va_start(args,format);
	PrintfUSART((unsigned int *)&USART6_SR, (unsigned int *)&USART6_DR, format, args );
}

void InitRandomNumberGenerator(void)
{
	RCC_AHB2ENR |= 1UL<<6;	//Random number generator clock enable
	RNG_CR |= 1UL<<2; //enable the generator
}

unsigned int ReadRandomNumberGenerator(void)
{
	unsigned int count=0, count2=0, rnd;
	
	do
	{
		count2 += 1;
		if(count>1)
		{
			RNG_CR &= ~(1UL<<2); //disable the generator
			RNG_SR &= ~(1UL<<6); //clear the seed error
			RNG_CR |= 1UL<<2; //enable the generator
		}	
		
		do
		{
			count += 1;
			if(count>1){  RNG_SR &= ~(1UL<<5); }	//if a retry, clear the clock error
			while((RNG_SR & 1)==0);	//wait for a value to be ready
			rnd=RNG_DR;
		}while(((RNG_SR>>5) & 1)==1);	//loop if there is a clock error
	}while(((RNG_SR>>6) & 1)==1); //loop if seed error
	
	return rnd;
}

void InitDAC(unsigned int mode, unsigned int BufferEN)
{
	RCC_APB1ENR |= 1<<29;	//enable DAC clock
	DAC_CR=0;
	
	if(mode>DAC_Stereo){ mode=DAC_Mono1; }	//if invalid mode, use mono1
	switch(mode)
	{
		case DAC_Mono1:
			ConfigPortA("---- ---- ---a ----");	//analog mode
			DAC_CR |= 1<<2;	//DAC channel1 trigger enabled
			DAC_CR |= 7<<3;	//DAC channel1 Software trigger
			if(BufferEN==DAC_Unbuffered){ DAC_CR |= 1<<1; }	//channel1 output buffer disabled
			DAC_CR |= 1;	//DAC channel1 enable	
		break;
			
		case DAC_Mono2:
			ConfigPortA("---- ---- --a- ----");	//analog mode
			DAC_CR |= 1<<18;	//DAC channel2 trigger enabled
			DAC_CR |= 7<<19;	//DAC channel2 Software trigger
			if(BufferEN==DAC_Unbuffered){ DAC_CR |= 1<<17; }	//channel2 output buffer disabled
			DAC_CR |= 1<<16;	//DAC channel2 enable
		break;
			
		case DAC_Stereo:
			ConfigPortA("---- ---- --aa ----");	//analog mode
			DAC_CR |= 1<<2;	//DAC channel1 trigger enabled
			DAC_CR |= 7<<3;	//DAC channel1 Software trigger
			if(BufferEN==DAC_Unbuffered){ DAC_CR |= 1<<1; }	//channel1 output buffer disabled
			DAC_CR |= 1;	//DAC channel1 enable	
		
			DAC_CR |= 1<<18;	//DAC channel2 trigger enabled
			DAC_CR |= 7<<19;	//DAC channel2 Software trigger
			if(BufferEN==DAC_Unbuffered){ DAC_CR |= 1<<17; }	//channel2 output buffer disabled
			DAC_CR |= 1<<16;	//DAC channel2 enable
		break;
	}
}

void SetDAC1(unsigned int DACval)	//12 bit 0-4095
{
	//while((DAC_SWTRIGR & 1)==1);	//wait for ready
	DAC_DHR12R1=DACval;
	DAC_SWTRIGR |= 1;	//DAC channel1 software trigger
}

void SetDAC2(unsigned int DACval)	//12 bit 0-4095
{
	//while((DAC_SWTRIGR & 2)==2);	//wait for ready
	DAC_DHR12R2=DACval;
	DAC_SWTRIGR |= 2;	//DAC channel2 software trigger
}

void SetDACStereo(unsigned int DACval1, unsigned int DACval2)	//12 bit 0-4095
{
	//while((DAC_SWTRIGR & 3)!=0);	//wait for ready
	DAC_DHR12RD=DACval1 | (DACval2<<16);
	DAC_SWTRIGR |= 3;	//DAC channel 1 and 2 software trigger
}
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
*/

void InitADC1(unsigned int channels, unsigned int resolution, unsigned int SamplingTime)	//each bit represents a channel, LSB=channel 0
{
	unsigned int APBfreq, i;
	
	RCC_APB2ENR |= 1UL<<8;
	
	ADC_CCR=0;
	ADC1_CR1=0;
	ADC1_SQR1=0;
	
	if((channels & 1)!=0){ ConfigPortA("---- ----  ---- ---a"); }
	if((channels & 1UL<<1)!=0){ ConfigPortA("---- ----  ---- --a-"); }
	if((channels & 1UL<<2)!=0){ ConfigPortA("---- ----  ---- -a--"); }
	if((channels & 1UL<<3)!=0){ ConfigPortA("---- ----  ---- a---"); }
	if((channels & 1UL<<4)!=0){ ConfigPortA("---- ----  ---a ----"); }
	if((channels & 1UL<<5)!=0){ ConfigPortA("---- ----  --a- ----"); }
	if((channels & 1UL<<6)!=0){ ConfigPortA("---- ----  -a-- ----"); }
	if((channels & 1UL<<7)!=0){ ConfigPortA("---- ----  a--- ----"); }
	if((channels & 1UL<<8)!=0){ ConfigPortB("---- ----  ---- ---a"); }
	if((channels & 1UL<<9)!=0){ ConfigPortB("---- ----  ---- --a-"); }
	if((channels & 1UL<<10)!=0){ ConfigPortC("---- ----  ---- ---a"); }
	if((channels & 1UL<<11)!=0){ ConfigPortC("---- ----  ---- --a-"); }
	if((channels & 1UL<<12)!=0){ ConfigPortC("---- ----  ---- -a--"); }
	if((channels & 1UL<<13)!=0){ ConfigPortC("---- ----  ---- a---"); }
	if((channels & 1UL<<14)!=0){ ConfigPortC("---- ----  ---a ----"); }
	if((channels & 1UL<<15)!=0){ ConfigPortC("---- ----  --a- ----"); }
	if((channels & 1UL<<16)!=0){ ADC_CCR |= 1UL<<23; }	//temperature sensor (and VREFINT enabled)
	if((channels & 1UL<<17)!=0){ ADC_CCR |= 1UL<<23; }	//VREFINT enabled (and temperature sensor)
	if((channels & 1UL<<18)!=0){ ADC_CCR |= 1UL<<22; }	//VBAT/2 enabled

	APBfreq=GetAPB2Freq();
	for(i=0;i<=3;i++)
	{
		if(APBfreq/((i+1)<<1)>=18000000)	//prescaler (/2, /4, /6, /8) for 18Mhz Max
		{
			ADC_CCR |= i<<16;
			break;
		}
	}
	
	ADC1_CR1 |= resolution<<24;
	ADC1_SMPR2=SamplingTime;
	ADC1_CR2 |= 1; //Enable ADC
}

void InitADC2(unsigned int channels, unsigned int resolution, unsigned int SamplingTime)	//each bit represents a channel, LSB=channel 0
{
	unsigned int APBfreq, i;
	
	RCC_APB2ENR |= 1UL<<9;
	
	ADC_CCR=0;
	ADC2_CR1=0;
	ADC2_SQR1=0;
	
	if((channels & 1)!=0){ ConfigPortA("---- ----  ---- ---a"); }
	if((channels & 1UL<<1)!=0){ ConfigPortA("---- ----  ---- --a-"); }
	if((channels & 1UL<<2)!=0){ ConfigPortA("---- ----  ---- -a--"); }
	if((channels & 1UL<<3)!=0){ ConfigPortA("---- ----  ---- a---"); }
	if((channels & 1UL<<4)!=0){ ConfigPortA("---- ----  ---a ----"); }
	if((channels & 1UL<<5)!=0){ ConfigPortA("---- ----  --a- ----"); }
	if((channels & 1UL<<6)!=0){ ConfigPortA("---- ----  -a-- ----"); }
	if((channels & 1UL<<7)!=0){ ConfigPortA("---- ----  a--- ----"); }
	if((channels & 1UL<<8)!=0){ ConfigPortB("---- ----  ---- ---a"); }
	if((channels & 1UL<<9)!=0){ ConfigPortB("---- ----  ---- --a-"); }
	if((channels & 1UL<<10)!=0){ ConfigPortC("---- ----  ---- ---a"); }
	if((channels & 1UL<<11)!=0){ ConfigPortC("---- ----  ---- --a-"); }
	if((channels & 1UL<<12)!=0){ ConfigPortC("---- ----  ---- -a--"); }
	if((channels & 1UL<<13)!=0){ ConfigPortC("---- ----  ---- a---"); }
	if((channels & 1UL<<14)!=0){ ConfigPortC("---- ----  ---a ----"); }
	if((channels & 1UL<<15)!=0){ ConfigPortC("---- ----  --a- ----"); }
	
	APBfreq=GetAPB2Freq();
	for(i=0;i<=3;i++)
	{
		if(APBfreq/((i+1)<<1)>=18000000)	//prescaler (/2, /4, /6, /8) for 18Mhz Max
		{
			ADC_CCR |= i<<16;
			break;
		}
	}
	
	ADC2_CR1 |= resolution<<24;
	ADC2_SMPR2=SamplingTime;
	ADC2_CR2 |= 1; //Enable ADC
}

void InitADC3(unsigned int channels, unsigned int resolution, unsigned int SamplingTime)	//each bit represents a channel, LSB=channel 0
{
	unsigned int APBfreq, i;
	
	RCC_APB2ENR |= 1UL<<10;
	
	ADC_CCR=0;
	ADC3_CR1=0;
	ADC3_SQR1=0;
	
	if((channels & 1)!=0){ ConfigPortA("---- ----  ---- ---a"); }
	if((channels & 1UL<<1)!=0){ ConfigPortA("---- ----  ---- --a-"); }
	if((channels & 1UL<<2)!=0){ ConfigPortA("---- ----  ---- -a--"); }
	if((channels & 1UL<<3)!=0){ ConfigPortA("---- ----  ---- a---"); }
	if((channels & 1UL<<10)!=0){ ConfigPortC("---- ----  ---- ---a"); }
	if((channels & 1UL<<11)!=0){ ConfigPortC("---- ----  ---- --a-"); }
	if((channels & 1UL<<12)!=0){ ConfigPortC("---- ----  ---- -a--"); }
	if((channels & 1UL<<13)!=0){ ConfigPortC("---- ----  ---- a---"); }

	APBfreq=GetAPB2Freq();
	for(i=0;i<=3;i++)
	{
		if(APBfreq/((i+1)<<1)>=18000000)	//prescaler (/2, /4, /6, /8) for 18Mhz Max
		{
			ADC_CCR |= i<<16;
			break;
		}
	}
	
	ADC3_CR1 |= resolution<<24;
	ADC3_SMPR2=SamplingTime;
	ADC3_CR2 |= 1; //Enable ADC
}

unsigned int ReadADC1(unsigned int channel)
{
	ADC1_SQR3=channel;
	ADC1_CR2 |= 1UL<<30;	//start conversion
	while((ADC1_SR & (1UL<<1))==0);	//wait for end of conversion
	return ADC1_DR;	//also clears end of conversion bit
}

unsigned int ReadADC2(unsigned int channel)
{
	ADC2_SQR3=channel;
	ADC2_CR2 |= 1UL<<30;	//start conversion
	while((ADC2_SR & (1UL<<1))==0);	//wait for end of conversion
	return ADC2_DR;	//also clears end of conversion bit
}

unsigned int ReadADC3(unsigned int channel)
{
	ADC3_SQR3=channel;
	ADC3_CR2 |= 1UL<<30;	//start conversion
	while((ADC3_SR & (1UL<<1))==0);	//wait for end of conversion
	return ADC3_DR;	//also clears end of conversion bit
}

//interrupts
void EnableInterruptPosition(unsigned int InterruptPosition)
{
	switch(InterruptPosition>>5)	// div by 32 (96 bits (positions) in 3 registers)
	{
		case 0:
				NVIC_ISER0 |= 1<<InterruptPosition;	//enable timer interrupt position
		break;
		case 1:
				NVIC_ISER1 |= 1<<(InterruptPosition-32);	//enable timer interrupt position
		break;
		case 2:
				NVIC_ISER2 |= 1<<(InterruptPosition-64);	//enable timer interrupt position
		break;
	}
}

void DisableInterruptPosition(unsigned int InterruptPosition)
{
	switch(InterruptPosition>>5)	// div by 32 (96 bits (positions) in 3 registers)
	{
		case 0:
				NVIC_ISER0 &= ~(1<<InterruptPosition);	//enable timer interrupt position
		break;
		case 1:
				NVIC_ISER1 &= ~(1<<(InterruptPosition-32));	//enable timer interrupt position
		break;
		case 2:
				NVIC_ISER2 &= ~(1<<(InterruptPosition-64));	//enable timer interrupt position
		break;
	}
}

//timer
void StartTimer(unsigned int prescaler, volatile unsigned int *TIM_CR1, volatile unsigned int *TIM_PSC)	//timer runs at twice the APBx bus speed when bus divider>1. pescaler is 16 bit, and must be between 0 and 0xFFFF CK_CNT=fCK_PSC/(prescaler+1)
{	
	*TIM_CR1 &= ~(3<<8);	//no clock division, tDTS = tCK_INT
	*TIM_CR1 |= 1<<7;			//Auto-reload preload enable: TIMx_ARR register is buffered (updated only at end of period)
	*TIM_CR1 &= ~(3<<5);	//Edge-aligned mode. The counter counts up or down depending on the direction bit (DIR).
	*TIM_CR1 &= ~(1<<4);	//Counter used as upcounter
	*TIM_CR1 &= ~(1<<3);	//Counter is not stopped at update event
	*TIM_CR1 &= ~(1<<1);	//UEV enabled. The Update (UEV) event is generated by one of the following events: Counter overflow/underflow, Setting the UG bit, Update generation through the slave mode controller. Buffered registers are then loaded with their preload values.
	*TIM_PSC=prescaler;		//prescaler
	*TIM_CR1 |= 1;			//counter enabled
}

unsigned int ReadAndResetTimerOverflow(volatile unsigned int *TIM_SR)
{
	unsigned int overflow=*TIM_SR & 1;	//read flag

	if(overflow){ *TIM_SR &= ~1; }	//reset flag
	return overflow;
}

void StartTimer2(unsigned int prescaler)
{
	RCC_APB1ENR |= 1;		//timer 2 clock enable
	StartTimer(prescaler, (unsigned int *)&TIM2_CR1, (unsigned int *)&TIM2_PSC);
	TIM2_EGR |= 1;	//Re-initialize the counter and generates an update of the registers. Note that the prescaler counter is cleared too
}

unsigned int ReadTimer2(void)
{
	return TIM2_CNT;
}

void ResetTimer2(void)
{
	TIM2_EGR |= 1;	//Re-initialize the counter and generates an update of the registers. Note that the prescaler counter is cleared too
	while((TIM2_EGR & 1) != 0);
}

unsigned int ReadAndResetTimerOverflow2(void)
{
	return ReadAndResetTimerOverflow((unsigned int *)&TIM2_SR);
}

void StartTimer5(unsigned int prescaler)
{
	RCC_APB1ENR |= 1<<3;		//timer 5 clock enable
	StartTimer(prescaler, (unsigned int *)&TIM5_CR1, (unsigned int *)&TIM5_PSC);
	TIM5_EGR |= 1;	//Re-initialize the counter and generates an update of the registers. Note that the prescaler counter is cleared too
}

unsigned int ReadTimer5(void)
{
	return TIM5_CNT;
}

void ResetTimer5(void)
{
	TIM5_EGR |= 1;	//Re-initialize the counter and generates an update of the registers. Note that the prescaler counter is cleared too
	while((TIM5_EGR & 1) != 0);
}

unsigned int ReadAndResetTimerOverflow5(void)
{
	return ReadAndResetTimerOverflow((unsigned int *)&TIM5_SR);
}

void StartTimer3(unsigned int prescaler)
{
	RCC_APB1ENR |= 1<<1;		//timer 3 clock enable
	StartTimer(prescaler, (unsigned int *)&TIM3_CR1, (unsigned int *)&TIM3_PSC);
	TIM3_EGR |= 1;	//Re-initialize the counter and generates an update of the registers. Note that the prescaler counter is cleared too
}

unsigned int ReadTimer3(void)
{
	return TIM3_CNT;
}

void ResetTimer3(void)
{
	TIM3_EGR |= 1;	//Re-initialize the counter and generates an update of the registers. Note that the prescaler counter is cleared too
	while((TIM3_EGR & 1) != 0);
}

unsigned int ReadAndResetTimerOverflow3(void)
{
	return ReadAndResetTimerOverflow((unsigned int *)&TIM3_SR);
}

void StartTimer4(unsigned int prescaler)
{
	RCC_APB1ENR |= 1<<2;		//timer 4 clock enable
	StartTimer(prescaler, (unsigned int *)&TIM4_CR1, (unsigned int *)&TIM4_PSC);
	TIM4_EGR |= 1;	//Re-initialize the counter and generates an update of the registers. Note that the prescaler counter is cleared too
}

unsigned int ReadTimer4(void)
{
	return TIM4_CNT;
}

void ResetTimer4(void)
{
	TIM4_EGR |= 1;	//Re-initialize the counter and generates an update of the registers. Note that the prescaler counter is cleared too
	while((TIM4_EGR & 1) != 0);
}

unsigned int ReadAndResetTimerOverflow4(void)
{
	return ReadAndResetTimerOverflow((unsigned int *)&TIM4_SR);
}




void StartTimer9(unsigned int prescaler)
{
	RCC_APB2ENR |= 1<<16;		//timer 9 clock enable
	StartTimer(prescaler, (unsigned int *)&TIM9_CR1, (unsigned int *)&TIM9_PSC);
	TIM4_EGR |= 1;	//Re-initialize the counter and generates an update of the registers. Note that the prescaler counter is cleared too
}

unsigned int ReadTimer9(void)
{
	return TIM9_CNT;
}

void ResetTimer9(void)
{
	TIM9_EGR |= 1;	//Re-initialize the counter and generates an update of the registers. Note that the prescaler counter is cleared too
	while((TIM9_EGR & 1) != 0);
}

unsigned int ReadAndResetTimerOverflow9(void)
{
	return ReadAndResetTimerOverflow((unsigned int *)&TIM9_SR);
}

void StartTimer10(unsigned int prescaler)
{
	RCC_APB2ENR |= 1<<17;		//timer 10 clock enable
	StartTimer(prescaler, (unsigned int *)&TIM10_CR1, (unsigned int *)&TIM10_PSC);
	TIM10_EGR |= 1;	//Re-initialize the counter and generates an update of the registers. Note that the prescaler counter is cleared too
}

unsigned int ReadTimer10(void)
{
	return TIM10_CNT;
}

void ResetTimer10(void)
{
	TIM10_EGR |= 1;	//Re-initialize the counter and generates an update of the registers. Note that the prescaler counter is cleared too
	while((TIM10_EGR & 1) != 0);
}

unsigned int ReadAndResetTimerOverflow10(void)
{
	return ReadAndResetTimerOverflow((unsigned int *)&TIM10_SR);
}

void StartTimer11(unsigned int prescaler)
{
	RCC_APB2ENR |= 1<<18;		//timer 11 clock enable
	StartTimer(prescaler, (unsigned int *)&TIM11_CR1, (unsigned int *)&TIM11_PSC);
	TIM11_EGR |= 1;	//Re-initialize the counter and generates an update of the registers. Note that the prescaler counter is cleared too
}

unsigned int ReadTimer11(void)
{
	return TIM11_CNT;
}

void ResetTimer11(void)
{
	TIM11_EGR |= 1;	//Re-initialize the counter and generates an update of the registers. Note that the prescaler counter is cleared too
	while((TIM11_EGR & 1) != 0);
}

unsigned int ReadAndResetTimerOverflow11(void)
{
	return ReadAndResetTimerOverflow((unsigned int *)&TIM11_SR);
}

void StartTimer12(unsigned int prescaler)
{
	RCC_APB1ENR |= 1<<6;		//timer 12 clock enable
	StartTimer(prescaler, (unsigned int *)&TIM12_CR1, (unsigned int *)&TIM12_PSC);
	TIM10_EGR |= 1;	//Re-initialize the counter and generates an update of the registers. Note that the prescaler counter is cleared too
}

unsigned int ReadTimer12(void)
{
	return TIM12_CNT;
}

void ResetTimer12(void)
{
	TIM12_EGR |= 1;	//Re-initialize the counter and generates an update of the registers. Note that the prescaler counter is cleared too
	while((TIM12_EGR & 1) != 0);
}

unsigned int ReadAndResetTimerOverflow12(void)
{
	return ReadAndResetTimerOverflow((unsigned int *)&TIM12_SR);
}

void StartTimer13(unsigned int prescaler)
{
	RCC_APB1ENR |= 1<<7;		//timer 13 clock enable
	StartTimer(prescaler, (unsigned int *)&TIM13_CR1, (unsigned int *)&TIM13_PSC);
	TIM13_EGR |= 1;	//Re-initialize the counter and generates an update of the registers. Note that the prescaler counter is cleared too
}

unsigned int ReadTimer13(void)
{
	return TIM13_CNT;
}

void ResetTimer13(void)
{
	TIM13_EGR |= 1;	//Re-initialize the counter and generates an update of the registers. Note that the prescaler counter is cleared too
	while((TIM13_EGR & 1) != 0);
}

unsigned int ReadAndResetTimerOverflow13(void)
{
	return ReadAndResetTimerOverflow((unsigned int *)&TIM13_SR);
}

void StartTimer14(unsigned int prescaler)
{
	RCC_APB1ENR |= 1<<8;		//timer 14 clock enable
	StartTimer(prescaler, (unsigned int *)&TIM14_CR1, (unsigned int *)&TIM14_PSC);
	TIM14_EGR |= 1;	//Re-initialize the counter and generates an update of the registers. Note that the prescaler counter is cleared too
}

unsigned int ReadTimer14(void)
{
	return TIM14_CNT;
}

void ResetTimer14(void)
{
	TIM14_EGR |= 1;	//Re-initialize the counter and generates an update of the registers. Note that the prescaler counter is cleared too
	while((TIM14_EGR & 1) != 0);
}

unsigned int ReadAndResetTimerOverflow14(void)
{
	return ReadAndResetTimerOverflow((unsigned int *)&TIM14_SR);
}

unsigned int GetTimer2InputClockFreq(void)
{
	if(((RCC_CFGR>>12) & 1)==0){ return GetAPB1Freq(); }else{ return GetAPB1Freq()*2; }	//if APBx prescaler is not divided, fAPBx else 2*fAPBs
}

unsigned int GetTimer5InputClockFreq(void)
{
	if(((RCC_CFGR>>12) & 1)==0){ return GetAPB1Freq(); }else{ return GetAPB1Freq()*2; }	//if APBx prescaler is not divided, fAPBx else 2*fAPBs
}

unsigned int GetTimer3InputClockFreq(void)
{
	if(((RCC_CFGR>>12) & 1)==0){ return GetAPB1Freq(); }else{ return GetAPB1Freq()*2; }	//if APBx prescaler is not divided, fAPBx else 2*fAPBs
}

unsigned int GetTimer4InputClockFreq(void)
{
	if(((RCC_CFGR>>12) & 1)==0){ return GetAPB1Freq(); }else{ return GetAPB1Freq()*2; }	//if APBx prescaler is not divided, fAPBx else 2*fAPBs
}

unsigned int GetTimer9InputClockFreq(void)
{
	if(((RCC_CFGR>>15) & 1)==0){ return GetAPB2Freq(); }else{ return GetAPB2Freq()*2; }	//if APBx prescaler is not divided, fAPBx else 2*fAPBs
}

unsigned int GetTimer10InputClockFreq(void)
{
	if(((RCC_CFGR>>15) & 1)==0){ return GetAPB2Freq(); }else{ return GetAPB2Freq()*2; }	//if APBx prescaler is not divided, fAPBx else 2*fAPBs
}

unsigned int GetTimer11InputClockFreq(void)
{
	if(((RCC_CFGR>>15) & 1)==0){ return GetAPB2Freq(); }else{ return GetAPB2Freq()*2; }	//if APBx prescaler is not divided, fAPBx else 2*fAPBs
}

unsigned int GetTimer12InputClockFreq(void)
{
	if(((RCC_CFGR>>12) & 1)==0){ return GetAPB1Freq(); }else{ return GetAPB1Freq()*2; }	//if APBx prescaler is not divided, fAPBx else 2*fAPBs
}

unsigned int GetTimer13InputClockFreq(void)
{
	if(((RCC_CFGR>>12) & 1)==0){ return GetAPB1Freq(); }else{ return GetAPB1Freq()*2; }	//if APBx prescaler is not divided, fAPBx else 2*fAPBs
}

unsigned int GetTimer14InputClockFreq(void)
{
	if(((RCC_CFGR>>12) & 1)==0){ return GetAPB1Freq(); }else{ return GetAPB1Freq()*2; }	//if APBx prescaler is not divided, fAPBx else 2*fAPBs
}

unsigned int GetTimer2Freq(void){ return GetTimer2InputClockFreq()/(TIM2_PSC+1); }
unsigned int GetTimer5Freq(void){ return GetTimer2InputClockFreq()/(TIM5_PSC+1); }
unsigned int GetTimer3Freq(void){ return GetTimer2InputClockFreq()/(TIM3_PSC+1); }
unsigned int GetTimer4Freq(void){ return GetTimer2InputClockFreq()/(TIM4_PSC+1); }
unsigned int GetTimer9Freq(void){ return GetTimer2InputClockFreq()/(TIM9_PSC+1); }
unsigned int GetTimer10Freq(void){ return GetTimer2InputClockFreq()/(TIM10_PSC+1); }
unsigned int GetTimer11Freq(void){ return GetTimer2InputClockFreq()/(TIM11_PSC+1); }
unsigned int GetTimer12Freq(void){ return GetTimer2InputClockFreq()/(TIM12_PSC+1); }
unsigned int GetTimer13Freq(void){ return GetTimer2InputClockFreq()/(TIM13_PSC+1); }
unsigned int GetTimer14Freq(void){ return GetTimer2InputClockFreq()/(TIM14_PSC+1); }

void StartTimerInterrupt(unsigned int TimerPrescaler, unsigned int CountPeriod, volatile unsigned int *TIM_CR1, volatile unsigned int *TIM_PSC, volatile unsigned int *TIM_ARR, volatile unsigned int *TIM_DIER, volatile unsigned int *TIM_EGR, unsigned int InterruptPosition)
{	
	*TIM_CR1 &= ~(3<<8);	//no clock division, tDTS = tCK_INT
	*TIM_CR1 |= 1<<7;			//Auto-reload preload enable: TIMx_ARR register is buffered (updated only at end of period)
	*TIM_CR1 &= ~(3<<5);	//Edge-aligned mode. The counter counts up or down depending on the direction bit (DIR).
	*TIM_CR1 &= ~(1<<4);	//Counter used as upcounter
	*TIM_CR1 &= ~(1<<3);	//Counter is not stopped at update event
	*TIM_CR1 &= ~(1<<1);	//UEV enabled. The Update (UEV) event is generated by one of the following events: Counter overflow/underflow, Setting the UG bit, Update generation through the slave mode controller. Buffered registers are then loaded with their preload values.
	*TIM_PSC=TimerPrescaler;		//prescaler
	
	*TIM_ARR=CountPeriod;	//period
	*TIM_DIER |= 1;				//Update interrupt enabled
	*TIM_EGR |= 1;				//Re-initialize the counter and generates an update of the registers
	while((*TIM_EGR & 1) != 0);
	
	EnableInterruptPosition(InterruptPosition);
	
	*TIM_CR1 |= 1;				//counter enabled
}

void StartTimer2InterruptPosition(unsigned int TimerPrescaler, unsigned int CountPeriod)
{
	RCC_APB1ENR |= 1;		//timer 2 clock enable
	StartTimerInterrupt(TimerPrescaler, CountPeriod, (unsigned int *)&TIM2_CR1, (unsigned int *)&TIM2_PSC, (unsigned int *)&TIM2_ARR, (unsigned int *)&TIM2_DIER, (unsigned int *)&TIM2_EGR, TIM2_Interrupt_Position);
}

void StartTimer5InterruptPosition(unsigned int TimerPrescaler, unsigned int CountPeriod)
{
	RCC_APB1ENR |= 1<<3;		//timer 5 clock enable
	StartTimerInterrupt(TimerPrescaler, CountPeriod, (unsigned int *)&TIM5_CR1, (unsigned int *)&TIM5_PSC, (unsigned int *)&TIM5_ARR, (unsigned int *)&TIM5_DIER, (unsigned int *)&TIM5_EGR, TIM5_Interrupt_Position);
}

void StartTimer3InterruptPosition(unsigned int TimerPrescaler, unsigned int CountPeriod)
{
	RCC_APB1ENR |= 1<<1;		//timer 3 clock enable
	StartTimerInterrupt(TimerPrescaler, CountPeriod, (unsigned int *)&TIM3_CR1, (unsigned int *)&TIM3_PSC, (unsigned int *)&TIM3_ARR, (unsigned int *)&TIM3_DIER, (unsigned int *)&TIM3_EGR, TIM3_Interrupt_Position);
}

void StartTimer4InterruptPosition(unsigned int TimerPrescaler, unsigned int CountPeriod)
{
	RCC_APB1ENR |= 1<<2;		//timer 4 clock enable
	StartTimerInterrupt(TimerPrescaler, CountPeriod, (unsigned int *)&TIM4_CR1, (unsigned int *)&TIM4_PSC, (unsigned int *)&TIM4_ARR, (unsigned int *)&TIM4_DIER, (unsigned int *)&TIM4_EGR, TIM4_Interrupt_Position);
}

//capture
void InitCapture(unsigned int CaptureChannel, volatile unsigned int *TIM_CCMR1, volatile unsigned int *TIM_CCMR2, volatile unsigned int *TIM_CCER, unsigned int filter, unsigned int edge, unsigned int edgeprescaler)
{
	switch(CaptureChannel)
	{
		case Capture_Channel1:
			//filter
			*TIM_CCMR1 &= ~(0xF<<4);
			*TIM_CCMR1 |= filter<<4;
		
			//prescaler
			*TIM_CCMR1 &= ~(3<<2);
			*TIM_CCMR1 |= edgeprescaler<<2;
		
			//mapping to TI1
			*TIM_CCMR1 &= ~3;
			*TIM_CCMR1 |= 1;
		break;
		case Capture_Channel2:
			//filter
			*TIM_CCMR1 &= ~(0xF<<12);
			*TIM_CCMR1 |= filter<<12;
		
			//prescaler
			*TIM_CCMR1 &= ~(3<<10);
			*TIM_CCMR1 |= edgeprescaler<<10;
		
			//mapping to TI2
			*TIM_CCMR1 &= ~(3<<8);
			*TIM_CCMR1 |= 1<<8;
		break;
		case Capture_Channel3:
			//filter
			*TIM_CCMR2 &= ~(0xF<<4);
			*TIM_CCMR2 |= filter<<4;
		
			//prescaler
			*TIM_CCMR2 &= ~(3<<2);
			*TIM_CCMR2 |= edgeprescaler<<2;
		
			//mapping to TI3
			*TIM_CCMR2 &= ~3;
			*TIM_CCMR2 |= 1;
		break;
		case Capture_Channel4:
			//filter
			*TIM_CCMR2 &= ~(0xF<<12);
			*TIM_CCMR2 |= filter<<12;
		
			//prescaler
			*TIM_CCMR2 &= ~(3<<10);
			*TIM_CCMR2 |= edgeprescaler<<10;
		
			//mapping to TI4
			*TIM_CCMR2 &= ~(3<<8);
			*TIM_CCMR2 |= 1<<8;
		break;
	}

	switch(edge)
	{
		case Capture_Rising_Edge:
			*TIM_CCER &= ~(((1<<3) | (1<<1))<<((CaptureChannel-1)*4));
		break;
		case Capture_Falling_Edge:
			*TIM_CCER &= ~((1<<3)<<((CaptureChannel-1)*4));
			*TIM_CCER |= (1<<1)<<((CaptureChannel-1)*4);
		break;
		case Capture_Both_Edges:
			*TIM_CCER |= ((1<<3) | (1<<1))<<((CaptureChannel-1)*4);
		break;
		default:
			*TIM_CCER &= ~(((1<<3) | (1<<1))<<((CaptureChannel-1)*4));	//rising
	}

	*TIM_CCER |= 1<<((CaptureChannel-1)*4);	//capture enabled
}

void InitCaptureT2(unsigned int TimerPrescaler, unsigned int CaptureChannel, unsigned int PinMapping, unsigned int filter, unsigned int edge, unsigned int edgeprescaler)
{
	StartTimer2(TimerPrescaler);
	
	switch(PinMapping)
	{
		case Capture_TIM2_CH1_PA0:
			ConfigPortA("---- ----  ---- ---t");	//PIN in AF mode
			SetAFA(0,AF_TIM2);
		break;
		case Capture_TIM2_CH1_PA5:
			ConfigPortA("---- ----  --t- ----");	//PIN in AF mode
			SetAFA(5,AF_TIM2);
		break;
		case Capture_TIM2_CH1_PA15:
			ConfigPortA("t--- ----  ---- ----");	//PIN in AF mode
			SetAFA(15,AF_TIM2);
		break;
		
		case Capture_TIM2_CH2_PA1:
			ConfigPortA("---- ----  ---- --t-");	//PIN in AF mode
			SetAFA(1,AF_TIM2);
		break;
		case Capture_TIM2_CH2_PB3:
			ConfigPortB("---- ----  ---- t---");	//PIN in AF mode
			SetAFB(3,AF_TIM2);
		break;
		
		case Capture_TIM2_CH3_PA2:
			ConfigPortA("---- ----  ---- -t--");	//PIN in AF mode
			SetAFA(2,AF_TIM2);
		break;
		case Capture_TIM2_CH3_PB10:
			ConfigPortB("---- -t--  ---- ----");	//PIN in AF mode
			SetAFB(10,AF_TIM2);
		break;
			
		case Capture_TIM2_CH4_PA3:
			
			ConfigPortA("---- ----  ---- t---");	//PIN in AF mode
			SetAFA(3,AF_TIM2);
		break;
		case Capture_TIM2_CH4_PB11:
			
			ConfigPortB("---- t---  ---- ----");	//PIN in AF mode
			SetAFB(11,AF_TIM2);
		break;
	}
	
	InitCapture(CaptureChannel, (unsigned int *)&TIM2_CCMR1, (unsigned int *)&TIM2_CCMR2, (unsigned int *)&TIM2_CCER, filter, edge, edgeprescaler);
	ClearCaptureT2(CaptureChannel);
}

void InitCaptureT5(unsigned int TimerPrescaler, unsigned int CaptureChannel, unsigned int PinMapping, unsigned int filter, unsigned int edge, unsigned int edgeprescaler)
{
	StartTimer5(TimerPrescaler);
	
	switch(PinMapping)
	{
		case Capture_TIM5_CH1_PA0:
			ConfigPortA("---- ----  ---- ---t");	//PIN in AF mode
			SetAFA(0,AF_TIM5);
		break;
//		case Capture_TIM5_CH1_PH10:
//		break;
		
		case Capture_TIM5_CH2_PA1:
			ConfigPortA("---- ----  ---- --t-");	//PIN in AF mode
			SetAFA(1,AF_TIM5);
		break;
//		case Capture_TIM5_CH2_PH11:
//		break;
		
		case Capture_TIM5_CH3_PA2:
			
			ConfigPortA("---- ----  ---- -t--");	//PIN in AF mode
			SetAFA(2,AF_TIM5);
		break;
//		case Capture_TIM5_CH3_PH12:
//		break;
		
		case Capture_TIM5_CH4_PA3:
			
			ConfigPortA("---- ----  ---- t---");	//PIN in AF mode
			SetAFA(3,AF_TIM5);
		break;
//		case Capture_TIM5_CH4_PI0:
//		break;
	}
	
	InitCapture(CaptureChannel, (unsigned int *)&TIM5_CCMR1, (unsigned int *)&TIM5_CCMR2, (unsigned int *)&TIM5_CCER, filter, edge, edgeprescaler);
	ClearCaptureT5(CaptureChannel);
}

void InitCaptureT3(unsigned int TimerPrescaler, unsigned int CaptureChannel, unsigned int PinMapping, unsigned int filter, unsigned int edge, unsigned int edgeprescaler)
{
	StartTimer3(TimerPrescaler);
	
	switch(PinMapping)
	{
		case Capture_TIM3_CH1_PA6:
			ConfigPortA("---- ----  -t-- ----");	//PIN in AF mode
			SetAFA(5,AF_TIM3);
		break;
		case Capture_TIM3_CH1_PB4:
			ConfigPortB("---- ----  ---t ----");	//PIN in AF mode
			SetAFB(4,AF_TIM3);
		break;
		case Capture_TIM3_CH1_PC6:
			ConfigPortC("---- ----  -t-- ----");	//PIN in AF mode
			SetAFC(6,AF_TIM3);
		break;
		
		case Capture_TIM3_CH2_PA7:
			ConfigPortA("---- ----  t--- ----");	//PIN in AF mode
			SetAFA(7,AF_TIM3);
		break;
		case Capture_TIM3_CH2_PB5:
			ConfigPortB("---- ----  --t- ----");	//PIN in AF mode
			SetAFB(5,AF_TIM3);
		break;
		case Capture_TIM3_CH2_PC7:
			ConfigPortC("---- ----  t--- ----");	//PIN in AF mode
			SetAFC(7,AF_TIM3);
		break;
		
		case Capture_TIM3_CH3_PB0:
			ConfigPortB("---- ----  ---- ---t");	//PIN in AF mode
			SetAFB(0,AF_TIM3);
		break;
		case Capture_TIM3_CH3_PC8:
			ConfigPortC("---- ---t  ---- ----");	//PIN in AF mode
			SetAFC(8,AF_TIM3);
		break;
		
		case Capture_TIM3_CH4_PB1:
			ConfigPortB("---- ----  ---- --t-");	//PIN in AF mode
			SetAFB(1,AF_TIM3);
		break;
		case Capture_TIM3_CH4_PC9:
			ConfigPortC("---- --t-  ---- ----");	//PIN in AF mode
			SetAFC(9,AF_TIM3);
		break;
	}
	
	InitCapture(CaptureChannel, (unsigned int *)&TIM3_CCMR1, (unsigned int *)&TIM3_CCMR2, (unsigned int *)&TIM3_CCER, filter, edge, edgeprescaler);
	ClearCaptureT3(CaptureChannel);
}

void InitCaptureT4(unsigned int TimerPrescaler, unsigned int CaptureChannel, unsigned int PinMapping, unsigned int filter, unsigned int edge, unsigned int edgeprescaler)
{
	StartTimer4(TimerPrescaler);
	
	switch(PinMapping)
	{
		case Capture_TIM4_CH1_PB6:
			ConfigPortB("---- ----  -t-- ----");	//PIN in AF mode
			SetAFB(6,AF_TIM3);
		break;
		case Capture_TIM4_CH1_PD12:
			ConfigPortD("---t ----  ---- ----");	//PIN in AF mode
			SetAFD(12,AF_TIM3);
		break;
		
		case Capture_TIM4_CH2_PB7:
			ConfigPortB("---- ----  t--- ----");	//PIN in AF mode
			SetAFB(7,AF_TIM3);
		break;
		case Capture_TIM4_CH2_PD13:
			ConfigPortD("--t- ----  ---- ----");	//PIN in AF mode
			SetAFD(13,AF_TIM3);
		break;
		
		case Capture_TIM4_CH3_PB8:
			ConfigPortB("---- ---t  ---- ----");	//PIN in AF mode
			SetAFB(8,AF_TIM3);
		break;
		case Capture_TIM4_CH3_PD14:
			ConfigPortD("-t-- ----  ---- ----");	//PIN in AF mode
			SetAFD(14,AF_TIM3);
		break;
		
		case Capture_TIM4_CH4_PB9:
			ConfigPortB("---- --t-  ---- ----");	//PIN in AF mode
			SetAFB(9,AF_TIM3);
		break;
		case Capture_TIM4_CH4_PD15:
			ConfigPortD("t--- ----  ---- ----");	//PIN in AF mode
			SetAFD(15,AF_TIM3);
		break;	
	}
	
	InitCapture(CaptureChannel, (unsigned int *)&TIM4_CCMR1, (unsigned int *)&TIM4_CCMR2, (unsigned int *)&TIM4_CCER, filter, edge, edgeprescaler);
	ClearCaptureT4(CaptureChannel);
}

unsigned int CheckCapture(volatile unsigned int *TIM_SR, unsigned int CaptureChannel){ return (*TIM_SR>>CaptureChannel) & 1; }
unsigned int CheckCaptureT2(unsigned int CaptureChannel){ return CheckCapture((unsigned int *)&TIM2_SR, CaptureChannel); }
unsigned int CheckCaptureT5(unsigned int CaptureChannel){ return CheckCapture((unsigned int *)&TIM5_SR, CaptureChannel); }
unsigned int CheckCaptureT3(unsigned int CaptureChannel){ return CheckCapture((unsigned int *)&TIM3_SR, CaptureChannel); }
unsigned int CheckCaptureT4(unsigned int CaptureChannel){ return CheckCapture((unsigned int *)&TIM4_SR, CaptureChannel); }

void WaitCaptureT2(unsigned int CaptureChannel){ while(CheckCaptureT2(CaptureChannel)==0); }
void WaitCaptureT5(unsigned int CaptureChannel){ while(CheckCaptureT5(CaptureChannel)==0); }
void WaitCaptureT3(unsigned int CaptureChannel){ while(CheckCaptureT3(CaptureChannel)==0); }
void WaitCaptureT4(unsigned int CaptureChannel){ while(CheckCaptureT4(CaptureChannel)==0); }

void ClearCapture(volatile unsigned int *TIM_SR, unsigned int CaptureChannel){ *TIM_SR &= ~(1<<CaptureChannel); }
void ClearCaptureT2(unsigned int CaptureChannel){ ClearCapture((unsigned int *)&TIM2_SR, CaptureChannel); }
void ClearCaptureT5(unsigned int CaptureChannel){ ClearCapture((unsigned int *)&TIM5_SR, CaptureChannel); }
void ClearCaptureT3(unsigned int CaptureChannel){ ClearCapture((unsigned int *)&TIM3_SR, CaptureChannel); }
void ClearCaptureT4(unsigned int CaptureChannel){ ClearCapture((unsigned int *)&TIM4_SR, CaptureChannel); }

//these also clear the capture flag once read
unsigned int ReadCaptureT2(unsigned int CaptureChannel)
{
	switch(CaptureChannel)
	{
		case 1:
			return TIM2_CCR1;
		case 2:
			return TIM2_CCR2;
		case 3:
			return TIM2_CCR3;
		case 4:
			return TIM2_CCR4;
		default:
			return 0;
	}
}

unsigned int ReadCaptureT5(unsigned int CaptureChannel)
{
	switch(CaptureChannel)
	{
		case 1:
			return TIM5_CCR1;
		case 2:
			return TIM5_CCR2;
		case 3:
			return TIM5_CCR3;
		case 4:
			return TIM5_CCR4;
		default:
			return 0;
	}
}

unsigned int ReadCaptureT3(unsigned int CaptureChannel)
{
	switch(CaptureChannel)
	{
		case 1:
			return TIM3_CCR1;
		case 2:
			return TIM3_CCR2;
		case 3:
			return TIM3_CCR3;
		case 4:
			return TIM3_CCR4;
		default:
			return 0;
	}
}

unsigned int ReadCaptureT4(unsigned int CaptureChannel)
{
	switch(CaptureChannel)
	{
		case 1:
			return TIM4_CCR1;
		case 2:
			return TIM4_CCR2;
		case 3:
			return TIM4_CCR3;
		case 4:
			return TIM4_CCR4;
		default:
			return 0;
	}
}

//PWM
void InitPWM(volatile unsigned int *TIM_CR1, volatile unsigned int *TIM_PSC, volatile unsigned int *TIM_ARR, volatile unsigned int *TIM_CCMR1, volatile unsigned int *TIM_CCER, volatile unsigned int *TIM_CCMR2, volatile unsigned int *TIM_EGR, unsigned int prescaler, unsigned int channel, unsigned int PWMMode, unsigned int PWMPolarity, unsigned int period)	//timer runs at twice the APBx bus speed when bus divider>1. pescaler is 16 bit, and must be between 0 and 0xFFFF CK_CNT=fCK_PSC/(prescaler+1)
{	
	*TIM_CR1 &= ~(3<<8);	//no clock division, tDTS = tCK_INT
	*TIM_CR1 |= 1<<7;			//Auto-reload preload enable: TIMx_ARR register is buffered (updated only at end of period)
	*TIM_CR1 &= ~(3<<5);	//clear center aligned mode configuration bits, Edge-aligned mode. The counter counts up or down depending on the direction bit (DIR).
	*TIM_CR1 &= ~(1<<4);	//Counter used as upcounter
	*TIM_CR1 &= ~(1<<3);	//Counter is not stopped at update event
	*TIM_CR1 &= ~(1<<1);	//UEV enabled. The Update (UEV) event is generated by one of the following events: Counter overflow/underflow, Setting the UG bit, Update generation through the slave mode controller. Buffered registers are then loaded with their preload values.
	*TIM_PSC=prescaler;		//prescaler
	*TIM_CR1 |= 1<<7;			//Auto-reload preload buffered
	*TIM_ARR=period;			//set period
	if(PWMMode==PWM_Center_Aligned_mode){ *TIM_CR1 |= 1<<5; }		//Center-aligned mode 1. The counter counts up and down alternatively. Output compare interrupt flags of channels configured in output (CCxS=00 in TIMx_CCMRx register) are set only when the counter is counting down.
	
	switch(channel)
	{
		case 1:
			*TIM_CCMR1 &= ~3;			//channel is configured as output
			*TIM_CCMR1 &= ~(7<<4);
			*TIM_CCMR1 |= 6<<4;		//Output compare 1 mode
			*TIM_CCMR1 |= 1<<3;		//Output compare 1 preload enable
			
			if(PWMPolarity==PWM_Polarity_Active_High){ *TIM_CCER &= ~(1<<1); }else{ *TIM_CCER |= 1<<1; }	//OC1 output polarity=active high/low
			*TIM_CCER |= 1;			//OC1 signal is output on the corresponding output pin
		break;
		
		case 2:
			*TIM_CCMR1 &= ~(3<<8);//channel is configured as output
			*TIM_CCMR1 &= ~(7<<12);
			*TIM_CCMR1 |= 6<<12;	//Output compare 2 mode
			*TIM_CCMR1 |= 1<<11;	//Output compare 2 preload enable
		
			if(PWMPolarity==PWM_Polarity_Active_High){ *TIM_CCER &= ~(1<<5); }else{ *TIM_CCER |= 1<<5; }	//OC2 output polarity=active high/low
			*TIM_CCER |= 1<<4;		//OC2 signal is output on the corresponding output pin
		break;
		
		case 3:
			*TIM_CCMR2 &= ~3;			//channel is configured as output
			*TIM_CCMR2 &= ~(7<<4);
			*TIM_CCMR2 |= 6<<4;		//Output compare 3 mode
			*TIM_CCMR2 |= 1<<3;		//Output compare 3 preload enable
			
			if(PWMPolarity==PWM_Polarity_Active_High){ *TIM_CCER &= ~(1<<9); }else{ *TIM_CCER |= 1<<9; }	//OC3 output polarity=active high/low
			*TIM_CCER |= 1<<8;		//OC3 signal is output on the corresponding output pin
		break;
		
		case 4:
			*TIM_CCMR2 &= ~(3<<8);//channel is configured as output
			*TIM_CCMR2 &= ~(7<<12);
			*TIM_CCMR2 |= 6<<12;	//Output compare 4 mode
			*TIM_CCMR2 |= 1<<11;	//Output compare 4 preload enable
			
			if(PWMPolarity==PWM_Polarity_Active_High){ *TIM_CCER &= ~(1<<13); }else{ *TIM_CCER |= 1<<13; }	//OC4 output polarity=active high/low
			*TIM_CCER |= 1<<12;		//OC4 signal is output on the corresponding output pin
		break;
	}

	*TIM_EGR |= 1;					//Re-initialize the counter and generates an update of the registers
	*TIM_CR1 |= 1;					//counter enabled
}

void InitPWM1(unsigned int prescaler, unsigned int PWMMode, unsigned int PWMPolarity, unsigned int period, unsigned int initduty, unsigned int PinMapping)	//PWM channels on timer1
{
	unsigned int channel=1;
	
	if(PWMMode==PWM_Center_Aligned_mode)	//if centre mode, halve the period and duty and invert polarity to preserve timing and pulse width
	{
		initduty=(period-initduty)>>1;
		period=period>>1;
		if(PWMPolarity==PWM_Polarity_Active_High){ PWMPolarity=PWM_Polarity_Active_Low; }else{ PWMPolarity=PWM_Polarity_Active_High; }
	}
	
	if((PinMapping==PWM_TIM1_CH1_PA8) || (PinMapping==PWM_TIM1_CH1_PE9)){ channel=1; }
	if((PinMapping==PWM_TIM1_CH2_PA9) || (PinMapping==PWM_TIM1_CH2_PE11)){ channel=2; }
	if((PinMapping==PWM_TIM1_CH3_PA10) || (PinMapping==PWM_TIM1_CH3_PE13)){ channel=3; }
	if((PinMapping==PWM_TIM1_CH4_PA11) || (PinMapping==PWM_TIM1_CH4_PE14)){ channel=4; }

	RCC_APB2ENR |= 1;		//timer 1 clock enable
	switch(channel)
	{
		case 1:
			switch(PinMapping)
			{
				case PWM_TIM1_CH1_PA8:
					ConfigPortA("---- ---t  ---- ----");	//afio 100Mhz on TIM1_CH1 (PA8)
					SetAFA(8,AF_TIM1);
				break;
				case PWM_TIM1_CH1_PE9:
					ConfigPortE("---- --t-  ---- ----");	//afio 100Mhz on TIM1_CH1 (PE9)
					SetAFE(9,AF_TIM1);
				break;
			}
			TIM1_CCR1=initduty;						//starting duty
		break;
			
		case 2:
			switch(PinMapping)
			{
				case PWM_TIM1_CH2_PA9:
					ConfigPortA("---- --t-  ---- ----");	//afio 100Mhz on TIM1_CH2 (PA9)
					SetAFA(9,AF_TIM1);
				break;
				case PWM_TIM1_CH2_PE11:
					ConfigPortE("---- t---  ---- ----");	//afio 100Mhz on TIM1_CH2 (PE11)
					SetAFE(11,AF_TIM1);
				break;
			}
			TIM1_CCR2=initduty;						//starting duty
		break;
			
		case 3:
			switch(PinMapping)
			{
				case PWM_TIM1_CH3_PA10:
					ConfigPortA("---- -t--  ---- ----");	//afio 100Mhz on TIM1_CH3 (PA10)
					SetAFA(10,AF_TIM1);
				break;
				case PWM_TIM1_CH3_PE13:
					ConfigPortE("--t- ----  ---- ----");	//afio 100Mhz on TIM1_CH3 (PE13)
					SetAFE(12,AF_TIM1);
				break;
			}
			TIM1_CCR3=initduty;						//starting duty
		break;
		case 4:
			switch(PinMapping)
			{
				case PWM_TIM1_CH4_PA11:
					ConfigPortA("---- t---  ---- ----");	//afio 100Mhz on TIM1_CH4 (PA11)
					SetAFA(11,AF_TIM1);
				break;
				case PWM_TIM1_CH4_PE14:
					ConfigPortE("-t-- ----  ---- ----");	//afio 100Mhz on TIM1_CH4 (PE14)
					SetAFE(14,AF_TIM1);
				break;
			}
			TIM1_CCR4=initduty;						//starting duty
		break;
	}
	
	InitPWM((unsigned int *)&TIM1_CR1, (unsigned int *)&TIM1_PSC, (unsigned int *)&TIM1_ARR, (unsigned int *)&TIM1_CCMR1, (unsigned int *)&TIM1_CCER, (unsigned int *)&TIM1_CCMR2, (unsigned int *)&TIM1_EGR, prescaler, channel, PWMMode, PWMPolarity, period);
}

void InitPWM2(unsigned int prescaler, unsigned int PWMMode, unsigned int PWMPolarity, unsigned int period, unsigned int initduty, unsigned int PinMapping)	//PWM channels on timer2
{
	unsigned int channel=1;
	
	if(PWMMode==PWM_Center_Aligned_mode)	//if centre mode, halve the period and duty and invert polarity to preserve timing and pulse width
	{
		initduty=(period-initduty)>>1;
		period=period>>1;
		if(PWMPolarity==PWM_Polarity_Active_High){ PWMPolarity=PWM_Polarity_Active_Low; }else{ PWMPolarity=PWM_Polarity_Active_High; }
	}
	
	if((PinMapping==PWM_TIM2_CH1_PA0) || (PinMapping==PWM_TIM2_CH1_PA5)){ channel=1; }
	if((PinMapping==PWM_TIM2_CH2_PA1) || (PinMapping==PWM_TIM2_CH2_PB3)){ channel=2; }
	if((PinMapping==PWM_TIM2_CH3_PA2) || (PinMapping==PWM_TIM2_CH3_PB10)){ channel=3; }
	if((PinMapping==PWM_TIM2_CH4_PA3) || (PinMapping==PWM_TIM2_CH4_PB11)){ channel=4; }

	RCC_APB1ENR |= 1;		//timer 2 clock enable
	switch(channel)
	{
		case 1:
			switch(PinMapping)
			{
				case PWM_TIM2_CH1_PA0:
					ConfigPortA("---- ----  ---- ---t");	//afio 100Mhz on TIM2_CH1 (PA0)
					SetAFA(0,AF_TIM2);
				break;
				case PWM_TIM2_CH1_PA5:
					ConfigPortA("---- ----  --t- ----");	//afio 100Mhz on TIM2_CH1 (PA5)
					SetAFA(5,AF_TIM2);
				break;
			}
			TIM2_CCR1=initduty;						//starting duty
		break;
		case 2:
			switch(PinMapping)
			{
				case PWM_TIM2_CH2_PA1:
					ConfigPortA("---- ----  ---- --t-");	//afio 100Mhz on TIM2_CH2 (PA1)
					SetAFA(1,AF_TIM2);
				break;
				case PWM_TIM2_CH2_PB3:
					ConfigPortB("---- ----  ---- t---");	//afio 100Mhz on TIM2_CH2 (PB3)
					SetAFB(3,AF_TIM2);
				break;
			}
			TIM2_CCR2=initduty;						//starting duty
		break;
		case 3:
			switch(PinMapping)
			{
				case PWM_TIM2_CH3_PA2:
					ConfigPortA("---- ----  ---- -t--");	//afio 100Mhz on TIM2_CH3 (PA2)
				SetAFA(2,AF_TIM2);
				break;
				case PWM_TIM2_CH3_PB10:
					ConfigPortB("---- -t--  ---- ----");	//afio 100Mhz on TIM2_CH3 (PB10)
					SetAFB(10,AF_TIM2);
				break;
			}
			TIM2_CCR3=initduty;						//starting duty
		break;
		case 4:
			switch(PinMapping)
			{
				case PWM_TIM2_CH4_PA3:
					ConfigPortA("---- ----  ---- t---");	//afio 190Mhz on TIM2_CH4 (PA3)
					SetAFA(3,AF_TIM2);
				break;
				case PWM_TIM2_CH4_PB11:
					ConfigPortB("---- t---  ---- ----");	//afio 100Mhz on TIM2_CH4 (PB11)
					SetAFB(11,AF_TIM2);
				break;
			}
			TIM2_CCR4=initduty;						//starting duty
		break;
	}
	
	InitPWM((unsigned int *)&TIM2_CR1, (unsigned int *)&TIM2_PSC, (unsigned int *)&TIM2_ARR, (unsigned int *)&TIM2_CCMR1, (unsigned int *)&TIM2_CCER, (unsigned int *)&TIM2_CCMR2, (unsigned int *)&TIM2_EGR, prescaler, channel, PWMMode, PWMPolarity, period);
}

void InitPWM3(unsigned int prescaler, unsigned int PWMMode, unsigned int PWMPolarity, unsigned int period, unsigned int initduty, unsigned int PinMapping)	//PWM channels on timer2
{
	unsigned int channel=1;
	
	if(PWMMode==PWM_Center_Aligned_mode)	//if centre mode, halve the period and duty and invert polarity to preserve timing and pulse width
	{
		initduty=(period-initduty)>>1;
		period=period>>1;
		if(PWMPolarity==PWM_Polarity_Active_High){ PWMPolarity=PWM_Polarity_Active_Low; }else{ PWMPolarity=PWM_Polarity_Active_High; }
	}
	
	if((PinMapping==PWM_TIM3_CH1_PA6) || (PinMapping==PWM_TIM3_CH1_PB4) || (PinMapping==PWM_TIM3_CH1_PC6)){ channel=1; }
	if((PinMapping==PWM_TIM3_CH2_PA7) || (PinMapping==PWM_TIM3_CH2_PB5) || (PinMapping==PWM_TIM3_CH2_PC7)){ channel=2; }
	if(PinMapping==PWM_TIM3_CH3_PB0){ channel=3; }
	if(PinMapping==PWM_TIM3_CH4_PB1){ channel=4; }

	RCC_APB1ENR |= 1<<1;		//timer 3 clock enable
	switch(channel)
	{
		case 1:
			switch(PinMapping)
			{
				case PWM_TIM3_CH1_PA6:
					ConfigPortA("---- ----  -t-- ----");	//afio 100Mhz on TIM3_CH1 (PA6)
					SetAFA(6,AF_TIM3);
				break;
				case PWM_TIM3_CH1_PB4:
					ConfigPortB("---- ----  ---t ----");	//afio 100Mhz on TIM3_CH1 (PB4)
					SetAFB(4,AF_TIM3);
				break;
				case PWM_TIM3_CH1_PC6:
					ConfigPortC("---- ----  -t-- ----");	//afio 100Mhz on TIM3_CH1 (PC6)
					SetAFC(6,AF_TIM3);
				break;
			}
			TIM3_CCR1=initduty;						//starting duty
		break;
		case 2:
			switch(PinMapping)
			{
				case PWM_TIM3_CH2_PA7:
					ConfigPortA("---- ----  t--- ----");	//afio 100Mhz on TIM3_CH2 (PA7)
					SetAFA(7,AF_TIM3);
				break;
				case PWM_TIM3_CH2_PB5:
					ConfigPortB("---- ----  --t- ----");	//afio 100Mhz on TIM3_CH2 (PB5)
					SetAFB(5,AF_TIM3);
				break;
				case PWM_TIM3_CH2_PC7:
					ConfigPortC("---- ----  t--- ----");	//afio 100Mhz on TIM3_CH2 (PC7)
					SetAFC(7,AF_TIM3);
				break;
			}
			TIM3_CCR2=initduty;						//starting duty
		break;
		case 3:
			switch(PinMapping)
			{
				case PWM_TIM3_CH3_PB0:
					ConfigPortB("---- ----  ---- ---t");	//afio 100Mhz on TIM3_CH3 (PB0)
				SetAFB(0,AF_TIM3);
				break;
			}
			TIM3_CCR3=initduty;						//starting duty
		break;
		case 4:
			switch(PinMapping)
			{
				case PWM_TIM3_CH4_PB1:
					ConfigPortB("---- ----  ---- --t-");	//afio 100Mhz on TIM3_CH4 (PB1)
					SetAFB(1,AF_TIM3);
				break;
			}
			TIM3_CCR4=initduty;						//starting duty
		break;
	}
	
	InitPWM((unsigned int *)&TIM3_CR1, (unsigned int *)&TIM3_PSC, (unsigned int *)&TIM3_ARR, (unsigned int *)&TIM3_CCMR1, (unsigned int *)&TIM3_CCER, (unsigned int *)&TIM3_CCMR2, (unsigned int *)&TIM3_EGR, prescaler, channel, PWMMode, PWMPolarity, period);
}

void InitPWM4(unsigned int prescaler, unsigned int PWMMode, unsigned int PWMPolarity, unsigned int period, unsigned int initduty, unsigned int PinMapping)	//PWM channels on timer2
{
	unsigned int channel=1;
	
	if(PWMMode==PWM_Center_Aligned_mode)	//if centre mode, halve the period and duty and invert polarity to preserve timing and pulse width
	{
		initduty=(period-initduty)>>1;
		period=period>>1;
		if(PWMPolarity==PWM_Polarity_Active_High){ PWMPolarity=PWM_Polarity_Active_Low; }else{ PWMPolarity=PWM_Polarity_Active_High; }
	}
	
	if((PinMapping==PWM_TIM4_CH1_PB6) || (PinMapping==PWM_TIM4_CH1_PD12)){ channel=1; }
	if((PinMapping==PWM_TIM4_CH2_PB7) || (PinMapping==PWM_TIM4_CH2_PD13)){ channel=2; }
	if((PinMapping==PWM_TIM4_CH3_PB8) || (PinMapping==PWM_TIM4_CH3_PD14)){ channel=3; }
	if((PinMapping==PWM_TIM4_CH4_PB9) || (PinMapping==PWM_TIM4_CH4_PD15)){ channel=4; }

	RCC_APB1ENR |= 1<<2;		//timer 4 clock enable
	switch(channel)
	{
		case 1:
			switch(PinMapping)
			{
				case PWM_TIM4_CH1_PB6:
					ConfigPortB("---- ----  -t-- ----");	//afio 100Mhz on TIM4_CH1 (PB6)
					SetAFB(6,AF_TIM4);
				break;
				case PWM_TIM4_CH1_PD12:
					ConfigPortD("---t ----  ---- ----");	//afio 100Mhz on TIM4_CH1 (PD12)
					SetAFD(12,AF_TIM4);
				break;
			}
			TIM4_CCR1=initduty;						//starting duty
		break;
		case 2:
			switch(PinMapping)
			{
				case PWM_TIM4_CH2_PB7:
					ConfigPortB("---- ----  t--- ----");	//afio 100Mhz on TIM4_CH2 (PB7)
					SetAFB(7,AF_TIM4);
				break;
				case PWM_TIM4_CH2_PD13:
					ConfigPortD("--t- ----  ---- ----");	//afio 100Mhz on TIM4_CH2 (PD13)
					SetAFD(13,AF_TIM4);
				break;
			}
			TIM4_CCR2=initduty;						//starting duty
		break;
		case 3:
			switch(PinMapping)
			{
				case PWM_TIM4_CH3_PB8:
					ConfigPortB("---- ---t  ---- ----");	//afio 100Mhz on TIM4_CH3 (PB8)
				SetAFB(8,AF_TIM4);
				break;
				case PWM_TIM4_CH3_PD14:
					ConfigPortD("-t-- ----  ---- ----");	//afio 100Mhz on TIM4_CH3 (PD14)
					SetAFD(14,AF_TIM4);
				break;
			}
			TIM4_CCR3=initduty;						//starting duty
		break;
		case 4:
			switch(PinMapping)
			{
				case PWM_TIM4_CH4_PB9:
					ConfigPortB("---- --t-  ---- ----");	//afio 190Mhz on TIM4_CH4 (PB9)
					SetAFB(9,AF_TIM4);
				break;
				case PWM_TIM4_CH4_PD15:
					ConfigPortD("t--- ----  ---- ----");	//afio 100Mhz on TIM4_CH4 (PD15)
					SetAFD(15,AF_TIM4);
				break;
			}
			TIM4_CCR4=initduty;						//starting duty
		break;
	}
	
	InitPWM((unsigned int *)&TIM4_CR1, (unsigned int *)&TIM4_PSC, (unsigned int *)&TIM4_ARR, (unsigned int *)&TIM4_CCMR1, (unsigned int *)&TIM4_CCER, (unsigned int *)&TIM4_CCMR2, (unsigned int *)&TIM4_EGR, prescaler, channel, PWMMode, PWMPolarity, period);
}

void InitPWM5(unsigned int prescaler, unsigned int PWMMode, unsigned int PWMPolarity, unsigned int period, unsigned int initduty, unsigned int PinMapping)	//PWM channels on timer2
{
	unsigned int channel=1;
	
	if(PWMMode==PWM_Center_Aligned_mode)	//if centre mode, halve the period and duty and invert polarity to preserve timing and pulse width
	{
		initduty=(period-initduty)>>1;
		period=period>>1;
		if(PWMPolarity==PWM_Polarity_Active_High){ PWMPolarity=PWM_Polarity_Active_Low; }else{ PWMPolarity=PWM_Polarity_Active_High; }
	}
	
	if((PinMapping==PWM_TIM5_CH1_PA0) || (PinMapping==PWM_TIM5_CH1_PH10)){ channel=1; }
	if((PinMapping==PWM_TIM5_CH2_PA1) || (PinMapping==PWM_TIM5_CH2_PH11)){ channel=2; }
	if((PinMapping==PWM_TIM5_CH3_PA2) || (PinMapping==PWM_TIM5_CH3_PH12)){ channel=3; }
	if((PinMapping==PWM_TIM5_CH4_PA4) || (PinMapping==PWM_TIM5_CH4_PI0)){ channel=4; }

	RCC_APB1ENR |= 1<<3;		//timer 5 clock enable
	switch(channel)
	{
		case 1:
			switch(PinMapping)
			{
				case PWM_TIM5_CH1_PA0:
					ConfigPortA("---- ----  ---- ---t");	//afio 100Mhz on TIM5_CH1 (PA0)
					SetAFA(0,AF_TIM5);
				break;
				case PWM_TIM5_CH1_PH10:
					ConfigPortH("---- -t--  ---- ----");	//afio 100Mhz on TIM5_CH1 (PH10)
					SetAFH(10,AF_TIM5);
				break;
			}
			TIM5_CCR1=initduty;						//starting duty
		break;
		case 2:
			switch(PinMapping)
			{
				case PWM_TIM5_CH2_PA1:
					ConfigPortA("---- ----  ---- --t-");	//afio 100Mhz on TIM5_CH2 (PA1)
					SetAFA(1,AF_TIM5);
				break;
				case PWM_TIM5_CH2_PH11:
					ConfigPortH("---- t---  ---- ----");	//afio 100Mhz on TIM5_CH2 (PH11)
					SetAFH(11,AF_TIM5);
				break;
			}
			TIM5_CCR2=initduty;						//starting duty
		break;
		case 3:
			switch(PinMapping)
			{
				case PWM_TIM5_CH3_PA2:
					ConfigPortA("---- ----  ---- -t--");	//afio 100Mhz on TIM5_CH3 (PA2)
				SetAFA(2,AF_TIM5);
				break;
				case PWM_TIM5_CH3_PH12:
					ConfigPortH("---t ----  ---- ----");	//afio 100Mhz on TIM5_CH3 (PH12)
					SetAFH(12,AF_TIM5);
				break;
			}
			TIM5_CCR3=initduty;						//starting duty
		break;
		case 4:
			switch(PinMapping)
			{
				case PWM_TIM5_CH4_PA4:
					ConfigPortA("---- ----  ---t ----");	//afio 190Mhz on TIM5_CH4 (PA4)
					SetAFA(4,AF_TIM5);
				break;
				case PWM_TIM5_CH4_PI0:
					ConfigPortI("---- ----  ---- ---t");	//afio 100Mhz on TIM5_CH4 (PI0)
					SetAFI(0,AF_TIM5);
				break;
			}
			TIM5_CCR4=initduty;						//starting duty
		break;
	}
	
	InitPWM((unsigned int *)&TIM5_CR1, (unsigned int *)&TIM5_PSC, (unsigned int *)&TIM5_ARR, (unsigned int *)&TIM5_CCMR1, (unsigned int *)&TIM5_CCER, (unsigned int *)&TIM5_CCMR2, (unsigned int *)&TIM5_EGR, prescaler, channel, PWMMode, PWMPolarity, period);
}

void InitPWM9(unsigned int prescaler, unsigned int PWMMode, unsigned int PWMPolarity, unsigned int period, unsigned int initduty, unsigned int PinMapping)	//PWM channels on timer9
{
	unsigned int channel=1;
	
	if(PWMMode==PWM_Center_Aligned_mode)	//if centre mode, halve the period and duty and invert polarity to preserve timing and pulse width
	{
		initduty=(period-initduty)>>1;
		period=period>>1;
		if(PWMPolarity==PWM_Polarity_Active_High){ PWMPolarity=PWM_Polarity_Active_Low; }else{ PWMPolarity=PWM_Polarity_Active_High; }
	}
	
	if((PinMapping==PWM_TIM9_CH1_PA2) || (PinMapping==PWM_TIM9_CH1_PE5)){ channel=1; }
	if((PinMapping==PWM_TIM9_CH2_PA3) || (PinMapping==PWM_TIM9_CH2_PE6)){ channel=2; }

	RCC_APB2ENR |= 1<<16;		//timer 9 clock enable
	switch(channel)
	{
		case 1:
			switch(PinMapping)
			{
				case PWM_TIM9_CH1_PA2:
					ConfigPortA("---- ----  ---- -t--");	//afio 100Mhz on TIM9_CH1 (PA2)
					SetAFA(2,AF_TIM9);
				break;
				case PWM_TIM9_CH1_PE5:
					ConfigPortE("---- ----  --t- ----");	//afio 100Mhz on TIM9_CH1 (PE5)
					SetAFE(5,AF_TIM9);
				break;
			}
			TIM9_CCR1=initduty;						//starting duty
		break;
			
		case 2:
			switch(PinMapping)
			{
				case PWM_TIM9_CH2_PA3:
					ConfigPortA("---- ----  ---- t---");	//afio 100Mhz on TIM9_CH2 (PA3)
					SetAFA(3,AF_TIM9);
				break;
				case PWM_TIM9_CH2_PE6:
					ConfigPortE("---- ----  -t-- ----");	//afio 100Mhz on TIM9_CH2 (PE6)
					SetAFE(6,AF_TIM9);
				break;
			}
			TIM9_CCR2=initduty;						//starting duty
		break;
	}
	
	InitPWM((unsigned int *)&TIM9_CR1, (unsigned int *)&TIM9_PSC, (unsigned int *)&TIM9_ARR, (unsigned int *)&TIM9_CCMR1, (unsigned int *)&TIM9_CCER, (unsigned int *)&TIM9_CCMR2, (unsigned int *)&TIM9_EGR, prescaler, channel, PWMMode, PWMPolarity, period);
}

void SetPWM1(unsigned int channel, unsigned int duty)
{
	if(((TIM1_CR1>>5) & 1)==1){ duty=((TIM1_ARR<<1)-duty)>>1; }	//if centre mode, change the duty

	switch(channel)
	{
		case 1:
			TIM1_CCR1=duty;						//duty
		break;
		case 2:
			TIM1_CCR2=duty;						//duty
		break;
		case 3:
			TIM1_CCR3=duty;						//duty
		break;
		case 4:
			TIM1_CCR4=duty;						//duty
		break;
	}
}

void SetPWM2(unsigned int channel, unsigned int duty)
{
	if(((TIM2_CR1>>5) & 1)==1){ duty=((TIM2_ARR<<1)-duty)>>1; }	//if centre mode, change the duty

	switch(channel)
	{
		case 1:
			TIM2_CCR1=duty;						//duty
		break;
		case 2:
			TIM2_CCR2=duty;						//duty
		break;
		case 3:
			TIM2_CCR3=duty;						//duty
		break;
		case 4:
			TIM2_CCR4=duty;						//duty
		break;
	}
}

void SetPWM3(unsigned int channel, unsigned int duty)
{
	if(((TIM3_CR1>>5) & 1)==1){ duty=((TIM3_ARR<<1)-duty)>>1; }	//if centre mode, change the duty

	switch(channel)
	{
		case 1:
			TIM3_CCR1=duty;						//duty
		break;
		case 2:
			TIM3_CCR2=duty;						//duty
		break;
		case 3:
			TIM3_CCR3=duty;						//duty
		break;
		case 4:
			TIM3_CCR4=duty;						//duty
		break;
	}
}

void SetPWM4(unsigned int channel, unsigned int duty)
{
	if(((TIM4_CR1>>5) & 1)==1){ duty=((TIM4_ARR<<1)-duty)>>1; }	//if centre mode, change the duty

	switch(channel)
	{
		case 1:
			TIM4_CCR1=duty;						//duty
		break;
		case 2:
			TIM4_CCR2=duty;						//duty
		break;
		case 3:
			TIM4_CCR3=duty;						//duty
		break;
		case 4:
			TIM4_CCR4=duty;						//duty
		break;
	}
}

void SetPWM5(unsigned int channel, unsigned int duty)
{
	if(((TIM5_CR1>>5) & 1)==1){ duty=((TIM5_ARR<<1)-duty)>>1; }	//if centre mode, change the duty

	switch(channel)
	{
		case 1:
			TIM5_CCR1=duty;						//duty
		break;
		case 2:
			TIM5_CCR2=duty;						//duty
		break;
		case 3:
			TIM5_CCR3=duty;						//duty
		break;
		case 4:
			TIM5_CCR4=duty;						//duty
		break;
	}
}

void SetPWM9(unsigned int channel, unsigned int duty)
{
	if(((TIM9_CR1>>5) & 1)==1){ duty=((TIM9_ARR<<1)-duty)>>1; }	//if centre mode, change the duty

	switch(channel)
	{
		case 1:
			TIM9_CCR1=duty;						//duty
		break;
		case 2:
			TIM9_CCR2=duty;						//duty
		break;
	}
}

void PWM_UpdateEnable1(unsigned int en)
{
	if(en==1)
	{
		TIM1_CR1 &= ~(1<<1);					//enable updates
		TIM1_SR &= ~1;							//clear update event flag,	This bit is set by hardware on an update event. It is cleared by software.
		while((TIM1_SR & 1)==0);				//wait for up update event (PWMs have been applied)
	}else
	{
		TIM1_CR1 |= 1<<1;						//disable updates
	}
}

void PWM_UpdateEnable2(unsigned int en)
{
	if(en==1)
	{
		TIM2_CR1 &= ~(1<<1);					//enable updates
		TIM2_SR &= ~1;							//clear update event flag,	This bit is set by hardware on an update event. It is cleared by software.
		while((TIM2_SR & 1)==0);				//wait for up update event (PWMs have been applied)
	}else
	{
		TIM2_CR1 |= 1<<1;						//disable updates
	}
}

void PWM_UpdateEnable3(unsigned int en)
{
	if(en==1)
	{
		TIM3_CR1 &= ~(1<<1);					//enable updates
		TIM3_SR &= ~1;							//clear update event flag,	This bit is set by hardware on an update event. It is cleared by software.
		while((TIM3_SR & 1)==0);				//wait for up update event (PWMs have been applied)
	}else
	{
		TIM3_CR1 |= 1<<1;						//disable updates
	}
}

void PWM_UpdateEnable4(unsigned int en)
{
	if(en==1)
	{
		TIM4_CR1 &= ~(1<<1);					//enable updates
		TIM4_SR &= ~1;							//clear update event flag,	This bit is set by hardware on an update event. It is cleared by software.
		while((TIM4_SR & 1)==0);				//wait for up update event (PWMs have been applied)
	}else
	{
		TIM4_CR1 |= 1<<1;						//disable updates
	}
}

void PWM_UpdateEnable5(unsigned int en)
{
	if(en==1)
	{
		TIM5_CR1 &= ~(1<<1);					//enable updates
		TIM5_SR &= ~1;							//clear update event flag,	This bit is set by hardware on an update event. It is cleared by software.
		while((TIM5_SR & 1)==0);				//wait for up update event (PWMs have been applied)
	}else
	{
		TIM5_CR1 |= 1<<1;						//disable updates
	}
}

void PWM_UpdateEnable9(unsigned int en)
{
	if(en==1)
	{
		TIM9_CR1 &= ~(1<<1);					//enable updates
		TIM9_SR &= ~1;							//clear update event flag,	This bit is set by hardware on an update event. It is cleared by software.
		while((TIM9_SR & 1)==0);				//wait for up update event (PWMs have been applied)
	}else
	{
		TIM9_CR1 |= 1<<1;						//disable updates
	}
}

//RTC
void InitRTC(unsigned int year, unsigned int month, unsigned int date, unsigned int DayOfWeek, unsigned int hours, unsigned int minutes, unsigned int seconds, int DaylightSavingsOffset, unsigned int format)
{
	RCC_CFGR |= 8<<16;	//HSE/8 (8MHz xtal) to give 1Mhz to the RTC
	RCC_APB1ENR |= 1<<28;	//Power interface clock enable
	RCC_APB1ENR |= 1<<18;	//Backup SRAM interface clock enable
	PWR_CR |= 1<<8;		//the DBP bit (Access to RTC and RTC Backup registers and backup SRAM enabled) in the PWR power control register (PWR_CR) for STM32F42xxx and STM32F43xxx has to be set before these can be modified
	RCC_BDCR |= 3<<8;	//HSE oscillator clock selected for RTC
	RCC_BDCR |= 1<<15;	//RTC clock enabled
	
	//dissable write protection of RTC registers
	RTC_WPR=0xCA;
	RTC_WPR=0x53;
	
	RTC_ISR |= 1<<7; //Set INIT bit to 1 in the RTC_ISR to stop the calendar counter and allow its value can be updated
	while(((RTC_ISR>>6) & 1)==0);	//Poll INITF bit of in the RTC_ISR register to wait for initialization phase mode to be entered
	
	//write TWICE to RTC_PRER to set the prescalers to generate 1Hz
	/*
	f CK_SPRE (1Hz)=(f RTCCLK (1MHz) / (PREDIV_A+1) / (PREDIV_S+1)

	Bits 22:16 PREDIV_A[6:0]: Asynchronous prescaler factor
	This is the asynchronous division factor:
	ck_apre frequency = RTCCLK frequency/(PREDIV_A+1)

	Bits 14:0 PREDIV_S[14:0]: Synchronous prescaler factor
	This is the synchronous division factor:
	ck_spre frequency = ck_apre frequency/(PREDIV_S+1)

	1=(1000000/(99+1))/(9999+1)
	*/
	RTC_PRER = 99<<16;
	RTC_PRER |= 9999;
	
	//Load the initial time and date values in the shadow registers (RTC_TR and RTC_DR), and configure the time format (12 or 24 hours) through the FMT bit in the RTC_CR register.
	RTC_TR = 0;	//this also sets 24 hour format
	RTC_TR |= (hours/10)<<20;	//hours (tens)
	RTC_TR |= (hours%10)<<16;	//hours (units)
	RTC_TR |= (minutes/10)<<12;	//minutes (tens)
	RTC_TR |= (minutes%10)<<8;	//minutes (units)
	RTC_TR |= (seconds/10)<<4;	//seconds (tens)
	RTC_TR |= seconds%10;	//seconds (units)
	
	RTC_DR=0;
	RTC_DR |= ((year%100)/10)<<20;	//year (tens)
	RTC_DR |= (year%10)<<16;	//year (units)
	RTC_DR |= DayOfWeek<<13;	//day of week 1=monday
	RTC_DR |= (month/10)<<12;	//month (tens)
	RTC_DR |= (month%10)<<8;	//month (units)
	RTC_DR |= (date/10)<<4;	//date (tens)
	RTC_DR |= date%10;	//date (units)
	
	RTCyear=year;	//global register
	
	if(format==0){ RTC_CR &= ~(1<<6); }else{ RTC_CR |= 1<<6; }	//0=24 hour/day format, 1=AM/PM hour format
	
	RTC_ISR |= ~(1<<7);	//Exit the initialization mode by clearing the INIT bit. When the initialization sequence is complete, the calendar starts counting
	while(((RTC_ISR>>5) & 1)==0);	//After an initialization, the software must wait until RSF is set before reading the RTC_SSR, RTC_TR and RTC_DR registers

	//The daylight saving time management is performed through bits SUB1H, ADD1H, and BKP of the RTC_CR register, must be set outside initialization mode
	if(DaylightSavingsOffset>0){ RTC_CR |= 1<<16; }	//Add 1 hour (summer time change)
	if(DaylightSavingsOffset<0){ RTC_CR |= 1<<17; }	//Subtract 1 hour (winter time change)
	if(DaylightSavingsOffset==0){ RTC_CR &= ~(3<<16); }	//no DST offset
}

unsigned int IsRTCUpdated(void)
{
	return (RTC_ISR>>5) & 1;
}

struct RTCType ReadRTC(void)
{
	struct RTCType DateTime;
	
	DateTime.AMPM=(RTC_TR>>22) & 1;
	DateTime.Hours=(((RTC_TR>>20) & 3)*10)+((RTC_TR>>16) & 0xF);
	DateTime.Minutes=(((RTC_TR>>12) & 7)*10)+((RTC_TR>>8) & 0xF);
	DateTime.Seconds=(((RTC_TR>>4) & 7)*10)+(RTC_TR & 0xF);
	DateTime.Year=(RTCyear-(RTCyear%100))+(((RTC_DR>>20) & 0xF)*10)+((RTC_DR>>16) & 0xF);
	DateTime.DayOfWeek=((RTC_DR>>3) & 7);
	DateTime.Month=(((RTC_TR>>12) & 1)*10)+((RTC_TR>>8) & 0xF);
	DateTime.Date=(((RTC_TR>>4) & 3)*10)+(RTC_TR & 0xF);
	
	RTC_ISR &= ~(1>>5);	//RSF must be cleared by software after the first calendar read, and then the software must wait until RSF is set before reading again the RTC_SSR, RTC_TR and RTC_DR registers

	return DateTime;
}


// Configure the systick timer with a timebase in ns, return the true timebase
unsigned int configSYSTICK(unsigned int timebase) {
	unsigned int HCLK = GetAHBFreq();
	unsigned int HCLK_period = ((unsigned int)1000000000<<2) / HCLK;	// Multiply by 4 to get two bits of extra resolution... (may still need more)
	unsigned int HCLK_ticks = ((timebase<<2) / HCLK_period);	

	SYSTICK_CSR &= ~(1<<1);												// Disable systick interrupt
	SYSTICK_CSR |= 1<<2;												// Clock source = core clock (HCLK)
	if ((HCLK_ticks & 7) == 0) {										// Check if we can slow down the systick timer by a factor of 8 to save power
		HCLK_ticks >>= 3;
		SYSTICK_CSR &= ~(1<<2);											// Clock source = HCLK / 8
	} 
	SYSTICK_RVR = HCLK_ticks - 1;
	SYSTICK_CSR |= 1<<0;												// Enable counter

	return (HCLK_period * ((timebase<<2) / HCLK_period))>>2;
}

// Wait for n ticks of SYSTICK timer, period set with configSYSTICK(unsigned int timebase)
void waitsys (unsigned int ticks) {
	while (ticks) {
		if (SYSTICK_CSR & (1<<16)) ticks -= 1;
	}
}
