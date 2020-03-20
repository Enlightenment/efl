/* EINA - EFL data type library
 * Copyright (C) 2012 Cedric Bail
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>

#include "eina_config.h"
#include "eina_lock.h" /* it will include pthread.h with proper flags */

#include "eina_sched.h"
#include "eina_cpu.h"
#include "eina_thread.h"
/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"

#include "eina_debug_private.h"

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN 
#endif
# include <Windows.h>



#include <errno.h>
#include <string.h>

#if defined(EINA_HAVE_PTHREAD_AFFINITY) || defined(EINA_HAVE_PTHREAD_SETNAME)
#ifndef __linux__
#define cpu_set_t cpuset_t
#endif
#endif

typedef struct _Eina_win32_thread_func{
   void *data;
   void *(*func)(void *data);
   
}Eina_win32_thread_func;

typedef struct _Eina_win32_thread_attr{
   LPSECURITY_ATTRIBUTES   lpThreadAttributes;
   SIZE_T                  dwStackSize;
   DWORD                   dwCreationFlags;
}Eina_win32_thread_attr;


inline void *
_eina_thread_join(Eina_Thread t)
{
   
   int ret = (int)WaitForSingleObject(t,INFINITE);

   if (ret != 0) return ret;
   return NULL;
}



DWORD WINAPI _eina_thread_func(void *params)
{
   return (DWORD)  ((Eina_win32_thread_func *)params)->func( (void*)  ((Eina_win32_thread_func *)params)->data);
}


void _eina_thread_set_priority(Eina_Thread_Priority prio, Eina_Thread *t){

   int nPriority;

   switch(prio){
      case EINA_THREAD_URGENT: nPriority =THREAD_PRIORITY_HIGHEST;
      case EINA_THREAD_NORMAL: nPriority = THREAD_PRIORITY_NORMAL;
      case EINA_THREAD_BACKGROUND: nPriority = THREAD_PRIORITY_BELOW_NORMAL;
      case EINA_THREAD_IDLE: nPriority =  THREAD_PRIORITY_IDLE;
   }

   SetThreadPriority((HANDLE)*t,nPriority);
}

inline Eina_Bool
_eina_thread_create(Eina_Thread *t, int affinity, void *(*func)(void *data), void *data)
{
   Eina_Bool ret;
   Eina_win32_thread_attr thread_attr;

   SECURITY_ATTRIBUTES sec_attributes;

   sec_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
   sec_attributes.lpSecurityDescriptor = NULL;
   sec_attributes.bInheritHandle = EINA_TRUE;

   thread_attr.lpThreadAttributes = &sec_attributes;
	    
   thread_attr.dwStackSize = 4000;
   thread_attr.dwCreationFlags = 0;

   LPDWORD threadID;

   
   Eina_win32_thread_func thread_func;
   Eina_Thread_Call *c = (Eina_Thread_Call*)(data);

   thread_func.func = func;
   thread_func.data = data;

	
	
   *t =(HANDLE) CreateThread(thread_attr.lpThreadAttributes,thread_attr.dwStackSize, &_eina_thread_func,&thread_func,thread_attr.dwCreationFlags,threadID);

  
   


   _eina_thread_set_priority(c->prio,t);  

   ret = (*t != NULL) ? EINA_TRUE : EINA_FALSE;

   if(affinity >= 0 && ret){
      #ifdef EINA_HAVE_PTHREAD_AFFINITY
      cpu_set_t cpu;
      CPU_ZERO(&cpu);
      CPU_SET(affinity, &cpu);
      SetThreadAffinityMask(*t,(DWORD_PTR*)&cpu);
      #else
      SetThreadAffinityMask(*t,(DWORD_PTR*)&affinity);
	  #endif
   }

   
   return ret;
}




inline Eina_Bool
_eina_thread_equal(Eina_Thread t1, Eina_Thread t2)
{
	DWORD t1_thread_id = GetThreadId((HANDLE)t1);
	DWORD t2_thread_id = GetThreadId((HANDLE)t2);

	return (t1_thread_id == t2_thread_id) ? EINA_TRUE : EINA_FALSE;

}

inline Eina_Thread
_eina_thread_self(void)
{

   return (Eina_Thread)GetCurrentThread();

}

HRESULT _eina_thread_set_name_win32(Eina_Thread thread, char *buf){
   return SetThreadDescription((HANDLE)thread, (PCWSTR)buf);
}

Eina_Bool _eina_thread_cancel(Eina_Thread thread){
   LPDWORD lpExitCode;
   Eina_Bool success = GetExitCodeThread((HANDLE)thread, lpExitCode);

   ExitThread(*lpExitCode);
   return success;

}



