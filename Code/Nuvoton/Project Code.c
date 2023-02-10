// #206912354 Ariel Ohayon
// Smpl_UART0 : while loop for UART0-TX keep transmitting 8 bytes string
//            : IRQ routine for UART0-RX keep receiving 8 bytes string & print to LCD
//            : need two learning board to perform UART communication (TX & RX at the same time)
//
// Nu-LB-NUC140
// pin32 GPB0/RX0 to another board's UART TX
// pin33 GPB1/TX0 to another board's UART RX

// SG5010 DC servo 0 - x
// pin1 : signal to PWM0/GPA12 (NUC140-pin65/NUC120-pin28)
// pin2 : Vcc
// pin3 : Gnd

// SG5010 DC servo 1 - y
// pin1 : signal to PWM1/GPA13 (NUC140-pin65/NUC120-pin28)
// pin2 : Vcc
// pin3 : Gnd


// Joystick
// Vx  = GPA0 (ADC0)
// Vy  = GPA1 (ADC1)
// Sel = GPB2


// PIR
// SIG = GPB3

#define Number_of_Bytes 1

#define  PWM_CLKSRC_SEL   0        //0: 12M, 1:32K, 2:HCLK, 3:22M
#define  PWM_ClockSource  12000000 // 12M
#define  PWM_PreScaler    119      // clock is divided by (PreScaler + 1)
#define  PWM_ClockDivider 4        // 0: 1/2, 1: 1/4, 2: 1/8, 3: 1/16, 4: 1

#define  SERVO_CYCTIME        2000 // 20ms
#define  SERVO_HITIME_MIN       50 // minimum Hi width = 0.5ms
#define  SERVO_HITIME_MAX      250 // maximum Hi width = 2.5ms

#define HITIME_MIN 30  // 0.5ms
#define HITIME_MAX 120 // 2.5ms

#include <stdio.h>
#include <string.h>
#include "DrvADC.h"
#include "Driver\DrvUART.h"
#include "Driver\DrvGPIO.h"
#include "Driver\DrvSYS.h"
#include "NUC1xx.h"
#include "NUC1xx-LB_002\LCD_Driver.h"
#include "LCD_Driver.h"

void PWM_Servo(uint8_t PWM_no, uint16_t Servo_HiTime);
void InitPWM(uint8_t PWM_no);
void scan();

volatile uint8_t comRbuf[16];
volatile uint16_t comRbytes = 0;
volatile uint16_t comRhead 	= 0;
volatile uint16_t comRtail 	= 0;

char TEXT1[16] = "TX: sending...  ";
char TEXT2[16] = "RX:             ";

int xhitime;
int yhitime;

/*---------------------------------------------------------------------------------------------------------*/
/* UART Callback function                                                                           	   */
/*---------------------------------------------------------------------------------------------------------*/
void UART_INT_HANDLE(void)
{
	uint8_t data[1] = 0x31;
	uint8_t i;
	uint8_t bInChar[1] = {0xFF};

	DrvUART_Write(UART_PORT0,data,1);
	
	while(UART0->ISR.RDA_IF==Number_of_Bytes) 
	{
		DrvUART_Read(UART_PORT0,bInChar,1);	
		if(comRbytes < 1) // check if Buffer is full
		{
			comRbuf[comRbytes] = bInChar[0];
			comRbytes++;
		}
		else if (comRbytes==Number_of_Bytes)
		{
			comRbytes=0;
		}			
	}
	if (comRbuf[0] == '1')
	{
		sprintf(TEXT2,"%s","Move Right");
		if (xhitime == 110)
		{
			xhitime = 110;
		}
		else
		{
			xhitime++;
		}
	}
	else if (comRbuf[0] == '0')
	{
		sprintf(TEXT2,"%s","Move Left");
		if (xhitime == 30)
		{
			xhitime = 30;
		}
		else
		{
			xhitime--;
		}
	}
	else if (comRbuf[0] == '2')
	{
		sprintf(TEXT2,"%s","Move Up");
		if (yhitime == 25)
		{
			yhitime = 25;
		}
		else
		{
			yhitime--;
		}
	}
	else if (comRbuf[0] == '3')
	{
		sprintf(TEXT2,"%s","Move Down");
		if (yhitime == 60)
		{
			yhitime = 60;
		}
		else
		{
			yhitime++;
		}
	}
	print_lcd(1,TEXT2);
	PWM_Servo(0, xhitime);//x
	PWM_Servo(1, yhitime);//y
}

int32_t main()
{
	int j;
	
	uint8_t TEXT[16];
	uint16_t Vx, Vy;
  uint8_t  SW;
	char mode[2]  = {"0"};
	char mode1[2] = {"4"};
	uint8_t data[1];
	uint8_t  i =0;
	uint8_t  dataout[9] = "NuMicro0";
	
	STR_UART_T sParam;

	UNLOCKREG();
  DrvSYS_Open(48000000);
	SYSCLK->PWRCON.XTL12M_EN = 1; // enable external clock (12MHz)
	SYSCLK->CLKSEL0.HCLK_S = 0;	  // select external clock (12MHz)
	LOCKREG();
	Initial_panel();
	clr_all_panel();
   	
	
	DrvGPIO_InitFunction(E_FUNC_UART0);		
	
	DrvADC_Open(ADC_SINGLE_END, ADC_SINGLE_CYCLE_OP, 0x03, INTERNAL_HCLK, 1); // ADC1 & ADC0	
	DrvGPIO_Open(E_GPB, 2, E_IO_INPUT); // SW	
	
	DrvGPIO_Open(E_GPB, 3, E_IO_INPUT); // PIR Sensor
	

	/* UART Setting */
    sParam.u32BaudRate 		= 9600;
    sParam.u8cDataBits 		= DRVUART_DATABITS_8;
    sParam.u8cStopBits 		= DRVUART_STOPBITS_1;
    sParam.u8cParity 		= DRVUART_PARITY_NONE;
    sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;

	/* Set UART Configuration */
 	if(DrvUART_Open(UART_PORT0,&sParam) != E_SUCCESS);  

	
	
	InitPWM(0);            // initialize PWM0 x
	InitPWM(1);            // initialize PWM1 y
	xhitime = 70;
	yhitime = 45;
	PWM_Servo(0, xhitime); // x
	PWM_Servo(1, yhitime); // y
	
	sprintf(data,"1");
	DrvUART_Write(UART_PORT0,data,1);
	while (!strcmp(mode,"0"))
	{
		DrvUART_Read(UART_PORT0,mode,1);
	}
	
	while(1)
	{
		if (!strcmp(mode,"1"))
		{
			DrvUART_DisableInt(UART_PORT0, DRVUART_RDAINT);
			if (DrvGPIO_GetBit(E_GPB,3))
			{
				sprintf(TEXT,"V");
				print_lcd(0,TEXT);
				for (j = 30; j < 110; j++)
				{
					PWM_Servo(0, j); // x
					DrvSYS_Delay(10000);
					DrvUART_Read(UART_PORT0,mode1,1);
					if ( (!strcmp(mode1,"0")) || (!strcmp(mode1,"1")) || (!strcmp(mode1,"2")) || (!strcmp(mode1,"3")))
					{
						scan();
					}
				}
				for (j = 110; j > 30; j--)
				{
					PWM_Servo(0, j); // x
					DrvSYS_Delay(10000);
					DrvUART_Read(UART_PORT0,mode1,1);
					if ( (!strcmp(mode1,"0")) || (!strcmp(mode1,"1")) || (!strcmp(mode1,"2")) || (!strcmp(mode1,"3")))
					{
						scan();
					}
				}
			}
			else
			{
				sprintf(TEXT,"X");
				print_lcd(0,TEXT);
			}
		}
		else if (!strcmp(mode,"2"))
		{
			DrvUART_DisableInt(UART_PORT0, DRVUART_RDAINT);
			DrvADC_StartConvert();                   // start A/D conversion
			while(DrvADC_IsConversionDone()==FALSE); // wait till conversion is done
			Vx = ADC->ADDR[0].RSLT & 0xFFF;
			Vy = ADC->ADDR[1].RSLT & 0xFFF;
			SW = DrvGPIO_GetBit(E_GPB,2);
			
			if (Vx > 3000)
			{
				if (xhitime == 30)
				{
					xhitime = 30;
				}
				else
				{
					xhitime--;
				}
			}
			else if (Vx < 1000) // 110 > xhitime > 30
			{
				if (xhitime == 110)
				{
					xhitime = 110;
				}
				else
				{
					xhitime++;
				}
			}
			
			if (Vy > 3000)
			{
				if (yhitime == 60)
				{
					yhitime = 60;
				}
				else
				{
					yhitime++;
				}
				
			}
			else if (Vy < 1000)	// 60 > yhitime > 25
			{
				if (yhitime == 25)
				{
					yhitime = 25;
				}
				else
				{
					yhitime--;
				}
			}
			
			sprintf(TEXT,"Vx = %4d",Vx);
			print_lcd(1,TEXT);
			sprintf(TEXT,"Vy = %4d",Vy);
			print_lcd(2,TEXT);
			
			sprintf(TEXT,"Button:%d",SW);
			print_lcd(3,TEXT);
			
			PWM_Servo(0, xhitime); // x
			PWM_Servo(1, yhitime); // y
			DrvSYS_Delay(10000);
		}
	}
	//DrvUART_Close(UART_PORT0);
}



void InitPWM(uint8_t PWM_no)
{
 	/* Step 1. GPIO initial */ 
	switch (PWM_no) {
		case 0 : SYS->GPAMFP.PWM0_AD13=1;     // Enable PWM0 multi-function pin
             SYSCLK->CLKSEL1.PWM01_S = PWM_CLKSRC_SEL; // Select 12Mhz for PWM clock source		
             SYSCLK->APBCLK.PWM01_EN =1;  // Enable PWM0 & PWM1 clock	
	           PWMA->PPR.CP01=1;			      // Prescaler 0~255, Setting 0 to stop output clock
	           PWMA->CSR.CSR0=0;			      // PWM clock = clock source/(Prescaler + 1)/divider
	           PWMA->PCR.CH0MOD=1;			    // 0:One-shot mode, 1:Auto-load mode
 								                          // CNR and CMR will be auto-cleared after setting CH0MOD form 0 to 1.	
	           PWMA->CNR0=0xFFFF;           // CNR : counting down   // PWM output high if CMRx+1 >= CNR
	           PWMA->CMR0=0xFFFF;		        // CMR : fix to compare  // PWM output low  if CMRx+1 <  CNR
	           PWMA->PCR.CH0INV=0;          // Inverter->0:off, 1:on
	           PWMA->PCR.CH0EN=1;			      // PWM function->0:Disable, 1:Enable
  	         PWMA->POE.PWM0=1;			      // Output to pin->0:Diasble, 1:Enable		
		         break;
		case 1 : SYS->GPAMFP.PWM1_AD14=1;     // Enable PWM1 multi-function pin
             SYSCLK->CLKSEL1.PWM01_S = PWM_CLKSRC_SEL; // Select 12Mhz for PWM clock source		
             SYSCLK->APBCLK.PWM01_EN =1;  // Enable PWM0 & PWM1 clock	
	           PWMA->PPR.CP01=1;			      // Prescaler 0~255, Setting 0 to stop output clock
	           PWMA->CSR.CSR1=0;			      // PWM clock = clock source/(Prescaler + 1)/divider
	           PWMA->PCR.CH1MOD=1;			    // 0:One-shot mode, 1:Auto-load mode
								                          // CNR and CMR will be auto-cleared after setting CH1MOD form 0 to 1.	
	           PWMA->CNR1=0xFFFF;           // CNR : counting down   // PWM output high if CMRx+1 >= CNR
	           PWMA->CMR1=0xFFFF;		        // CMR : fix to compare  // PWM output low  if CMRx+1 <  CNR		
	           PWMA->PCR.CH1INV=0;          // Inverter->0:off, 1:on
	           PWMA->PCR.CH1EN=1;			      // PWM function->0:Disable, 1:Enable
  	         PWMA->POE.PWM1=1;			      // Output to pin->0:Diasble, 1:Enable							 
						 break;
		default:break;
		}
}

void PWM_Servo(uint8_t PWM_no, uint16_t Servo_HiTime)
{
    //PWM_FreqOut = PWM_Clock / (PWM_PreScaler + 1) / PWM_ClockDivider / (PWM_CNR + 1); 
	  // Duty Cycle = (CMR0+1) / (CNR0+1)

	  //PWM setting	 
		switch(PWM_no) {
			case 0 : PWMA->CSR.CSR0 = 0;             // divider factor = 0: 1/2, 1: 1/4, 2: 1/8, 3: 1/16, 4: 1
	             PWMA->PPR.CP01 = PWM_PreScaler; // set PreScaler
			         PWMA->CNR0 = SERVO_CYCTIME -1;    // set CNR
	             PWMA->CMR0 = Servo_HiTime -1;     // set CMR
						   PWMA->POE.PWM0=1;
			         break;
			case 1 : PWMA->CSR.CSR1 = 0;             // divider factor = 0: 1/2, 1: 1/4, 2: 1/8, 3: 1/16, 4: 1
	             PWMA->PPR.CP01 = PWM_PreScaler; // set PreScaler
			         PWMA->CNR1 = SERVO_CYCTIME -1;    // set CNR
	             PWMA->CMR1 = Servo_HiTime -1;     // set CMR
			         PWMA->POE.PWM1=1;
			         break;
			default:break;
		}
}

void scan()
{
	int out = 0;
	char x[2];
	DrvUART_EnableInt(UART_PORT0, DRVUART_RDAINT, UART_INT_HANDLE);
	while (1)
	{
		DrvUART_Read(UART_PORT0,x,1);
		if (!strcmp(x,"4"))
		{
			if (out < 1000)
			{
				out++;
			}
			else
			{
				return;
			}
		}
	}
}
