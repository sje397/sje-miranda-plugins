#include "stdafx.h"
#include "pinggraph.h"

#include <map>

#define ID_REPAINT_TIMER		10101

struct WindowData {
	HANDLE hContact;
	HistoryList list;
};

#define WM_REBUILDLIST			(WM_USER + 1)

LRESULT CALLBACK GraphWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_REBUILDLIST:
		{
			WindowData *wd = (WindowData *)GetWindowLong(hwnd, GWL_USERDATA);

			bool found = false;
			EnterCriticalSection(&data_list_cs);
			for(PINGLIST::iterator i = data_list.begin(); i != data_list.end(); i++) {
				if(i->hContact == wd->hContact) {
					wd->list = i->history;
					found = true;
					break;
				}
			}
			LeaveCriticalSection(&data_list_cs);

			if(!found) {
				PostMessage(hwnd, WM_CLOSE, 0, 0);
				return TRUE;
			}

			InvalidateRect(hwnd, 0, FALSE);
		}
		return TRUE;
	case WM_SHOWWINDOW:
		if(wParam == TRUE && lParam == 0) {
			//WindowData *wd = (WindowData *)GetWindowLong(hwnd, GWL_USERDATA);

			KillTimer(hwnd, ID_REPAINT_TIMER);
#ifdef min
			SetTimer(hwnd, ID_REPAINT_TIMER, min(options.ping_period * 1000, 5000), 0);
#else
			SetTimer(hwnd, ID_REPAINT_TIMER, std::min(options.ping_period * 1000, 5000), 0);
#endif

			SendMessage(hwnd, WM_REBUILDLIST, 0, 0);
		}
		break;

	case WM_TIMER:
		{
			SendMessage(hwnd, WM_REBUILDLIST, 0, 0);
		}
		return TRUE;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			RECT r;
			WindowData *wd = (WindowData *)GetWindowLong(hwnd, GWL_USERDATA);
			if(hdc = BeginPaint(hwnd, &ps)) {
				GetClientRect(hwnd, &r);
				FillRect(hdc, &r, GetSysColorBrush(COLOR_WINDOW));

				short max_value = -1; // this is minimum graph height, in ms
				for(HistoryList::iterator hli = wd->list.begin(); hli != wd->list.end(); hli++) {
					if(hli->first > max_value) max_value = hli->first;
				}

				max_value = (int)(max_value * 1.2f);
				if(max_value < MIN_GRAPH_HEIGHT) max_value = MIN_GRAPH_HEIGHT;

#ifdef max
				float unit_width = (r.right - r.left) / (float)max((int)wd->list.size(), MIN_BARS), // space for at least MIN_BARS bars
#else
				float unit_width = (r.right - r.left) / (float)std::max((int)wd->list.size(), MIN_BARS), // space for at least MIN_BARS bars
#endif
					unit_height = (r.bottom - r.top) / (float)max_value;

				RECT bar;
				bar.bottom = r.bottom;
				float x = r.right - (unit_width * wd->list.size() + 0.5f);
				bar.left = (int)(x + 0.5f);
				bar.right = (int)(x + unit_width + 0.5f);

				DWORD last_time = 0, time, start_time = 0;
				if(wd->list.size())
					start_time = wd->list.begin()->second;

				HPEN hPenOld, hPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DDKSHADOW));
				hPenOld = (HPEN)SelectObject(hdc, hPen);

				for(HistoryList::iterator hi = wd->list.begin(); hi != wd->list.end(); hi++) {
					if(hi->first != -1) {
						bar.top = bar.bottom - (int)(hi->first * unit_height + 0.5f);
						FillRect(hdc, &bar, GetSysColorBrush(COLOR_HOTLIGHT));
					}

					time = hi->second - start_time;
					if(time / MARK_PERIOD != last_time / MARK_PERIOD) {
						MoveToEx(hdc, bar.left, r.bottom, 0);
						LineTo(hdc, bar.left, r.top);
					}

					last_time = time;

					x += unit_width;
					bar.left = bar.right;
					bar.right = (int)(x + unit_width + 0.5f);
				}

				// draw horizontal lines to mark every 100ms
				for(int li = 0; li < max_value; li += MARK_TIME) {
					MoveToEx(hdc, r.left, r.bottom - (int)(li * unit_height + 0.5f), 0);
					LineTo(hdc, r.right, r.bottom - (int)(li * unit_height + 0.5f));
				}
				
				SelectObject(hdc, hPenOld);
				DeleteObject(hPen);

				SetBkMode(hdc, TRANSPARENT);
				SetTextColor(hdc, GetSysColor(COLOR_3DDKSHADOW));
				char buff[64];
				sprintf(buff, "%dms", MARK_TIME);
				TextOut(hdc, r.left + 10, r.bottom - (int)(unit_height * MARK_TIME + 0.5f), buff, 5);

				EndPaint(hwnd, &ps);
			}
		}
		return TRUE;

	case WM_ERASEBKGND:
		return TRUE;

	case WM_SIZE:
		InvalidateRect(hwnd, 0, FALSE);
		break;
	case WM_CLOSE:
		{
			KillTimer(hwnd, ID_REPAINT_TIMER);
			WindowData *wd = (WindowData *)GetWindowLong(hwnd, GWL_USERDATA);
			if(CallService(MS_DB_CONTACT_IS, (WPARAM)wd->hContact, 0))
				Utils_SaveWindowPosition(hwnd, wd->hContact, PROTO, "pinggraphwnd");
		}
		break;
	case WM_DESTROY:
		{
			WindowData *wd = (WindowData *)GetWindowLong(hwnd, GWL_USERDATA);
			DBDeleteContactSetting(wd->hContact, PROTO, "WindowHandle");
			delete wd;
		}
		// drop through
	};

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

int ShowGraph(WPARAM wParam, LPARAM lParam) {
	HWND hGraphWnd = (HWND)DBGetContactSettingDword((HANDLE)wParam, PROTO, "WindowHandle", 0);
	if(hGraphWnd) {
		ShowWindow(hGraphWnd, SW_SHOW);
		SetWindowPos(hGraphWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		return 0;
	}

	WNDCLASS wndclass;
    wndclass.style         = 0;
    wndclass.lpfnWndProc   = GraphWindowProc;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = 0;
    wndclass.hInstance     = hInst;
    wndclass.hIcon         = LoadSkinnedProtoIcon(PROTO, ID_STATUS_ONLINE);
    wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)(COLOR_3DFACE+1);
    wndclass.lpszMenuName  = NULL;
    wndclass.lpszClassName = PROTO "GraphWindow";
	RegisterClass(&wndclass);

	DBVARIANT dbv;
	char title[256];
	strcpy(title, "Ping Graph");
	if(!DBGetContactSetting((HANDLE)wParam, PROTO, "Nick", &dbv)) {
		strcat(title, " - ");
		strncat(title, dbv.pszVal, 256 - 13);
		DBFreeVariant(&dbv);
	}

	HWND parent = 0;
	hGraphWnd = CreateWindowEx(0, PROTO "GraphWindow", title,
						(WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN) & ~CS_VREDRAW & ~CS_HREDRAW,
						0,0,800,600,parent,NULL,hInst,NULL);
	
	DBWriteContactSettingDword((HANDLE)wParam, PROTO, "WindowHandle", (DWORD)hGraphWnd);

	WindowData *wd = new WindowData;
	wd->hContact = (HANDLE)wParam; // wParam is contact handle

	SetWindowLong(hGraphWnd, GWL_USERDATA, (LONG)wd); 

	if(CallService(MS_DB_CONTACT_IS, (WPARAM)wd->hContact, 0))
		Utils_RestoreWindowPosition(hGraphWnd, wd->hContact, PROTO, "pinggraphwnd");

	if(!IsWindowVisible(hGraphWnd))
		ShowWindow(hGraphWnd, SW_SHOW);

	return 0;
}

// save window positions, close windows
void graphs_cleanup() {
	HWND hwnd;
	char *proto;
	HANDLE hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
	while ( hContact != NULL )
	{
		proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
		if (proto &&  lstrcmp( PROTO, proto) == 0) {
			if((hwnd = (HWND)DBGetContactSettingDword(hContact, PROTO, "WindowHandle", 0)) != 0) {
				DestroyWindow(hwnd);
				DBWriteContactSettingByte(hContact, PROTO, "WindowWasOpen", 1);
			}
		}

		hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
	}	
}

// restore windows that were open when cleanup was called last
void graphs_init() {
	char *proto;
	HANDLE hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
	while ( hContact != NULL )
	{
		proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
		if (proto &&  lstrcmp( PROTO, proto) == 0) {
			if(DBGetContactSettingByte(hContact, PROTO, "WindowWasOpen", 0) != 0) {
				ShowGraph((WPARAM)hContact, 0);
				DBWriteContactSettingByte(hContact, PROTO, "WindowWasOpen", 0);
			}
		}

		hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
	}	
}
