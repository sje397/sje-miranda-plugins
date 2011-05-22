#ifndef _WINDOWTIMEOUT_H
#define _WINDOWTIMEOUT_H

#include <windows.h>
#include <newpluginapi.h>
#include <stdio.h>

#include "resource.h"

#define PLUG		"WindowTimeout"

#include "../include/m_system.h"
#include "../include/m_idle.h"
#include "../include/m_skin.h"
#include "../include/m_database.h"
#include "../include/m_options.h"
#include "../include/m_langpack.h"
#include "../include/m_message.h"
#include "../include/m_protocols.h"
#include "../include/m_protosvc.h"
#include "../include/m_protomod.h"


#define TYPING_CHECK_DELAY	100 // ms

typedef struct Entry {
	HANDLE hContact;
	HWND hwnd;
	UINT timer_id;
	struct Entry *next, *prev;
	BOOL typing;
} tag_Entry;


void add_entry(HANDLE hContact);
void set_window_handle(HANDLE hContact, HWND hwnd);
void set_typing(BOOL typing);
void remove_entry(HANDLE hContact);
void reset_timer(HANDLE hContact);
struct Entry *get_entry(HANDLE hContact);

typedef struct {
	int timeout;
	BOOL monitor_recv_only;
} Options;

extern HINSTANCE hInst;
extern PLUGINLINK *pluginLink;
extern struct MM_INTERFACE mmi;

#define mir_alloc(x)	mmi.mmi_malloc(x)
#define mir_free(x)		mmi.mmi_free(x)

extern Options options;
int OptInit(WPARAM wParam, LPARAM lParam);

void load_options();

void msg(char *msg);

#endif
