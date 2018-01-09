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


//输出所有定时器序号以及对应的在激活列表中的序号
void get_timer_list(void)
{
	SOFT_TIMER_ALL_LIST *list = NULL;				//定时器列表
	size_t count = 1;
	
	printf("\r\nAll timer list:\r\n");
	list = AllTimerList;
	while(list != NULL)
	{
		printf("Timer(%d):\t%08X",count++,list->TimerIndex);
		
		if(FindNode(ActivTimerList,(SOFT_TIMER_ACTIVE_LIST *)(list->TimerIndex)))
		{
			printf("\tactivated\r\n");
		}
		else
			printf("\r\n");
		
		list = list->next_node;
	}
#ifdef USEREADYLIST		
	count = 1;
	printf("\r\nReady timer list:\r\n");
	SOFT_TIMER_READY_LIST *Readylist = ReadyTimerList;			//定时器链表头指针
	while(Readylist != NULL)
	{
		printf("Timer(%d):\t%08X\r\n",count++,(u32)(Readylist->TimerIndex));
		Readylist = Readylist->next_node;
	}
#endif
}



static M_TimerHandle timertest_TimerIndex = NULL;
static M_TimerHandle testhandle_TimerIndex = NULL;
static void handle1(size_t TimeInterVal)
{
	printf("handle1 timer running\r\n");
}
static void testhandle(size_t TimeInterVal)
{
	static bool flag = true;
	
	if(flag)
	{
		if(SoftTimer_Start(testhandle_TimerIndex))
		{
			printf("handle1 timer start\r\n");
		}
		else
		{
			printf("handle1 timer start error\r\n");
		}
		flag = false;
	}
	else
	{
		if(SoftTimer_Stop(testhandle_TimerIndex))
		{
			printf("handle1 timer stop\r\n");
		}
		else
		{
			printf("handle1 timer stop error\r\n");
		}
		flag = true;
	}
}
bool softtimer_test_start(void)
{
	if(timertest_TimerIndex == NULL)
	{
		if(SoftTimer_Create(&timertest_TimerIndex,2000,testhandle,true))
		{
			if(SoftTimer_Start(timertest_TimerIndex))
			{
				printf("Test timer start,index :(%08X)\r\n",timertest_TimerIndex);
				
				if(SoftTimer_Create(&testhandle_TimerIndex,200,handle1,true))
				{
					printf("handle1 timer create success :(%08X)\r\n",testhandle_TimerIndex);
					return true;
				}
				else
				{
					printf("handle1 timer create error\r\n");
					return false;
				}
			}
			else
			{
				printf("Test timer start error\r\n");
				return false;
			}
		}
		else
		{
			printf("Test timer create error\r\n");
			return false;
		}
	}
	else
	{
		if(TimerList_Get(AllTimerList,timertest_TimerIndex))
		{
			if(FindNode(ActivTimerList,(SOFT_TIMER_ACTIVE_LIST *)timertest_TimerIndex))
			{
				printf("Test timer activated already\r\n");
			}
			else
			{
				printf("Test timer inactivated\r\n");
			}
		}
		else
		{
			printf("Test timer Index error: Non-existent\r\n");
		}
		return false;
	}
}
void softtimer_test_stop(void)
{
	SoftTimer_Kill(&timertest_TimerIndex);
	SoftTimer_Kill(&testhandle_TimerIndex);
}




void __定时器中断或者周期性任务__(void) 
{
	uint32_t CycleTime = 中断时间或者任务周期，单位ms;
	SOFT_TIMER_InterruptHandle(CycleTime); 
}



int main(void)
{
	while(1)
	{
		SOFT_TIMER_CycleHandle();
	}
}
