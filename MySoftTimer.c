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

#define SOFT_TIMER_INTERVAL  10 							//硬件定时器中断时间

//最大允许定时时间，超过这个值会造成该软定时器永远无法触发
static const uint32_t SOFT_TIMER_MAX_TIME = ((((uint32_t)(~0))/SOFT_TIMER_INTERVAL)*SOFT_TIMER_INTERVAL);
//已激活定时器链表节点结构体
typedef struct LINK_NODE
{
	uint32_t 			timeout;			// timeout时间
	uint32_t			timecount;			// 计时
	SoftTimerCallbackFunction	handle; 			// 回调函数指针
	uint8_t 			enable;				// timer使能
	uint8_t 			reinstall;			// 重复使能有效
	uint8_t				count_ok;			// 计数时间到
	struct LINK_NODE* 	next_node;			// 下一个节点指针
}SOFT_TIMER_LINK_NODE;
static SOFT_TIMER_LINK_NODE *ActivTimerList = NULL;			//已激活的定时器链表头指针

//定时器列表 链表节点结构体
typedef struct LIST_NODE
{
	uint32_t 			TimerIndex;			// 定时器序号
	struct LIST_NODE* 	next_node;			// 下一个节点指针
}LIST_LINK_NODE;
static LIST_LINK_NODE *AllTimerList = NULL;					//定时器列表

/**
  * @brief	定时器创建完后放入列表
  * @param	**ListHead：指向链表头指针的指针
  * @param	index：定时器序号
  *
  * @return	bool
  * @remark	链表头为空的话则创建新链表并更新链表头指针
  */
static bool TimerList_Put(LIST_LINK_NODE **ListHead,const uint32_t index)
{
	LIST_LINK_NODE* p = NULL;
 	LIST_LINK_NODE* q = NULL;
	
	if(index == NULL)				//加入的是空的定时器序号
		return false;

	if(*ListHead==NULL)				//链表为空，创建新链表
	{
		*ListHead = (LIST_LINK_NODE *)malloc(sizeof(LIST_LINK_NODE));
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
	p = (LIST_LINK_NODE *)malloc(sizeof(LIST_LINK_NODE));
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
static bool TimerList_Pop(LIST_LINK_NODE **ListHead,const uint32_t index)
{
	LIST_LINK_NODE* p = NULL;
	LIST_LINK_NODE* q = *ListHead;

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
static bool TimerList_Get(LIST_LINK_NODE *ListHead,const uint32_t index)
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
static bool LinkOneNode(SOFT_TIMER_LINK_NODE **ListHead,SOFT_TIMER_LINK_NODE *NewNode)
{
	SOFT_TIMER_LINK_NODE* p = NULL;
 	SOFT_TIMER_LINK_NODE* q = NULL;
	
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
static bool UnlinkOneNode(SOFT_TIMER_LINK_NODE **ListHead,SOFT_TIMER_LINK_NODE *TempNode)
{
	SOFT_TIMER_LINK_NODE* p = NULL;
	SOFT_TIMER_LINK_NODE* q = *ListHead;

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
static bool FindNode(SOFT_TIMER_LINK_NODE *ListHead,SOFT_TIMER_LINK_NODE *TempNode)
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
static bool DeleteOneNode(SOFT_TIMER_LINK_NODE **ListHead,SOFT_TIMER_LINK_NODE *TempNode)
{
	SOFT_TIMER_LINK_NODE* p = NULL;
	SOFT_TIMER_LINK_NODE* q = *ListHead;
	
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
  * @param	interval:定时器定时间隔
  * @param	timproc:定时器回调函数
  * @param	on_off:定时器重装载使能
  *
  * @return	定时器序号
  * @remark	定时器序号为链表节点指针，具有唯一性
  */
uint32_t SOFT_TIMER_CreateTimer(const uint32_t interval, SoftTimerCallbackFunction timproc,uint8_t on_off)
{
	SOFT_TIMER_LINK_NODE *NewNode = NULL;
	
	if(interval > SOFT_TIMER_MAX_TIME)
		return NULL;
	
	NewNode = (SOFT_TIMER_LINK_NODE *)malloc(sizeof(SOFT_TIMER_LINK_NODE));
	if(NewNode != NULL)
	{
		//初始化定时器数据
		NewNode->timeout      	= interval;
		NewNode->timecount    	= 0;
		NewNode->handle       	= timproc;
		NewNode->enable       	= false;
		NewNode->reinstall    	= on_off;
		NewNode->count_ok     	= false;
		NewNode->next_node      = NULL;
		
		//放入定时器列表
		if(TimerList_Put(&AllTimerList,(uint32_t)NewNode))
		{
			return (uint32_t)NewNode;
		}
		else
		{
			free(NewNode);
			return NULL;
		}
	}
	return NULL;
}


/**
  * @brief	删除软定时器
  * @param	*index:指向定时器序号的指针
  *
  * @return	bool
  * @remark	
  */
bool SOFT_TIMER_KillTimer(uint32_t *index)
{
	bool rt;
	
	if(!TimerList_Get(AllTimerList,*index))
		return false;
	
	rt = DeleteOneNode(&ActivTimerList,(SOFT_TIMER_LINK_NODE *)(*index));		//无论链表中是否存在该节点都free掉
	TimerList_Pop(&AllTimerList,*index);
	*index = 0;
	return rt;
}


/**
  * @brief	启动软定时器
  * @param	index:定时器序号
  *
  * @return	bool
  * @remark	
  */
bool SOFT_TIMER_StartTimer(uint32_t index)
{
	//检查是否已经加入激活链表
	if(FindNode(ActivTimerList,(SOFT_TIMER_LINK_NODE *)index))
	{
		((SOFT_TIMER_LINK_NODE *)index)->enable     = true;
		return true;
	}
	else
	{
		if(!TimerList_Get(AllTimerList,index))				//判断是否存在该定时器
			return false;
		
		//复位定时器计时数据
		((SOFT_TIMER_LINK_NODE *)index)->enable     = true;
		((SOFT_TIMER_LINK_NODE *)index)->count_ok   = false;
		((SOFT_TIMER_LINK_NODE *)index)->timecount  = 0;
		return LinkOneNode(&ActivTimerList,(SOFT_TIMER_LINK_NODE *)index);
	}
}


/**
  * @brief	停止软定时器
  * @param	index:定时器序号
  *
  * @return	bool
  * @remark	
  */
bool SOFT_TIMER_StopTimer(uint32_t index)
{
	//检查是否已经加入链表
	if(FindNode(ActivTimerList,(SOFT_TIMER_LINK_NODE *)index))
	{
		((SOFT_TIMER_LINK_NODE *)index)->enable     = false;
		((SOFT_TIMER_LINK_NODE *)index)->count_ok   = false;
		((SOFT_TIMER_LINK_NODE *)index)->timecount  = 0;
		return UnlinkOneNode(&ActivTimerList,(SOFT_TIMER_LINK_NODE *)index);
	}
	else
		return true;
}


/**
  * @brief	软定时器是否已经启动
  * @param	index:定时器序号
  *
  * @return	bool
  * @remark	
  */
bool SOFT_TIMER_IsTimerStart(uint32_t index)
{
	if(FindNode(ActivTimerList,(SOFT_TIMER_LINK_NODE *)index))
		return ((SOFT_TIMER_LINK_NODE *)index)->enable;
	else
		return false;
}


/**
  * @brief	周期调用，刷新软定时器计时时间
  * @param	void
  *
  * @return	void
  * @remark	在硬件定时器中断中调用
  */
void SOFT_TIMER_InterruptHandle(void)
{
	SOFT_TIMER_LINK_NODE *p = ActivTimerList;

	//所有定时器时间扫描
	while(p != NULL)
	{
		if( (p->enable) && !(p->count_ok) )
		{
			p->timecount+=SOFT_TIMER_INTERVAL;
			if(p->timecount>=p->timeout)
			{
				//计数达到，允许执行
				p->count_ok = true;
			}
		}
		p = p->next_node;
	}
}


/**
  * @brief	扫描定时器，执行定时器回调函数
  * @param	void
  *
  * @return	void
  * @remark	在主循环中调用
  */
void SOFT_TIMER_CycleHandle(void)
{
	SOFT_TIMER_LINK_NODE *p = ActivTimerList;
	
	while(p != NULL)
	{
		if( (p->enable) && (p->count_ok) )		//使能且计时时间到
		{
			//恢复定时器状态
			p->count_ok  = false;
			p->timecount = 0;
			p->enable = p->reinstall;			//重新启动或停止定时器
			
			//执行当前定时器回调函数
			if(p->handle !=NULL)
			{
				(p->handle)(p->timeout);
			}
		}
		p = p->next_node;
	}
}

