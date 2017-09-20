/**
  ******************************************************************************
  * @file    MySoftTimer.h
  * @author  mgdg
  * @version V1.0.0
  * @date    2017-09-04
  * @brief   
  ******************************************************************************
 **/


#ifndef __MYSOFTTIMER_H
#define __MYSOFTTIMER_H

#include <stdio.h>
#include <stdlib.h>

typedef void (*SoftTimerCallbackFunction)(const uint32_t interval);

uint32_t SOFT_TIMER_CreateTimer(const uint32_t interval, SoftTimerCallbackFunction timproc,const uint8_t on_off);
bool SOFT_TIMER_KillTimer(uint32_t *index);
bool SOFT_TIMER_StartTimer(uint32_t index);
bool SOFT_TIMER_StopTimer(uint32_t index);
bool SOFT_TIMER_IsTimerStart(uint32_t index);

void SOFT_TIMER_InterruptHandle(void);
void SOFT_TIMER_CycleHandle(void);
#endif
