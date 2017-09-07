/**
  ******************************************************************************
  * @file    demo.c
  * @author  mgdg
  * @version V1.0.0
  * @date    2017-09-04
  * @brief   
  ******************************************************************************
 **/

/*

	软定时器测试示例程序。
	说明：调用softtimer_test_start()启动测试，调用softtimer_test_stop()停止测试。
	运行现象：每隔两秒输出4条调试信息"handle1 timer running"，每条调试信息间隔200ms。

*/

#include "MySoftTimer.h"


//定时器序号
uint32_t timertest_TimerIndex = 0;
uint32_t testhandle_TimerIndex = 0;


static void handle1(void)
{
	printf("handle1 timer running");
}
static void testhandle(void)
{
	static bool flag = true;
	
	if(flag)
	{
		if(SOFT_TIMER_StartTimer(testhandle_TimerIndex))
		{
			printf("handle1 timer start");
		}
		else
		{
			printf("handle1 timer start error");
		}
		flag = false;
	}
	else
	{
		if(SOFT_TIMER_StopTimer(testhandle_TimerIndex))
		{
			printf("handle1 timer stop");
		}
		else
		{
			printf("handle1 timer stop error");
		}
		flag = true;
	}
}
bool softtimer_test_start(void)
{
	if(timertest_TimerIndex == 0)
	{
		timertest_TimerIndex = SOFT_TIMER_CreateTimer(2000,testhandle,true);
		if(timertest_TimerIndex != 0)
		{
			if(SOFT_TIMER_StartTimer(timertest_TimerIndex))
			{
				printf("Test timer start,index :(%08X)",timertest_TimerIndex);
				
				testhandle_TimerIndex = SOFT_TIMER_CreateTimer(200,handle1,true);
				if(testhandle_TimerIndex != 0)
				{
					printf("handle1 timer create success :(%08X)",testhandle_TimerIndex);
					return true;
				}
				else
				{
					printf("handle1 timer create error");
					return false;
				}
			}
			else
			{
				printf("Test timer start error");
				return false;
			}
		}
		else
		{
			printf("Test timer create error");
			return false;
		}
	}
	else
	{
		printf("Test timer Index error");
		return false;
	}
}
void softtimer_test_stop(void)
{
	SOFT_TIMER_KillTimer(&timertest_TimerIndex);
	SOFT_TIMER_KillTimer(&testhandle_TimerIndex);
}

