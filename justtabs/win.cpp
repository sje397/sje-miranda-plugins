#include "win.h"
#include "options.h"

HWND pluginwind = 0;
bool first_show = true;

HWND tab_ctrl;
HIMAGELIST clist_imagelist;
HWND active_window;
int message_icon_index = 0;

#ifdef _WIN32
#define STATUSCLASSNAMEW        L"msctls_statusbar32"
#define STATUSCLASSNAMEA        "msctls_statusbar32"

#ifdef UNICODE
#define STATUSCLASSNAME         STATUSCLASSNAMEW
#else
#define STATUSCLASSNAME         STATUSCLASSNAMEA
#endif

#else
#define STATUSCLASSNAME         "msctls_statusbar"
#endif

#define EM_SUBCLASSED			(WM_USER+0x101)
#define EM_UNSUBCLASSED			(WM_USER+0x102)

#define WMU_SETTAB				(WM_USER+0x300)

#define WMU_CALCWINICON			(WM_USER+0x301)

HANDLE window_list = 0;

typedef struct TabCtrlDataStruct
{
	int		lastClickTime;
	WPARAM  clickWParam;
	LPARAM  clickLParam;
	POINT	mouseLBDownPos;
	int		lastClickTab;
	HIMAGELIST hDragImageList;
	int		bDragging;
	int		bDragged;
	int		destTab;
	int		srcTab;
} TabCtrlData;

typedef struct {
	POINT p;
	bool moving;
} WindowData;

typedef struct {
	HWND hwndMsg;
	HANDLE hContact;
} TabData;

int GetTabFromHWND(HWND tabs, HWND child)
{
	TCITEM tci;
	int l, i;
	l = TabCtrl_GetItemCount(tabs);
	for (i = 0; i < l; i++) {
		tci.mask = TCIF_PARAM;
		TabCtrl_GetItem(tabs, i, &tci);
		if (((TabData *) tci.lParam)->hwndMsg == child) {
			return i;
		}
	}
	return -1;
}

int GetTabFromContact(HWND tabs, HANDLE hContact)
{
	TCITEM tci;
	int l, i;
	l = TabCtrl_GetItemCount(tabs);
	for (i = 0; i < l; i++) {
		tci.mask = TCIF_PARAM;
		TabCtrl_GetItem(tabs, i, &tci);
		if (((TabData *) tci.lParam)->hContact == hContact) {
			return i;
		}
	}
	return -1;
}


static WNDPROC OldTabCtrlProc;
BOOL CALLBACK TabCtrlProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void SubclassTabCtrl(HWND hwnd) {
	OldTabCtrlProc = (WNDPROC) SetWindowLong(hwnd, GWL_WNDPROC, (LONG) TabCtrlProc);
	SendMessage(hwnd, EM_SUBCLASSED, 0, 0);
}

void UnsubclassTabCtrl(HWND hwnd) {
	SendMessage(hwnd, EM_UNSUBCLASSED, 0, 0);
	SetWindowLong(hwnd, GWL_WNDPROC, (LONG) OldTabCtrlProc);
}

//LRESULT CALLBACK FrameProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
BOOL CALLBACK FrameProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {

		//case WM_CREATE:
		case WM_INITDIALOG:
			{
				WindowData *data = new WindowData;
				SetWindowLong(hwnd, GWL_USERDATA, (LONG)data);
				data->moving = false;

				RECT r;
				GetClientRect(hwnd, &r); 
				//tab_imagelist = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), IsWinVerXPPlus()? ILC_COLOR32 | ILC_MASK : ILC_COLOR16 | ILC_MASK, 0, 1);
				//ImageList_AddIcon(tab_imagelist, LoadSkinnedIcon(SKINICON_OTHER_MIRANDA));
				//HICON hIconMsg = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
				//ImageList_AddIcon(tab_imagelist, hIconMsg);

				//tab_ctrl = CreateWindow(WC_TABCONTROL, "", WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | TCS_MULTILINE | TCS_FOCUSNEVER, 0, 0, r.right, r.bottom, hwnd, NULL, NULL, NULL); 
				tab_ctrl = GetDlgItem(hwnd, IDC_TABS);
				//TabCtrl_SetImageList(tab_ctrl, tab_imagelist);
				
				TabCtrl_SetImageList(tab_ctrl, clist_imagelist);

				SubclassTabCtrl(tab_ctrl);
				active_window = 0;

				{
					HMENU hMenu = GetSystemMenu( hwnd, FALSE );
					InsertMenu( hMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
					if (options.ontop) {
						InsertMenu( hMenu, 0, MF_BYPOSITION | MF_ENABLED | MF_CHECKED | MF_STRING, IDM_TOPMOST, TranslateT("Always On Top"));
						SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
					} else {
						InsertMenu( hMenu, 0, MF_BYPOSITION | MF_ENABLED | MF_UNCHECKED | MF_STRING, IDM_TOPMOST, TranslateT("Always On Top"));
					}
				}

				SendDlgItemMessage(hwnd,IDC_CHAT_CLOSE, BUTTONSETASFLATBTN, 0, 0);
				SendDlgItemMessage(hwnd, IDC_CHAT_CLOSE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIcon(hInst, MAKEINTRESOURCE(IDI_CLOSE)));
				//SendMessage(GetDlgItem(hwnd,IDC_CHAT_CLOSE), BUTTONADDTOOLTIP, (WPARAM)Translate("Close current tab"), 0);
				SendMessage(hwnd, WM_SIZE, 0, 0);

				return FALSE;
			}

		case WM_SHOWWINDOW:
			if((BOOL)wParam == TRUE && first_show) {
				first_show = false;
				Utils_RestoreWindowPosition(hwnd, 0, MODULE, "MainWin");
			}
			break;

		case WM_SIZE:
			{
 				RECT rc; 

				// Calculate the display rectangle, assuming the 
				// tab control is the size of the client area. 
				GetClientRect(hwnd, &rc);

				// Size the tab control to fit the client area. 
				//SetWindowPos(tab_ctrl, NULL, 0, 5, rc.right + 3, rc.bottom + 3, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE); 
				rc.top += 2;
				rc.right += 2;
				rc.bottom += 2;

				MoveWindow(tab_ctrl, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, FALSE);

				TabCtrl_AdjustRect(tab_ctrl, FALSE, &rc); 
				// Position and size the static control to fit the 
				// tab control's display area, and make sure the 
				// static control is in front of the tab control. 
				if(active_window)
					SetWindowPos(active_window, HWND_TOP, rc.left - 3, rc.top - 1, rc.right - rc.left + 4, rc.bottom - rc.top + 2, 0);							

				GetClientRect(hwnd, &rc);
				SetWindowPos(GetDlgItem(hwnd,IDC_CHAT_CLOSE), tab_ctrl, rc.right - 19, 3, 16, 16, SWP_NOACTIVATE);
				//RedrawWindow(GetDlgItem(hwnd, IDC_CHAT_CLOSE), 0, 0, RDW_INVALIDATE);
			}

			break;

		case WM_DESTROY:
			//SendMessage(hwnd, WM_CLOSE, 0, 0);
			{
				TCITEM tci;
				int l, i;
				l = TabCtrl_GetItemCount(tab_ctrl);
				for (i = 0; i < l; i++) {
					tci.mask = TCIF_PARAM;
					TabCtrl_GetItem(tab_ctrl, i, &tci);
					delete (TabData *)tci.lParam;
				}
				UnsubclassTabCtrl(tab_ctrl);
				WindowData *window_data = (WindowData *)GetWindowLong(hwnd, GWL_USERDATA);
				delete window_data;

				ImageList_Destroy(clist_imagelist);
			}
			break;

		case WM_PREADDCHILD:
			{
				HWND child = (HWND)wParam;
				LONG style = GetWindowLong(child, GWL_STYLE);
				style &= ~(WS_THICKFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION);
				style |= WS_CHILD;
				SetWindowLong(child, GWL_STYLE, style);
				SetParent(child, hwnd);
			}
			return TRUE;
		case WM_ADDCHILD:
			{
				HWND child = (HWND)wParam;
				HANDLE hContact = (HANDLE)lParam;

				if(WindowList_Find(window_list, hContact) == 0) {
					WindowList_Add(window_list, child, hContact);

					TCHAR *contactName = (TCHAR *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hContact, (LPARAM)GCDNF_TCHAR);

					TCITEM tie; 
					tie.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM; 
					tie.iImage = options.tab_icon ? (int)CallService(MS_CLIST_GETCONTACTICON, (WPARAM)hContact, 0) : -1; 
					tie.pszText = contactName;
					TabData *td = new TabData;
					td->hContact = hContact;
					td->hwndMsg = child;
					tie.lParam = (LPARAM)td;

					int count = TabCtrl_GetItemCount(tab_ctrl);
					TabCtrl_InsertItem(tab_ctrl, count, &tie);
					TabCtrl_SetCurSel(tab_ctrl, count);
					SendMessage(hwnd, WMU_SETTAB, 0, 0);
					
					ShowWindow(hwnd, SW_SHOW);
					RedrawWindow(GetDlgItem(hwnd, IDC_CHAT_CLOSE), 0, 0, RDW_INVALIDATE);

				}
			}
			return TRUE;
		case WM_REMCHILD:
			{
				HWND child = (HWND)wParam;
				int l, i = GetTabFromHWND(tab_ctrl, child);

				TCITEM tie; 
				tie.mask = TCIF_PARAM;
				SendMessage(tab_ctrl, TCM_GETITEM, (WPARAM)i, (LPARAM)&tie);

				TabCtrl_DeleteItem(tab_ctrl, i);
				WindowList_Remove(window_list, child);

				delete (TabData *)tie.lParam;

				l = TabCtrl_GetItemCount(tab_ctrl);
				if(l == 0)
					SendMessage(hwnd, WM_CLOSE, 0, 0);
				else {
					if(i >= l) i = l - 1;
					TabCtrl_SetCurSel(tab_ctrl, i);
					SendMessage(hwnd, WMU_SETTAB, 0, 0);
				}
			}
			return TRUE;
		case WMU_SETTAB:
			{	
				int iPage = TabCtrl_GetCurSel(tab_ctrl); 
				TCHAR tbuff[128];
				
				TCITEM tie; 
				tie.pszText = tbuff;
				tie.cchTextMax = sizeof(tbuff) / sizeof(TCHAR);
				tie.mask = TCIF_PARAM | TCIF_TEXT | TCIF_IMAGE;
				SendMessage(tab_ctrl, TCM_GETITEM, (WPARAM)iPage, (LPARAM)&tie);
				SetWindowText(hwnd, tie.pszText);

				HWND child = ((TabData *)tie.lParam)->hwndMsg;
				HANDLE hContact = ((TabData *)tie.lParam)->hContact;
				if(active_window == child) return TRUE;

				/*
				// use GETCONTACTICON service instead of tie.iImage, because tie.iImage may be -1 depending on options
				HANDLE hIcon = ImageList_GetIcon(clist_imagelist, (int)CallService(MS_CLIST_GETCONTACTICON, (WPARAM)hContact, 0), 0);
				SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
				SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
				*/

				if(active_window) {
					SetWindowPos(active_window, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
					ShowWindow(active_window, SW_HIDE);
				}
				active_window = child;

				RECT rc;
				GetClientRect(hwnd, &rc);
				rc.top += 2;
				rc.right += 2;
				rc.bottom += 2;
				TabCtrl_AdjustRect(tab_ctrl, FALSE, &rc); 
				SetWindowPos(active_window, HWND_TOP, rc.left - 3, rc.top - 1, rc.right - rc.left + 4, rc.bottom - rc.top + 2, SWP_SHOWWINDOW);							

				tie.mask = TCIF_IMAGE;
				tie.iImage = options.tab_icon ? (int)CallService(MS_CLIST_GETCONTACTICON, (WPARAM)hContact, 0) : -1;
				SendMessage(tab_ctrl, TCM_SETITEM, (WPARAM)iPage, (LPARAM)&tie);
				SetFocus(active_window);

				SendMessage(hwnd, WMU_CALCWINICON, (WPARAM)hContact, 0);
				RedrawWindow(GetDlgItem(hwnd, IDC_CHAT_CLOSE), 0, 0, RDW_INVALIDATE);
			}
			return TRUE;
		case WM_NOTIFY: 
			{
				NMHDR *nmhdr = (NMHDR *)lParam;
				switch (nmhdr->code) { 
					case 0: // menu command processing 
						break;

					case TCN_SELCHANGE: 
						SendMessage(hwnd, WMU_SETTAB, 0, 0);
						break; 
				} 
			}
			break; 
		case WM_CLOSE:
			{
				Utils_SaveWindowPosition(hwnd, 0, MODULE, "MainWin");
				active_window = 0;
				SetFocus(tab_ctrl); // crash if focus is in typing window when srmm closed (EN_KILLFOCUS command, null dat)

				// clost windows in reverse order (for e.g. 'last contact')
				int l, i;
				HWND hwndMsg;
				TCITEM tci; 
				l = TabCtrl_GetItemCount(tab_ctrl);
				for (i = l - 1; i >= 0; --i) {
					tci.mask = TCIF_PARAM;
					TabCtrl_GetItem(tab_ctrl, i, &tci);
					hwndMsg = ((TabData *)tci.lParam)->hwndMsg;

					SendMessage(hwndMsg, WM_CLOSE, 0, 0);
				}
				//WindowList_Broadcast(window_list, WM_CLOSE, 0, 0);

				ShowWindow(hwnd, SW_HIDE);
			}
			return TRUE;
		case WM_HIGHLIGHTCONTACTWND:
			{
				HANDLE hContact = (HANDLE)wParam;
				HWND child = WindowList_Find(window_list, hContact);
				if(child) {
					int t = GetTabFromHWND(tab_ctrl, child);
					if(IsIconic(hwnd) || TabCtrl_GetCurSel(tab_ctrl) != t) {
						//TabCtrl_HighlightItem(tab_ctrl, t, 1);
						TCITEM tie; 
						tie.mask = TCIF_IMAGE;
						tie.iImage = message_icon_index;
						SendMessage(tab_ctrl, TCM_SETITEM, (WPARAM)t, (LPARAM)&tie);
					}
					SendMessage(hwnd, WMU_CALCWINICON, 0, 0);
					SetForegroundWindow(hwnd);
				}
			}
			return TRUE;
		case WM_SHOWCONTACTWND:
			{
				HANDLE hContact = (HANDLE)wParam;
				HWND child = WindowList_Find(window_list, hContact);
				if(child) {
					int t = GetTabFromHWND(tab_ctrl, child);
					TabCtrl_SetCurSel(tab_ctrl, t);
					SendMessage(hwnd, WMU_SETTAB, 0, 0);
					ShowWindow(hwnd, SW_SHOWNORMAL);
				}
			}
			return TRUE;
		case WM_MOUSEMOVE:
			if(wParam & MK_LBUTTON) {
				SetCapture(hwnd);

				POINT newp;
				newp.x = (short)LOWORD(lParam);
				newp.y = (short)HIWORD(lParam);

				ClientToScreen(hwnd, &newp);

				WindowData *window_data = (WindowData *)GetWindowLong(hwnd, GWL_USERDATA);
				if(!window_data->moving) {
					window_data->moving = true;
				} else {
					RECT r;
					GetWindowRect(hwnd, &r);
					
					SetWindowPos(hwnd, 0, r.left + (newp.x - window_data->p.x), r.top + (newp.y - window_data->p.y), 0, 0, SWP_NOSIZE | SWP_NOZORDER);
				}
				window_data->p.x = newp.x;
				window_data->p.y = newp.y;			
			} else {
				ReleaseCapture();
				WindowData *window_data = (WindowData *)GetWindowLong(hwnd, GWL_USERDATA);
				window_data->moving = false;
			}
			return TRUE;
		case WM_RESETTABICONS:
			{
				HANDLE hContact = (HANDLE)wParam;
				TCITEM tci;
				if(hContact) {
					// wParam contains clist image list icon index from contact icon changed event - but that could be e.g. flashing online notification
					// so we get the contact icon in ici, which is just the proto icon
					int t = GetTabFromContact(tab_ctrl, hContact), ici = (int)CallService(MS_CLIST_GETCONTACTICON, (WPARAM)hContact, 0);
					if(t != -1) {
						tci.mask = TCIF_IMAGE;
						tci.iImage = options.tab_icon ? ici : -1;
						SendMessage(tab_ctrl, TCM_SETITEM, (WPARAM)t, (LPARAM)&tci);

						/*
						if(t == TabCtrl_GetCurSel(tab_ctrl)) {
							// use GETCONTACTICON service instead of tie.iImage, because tie.iImage may be -1 depending on options
							HANDLE hIcon = ImageList_GetIcon(clist_imagelist, ici, 0);
							SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
							SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
						}
						*/
						SendMessage(hwnd, WMU_CALCWINICON, 0, 0);
					}
				} else {
					int l, i;
					HANDLE hContact;
					l = TabCtrl_GetItemCount(tab_ctrl);
					for (i = 0; i < l; i++) {
						tci.mask = TCIF_PARAM;
						TabCtrl_GetItem(tab_ctrl, i, &tci);
						hContact = ((TabData *)tci.lParam)->hContact;

						tci.mask = TCIF_IMAGE;
						tci.iImage = options.tab_icon ? (int)CallService(MS_CLIST_GETCONTACTICON, (WPARAM)hContact, 0) : -1;
						SendMessage(tab_ctrl, TCM_SETITEM, (WPARAM)i, (LPARAM)&tci);
					}
				}
			}
			return TRUE;
		case WMU_CALCWINICON:
			{
				int l, i, s;
				TCITEM tci;
				HANDLE hContact = 0;
				if(wParam) hContact = (HANDLE)wParam;
				bool use_message_icon = false;
				l = TabCtrl_GetItemCount(tab_ctrl);
				s = TabCtrl_GetCurSel(tab_ctrl);
				for (i = 0; i < l; i++) {
					tci.mask = TCIF_IMAGE | TCIF_PARAM;
					TabCtrl_GetItem(tab_ctrl, i, &tci);
					if(tci.iImage == message_icon_index)
						use_message_icon = true;
					else if(i == s) {
						hContact = ((TabData *)tci.lParam)->hContact;
					}
				}

				int ici = (use_message_icon ? message_icon_index : (int)CallService(MS_CLIST_GETCONTACTICON, (WPARAM)hContact, 0));
				HANDLE hIcon = ImageList_GetIcon(clist_imagelist, ici, 0);
				SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
				SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
			}
			return TRUE;
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_ACTIVE) {
				TCITEM tci;
				int s = TabCtrl_GetCurSel(tab_ctrl);
				if(s != -1) {
					tci.mask = TCIF_IMAGE | TCIF_PARAM;
					TabCtrl_GetItem(tab_ctrl, s, &tci);
					HANDLE hContact = ((TabData *)tci.lParam)->hContact;
					SendMessage(hwnd, WM_RESETTABICONS, (WPARAM)hContact, lParam);
				}
			}
			break;
		case WM_SYSCOMMAND:
			if (wParam == IDM_TOPMOST) {
				HMENU hMenu = GetSystemMenu(hwnd, FALSE);
				CheckMenuItem(hMenu, IDM_TOPMOST, MF_BYCOMMAND | (options.ontop ? MF_UNCHECKED : MF_CHECKED));
				SetWindowPos(hwnd, (options.ontop ? HWND_NOTOPMOST : HWND_TOPMOST), 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
				options.ontop = !options.ontop;
				DBWriteContactSettingByte(0, MODULE, "OnTop", options.ontop ? 1 : 0);
				return TRUE;
			}
			break;
		case WM_COMMAND:
			if ( HIWORD( wParam ) == BN_CLICKED ) {
				switch( LOWORD( wParam )) {
					case IDC_CHAT_CLOSE:
						{
							TCITEM tci;
							int s = TabCtrl_GetCurSel(tab_ctrl);
							if(s != -1) {
								tci.mask = TCIF_PARAM;
								TabCtrl_GetItem(tab_ctrl, s, &tci);
								SendMessage(((TabData *)tci.lParam)->hwndMsg, WM_CLOSE, 0, 0);
							}
						}
						return TRUE;
				}
			}
			break;
		case WM_FONTRELOAD:
			//LOGFONT lf;
		    //SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, FALSE);
			//lf.lfHeight = -12;
			//HFONT hFont = CreateFontIndirect(&lf);
			SendMessage(tab_ctrl, WM_SETFONT, (WPARAM)hFontTabs, MAKELPARAM(TRUE, 0));
			//RedrawWindow(hwnd, 0, 0, RDW_INVALIDATE);
			SendMessage(hwnd, WM_SIZE, 0, 0);
			return TRUE;
	}

	//return DefWindowProc(hwnd, msg, wParam, lParam);
	return 0;
}

int CreateFrame(HWND parent, HINSTANCE hInst) {
	InitCommonControls();

	/*
	WNDCLASS wndclass;
	wndclass.style         = 0; //CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = FrameProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = hInst;
	wndclass.hIcon         = NULL;
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)(COLOR_3DFACE+1);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = _T("TabFrame");
	RegisterClass(&wndclass);
	*/

	window_list = (HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0);
	
	//clist_imagelist = (HIMAGELIST)CallService(MS_CLIST_GETICONSIMAGELIST, 0, 0);
	HIMAGELIST iml = (HIMAGELIST)CallService(MS_CLIST_GETICONSIMAGELIST, 0, 0);
	clist_imagelist = ImageList_Duplicate(iml);
	message_icon_index = ImageList_ReplaceIcon(clist_imagelist, 0, LoadSkinnedIcon(SKINICON_EVENT_MESSAGE));

	/*
	pluginwind = CreateWindowEx(WS_EX_APPWINDOW, _T("TabFrame"),"", 
		0,//options.show_titlebar ? STYLE_TITLE : STYLE_NOTITLE, 
		0, 0, 800, 600, parent, NULL, hInst, NULL);
	*/
	pluginwind = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_CONTAINER), 0, FrameProc, 0);
	EnableTitleBar();

	FixWindowStyle();

	return 0;
}

BOOL CALLBACK TabCtrlProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TabCtrlData *dat;
	dat = (TabCtrlData *) GetWindowLong(hwnd, GWL_USERDATA);
    switch(msg) {
    	case EM_SUBCLASSED:
			dat = (TabCtrlData *) mir_alloc(sizeof(TabCtrlData));
			SetWindowLong(hwnd, GWL_USERDATA, (LONG) dat);
			dat->bDragging = FALSE;
	        return 0;
        case WM_MBUTTONDOWN:
		{
			TCITEM tci;
			int tabId;
			TabData *mwtd;
			TCHITTESTINFO thinfo;
			thinfo.pt.x = (lParam<<16)>>16;
			thinfo.pt.y = lParam>>16;
			tabId = TabCtrl_HitTest(hwnd, &thinfo);
			if (tabId >= 0) {
				tci.mask = TCIF_PARAM;
				TabCtrl_GetItem(hwnd, tabId, &tci);
				mwtd = (TabData *) tci.lParam;
				if (mwtd) SendMessage(mwtd->hwndMsg, WM_CLOSE, 0, 0);
			}
	        return 0;
        }
		case WM_LBUTTONDBLCLK:
		{
			TCITEM tci;
			TCHITTESTINFO thinfo;
			int tabId;
			TabData *mwtd;
			thinfo.pt.x = (lParam<<16)>>16;
			thinfo.pt.y = lParam>>16;
			tabId = TabCtrl_HitTest(hwnd, &thinfo);
			if (tabId >=0 ) {
				tci.mask = TCIF_PARAM;
				TabCtrl_GetItem(hwnd, tabId, &tci);
				mwtd = (TabData *) tci.lParam;
				if (mwtd) SendMessage(mwtd->hwndMsg, WM_CLOSE, 0, 0);
			}
			dat->lastClickTab = -1;//Child = NULL;
		}
		break;
		case WM_LBUTTONDOWN:
		{
			if (!dat->bDragging) {
				FILETIME ft;
				TCHITTESTINFO thinfo;
				GetSystemTimeAsFileTime(&ft);
				thinfo.pt.x = (lParam<<16)>>16;
				thinfo.pt.y = lParam>>16;
				dat->srcTab = dat->destTab = TabCtrl_HitTest(hwnd, &thinfo);
				if (dat->srcTab >=0 ) {
					dat->lastClickTab = dat->srcTab; //Child = GetChildFromTab(hwnd, dat->srcTab)->hwnd;
				} else {
					dat->lastClickTab = -1;//Child = NULL;
				}
				dat->bDragging = TRUE;
				dat->bDragged = FALSE;
				dat->clickLParam = lParam;
				dat->clickWParam = wParam;
				dat->lastClickTime = ft.dwLowDateTime;
				dat->mouseLBDownPos.x = thinfo.pt.x;
				dat->mouseLBDownPos.y = thinfo.pt.y;
				SetCapture(hwnd);
				return 0;
			}
		}
		break;
		case WM_CAPTURECHANGED:
		case WM_LBUTTONUP:
			if (dat->bDragging) {
				TCHITTESTINFO thinfo;
				thinfo.pt.x = (lParam<<16)>>16;
				thinfo.pt.y = lParam>>16;
				if (dat->bDragged) {
					ImageList_DragLeave(GetDesktopWindow());
					ImageList_EndDrag();
					ImageList_Destroy(dat->hDragImageList);
					SetCursor(LoadCursor(NULL, IDC_ARROW));
					dat->destTab = TabCtrl_HitTest(hwnd, &thinfo);
					if (thinfo.flags != TCHT_NOWHERE && dat->destTab != dat->srcTab)  {
						NMHDR nmh;
						TCHAR  sBuffer[501];
						TCITEM item;
						int curSel;
						curSel = TabCtrl_GetCurSel(hwnd);
						item.mask = TCIF_IMAGE | TCIF_PARAM | TCIF_TEXT;
						item.pszText = sBuffer;
						item.cchTextMax = sizeof(sBuffer)/sizeof(TCHAR);
						TabCtrl_GetItem(hwnd, dat->srcTab, &item);
						sBuffer[sizeof(sBuffer)/sizeof(TCHAR)-1] = '\0';
						if (curSel == dat->srcTab) {
							curSel = dat->destTab;
						} else {
							if (curSel > dat->srcTab && curSel <= dat->destTab) {
								curSel--;
							} else if (curSel < dat->srcTab && curSel >= dat->destTab) {
								curSel++;
							}
						}
						TabCtrl_DeleteItem(hwnd, dat->srcTab);
						TabCtrl_InsertItem(hwnd, dat->destTab, &item );
						TabCtrl_SetCurSel(hwnd, curSel);
						nmh.hwndFrom = hwnd;
						nmh.idFrom = GetDlgCtrlID(hwnd);
						nmh.code = TCN_SELCHANGE;
						SendMessage(GetParent(hwnd), WM_NOTIFY, nmh.idFrom, (LPARAM)&nmh);
						UpdateWindow(hwnd);
					} else if (GetKeyState(VK_CONTROL) & 0x8000) {
						/*
						MessageWindowTabData *mwtd;
						TCITEM tci;
						POINT pt;
						NewMessageWindowLParam newData = { 0 };
						tci.mask = TCIF_PARAM;
						TabCtrl_GetItem(hwnd, dat->srcTab, &tci);
						mwtd = (MessageWindowTabData *) tci.lParam;
						if (mwtd != NULL) {
							HWND hChild = mwtd->hwnd;
							HANDLE hContact = mwtd->hContact;
							HWND hParent;
							GetCursorPos(&pt);
							hParent = WindowFromPoint(pt);
							while (GetParent(hParent) != NULL) {
								hParent = GetParent(hParent);
							}
							hParent = WindowList_Find(g_dat->hParentWindowList, hParent);
							if ((hParent != NULL && hParent != GetParent(hwnd)) || (hParent == NULL && mwtd->parent->childrenCount > 1)) {
								if (hParent == NULL) {
									MONITORINFO mi;
									HMONITOR hMonitor;
									RECT rc, rcDesktop;
									newData.hContact = hContact;
									hParent = (HWND)CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_MSGWIN), NULL, DlgProcParentWindow, (LPARAM) & newData);
									GetWindowRect(hParent, &rc);
									rc.right = (rc.right - rc.left);
									rc.bottom = (rc.bottom - rc.top);
									rc.left = pt.x - rc.right / 2;
									rc.top = pt.y - rc.bottom / 2;
									hMonitor = MonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST);
									mi.cbSize = sizeof(mi);
									GetMonitorInfo(hMonitor, &mi);
									rcDesktop = mi.rcWork;
									if (rc.left < rcDesktop.left) {
										rc.left = rcDesktop.left;
									}
									if (rc.top < rcDesktop.top) {
										rc.top = rcDesktop.top;
									}
									MoveWindow(hParent, rc.left, rc.top, rc.right, rc.bottom, FALSE);

								}
								SetParent(hChild, hParent);
								SendMessage(GetParent(hwnd), CM_REMOVECHILD, 0, (LPARAM) hChild);
								SendMessage(hChild, DM_SETPARENT, 0, (LPARAM) hParent);
								SendMessage(hParent, CM_ADDCHILD, (WPARAM)hChild, (LPARAM) hContact);
								SendMessage(hParent, CM_ACTIVATECHILD, 0, (LPARAM) hChild);
								NotifyLocalWinEvent(hContact, hChild, MSG_WINDOW_EVT_CLOSING);
								NotifyLocalWinEvent(hContact, hChild, MSG_WINDOW_EVT_CLOSE);
								NotifyLocalWinEvent(hContact, hChild, MSG_WINDOW_EVT_OPENING);
								NotifyLocalWinEvent(hContact, hChild, MSG_WINDOW_EVT_OPEN);
								ShowWindow(hParent, SW_SHOWNA);
							}
						}
						*/
					}
				} else {
					SendMessage(hwnd, WM_LBUTTONDOWN, dat->clickWParam, dat->clickLParam);
				}
				dat->bDragged = FALSE;
				dat->bDragging = FALSE;
				ReleaseCapture();
			}
			break;
		case WM_MOUSEMOVE:
			if (!(wParam & MK_LBUTTON)) break;
			if (dat->bDragging) {
				FILETIME ft;
				TCHITTESTINFO thinfo;
				GetSystemTimeAsFileTime(&ft);
				thinfo.pt.x = (lParam<<16)>>16;
				thinfo.pt.y = lParam>>16;
				if (!dat->bDragged) {
					if ((abs(thinfo.pt.x-dat->mouseLBDownPos.x)<3 && abs(thinfo.pt.y-dat->mouseLBDownPos.y)<3)
						|| (ft.dwLowDateTime - dat->lastClickTime) < 10*1000*150)
						break;
				}
				if (!dat->bDragged) {
					POINT pt;
					RECT rect;
					RECT rect2;
					HDC hDC, hMemDC;
					HBITMAP hBitmap, hOldBitmap;
					HBRUSH hBrush = CreateSolidBrush(RGB(255,0,254));
					GetCursorPos(&pt);
					TabCtrl_GetItemRect(hwnd, dat->srcTab, &rect);
					rect.right -= rect.left-1;
					rect.bottom -= rect.top-1;
					rect2.left = 0; rect2.right = rect.right; rect2.top = 0; rect2.bottom = rect.bottom;
					dat->hDragImageList = ImageList_Create(rect.right, rect.bottom, ILC_COLOR | ILC_MASK, 0, 1);
					hDC = GetDC(hwnd);
					hMemDC = CreateCompatibleDC(hDC);
					hBitmap = CreateCompatibleBitmap(hDC, rect.right, rect.bottom);
					hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);
					FillRect(hMemDC, &rect2, hBrush);
					SetWindowOrgEx (hMemDC, rect.left, rect.top, NULL);
					SendMessage(hwnd, WM_PRINTCLIENT, (WPARAM)hMemDC, PRF_CLIENT);
					SelectObject(hMemDC, hOldBitmap);
					ImageList_AddMasked(dat->hDragImageList, hBitmap, RGB(255,0,254));
					DeleteObject(hBitmap);
					DeleteObject(hBrush);
					ReleaseDC(hwnd, hDC);
					DeleteDC(hMemDC);
					ImageList_BeginDrag(dat->hDragImageList, 0, dat->mouseLBDownPos.x - rect.left, dat->mouseLBDownPos.y - rect.top);
					ImageList_DragEnter(GetDesktopWindow(), pt.x, pt.y);
					SetCursor(LoadCursor(0, IDC_HAND));
					dat->mouseLBDownPos.x = thinfo.pt.x;
					dat->mouseLBDownPos.y = thinfo.pt.y;
				} else {
					POINT pt;
					GetCursorPos(&pt);
					ImageList_DragMove(pt.x, pt.y);
				}
				dat->bDragged = TRUE;
				return 0;
			}
			break;
       	case EM_UNSUBCLASSED:
			mir_free(dat);
			return 0;
	}
	return CallWindowProc(OldTabCtrlProc, hwnd, msg, wParam, lParam);
}

void FixWindowStyle() {
	if(pluginwind) SetWindowPos(pluginwind, (options.ontop ? HWND_TOPMOST : HWND_NOTOPMOST), 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);	
}
