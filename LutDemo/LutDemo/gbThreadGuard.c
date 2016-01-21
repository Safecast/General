//
//  gbThreadGuard.c
//  Safecast
//
//  Created by Nicholas Dolezal on 11/8/15.
//  Copyright (c) 2015 The Momoko Ito Foundation. All rights reserved.
//

#include "gbThreadGuard.h"


typedef int gbThreadGuard_InternalAccessType; enum
{
    kGB_ThreadGuard_InternalAccessType_Read              = 0,
    kGB_ThreadGuard_InternalAccessType_Increment         = 1,
    kGB_ThreadGuard_InternalAccessType_Decrement         = 2,
    kGB_ThreadGuard_InternalAccessType_ReadAndLockIfZero = 3
};

int gbThreadGuard_IncDecrementWithSemaphore(gbThreadGuard* tg,
                                            const int      access_type)
{
    int retVal = -1;
    
    dispatch_semaphore_wait(tg->sema_guard, DISPATCH_TIME_FOREVER);
    
    if (access_type == kGB_ThreadGuard_InternalAccessType_Increment)
    {
        tg->active_threads = tg->active_threads + 1;
    }//if
    else if (access_type == kGB_ThreadGuard_InternalAccessType_Decrement)
    {
        tg->active_threads = tg->active_threads - 1;
    }//else if
    
    retVal = tg->active_threads;
    
    if (access_type != kGB_ThreadGuard_InternalAccessType_ReadAndLockIfZero
        || tg->active_threads > 0)
    {
        dispatch_semaphore_signal(tg->sema_guard);
    }//if
    
    return retVal;
}//gbThreadGuard_IncDecrementWithSemaphore


bool gbThreadGuard_WaitUntilQueueDoneWithLock(gbThreadGuard* tg, const bool is_lock_requested)
{
    bool      success     = false;
    const int access_type = is_lock_requested ? kGB_ThreadGuard_InternalAccessType_ReadAndLockIfZero
                                              : kGB_ThreadGuard_InternalAccessType_Read;
    
    int polled_n = gbThreadGuard_IncDecrementWithSemaphore(tg, access_type);
    
    if (polled_n > 0)
    {
        printf("gbThreadGuard_WaitUntilQueueDoneWithLock: Waiting on queue: %d\n", (int)polled_n);
        
        uint32_t        currentSleepMS  = 0;
        const uint32_t kSleepIntervalMS = 100;                      // 100ms
        const uint32_t kSleepIntervalUS = kSleepIntervalMS * 1000;  // 100ms -> us
        
        while (polled_n > 0)
        {
            success = polled_n <= 0;
            
            if (success || (tg->timeout_ms != GB_THREAD_GUARD_TIME_FOREVER && currentSleepMS > tg->timeout_ms))
            {
                break;
            }//if
            else if (currentSleepMS > 0 && currentSleepMS % 10000 == 0)
            {
                printf("gbThreadGuard_WaitUntilQueueDoneWithLock: Waiting on queue: %d\n", polled_n);
            }//else if
            
            polled_n = gbThreadGuard_IncDecrementWithSemaphore(tg, access_type);
            
            usleep(kSleepIntervalUS);
            currentSleepMS += kSleepIntervalMS;
        }//while
        
        if (tg->timeout_ms != GB_THREAD_GUARD_TIME_FOREVER && currentSleepMS > tg->timeout_ms)
        {
            printf("gbThreadGuard_WaitUntilQueueDoneWithLock: [WARN] Timeout after %u ms while waiting on queue: %d\n", currentSleepMS, polled_n);
        }//if
    }//if
    
    return success;
}//gbThreadGuard_WaitUntilQueueDoneWithLock



void gbThreadGuard_Create(gbThreadGuard** tg, const uint32_t timeout_ms)
{
    gbThreadGuard* _tg  = malloc(sizeof(gbThreadGuard));
    _tg->active_threads = 0;
    _tg->timeout_ms     = timeout_ms;
    _tg->sema_guard     = dispatch_semaphore_create(1);
    
    *tg = _tg;
}//gbThreadGuard_Create

void gbThreadGuard_Wait(gbThreadGuard* tg)
{
    gbThreadGuard_IncDecrementWithSemaphore(tg, kGB_ThreadGuard_InternalAccessType_Increment);
}//gbThreadGuard_Wait

void gbThreadGuard_Destroy(gbThreadGuard* tg)
{
    dispatch_release(tg->sema_guard);
    free(tg);
    tg = NULL;
}//gbThreadGuard_Destroy

void gbThreadGuard_Signal(gbThreadGuard* tg)
{
    gbThreadGuard_IncDecrementWithSemaphore(tg, kGB_ThreadGuard_InternalAccessType_Decrement);
}//gbThreadGuard_Signal

bool gbThreadGuard_WaitExclusive(gbThreadGuard* tg)
{
    return gbThreadGuard_WaitUntilQueueDoneWithLock(tg, true);
}//gbThreadGuard_WaitExclusive

void gbThreadGuard_SignalExclusive(gbThreadGuard* tg)
{
    dispatch_semaphore_signal(tg->sema_guard);
}//gbThreadGuard_SignalExclusive




