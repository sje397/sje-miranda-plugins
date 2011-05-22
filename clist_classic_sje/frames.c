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
#include "m_cluiframes.h"
#include <stdio.h>
#include <m_utils.h>

#define WMU_SNAPTO		(WM_USER + 0x10)
#define SNAP_THRESH		10

// my own alignment enum
typedef enum {AL_TOP, AL_BOTTOM, AL_LEFT, AL_RIGHT} FrameAlign;

int next_frame_id = 1;
CRITICAL_SECTION cs;

DWORD id_pos_timer = 0;

BOOL clist_visible = FALSE;

HMODULE hUserDll = 0;
BOOL(WINAPI * MyFramesSetLayeredWindowAttributes) (HWND, COLORREF, BYTE, DWORD) = 0;
BOOL(WINAPI * MyAnimateWindow) (HWND hWnd, DWORD dwTime, DWORD dwFlags) = 0;

void SetCListVisible(BOOL visible) {
	clist_visible = visible;
}

int height_client_to_frame(int client_height, LONG style, LONG ex_style) {
	RECT tr;
	tr.top = tr.right = tr.left = 0;
	tr.bottom = client_height;
	AdjustWindowRectEx(&tr, style, FALSE, ex_style);
	return tr.bottom - tr.top;
}
	
int ReadFrameFlags(char *frame_name, int default_flags) {
	char setting[256];
	mir_snprintf(setting, 256, "FrameFlags_%s", frame_name);
	return DBGetContactSettingDword(0, "SimpleCLUIFrames", setting, default_flags);
}

int ReadFrameHeight(char *frame_name, int default_height) {
	char setting[256];
	mir_snprintf(setting, 256, "FrameHeight_%s", frame_name);
	return DBGetContactSettingDword(0, "SimpleCLUIFrames", setting, default_height);
}

int ReadFrameWidth(char *frame_name, int default_width) {
	char setting[256];
	mir_snprintf(setting, 256, "FrameWidth_%s", frame_name);
	return DBGetContactSettingDword(0, "SimpleCLUIFrames", setting, default_width);
}

BOOL ReadFrameFloating(char *frame_name, BOOL default_floating) {
	char setting[256];
	mir_snprintf(setting, 256, "FrameFloat_%s", frame_name);
	return DBGetContactSettingByte(0, "SimpleCLUIFrames", setting, default_floating ? 1 : 0) ? TRUE : FALSE;
}

FrameAlign ReadFrameAlign(char *frame_name, FrameAlign default_align) {
	char setting[256];
	mir_snprintf(setting, 256, "FrameAlign_%s", frame_name);
	return (FrameAlign)DBGetContactSettingByte(0, "SimpleCLUIFrames", setting, (int)default_align);
}

int ReadFrameOffX(char *frame_name, int default_off_x) {
	char setting[256];
	mir_snprintf(setting, 256, "FrameOffX_%s", frame_name);
	return DBGetContactSettingDword(0, "SimpleCLUIFrames", setting, default_off_x);
}

int ReadFrameOffY(char *frame_name, int default_off_y) {
	char setting[256];
	mir_snprintf(setting, 256, "FrameOffY_%s", frame_name);
	return DBGetContactSettingDword(0, "SimpleCLUIFrames", setting, default_off_y);
}

void StoreFrameFlags(char *frame_name, int flags) {
	char setting[256];
	mir_snprintf(setting, 256, "FrameFlags_%s", frame_name);
	DBWriteContactSettingDword(0, "SimpleCLUIFrames", setting, flags);
}

void StoreFrameHeight(char *frame_name, int height) {
	char setting[256];
	mir_snprintf(setting, 256, "FrameHeight_%s", frame_name);
	DBWriteContactSettingDword(0, "SimpleCLUIFrames", setting, height);
}

void StoreFrameWidth(char *frame_name, int width) {
	char setting[256];
	mir_snprintf(setting, 256, "FrameWidth_%s", frame_name);
	DBWriteContactSettingDword(0, "SimpleCLUIFrames", setting, width);
}

void StoreFrameFloating(char *frame_name, BOOL floating) {
	char setting[256];
	mir_snprintf(setting, 256, "FrameFloat_%s", frame_name);
	DBWriteContactSettingByte(0, "SimpleCLUIFrames", setting, floating ? 1 : 0);
}

void StoreFrameAlign(char *frame_name, FrameAlign align) {
	char setting[256];
	mir_snprintf(setting, 256, "FrameAlign_%s", frame_name);
	DBWriteContactSettingByte(0, "SimpleCLUIFrames", setting, (BYTE)align);
}

void StoreFrameOffX(char *frame_name, int off_x) {
	char setting[256];
	mir_snprintf(setting, 256, "FrameOffX_%s", frame_name);
	DBWriteContactSettingDword(0, "SimpleCLUIFrames", setting, off_x);
}

void StoreFrameOffY(char *frame_name, int off_y) {
	char setting[256];
	mir_snprintf(setting, 256, "FrameOffY_%s", frame_name);
	DBWriteContactSettingDword(0, "SimpleCLUIFrames", setting, off_y);
}

struct FrameListNode {
	CLISTFrame frame_data;
	HWND hwndParent;
	int id;
	struct FrameListNode *next;
	LONG origStyle, origExStyle;
	BOOL floating;
	FrameAlign align;
	int width, height; // for frame (frame_data->height is client - which is only used as input from API [addframe, set frame options])

	int off_x, off_y; // offsets from clist location for non-floating mode

	BOOL sizing, size_move;
};

struct FrameListNode *frame_list = 0;

struct FrameListNode *GetFrame(int id) {
	struct FrameListNode *current, *ret = 0;
	EnterCriticalSection(&cs);
	current = frame_list;
	while(current) {
		if(current->id == id) {
			ret = current;
			break;
		}
		current = current->next;
	}
	LeaveCriticalSection(&cs);
	return ret;
}

void PositionFramesNow() {
	HWND hwndClist = (HWND)CallService(MS_CLUI_GETHWND, 0, 0),
		hwndFrame;
	LONG clistStyle = GetWindowLong(hwndClist, GWL_STYLE),
		clistExStyle = GetWindowLong(hwndClist, GWL_EXSTYLE);
	
	RECT r, fr;
	int frameHeight, frameWidth;
	struct FrameListNode *current;
	BOOL hwndVis;
	GetWindowRect(hwndClist, &r);
		
	EnterCriticalSection(&cs);
	current = frame_list;
	while(current) {
		hwndFrame = current->hwndParent;
		frameHeight = current->height;
		frameWidth = current->width;
		hwndVis = IsWindowVisible(hwndFrame);
		if(clist_visible && (current->frame_data.Flags & F_VISIBLE)) {
			if(GetWindowLong(hwndFrame, GWL_STYLE) != clistStyle) {
				SetWindowLong(hwndFrame, GWL_STYLE, clistStyle);
				SetWindowPos(hwndFrame, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);
			}
			if(GetWindowLong(hwndFrame, GWL_EXSTYLE) != clistExStyle) {
				SetWindowLong(hwndFrame, GWL_EXSTYLE, clistExStyle);
				if (MyFramesSetLayeredWindowAttributes && (clistExStyle & WS_EX_LAYERED))
						MyFramesSetLayeredWindowAttributes(
							hwndFrame, RGB(0, 0, 0),
							(BYTE) DBGetContactSettingByte(NULL, "CList", "AutoAlpha", SETTING_AUTOALPHA_DEFAULT),
							LWA_ALPHA
						);
			
			}

			// convert client height to frame height for collapsed frames
			if(!(current->frame_data.Flags & F_UNCOLLAPSED))
				frameHeight = height_client_to_frame(0, clistStyle, clistExStyle);

			GetWindowRect(hwndFrame, &fr);
			if(!current->floating) {
				if(fr.left != r.left + current->off_x || fr.top != r.top + current->off_y || fr.right != r.left + current->off_x + frameWidth || fr.bottom != r.top + current->off_y + frameHeight) {
					SetWindowPos(hwndFrame, 0, r.left + current->off_x, r.top + current->off_y , frameWidth, frameHeight, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
				}
			} else {
				if(fr.bottom - fr.top != frameHeight || fr.right - fr.left != frameWidth) {
					SetWindowPos(hwndFrame, 0, 0, 0, frameWidth, frameHeight, SWP_NOZORDER | SWP_NOACTIVATE| SWP_NOMOVE | SWP_NOSENDCHANGING);
				}
			}

			if(!hwndVis) {
				//SetWindowPos(hwndFrame, 0, 0, 0, frameWidth, frameHeight, SWP_NOMOVE | SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_ASYNCWINDOWPOS);
				ShowWindow(hwndFrame, SW_SHOWNOACTIVATE);
				UpdateWindow(hwndFrame);
				//InvalidateRect(hwndFrame, 0, TRUE);
				//UpdateWindow(hwndFrame);
				//InvalidateRect(current->frame_data.hWnd, 0, TRUE);
				//UpdateWindow(current->frame_data.hWnd);
				//PostMessage(hwndFrame, WM_SIZE, 0, 0);
				RedrawWindow(hwndFrame, 0, 0, RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_ERASE);
			}
		} else
			if(IsWindowVisible(hwndFrame)) ShowWindow(hwndFrame, SW_HIDE);

		current = current->next;
	}
	LeaveCriticalSection(&cs);
}

//void PosTimerProc(HWND hwnd, UINT msg, UINT_PTR idEvent, DWORD time) {
//	PositionFramesNow();
//}

// schedule a frame position update
void PositionFrames() {
	/*
	if(id_pos_timer) {
		KillTimer(0, id_pos_timer);
		id_pos_timer = 0;
	}
	id_pos_timer = SetTimer(0, 0, 1000, (TIMERPROC)PosTimerProc);
	*/
	PositionFramesNow();
}


int AddFrame(WPARAM wParam, LPARAM lParam) {
	CLISTFrame *frame_data = (CLISTFrame *)wParam;
	HWND hwndClist = (HWND)CallService(MS_CLUI_GETHWND, 0, 0);
	
	LONG clistStyle = GetWindowLong(hwndClist, GWL_STYLE),
		clistExStyle = GetWindowLong(hwndClist, GWL_EXSTYLE),
		childStyle;

	struct FrameListNode *node;
	TCHAR title[256];
	HWND hwndParent;
	RECT r;

	if(frame_data->cbSize != sizeof(CLISTFrame))
		return -1;

	childStyle = GetWindowLong(frame_data->hWnd, GWL_STYLE);
	childStyle &= ~(WS_POPUP | WS_BORDER);
	childStyle |= (WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN);

	SetWindowLong(frame_data->hWnd, GWL_STYLE, childStyle);
	SetWindowPos(frame_data->hWnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

	frame_data->Flags = ReadFrameFlags(frame_data->name, frame_data->Flags | F_UNCOLLAPSED);
	StoreFrameFlags(frame_data->name, frame_data->Flags);

#ifdef _UNICODE
	MultiByteToWideChar(CallService(MS_LANGPACK_GETCODEPAGE, 0, 0), 0, frame_data->name, -1, title, 256);
#else
	strncpy(title, frame_data->name, 256);
#endif

	node = (struct FrameListNode *)malloc(sizeof(struct FrameListNode));
	memcpy(&node->frame_data, frame_data, sizeof(CLISTFrame));
	node->id = next_frame_id++;
	node->origStyle = GetWindowLong(node->frame_data.hWnd, GWL_STYLE);
	node->origExStyle = GetWindowLong(node->frame_data.hWnd, GWL_EXSTYLE);
	node->frame_data.name = _strdup(frame_data->name);
	node->sizing = node->size_move = FALSE;

	node->align = ReadFrameAlign(frame_data->name, AL_TOP);

	node->width = ReadFrameWidth(frame_data->name, 150);
	//node->floating = ReadFrameFloating(frame_data->name, FALSE);
	node->floating = FALSE;
	node->off_x = ReadFrameOffX(frame_data->name, 0);
	node->off_y = ReadFrameOffY(frame_data->name, 0);

	node->height = ReadFrameHeight(frame_data->name, height_client_to_frame(frame_data->height, clistStyle, clistExStyle));
	StoreFrameHeight(frame_data->name, node->height);

	GetWindowRect(hwndClist, &r);
	//hwndParent = CreateWindowEx(clistExStyle, _T("CListFrameClass"), title, clistStyle, r.left + node->off_x, r.top + node->off_y, node->width, node->height, hwndClist, NULL, g_hInst, NULL);
	hwndParent = CreateWindowEx(clistExStyle, _T("CListFrameClass"), title, clistStyle, r.left + node->off_x, r.top + node->off_y, 0, 0, hwndClist, NULL, g_hInst, NULL);
	if (MyFramesSetLayeredWindowAttributes && (clistExStyle & WS_EX_LAYERED))
		MyFramesSetLayeredWindowAttributes(hwndParent, RGB(0, 0, 0),
		(BYTE) DBGetContactSettingByte(NULL, "CList", "AutoAlpha", SETTING_AUTOALPHA_DEFAULT),
		LWA_ALPHA);

	node->hwndParent = hwndParent;
	SetParent(frame_data->hWnd, hwndParent);
	PostMessage(hwndParent, WM_CHANGEUISTATE, UIS_INITIALIZE, 0);

	SetWindowLong(hwndParent, GWL_USERDATA, (LONG)node);

	EnterCriticalSection(&cs);
	node->next = frame_list;
	frame_list = node;
	LeaveCriticalSection(&cs);

	return node->id;
}

int ShowHideFrame(WPARAM wParam, LPARAM lParam) {
	int id = (int)wParam;
	struct FrameListNode *node = GetFrame(id);
	if(node) {
		node->frame_data.Flags ^= F_VISIBLE;
		StoreFrameFlags(node->frame_data.name, node->frame_data.Flags);
		PositionFrames();
	}
	return 0;
}

int ShowAllFrames(WPARAM wParam, LPARAM lParam) {
	struct FrameListNode *current;

	EnterCriticalSection(&cs);
	current = frame_list;
	while(current) {
		current->frame_data.Flags |= F_VISIBLE;
		StoreFrameFlags(current->frame_data.name, current->frame_data.Flags);
		current = current->next;
	}
	LeaveCriticalSection(&cs);
	PositionFrames();
	return 0;
}

int UpdateFrame(WPARAM wParam, LPARAM lParam) {
	int id = (int)wParam, flags = (int)lParam;
	struct FrameListNode *node = GetFrame(id);
	if(node) {
		if(flags & FU_FMREDRAW) {
			RedrawWindow(node->hwndParent, 0, 0, RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_ERASE);
			//RedrawWindow(node->frame_data.hWnd, 0, 0, RDW_FRAME | RDW_INVALIDATE);
			//InvalidateRect(node->hwndParent, 0, TRUE);
		}
	}
	return 0;
}


int GetFrameOptions(WPARAM wParam, LPARAM lParam) {
	//int id = (int)LOWORD(wParam), flags = (int)HIWORD(wParam); // grr - header file wrong!
	int id = (int)HIWORD(wParam), flags = (int)LOWORD(wParam);
	struct FrameListNode *node = GetFrame(id);
	if(node) {
		switch(flags) {
			case FO_FLAGS:
				return node->frame_data.Flags;
			case FO_FLOATING:
				return node->floating;
			case FO_HEIGHT:
				return node->frame_data.height;
		}
	}

	return -1;
}

int SetFrameOptions(WPARAM wParam, LPARAM lParam) {
	//int id = (int)LOWORD(wParam), flags = (int)HIWORD(wParam); // grr - header file wrong!
	int id = (int)HIWORD(wParam), flag = (int)LOWORD(wParam);
	int value = (int)lParam;
	struct FrameListNode *node = GetFrame(id);
	if(node) {
		switch(flag) {
			case FO_HEIGHT:
				node->frame_data.height = value;
				{
					HWND hwndClist = (HWND)CallService(MS_CLUI_GETHWND, 0, 0);
					LONG clistStyle = GetWindowLong(hwndClist, GWL_STYLE),
						clistExStyle = GetWindowLong(hwndClist, GWL_EXSTYLE);
					node->height = height_client_to_frame(value, clistStyle, clistExStyle);
					StoreFrameHeight(node->frame_data.name, node->height);
				}
				break;
			case FO_FLAGS:
				node->frame_data.Flags = value;
				StoreFrameFlags(node->frame_data.name, node->frame_data.Flags);
				break;
			case FO_FLOATING:
				node->floating = value ? TRUE : FALSE;
				StoreFrameFloating(node->frame_data.name, node->floating);
				break;
			default:
				return -1;
		}

		PositionFrames();
	}

	return 0;
}

void MakeFrameServices() {
	CreateServiceFunction(MS_CLIST_FRAMES_ADDFRAME, AddFrame);
	CreateServiceFunction(MS_CLIST_FRAMES_SHFRAME, ShowHideFrame);
	CreateServiceFunction(MS_CLIST_FRAMES_UPDATEFRAME, UpdateFrame);
	CreateServiceFunction(MS_CLIST_FRAMES_GETFRAMEOPTIONS, GetFrameOptions);
	CreateServiceFunction(MS_CLIST_FRAMES_SETFRAMEOPTIONS, SetFrameOptions);
	CreateServiceFunction(MS_CLIST_FRAMES_SHOWALLFRAMES, ShowAllFrames);
}

void AddMainMenuItem() {
	CLISTMENUITEM menu;
	ZeroMemory(&menu,sizeof(menu));
	menu.cbSize=sizeof(menu);
	menu.flags = CMIM_ALL;
	
	menu.hIcon=LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_BLANK));

	menu.pszName = (char *)Translate("Show all frames");
	menu.pszService = MS_CLIST_FRAMES_SHOWALLFRAMES;
	menu.position = 500100000;
	CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&menu);
}

int FramesModulesLoaded(WPARAM wParam, LPARAM lParam) {
	SetCListVisible(IsWindowVisible((HWND)CallService(MS_CLUI_GETHWND, 0, 0)));
	PositionFrames();
	AddMainMenuItem();
	return 0;
}

LRESULT CALLBACK FrameWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {
		case WM_MOVE:
			//PUShowMessage("move", SM_NOTIFY);
			if(!IsWindowVisible(hwnd)) break;
			{

				struct FrameListNode *node = (struct FrameListNode *)GetWindowLong(hwnd, GWL_USERDATA);
				if(node) {
					RECT r, clr;
					HWND hwndClist = (HWND)CallService(MS_CLUI_GETHWND, 0, 0);
					GetWindowRect(hwndClist, &clr);
					GetWindowRect(hwnd, &r);
					
					node->off_x = r.left - clr.left;
					node->off_y = r.top - clr.top;
					StoreFrameOffX(node->frame_data.name, node->off_x);
					StoreFrameOffY(node->frame_data.name, node->off_y);
				}
			}
			break;
		case WM_SIZE:
			//PUShowMessage("size", SM_NOTIFY);
			//if(!IsWindowVisible(hwnd)) break;
			{
				struct FrameListNode *node = (struct FrameListNode *)GetWindowLong(hwnd, GWL_USERDATA);
				if(node) {
					HWND hwndChild = node->frame_data.hWnd;
					RECT r, cr;
					GetClientRect(hwnd, &r);
					GetWindowRect(hwndChild, &cr);
			        SetWindowPos(hwndChild, HWND_TOP, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_SHOWWINDOW);
					//SetWindowPos(hwndChild, 0, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOZORDER | SWP_NOACTIVATE);
					if(cr.left != r.left || cr.top != r.top || cr.right != r.right || cr.bottom != r.bottom) {
						cr = r;
						GetWindowRect(hwnd, &r);
						// frame_data.height is client height
						// but node width and height are frame 
						if(node->frame_data.Flags & F_UNCOLLAPSED) {
							node->frame_data.height = cr.bottom - cr.top;
							node->height = r.bottom - r.top;
							StoreFrameHeight(node->frame_data.name, node->height);
						}
						node->width = r.right - r.left;
						StoreFrameWidth(node->frame_data.name, node->width);
						PositionFrames();
					}

					if(node->size_move)
						node->sizing = TRUE;
					UpdateWindow(hwnd);
				}
			}
			break;
		case WM_ENTERSIZEMOVE:
			{
				struct FrameListNode *node = (struct FrameListNode *)GetWindowLong(hwnd, GWL_USERDATA);
				if(node) node->size_move = TRUE;
			}
			break;
		case WM_EXITSIZEMOVE:
			{
				struct FrameListNode *node = (struct FrameListNode *)GetWindowLong(hwnd, GWL_USERDATA);
				if(node) node->size_move = node->sizing = FALSE;
			}
			break;
		case WM_CLOSE:
			{
				struct FrameListNode *node = (struct FrameListNode *)GetWindowLong(hwnd, GWL_USERDATA);
				if(node) ShowHideFrame(node->id, 0);
			}
			return 0;
		case WM_NCLBUTTONDBLCLK:
			//PUShowMessage("dbl click", SM_NOTIFY);
			{
				HRESULT hr = DefWindowProc(hwnd, WM_NCHITTEST, 0, lParam);
				if(hr == HTCAPTION) {
					struct FrameListNode *node = (struct FrameListNode *)GetWindowLong(hwnd, GWL_USERDATA);
					if(node) {
						node->frame_data.Flags ^= F_UNCOLLAPSED;
						StoreFrameFlags(node->frame_data.name, node->frame_data.Flags);
						PositionFrames();
					}
				}
			}
			break;
		case WM_WINDOWPOSCHANGING:
			{
				struct FrameListNode *node = (struct FrameListNode *)GetWindowLong(hwnd, GWL_USERDATA);
				if(node) {
					WINDOWPOS *winpos = (WINDOWPOS *)lParam;
					HWND hwndClist = (HWND)CallService(MS_CLUI_GETHWND, 0, 0);
					struct FrameListNode *current;

					if(IsWindowVisible(hwndClist))
						SendMessage(hwnd, WMU_SNAPTO, (WPARAM)hwndClist, (LPARAM)winpos);

					EnterCriticalSection(&cs);
					current = frame_list;
					while(current) {
						if(IsWindowVisible(current->hwndParent))
							SendMessage(hwnd, WMU_SNAPTO, (WPARAM)current->hwndParent, (LPARAM)winpos);
						current = current->next;
					}
					LeaveCriticalSection(&cs);

					//PositionFrames();
				}
			}
			return 0;
			//break;
		case WMU_SNAPTO:
			{
				BOOL snapped = FALSE;
				WINDOWPOS *winpos = (WINDOWPOS *)lParam;
				HWND hwndOther = (HWND)wParam;
				RECT sr;
				struct FrameListNode *node = (struct FrameListNode *)GetWindowLong(hwnd, GWL_USERDATA);

				if(node && GetWindowRect(hwndOther,&sr)) {
					if((winpos->x <= (sr.right + SNAP_THRESH)) && (winpos->x >= (sr.right - SNAP_THRESH))) {  
						if ((winpos->y + winpos->cy >= sr.top) && (winpos->y <= sr.bottom)) {
							snapped = TRUE;
							if(node->sizing) winpos->cx += winpos->x - sr.right;
							winpos->x = sr.right;  
						}
					} 
					
					if((winpos->x + winpos->cx <= (sr.left + SNAP_THRESH)) && (winpos->x + winpos->cx >= (sr.left - SNAP_THRESH))) {  
						if ((winpos->y + winpos->cy >= sr.top) && (winpos->y <= sr.bottom)) {
							snapped = TRUE;
							
							if(node->sizing) winpos->cx = sr.left - winpos->x;
							else winpos->x = sr.left - winpos->cx;  
						}
					} 
					
					if((winpos->y <= (sr.bottom + SNAP_THRESH)) && (winpos->y >= (sr.bottom - SNAP_THRESH))) {  
						if ((winpos->x + winpos->cx >= sr.left) && (winpos->x <= sr.right)) {
							snapped = TRUE;
							if(node->sizing) winpos->cy += winpos->y - sr.bottom;
							winpos->y = sr.bottom;  
						}
					} 
					
					if((winpos->y + winpos->cy <= (sr.top + SNAP_THRESH)) && (winpos->y + winpos->cy >= (sr.top - SNAP_THRESH))) {  
						if ((winpos->x + winpos->cx >= sr.left) && (winpos->x <= sr.right)) {
							snapped = TRUE;
							
							if(node->sizing) winpos->cy = sr.top - winpos->y;
							else winpos->y = sr.top - winpos->cy;  
						}
					} 
					
					if((winpos->x <= (sr.left + SNAP_THRESH)) && (winpos->x >= (sr.left - SNAP_THRESH))) {  
						if ((winpos->y + winpos->cy == sr.top) || (winpos->y == sr.bottom)) {
							snapped = TRUE;
							if(node->sizing) winpos->cx += winpos->x - sr.left;
							winpos->x = sr.left;  
						}
					} 
					
					if((winpos->x + winpos->cx <= (sr.right + SNAP_THRESH)) && (winpos->x + winpos->cx >= (sr.right - SNAP_THRESH))) {  
						if ((winpos->y + winpos->cy == sr.top) || (winpos->y == sr.bottom)) {
							snapped = TRUE;
							if(node->sizing) winpos->cx = sr.right - winpos->x;
							else winpos->x = sr.right - winpos->cx;  
						}
					} 
					
					if((winpos->y <= (sr.top + SNAP_THRESH)) && (winpos->y >= (sr.top - SNAP_THRESH))) {  
						if ((winpos->x + winpos->cx == sr.left) || (winpos->x == sr.right)) {
							snapped = TRUE;
							if(node->sizing) winpos->cy += winpos->y - sr.top;
							winpos->y = sr.top;  
						}
					} 
					
					if((winpos->y + winpos->cy <= (sr.bottom + SNAP_THRESH)) && (winpos->y + winpos->cy >= (sr.bottom - SNAP_THRESH))) {  
						if ((winpos->x + winpos->cx == sr.left) || (winpos->x == sr.right)) {
							snapped = TRUE;
							if(node->sizing) winpos->cy = sr.bottom - winpos->y;
							else winpos->y = sr.bottom - winpos->cy;  
						}
					} 
				}
				return snapped;
			}
		/*
		case WM_SHOWWINDOW:
			{
				struct FrameListNode *node = (struct FrameListNode *)GetWindowLong(hwnd, GWL_USERDATA);
				if(!node) break;
				if (lParam)
					break;
				if (!DBGetContactSettingByte(NULL, "CLUI", "FadeInOut", 0) || !IsWinVer2000Plus())
					break;
				if(!(node->frame_data.Flags & F_VISIBLE))
					break;
				if (wParam)
					SetForegroundWindow(hwnd);
				if(MyAnimateWindow)
					MyAnimateWindow(hwnd, 200, AW_BLEND | (wParam ? 0 : AW_HIDE));
				//SetWindowPos(cli.hwndContactTree, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
			}
			break;
		*/
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void InitFrames() {
	WNDCLASS wndclass;

	hUserDll = LoadLibraryA("user32.dll");
	if (hUserDll) {
		MyFramesSetLayeredWindowAttributes = (BOOL(WINAPI *) (HWND, COLORREF, BYTE, DWORD)) GetProcAddress(hUserDll, "SetLayeredWindowAttributes");
		MyAnimateWindow = (BOOL(WINAPI *) (HWND, DWORD, DWORD)) GetProcAddress(hUserDll, "AnimateWindow");
	}

    wndclass.style         = CS_VREDRAW | CS_HREDRAW | CS_GLOBALCLASS;
    wndclass.lpfnWndProc   = FrameWindowProc;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = 0;
    wndclass.hInstance     = g_hInst;
    wndclass.hIcon         = 0;
    wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)(COLOR_3DFACE+1);
    wndclass.lpszMenuName  = NULL;
    wndclass.lpszClassName = _T("CListFrameClass");
	RegisterClass(&wndclass);

	InitializeCriticalSection(&cs);

	MakeFrameServices();
}

void DeinitFrames() {
	struct FrameListNode *current;

	EnterCriticalSection(&cs);
	current = frame_list;
	while(current) {
		DestroyWindow(current->hwndParent);

		frame_list = current->next;
		free(current->frame_data.name);
		free(current);
		current = frame_list;
	}
	LeaveCriticalSection(&cs);
	DeleteCriticalSection(&cs);

	UnregisterClass(_T("CListFrameClas"), g_hInst);

	if(hUserDll)
		FreeLibrary(hUserDll);
}
