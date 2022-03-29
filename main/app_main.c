
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



//定义LED灯的IO口
#define LED_RED_IO 		32  //对应红灯的LED，绿灯为15，蓝灯为16
#define LED_GREEN_IO 	15  //对应红灯的LED，绿灯为15，蓝灯为16
#define LED_BLUE_IO 	16  //对应红灯的LED，绿灯为15，蓝灯为16

#define LEDC_MAX_DUTY         	(8191)	//2的13次方-1(13位PWM)
#define LEDC_FADE_TIME    		(1000)	//渐变时间(ms)

#define PWM_RED_CHANNEL   LEDC_CHANNEL_0   //定义红灯通道
#define PWM_GREEN_CHANNEL   LEDC_CHANNEL_1   //定义绿灯通道
#define PWM_BLUE_CHANNEL   LEDC_CHANNEL_2   //定义蓝灯通道

unsigned char pwm_mode=0;	//PWM模块，如果为1表示通过库函数实现渐变功能

//ledc配置结构体
ledc_channel_config_t 	g_ledc_red,g_ledc_green,g_ledc_blue;


//定义按键的IO口
#define KEY_IO      0

//返回1表示有键按下，0表示没有键按下
int key_read_key1(void)
{
    if(gpio_get_level(KEY_IO)==0)//按键按下
    {
        //等待松手，最傻的办法
        while(gpio_get_level(KEY_IO)==0)
        {
            vTaskDelay(1);
        }

        return 1;
    }

    return 0;
}

//按键初始化
void initKey()
{
    //按键选择和设置为IO输入
    gpio_pad_select_gpio(KEY_IO);
    gpio_set_direction(KEY_IO, GPIO_MODE_INPUT);
}

void PWM_init(void)
{
	//定时器配置结构体
	ledc_timer_config_t 	ledc_timer;
	//定时器配置->timer0
	ledc_timer.duty_resolution = LEDC_TIMER_13_BIT; //PWM分辨率
	ledc_timer.freq_hz = 5000;                      //频率
	ledc_timer.speed_mode = LEDC_HIGH_SPEED_MODE;  	//速度
	ledc_timer.timer_num = LEDC_TIMER_0;           	// 选择定时器
	ledc_timer_config(&ledc_timer);					//设置定时器PWM模式

	//PWM通道0配置->IO32->红色灯
	g_ledc_red.channel    = PWM_RED_CHANNEL;		//PWM通道
	g_ledc_red.duty       = LEDC_MAX_DUTY;						//占空比
	g_ledc_red.gpio_num   = LED_RED_IO;				//IO映射
	g_ledc_red.speed_mode = LEDC_HIGH_SPEED_MODE;	//速度
	g_ledc_red.timer_sel  = LEDC_TIMER_0;			//选择定时器
	ledc_channel_config(&g_ledc_red);				//配置PWM


	//PWM通道0配置->IO15->绿色灯
	g_ledc_green.channel    = PWM_GREEN_CHANNEL;		//PWM通道
	g_ledc_green.duty       = LEDC_MAX_DUTY;			//占空比
	g_ledc_green.gpio_num   = LED_GREEN_IO;				//IO映射
	g_ledc_green.speed_mode = LEDC_HIGH_SPEED_MODE;	//速度
	g_ledc_green.timer_sel  = LEDC_TIMER_0;			//选择定时器
	ledc_channel_config(&g_ledc_green);				//配置PWM

	//PWM通道0配置->IO32->蓝色灯
	g_ledc_blue.channel    = PWM_BLUE_CHANNEL;		//PWM通道
	g_ledc_blue.duty       = LEDC_MAX_DUTY;			//占空比
	g_ledc_blue.gpio_num   = LED_BLUE_IO;			//IO映射
	g_ledc_blue.speed_mode = LEDC_HIGH_SPEED_MODE;	//速度
	g_ledc_blue.timer_sel  = LEDC_TIMER_0;			//选择定时器
	ledc_channel_config(&g_ledc_blue);				//配置PWM

	//PWM模式为1的时候，使能ledc渐变功能
	if(pwm_mode==1)
	{
		ledc_fade_func_install(0);
	}
}

//设置红灯的PWM级别
//输入level取值0~255
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
		//计算占空比
		duty=(level*LEDC_MAX_DUTY)/255;
	}


	ledc_set_duty(LEDC_HIGH_SPEED_MODE, PWM_RED_CHANNEL, duty);//修改占空比
	ledc_update_duty(LEDC_HIGH_SPEED_MODE, PWM_RED_CHANNEL);//新的占空比生效
}

//设置绿灯的PWM级别
//输入level取值0~255
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
		//计算占空比
		duty=(level*LEDC_MAX_DUTY)/255;
	}

	ledc_set_duty(LEDC_HIGH_SPEED_MODE, PWM_GREEN_CHANNEL, duty);//修改占空比
	ledc_update_duty(LEDC_HIGH_SPEED_MODE, PWM_GREEN_CHANNEL);//新的占空比生效
}

//设置蓝灯的PWM级别
//输入level取值0~255
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
		//计算占空比
		duty=(level*LEDC_MAX_DUTY)/255;
	}

	ledc_set_duty(LEDC_HIGH_SPEED_MODE, PWM_BLUE_CHANNEL, duty);//修改占空比
	ledc_update_duty(LEDC_HIGH_SPEED_MODE, PWM_BLUE_CHANNEL);//新的占空比生效
}


//通过渐变功能演示PWM任务
void task_pwm1(void *pvParameter)
{
  while(1)
  {
    if(1==pwm_mode)
    {    
      printf("pwm mode1.\r\n");
    	//渐变功能演示PWM

      ///////////
      //红灯占空比100%-->0%-->100%，时间2*LEDC_FADE_TIME
      //红灯：灭-->亮-->灭，的过程
      ///////////
      
    	//红灯占空比100% 渐变至0%，时间LEDC_FADE_TIME
    	ledc_set_fade_with_time(g_ledc_red.speed_mode,
                      g_ledc_red.channel, 
                      0,
                      LEDC_FADE_TIME);
    	//渐变开始
    	ledc_fade_start(g_ledc_red.speed_mode,
                      g_ledc_red.channel, 
                      LEDC_FADE_NO_WAIT);
      vTaskDelay(LEDC_FADE_TIME / portTICK_PERIOD_MS);


    	//红灯占空比0%渐变至100%，时间LEDC_FADE_TIME
    	ledc_set_fade_with_time(g_ledc_red.speed_mode,
                      g_ledc_red.channel, 
                      LEDC_MAX_DUTY,
                      LEDC_FADE_TIME);
    	//渐变开始
    	ledc_fade_start(g_ledc_red.speed_mode,
                			g_ledc_red.channel, 
                			LEDC_FADE_NO_WAIT);
      vTaskDelay(LEDC_FADE_TIME / portTICK_PERIOD_MS);
      vTaskDelay(LEDC_FADE_TIME / portTICK_PERIOD_MS);

      ///////////
      //绿灯占空比100%-->0%-->100%，时间2*LEDC_FADE_TIME
      //绿灯：灭-->亮-->灭，的过程
      //////////

    	//绿灯占空比100%渐变至0%，时间LEDC_FADE_TIME
    	ledc_set_fade_with_time(g_ledc_green.speed_mode,
                      g_ledc_green.channel, 
                      0,
                      LEDC_FADE_TIME);
    	//渐变开始
    	ledc_fade_start(g_ledc_green.speed_mode,
                      g_ledc_green.channel, 
                      LEDC_FADE_NO_WAIT);
      vTaskDelay(LEDC_FADE_TIME / portTICK_PERIOD_MS);


    	//绿灯占空比0%渐变至100%，时间LEDC_FADE_TIME
    	ledc_set_fade_with_time(g_ledc_green.speed_mode,
                      g_ledc_green.channel, 
                      LEDC_MAX_DUTY,
                      LEDC_FADE_TIME);
    	//渐变开始
    	ledc_fade_start(g_ledc_green.speed_mode,
                      g_ledc_green.channel, 
                      LEDC_FADE_NO_WAIT);
      vTaskDelay(LEDC_FADE_TIME / portTICK_PERIOD_MS);
      vTaskDelay(LEDC_FADE_TIME / portTICK_PERIOD_MS);



      ///////////
      //蓝灯占空比100%-->0%-->100%，时间2*LEDC_FADE_TIME
      //蓝灯：灭-->亮-->灭，的过程
      //////////


    	//蓝灯占空比100%渐变至0%，时间LEDC_FADE_TIME
    	ledc_set_fade_with_time(g_ledc_blue.speed_mode,
                      g_ledc_blue.channel, 
                      0,
                      LEDC_FADE_TIME);
    	//渐变开始
    	ledc_fade_start(g_ledc_blue.speed_mode,
                      g_ledc_blue.channel, 
                      LEDC_FADE_NO_WAIT);
    	//延时LEDC_FADE_TIME，给LEDC控制时间
      vTaskDelay(LEDC_FADE_TIME / portTICK_PERIOD_MS);
      
                      
    	//蓝灯占空比0%渐变至100%，时间LEDC_FADE_TIME
    	ledc_set_fade_with_time(g_ledc_blue.speed_mode,
                      g_ledc_blue.channel, 
                      LEDC_MAX_DUTY,
                      LEDC_FADE_TIME);
     	//渐变开始
    	ledc_fade_start(g_ledc_blue.speed_mode,
                      g_ledc_blue.channel, 
                      LEDC_FADE_NO_WAIT);
      
    	//延时LEDC_FADE_TIME，给LEDC控制时间
      vTaskDelay(LEDC_FADE_TIME / portTICK_PERIOD_MS);
      vTaskDelay(LEDC_FADE_TIME / portTICK_PERIOD_MS);
    	
    }
    else{
      vTaskDelay(5);//延时一下	

    }
  }	
}

//用户修改占空比，改变LED灯亮度任务
void task_pwm2(void *pvParameter)
{
  int pwm_level=0;//0~255占空比
  int pwm_index=1;//红1绿2蓝3

  while(1)
  {
    if(0==pwm_mode)
    {
      printf("pwm mode2.\r\n");
    	if(pwm_index==1)
    	{
    		//修改红灯占空比
    		CtrRBG_R(pwm_level);
    		pwm_level+=10;//占空比递增

    		if(pwm_level>255)
    		{
    			pwm_level=0;
    			pwm_index=2;
    			CtrRBG_R(255);
    		}
    	}
    	else if(pwm_index==2)
    	{
    		//修改绿灯占空比
    		CtrRBG_G(pwm_level);
    		pwm_level+=10;//占空比递增

    		if(pwm_level>255)
    		{
    			pwm_level=0;
    			pwm_index=3;
    			CtrRBG_G(255);
    		}
    	}
    	else
    	{
    		//修改蓝灯占空比
    		CtrRBG_B(pwm_level);
    		pwm_level+=10;//占空比递增

    		if(pwm_level>255)
    		{
    			pwm_level=0;
    			pwm_index=1;
    			CtrRBG_B(255);
    		}
    	}		
    }
    vTaskDelay(5);//延时一下	
  } 

}

//用户函数入口，相当于main函数
unsigned char edp_buff[60*1024]={0};
void app_main()
{
  initKey();//按键初始化
  PWM_init();//PWM初始化

  //创建两个任务，用于执行不同的PWM模式
  xTaskCreate(&task_pwm1, "task_pwm1", 4096, NULL, 9, NULL);
  xTaskCreate(&task_pwm2, "task_pwm2", 4096, NULL, 9, NULL);

edp_buff[0]=1;
edp_buff[1]=1;
edp_buff[2]=1;

  while(1)
  {
    //通过按键改变PWM模式
    if(1==key_read_key1())
    {
      pwm_mode=(pwm_mode==1)?0:1;
      PWM_init();//重新初始化ledc初始化
    }

    vTaskDelay(10);//延时一下			
  }  
}
