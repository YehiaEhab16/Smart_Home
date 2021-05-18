/*******************************************************/
/*******************************************************/
/************		Author: Yehia Ehab 		************/
/************		Layer: MCAL     		************/
/************		SWC: USART 				************/
/************		Version: 1.00	 		************/
/************		Date: 6-10-2020 		************/
/*******************************************************/
/*******************************************************/

#include "STD_TYPES.h"
#include "BIT_MATH.h"

#include "USART_config.h"
#include "USART_interface.h"
#include "USART_private.h"
#include "USART_register.h"

static u8 USART_u8State=IDLE, Global_u8Index, Global_u8BufferSize;

static u8* Global_pu8Data;

static char* Global_pchString;
static void (*USART_pvCallBackFunc)(void);

void USART_voidInit(void)
{
	//variable to hold UCSRC value
	u8 Local_u8UCSRC=0, Local_u16BaudRate=0;

	//Select the UCSRC register
	SET_BIT(Local_u8UCSRC, UCSRC_URSEL);

	//Select UART Mode
	#if USART_MODE==ASYNCH
	CLR_BIT(Local_u8UCSRC, UCSRC_UMSEL);

	#elif USART_MODE==SYNCH
	SET_BIT(Local_u8UCSRC, UCSRC_UMSEL);

	#if CLC_POLARITY==0
	CLR_BIT(Local_u8UCSRC, UCSRC_UCPOL);

	#elif CLC_POLARITY==1
	SET_BIT(Local_u8UCSRC, UCSRC_UCPOL);

	#endif

	#endif

	//Select Parity Mode
	#if USART_PARITY==DISABLED
	CLR_BIT(Local_u8UCSRC, UCSRC_UPM1);
	CLR_BIT(Local_u8UCSRC, UCSRC_UPM0);

	#elif USART_PARITY==EVEN_PARITY
	SET_BIT(Local_u8UCSRC, UCSRC_UPM1);
	CLR_BIT(Local_u8UCSRC, UCSRC_UPM0);

	#elif USART_PARITY==ODD_PARITY
	SET_BIT(Local_u8UCSRC, UCSRC_UPM1);
	SET_BIT(Local_u8UCSRC, UCSRC_UPM0);

	#endif

	//Select Stop Bit
	#if STOP_BIT==1
	CLR_BIT(Local_u8UCSRC, UCSRC_USBS);

	#elif STOP_BIT==2
	SET_BIT(Local_u8UCSRC, UCSRC_USBS);

	#endif

	//Select Character Size
	#if CHAR_SIZE==5
	CLR_BIT(Local_u8UCSRC, UCSRC_UCSZ0);
	CLR_BIT(Local_u8UCSRC, UCSRC_UCSZ1);
	CLR_BIT(UCSRB, UCSRB_UCSZ2);

	#elif CHAR_SIZE==6
	SET_BIT(Local_u8UCSRC, UCSRC_UCSZ0);
	CLR_BIT(Local_u8UCSRC, UCSRC_UCSZ1);
	CLR_BIT(UCSRB, UCSRB_UCSZ2);

	#elif CHAR_SIZE==7
	CLR_BIT(Local_u8UCSRC, UCSRC_UCSZ0);
	SET_BIT(Local_u8UCSRC, UCSRC_UCSZ1);
	CLR_BIT(UCSRB, UCSRB_UCSZ2);

	#elif CHAR_SIZE==8
	SET_BIT(Local_u8UCSRC, UCSRC_UCSZ0);
	SET_BIT(Local_u8UCSRC, UCSRC_UCSZ1);
	CLR_BIT(UCSRB, UCSRB_UCSZ2);

	#elif CHAR_SIZE==9
	SET_BIT(Local_u8UCSRC, UCSRC_UCSZ0);
	SET_BIT(Local_u8UCSRC, UCSRC_UCSZ1);
	SET_BIT(UCSRB, UCSRB_UCSZ2);

	#endif

	//Assign the UCSRC value in one operation
	UCSRC = Local_u8UCSRC;

	//Set Baud rate (10 MHz)
	Local_u16BaudRate = (u16)(((f32)10000000 / (f32)(16 * BAUD_RATE)) - 1);

	UBRRH = (u8)(Local_u16BaudRate>>8);

	UBRRL = (u8)Local_u16BaudRate;

	//Transmitter Enable
	SET_BIT(UCSRB, UCSRB_TXEN);

	//Receiver Enable
	SET_BIT(UCSRB, UCSRB_RXEN);
}

u8 USART_voidSendData(u8 Copy_u8Data)
{
	u8 Local_u8ErrorState=OK;
	u32 Local_u32Counter=0;

	if(USART_u8State==IDLE)
	{
		USART_u8State=BUSY;
		//Wait until transmit buffer is empty
		while((GET_BIT(UCSRA, UCSRA_UDRE))==0 && (Local_u32Counter<USART_u32Timeout))
		{
			Local_u32Counter++;
		}

		if(Local_u32Counter==USART_u32Timeout)
			Local_u8ErrorState = TIMEOUT_STATE;
		else
			UDR = Copy_u8Data;	//Send the data

		USART_u8State = IDLE;
	}
	else
		Local_u8ErrorState=BUSY_STATE;

	return Local_u8ErrorState;
}

u8 USART_u8ReceiveData(u8* Copy_pu8Data)
{
	u8 Local_u8ErrorState=OK;
	u32 Local_u32Counter=0;

	if(Copy_pu8Data!=NULL)
	{
		if(USART_u8State==IDLE)
		{
			USART_u8State=BUSY;
			//Wait until transmit buffer is empty
			while((GET_BIT(UCSRA, UCSRA_RXC))==0 && (Local_u32Counter<USART_u32Timeout))
			{
				Local_u32Counter++;
			}

			if(Local_u32Counter==USART_u32Timeout)
				Local_u8ErrorState = TIMEOUT_STATE;
			else
				*Copy_pu8Data = UDR;	//Receive the data

			USART_u8State = IDLE;
		}
		else
			Local_u8ErrorState=BUSY_STATE;
	}
	else
		Local_u8ErrorState=NULL_POINTER;

	return Local_u8ErrorState;
}

u8 USART_voidSendStringSynch(const char* Copy_pchString)
{
	u8 Local_u8ErrorState=OK, Local_u8Iterator=0;

	if(Copy_pchString!=NULL)
		while(*(Copy_pchString+Local_u8Iterator)!='\0')
		{
			USART_voidSendData((u8)(*(Copy_pchString+Local_u8Iterator)));
			Local_u8Iterator++;
		}
	else
		Local_u8ErrorState=NULL_POINTER;

	return Local_u8ErrorState;
}
u8 USART_voidSendStringAsynch(const char* Copy_pchString, void (*Copy_pvNotificationFunc)(void))
{
	u8 Local_u8ErrorState=OK;

	if((Copy_pchString!=NULL) && (Copy_pvNotificationFunc!=NULL))
	{
		if(USART_u8State==IDLE)
		{
			USART_u8State=BUSY;

			Global_pchString = Copy_pchString;
			USART_pvCallBackFunc=Copy_pvNotificationFunc;
			Global_u8Index=0;

			//Enable Empty Register Interrupt Enable
			SET_BIT(UCSRB, UCSRB_UDRIE);
		}
		else
			Local_u8ErrorState=BUSY_STATE;
	}
	else
		Local_u8ErrorState=NULL_POINTER;

	return Local_u8ErrorState;
}

u8 USART_u8ReceiveBufferSynch(u8* Copy_pu8Data, u8 Copy_u8BufferSize)
{
	u8 Local_u8ErrorState=OK, Local_u8Iterator;
	u32 Local_u32Counter=0;

	if(Copy_pu8Data!=NULL)
		for(Local_u8Iterator=0;Local_u8Iterator<Copy_u8BufferSize;Local_u8Iterator++)
			{
				if(USART_u8State==IDLE)
				{
					USART_u8State=BUSY;
					//Wait until transmit buffer is empty
					while((GET_BIT(UCSRA, UCSRA_RXC))==0 && (Local_u32Counter<USART_u32Timeout))
					{
						Local_u32Counter++;
					}

					if(Local_u32Counter==USART_u32Timeout)
						Local_u8ErrorState = TIMEOUT_STATE;
					else
						*(Copy_pu8Data+Local_u8Iterator) = UDR;	//Receive the data

					USART_u8State = IDLE;
				}
				else
					Local_u8ErrorState=BUSY_STATE;
			}
	else
		Local_u8ErrorState=NULL_POINTER;

	return Local_u8ErrorState;
}
u8 USART_u8ReceiveBufferAsynch(u8* Copy_pu8Data, void(*Copy_pvNotificationFunc)(void), u8 Copy_u8BufferSize)
{
	u8 Local_u8ErrorState=OK;

	if((Copy_pu8Data!=NULL) && (Copy_pvNotificationFunc!=NULL))
	{
		if(USART_u8State==IDLE)
		{
			USART_u8State=BUSY;

			Global_pu8Data = Copy_pu8Data;
			Global_u8BufferSize=Copy_u8BufferSize;
			USART_pvCallBackFunc=Copy_pvNotificationFunc;
			Global_u8Index=0;

			//Enable Receive Interrupt Enable
			SET_BIT(UCSRB, UCSRB_RXCIE);
		}
		else
			Local_u8ErrorState=BUSY_STATE;
	}
	else
		Local_u8ErrorState=NULL_POINTER;

	return Local_u8ErrorState;
}

void __vector_13 (void)		__attribute__((signal));	//Receive Complete
void __vector_13 (void)
{
	*(Global_pu8Data+Global_u8Index) = UDR;	//Receive the data
	Global_u8Index++;

	if(Global_u8Index==Global_u8BufferSize)
	{
		//State is Idle
		USART_u8State=IDLE;

		//Disable Interrupt Enable
		CLR_BIT(UCSRB, UCSRB_RXCIE);

		//Invoke Notification Function
		USART_pvCallBackFunc();
	}
}

void __vector_14 (void)		__attribute__((signal));	//Data Register Empty
void __vector_14 (void)
{
	UDR = *(Global_pchString+Global_u8Index);	//Send the data
	Global_u8Index++;

	if(*(Global_pchString+Global_u8Index)=='\0')
	{
		//State is Idle
		USART_u8State=IDLE;

		//Disable Interrupt Enable
		CLR_BIT(UCSRB, UCSRB_UDRIE);

		//Invoke Notification Function
		USART_pvCallBackFunc();
	}
}
