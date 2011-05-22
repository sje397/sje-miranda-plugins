#ifndef _PINGLIST_H
#define _PINGLIST_H

#include "common.h"

#include <vector>
typedef std::vector<PINGADDRESS> PINGLIST;

#include <algorithm> // for sort

extern PINGLIST list_items;
extern HANDLE reload_event_handle;
extern CRITICAL_SECTION list_cs;

int LoadPingList(WPARAM wParam, LPARAM lParam);
int GetPingList(WPARAM wParam,LPARAM lParam);
int SavePingList(WPARAM wParam, LPARAM lParam);
int SetPingList(WPARAM wParam, LPARAM lParam); // use when you modified db yourself
int SetAndSavePingList(WPARAM wParam, LPARAM lParam);
int ClearPingList(WPARAM wParam, LPARAM lParam);

struct SAscendingSort
{
     bool operator()(const PINGADDRESS &rpStart, const PINGADDRESS &rpEnd)
     {
          return rpStart.index < rpEnd.index;
     }
};

// only call with list_cs locked!
void write_ping_addresses();


BOOL changing_clist_handle();
void set_changing_clist_handle(BOOL flag);

void reset_myhandle();

#endif
