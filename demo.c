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

//输出所有定时器序号以及对应的在激活列表中的序号
void get_timer_list(void)
{
	LIST_LINK_NODE *list = NULL;				//定时器列表
	SOFT_TIMER_LINK_NODE *actlist = NULL;			//定时器链表头指针
	uint8_t count = 1;
	
	printf("\r\nAll timer list:\r\n");
	list = AllTimerList;
	while(list != NULL)
	{
		printf("Timer(%d):\t%08X",count++,list->TimerIndex);
		
		if(FindNode(ActivTimerList,(SOFT_TIMER_LINK_NODE *)(list->TimerIndex)))
		{
			printf("\tactivated\r\n");
		}
		else
			printf("\r\n");
		
		list = list->next_node;
	}
	
	count = 1;
	printf("\r\nActivated timer list:\r\n");
	actlist = ActivTimerList;
	while(actlist != NULL)
	{
		printf("Timer(%d):\t%08X\r\n",count++,(u32)actlist);
		actlist = actlist->next_node;
	}
}

uint32_t timertest_TimerIndex = 0;
uint32_t testhandle_TimerIndex = 0;
static void handle1(const uint32_t TimeInterVal)
{
	printf("handle1 timer running");
}
static void testhandle(const uint32_t TimeInterVal)
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
		if(SOFT_TIMER_IsTimerStart(timertest_TimerIndex))
			printf("Test timer activated already");
		else
			printf("Test timer inactivated");
		return false;
	}
}
void softtimer_test_stop(void)
{
	SOFT_TIMER_KillTimer(&timertest_TimerIndex);
	SOFT_TIMER_KillTimer(&testhandle_TimerIndex);
}

