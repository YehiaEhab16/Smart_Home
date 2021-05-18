/*******************************************************/
/*******************************************************/
/************		Author: Yehia Ehab 		************/
/************		Layer: MCAL     		************/
/************		SWC: USART 				************/
/************		Version: 1.00	 		************/
/************		Date: 6-10-2020 		************/
/*******************************************************/
/*******************************************************/


/* Preprocessor file guard */
#ifndef USART_INTERFACE_H_
#define USART_INTERFACE_H_


void USART_voidInit(void);

u8 USART_voidSendData(u8 Copy_u8Data);

u8 USART_u8ReceiveData(u8* Copy_pu8Data);

u8 USART_voidSendStringSynch(const char* Copy_pchString);
u8 USART_voidSendStringAsynch(const char* Copy_pchString, void (*Copy_pvNotificationFunc)(void));

u8 USART_u8ReceiveBufferSynch(u8* Copy_pu8Data, u8 Copy_u8BufferSize);
u8 USART_u8ReceiveBufferAsynch(u8* Copy_pu8Data, void(*Copy_pvNotificationFunc)(void), u8 Copy_u8BufferSize);

//Asynchronous must enable Global Interrupt


#endif

