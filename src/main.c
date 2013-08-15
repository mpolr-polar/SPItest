/**
 *****************************************************************************
 **
 **  File        : main.c
 **
 **  Abstract    : main function.
 **
 **  Functions   : main
 **
 **  Environment : Eclipse with Atollic TrueSTUDIO(R) Engine
 **                STMicroelectronics STM32F4xx Standard Peripherals Library
 **
 **
 **
 *****************************************************************************
 */

/* Includes */
#include "stm32f4xx.h"
#include "stm32f4_discovery.h"
#include "delay.h"

/* Private macro */
#define SPI1_NSS_Pin		GPIO_Pin_4
#define SPI1_SCK_Pin		GPIO_Pin_5
#define SPI1_MISO_Pin		GPIO_Pin_6
#define SPI1_MOSI_Pin		GPIO_Pin_7

#define X_DATA_8b	0x06
#define Y_DATA_8b	0x07
#define Z_DATA_8b	0x08

/* Private variables */


/* Private function prototypes */
uint8_t SPI_Send(uint8_t Data);
void	SPI_Select();
void	SPI_Deselect();

void	Accel_Write(uint8_t add, uint8_t val);
uint8_t	Accel_Read(uint8_t add);

void uart_putchar(const char c);
void uart_putstr(const char* str);
void uart_putdec(uint16_t i);

/* Private functions */
uint8_t SPI_Send(uint8_t Data)
{
	uint16_t RxData;
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);		// wait until transmit complete
	SPI_I2S_SendData(SPI1,Data);										// write data to be transmitted to the SPI data register
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);	// wait until receive complete
	RxData = SPI_I2S_ReceiveData(SPI1);								// return received data from SPI data register
	return (uint8_t)RxData;
}

void SPI_Select()
{
	GPIO_ResetBits(GPIOA,SPI1_NSS_Pin);
}

void SPI_Deselect()
{
	GPIO_SetBits(GPIOA,SPI1_NSS_Pin);
}

void Accel_Write(uint8_t add, uint8_t val)
{
	SPI_Select();
	uint8_t RxData;

	RxData = SPI_Send(0b10000000 | (add<<1));
	RxData = SPI_Send(val);

	SPI_Deselect();
}

uint8_t Accel_Read(uint8_t add)
{
	SPI_Select();
	uint8_t RxData;

	RxData = SPI_Send(0b00000000 | (add<<1));
	RxData = SPI_Send(0x0);

	SPI_Deselect();

	return RxData;
}

void uart_putchar(const char c)
{
	/*Wait while there is the data to send*/
	while(USART_GetFlagStatus(USART2,USART_FLAG_TXE) == RESET){}

	/* Output the character data */
	USART_SendData(USART2,c);

	/* Wait while it have not finished sending */
	while(USART_GetFlagStatus(USART2,USART_FLAG_TC) == RESET){}
}

void uart_putstr(const char* str)
{
	char c;	//for output string

	/* Output the string data by one character */
	while(1)
	{
		c = *str++;
		if(c == 0)break;
		uart_putchar(c);
	}
}

void uart_putdec(uint16_t i)
{
	uart_putchar(i/1000 + '0');
	i %= 1000;
	uart_putchar(i/100 + '0');
	i %= 100;
	uart_putchar(i/10 + '0');
	i %= 10;
	uart_putchar(i + '0');
}

/**
 **===========================================================================
 **
 **  Abstract: main program
 **
 **===========================================================================
 */
int main(void)
{
	delay_init();

	/* initialize structure variables */
	GPIO_InitTypeDef 	GPIO_InitStructure;
	GPIO_InitTypeDef	GPIO_USARTStructure;
	SPI_InitTypeDef	 	SPI_InitStructure;
	USART_InitTypeDef	USART_InitStructure;

	/* supply clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);

	/* Configure SPI1 pins: SCK, MISO, MOSI */
	GPIO_InitStructure.GPIO_Pin		= SPI1_SCK_Pin | SPI1_MISO_Pin | SPI1_MOSI_Pin;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);

	/* Configure I/O for NSS pin: NSS */
	GPIO_InitStructure.GPIO_Pin		= SPI1_NSS_Pin;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_OUT;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIOA->ODR = SPI1_NSS_Pin;

	/* SPI1	configuration */
	SPI_InitStructure.SPI_Direction			= SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode				= SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize			= SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL				= SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA				= SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS				= SPI_NSS_Soft | SPI_NSSInternalSoft_Set;
	SPI_InitStructure.SPI_BaudRatePrescaler	= SPI_BaudRatePrescaler_32;
	SPI_InitStructure.SPI_FirstBit			= SPI_FirstBit_MSB;
	SPI_Init(SPI1, &SPI_InitStructure);
	SPI_Cmd(SPI1, ENABLE);

	//GPIO initialize for USART2
	/* Configure USART2 Tx as alternate function push_pull */
	GPIO_USARTStructure.GPIO_Pin	= GPIO_Pin_2;
	GPIO_USARTStructure.GPIO_Speed	= GPIO_Speed_100MHz;
	GPIO_USARTStructure.GPIO_Mode	= GPIO_Mode_AF;
	GPIO_USARTStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_USARTStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_USARTStructure);

	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2);
	//GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2);

	/* Configure USART2 Rx as inport floating */
/*	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
*/

	//USART initialize
	USART_InitStructure.USART_BaudRate				= 9600;
	USART_InitStructure.USART_WordLength			= USART_WordLength_8b;
	USART_InitStructure.USART_StopBits				= USART_StopBits_1;
	USART_InitStructure.USART_Parity				= USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl	= USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode					= USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);
	USART_Cmd(USART2,ENABLE);

	uint8_t i;
	uint8_t Reg;
//	int8_t  Out;
	uart_putstr("start reading\n\r");
	uart_putstr("\n\r");
	Accel_Write(0x16, 0b01010101);
	uart_putstr("Initializing accelerometer succeeded!");
	uart_putstr("\n\r");
	while(1)
	{
		for(i=0; i<3; i++){
			Reg = Accel_Read(X_DATA_8b + i);
			uart_putdec(Reg);
			uart_putstr("   ");
		}
		uart_putstr("\n\r");
	}

	return 0;
}


void EVAL_AUDIO_TransferComplete_CallBack(uint32_t pBuffer, uint32_t Size){
	/* TODO, implement your code here */
	return;
}

/*
 * Callback used by stm324xg_eval_audio_codec.c.
 * Refer to stm324xg_eval_audio_codec.h for more info.
 */
uint16_t EVAL_AUDIO_GetSampleCallBack(void){
	/* TODO, implement your code here */
	return -1;
}

