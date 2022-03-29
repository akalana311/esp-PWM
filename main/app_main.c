
#include <stdio.h>
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"



//����LED�Ƶ�IO��
#define LED_RED_IO 		32  //��Ӧ��Ƶ�LED���̵�Ϊ15������Ϊ16
#define LED_GREEN_IO 	15  //��Ӧ��Ƶ�LED���̵�Ϊ15������Ϊ16
#define LED_BLUE_IO 	16  //��Ӧ��Ƶ�LED���̵�Ϊ15������Ϊ16

#define LEDC_MAX_DUTY         	(8191)	//2��13�η�-1(13λPWM)
#define LEDC_FADE_TIME    		(1000)	//����ʱ��(ms)

#define PWM_RED_CHANNEL   LEDC_CHANNEL_0   //������ͨ��
#define PWM_GREEN_CHANNEL   LEDC_CHANNEL_1   //�����̵�ͨ��
#define PWM_BLUE_CHANNEL   LEDC_CHANNEL_2   //��������ͨ��

unsigned char pwm_mode=0;	//PWMģ�飬���Ϊ1��ʾͨ���⺯��ʵ�ֽ��书��

//ledc���ýṹ��
ledc_channel_config_t 	g_ledc_red,g_ledc_green,g_ledc_blue;


//���尴����IO��
#define KEY_IO      0

//����1��ʾ�м����£�0��ʾû�м�����
int key_read_key1(void)
{
    if(gpio_get_level(KEY_IO)==0)//��������
    {
        //�ȴ����֣���ɵ�İ취
        while(gpio_get_level(KEY_IO)==0)
        {
            vTaskDelay(1);
        }

        return 1;
    }

    return 0;
}

//������ʼ��
void initKey()
{
    //����ѡ�������ΪIO����
    gpio_pad_select_gpio(KEY_IO);
    gpio_set_direction(KEY_IO, GPIO_MODE_INPUT);
}

void PWM_init(void)
{
	//��ʱ�����ýṹ��
	ledc_timer_config_t 	ledc_timer;
	//��ʱ������->timer0
	ledc_timer.duty_resolution = LEDC_TIMER_13_BIT; //PWM�ֱ���
	ledc_timer.freq_hz = 5000;                      //Ƶ��
	ledc_timer.speed_mode = LEDC_HIGH_SPEED_MODE;  	//�ٶ�
	ledc_timer.timer_num = LEDC_TIMER_0;           	// ѡ��ʱ��
	ledc_timer_config(&ledc_timer);					//���ö�ʱ��PWMģʽ

	//PWMͨ��0����->IO32->��ɫ��
	g_ledc_red.channel    = PWM_RED_CHANNEL;		//PWMͨ��
	g_ledc_red.duty       = LEDC_MAX_DUTY;						//ռ�ձ�
	g_ledc_red.gpio_num   = LED_RED_IO;				//IOӳ��
	g_ledc_red.speed_mode = LEDC_HIGH_SPEED_MODE;	//�ٶ�
	g_ledc_red.timer_sel  = LEDC_TIMER_0;			//ѡ��ʱ��
	ledc_channel_config(&g_ledc_red);				//����PWM


	//PWMͨ��0����->IO15->��ɫ��
	g_ledc_green.channel    = PWM_GREEN_CHANNEL;		//PWMͨ��
	g_ledc_green.duty       = LEDC_MAX_DUTY;			//ռ�ձ�
	g_ledc_green.gpio_num   = LED_GREEN_IO;				//IOӳ��
	g_ledc_green.speed_mode = LEDC_HIGH_SPEED_MODE;	//�ٶ�
	g_ledc_green.timer_sel  = LEDC_TIMER_0;			//ѡ��ʱ��
	ledc_channel_config(&g_ledc_green);				//����PWM

	//PWMͨ��0����->IO32->��ɫ��
	g_ledc_blue.channel    = PWM_BLUE_CHANNEL;		//PWMͨ��
	g_ledc_blue.duty       = LEDC_MAX_DUTY;			//ռ�ձ�
	g_ledc_blue.gpio_num   = LED_BLUE_IO;			//IOӳ��
	g_ledc_blue.speed_mode = LEDC_HIGH_SPEED_MODE;	//�ٶ�
	g_ledc_blue.timer_sel  = LEDC_TIMER_0;			//ѡ��ʱ��
	ledc_channel_config(&g_ledc_blue);				//����PWM

	//PWMģʽΪ1��ʱ��ʹ��ledc���书��
	if(pwm_mode==1)
	{
		ledc_fade_func_install(0);
	}
}

//���ú�Ƶ�PWM����
//����levelȡֵ0~255
void CtrRBG_R(unsigned char level)
{
	int duty=0;

	if(level==255)
	{
		duty=LEDC_MAX_DUTY;
	}
	else if(level==0)
	{
		duty=0;
	}
	else
	{
		//����ռ�ձ�
		duty=(level*LEDC_MAX_DUTY)/255;
	}


	ledc_set_duty(LEDC_HIGH_SPEED_MODE, PWM_RED_CHANNEL, duty);//�޸�ռ�ձ�
	ledc_update_duty(LEDC_HIGH_SPEED_MODE, PWM_RED_CHANNEL);//�µ�ռ�ձ���Ч
}

//�����̵Ƶ�PWM����
//����levelȡֵ0~255
void CtrRBG_G(unsigned char level)
{
	int duty=0;

	if(level==255)
	{
		duty=LEDC_MAX_DUTY;
	}
	else if(level==0)
	{
		duty=0;
	}
	else
	{
		//����ռ�ձ�
		duty=(level*LEDC_MAX_DUTY)/255;
	}

	ledc_set_duty(LEDC_HIGH_SPEED_MODE, PWM_GREEN_CHANNEL, duty);//�޸�ռ�ձ�
	ledc_update_duty(LEDC_HIGH_SPEED_MODE, PWM_GREEN_CHANNEL);//�µ�ռ�ձ���Ч
}

//�������Ƶ�PWM����
//����levelȡֵ0~255
void CtrRBG_B(unsigned char level)
{
	int duty=0;

	if(level==255)
	{
		duty=LEDC_MAX_DUTY;
	}
	else if(level==0)
	{
		duty=0;
	}
	else
	{
		//����ռ�ձ�
		duty=(level*LEDC_MAX_DUTY)/255;
	}

	ledc_set_duty(LEDC_HIGH_SPEED_MODE, PWM_BLUE_CHANNEL, duty);//�޸�ռ�ձ�
	ledc_update_duty(LEDC_HIGH_SPEED_MODE, PWM_BLUE_CHANNEL);//�µ�ռ�ձ���Ч
}


//ͨ�����书����ʾPWM����
void task_pwm1(void *pvParameter)
{
  while(1)
  {
    if(1==pwm_mode)
    {    
      printf("pwm mode1.\r\n");
    	//���书����ʾPWM

      ///////////
      //���ռ�ձ�100%-->0%-->100%��ʱ��2*LEDC_FADE_TIME
      //��ƣ���-->��-->�𣬵Ĺ���
      ///////////
      
    	//���ռ�ձ�100% ������0%��ʱ��LEDC_FADE_TIME
    	ledc_set_fade_with_time(g_ledc_red.speed_mode,
                      g_ledc_red.channel, 
                      0,
                      LEDC_FADE_TIME);
    	//���俪ʼ
    	ledc_fade_start(g_ledc_red.speed_mode,
                      g_ledc_red.channel, 
                      LEDC_FADE_NO_WAIT);
      vTaskDelay(LEDC_FADE_TIME / portTICK_PERIOD_MS);


    	//���ռ�ձ�0%������100%��ʱ��LEDC_FADE_TIME
    	ledc_set_fade_with_time(g_ledc_red.speed_mode,
                      g_ledc_red.channel, 
                      LEDC_MAX_DUTY,
                      LEDC_FADE_TIME);
    	//���俪ʼ
    	ledc_fade_start(g_ledc_red.speed_mode,
                			g_ledc_red.channel, 
                			LEDC_FADE_NO_WAIT);
      vTaskDelay(LEDC_FADE_TIME / portTICK_PERIOD_MS);
      vTaskDelay(LEDC_FADE_TIME / portTICK_PERIOD_MS);

      ///////////
      //�̵�ռ�ձ�100%-->0%-->100%��ʱ��2*LEDC_FADE_TIME
      //�̵ƣ���-->��-->�𣬵Ĺ���
      //////////

    	//�̵�ռ�ձ�100%������0%��ʱ��LEDC_FADE_TIME
    	ledc_set_fade_with_time(g_ledc_green.speed_mode,
                      g_ledc_green.channel, 
                      0,
                      LEDC_FADE_TIME);
    	//���俪ʼ
    	ledc_fade_start(g_ledc_green.speed_mode,
                      g_ledc_green.channel, 
                      LEDC_FADE_NO_WAIT);
      vTaskDelay(LEDC_FADE_TIME / portTICK_PERIOD_MS);


    	//�̵�ռ�ձ�0%������100%��ʱ��LEDC_FADE_TIME
    	ledc_set_fade_with_time(g_ledc_green.speed_mode,
                      g_ledc_green.channel, 
                      LEDC_MAX_DUTY,
                      LEDC_FADE_TIME);
    	//���俪ʼ
    	ledc_fade_start(g_ledc_green.speed_mode,
                      g_ledc_green.channel, 
                      LEDC_FADE_NO_WAIT);
      vTaskDelay(LEDC_FADE_TIME / portTICK_PERIOD_MS);
      vTaskDelay(LEDC_FADE_TIME / portTICK_PERIOD_MS);



      ///////////
      //����ռ�ձ�100%-->0%-->100%��ʱ��2*LEDC_FADE_TIME
      //���ƣ���-->��-->�𣬵Ĺ���
      //////////


    	//����ռ�ձ�100%������0%��ʱ��LEDC_FADE_TIME
    	ledc_set_fade_with_time(g_ledc_blue.speed_mode,
                      g_ledc_blue.channel, 
                      0,
                      LEDC_FADE_TIME);
    	//���俪ʼ
    	ledc_fade_start(g_ledc_blue.speed_mode,
                      g_ledc_blue.channel, 
                      LEDC_FADE_NO_WAIT);
    	//��ʱLEDC_FADE_TIME����LEDC����ʱ��
      vTaskDelay(LEDC_FADE_TIME / portTICK_PERIOD_MS);
      
                      
    	//����ռ�ձ�0%������100%��ʱ��LEDC_FADE_TIME
    	ledc_set_fade_with_time(g_ledc_blue.speed_mode,
                      g_ledc_blue.channel, 
                      LEDC_MAX_DUTY,
                      LEDC_FADE_TIME);
     	//���俪ʼ
    	ledc_fade_start(g_ledc_blue.speed_mode,
                      g_ledc_blue.channel, 
                      LEDC_FADE_NO_WAIT);
      
    	//��ʱLEDC_FADE_TIME����LEDC����ʱ��
      vTaskDelay(LEDC_FADE_TIME / portTICK_PERIOD_MS);
      vTaskDelay(LEDC_FADE_TIME / portTICK_PERIOD_MS);
    	
    }
    else{
      vTaskDelay(5);//��ʱһ��	

    }
  }	
}

//�û��޸�ռ�ձȣ��ı�LED����������
void task_pwm2(void *pvParameter)
{
  int pwm_level=0;//0~255ռ�ձ�
  int pwm_index=1;//��1��2��3

  while(1)
  {
    if(0==pwm_mode)
    {
      printf("pwm mode2.\r\n");
    	if(pwm_index==1)
    	{
    		//�޸ĺ��ռ�ձ�
    		CtrRBG_R(pwm_level);
    		pwm_level+=10;//ռ�ձȵ���

    		if(pwm_level>255)
    		{
    			pwm_level=0;
    			pwm_index=2;
    			CtrRBG_R(255);
    		}
    	}
    	else if(pwm_index==2)
    	{
    		//�޸��̵�ռ�ձ�
    		CtrRBG_G(pwm_level);
    		pwm_level+=10;//ռ�ձȵ���

    		if(pwm_level>255)
    		{
    			pwm_level=0;
    			pwm_index=3;
    			CtrRBG_G(255);
    		}
    	}
    	else
    	{
    		//�޸�����ռ�ձ�
    		CtrRBG_B(pwm_level);
    		pwm_level+=10;//ռ�ձȵ���

    		if(pwm_level>255)
    		{
    			pwm_level=0;
    			pwm_index=1;
    			CtrRBG_B(255);
    		}
    	}		
    }
    vTaskDelay(5);//��ʱһ��	
  } 

}

//�û�������ڣ��൱��main����
unsigned char edp_buff[60*1024]={0};
void app_main()
{
  initKey();//������ʼ��
  PWM_init();//PWM��ʼ��

  //����������������ִ�в�ͬ��PWMģʽ
  xTaskCreate(&task_pwm1, "task_pwm1", 4096, NULL, 9, NULL);
  xTaskCreate(&task_pwm2, "task_pwm2", 4096, NULL, 9, NULL);

edp_buff[0]=1;
edp_buff[1]=1;
edp_buff[2]=1;

  while(1)
  {
    //ͨ�������ı�PWMģʽ
    if(1==key_read_key1())
    {
      pwm_mode=(pwm_mode==1)?0:1;
      PWM_init();//���³�ʼ��ledc��ʼ��
    }

    vTaskDelay(10);//��ʱһ��			
  }  
}
