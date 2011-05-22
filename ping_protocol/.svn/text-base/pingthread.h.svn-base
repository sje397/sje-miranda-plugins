#ifndef _PINGTHREAD_H
#define _PINGTHREAD_H

#include <string>
#include <sstream>
#include "pinglist.h"
#include "utils.h"
#include "options.h"

extern HANDLE mainThread;
extern HANDLE hWakeEvent; 
extern CRITICAL_SECTION thread_finished_cs, list_changed_cs, data_list_cs;

extern PINGLIST data_list;

void start_ping_thread();
void stop_ping_thread();

int FillList(WPARAM wParam, LPARAM lParam);

#endif
