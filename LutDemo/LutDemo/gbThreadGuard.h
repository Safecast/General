//
//  gbThreadGuard.h
//  Safecast
//
//  Created by Nicholas Dolezal on 11/8/15.
//  Copyright (c) 2015 The Momoko Ito Foundation. All rights reserved.
//

// =============
// gbThreadGuard
// =============
//
// gbThreadGuard contains structs and interfaces necessary to protect a block
// of code from certain events while preserving multithreading in others.
//
// A dispatch semaphore or mutex lock is similar, but results in that block
// being available to only a single thread.
//
// gbThreadGuard uses a single threaded access accumulator internally to reflect
// the state of the protected code.
//
// Semantics are similar to a dispatch semaphore, but with the addition of
// -Exclusive suffixed functions.  These will wait for other threads to
// signal using the normal functions.
//
// Example:
// ========
//
// // Normal multithreaded code path:
//
// void MyImageProcessingFunction(MyConverter* c, gbThreadGuard* tg)
// {
//      gbThreadGuard_Wait(tg);
//      Converter_ConvertStuff(c, etc etc);
//      gbThreadGuard_Signal(tg);
// }
//
// // Exclusive code path:
//
// void ChangeMyConverter(MyConverter* c, gbThreadGuard* tg)
// {
//      gbThreadGuard_WaitExclusive(tg);
//      Converter_Destroy(c);
//      c = NULL;
//      Converter_Create(&c, new_options);
//      gbThreadGuard_SignalExclusive(tg);
// }
//

#ifndef gbThreadGuard_h
#define gbThreadGuard_h

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <dispatch/dispatch.h>

#define GB_THREAD_GUARD_TIME_FOREVER 0xFFFFFFU

typedef struct gbThreadGuard
{
    int                  active_threads;
    uint32_t             timeout_ms;
    dispatch_semaphore_t sema_guard;
} gbThreadGuard;

// =====================
// gbThreadGuard_Create:
// =====================
//
// Creates and returns a pointer to a newly allocated gbThreadGuard, with an
// optional internal timeout.  Note this timeout is only used for waiting on
// active_threads to decrement to 0, it is not passsed on to the internal
// internal dispatch_semaphore_t which has a wait time of forever.
//
// In most cases, timeout_ms should be GB_THREAD_GUARD_TIME_FOREVER.
//
void gbThreadGuard_Create(gbThreadGuard** tg, const uint32_t timeout_ms);

// ======================
// gbThreadGuard_Destroy:
// ======================
//
// Destroys an allocated gbThreadGuard and frees all allocated resources
// associated with it, including the gbThreadGuard struct as well.
//
void gbThreadGuard_Destroy(gbThreadGuard* tg);

// ===================
// gbThreadGuard_Wait:
// ===================
//
// This should be used for the common multithreaded operation being performed
// by your code.  It internally increments active_threads in a threadsafe
// manner.
//
void gbThreadGuard_Wait(gbThreadGuard* tg);

// =====================
// gbThreadGuard_Signal:
// =====================
//
// This should be used for the common multithreaded operation being performed
// by your code.  It internally decrements active_threads in a threadsafe
// manner.
//
void gbThreadGuard_Signal(gbThreadGuard* tg);

// ============================
// gbThreadGuard_WaitExclusive:
// ============================
//
// This should be used for whatever unsafe action you are performing that would
// break the normal multithreaded operation.  It internally waits until
// active_threads is zero, then blocks all other threads via the internal
// dispatch_semaphore_t.
//
// If a timeout was specified, this may return false, indicating that
// active_threads never reached zero and it could not obtain the lock via the
// dispatch_semaphore_t.
//
bool gbThreadGuard_WaitExclusive(gbThreadGuard* tg);

// ==============================
// gbThreadGuard_SignalExclusive:
// ==============================
//
// This should be used when you are done with whatever unsafe action obtained
// a lock with gbThreadGuard_WaitExclusive, and it is either safe for the
// normal multithreaded operation to commence, or you can guarantee the normal
// multithreaded operation will on longer occur.
//
void gbThreadGuard_SignalExclusive(gbThreadGuard* tg);

#endif





