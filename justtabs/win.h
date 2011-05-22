#ifndef _JT_WIN_H
#define _JT_WIN_H

#define _WIN32_IE		0x400

#include <windows.h>
#include <commctrl.h>

#include "resource.h"
#include <stdio.h>

#include <newpluginapi.h>
#include <m_system.h>
#include <m_database.h>
#include <m_options.h>
#include <m_langpack.h>
#include <m_popup.h>
#include <m_skin.h>
#include <m_clist.h>
#include <m_clui.h>
#include <m_message.h>
#include <m_utils.h>
#include <m_protocols.h>
#include <m_updater.h>
#include <m_button.h>
#include <win2k.h>
#include <m_fontservice.h>

#define MODULE		"JustTabs"

extern HWND pluginwind;
extern HINSTANCE hInst;

#define WM_PREADDCHILD			(WM_USER + 0x1FF)
#define WM_ADDCHILD				(WM_USER + 0x200)
#define WM_REMCHILD				(WM_USER + 0x201)
#define WM_SHOWCONTACTWND		(WM_USER + 0x202)
#define WM_HIGHLIGHTCONTACTWND	(WM_USER + 0x203)
#define WM_RESETTABICONS		(WM_USER + 0x204)
#define WM_FONTRELOAD			(WM_USER + 0x205)

#define STYLE_TITLE				((DS_SETFONT | DS_3DLOOK | DS_FIXEDSYS | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME) & ~WS_VISIBLE)
#define STYLE_NOTITLE			((DS_SETFONT | DS_3DLOOK | DS_FIXEDSYS | WS_THICKFRAME) & ~WS_VISIBLE)

LRESULT CALLBACK FrameWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
int CreateFrame(HWND parent, HINSTANCE hInst);

void FixWindowStyle();

extern HFONT hFontTabs;

#ifndef MIID_JUSTTABS
#define MIID_JUSTTABS	{0x66847dd, 0xd3c5, 0x4ee9, { 0x93, 0x7c, 0x95, 0x2c, 0xcf, 0x8b, 0x59, 0x2e}}
#endif

#endif
