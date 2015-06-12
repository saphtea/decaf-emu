#include "coreinit.h"
#include "coreinit_mutex.h"

// TODO: Ensure we can use custom OSMutex data instead of original

void
OSInitMutex(WMutex *mutex)
{
   new (mutex) WMutex();
   mutex->tag = WMutex::Tag;
   mutex->name = nullptr;
   mutex->mutex = std::make_unique<std::recursive_mutex>();
}

void
OSInitMutexEx(WMutex *mutex, char *name)
{
   OSInitMutex(mutex);
   mutex->name = make_p32(name);
}

void
OSLockMutex(WMutex *mutex)
{
   assert(mutex->tag == WMutex::Tag);
   mutex->mutex->lock();
}

BOOL
OSTryLockMutex(WMutex *mutex)
{
   assert(mutex->tag == WMutex::Tag);
   return mutex->mutex->try_lock();
}

void
OSUnlockMutex(WMutex *mutex)
{
   assert(mutex->tag == WMutex::Tag);
   mutex->mutex->unlock();
}

void
OSInitCond(WCondition *cond)
{
   new (cond) WCondition();
   cond->tag = WCondition::Tag;
   cond->name = nullptr;
   cond->cvar = std::make_unique<std::condition_variable_any>();
}

void
OSInitCondEx(WCondition *cond, char *name)
{
   OSInitCond(cond);
   cond->name = make_p32(name);
}

void
OSWaitCond(WCondition *cond, WMutex *mutex)
{
   std::unique_lock<std::recursive_mutex> lock { *mutex->mutex };
   cond->cvar->wait(lock);
}

void
OSSignalCond(WCondition *cond)
{
   cond->cvar->notify_all();
}

void
CoreInit::registerMutexFunctions()
{
   RegisterSystemFunction(OSInitMutex);
   RegisterSystemFunction(OSInitMutexEx);
   RegisterSystemFunction(OSLockMutex);
   RegisterSystemFunction(OSTryLockMutex);
   RegisterSystemFunction(OSUnlockMutex);
   RegisterSystemFunction(OSInitCond);
   RegisterSystemFunction(OSInitCondEx);
   RegisterSystemFunction(OSWaitCond);
   RegisterSystemFunction(OSSignalCond);
}
