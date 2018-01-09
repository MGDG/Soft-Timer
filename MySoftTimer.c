/**
  ******************************************************************************
  * @file    MySoftTimer.c
  * @author  mgdg
  * @version V1.0.0
  * @date    2017-09-04
  * @brief   
  ******************************************************************************
 **/

#include "MySoftTimer.h"

//使用就绪链表，需要频繁申请释放内存，效率是否会降低，同时容易造成内存碎片
//可能存在一个任务未执行完，另一个不停地加入到就绪链表中，导致程序一直在就绪链表中运行，从而无法跳出执行其他程序
//#define USEREADYLIST

//定时器属性
typedef struct ACTIVE_NODE
{
	size_t 						timeout;			// timeout时间
	size_t						timecount;			// 计时
	SoftTimerCallbackFunc		handle; 			// 回调函数指针
	struct ACTIVE_NODE* 		next_node;			// 下一个节点指针
	bool 						Reload;				// 重复使能有效
	bool 						enable;				// timer使能
	bool						count_ok;			// 计数时间到
//	uint8_t						priority;			// 优先级
}SOFT_TIMER_ACTIVE_LIST;
static SOFT_TIMER_ACTIVE_LIST *ActivTimerList = NULL;	//已激活的定时器链表头指针

//定时器列表 链表节点结构体
typedef struct LIST_NODE
{
	M_TimerHandle				TimerIndex;			// 定时器序号
	struct LIST_NODE* 			next_node;			// 下一个节点指针
}SOFT_TIMER_ALL_LIST;
static SOFT_TIMER_ALL_LIST *AllTimerList = NULL;		//已创建定时器链表头指针

#ifdef USEREADYLIST	
//已经就绪的定时器链表
typedef struct READY_NODE
{
	SOFT_TIMER_ACTIVE_LIST* 	TimerIndex;			// 定时器序号
	struct READY_NODE* 			next_node;			// 下一个节点指针
}SOFT_TIMER_READY_LIST;
static SOFT_TIMER_READY_LIST *ReadyTimerList = NULL;	//已经就绪的定时器链表头指针
#endif

/**
  * @brief	定时器创建完后放入列表
  * @param	**ListHead：指向链表头指针的指针
  * @param	index：定时器序号
  *
  * @return	bool
  * @remark	链表头为空的话则创建新链表并更新链表头指针
  */
static bool TimerList_Put(SOFT_TIMER_ALL_LIST **ListHead,M_TimerHandle index)
{
	SOFT_TIMER_ALL_LIST* p = NULL;
 	SOFT_TIMER_ALL_LIST* q = NULL;
	
	if(index == NULL)				//加入的是空的定时器序号
		return false;

	if(*ListHead==NULL)				//链表为空，创建新链表
	{
		*ListHead = (SOFT_TIMER_ALL_LIST *)malloc(sizeof(SOFT_TIMER_ALL_LIST));
		if(*ListHead != NULL)
		{
			(*ListHead)->TimerIndex = index;
			(*ListHead)->next_node = NULL;
			return true;
		}
		return false;
	}

	q = *ListHead;
 	p = (*ListHead)->next_node;
 	while(p != NULL)
 	{
		if(q->TimerIndex == index)				//链表中已经存在
			return true;
 		q = p;
 		p = p->next_node;
 	}
	
	//链表中不存在，创建一个新节点
	p = (SOFT_TIMER_ALL_LIST *)malloc(sizeof(SOFT_TIMER_ALL_LIST));
	if(p != NULL)
	{
		//初始化节点数据
		p->TimerIndex = index;
		p->next_node = NULL;
		//加入链表
		q->next_node = p;			
		return true;
	}
	return false;
}


/**
  * @brief	定时器删除后从列表中删除
  * @param	**ListHead：指向链表头指针的指针
  * @param	index：定时器序号
  *
  * @return	bool
  * @remark	删除后更新链表头指针
  */
static bool TimerList_Pop(SOFT_TIMER_ALL_LIST **ListHead,M_TimerHandle index)
{
	SOFT_TIMER_ALL_LIST* p = NULL;
	SOFT_TIMER_ALL_LIST* q = *ListHead;

	//无效链表头节点、无效定时器序号
	if(ListHead==NULL || index==NULL)
		return false;

	while( (q->TimerIndex != index) && (q != NULL) )
	{
		p = q;
		q = q->next_node;
	}

	if(q->TimerIndex != index)		//链表中不存在该节点
		return false;

	if(q==*ListHead)				//删除的是第一个节点
		*ListHead = (*ListHead)->next_node;	
	else
		p->next_node = q->next_node;
	
	free(q);						//释放掉被删除的节点
	return true;
}


/**
  * @brief	查询列表中是否存在该定时器
  * @param	*ListHead：链表头指针
  * @param	index：定时器序号
  *
  * @return	bool
  * @remark	
  */
static bool TimerList_Get(SOFT_TIMER_ALL_LIST *ListHead,M_TimerHandle index)
{
	if(ListHead == NULL)			//空链表
		return false;
	
	while(ListHead != NULL)
	{
		if(ListHead->TimerIndex == index)
			return true;
		ListHead = ListHead->next_node;
	}
	return false;
}


/**
  * @brief	添加链表节点
  * @param	**ListHead：指向链表头指针的指针
  * @param	*NewNode：节点指针
  *
  * @return	bool
  * @remark	链表头为空的话则创建新链表并更新链表头指针
  */
static bool LinkOneNode(SOFT_TIMER_ACTIVE_LIST **ListHead,SOFT_TIMER_ACTIVE_LIST *NewNode)
{
	SOFT_TIMER_ACTIVE_LIST* p = NULL;
 	SOFT_TIMER_ACTIVE_LIST* q = NULL;
	
	if(NewNode == NULL)
		return false;
	
	NewNode->next_node = NULL;
	
	if(*ListHead==NULL)
	{
		*ListHead = NewNode;			//空链表，链接的节点设为头节点
		return true;
	}

	q = *ListHead;
 	p = (*ListHead)->next_node;
 	while(p != NULL)
 	{
 		q = p;
 		p = p->next_node;
 	}
	
	q->next_node = NewNode;

 	return true;
}


/**
  * @brief	移除链表节点
  * @param	**ListHead：指向链表头指针的指针
  * @param	*TempNode：节点指针
  *
  * @return	bool
  * @remark	删除后更新链表头指针
  */
static bool UnlinkOneNode(SOFT_TIMER_ACTIVE_LIST **ListHead,SOFT_TIMER_ACTIVE_LIST *TempNode)
{
	SOFT_TIMER_ACTIVE_LIST* p = NULL;
	SOFT_TIMER_ACTIVE_LIST* q = *ListHead;

	//无效链表头节点、无效指定节点
	if(ListHead==NULL || TempNode==NULL)
		return false;

	while(q != TempNode && q != NULL)
	{
		p = q;
		q = q->next_node;
	}

	if(q != TempNode)				//链表中不存在该节点
		return false;

	if(q==*ListHead)				//删除的是第一个节点
	{
		*ListHead = (*ListHead)->next_node;	
	}
	else
	{
		p->next_node = q->next_node;
	}
	return true;
}


/**
  * @brief	查找链表中是否存在该节点
  * @param	*ListHead：链表头指针
  * @param	*TempNode：节点指针
  *
  * @return	bool
  * @remark	
  */
static bool FindNode(SOFT_TIMER_ACTIVE_LIST *ListHead,SOFT_TIMER_ACTIVE_LIST *TempNode)
{
	//无效链表头节点、无效指定节点
	if(ListHead==NULL || TempNode==NULL)
		return false;
	
	while(ListHead != TempNode && ListHead != NULL)
	{
		ListHead = ListHead->next_node;
	}

	return (ListHead == TempNode);
}


/**
  * @brief	删除链表节点
  * @param	**ListHead：指向链表头指针的指针
  * @param	*TempNode：节点指针
  *
  * @return	bool
  * @remark	删除后更新链表头指针
  */
static bool DeleteOneNode(SOFT_TIMER_ACTIVE_LIST **ListHead,SOFT_TIMER_ACTIVE_LIST *TempNode)
{
	SOFT_TIMER_ACTIVE_LIST* p = NULL;
	SOFT_TIMER_ACTIVE_LIST* q = *ListHead;
	
	if(TempNode==NULL)				//无效指定节点
		return false;
	
	if(*ListHead==NULL)				//无效链表头节点
	{
		free(TempNode);				//释放掉被删除的节点
		return true;
	}
		
	while(q != TempNode && q != NULL)
	{
		p = q;
		q = q->next_node;
	}

	if(q != TempNode)				//链表中不存在该节点
	{
		free(TempNode);				//释放掉被删除的节点
		return true;
	}

	if(q==*ListHead)				//删除的是第一个节点
	{
		(*ListHead) = (*ListHead)->next_node;				
	}
	else
	{
		p->next_node = q->next_node;
	}
	free(q);						//释放掉被删除的节点
	return true;
}


/**
  * @brief	创建软定时器
  * @param	*timer:定时器句柄
  * @param	interval:定时器定时间隔
  * @param	func:定时器回调函数
  * @param	on_off:定时器重装载使能
  *
  * @return	bool：创建成功返回true，失败返回false
  * @remark	已创建过的定时器必须删除后才能重新创建，否则返回false
  */
bool SoftTimer_Create(M_TimerHandle *timer,size_t interval, SoftTimerCallbackFunc func,bool Reload)
{
	SOFT_TIMER_ACTIVE_LIST *NewNode;
	
	//判断是否已经创建过了
	if(TimerList_Get(AllTimerList,*timer))
		return false;
	
	NewNode = (SOFT_TIMER_ACTIVE_LIST *)malloc(sizeof(SOFT_TIMER_ACTIVE_LIST));
	
	if(NewNode != NULL)
	{
		//初始化定时器数据
		NewNode->timeout      	= interval;
		NewNode->timecount    	= 0;
		NewNode->handle       	= func;
		NewNode->enable       	= false;
		NewNode->Reload	    	= Reload;
		NewNode->count_ok     	= false;
		NewNode->next_node      = NULL;
		
		//放入定时器列表
		if(TimerList_Put(&AllTimerList,(M_TimerHandle)NewNode))
		{
			*timer = (M_TimerHandle)NewNode;
			return true;
		}
		else
		{
			free(NewNode);
			return false;
		}
	}
	return false;
}


/**
  * @brief	删除软定时器
  * @param	*timer:定时器句柄的指针
  *
  * @return	bool
  * @remark	
  */
bool SoftTimer_Kill(M_TimerHandle *timer)
{
	bool rt;
	
	if(!TimerList_Get(AllTimerList,*timer))
		return false;
	
	rt = DeleteOneNode(&ActivTimerList,(SOFT_TIMER_ACTIVE_LIST *)(*timer));		//无论链表中是否存在该节点都free掉
	TimerList_Pop(&AllTimerList,*timer);
	*timer = NULL;
	return rt;
}


/**
  * @brief	启动软定时器
  * @param	timer:定时器句柄
  *
  * @return	bool
  * @remark	
  */
bool SoftTimer_Start(M_TimerHandle timer)
{
	//检查是否已经加入激活链表
	if(FindNode(ActivTimerList,(SOFT_TIMER_ACTIVE_LIST *)timer))
	{
		((SOFT_TIMER_ACTIVE_LIST *)timer)->enable     = true;
		return true;
	}
	else
	{
		if(!TimerList_Get(AllTimerList,timer))				//判断是否存在该定时器
			return false;
		
		//复位定时器计时数据
		((SOFT_TIMER_ACTIVE_LIST *)timer)->enable     = true;
		((SOFT_TIMER_ACTIVE_LIST *)timer)->count_ok   = false;
		((SOFT_TIMER_ACTIVE_LIST *)timer)->timecount  = 0;
		return LinkOneNode(&ActivTimerList,(SOFT_TIMER_ACTIVE_LIST *)timer);
	}
}


/**
  * @brief	停止软定时器
  * @param	timer:定时器句柄
  *
  * @return	bool
  * @remark	
  */
bool SoftTimer_Stop(M_TimerHandle timer)
{
	//检查是否已经加入链表
	if(FindNode(ActivTimerList,(SOFT_TIMER_ACTIVE_LIST *)timer))
	{
		((SOFT_TIMER_ACTIVE_LIST *)timer)->enable     = false;
		((SOFT_TIMER_ACTIVE_LIST *)timer)->count_ok   = false;
		((SOFT_TIMER_ACTIVE_LIST *)timer)->timecount  = 0;
		return UnlinkOneNode(&ActivTimerList,(SOFT_TIMER_ACTIVE_LIST *)timer);
	}
	else
		return true;
}


/**
  * @brief	软定时器是否已经启动
  * @param	timer:定时器句柄
  *
  * @return	bool
  * @remark	
  */
bool SoftTimer_IsTimerStart(const M_TimerHandle timer)
{
	if(FindNode(ActivTimerList,(SOFT_TIMER_ACTIVE_LIST *)timer))
		return ((SOFT_TIMER_ACTIVE_LIST *)timer)->enable;
	else
		return false;
}


/**
  * @brief	周期调用，刷新软定时器计时时间
  * @param	Interval 调用间隔，单位ms
  *
  * @return	void
  * @remark	在硬件定时器中断中调用
  */
void SoftTimer_InterruptHandle(size_t Interval)
{
	SOFT_TIMER_ACTIVE_LIST *p = ActivTimerList;
#ifdef USEREADYLIST	
	//所有定时器时间扫描
	while(p != NULL)
	{
#if 1
		//这种方式将在任务执行完成后才开始重新计时，任务运行的周期为 （任务执行时间+定时时间+其他任务执行时间），定时精度比较低
		if( (p->enable) && !(p->count_ok) )
		{
			p->timecount+=Interval;
			if(p->timecount>=p->timeout)
			{
				p->timecount = 0;
				p->enable = p->Reload;				//重新启动或停止定时器
				if(p->count_ok != true)
				{
					p->count_ok = true;
					
					//创建节点
					SOFT_TIMER_READY_LIST *temp = (SOFT_TIMER_READY_LIST *)malloc(sizeof(SOFT_TIMER_READY_LIST));
					
					if(temp != NULL)
					{
						//插入到链表尾部
						temp->TimerIndex = p;
						temp->next_node = NULL;
						
						if(ReadyTimerList == NULL)
						{
							ReadyTimerList = temp;
						}
						else
						{
							SOFT_TIMER_READY_LIST *a = ReadyTimerList;
							SOFT_TIMER_READY_LIST *b = ReadyTimerList->next_node;
							
							while(b != NULL)
							{
								a = b;
								b = b->next_node;
							}
							
							a->next_node = temp;
						}
					}
					else
					{
						//节点创建失败
						p->count_ok = false;
					}
				}
			}
		}
#else
		//这种方式有BUG，如果某个任务的执行时间很长超过了它的定时周期，可能导致该任务不停地加入到就绪链表中，主循环就会一直执行就绪链表无法退出执行其他的
		if(p->enable)
		{
			p->timecount+=Interval;
			if(p->timecount>=p->timeout)
			{
				if(Interval > p->timeout)
					p->timecount = 0;					//如果调用间隔比定时时间还长，直接等0防止累加溢出
				else
					p->timecount -= p->timeout;			//算上多余的时间，这样计时精确一点
				
				p->enable = p->Reload;				//重新启动或停止定时器
				
				if(p->count_ok != true)
				{		
					p->count_ok = true;
					
					//创建节点
					SOFT_TIMER_READY_LIST *temp = (SOFT_TIMER_READY_LIST *)malloc(sizeof(SOFT_TIMER_READY_LIST));
					
					if(temp != NULL)
					{
						//直接插入到链表头
//						temp->TimerIndex = p;
//						temp->next_node = ReadyTimerList;
//						
//						ReadyTimerList = temp;
						
						
						//插入到链表尾部
						temp->TimerIndex = p;
						temp->next_node = NULL;
						
						if(ReadyTimerList == NULL)
						{
							ReadyTimerList = temp;
						}
						else
						{
							SOFT_TIMER_READY_LIST *a = ReadyTimerList;
							SOFT_TIMER_READY_LIST *b = ReadyTimerList->next_node;
							
							while(b != NULL)
							{
								a = b;
								b = b->next_node;
							}
							
							a->next_node = temp;
						}
					}
					else
					{
						//节点创建失败
						p->count_ok = false;
					}
				}
			}
		}
#endif
		p = p->next_node;
	}

#else
	//所有定时器时间扫描
	while(p != NULL)
	{
		if( (p->enable) && !(p->count_ok) )
		{
			p->timecount+=Interval;

			if(p->timecount>=p->timeout)
			{
				if(Interval > p->timeout)
					p->timecount = 0;					//如果调用间隔比定时时间还长，直接等0防止累加溢出
				else
					p->timecount -= p->timeout;			//算上多余的时间，这样计时精确一点
				
				//计数达到，允许执行
				p->count_ok = true;
				
//				p->timecount = 0;
				p->enable = p->Reload;			//重新启动或停止定时器
				
			}

		}
		p = p->next_node;
	}
#endif
}


/**
  * @brief	扫描定时器，执行定时器回调函数
  * @param	void
  *
  * @return	void
  * @remark	在主循环中调用
  */
void SoftTimer_CycleHandle(void)
{
#ifdef USEREADYLIST	
	//扫描已就绪链表，执行完后将节点从就绪链表中删除
	SOFT_TIMER_READY_LIST *temp;
	
	while(ReadyTimerList != NULL)
	{
		SoftTimerCallbackFunc handle = (ReadyTimerList->TimerIndex)->handle;
		if(handle != NULL)
			handle((ReadyTimerList->TimerIndex)->timeout);
		
		//回调函数执行完成后，清除计时完成标志位
		ReadyTimerList->TimerIndex->count_ok = false;
		
		temp = ReadyTimerList;
		
		ReadyTimerList = ReadyTimerList->next_node;
		
		free(temp);
	}

#else
	SOFT_TIMER_ACTIVE_LIST *p = ActivTimerList;
	
	while(p != NULL)
	{
		if( (p->enable) && (p->count_ok) )		//使能且计时时间到
		{
			//恢复定时器状态
			p->count_ok  = false;

			
			//执行当前定时器回调函数
			if(p->handle !=NULL)
				(p->handle)(p->timeout);
		}
		p = p->next_node;
	}
#endif
}


