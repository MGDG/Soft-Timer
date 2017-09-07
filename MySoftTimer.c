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


static SOFT_TIMER_LINK_NODE *SoftTimerHead = NULL;			//定时器链表头指针

#define SOFT_TIMER_INTERVAL  10 							//硬件定时器中断时间 （单位 ms）

//链表节点结构体定义
typedef struct LINK_NODE
{
	uint32_t 			timeout;			// timeout时间
	uint32_t			timecount;			// 计时
	Function 			handle; 			// 函数指针
	uint8_t 			enable;				// timer使能
	uint8_t 			reinstall;			// 重复使能
	uint8_t				count_ok;			// 计数时间到
	struct LINK_NODE* 	next_node;			// 下一个节点指针
}SOFT_TIMER_LINK_NODE;


/**
  * @brief	创建链表节点
  * @param	void
  *
  * @return	新节点指针
  * @remark	
  */
static SOFT_TIMER_LINK_NODE *CreateNode(void)
{
	SOFT_TIMER_LINK_NODE *new_node = NULL;

	new_node = (SOFT_TIMER_LINK_NODE *)malloc(sizeof(SOFT_TIMER_LINK_NODE));

	if(new_node != NULL)
	{
		new_node->next_node = NULL;
	}
	return new_node;
}

/**
  * @brief	在链表末尾添加新节点并初始化节点
  * @param	**ListHead:链表头节点指针
  * @param	data:新节点数据
  *
  * @return	新节点指针
  * @remark	输入的链表头为空则创建新链表
  */
static SOFT_TIMER_LINK_NODE *AddNewNode(SOFT_TIMER_LINK_NODE **ListHead,const SOFT_TIMER_LINK_NODE data)
{
	SOFT_TIMER_LINK_NODE* p = NULL;
 	SOFT_TIMER_LINK_NODE* q = NULL;
	SOFT_TIMER_LINK_NODE *NewNode = NULL;
	bool FirstCreate = false;
	
	if(*ListHead==NULL)
	{
		*ListHead = CreateNode();
		NewNode = *ListHead;
		FirstCreate = true;
	}
	else
	{
		NewNode = CreateNode();
	}

	if(NewNode==NULL)
		return NULL;

	q = *ListHead;
 	p = (*ListHead)->next_node;
 	while(p != NULL)
 	{
 		q = p;
 		p = p->next_node;
 	}
	
	if(!FirstCreate)
		q->next_node = NewNode;
	
	memcpy(NewNode,&data,sizeof(SOFT_TIMER_LINK_NODE));

 	return NewNode;
}

/**
  * @brief	删除链表节点
  * @param	*ListHead:链表头节点指针
  * @param	*TempNode:节点指针
  *
  * @return	链表头指针
  * @remark	
  */
static SOFT_TIMER_LINK_NODE *DeleteOneNode(SOFT_TIMER_LINK_NODE *ListHead,SOFT_TIMER_LINK_NODE *TempNode)
{
	SOFT_TIMER_LINK_NODE* p = NULL;
	SOFT_TIMER_LINK_NODE* q = ListHead;
	SOFT_TIMER_LINK_NODE* t = NULL;

	if(ListHead==NULL)				//无效链表头节点
		return NULL;
	if(TempNode==NULL)				//无效指定节点
		return ListHead;

	while(q != TempNode && q != NULL)
	{
		p = q;
		q = q->next_node;
	}

	if(q != TempNode)				//链表中不存在该节点
		return ListHead;

	if(q==ListHead)					//删除的是第一个节点
	{
		t = ListHead->next_node;				
	}
	else
	{
		p->next_node = q->next_node;
		t = ListHead;
	}
	free(q);						//释放掉被删除的节点
	return t;
}


/**
  * @brief	获取节点数据
  * @param	*ListHead:链表头节点指针
  * @param	*NodeIndex:节点指针
  * @param	*NodeData:节点数据输出指针
  *
  * @return	bool
  * @remark	
  */
static bool Get_Node_Data(SOFT_TIMER_LINK_NODE *ListHead,const SOFT_TIMER_LINK_NODE *NodeIndex, SOFT_TIMER_LINK_NODE *NodeData)
{
	if(ListHead==NULL || NodeIndex==NULL)
		return false;

	while(ListHead != NodeIndex && ListHead != NULL)
	{
		ListHead = ListHead->next_node;
	}
	
	if(ListHead != NodeIndex)				//链表中不存在该节点
		return false;
	
	memcpy(NodeData,ListHead,sizeof(SOFT_TIMER_LINK_NODE));
	
	return true;
}


/**
  * @brief	设置节点数据
  * @param	*ListHead:链表头节点指针
  * @param	*NodeIndex:节点指针
  * @param	NodeData:节点数据
  *
  * @return	bool
  * @remark	
  */
static bool Set_Node_Data(SOFT_TIMER_LINK_NODE *ListHead,SOFT_TIMER_LINK_NODE *NodeIndex, const SOFT_TIMER_LINK_NODE NodeData)
{
	if(ListHead==NULL || NodeIndex==NULL)
		return false;
	
	while(ListHead != NodeIndex && ListHead != NULL)
	{
		ListHead = ListHead->next_node;
	}
	
	if(ListHead != NodeIndex)				//链表中不存在该节点
		return false;
	
	memcpy(ListHead,(SOFT_TIMER_LINK_NODE *)&NodeData,sizeof(SOFT_TIMER_LINK_NODE));
	
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
uint32_t SOFT_TIMER_CreateTimer(const uint32_t interval, Function timproc,const uint8_t on_off)
{
	SOFT_TIMER_LINK_NODE TempTimer;
	
	if(interval < SOFT_TIMER_INTERVAL)
		return NULL;

	//初始化定时器数据
	TempTimer.timeout      	= interval;
	TempTimer.timecount    	= 0;
	TempTimer.handle       	= timproc;
	TempTimer.enable       	= false;
	TempTimer.reinstall    	= on_off;
	TempTimer.count_ok     	= false;
	TempTimer.next_node     = NULL;
	
	//创建链表节点
	return (u32)AddNewNode(&SoftTimerHead,TempTimer);
}


/**
  * @brief	删除软定时器
  * @param	index:定时器序号
  *
  * @return	bool
  * @remark	
  */
bool SOFT_TIMER_KillTimer(uint32_t *index)
{
	
	if(*index != 0)
	{
		SoftTimerHead = DeleteOneNode(SoftTimerHead,(SOFT_TIMER_LINK_NODE *)(*index));
		*index = 0;
		return true;
	}
	return false;
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
	SOFT_TIMER_LINK_NODE TimerData;

	if(Get_Node_Data(SoftTimerHead,(SOFT_TIMER_LINK_NODE *)index, &TimerData))
	{
		if(false == TimerData.enable)			//定时器未启动
		{
			TimerData.enable     = true;
			TimerData.count_ok   = false;
			TimerData.timecount  = 0;

			if(!Set_Node_Data(SoftTimerHead,(SOFT_TIMER_LINK_NODE *)index,TimerData))
				return false;
		}
		return true;
	}
	else
		return false;
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
	SOFT_TIMER_LINK_NODE TimerData;

	if(Get_Node_Data(SoftTimerHead,(SOFT_TIMER_LINK_NODE *)index, &TimerData))
	{
		TimerData.count_ok   = false;
		TimerData.enable     = false;
		TimerData.timecount  = 0;
		if(Set_Node_Data(SoftTimerHead,(SOFT_TIMER_LINK_NODE *)index,TimerData))
			return true;
		else
			return false;
	}
	else
		return false;
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
	SOFT_TIMER_LINK_NODE TimerData;

	if(Get_Node_Data(SoftTimerHead,(SOFT_TIMER_LINK_NODE *)index, &TimerData))
	{
		return TimerData.enable;
	}
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
	SOFT_TIMER_LINK_NODE *p = SoftTimerHead;

	//所有定时器时间扫描
	while(p != NULL)
	{
		if( (p->enable) && !(p->count_ok) )
		{
			p->timecount+=SOFT_TIMER_INTERVAL;
			if(p->timecount>=p->timeout)
			{
				//计数达到，允许执行回调函数
				p->count_ok = true;
			}
		}
		p = p->next_node;
	}
}


/**
  * @brief	扫描定时器，执行回调函数
  * @param	void
  *
  * @return	void
  * @remark	在主循环中调用
  */
void SOFT_TIMER_CycleHandle(void)
{
	SOFT_TIMER_LINK_NODE *p = SoftTimerHead;
	
	while(p != NULL)
	{
		if( (p->enable) && (p->count_ok) )		//使能且计时时间到
		{
			//恢复定时器状态
			p->count_ok  = false;
			p->timecount = 0;
			p->enable = p->reinstall;			//重新启动或停止定时器
			
			//运行当前定时器回调函数
			if(p->handle !=NULL)
				(p->handle)();
		}	
		p = p->next_node;
	}
}


