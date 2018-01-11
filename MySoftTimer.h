/**
  ******************************************************************************
  * @file    MySoftTimer.h
  * @author  linqh
  * @version V1.0.0
  * @date    2017-09-04
  * @brief   
  ******************************************************************************
 **/


#ifndef __MYSOFTTIMER_H
#define __MYSOFTTIMER_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/**
  * @brief	软定时器句柄
  * @remark	
  */
typedef void *M_TimerHandle;

/**
  * @brief	软定时器回调函数
  * @param	size_t:被调用间隔，单位ms
  *
  * @return	void
  * @remark	
  */
typedef void (*SoftTimerCallbackFunc)(size_t);

/**
  * @brief	创建软定时器
  * @param	*timer:定时器句柄
  * @param	interval:定时器定时间隔
  * @param	func:定时器回调函数
  * @param	Reload:定时器重装载使能
  *
  * @return	bool：创建成功返回true，失败返回false
  * @remark	已创建过的定时器必须删除后才能重新创建，否则返回false
  */
bool SoftTimer_Create(M_TimerHandle *timer,size_t interval, SoftTimerCallbackFunc func,bool Reload);

/**
  * @brief	删除软定时器
  * @param	*timer:定时器句柄的指针
  *
  * @return	bool
  * @remark	
  */
bool SoftTimer_Kill(M_TimerHandle *timer);

/**
  * @brief	启动软定时器
  * @param	timer:定时器句柄
  *
  * @return	bool
  * @remark	
  */
bool SoftTimer_Start(M_TimerHandle timer);

/**
  * @brief	停止软定时器
  * @param	timer:定时器句柄
  *
  * @return	bool
  * @remark	
  */
bool SoftTimer_Stop(M_TimerHandle timer);

/**
  * @brief	软定时器是否已经启动
  * @param	timer:定时器句柄
  *
  * @return	bool
  * @remark	
  */
bool SoftTimer_IsTimerStart(const M_TimerHandle timer);

/**
  * @brief	周期调用，刷新软定时器计时时间
  * @param	Interval 调用间隔，单位ms
  *
  * @return	void
  * @remark	在硬件定时器中断中调用
  */
void SoftTimer_InterruptHandle(size_t Interval);

/**
  * @brief	扫描定时器，执行定时器回调函数
  * @param	void
  *
  * @return	void
  * @remark	在主循环中调用
  */
void SoftTimer_CycleHandle(void);


#endif
