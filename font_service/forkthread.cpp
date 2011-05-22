/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project, 
all portions of this codebase are copyrighted to the people 
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include "stdafx.h"

struct FORK_ARG
{
    HANDLE hEvent;
    void (__cdecl * threadcode) (void *);
    unsigned (__stdcall * threadcodeex) (void *);
    void *arg;
};

void __cdecl forkthread_r(void *fa)
{
    void (*callercode) (void *) = ((struct FORK_ARG *)fa)->threadcode;
    void *arg = ((struct FORK_ARG *)fa)->arg;
    CallService(MS_SYSTEM_THREAD_PUSH, 0, 0);
    SetEvent(((struct FORK_ARG *)fa)->hEvent);
    __try {
        callercode(arg);
    }
    __finally {
        CallService(MS_SYSTEM_THREAD_POP, 0, 0);
    }
    return;
}

unsigned long forkthread(void (__cdecl * threadcode) (void *), unsigned long stacksize, void *arg)
{
    unsigned long rc;
    struct FORK_ARG fa;
    fa.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    fa.threadcode = threadcode;
    fa.arg = arg;
    rc = _beginthread(forkthread_r, stacksize, &fa);
    if ((unsigned long) -1L != rc) {
        WaitForSingleObject(fa.hEvent, INFINITE);
    }                           //if
    CloseHandle(fa.hEvent);
    return rc;
}

unsigned int __stdcall forkthreadex_r(void *fa)
{
    unsigned (__stdcall * threadcode) (void *) = ((struct FORK_ARG *)fa)->threadcodeex;
    void *arg = ((struct FORK_ARG *)fa)->arg;
    unsigned long rc;

    CallService(MS_SYSTEM_THREAD_PUSH, 0, 0);
    SetEvent(((struct FORK_ARG *)fa)->hEvent);
    __try {
        rc = threadcode(arg);
    }
    __finally {
        CallService(MS_SYSTEM_THREAD_POP, 0, 0);
    }
    return rc;
}

unsigned long forkthreadex(void *sec, unsigned stacksize, unsigned (__stdcall * threadcode) (void *), void *arg, unsigned cf, unsigned *thraddr)
{
    unsigned long rc;
    struct FORK_ARG fa;
    fa.threadcodeex = threadcode;
    fa.arg = arg;
    fa.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    rc = _beginthreadex(sec, stacksize, forkthreadex_r, &fa, 0, thraddr);
    if (rc) {
        WaitForSingleObject(fa.hEvent, INFINITE);
    }
    CloseHandle(fa.hEvent);
    return rc;
}
