/*
 * main.c
 *
 *  Created on: Sep 24, 2020
 *      Author: yehia
 */

#include "STD_TYPES.h"

#include <util/delay.h>

#include "DIO_interface.h"
#include "PORT_interface.h"
#include "ADC_interface.h"
#include "TIMERS_interface.h"
#include "USART_interface.h"

#include "SSD_interface.h"
#include "SERVO_interface.h"
#include "BUZZ_interface.h"

#define CONNECT_TIME			2000
#define DELAY_TIME				200

void main(void)
{
	Servo_t Local_Servo_tMotor = {CHANNEL_B, 0};

	PWM_t Local_PWM_tDCmotor = {TIMER2, 0};
	PWM_t Local_PWM_tLedRed = {TIMER0, 0};


	SSD_t Local_SSD_tSevenSegment = {SSD_PORTC, SSD_PORTD, SSD_PIN2, SSD_u8COMMON_CATHODE};

	u8 Local_u8ID[5]={0}, Local_u8Password[5]={0}, Local_u8Trials=3, Local_u8Action='6';
	u8 Local_u8BuzzerFlag=1, Local_u8LightFlag=1, Local_u8MotorFlag=1, Local_u8ServoFlag=1;
	u16 Local_u16Ldr, Local_u16Digital, Local_u16Temperature;

	PORT_voidInit();
	SERVO_voidInit(&Local_Servo_tMotor);
	PWM_voidInit(&Local_PWM_tDCmotor);
	PWM_voidInit(&Local_PWM_tLedRed);
	ADC_voidInit();
	USART_voidInit();

	_delay_ms(CONNECT_TIME);
	USART_voidSendStringSynch("Welcome to smart home, please enter your ID");

	while(1)
	{
		USART_u8ReceiveBufferSynch(Local_u8ID, 4);
		Local_u8ID[4] = '\0';

		if(Local_u8ID[0]!=0 && Local_u8ID[1]!=0 && Local_u8ID[2]!=0 && Local_u8ID[3]!=0)
		{
			USART_voidSendStringSynch("Please enter your Password");
			while(1)
			{
				SSD_voidSendNumber(&Local_SSD_tSevenSegment, Local_u8Trials);
				SSD_voidDisplayOn(&Local_SSD_tSevenSegment);

				USART_u8ReceiveBufferSynch(Local_u8Password, 4);
				Local_u8Password[4]='\0';
				if(Local_u8Password[0]!=0 && Local_u8Password[1]!=0 && Local_u8Password[2]!=0 && Local_u8Password[3]!=0)
				{
					if(Local_u8Password[0]==Local_u8ID[3] && Local_u8Password[1]==Local_u8ID[2] && Local_u8Password[2]==Local_u8ID[1] && Local_u8Password[3]==Local_u8ID[0])
					{
						SSD_voidDisplayOff(&Local_SSD_tSevenSegment);
						USART_voidSendStringSynch("Welcome to your home, to open or close the door say one, to open or close the light say two, light level is adjustable, to open or close the fan say three, fan speed is adjustable, to play music say four, and to close the system say five");
						while(1)
						{
							ADC_u8StartConversionSynch(0, &Local_u16Ldr);
							ADC_u8StartConversionSynch(1, &Local_u16Digital);

							Local_u16Temperature = ((u32)Local_u16Digital * 5000UL) / 256UL;
							Local_u16Temperature/=10;
							USART_u8ReceiveData(&Local_u8Action);

							if(Local_u8LightFlag==0)
								Local_PWM_tLedRed.PWM_Value = ~((u8)(Local_u16Ldr));
							else if(Local_u8LightFlag==1)
								Local_PWM_tLedRed.PWM_Value = 0;
							PWM_voidAnalogWrite(&Local_PWM_tLedRed);

							if(Local_u8MotorFlag==0)
							{
								if(Local_u16Temperature>31)
									Local_PWM_tDCmotor.PWM_Value=255;
								else if(Local_u16Temperature>29)
									Local_PWM_tDCmotor.PWM_Value=230;
								else if(Local_u16Temperature>26)
									Local_PWM_tDCmotor.PWM_Value=215;
								else if(Local_u16Temperature>22)
									Local_PWM_tDCmotor.PWM_Value=200;
								else if(Local_u16Temperature>20)
									Local_PWM_tDCmotor.PWM_Value=185;
								else
									Local_PWM_tDCmotor.PWM_Value=0;
							}
							else if(Local_u8MotorFlag==1)
								Local_PWM_tDCmotor.PWM_Value = 0;
							PWM_voidAnalogWrite(&Local_PWM_tDCmotor);

							if(Local_u8BuzzerFlag==0)
								BUZZ_voidToneControl(20000);
							else if(Local_u8BuzzerFlag==1)
								BUZZ_voidBuzzerOff();

							if(Local_u8ServoFlag==0)
								Local_Servo_tMotor.Copy_u16Angle=180;
							else if(Local_u8ServoFlag==1)
								Local_Servo_tMotor.Copy_u16Angle=0;
							SERVO_voidRotate(&Local_Servo_tMotor);

							if(Local_u8Action=='5')
							{
								USART_voidSendStringSynch("System is shutting down");
								while(1);
							}
							else if(Local_u8Action=='4')
							{
								Local_u8BuzzerFlag = !(Local_u8BuzzerFlag);
								Local_u8Action='6';
							}
							else if(Local_u8Action=='3')
							{
								Local_u8MotorFlag = !(Local_u8MotorFlag);
								Local_u8Action='6';
							}
							else if(Local_u8Action=='2')
							{
								Local_u8LightFlag = !(Local_u8LightFlag);
								Local_u8Action='6';
							}
							else if(Local_u8Action=='1')
							{
								Local_u8ServoFlag = !(Local_u8ServoFlag);
								Local_u8Action = '6';
							}

						}
					}
					else
					{
						if(Local_u8Trials>1)
						{
							Local_u8Password[0]=0;Local_u8Password[1]=0;Local_u8Password[2]=0;Local_u8Password[3]=0;
							Local_u8Trials--;
							USART_voidSendStringSynch("Wrong Password, Try again");
						}
						else
						{
							Local_u8Trials--;
							SSD_voidSendNumber(&Local_SSD_tSevenSegment,Local_u8Trials);
							USART_voidSendStringSynch("No more trials, system is shutting down");
							while(1);
						}

					}
				}
			}

		}
	}
}
