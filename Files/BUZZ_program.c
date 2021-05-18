/*******************************************************/
/************		Author: Yehia Ehab 		************/
/************		Layer: HAL     		************/
/************		SWC: BUZZ			************/
/************		Version: 1.00	 		************/
/************		Date: 27-9-2020 		************/
/*******************************************************/
/*******************************************************/

#include "STD_TYPES.h"

#include <util/delay.h>

#include "DIO_interface.h"

#include "BUZZ_config.h"
#include "BUZZ_interface.h"
#include "BUZZ_private.h"

void BUZZ_voidBuzzerOn(void)
{
	DIO_u8SetPinValue(BUZZ_PORT, BUZZ_PIN, DIO_u8PIN_HIGH);
}

void BUZZ_voidBuzzerOff(void)
{
	DIO_u8SetPinValue(BUZZ_PORT, BUZZ_PIN, DIO_u8PIN_LOW);
}

void BUZZ_voidToneControl(u16 Copy_16DelayTime)
{
	DIO_u8SetPinValue(BUZZ_PORT, BUZZ_PIN, DIO_u8PIN_HIGH);
	_delay_us(Copy_16DelayTime);
	DIO_u8SetPinValue(BUZZ_PORT, BUZZ_PIN, DIO_u8PIN_LOW);
	_delay_us(Copy_16DelayTime);
}
