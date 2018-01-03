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

#include <stdio.h>
#include <stdlib.h>


typedef void (*SoftTimerCallbackFunc)(const uint32_t);

uint32_t SOFT_TIMER_CreateTimer(const uint32_t interval, SoftTimerCallbackFunc timproc,uint8_t on_off);
bool SOFT_TIMER_KillTimer(uint32_t *index);
bool SOFT_TIMER_StartTimer(uint32_t index);
bool SOFT_TIMER_StopTimer(uint32_t index);
bool SOFT_TIMER_IsTimerStart(uint32_t index);

void SOFT_TIMER_InterruptHandle(uint32_t Interval);
void SOFT_TIMER_CycleHandle(void);

#endif
