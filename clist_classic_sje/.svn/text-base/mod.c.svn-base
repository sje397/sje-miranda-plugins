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

#include "commonheaders.h"
#define DCX_USESTYLE		0x10000

LRESULT (CALLBACK *oldContactListWndProc)(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) = 0;
LRESULT (CALLBACK *oldContactListControlWndProc)(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) = 0;
LRESULT (CALLBACK *oldStatusBarWndProc)(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) = 0;

void PositionFrames();
void SetCListVisible(BOOL clist_visible);

//--------------------- patch by TioDuke to implement ME_CLIST_FRAMES_SB_SHOW/HIDE_TOOLTIP events ---------------
#define TM_STATUSBAR 23435234

POINT lastpnt={0};
BOOL  tooltipshoing=FALSE;

HANDLE hStatusBarShowToolTipEvent=NULL, hStatusBarHideToolTipEvent=NULL;

#define ME_CLIST_FRAMES_SB_SHOW_TOOLTIP		"CListFrames/StatusBarShowToolTip"
#define ME_CLIST_FRAMES_SB_HIDE_TOOLTIP		"CListFrames/StatusBarHideToolTip"
//---------------------------------------------------------------------------------------------------------------

LRESULT CALLBACK ContactListControlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {
	case WM_NCHITTEST:
		{
			LRESULT lr = SendMessage(GetParent(hwnd), WM_NCHITTEST, wParam, lParam);
			if(lr == HTLEFT || lr == HTRIGHT || lr == HTBOTTOM || lr == HTTOP || lr == HTTOPLEFT || lr == HTTOPRIGHT
				|| lr == HTBOTTOMLEFT || lr == HTBOTTOMRIGHT)
				return HTTRANSPARENT;
			break;
		}
	}

	return oldContactListControlWndProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK StatusBarWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {
	case WM_CREATE:
		tooltipshoing = FALSE;
		break;
	case WM_TIMER:
		if (wParam == TM_STATUSBAR) {
			POINT pt;

			KillTimer(hwnd, TM_STATUSBAR);
			GetCursorPos(&pt);
			if (pt.x == lastpnt.x && pt.y == lastpnt.y) {
				int i, nParts;
				RECT rc;
				char *szProto;

				ScreenToClient(hwnd, &pt);
				nParts = SendMessage(hwnd, SB_GETPARTS, 0, 0);
				for (i=0; i < nParts; i++) {
					SendMessage(hwnd, SB_GETRECT, i, (LPARAM)&rc);
					if (PtInRect(&rc, pt)) {
						szProto = (char *)SendMessage(hwnd, SB_GETTEXT, i, 0);
						if (!szProto)
							return CallWindowProc(oldStatusBarWndProc, hwnd, msg, wParam, lParam);
						NotifyEventHooks(hStatusBarShowToolTipEvent, (WPARAM)szProto, 0);
						tooltipshoing = TRUE;
					}
				}
			}
		}
		break;
	case WM_SETCURSOR:
		{
			POINT pt;

			GetCursorPos(&pt);
			if (pt.x == lastpnt.x && pt.y == lastpnt.y)
				return CallWindowProc(oldStatusBarWndProc, hwnd, msg, wParam, lParam);
			lastpnt = pt;
			if (tooltipshoing) {
				NotifyEventHooks(hStatusBarHideToolTipEvent, 0, 0);
				tooltipshoing = FALSE;
			}
			KillTimer(hwnd, TM_STATUSBAR);
			SetTimer(hwnd, TM_STATUSBAR, DBGetContactSettingWord(NULL, "CLC","InfoTipHoverTime", 750), 0);
		}
		break;
	case WM_ERASEBKGND:
		if(DBGetContactSettingByte(0, "CLUI", "SBarCLCBkGnd", 0)) {
			HDC hdc = (HDC)wParam;
			RECT r;
			HBRUSH hBrush = CreateSolidBrush((COLORREF)DBGetContactSettingDword(0, "CLC", "BkColour", COLOR_3DSHADOW));
			GetClientRect(hwnd, &r);
			FillRect(hdc, &r, hBrush);
			DeleteObject(hBrush);
			return 1;
		}
		break;
	case WM_NCHITTEST:
		{
			LRESULT lr = SendMessage(GetParent(hwnd), WM_NCHITTEST, wParam, lParam);
			if(lr == HTLEFT || lr == HTRIGHT || lr == HTBOTTOM || lr == HTTOP || lr == HTTOPLEFT || lr == HTTOPRIGHT
				|| lr == HTBOTTOMLEFT || lr == HTBOTTOMRIGHT)
				return HTTRANSPARENT;
			break;
		}
	}

	return CallWindowProc(oldStatusBarWndProc, hwnd, msg, wParam, lParam);
}


#define M_PAINTFRAME  (WM_USER+0x08)
LRESULT CALLBACK ContactListWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {
	case WM_CREATE:
		{
			HRESULT hr = oldContactListWndProc(hwnd, msg, wParam, lParam);
			oldStatusBarWndProc = (WNDPROC)SetWindowLong(pcli->hwndStatus, GWL_WNDPROC, (LONG)StatusBarWndProc);
			return hr;
		}
	//case WM_CREATE:
	case (WM_USER + 1): //M_CREATECLC:
		{
			LRESULT ret = oldContactListWndProc(hwnd, msg, wParam, lParam);
			if(DBGetContactSettingByte(0, "CList", "NoBorder", 0) && !DBGetContactSettingByte(NULL, "CLUI", "ShowCaption", SETTING_SHOWCAPTION_DEFAULT)) {
				//SetWindowLong(pcli->hwndContactList, GWL_EXSTYLE, GetWindowLong(pcli->hwndContactList, GWL_EXSTYLE) | WS_EX_TOOLWINDOW);
				SetWindowLong(pcli->hwndContactList, GWL_STYLE,
					GetWindowLong(pcli->hwndContactList, GWL_STYLE) & ~(WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_POPUPWINDOW | WS_THICKFRAME));
				SetWindowPos(pcli->hwndContactList, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
				//RedrawWindow(pcli->hwndContactList, NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
			}
			return ret;
		}
	case WM_NCHITTEST:
		{
			if(DBGetContactSettingByte(0, "CList", "NoBorder", 0) && !DBGetContactSettingByte(NULL, "CLUI", "ShowCaption", SETTING_SHOWCAPTION_DEFAULT)) {
				POINT pt;
				RECT r;
				HRESULT hr;
				GetClientRect(hwnd, &r);
				pt.x = LOWORD(lParam);
				pt.y = HIWORD(lParam);
				ScreenToClient(hwnd, &pt);

				// allow for HTSIZE
				hr = DefWindowProc(hwnd, WM_NCHITTEST, wParam, lParam);	
				if(hr == HTCLIENT) {
					if(pt.x > GetSystemMetrics(SM_CXFRAME) && pt.x < r.right - GetSystemMetrics(SM_CXFRAME)) {
						if(pt.y > GetSystemMetrics(SM_CYFRAME) && pt.y < r.bottom - GetSystemMetrics(SM_CYFRAME))
							hr = HTCLIENT;
						else if(pt.y < GetSystemMetrics(SM_CYFRAME))
							hr = HTTOP;
						else if(pt.y > r.bottom - GetSystemMetrics(SM_CYFRAME))
							hr = HTBOTTOM;
					} else if(pt.x < GetSystemMetrics(SM_CXFRAME)) {
						if(pt.y > GetSystemMetrics(SM_CYFRAME) && pt.y < r.bottom - GetSystemMetrics(SM_CYFRAME))
							hr = HTLEFT;
						else if(pt.y < GetSystemMetrics(SM_CYFRAME))
							hr = HTTOPLEFT;
						else if(pt.y > r.bottom - GetSystemMetrics(SM_CYFRAME))
							hr = HTBOTTOMLEFT;
					} else if(pt.x > r.right - GetSystemMetrics(SM_CXFRAME)) {
						if(pt.y > 5 && pt.y < r.bottom - GetSystemMetrics(SM_CYFRAME))
							hr = HTRIGHT;
						else if(pt.y < GetSystemMetrics(SM_CYFRAME))
							hr = HTTOPRIGHT;
						else if(pt.y > r.bottom - GetSystemMetrics(SM_CYFRAME))
							hr = HTBOTTOMRIGHT;
					}			
				}
				
				if (hr == HTSIZE || hr == HTTOP || hr == HTTOPLEFT || hr == HTTOPRIGHT ||
					hr == HTBOTTOM || hr == HTBOTTOMRIGHT || hr == HTBOTTOMLEFT)
					if (DBGetContactSettingByte(NULL, "CLUI", "AutoSize", 0))
						hr = HTCLIENT;

				return hr;
			}
			
		}
		break;
	case WM_CONTEXTMENU:
		{
			RECT rc;
			POINT pt;
			CLIST_INTERFACE cli = *( CLIST_INTERFACE* )CallService(MS_CLIST_RETRIEVE_INTERFACE, 0, (LPARAM)g_hInst);


			pt.x = (short) LOWORD(lParam);
			pt.y = (short) HIWORD(lParam);
			// x/y might be -1 if it was generated by a kb click                    
			GetWindowRect(cli.hwndContactTree, &rc);
			if (pt.x == -1 && pt.y == -1) {
				// all this is done in screen-coords!
				GetCursorPos(&pt);
				// the mouse isnt near the window, so put it in the middle of the window
				if (!PtInRect(&rc, pt)) {
					pt.x = rc.left + (rc.right - rc.left) / 2;
					pt.y = rc.top + (rc.bottom - rc.top) / 2;
				}
			}
			if (PtInRect(&rc, pt)) {
				HMENU hMainMenu = LoadMenu(cli.hInst, MAKEINTRESOURCE(IDR_CONTEXT)), 
					hMenu = GetSubMenu(hMainMenu, 1);

				{
					MENUITEMINFO mi;
					ZeroMemory(&mi, sizeof(mi));
					mi.cbSize = MENUITEMINFO_V4_SIZE;
					mi.fMask = MIIM_SUBMENU | MIIM_TYPE;
					mi.fType = MFT_STRING;
					mi.hSubMenu = (HMENU) CallService(MS_CLIST_MENUGETMAIN, 0, 0);
					mi.dwTypeData = _T("&Main Menu");
					SetMenuItemInfo(hMenu, 9, TRUE, &mi);
				}
				

				CallService(MS_LANGPACK_TRANSLATEMENU, (WPARAM) hMenu, 0);
				CheckMenuItem(hMenu, POPUP_HIDEOFFLINE,
					DBGetContactSettingByte(NULL, "CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT) ? MF_CHECKED : MF_UNCHECKED);
				CheckMenuItem(hMenu, POPUP_HIDEOFFLINEROOT, SendMessage(cli.hwndContactTree, CLM_GETHIDEOFFLINEROOT, 0, 0) ? MF_CHECKED : MF_UNCHECKED);
				CheckMenuItem(hMenu, POPUP_HIDEEMPTYGROUPS,
					GetWindowLong(cli.hwndContactTree, GWL_STYLE) & CLS_HIDEEMPTYGROUPS ? MF_CHECKED : MF_UNCHECKED);
				CheckMenuItem(hMenu, POPUP_DISABLEGROUPS, GetWindowLong(cli.hwndContactTree, GWL_STYLE) & CLS_USEGROUPS ? MF_UNCHECKED : MF_CHECKED);
				TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
				RemoveMenu(hMenu, 9, MF_BYPOSITION);
				DestroyMenu(hMainMenu);
				return 0;
			}
			GetWindowRect(cli.hwndStatus, &rc);
			if (PtInRect(&rc, pt)) {
				HMENU hMenu;
				if (DBGetContactSettingByte(NULL, "CLUI", "SBarRightClk", 0))
					hMenu = (HMENU) CallService(MS_CLIST_MENUGETMAIN, 0, 0);
				else
					hMenu = (HMENU) CallService(MS_CLIST_MENUGETSTATUS, 0, 0);
				TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
				return 0;
		}	}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case POPUP_OPTIONS:
			{
				OPENOPTIONSDIALOG ood = {0};
				ood.cbSize = sizeof(ood);
				CallService(MS_OPT_OPENOPTIONS, 0, (LPARAM)&ood);
			}
			return TRUE;
		case POPUP_EXIT:
			SendMessage((HWND)CallService(MS_CLUI_GETHWND, 0, 0), WM_DESTROY, 0, 0);
			return TRUE;
		}
		break;

	/*
	case WM_MOVE:
	case WM_SIZE:
	case WM_STYLECHANGED:
		PositionFrames();
		break;
	case WM_SHOWWINDOW:
		SetCListVisible(wParam);
		PositionFrames();
		break;
	*/
	}

	//return CallWindowProc(oldContactListWndProc, hwnd, msg, wParam, lParam);
	return oldContactListWndProc(hwnd, msg, wParam, lParam);
}

void ReplaceCListWndProc(CLIST_INTERFACE *pcli) {
	oldContactListWndProc =  pcli->pfnContactListWndProc;
	pcli->pfnContactListWndProc = ContactListWndProc;

	oldContactListControlWndProc =  pcli->pfnContactListControlWndProc;
	pcli->pfnContactListControlWndProc = ContactListControlWndProc;

	hStatusBarShowToolTipEvent = CreateHookableEvent(ME_CLIST_FRAMES_SB_SHOW_TOOLTIP);
	hStatusBarHideToolTipEvent = CreateHookableEvent(ME_CLIST_FRAMES_SB_HIDE_TOOLTIP);
}
