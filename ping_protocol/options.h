#ifndef _PING_OPTIONS
#define _PING_OPTIONS

#include "common.h"
#include "pinglist.h"
#include "utils.h"
#include "icmp.h"

#include "resource.h"

// wake event for ping thread
extern HANDLE hWakeEvent;

extern PingOptions options;
extern PINGADDRESS add_edit_addr;

BOOL CALLBACK DlgProcDestEdit(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

int PingOptInit(WPARAM wParam,LPARAM lParam);
void LoadOptions();
void SaveOptions();

#endif
