#include "main.h"
#include "rcc.h"
#include "lcd.h"
#include "tim.h"
#include "adc.h"
#include "usart.h"
#include "key_led.h"

#define Ka	225
#define Kb	112.5
#define Kadc 3.3/4096

void key_proc(void);
void led_proc(void);
void lcd_proc(void);
void pwm_proc(void);
void adc_proc(void);

//for delay
__IO uint32_t uwTick_key, uwTick_led, uwTick_lcd, uwTick_pwm, uwTick_adc;

//for led
uint8_t led;

//for key
uint8_t key_val, key_old, key_down, key_up;

//for lcd
uint8_t string_lcd[21];
uint8_t which_index;

//for pwms
uint32_t plus_t;

uint32_t pwm1_t;
uint32_t pwm1_d;
float pwm1_duty;

uint32_t pwm2_t;
uint32_t pwm2_d;
float pwm2_duty;

//for flags
uint8_t set_pwm_flag;
uint8_t which_pwm;

//for guang_min
uint16_t TRAO_val;
uint16_t TRAO_old;
uint8_t light_flag; //0liang  1an
uint8_t light_flag_old = 1;

//for usart
char rx_buf[50];
char tx_buf[50];

uint8_t a_;
uint8_t a_a;

uint8_t b_;
uint8_t b_b;

uint8_t q_;
uint8_t q_a;
uint8_t q_b;

uint8_t buf_ready;

uint8_t send_a1;
uint8_t send_a2;


//for tasks
float a;
float b;
float a_old;
float b_old;
float ax;
float bx;

uint8_t Pax = 20;
uint8_t Pbx = 20;
uint32_t Pf = 1000;

uint8_t index_ab = 0;
float a_buf[5];
float b_buf[5];
float a_buf_pai[5];
float b_buf_pai[5];


char mode = 'A';

int main(void)
{
  HAL_Init();
	//HAL_Delay(1);
  SystemClock_Config();
	
	key_led_init();
	
	PLUS_Init();
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);
	HAL_TIM_Base_Start(&htim2);
	
	MX_USART1_UART_Init();
	HAL_UART_Receive_IT(&huart1, (uint8_t *)rx_buf, 1);
	
//	PWM1_Init();
//	HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
//	HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_2);
//	HAL_TIM_Base_Start(&htim3);
	MX_ADC2_Init();
	
	LCD_Init();
	LCD_SetTextColor(White);
	LCD_SetBackColor(Black);
	LCD_Clear(Black);

  while (1)
	{
		led_proc();
		key_proc();
		lcd_proc();
		pwm_proc();
		adc_proc();
  }
}

void led_proc(void)
{
	if(uwTick - uwTick_led < 50)	return;
	uwTick_led = uwTick;
	
	if(ax > Pax)
		led |= 0x01;
	else 
		led &= ~0x01;
	
	if(bx > Pbx)
		led |= 0x02;
	else 
		led &= ~0x02;
	
	if((1000000/plus_t) > Pf)
		led |= 0x04;
	else 
		led &= ~0x04;
	
	if(mode == 'A')
		led |= 0x08;
	else 
		led &= ~0x08;		
	
	if((90-b+a)<10)
		led |= 0x10;
	else 
		led &= ~0x10;	
	
	led_disp(led);
}

void key_proc(void)
{
	if(uwTick - uwTick_key < 50)	return;
	uwTick_key = uwTick;

	key_val = read_key();
	key_down = key_val & (key_val ^ key_old);
	key_up  = ~key_val & (key_val ^ key_old);
	key_old = key_val;
	
	if(key_down == 1)
	{
		//set_pwm_flag = 1;
		LCD_Clear(Black);
		which_index ^= 1;
	}
	
	else if(key_down == 2)
	{
		if(which_index == 1)
		{
			Pax += 10;
			Pbx += 10;
			if(Pax > 60)
				Pax = 10;
			if(Pbx > 60)
				Pbx = 10;		
		}
	}
	
	else if(key_down == 3)
	{
		if(which_index == 1)
		{
			Pf += 1000;
			if(Pf > 10000)
				Pf = 1000;
		}
		
		else 
		{
			if(mode == 'A')
				mode = 'B';
			else 
				mode = 'A';
		}
	}
	
	else if(key_down == 4)
	{
		if(mode == 'A')
			set_pwm_flag = 1;
	}
}

void adc_proc(void)
{
	if(uwTick - uwTick_adc < 50)	return;
	uwTick_adc = uwTick;
	
	TRAO_val = get_TRAO();
	if(TRAO_val < 3500)
		light_flag = 1;
	else 
		light_flag = 0;
	
	if(light_flag_old==1 && light_flag == 0)
		if(mode == 'B')		
			set_pwm_flag = 1;
	
	light_flag_old = light_flag;

}

void pwm_proc(void)
{
	if(uwTick - uwTick_pwm < 50)	return;
	uwTick_pwm = uwTick;
	
	int i;
	
	if(set_pwm_flag)
	{
		PWM1_Init();
		//which_pwm = 0;
		HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
		HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_2);
		HAL_TIM_Base_Start(&htim3);
		//while(get_pwm1_flag==0);
		HAL_Delay(20);
		HAL_TIM_IC_Stop_IT(&htim3, TIM_CHANNEL_1);
		HAL_TIM_IC_Stop_IT(&htim3, TIM_CHANNEL_2);
		HAL_TIM_Base_Stop(&htim3);
		
		PWM2_Init();
		//which_pwm = 1;
		HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
		HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_2);
		HAL_TIM_Base_Start(&htim3);
		
		//while(get_pwm2_flag==0);
		HAL_Delay(20);
		HAL_TIM_IC_Stop_IT(&htim3, TIM_CHANNEL_1);
		HAL_TIM_IC_Stop_IT(&htim3, TIM_CHANNEL_2);
		HAL_TIM_Base_Stop(&htim3);
		
		if(pwm1_duty < 0.1)
			a = 0;
		else if(pwm1_duty > 0.9)
			a = 180;
		else 
			a = pwm1_duty*Ka-22.5;
		
		if(pwm2_duty < 0.1)
			b = 0;
		else if(pwm2_duty > 0.9)
			b = 90;
		else 
			b = pwm2_duty*Kb-11.25;

		ax = a - a_old;
		if(ax < 0)
			ax = -ax;
		bx = b - b_old;	
		if(bx < 0)
			bx = -bx;
		
		a_old = a;
		b_old = b;
		
		set_pwm_flag = 0;
		
		if(index_ab < 5)
		{
			a_buf[index_ab] = a;
			b_buf[index_ab] = b;	
			index_ab += 1;			
		}

		else if(index_ab == 5)
		{
			for(i=0; i<4; i++)
			{
				a_buf[i] = a_buf[i+1];
				b_buf[i] = b_buf[i+1];
			}
			a_buf[4] = a;
			b_buf[4] = b;	
		}
	}
}



void lcd_proc(void)
{
	if(uwTick - uwTick_lcd < 50)	return;
	uwTick_lcd = uwTick;
	
	if(which_index == 0)
	{
		sprintf((char *)string_lcd, "        DATA");
		LCD_DisplayStringLine(Line1, string_lcd);
		
		sprintf((char *)string_lcd, "   a:%.1f   ", a);
		LCD_DisplayStringLine(Line2, string_lcd);

		sprintf((char *)string_lcd, "   b:%.1f   ", b);
		LCD_DisplayStringLine(Line3, string_lcd);
	
		sprintf((char *)string_lcd, "   f:%dHz  ", 1000000/plus_t);
		LCD_DisplayStringLine(Line4, string_lcd);
		
		sprintf((char *)string_lcd, "   ax:%.0f   ", ax);
		LCD_DisplayStringLine(Line6, string_lcd);

		sprintf((char *)string_lcd, "   bx:%.0f   ", bx);
		LCD_DisplayStringLine(Line7, string_lcd);
	
		sprintf((char *)string_lcd, "   mode:%c   ", mode);
		LCD_DisplayStringLine(Line8, string_lcd);
	}
	
	else 
	{
		sprintf((char *)string_lcd, "        PARA");
		LCD_DisplayStringLine(Line1, string_lcd);
		
		sprintf((char *)string_lcd, "   Pax:%d", (uint32_t)Pax);
		LCD_DisplayStringLine(Line2, string_lcd);

		sprintf((char *)string_lcd, "   Pax:%d", (uint32_t)Pbx);
		LCD_DisplayStringLine(Line3, string_lcd);	
		
		sprintf((char *)string_lcd, "   Pf:%d   ", Pf);
		LCD_DisplayStringLine(Line4, string_lcd);	
	}

}


void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM2)
	{
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
		{
			plus_t = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
		}
	}
	
	else if(htim->Instance == TIM3)
	{
		if(which_pwm == 0)
		{
			if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
			{
				pwm1_t = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
				pwm1_duty = (float)pwm1_d / pwm1_t;
			}
			else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
			{
				pwm1_d = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
			}				
		}
		
		else if(which_pwm == 1)
		{
			if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
			{
				pwm2_t = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
				pwm2_duty = (float)pwm2_d / pwm2_t;
			}
			else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
			{
				pwm2_d = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
			}		
		
		}

	}
	
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	int i,j;
	float temp;
	
	if(q_ == 0)
	{
		if((rx_buf[0] == 'a') && (a_ == 0))
		{
			a_ = 1;	
			HAL_UART_Receive_IT(&huart1, (uint8_t *)rx_buf, 1);
		}
		
		else if((rx_buf[0] == 'b') && (b_ == 0))
		{
			b_ = 1;	
			HAL_UART_Receive_IT(&huart1, (uint8_t *)rx_buf, 1);
		}	
	}

	
	else if(a_==1 || b_==1) 
	{
		if((rx_buf[0] == '?') && (a_ == 1))
		{
			sprintf(tx_buf, "a:%.1f\r\n", a);
			HAL_UART_Transmit(&huart1, (uint8_t *)tx_buf, strlen(tx_buf), 50);
			a_ = 0;
			HAL_UART_Receive_IT(&huart1, (uint8_t *)rx_buf, 1);
		}
		
		else if((rx_buf[0] == '?') && (b_ == 1))
		{
			sprintf(tx_buf, "b:%.1f\r\n", b);
			HAL_UART_Transmit(&huart1, (uint8_t *)tx_buf, strlen(tx_buf), 50);
			b_ = 0;
			HAL_UART_Receive_IT(&huart1, (uint8_t *)rx_buf, 1);
		}	
		
		else if((rx_buf[0] == 'a') && (a_ == 1))
		{
			a_a = 1;
			a_ = 0;
			HAL_UART_Receive_IT(&huart1, (uint8_t *)rx_buf, 1);
		}
		
		else if((rx_buf[0] == '?') && (a_a == 1))
		{
			sprintf(tx_buf, "aa:%.1f-%.1f-%.1f-%.1f-%.1f\r\n", a_buf[4], a_buf[3], a_buf[2], a_buf[1], a_buf[0]);
			HAL_UART_Transmit(&huart1, (uint8_t *)tx_buf, strlen(tx_buf), 50);
			a_ = 0;
			a_a = 0;
			
			HAL_UART_Receive_IT(&huart1, (uint8_t *)rx_buf, 1);	
		}
		
		else if((rx_buf[0] == 'b') && (b_ == 1))
		{
			b_b = 1;
			b_ = 0;
			HAL_UART_Receive_IT(&huart1, (uint8_t *)rx_buf, 1);
		}
		
		else if((rx_buf[0] == '?') && (b_b == 1))
		{
			sprintf(tx_buf, "bb:%.1f-%.1f-%.1f-%.1f-%.1f\r\n", b_buf[4], b_buf[3], b_buf[2], b_buf[1], b_buf[0]);
			HAL_UART_Transmit(&huart1, (uint8_t *)tx_buf, strlen(tx_buf), 50);
			b_ = 0;
			b_b = 0;
			
			HAL_UART_Receive_IT(&huart1, (uint8_t *)rx_buf, 1);	
		}		
	}
	
	else if(a_!=1 && b_!=1)
		{	

			if((rx_buf[0] == 'q') && (q_ == 0))
			{
				q_ = 1;	
				HAL_UART_Receive_IT(&huart1, (uint8_t *)rx_buf, 1);
			}
					
			else if((rx_buf[0] == 'a') && (q_ == 1))
			{
				q_a = 1;	
				q_ = 0;	
				HAL_UART_Receive_IT(&huart1, (uint8_t *)rx_buf, 1);
			}
			
			else if((rx_buf[0] == 'b') && (q_ == 1))
			{
				q_b = 1;	
				q_ = 0;	
				HAL_UART_Receive_IT(&huart1, (uint8_t *)rx_buf, 1);
			}
			
			else if((rx_buf[0] == '?') && (q_a == 1))
			{
				for(i=0; i<5; i++)
				{
					a_buf_pai[i] = a_buf[i];
				}
				
				for(i=4; i>0; i--)
				{
					for(j=0; j<i; j++)
					{
						if(a_buf_pai[j+1] < a_buf_pai[j])
						{
							temp = a_buf_pai[j];
							a_buf_pai[j] = a_buf_pai[j+1];
							a_buf_pai[j+1] = temp;
						}				
					}
				}
				
				sprintf(tx_buf, "qa:%.1f-%.1f-%.1f-%.1f-%.1f\r\n", a_buf_pai[0], a_buf_pai[1], a_buf_pai[2], a_buf_pai[3], a_buf_pai[4]);
				HAL_UART_Transmit(&huart1, (uint8_t *)tx_buf, strlen(tx_buf), 50);			
				q_a = 0;	
				q_ = 0;		
				
				HAL_UART_Receive_IT(&huart1, (uint8_t *)rx_buf, 1);
			}
			
			else if((rx_buf[0] == '?') && (q_b == 1))
			{
				for(i=0; i<5; i++)
				{
					b_buf_pai[i] = b_buf[i];
				}
				
				for(i=4; i>0; i--)
				{
					for(j=0; j<i; j++)
					{
						if(b_buf_pai[j+1] < b_buf_pai[j])
						{
							temp = b_buf_pai[j];
							b_buf_pai[j] = b_buf_pai[j+1];
							b_buf_pai[j+1] = temp;
						}				
					}
				}
				sprintf(tx_buf, "qa:%.1f-%.1f-%.1f-%.1f-%.1f\r\n", b_buf_pai[0], b_buf_pai[1], b_buf_pai[2], b_buf_pai[3], b_buf_pai[4]);
				HAL_UART_Transmit(&huart1, (uint8_t *)tx_buf, strlen(tx_buf), 50);			
				q_b = 0;	
				q_ = 0;			

				HAL_UART_Receive_IT(&huart1, (uint8_t *)rx_buf, 1);				
			
			}
			
		}
	
//	else 
//	{
//		sprintf(tx_buf, "error\r\n");
//		HAL_UART_Transmit(&huart1, (uint8_t *)tx_buf, strlen(tx_buf), 50);	
//	}
	
}


void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

