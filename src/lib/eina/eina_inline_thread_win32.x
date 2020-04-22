/* EINA - EFL data type library
 * Copyright (C) 2020 Carlos Ré Signor
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */
#ifndef EINA_INLINE_THREAD_WIN32_H_
#define EINA_INLINE_THREAD_WIN32_H_

#include "eina_thread_win32.h"

typedef struct _Eina_Win32_Thread_Func
{
   void *data;
   void *(*func)(void *data);
} Eina_Win32_Thread_Func;

typedef struct _Eina_Win32_Thread_Attr
{
   LPSECURITY_ATTRIBUTES lpThreadAttributes;
   SIZE_T dwStackSize;
   DWORD dwCreationFlags;
} Eina_Win32_Thread_Attr;

static inline int *
_eina_thread_join(Eina_Thread t)
{
   int ret = (int)WaitForSingleObject(t, INFINITE);

   if (ret != 0) return ret;
   return NULL;
}

DWORD WINAPI 
_eina_thread_func(void *params)
{
   return (DWORD)((Eina_Win32_Thread_Func *)params)->
      func((void *)((Eina_Win32_Thread_Func *)params)->data);
}

static inline void
_eina_thread_set_priority(Eina_Thread_Priority prio, Eina_Thread *t)
{

   int nPriority = THREAD_PRIORITY_NORMAL;

   switch (prio)
     {
      case EINA_THREAD_URGENT:
        nPriority = THREAD_PRIORITY_HIGHEST;
        break;
      case EINA_THREAD_NORMAL:
        nPriority = THREAD_PRIORITY_NORMAL;
        break;
      case EINA_THREAD_BACKGROUND:
        nPriority = THREAD_PRIORITY_BELOW_NORMAL;
        break;
      case EINA_THREAD_IDLE:
        nPriority = THREAD_PRIORITY_IDLE;
        break;
      default:
        nPriority = THREAD_PRIORITY_NORMAL;
        break;       
     }

   SetThreadPriority((HANDLE)*t, nPriority);
}

static inline Eina_Bool
_eina_thread_create(Eina_Thread *t, int affinity, void *(*func)(void *data), void *data)
{
   Eina_Bool ret;
   Eina_Win32_Thread_Attr thread_attr;

   SECURITY_ATTRIBUTES sec_attributes;

   sec_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
   sec_attributes.lpSecurityDescriptor = NULL;
   sec_attributes.bInheritHandle = EINA_TRUE;

   thread_attr.lpThreadAttributes = &sec_attributes;

   thread_attr.dwStackSize = 4000;
   thread_attr.dwCreationFlags = 0;

   LPDWORD threadID;

   Eina_Win32_Thread_Func thread_func;
   Eina_Thread_Call *c = (Eina_Thread_Call *)(data);

   thread_func.func = func;
   thread_func.data = data;

   *t = CreateThread(thread_attr.lpThreadAttributes, thread_attr.dwStackSize, &_eina_thread_func, &thread_func, thread_attr.dwCreationFlags, threadID);

   _eina_thread_set_priority(c->prio, t);

   ret = (*t != NULL) ? EINA_TRUE : EINA_FALSE;

   if (affinity >= 0 && ret)
     {
   #ifdef EINA_HAVE_WIN32_THREAD_AFFINITY
        SetThreadAffinityMask(*t, (DWORD_PTR *)&affinity);
   #endif
     }
   
   return ret;
}

static inline Eina_Bool
_eina_thread_equal(Eina_Thread t1, Eina_Thread t2)
{
   DWORD t1_thread_id = GetThreadId(t1);
   DWORD t2_thread_id = GetThreadId(t2);

   return (t1_thread_id == t2_thread_id) ? EINA_TRUE : EINA_FALSE;
}

static inline Eina_Thread
_eina_thread_self(void)
{
   return GetCurrentThread();
}

static inline Eina_Bool
_eina_thread_name_set(Eina_Thread thread, char *buf)
{
   HRESULT res = SetThreadDescription((HANDLE)thread, (PCWSTR)buf);
   return HRESULT_CODE(res);
}

static inline Eina_Bool
_eina_thread_cancel(Eina_Thread thread)
{
   LPDWORD lpExitCode;
   Eina_Bool success = GetExitCodeThread((HANDLE)thread, lpExitCode);
   ExitThread(*lpExitCode);
   return !success;
}

static inline UNIMPLEMENTED
void _eina_thread_setcanceltype(int type, int *oldtype)
{
   #warning _eina_thread_setcanceltype is not implemented.
}

static inline UNIMPLEMENTED
int _eina_thread_setcancelstate(int type, int *oldtype)
{
   #warning _eina_thread_setcancelstate is not implemented.
   return 0;
}
static inline UNIMPLEMENTED
void _eina_thread_cancel_checkpoint(void)
{
   #warning _eina_thread_cancel_checkpoint is not implemented.
}

static inline Eina_Bool
_eina_thread_cancellable_set(Eina_Bool cancellable, Eina_Bool *was_cancellable)
{
   int state = cancellable ? EINA_THREAD_CANCEL_ENABLE :
EINA_THREAD_CANCEL_DISABLE;
   if (!state)
     {
        *was_cancellable = EINA_TRUE;
        return EINA_FALSE;
     }
   else
     {
        *was_cancellable = EINA_TRUE;
        return EINA_TRUE;
     }
}
#endif