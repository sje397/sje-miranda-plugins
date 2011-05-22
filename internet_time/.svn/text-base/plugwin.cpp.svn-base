#include "stdafx.h"
#include "plugwin.h"

extern HINSTANCE hInst;
HFONT TitleBarFont;

HANDLE   mainThread;

HBRUSH tbrush = 0;

HANDLE mainMenuItem, hIconsChangedEvent, hDBChange;

HANDLE upIcon = 0, downIcon = 0;

ITEMLIST listbox_items, temp_listbox_items;

TCHAR format_string[512], date_format_string[512];

int Frameid=-1;
HWND label;

HWND pluginwind = 0, hwnd_clist = 0;

bool hook_window_behaviour_to_clist = true;

static HMODULE hUserDll;
BOOL (WINAPI *MySetLayeredWindowAttributes)(HWND,COLORREF,BYTE,DWORD);
BOOL (WINAPI *MyAnimateWindow)(HWND hWnd,DWORD dwTime,DWORD dwFlags);

#define TM_AUTOALPHA  1
static int transparentFocus=1;

REG_TZI ce_reg_tzi;

static int CLUILoadTitleBarFont()
{
	char facename[]="MS Shell Dlg";
	HFONT hfont;
	LOGFONT logfont;
	memset(&logfont,0,sizeof(logfont));
	memcpy(logfont.lfFaceName,facename,sizeof(facename));
	logfont.lfWeight=FW_NORMAL;
	logfont.lfHeight=-10;
	hfont=CreateFontIndirect(&logfont);
	return((int)hfont);
}

void set_minmax(bool mm) {
	hook_window_behaviour_to_clist = mm;
	if(hook_window_behaviour_to_clist) {
		BYTE state = DBGetContactSettingByte(NULL, "CList", "Setting", SETTING_STATE_NORMAL);

		if(pluginwind) {
			if(state == SETTING_STATE_NORMAL)
				ShowWindow(pluginwind, SW_SHOW);
			else
				ShowWindow(pluginwind, SW_HIDE);
		}
	}
}

void CALLBACK plug1TimerProc2(
  HWND hwnd,         // handle to window
  UINT uMsg,         // WM_TIMER message
  UINT idEvent,  // timer identifier
  DWORD dwTime       // current system time
)
{
	if(pluginwind) {
		InvalidateRect(pluginwind, 0, FALSE);
		//ValidateRect(pluginwind, 0);
	}
}

void CALLBACK plug1TimerProc3(
  HWND hwnd,         // handle to window
  UINT uMsg,         // WM_TIMER message
  UINT idEvent,  // timer identifier
  DWORD dwTime       // current system time
)
{
	if(pluginwind && hwnd_clist && hook_window_behaviour_to_clist) {
		if(IsWindowVisible(pluginwind) != IsWindowVisible(hwnd_clist))
			ShowWindow(pluginwind, IsWindowVisible(hwnd_clist) ? SW_SHOW : SW_HIDE);
	}
}

LRESULT CALLBACK mypluginwindow1proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT rect;
	MEASUREITEMSTRUCT *mis;
	LPDRAWITEMSTRUCT dis;
	SIZE textSize;
	RECT r;

	switch(msg)
	{
		
	case WM_MEASUREITEM:
		mis = (MEASUREITEMSTRUCT *)lParam;
		mis->itemWidth = 100;
		mis->itemHeight = 20;
		return 0;
		break;
		
	case WM_DRAWITEM:
		dis = (LPDRAWITEMSTRUCT)lParam;
		//pItemData = (LISTITEM *)dis->itemData;
		if(dis->hwndItem == label) {
			if(dis->itemID != -1) {
				COLORREF tcol;
				HBRUSH ttbrush = 0;

				if(dis->itemState & ODS_SELECTED && dis->itemState & ODS_FOCUS) {
					tcol = DBGetContactSettingDword(NULL,"CLC","SelBkColour", CLCDEFAULT_SELBKCOLOUR);
					SetBkColor(dis->hDC, tcol);

					FillRect(dis->hDC, &dis->rcItem, (ttbrush = CreateSolidBrush(tcol)));
					
					//SendMessage(label, LB_GETITEMRECT, (WPARAM)dis->itemID, (LPARAM)&r);
					//FillRect(dis->hDC, &r, (ttbrush = CreateSolidBrush(tcol)));
					
					tcol = DBGetContactSettingDword(NULL,"CLC","SelTextColour", CLCDEFAULT_SELTEXTCOLOUR);
					SetTextColor(dis->hDC, tcol);
				} else {
					tcol = DBGetContactSettingDword(NULL,"CLC","BkColour", CLCDEFAULT_BKCOLOUR);
					SetBkColor(dis->hDC, tcol);
					
					FillRect(dis->hDC, &dis->rcItem, (ttbrush = CreateSolidBrush(tcol)));
					
					//SendMessage(label, LB_GETITEMRECT, (WPARAM)dis->itemID, (LPARAM)&r);
					//FillRect(dis->hDC, &r, (ttbrush = CreateSolidBrush(tcol)));

					tcol = DBGetContactSettingDword(NULL,"CLC","Font0Col", CLCDEFAULT_TEXTCOLOUR);					
					SetTextColor(dis->hDC, tcol);
				}				
				
				SetBkMode(dis->hDC, TRANSPARENT);
				{
					SYSTEMTIME st, other_st;
					TCHAR buf[512] = "";
					MyGetSystemTime(&st);
					

					TIME_ZONE_INFORMATION tzi;
					//get TZI for GMT + 1 (i.e. Central Europe Standard Time)
					tzi.Bias = ce_reg_tzi.Bias;
					tzi.DaylightBias = ce_reg_tzi.DaylightBias;
					tzi.DaylightDate = ce_reg_tzi.DaylightDate;
					tzi.StandardBias = ce_reg_tzi.StandardBias;
					tzi.StandardDate = ce_reg_tzi.StandardDate;

					// disable daylight savings
					tzi.StandardDate.wMonth = 0;

					MySystemTimeToTzSpecificLocalTime(&tzi, &st, &other_st);

					double beats = (other_st.wHour + other_st.wMinute / 60.0 + other_st.wSecond / 60.0 / 60.0 + other_st.wMilliseconds / 1000.0 / 60.0 / 60.0) / 24.0 * 1000.0;

					// doesn't print leading zeros!!
					//sprintf(buf, "@%03.2f", beats);

					buf[0] = '@';
					int t = (int)(beats / 100);
					beats -= t * 100;
					buf[1] = '0' + t;
					t = (int)(beats / 10);
					beats -= t * 10;
					buf[2] = '0' + t;
					t = (int)beats;
					beats -= t;
					buf[3] = '0' + t;
					buf[4] = '.';
					t = (int)(beats / 0.1);
					beats -= t * 0.1;
					buf[5] = '0' + t;
					t = (int)(beats / 0.01);
					buf[6] = '0' + t;
					buf[7] = '\0';

					GetTextExtentPoint32(dis->hDC,"Internet Time",lstrlen("Internet Time"),&textSize);
					TextOut(dis->hDC,dis->rcItem.left,(dis->rcItem.top+dis->rcItem.bottom-textSize.cy)>>1,"Internet Time",lstrlen("Internet Time"));

					GetTextExtentPoint32(dis->hDC,buf,lstrlen(buf),&textSize);
					TextOut(dis->hDC,dis->rcItem.right - textSize.cx - 2,(dis->rcItem.top+dis->rcItem.bottom-textSize.cy)>>1,buf,lstrlen(buf));

				}

				SetBkMode(dis->hDC, OPAQUE);

				if(ttbrush) DeleteObject(ttbrush);
			}
		}
		return 0;
		break;
	case WM_CTLCOLORLISTBOX:
		{

			if(tbrush) DeleteObject(tbrush);

			return (BOOL)(tbrush = CreateSolidBrush((COLORREF)DBGetContactSettingDword(NULL,"CLC","BkColour", CLCDEFAULT_BKCOLOUR)));
		}
		break;
	case WM_ERASEBKGND: 
//		{
//			HDC hDC = (HDC)wParam;
//			HBRUSH tempBrush = CreateSolidBrush((COLORREF)DBGetContactSettingDword(NULL,"CLC","BkColour", CLCDEFAULT_BKCOLOUR));
//			GetClientRect(hwnd, &r);
//			FillRect(hDC, &r, tempBrush);
//			DeleteObject(tempBrush);
//		}
		//return DefWindowProc(hwnd, msg, wParam, lParam);
		return TRUE;


	case WM_DELETEITEM:
		// we don't need to delete stuff - it's handled already
		return 0;
		break;
		
	case WM_SYSCOLORCHANGE:
		SendMessage(label,msg,wParam,lParam);
		break;

	case WM_CREATE:
		GetClientRect(hwnd,&r);	
		//label=CreateWindow("LISTBOX", "", (WS_VISIBLE | WS_CHILD | LBS_STANDARD | LBS_OWNERDRAWFIXED | LBS_NOTIFY) & ~WS_BORDER, r.left, r.top, r.right - r.left, r.bottom - r.top, hwnd, NULL, hInst,0);
		label=CreateWindow("LISTBOX", "", (WS_VISIBLE | WS_CHILD | LBS_NOINTEGRALHEIGHT| LBS_STANDARD | WS_CLIPCHILDREN | LBS_OWNERDRAWFIXED | LBS_NOTIFY) & ~WS_BORDER, r.left, r.top, r.right - r.left, r.bottom - r.top, hwnd, NULL, hInst,0);

		SendMessage(label, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)"Internet Time");

		if (DBGetContactSettingByte(NULL,"CList","Transparent",0))
		{
			if(ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) {

			} else {
				SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
				if (MySetLayeredWindowAttributes) MySetLayeredWindowAttributes(hwnd, RGB(0,0,0), (BYTE)DBGetContactSettingByte(NULL,"CList","Alpha",SETTING_ALPHA_DEFAULT), LWA_ALPHA);
			}
			//if (MySetLayeredWindowAttributes) MySetLayeredWindowAttributes(hwnd, RGB(0,0,0), 255, LWA_ALPHA);		
		}

		{	
			//CreateWindow("button","button1",WS_VISIBLE|WS_CHILD	,120,4,60,22,hwnd,NULL,hInst,0);
			//label=CreateWindow("LISTBOX", "", (WS_VISIBLE | WS_CHILD | LBS_STANDARD | LBS_OWNERDRAWFIXED | LBS_NOTIFY) & ~WS_BORDER, 2, 2, 120, 60, hwnd, NULL, hInst,0);
			
			// 400 ms is about 1/2 of 1/100 of a beat (1/100 of a beat is our display resolution)
			SetTimer(hwnd,TIMER_ID2, 432, plug1TimerProc2);
			SetTimer(hwnd,TIMER_ID3, 1000, plug1TimerProc3);
			return(0);

		};


	case WM_ACTIVATE:
		if(wParam==WA_INACTIVE) {
			if((HWND)wParam!=hwnd)
				if(DBGetContactSettingByte(NULL,"CList","Transparent",SETTING_TRANSPARENT_DEFAULT))
					if(transparentFocus)
						SetTimer(hwnd, TM_AUTOALPHA,250,NULL);
		}
		else {
			if(DBGetContactSettingByte(NULL,"CList","Transparent",SETTING_TRANSPARENT_DEFAULT)) {
				KillTimer(hwnd,TM_AUTOALPHA);
				if (MySetLayeredWindowAttributes) MySetLayeredWindowAttributes(hwnd, RGB(0,0,0), (BYTE)DBGetContactSettingByte(NULL,"CList","Alpha",SETTING_ALPHA_DEFAULT), LWA_ALPHA);
				transparentFocus=1;
			}
		}
		return DefWindowProc(hwnd,msg,wParam,lParam);

	case WM_SETCURSOR:
		if(DBGetContactSettingByte(NULL,"CList","Transparent",SETTING_TRANSPARENT_DEFAULT)) {
			if (!transparentFocus && GetForegroundWindow()!=hwnd && MySetLayeredWindowAttributes) {
				MySetLayeredWindowAttributes(hwnd, RGB(0,0,0), (BYTE)DBGetContactSettingByte(NULL,"CList","Alpha",SETTING_ALPHA_DEFAULT), LWA_ALPHA);
				transparentFocus=1;
				SetTimer(hwnd, TM_AUTOALPHA,250,NULL);
			}
		}
		return DefWindowProc(hwnd,msg,wParam,lParam);

	case WM_TIMER:
		if ((int)wParam==TM_AUTOALPHA)
		{	int inwnd;
			
			if (GetForegroundWindow()==hwnd) {
				KillTimer(hwnd,TM_AUTOALPHA);
				inwnd=1;
			}
			else {
				POINT pt;
				HWND hwndPt;
				pt.x=(short)LOWORD(GetMessagePos());
				pt.y=(short)HIWORD(GetMessagePos());
				hwndPt=WindowFromPoint(pt);
				inwnd=(hwndPt==hwnd || GetParent(hwndPt)==hwnd);
			}
			if (inwnd!=transparentFocus && MySetLayeredWindowAttributes)
			{ //change
				transparentFocus=inwnd;
				if(transparentFocus) MySetLayeredWindowAttributes(hwnd, RGB(0,0,0), (BYTE)DBGetContactSettingByte(NULL,"CList","Alpha",SETTING_ALPHA_DEFAULT), LWA_ALPHA);
				else MySetLayeredWindowAttributes(hwnd, RGB(0,0,0), (BYTE)DBGetContactSettingByte(NULL,"CList","AutoAlpha",SETTING_AUTOALPHA_DEFAULT), LWA_ALPHA);
			}
			if(!transparentFocus) KillTimer(hwnd,TM_AUTOALPHA);
		}
		return TRUE;

	case WM_SHOWWINDOW:
	{	static int noRecurse=0;
		if(lParam) break;
		if(noRecurse) break;
		if(!DBGetContactSettingByte(NULL,"CLUI","FadeInOut",0) || !IsWinVer2000Plus()) break;
		if(GetWindowLong(hwnd,GWL_EXSTYLE)&WS_EX_LAYERED) {
			DWORD thisTick,startTick;
			int sourceAlpha,destAlpha;
			if(wParam) {
				sourceAlpha=0;
				destAlpha=(BYTE)DBGetContactSettingByte(NULL,"CList","Alpha",SETTING_AUTOALPHA_DEFAULT);
				MySetLayeredWindowAttributes(hwnd, RGB(0,0,0), 0, LWA_ALPHA);
				noRecurse=1;
				ShowWindow(hwnd,SW_SHOW);
				noRecurse=0;
			}
			else {
				sourceAlpha=(BYTE)DBGetContactSettingByte(NULL,"CList","Alpha",SETTING_AUTOALPHA_DEFAULT);
				destAlpha=0;
			}
			for(startTick=GetTickCount();;) {
				thisTick=GetTickCount();
				if(thisTick>=startTick+200) break;
				MySetLayeredWindowAttributes(hwnd, RGB(0,0,0), (BYTE)(sourceAlpha+(destAlpha-sourceAlpha)*(int)(thisTick-startTick)/200), LWA_ALPHA);
			}
			MySetLayeredWindowAttributes(hwnd, RGB(0,0,0), (BYTE)destAlpha, LWA_ALPHA);
		}
		else {
//			if(wParam) SetForegroundWindow(hwnd);
			MyAnimateWindow(hwnd,200,AW_BLEND|(wParam?0:AW_HIDE));
			//SetWindowPos(label,0,0,0,0,0,SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);
		}
		//int res = DefWindowProc(hwnd, msg, wParam, lParam);
		//return res;
		DBWriteContactSettingByte(NULL, "InternetTime", "WindowVisible", wParam ? 1 : 0);
		break;
		//return FALSE; //break;
	}

	case WM_SIZE:
		//PostMessage(label, WM_SIZE, wParam, lParam);
		
		GetClientRect(hwnd,&rect);	
		SetWindowPos(label, 0, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, SWP_NOZORDER);
		InvalidateRect(hwnd, &rect, FALSE);
		return 0;


	case WM_DESTROY:
		if(tbrush) DeleteObject(tbrush);

		if(!ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) {
			GetWindowRect(hwnd, &r);

			DBWriteContactSettingDword(NULL, "InternetTime", "WindowX", r.left);
			DBWriteContactSettingDword(NULL, "InternetTime", "WindowY", r.top);
			DBWriteContactSettingDword(NULL, "InternetTime", "WindowWidth", r.right - r.left);
			DBWriteContactSettingDword(NULL, "InternetTime", "WindowHeight", r.bottom - r.top);
		}

		KillTimer(hwnd,TIMER_ID2);
		DestroyWindow(label);

		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_CLOSE:
		if(!ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) {
			if(!hook_window_behaviour_to_clist)
				ShowWindow(hwnd, SW_HIDE);
			return 0;
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);

	default:
			return DefWindowProc(hwnd, msg, wParam, lParam);

	};
	
	return(TRUE);
};

static int PlugShowWindow(WPARAM wParam, LPARAM lParam) {
	if(pluginwind && !hook_window_behaviour_to_clist) {
		ShowWindow(pluginwind, IsWindowVisible(pluginwind) ? SW_HIDE : SW_SHOW);
		DBWriteContactSettingByte(NULL, "InternetTime", "WindowVisible", IsWindowVisible(pluginwind) ? 1 : 0);
	}
	return 0;
}

static int DBSettingChange(WPARAM wParam, LPARAM lParam) {
	if(pluginwind && (HANDLE)wParam == NULL) {
		DBCONTACTWRITESETTING *db_write = (DBCONTACTWRITESETTING *)lParam;
		if(hook_window_behaviour_to_clist) {
			if(!strcmp("CList", db_write->szModule)) {
				if(!strcmp("State", db_write->szSetting)) {
					// contact list setting change
					BYTE state = db_write->value.bVal;

					if(state == SETTING_STATE_NORMAL)
						ShowWindow(pluginwind, SW_SHOW);
					else
						ShowWindow(pluginwind, SW_HIDE);
				} else if(!strcmp("Transparent", db_write->szSetting)) {
					if(db_write->value.bVal) {
						SetWindowLong(pluginwind, GWL_EXSTYLE, GetWindowLong(pluginwind, GWL_EXSTYLE) | WS_EX_LAYERED);
						if (MySetLayeredWindowAttributes) MySetLayeredWindowAttributes(pluginwind, RGB(0,0,0), (BYTE)DBGetContactSettingByte(NULL,"CList","Alpha",SETTING_ALPHA_DEFAULT), LWA_ALPHA);
					} else {
						SetWindowLong(pluginwind, GWL_EXSTYLE, GetWindowLong(pluginwind, GWL_EXSTYLE) & ~WS_EX_LAYERED);
					}
				} else if(!strcmp("Alpha", db_write->szSetting)) {
					ShowWindow(pluginwind, IsWindowVisible(pluginwind) ? SW_SHOW : SW_HIDE);
				} else if(!strcmp("AutoAlpha", db_write->szSetting)) {
					ShowWindow(pluginwind, IsWindowVisible(pluginwind) ? SW_SHOW : SW_HIDE);
				}
			}
		}

		if(db_write && !strcmp("CLC", db_write->szModule))
			InvalidateRect(pluginwind, 0, FALSE);
	}

	return 0;
}

int addmypluginwindow1(HWND parent)
{
	if(!get_reg_tzi_for_central_european(ce_reg_tzi)) {
		MessageBox(0, "Could not get Swiss time!", "Error", MB_OK);
		return 0;
	}

	/*
	std::ostringstream oss;
	oss << "Bias for swiss time: " << ce_reg_tzi.Bias;
	MessageBox(0, oss.str().c_str(), "blah", MB_OK);
	*/

	hUserDll = LoadLibrary("user32.dll");
	if (hUserDll) {
		MySetLayeredWindowAttributes = (BOOL (WINAPI *)(HWND,COLORREF,BYTE,DWORD))GetProcAddress(hUserDll, "SetLayeredWindowAttributes");
		MyAnimateWindow=(BOOL (WINAPI*)(HWND,DWORD,DWORD))GetProcAddress(hUserDll,"AnimateWindow");
	}

	hwnd_clist = parent;

	hook_window_behaviour_to_clist = (DBGetContactSettingByte(NULL, "InternetTime", "MinMax", DEFAULT_MINMAX ? 1 : 0) == 1);

	WNDCLASS wndclass;
	CLISTFrame Frame;
	int font;

	char pluginname[]="Internet Time";

	hDBChange = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, DBSettingChange);
	
    wndclass.style         = 0;
    wndclass.lpfnWndProc   = mypluginwindow1proc;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = 0;
    wndclass.hInstance     = hInst;
    wndclass.hIcon         = NULL;
    wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)(COLOR_3DFACE+1);
    wndclass.lpszMenuName  = NULL;
    wndclass.lpszClassName = pluginname;
	RegisterClass(&wndclass);


	if(ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) {
		pluginwind=CreateWindow(pluginname,Translate("Internet Time"),
						WS_BORDER|WS_CHILD|WS_VISIBLE,
						0,0,0,0,parent,NULL,hInst,NULL);
	} else {
		CreateServiceFunction("InternetTime/ShowWindow", PlugShowWindow);

		int x, y, width, height;
		bool visible;

		x = (int)DBGetContactSettingDword(NULL, "InternetTime", "WindowX", DEFAULT_WINDOW_X);
		y = (int)DBGetContactSettingDword(NULL, "InternetTime", "WindowY", DEFAULT_WINDOW_Y);
		width = (int)DBGetContactSettingDword(NULL, "InternetTime", "WindowWidth", DEFAULT_WINDOW_WIDTH);
		height = (int)DBGetContactSettingDword(NULL, "InternetTime", "WindowHeight", DEFAULT_WINDOW_HEIGHT);


		if(hook_window_behaviour_to_clist) {
			visible = (DBGetContactSettingByte(NULL, "CList", "State", SETTING_STATE_NORMAL) == SETTING_STATE_NORMAL);
		} else {
			visible = ((int)DBGetContactSettingByte(NULL, "InternetTime", "WindowVisible", 1) == 1);
		}

		pluginwind=CreateWindowEx(WS_EX_TOOLWINDOW, pluginname,Translate("Internet Time"),
						WS_POPUPWINDOW | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | (visible ? WS_VISIBLE : 0),
						x,y,width,height,parent,NULL,hInst,NULL);

		/*
		CLISTMENUITEM mi;
		memset( &mi, 0, sizeof( mi ));
		mi.flags = 0;
		mi.popupPosition = 0;
		mi.pszPopupName = NULL;
		mi.cbSize = sizeof( mi );
		mi.position = 2000420000;
		mi.hIcon = 0;//LoadIcon( hInst, 0);
		mi.pszName = Translate( "Hide/Show &World Time Window" );
		mi.pszService = "InternetTime/ShowWindow";
		CallService( MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM)&mi );
		*/
	}
	

	font=SendMessage(parent,WM_GETFONT,0,0);
	SendMessage(pluginwind,WM_SETFONT,font,0);

	TitleBarFont = (HFONT)CLUILoadTitleBarFont();
	SendMessage(label,WM_SETFONT,(WPARAM)TitleBarFont,0);

	memset(&Frame,0,sizeof(Frame));
	Frame.name=(char *)malloc(255);
	memset(Frame.name,0,255);

	memcpy(Frame.name,pluginname,sizeof(pluginname));

	Frame.cbSize=sizeof(CLISTFrame);
	Frame.hWnd=pluginwind;
	Frame.align=alBottom;
	Frame.Flags=F_VISIBLE|F_SHOWTB|F_SHOWTBTIP;
	Frame.height=30;
	
	if(ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) {
		Frameid=CallService(MS_CLIST_FRAMES_ADDFRAME,(WPARAM)&Frame,0);
	}

	return 0;	

};

// popup support

LRESULT CALLBACK NullWindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message ) {
		case WM_COMMAND: {
			PUDeletePopUp( hWnd );
			break;
		}

		case WM_CONTEXTMENU:
			PUDeletePopUp( hWnd );
			break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

void CALLBACK sttMainThreadCallback( ULONG dwParam )
{
	POPUPDATAEX* ppd = ( POPUPDATAEX* )dwParam;

	if ( ServiceExists(MS_POPUP_ADDPOPUPEX) )
		CallService( MS_POPUP_ADDPOPUPEX, ( WPARAM )ppd, 0 );
	else 
		if ( ServiceExists(MS_POPUP_ADDPOPUP) )
			CallService( MS_POPUP_ADDPOPUP, ( WPARAM )ppd, 0 );

	free( ppd );
}

void __stdcall	ShowPopup( const char* line1, const char* line2, int flags )
{
	if ( !ServiceExists( MS_POPUP_ADDPOPUP )) {	
		MessageBox( NULL, line2, Translate("World Time"), MB_OK | MB_ICONINFORMATION );
		return;
	}

	POPUPDATAEX* ppd = ( POPUPDATAEX* )calloc( sizeof( POPUPDATAEX ), 1 );

	ppd->lchContact = NULL;
	ppd->lchIcon = 0;
	strcpy( ppd->lpzContactName, line1 );
	strcpy( ppd->lpzText, line2 );

	ppd->colorBack = GetSysColor( COLOR_BTNFACE );
	ppd->colorText = GetSysColor( COLOR_WINDOWTEXT );
	ppd->iSeconds = 10;

	ppd->PluginWindowProc = ( WNDPROC )NullWindowProc;
	ppd->PluginData = NULL;

	QueueUserAPC( sttMainThreadCallback , mainThread, ( ULONG )ppd );
}

void plugwin_cleanup() {
	UnhookEvent(hDBChange);
}