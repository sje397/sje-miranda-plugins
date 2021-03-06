#include "common.h"
#include "popwin.h"
#include "message_pump.h"
#include "options.h"

HMODULE hUserDll;
BOOL (WINAPI *MySetLayeredWindowAttributes)(HWND,COLORREF,BYTE,DWORD) = 0;
BOOL (WINAPI *MyAnimateWindow)(HWND hWnd,DWORD dwTime,DWORD dwFlags) = 0;
HMONITOR (WINAPI *MyMonitorFromRect)(LPCRECT rect, DWORD flags) = 0;
BOOL (WINAPI *MyGetMonitorInfo)(HMONITOR hMonitor, LPMONITORINFO mi) = 0;

#define ID_CLOSETIMER		0x0101
#define ID_MOVETIMER		0x0102

DWORD pop_start_x, pop_start_y;
int global_mouse_in = 0;

void trimW(wchar_t *str) {
	int len = (int)wcslen(str), pos;
	// trim whitespace (e.g. from OTR detection)
	for(pos = len - 1; pos >= 0; pos--) {
		if(str[pos] == L' ' || str[pos] == L'\t' || str[pos] == L'\r' || str[pos] == L'\n') str[pos] = 0;
		else break;
	}

	// remove tabs
	for(pos = len - 1; pos >= 0; pos--)
		if(str[pos] == L'\t') str[pos] = L' ';
}

void SetStartValues(void) 
{
	RECT wa_rect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &wa_rect, 0);
	if (options.use_mim_monitor && MyMonitorFromRect && MyGetMonitorInfo) 
	{
		RECT clr;
		GetWindowRect((HWND)CallService(MS_CLUI_GETHWND, 0, 0), &clr);
		HMONITOR hMonitor = MyMonitorFromRect(&clr, MONITOR_DEFAULTTONEAREST);
		if (hMonitor)
		{
			MONITORINFO mi;
			mi.cbSize = sizeof(mi);
			if (MyGetMonitorInfo(hMonitor, &mi))
				wa_rect = mi.rcWork;
		}
	}

	if(options.location == PL_BOTTOMRIGHT || options.location == PL_TOPRIGHT)
		pop_start_x = wa_rect.right - options.win_width - 1;
	else
		pop_start_x = wa_rect.left + 1;

	if(options.location == PL_BOTTOMRIGHT || options.location == PL_BOTTOMLEFT)
		pop_start_y = wa_rect.bottom - 1;
	else
		pop_start_y = wa_rect.top + 1;
}

struct HWNDStackNode {
	HWND hwnd;
	struct HWNDStackNode *next;
};

HWNDStackNode *hwnd_stack_top = 0;
int stack_size = 0;

void RepositionWindows() {
	HWNDStackNode *current;	
	int x = pop_start_x, y = pop_start_y;
	int height;//, total_height = 0;

	/*
	current = hwnd_stack_top;
	while(current) {
		SendMessage(current->hwnd, PUM_GETHEIGHT, (WPARAM)&height, 0);
		total_height += height;
		current = current->next;
	}
	*/

	current = hwnd_stack_top;
	while(current) {
		SendMessage(current->hwnd, PUM_GETHEIGHT, (WPARAM)&height, 0);
		if(options.location == PL_BOTTOMRIGHT || options.location == PL_BOTTOMLEFT) y -= height + 1;
		SendMessage(current->hwnd, PUM_MOVE, (WPARAM)x, (LPARAM)y);		
		if(options.location == PL_TOPRIGHT || options.location == PL_TOPLEFT) y += height + 1;

		current = current->next;
	}
}

void AddWindowToStack(HWND hwnd) {
	SetStartValues();

	HWNDStackNode *new_node = (HWNDStackNode *)mir_alloc(sizeof(HWNDStackNode));
	new_node->hwnd = hwnd;
	new_node->next = hwnd_stack_top;
	hwnd_stack_top = new_node;

	int height;
	SendMessage(hwnd, PUM_GETHEIGHT, (WPARAM)&height, 0);

	int x = pop_start_x, y = pop_start_y;
	if(options.location == PL_BOTTOMRIGHT || options.location == PL_TOPRIGHT)
		x += options.win_width;
	else
		x -= options.win_width;

	if(options.location == PL_BOTTOMRIGHT || options.location == PL_BOTTOMLEFT) y -= height;
	SetWindowPos(hwnd, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	if(options.location == PL_TOPRIGHT || options.location == PL_TOPLEFT) y += height;

	stack_size++;

	RepositionWindows();
}

void RemoveWindowFromStack(HWND hwnd) {
	HWNDStackNode *current = hwnd_stack_top, *prev = 0;
	while(current) {
		if(current->hwnd == hwnd) {
			if(prev) {
				prev->next = current->next;
			} else {
				hwnd_stack_top = current->next;
			}
			mir_free(current);
			stack_size--;
			break;
		}
		
		prev = current; 
		current = current->next;
	}

	if(hwnd_stack_top) RepositionWindows();
}

void ClearStack() {
	while(hwnd_stack_top) {
		DestroyWindow(hwnd_stack_top->hwnd);
	}
}

void BroadcastMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
	HWNDStackNode *current = hwnd_stack_top;
	while(current) {
		SendMessage(current->hwnd, msg, wParam, lParam);
		current = current->next;
	}
}

struct PopupWindowData {
	PopupData *pd;
	int new_x, new_y;
	bool is_round, av_is_round, mouse_in, close_on_leave;
	bool custom_col;
	HBRUSH bkBrush, barBrush, underlineBrush;
	HPEN bPen;
	TCHAR tbuff[128];
	int tb_height, av_height, text_height, time_height, time_width;
	int real_av_width, real_av_height;
	bool have_av;
	HANDLE hNotify;
};

LRESULT CALLBACK PopupWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	PopupWindowData *pwd = (PopupWindowData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	PopupData *pd = 0;
	if(pwd) pd = pwd->pd;

	switch(uMsg) {
		case WM_CREATE:
			{
				CREATESTRUCT *cs = (CREATESTRUCT *)lParam;
				pwd = (PopupWindowData *)mir_alloc(sizeof(PopupWindowData));
				pd = (PopupData *)cs->lpCreateParams;
				pwd->pd = pd;
				pwd->hNotify = 0;

				trimW(pwd->pd->pwzTitle);
				trimW(pwd->pd->pwzText);

				pwd->is_round = options.round;
				pwd->av_is_round = options.av_round;
				pwd->mouse_in = pwd->close_on_leave = false;
				pwd->custom_col = (pd->colorBack != pd->colorText);
				
				pwd->tb_height = pwd->av_height = pwd->text_height = pwd->time_height = pwd->time_width = 0;
				pwd->have_av = false;

				if(pwd->custom_col) {
					pwd->bkBrush = CreateSolidBrush(pd->colorBack);
					
					//pwd->barBrush = CreateSolidBrush(pd->colorBack / 2); // make sidebar a dark version of the bg
					//DWORD darkBg = (((pd->colorBack & 0xff0000) >> 1) & 0xff0000) + (((pd->colorBack & 0xff00) >> 1) & 0xff00) + (((pd->colorBack & 0xff) >> 1) & 0xff);
					//DWORD darkBg = (pdColorBack >> 1) & 0x7f7f7f; // equivalent to above :)

					DWORD darkBg = pd->colorBack - ((pd->colorBack >> 2) & 0x3f3f3f); // 3/4 of current individual RGB components
					pwd->barBrush = CreateSolidBrush(darkBg); // make sidebar a dark version of the bg
					pwd->underlineBrush = CreateSolidBrush(pd->colorBack); // make sidebar a dark version of the bg
				} else {
					pwd->bkBrush = CreateSolidBrush(colBg);
					pwd->barBrush = CreateSolidBrush(colSidebar);
					pwd->underlineBrush = CreateSolidBrush(colTitleUnderline);
				}

				if(options.border) pwd->bPen = (HPEN)CreatePen(PS_SOLID, 1, colBorder); 
				else pwd->bPen = CreatePen(PS_SOLID, 1, pwd->custom_col ? pd->colorBack : colBg);
				
				SYSTEMTIME st;
				GetLocalTime(&st);
				GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &st, 0, pwd->tbuff, 128);

				SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pwd);

				if(pd->timeout == -1 || (pd->timeout == 0 && options.default_timeout == -1)) {
					// make a really long timeout - say 7 days? ;)
					SetTimer(hwnd, ID_CLOSETIMER, 7 * 24 * 60 * 60 * 1000, 0);
				} else {
					if(pd->timeout == 0) {
						SetTimer(hwnd, ID_CLOSETIMER, options.default_timeout * 1000, 0);
					} else {
						SetTimer(hwnd, ID_CLOSETIMER, pd->timeout * 1000, 0);
					}
				}

				AddWindowToStack(hwnd); // this updates our size
			}

			// transparency
#ifdef WS_EX_LAYERED 
			SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
#endif

#ifdef CS_DROPSHADOW
			if (options.drop_shadow) {
				  SetClassLong(hwnd, GCL_STYLE, CS_DROPSHADOW);
			}
#endif

#ifdef LWA_ALPHA
			if(MySetLayeredWindowAttributes) {
				MySetLayeredWindowAttributes(hwnd, RGB(0,0,0), (int)(options.opacity / 100.0 * 255), LWA_ALPHA);
				if(options.trans_bg) {
					COLORREF bg;
					if(pd->colorBack == pd->colorText)
						bg = colBg;
					else
						bg = pd->colorBack;
					MySetLayeredWindowAttributes(hwnd, bg, 0, LWA_COLORKEY);
				}
			}
#endif
			PostMessage(hwnd, PM_INIT, (WPARAM)hwnd, 0);
			return 0;
		case WM_MOUSEMOVE:
			if(pwd && !pwd->mouse_in) {
				pwd->mouse_in = true;
				global_mouse_in++;
				TRACKMOUSEEVENT tme = { sizeof(tme) };
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = hwnd;
				TrackMouseEvent(&tme);
			}
			break;
		case WM_MOUSELEAVE:
			if(pwd && pwd->mouse_in) {
				pwd->mouse_in = false;
				global_mouse_in--;
			}
			return 0;
		case WM_LBUTTONUP:
			// fake STN_CLICKED notification
			SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(0, STN_CLICKED), 0);
			break;
		case WM_TIMER:
			if(wParam == ID_CLOSETIMER) {
				KillTimer(hwnd, ID_CLOSETIMER);
				if(pwd->mouse_in || (options.global_hover && global_mouse_in))
					SetTimer(hwnd, ID_CLOSETIMER, 800, 0); // reset timer if mouse in window - allow another 800 ms
				else {
					PostMessage(hwnd, PM_DESTROY, 0, 0);
				}
				return TRUE;
			} else if(wParam == ID_MOVETIMER) {
				RECT r;
				GetWindowRect(hwnd, &r);

				if(r.left == pwd->new_x && r.top == pwd->new_y) {
					KillTimer(hwnd, ID_MOVETIMER);
					return TRUE;
				} 
				int adj_x = (pwd->new_x - r.left) / 4, adj_y = (pwd->new_y - r.top) / 4;
				if(adj_x == 0) adj_x = (pwd->new_x - r.left);
				if(adj_y == 0) adj_y = (pwd->new_y - r.top);

				int x = r.left + adj_x, y = r.top + adj_y;
				//SetWindowPos(hwnd, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS);
				SetWindowPos(hwnd, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);


				/*
				// clip to monitor bounds (paints badly!)
				//HDC hdc = GetDC(hwnd);
				HMONITOR hMonitor = MonitorFromRect(&r, MONITOR_DEFAULTTONEAREST);
				MONITORINFO mi;
				mi.cbSize = sizeof(mi);
				GetMonitorInfo(hMonitor, &mi);

				POINT p[2];
				p[0].x = mi.rcWork.left; p[0].y = mi.rcWork.top; p[1].x = mi.rcWork.right; p[1].y = mi.rcWork.bottom;
				//LPtoDP(hdc, p, 2);
				ScreenToClient(hwnd, &p[0]); ScreenToClient(hwnd, &p[1]);

				HRGN hMonRgn = CreateRectRgn(p[0].x, p[0].y, p[1].x, p[1].y);
				//ReleaseDC(hwnd, hdc);

				RECT cr; GetClientRect(hwnd, &cr);
				HRGN hWndRgn = CreateRectRgn(cr.left, cr.top, cr.right + 3, cr.bottom + 3);
				CombineRgn(hMonRgn, hMonRgn, hWndRgn, RGN_AND);

				// round corners
				if(options.round) {
					HRGN hRgn1;
					int v,h, w=10;
					h=(r.right-r.left)>(w*2)?w:(r.right-r.left);
					v=(r.bottom-r.top)>(w*2)?w:(r.bottom-r.top);
					h=(h<v)?h:v;
					hRgn1=CreateRoundRectRgn(0,0,(r.right-r.left+1),(r.bottom-r.top+1),h,h);
					CombineRgn(hMonRgn, hMonRgn, hRgn1, RGN_AND);
					DeleteObject(hRgn1);
				}

				SetWindowRgn(hwnd, hMonRgn, TRUE);

				DeleteObject(hWndRgn);
				
				InvalidateRect(hwnd, 0, TRUE);
				*/

				if(!IsWindowVisible(hwnd)) {
					ShowWindow(hwnd, SW_SHOWNOACTIVATE);
					UpdateWindow(hwnd);
				}
				return TRUE;
			}
			break;
		case WM_ERASEBKGND:
			{
				HDC hdc = (HDC) wParam; 
				RECT r, r_bar;
				GetClientRect(hwnd, &r); 
				
				// bg
				FillRect(hdc, &r, pwd->bkBrush);
				// sidebar
				r_bar = r;
				r_bar.right = r.left + options.sb_width;
				FillRect(hdc, &r_bar, pwd->barBrush);
				// border
				if(options.border) {

					HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
					HPEN hOldPen = (HPEN)SelectObject(hdc, pwd->bPen);

					int h = 0;
					if(options.round) {
						int v;
						int w=14;
						h=(r.right-r.left)>(w*2)?w:(r.right-r.left);
						v=(r.bottom-r.top)>(w*2)?w:(r.bottom-r.top);
						h=(h<v)?h:v;
					//} else {
					//	Rectangle(hdc, r.left, r.top, (r.right - r.left), (r.bottom - r.top));
					}
					RoundRect(hdc, 0, 0, (r.right - r.left), (r.bottom - r.top), h, h);

					SelectObject(hdc, hOldBrush);
					SelectObject(hdc, hOldPen);
				}

			}
			return TRUE;
		case WM_PAINT:
			{
				RECT r;
				//if(GetUpdateRect(hwnd, &r, TRUE)) {
					PAINTSTRUCT ps;
					BeginPaint(hwnd, &ps);
					HDC hdc = ps.hdc;
					GetClientRect(hwnd, &r);

					// text background
					//if(pwd->custom_col) SetBkColor(ps.hdc, pd->colorBack);
					//else SetBkColor(ps.hdc, colBg);
					SetBkMode(hdc, TRANSPARENT);

					// avatar & time if with avatar
					if(options.av_layout != PAV_NONE && (pwd->have_av || options.time_layout == PT_WITHAV)) {
						RECT avr;
						avr.top = options.av_padding;
			
						if(options.av_layout == PAV_LEFT) {
							avr.left = r.left + options.av_padding;
							if(pwd->have_av && options.time_layout == PT_WITHAV) avr.right = avr.left + max(pwd->real_av_width, pwd->time_width);
							else if(pwd->have_av) avr.right = avr.left + pwd->real_av_width;
							else avr.right = avr.left + pwd->time_width;
							r.left = avr.right;
						} else if(options.av_layout == PAV_RIGHT) {
							avr.right = r.right - options.av_padding;
							if(pwd->have_av && options.time_layout == PT_WITHAV) avr.left = avr.right - max(pwd->real_av_width, pwd->time_width);
							else if(pwd->have_av) avr.left = avr.right - pwd->real_av_width;
							else avr.left = avr.right - pwd->time_width;
							r.right = avr.left;
						}
						
						if(options.time_layout == PT_WITHAV) {
							avr.top = options.padding;
							avr.bottom = avr.top + pwd->time_height;
							if(pwd->custom_col) SetTextColor(ps.hdc, pd->colorText);
							else SetTextColor(ps.hdc, colTime);
							if(hFontTime) SelectObject(hdc, (HGDIOBJ)hFontTime);
							DrawText(ps.hdc, pwd->tbuff, (int)_tcslen(pwd->tbuff), &avr, DT_VCENTER | DT_CENTER | DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
							avr.top = avr.bottom + options.av_padding;
						}

						if(pwd->have_av) {
							// correct for wider time
							if(options.time_layout == PT_WITHAV && pwd->time_width > options.av_size) {
								avr.left = avr.left + (pwd->time_width - pwd->real_av_width) / 2;
								avr.right = avr.left + pwd->real_av_width;
							}
							avr.bottom = avr.top + pwd->real_av_height;

							AVATARDRAWREQUEST adr = {0};
							adr.cbSize = sizeof(adr);
							adr.hContact = pd->hContact;
							adr.hTargetDC = ps.hdc;
							adr.rcDraw = avr;
							adr.dwFlags = (pwd->av_is_round ? AVDRQ_ROUNDEDCORNER : 0);
							adr.radius = 5; //(pwd->av_is_round ? 5 : 0);
							
							CallService(MS_AV_DRAWAVATAR, 0, (LPARAM)&adr);
						}
					}
					
					// title icon
					int iconx, textxmin = r.left + options.padding, textxmax = r.right - options.padding;
					if(pd->hIcon) {
						if(options.right_icon) {
							iconx = r.right - (16 + options.padding);
							textxmax -= 16 + options.padding;
						} else {
							iconx = r.left + options.padding;
							textxmin += 16 + options.padding;
						}
						DrawIconEx(ps.hdc, iconx, options.padding + (pwd->tb_height - 16) / 2, pd->hIcon, 16, 16, 0, NULL, DI_NORMAL);
					}

					// title time
					if(options.time_layout == PT_LEFT || options.time_layout == PT_RIGHT) {
						RECT ttr;
						ttr.top = r.top + options.padding; ttr.bottom = ttr.top + pwd->tb_height;
						if(pwd->custom_col) SetTextColor(ps.hdc, pd->colorText);
						else SetTextColor(ps.hdc, colTime);
						if(hFontTime) SelectObject(hdc, (HGDIOBJ)hFontTime);
						switch(options.time_layout) {
							case PT_LEFT:
								ttr.left = textxmin; ttr.right = ttr.left + pwd->time_width;
								textxmin += pwd->time_width + options.padding;
								DrawText(ps.hdc, pwd->tbuff, (int)_tcslen(pwd->tbuff), &ttr, DT_VCENTER | DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
								break;
							case PT_RIGHT:
								ttr.right = textxmax; ttr.left = ttr.right - pwd->time_width;
								textxmax -= pwd->time_width + options.padding;
								DrawText(ps.hdc, pwd->tbuff, (int)_tcslen(pwd->tbuff), &ttr, DT_VCENTER | DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
								break;
							default:
								break;
						}
					}

					if(textxmin < options.sb_width) textxmin = options.sb_width + options.padding / 2;

					// title text
					if(hFontFirstLine) SelectObject(ps.hdc, (HGDIOBJ)hFontFirstLine);
					RECT tr;
					tr.left = textxmin; tr.right = textxmax; tr.top = r.top + options.padding; tr.bottom = tr.top + pwd->tb_height;
					
					if(pwd->custom_col) SetTextColor(ps.hdc, pd->colorText);
					else SetTextColor(ps.hdc, colFirstLine);
					TCHAR *title = mir_u2t(pd->pwzTitle);
					DrawText(ps.hdc, title, (int)_tcslen(title), &tr, DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS | DT_SINGLELINE | DT_NOPREFIX);
					mir_free(title);

					// title underline
					RECT tur;
					tur.left = r.left + options.sb_width + options.padding;
					tur.right = r.right - options.padding;
					tur.top = tr.bottom + options.padding/2;
					tur.bottom = tur.top + 1;
					FillRect(ps.hdc, &tur, pwd->underlineBrush);

					// second line(s)
					if(pd->pwzText[0]) {
						if(hFontSecondLine) SelectObject(ps.hdc, (HGDIOBJ)hFontSecondLine);
						if(!pwd->custom_col)
							SetTextColor(ps.hdc, colSecondLine);

						// expand text if no avatar and the time isn't too large
						if(options.av_layout != PAV_NONE && options.time_layout == PT_WITHAV && pwd->time_height <= pwd->tb_height && !pwd->have_av)
							GetClientRect(hwnd, &r);

						TCHAR *text = mir_u2t(pd->pwzText);
						tr.left = r.left + options.padding + options.text_indent; tr.right = r.right - options.padding; tr.top = tr.bottom + options.padding; tr.bottom = r.bottom - options.padding;
						DrawText(ps.hdc, text, (int)_tcslen(text), &tr, DT_NOPREFIX | DT_WORDBREAK | DT_EXTERNALLEADING | DT_TOP | DT_LEFT | DT_WORD_ELLIPSIS);
						mir_free(text);
					}

					EndPaint(hwnd, &ps);
				//}
			}
			return 0;
		case WM_DESTROY: 
			if(pwd->mouse_in) global_mouse_in--;

			ShowWindow(hwnd, SW_HIDE);				

			DeleteObject(pwd->bkBrush);
			DeleteObject(pwd->bPen);
			DeleteObject(pwd->barBrush);
			DeleteObject(pwd->underlineBrush);
			KillTimer(hwnd, ID_MOVETIMER);
			KillTimer(hwnd, ID_CLOSETIMER);

			RemoveWindowFromStack(hwnd);

			SendMessage(hwnd, PM_DIENOTIFY, 0, 0);

			if (pd) {
				mir_free(pd->pwzTitle);
				mir_free(pd->pwzText);
				mir_free(pd);
			}
			mir_free(pwd); pwd = 0; pd = 0;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);

			break;
		case PUM_UPDATERGN:
			// round corners
			if(pwd->is_round) {
				HRGN hRgn1;
				RECT r;
				
				int v,h;
				int w=11;
				GetWindowRect(hwnd,&r);
				h=(r.right-r.left)>(w*2)?w:(r.right-r.left);
				v=(r.bottom-r.top)>(w*2)?w:(r.bottom-r.top);
				h=(h<v)?h:v;
				hRgn1=CreateRoundRectRgn(0,0,(r.right-r.left) + 1,(r.bottom-r.top) + 1,h,h);
				SetWindowRgn(hwnd,hRgn1,FALSE);
			}
			return TRUE;

		case PUM_MOVE:
			{
				if(options.animate) {
					KillTimer(hwnd, ID_MOVETIMER);
					pwd->new_x = (int)wParam;
					pwd->new_y = (int)lParam;
					SetTimer(hwnd, ID_MOVETIMER, 10, 0);
				} else {
					SetWindowPos(hwnd, 0, (int)wParam, (int)lParam, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
					if(!IsWindowVisible(hwnd)) { 
						ShowWindow(hwnd, SW_SHOWNOACTIVATE);
						UpdateWindow(hwnd);
					}
				}
			}
			return TRUE;
		case PUM_SETTEXT:
			{
				mir_free(pd->pszText);
				if(lParam)
					pd->pwzText = mir_wstrdup((wchar_t *)lParam);
				else
					pd->pwzText = NULL;
				// mir_free((void *)lParam); // freed in message pump in case the window has gone
				InvalidateRect(hwnd, 0, TRUE);
				RepositionWindows();
			}
			return TRUE;

		case PUM_GETCONTACT:
			{
				HANDLE *phContact = (HANDLE *)wParam;
				*phContact = pd->hContact;
				if(lParam) SetEvent((HANDLE)lParam);
			}
			return TRUE;
		case PUM_GETHEIGHT:
			{
				int *pHeight = (int *)wParam;
				HDC hdc = GetDC(hwnd);
				SIZE size;

				// time_height + width
				if(options.time_layout != PT_NONE) {
					SIZE size_t;
					if(hFontTime) SelectObject(hdc, (HGDIOBJ)hFontTime);
					GetTextExtentPoint32(hdc, pwd->tbuff, (int)_tcslen(pwd->tbuff), &size_t);
					pwd->time_height = size_t.cy;
					pwd->time_width = size_t.cx;
				}

				// titlebar height
				if(hFontFirstLine) SelectObject(hdc, (HGDIOBJ)hFontFirstLine);
				TCHAR *title = mir_u2t(pd->pwzTitle);
				GetTextExtentPoint32(hdc, title, (int)_tcslen(title), &size);
				mir_free(title);
				pwd->tb_height = size.cy;
				if(options.time_layout == PT_LEFT || options.time_layout == PT_RIGHT) {
					if(pwd->tb_height < pwd->time_height) pwd->tb_height = pwd->time_height;
				}
				if(pwd->tb_height < 16) pwd->tb_height = 16;

				// avatar height
				if(options.av_layout != PAV_NONE && ServiceExists(MS_AV_DRAWAVATAR)) {
					AVATARCACHEENTRY *ace = (AVATARCACHEENTRY *)CallService(MS_AV_GETAVATARBITMAP, (WPARAM)pd->hContact, 0);
					if(ace && (ace->dwFlags & AVS_BITMAP_VALID) && !(ace->dwFlags & AVS_HIDEONCLIST)) {
						if(ace->bmHeight >= ace->bmWidth) {
							pwd->real_av_height = options.av_size;
							pwd->real_av_width = (int)(options.av_size * (ace->bmWidth / (double)ace->bmHeight));
						} else {
							pwd->real_av_height = (int)(options.av_size * (ace->bmHeight / (double)ace->bmWidth));
							pwd->real_av_width = options.av_size;
						}
						pwd->have_av = true;
						pwd->av_height = pwd->real_av_height; 
					}
				}

				// text height
				if (pd->pwzText[0]) {
					RECT r;
					r.left = r.top = 0; 
					r.right = options.win_width - 2 * options.padding - options.text_indent;
					if(pwd->have_av && options.time_layout == PT_WITHAV)
						r.right -= (max(options.av_size, pwd->time_width) + options.padding); 
					else if(pwd->have_av)
						r.right -= (options.av_size + options.padding); 
					else if(options.av_layout != PAV_NONE && options.time_layout == PT_WITHAV && pwd->time_height >= pwd->tb_height)
						r.right -= pwd->time_width + options.padding;

					if(hFontSecondLine) SelectObject(hdc, (HGDIOBJ)hFontSecondLine);
					TCHAR *text = mir_u2t(pd->pwzText);
					DrawText(hdc, text, (int)_tcslen(text), &r, DT_CALCRECT | DT_NOPREFIX | DT_WORDBREAK | DT_EXTERNALLEADING | DT_TOP | DT_LEFT | DT_WORD_ELLIPSIS);
					pwd->text_height = r.bottom;
					mir_free(text);
				}

				ReleaseDC(hwnd, hdc);

				if(options.time_layout == PT_WITHAV && options.av_layout != PAV_NONE) 
					*pHeight = max(pwd->tb_height + pwd->text_height + 3 * options.padding, pwd->av_height + pwd->time_height + options.padding + 2 * options.av_padding);
				else 
					*pHeight = max(pwd->tb_height + pwd->text_height + 3 * options.padding, pwd->av_height + 2 * options.av_padding);

				if(*pHeight > options.win_max_height) *pHeight = options.win_max_height;

				RECT r;
				GetWindowRect(hwnd, &r);
				if(r.right - r.left != options.win_width || r.bottom - r.top != *pHeight) {
					SetWindowPos(hwnd, 0, 0, 0, options.win_width, *pHeight, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
					SendMessage(hwnd, PUM_UPDATERGN, 0, 0);
					InvalidateRect(hwnd, 0, TRUE);
				}
			}
			return TRUE;
		case PUM_GETOPAQUE:
			{
				void **pData = (void **)wParam;
				if(pd) *pData = pd->opaque;
				if(lParam) SetEvent((HANDLE)lParam);
			}
			return TRUE;
		case PUM_CHANGE:
			{
				KillTimer(hwnd, ID_CLOSETIMER);
				if (pd) {
					mir_free(pd->pwzTitle);
					mir_free(pd->pwzText);
					mir_free(pd);
				}
				pwd->pd = pd = (PopupData *)lParam;

				if(pd->timeout != -1) {
					if(pd->timeout == 0) {
						SetTimer(hwnd, ID_CLOSETIMER, 7 * 1000, 0);
					} else {
						SetTimer(hwnd, ID_CLOSETIMER, pd->timeout * 1000, 0);
					}
				} else {
					// make a really long timeout - say 7 days? ;)
					SetTimer(hwnd, ID_CLOSETIMER, 7 * 24 * 60 * 60 * 1000, 0);
				}

				InvalidateRect(hwnd, 0, TRUE);
				RepositionWindows();
			}
			return TRUE;

		case PUM_SETNOTIFYH:
			pwd->hNotify = (HANDLE)wParam;
			return TRUE;

		case PUM_UPDATENOTIFY:
			if(pwd->hNotify == (HANDLE)wParam) {
				pd->colorBack = MNotifyGetDWord(pwd->hNotify, NFOPT_BACKCOLOR, colBg);
				pd->colorText = MNotifyGetDWord(pwd->hNotify, NFOPT_TEXTCOLOR, colSecondLine);
				pd->timeout = MNotifyGetDWord(pwd->hNotify, NFOPT_TIMEOUT, options.default_timeout);
				pd->hContact = (HANDLE)MNotifyGetDWord(pwd->hNotify, NFOPT_CONTACT, 0);
				pd->hIcon = (HICON)MNotifyGetDWord(pwd->hNotify, NFOPT_ICON, 0);
				
				const wchar_t *swzName = MNotifyGetWString(pwd->hNotify, NFOPT_TITLEW, 0);
				mir_free(pd->pwzTitle);
				pd->pwzTitle = mir_wstrdup(swzName);

				const wchar_t *swzText = MNotifyGetWString(pwd->hNotify, NFOPT_TEXTW, 0);
				mir_free(pd->pwzText);
				pd->pwzText = mir_wstrdup(swzText);

				InvalidateRect(hwnd, 0, TRUE);
				RepositionWindows();
			}

			return TRUE;
		case PUM_KILLNOTIFY:
			if(pwd->hNotify != (HANDLE)wParam)
				return TRUE;
			// drop through

		case PM_DESTROY:
			PostMPMessage(MUM_DELETEPOPUP, 0, (LPARAM)hwnd);
			return TRUE;
	}


	if(pd && pd->windowProc)
		return CallWindowProc(pd->windowProc, hwnd, uMsg, wParam, lParam);
	else {
		// provide a way to close popups, if no PluginWindowProc is provided
		if(uMsg == WM_CONTEXTMENU) {
			SendMessage(hwnd, PM_DESTROY, 0, 0);
			return TRUE;
		} else
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

void InitWindowStack() {
	hUserDll = LoadLibrary(_T("user32.dll"));
	if (hUserDll) {
		MySetLayeredWindowAttributes = (BOOL (WINAPI *)(HWND,COLORREF,BYTE,DWORD))GetProcAddress(hUserDll, "SetLayeredWindowAttributes");
		MyAnimateWindow=(BOOL (WINAPI*)(HWND,DWORD,DWORD))GetProcAddress(hUserDll,"AnimateWindow");
		MyMonitorFromRect=(HMONITOR (WINAPI*)(LPCRECT,DWORD))GetProcAddress(hUserDll, "MonitorFromRect");
		#if defined( _UNICODE )
			MyGetMonitorInfo=(BOOL (WINAPI*)(HMONITOR,LPMONITORINFO))GetProcAddress(hUserDll, "GetMonitorInfoW");
		#else
			MyGetMonitorInfo=(BOOL (WINAPI*)(HMONITOR,LPMONITORINFO))GetProcAddress(hUserDll, "GetMonitorInfoA");
		#endif
	}
}

void DeinitWindowStack() {
	ClearStack();
	if(hUserDll) FreeLibrary(hUserDll);
}

