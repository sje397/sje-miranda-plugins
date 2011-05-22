#ifndef _PING_UTILS
#define _PING_UTILS

//#include <string>
#include <time.h>

#include "common.h"
#include "pingthread.h" // for mainthread, for popup
//#include "icmp.h"
#include "rawping.h"
#include "options.h"
#include "icmp.h"

/*
std::string trim_string(const std::string &in);
void capitalize(std::string &in);
*/
void __stdcall	ShowPopup( const char* line1, const char* line2, int flags );

int PluginPing(WPARAM wParam,LPARAM lParam);

void Lock(CRITICAL_SECTION *cs, char *lab);
void Unlock(CRITICAL_SECTION *cs);

int PingDisableAll(WPARAM wParam, LPARAM lParam);
int PingEnableAll(WPARAM wParam, LPARAM lParam);

int ToggleEnabled(WPARAM wParam, LPARAM lParam);

int ContactDblClick(WPARAM wParam, LPARAM lParam);

int EditContact(WPARAM wParam, LPARAM lParam);
// read in addresses from old pingplug
void import_ping_addresses();

extern bool use_raw_ping;

#endif
