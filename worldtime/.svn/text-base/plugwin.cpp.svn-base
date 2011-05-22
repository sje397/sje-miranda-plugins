#include "common.h"
#include "plugwin.h"
#include "commctrl.h"

HFONT TitleBarFont;
HFONT ContactFont;
COLORREF ContactFontColour;

FontIDT font_id; // for use with FontService plugin

HBRUSH tbrush = 0;

HANDLE mainMenuItem, hIconsChangedEvent, hDBChange, hIcoLibIconsChanged;

HICON upIcon, downIcon, riseIcon, setIcon;

HMENU hMainMenu;
UINT menuItemId;

ITEMLIST listbox_items(5);

TCHAR format_string[512], date_format_string[512];

int Frameid=-1;
HWND label;

HWND pluginwind = 0, hwnd_clist = 0;

bool hook_window_behaviour_to_clist = true;
bool set_format = false;
bool show_icons = true;
bool hide_menu = false;

BOOL (WINAPI *MySetLayeredWindowAttributes)(HWND,COLORREF,BYTE,DWORD);
BOOL (WINAPI *MyAnimateWindow)(HWND hWnd,DWORD dwTime,DWORD dwFlags);

#define TM_AUTOALPHA  1
static int transparentFocus=1;

#define WM_WTREFRESH			(WM_USER + 10)
#define WMU_INITIALIZE			(WM_USER + 11)
#define WMU_SIZELIST			(WM_USER + 12)

static HFONT CLUILoadTitleBarFont()
{
	HFONT hfont;
	LOGFONT logfont = {0};
	_tcscpy(logfont.lfFaceName, _T("MS Shell Dlg"));
	logfont.lfWeight=FW_NORMAL;
	logfont.lfHeight=-10;
	hfont=CreateFontIndirect(&logfont);
	return hfont;
}

HFONT GetCLCFont0()
{
	DBVARIANT dbv;
	char idstr[10];
	BYTE style;
	LOGFONT log_font;
	LOGFONT *lf = &log_font;
	int i = 0;
	HFONT hFont;
	HDC hdc = GetDC(pluginwind);

	SystemParametersInfo(SPI_GETICONTITLELOGFONT,sizeof(LOGFONT),lf,FALSE);

	if(!DBGetContactSettingTString(NULL,"WorldTime","FontName",&dbv)) {
		lstrcpy(lf->lfFaceName, dbv.ptszVal);
		DBFreeVariant(&dbv);
	} else {
		mir_snprintf(idstr, SIZEOF(idstr), "Font%dName", i);
		if(!DBGetContactSettingTString(NULL,"CLC",idstr,&dbv)) {
			lstrcpy(lf->lfFaceName,dbv.ptszVal);
			DBFreeVariant(&dbv);
		}
	}
    mir_snprintf(idstr, SIZEOF(idstr), "Font%dCol", i);
    ContactFontColour = (COLORREF)DBGetContactSettingDword(NULL, "CLC", idstr, GetSysColor(COLOR_WINDOWTEXT));
	if(!DBGetContactSetting(NULL,"WorldTime","FontSize",&dbv)) {
		lf->lfHeight=(char)dbv.bVal;
	} else {
		mir_snprintf(idstr, SIZEOF(idstr), "Font%dSize", i);
		lf->lfHeight=(char)DBGetContactSettingByte(NULL,"CLC",idstr,lf->lfHeight);
		if(lf->lfHeight > 0)
			lf->lfHeight=-MulDiv(abs(lf->lfHeight), GetDeviceCaps(hdc, LOGPIXELSY), 72);
	}

#define DBFONTF_BOLD       1
#define DBFONTF_ITALIC     2
#define DBFONTF_UNDERLINE  4

	if(!DBGetContactSettingString(NULL,"WorldTime","FontSty",&dbv)) {
		style=(BYTE)DBGetContactSettingByte(NULL,"WorldTime","FontSty",(lf->lfWeight==FW_NORMAL?0:DBFONTF_BOLD)|(lf->lfItalic?DBFONTF_ITALIC:0)|(lf->lfUnderline?DBFONTF_UNDERLINE:0));
	} else {
		mir_snprintf(idstr, SIZEOF(idstr), "Font%dSty", i);
		style=(BYTE)DBGetContactSettingByte(NULL,"CLC",idstr,(lf->lfWeight==FW_NORMAL?0:DBFONTF_BOLD)|(lf->lfItalic?DBFONTF_ITALIC:0)|(lf->lfUnderline?DBFONTF_UNDERLINE:0));
	}
	lf->lfWidth=lf->lfEscapement=lf->lfOrientation=0;
	lf->lfWeight=style&DBFONTF_BOLD?FW_BOLD:FW_NORMAL;
	lf->lfItalic=(style&DBFONTF_ITALIC)!=0;
	lf->lfUnderline=(style&DBFONTF_UNDERLINE)!=0;
	lf->lfStrikeOut=0;

	if(!DBGetContactSetting(NULL,"WorldTime","FontSet",&dbv)) {
		lf->lfCharSet = dbv.bVal;
	} else {
		mir_snprintf(idstr, SIZEOF(idstr), "Font%dSet", i);
		lf->lfCharSet=DBGetContactSettingByte(NULL,"CLC",idstr,lf->lfCharSet);
	}
	lf->lfOutPrecision=OUT_DEFAULT_PRECIS;
	lf->lfClipPrecision=CLIP_DEFAULT_PRECIS;
	lf->lfQuality=DEFAULT_QUALITY;
	lf->lfPitchAndFamily=DEFAULT_PITCH|FF_DONTCARE;

	hFont=CreateFontIndirect(lf);
	ReleaseDC(pluginwind, hdc);
	return hFont;
}

HFONT GetFont() {
	if(ServiceExists(MS_FONT_REGISTERT)) {
		LOGFONT log_font;
		ContactFontColour = (COLORREF)CallService(MS_FONT_GETT, (WPARAM)&font_id, (LPARAM)&log_font);
		return CreateFontIndirect(&log_font);
	} else
		return GetCLCFont0();
}

int ReloadFont(WPARAM wParam, LPARAM lParam) {
	DeleteObject(ContactFont);
	ContactFont = GetFont();
	if(pluginwind) {
		/*
		RECT r;
		SIZE textSize;
		GetWindowRect(pluginwind, &r);
		HFONT hOldFont = (HFONT)SelectObject(GetDC(pluginwind), ContactFont);
		GetTextExtentPoint32(GetDC(pluginwind),"X",1,&textSize);
		SelectObject(GetDC(pluginwind), hOldFont);
		SetWindowPos(pluginwind, 0, 0, 0, r.right - r.left, textSize.cy * listbox_items.size(), SWP_NOZORDER | SWP_NOMOVE);
		if(Frameid != -1) {
			CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS, (WPARAM)MAKELONG(FO_HEIGHT, Frameid), (LPARAM)DBGetContactSettingByte(0, "WorldTime", "FontSize", 10) * listbox_items.size());
		}
		*/
		InvalidateRect(label, 0, TRUE);
	}
	return 0;
}

/*
void CALLBACK plug1TimerProc(
  HWND hwnd,         // handle to window
  UINT uMsg,         // WM_TIMER message
  UINT idEvent,  // timer identifier
  DWORD dwTime       // current system time
)
{
	char TBcapt[255];
	SYSTEMTIME systime;
	
	GetLocalTime(&systime);
	
	wsprintf(TBcapt,"%s %02d:%02d:%02d",Translate("World Time"), systime.wHour, systime.wMinute, systime.wSecond);
		
	CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS,MAKEWPARAM(FO_TBNAME,Frameid),(LPARAM)TBcapt);
	CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS,MAKEWPARAM(FO_TBTIPNAME,Frameid),(LPARAM)TBcapt);
	CallService(MS_CLIST_FRAMES_UPDATEFRAME,Frameid,FU_TBREDRAW);
};
*/

void CALLBACK plug1TimerProc2(
  HWND hwnd,         // handle to window
  UINT uMsg,         // WM_TIMER message
  UINT_PTR idEvent,  // timer identifier
  DWORD dwTime       // current system time
)
{
	if(pluginwind && label) {
		//InvalidateRect(pluginwind, 0, TRUE);
		//InvalidateRect(label, 0, FALSE);
		//ValidateRect(pluginwind, 0);
		SendMessage(pluginwind, WM_WTREFRESH, 0, 0);
	}
};

void CALLBACK plug1TimerProc3(
  HWND hwnd,         // handle to window
  UINT uMsg,         // WM_TIMER message
  UINT_PTR idEvent,  // timer identifier
  DWORD dwTime       // current system time
)
{
	if(pluginwind && hwnd_clist && hook_window_behaviour_to_clist) {
		if(IsWindowVisible(pluginwind) != IsWindowVisible(hwnd_clist))
			ShowWindow(pluginwind, IsWindowVisible(hwnd_clist) ? SW_SHOW : SW_HIDE);
	}
}

int minutes_diff(SYSTEMTIME *t1, SYSTEMTIME *t2) {
	return (t1->wHour - t2->wHour) * 60 + (t1->wMinute - t2->wMinute);
}


bool FrameIsFloating() {
	if(Frameid == -1) 
		return true; // no frames, always floating
	
	return (CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLOATING, Frameid), 0) != 0);
}

WNDPROC oldListWindowProc;

LRESULT CALLBACK SubclassedListWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	RECT r;

	switch(msg) {
	case WM_PAINT:
		if(SendMessage(hwnd, LB_GETCOUNT, 0, 0) == 0) {
			hdc = BeginPaint(hwnd, &ps);
			GetClientRect(hwnd, &r);
			if(FrameIsFloating()) {
				//SkinDrawGlyph(hdc, &r, &r, "Main Window/Backgrnd");
				SkinDrawGlyph(hdc, &r, &r, "World Time/Background");
			} else {
				//SkinDrawWindowBack(hwnd, hdc, &r, "Main Window/Backgrnd");
				SkinDrawGlyph(hdc, &r, &r, "World Time/Background");
			}
			EndPaint(hwnd, &ps);
			return TRUE;
		} else
			break;
	}
	
	return CallWindowProc(oldListWindowProc, hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK mypluginwindow1proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT rect;
	MEASUREITEMSTRUCT *mis;
	LPDRAWITEMSTRUCT dis;
	LISTITEM *pItemData;
	SIZE textSize;
	RECT r;
	LPARAM lp;
	int sel;
	HFONT oldFont;
	//PAINTSTRUCT ps;

	switch(msg)
	{
		
	case WM_MEASUREITEM:
		{
			mis = (MEASUREITEMSTRUCT *)lParam;
			mis->itemWidth = 100;
			mis->itemHeight = DBGetContactSettingWord(NULL, "WorldTime", "RowHeight",GetSystemMetrics(SM_CYSMICON));
			return TRUE;
		}
		
	case WM_DRAWITEM:
		dis = (LPDRAWITEMSTRUCT)lParam;
		//pItemData = (LISTITEM *)dis->itemData;
		if(dis->hwndItem == label) {
			HBRUSH ttbrush = 0;
			RECT r;
			COLORREF tcol;
			//GetClientRect(dis->hwndItem, &r);
			GetClientRect(hwnd, &r);
			if(dis->itemID != -1) {

				oldFont = (HFONT)SelectObject(dis->hDC, ContactFont);
				GetTextExtentPoint32(dis->hDC,_T("X"),1,&textSize);
				SendMessage(label, LB_SETITEMHEIGHT, 0, (LPARAM)DBGetContactSettingWord(NULL, "WorldTime", "RowHeight",GetSystemMetrics(SM_CYSMICON)));
				// stop full list erase
				//dis->rcItem.bottom = dis->rcItem.top + DBGetContactSettingWord(NULL, "WorldTime", "RowHeight",GetSystemMetrics(SM_CYSMICON));

				if(dis->itemState & ODS_SELECTED && dis->itemState & ODS_FOCUS) {

					if(ServiceExists(MS_SKIN_DRAWGLYPH)) {
						if(FrameIsFloating()) {
							//SkinDrawGlyph(dis->hDC, &r, &dis->rcItem, "Main Window/Backgrnd");
							SkinDrawGlyph(dis->hDC, &r, &dis->rcItem, "World Time/Selection Background");
						} else {
							//SkinDrawWindowBack(label, dis->hDC, &dis->rcItem, "Main Window/Backgrnd");
							SkinDrawGlyph(dis->hDC, &r, &dis->rcItem, "World Time/Selection Background");
						}
					} else {
						tcol = DBGetContactSettingDword(NULL,"CLC","SelBkColour", CLCDEFAULT_SELBKCOLOUR);
						SetBkColor(dis->hDC, tcol);
						FillRect(dis->hDC, &dis->rcItem, (ttbrush = CreateSolidBrush(tcol)));
					}
					
					tcol = DBGetContactSettingDword(NULL,"CLC","SelTextColour", CLCDEFAULT_SELTEXTCOLOUR);
					SetTextColor(dis->hDC, tcol);
				} else {
					
					if(ServiceExists(MS_SKIN_DRAWGLYPH)) {
						
						if(FrameIsFloating()) {
							//SkinDrawGlyph(dis->hDC, &r, &dis->rcItem, "Main Window/Backgrnd");
							SkinDrawGlyph(dis->hDC, &r, &dis->rcItem, "World Time/Background");
						} else {
							//SkinDrawWindowBack(label, dis->hDC, &dis->rcItem, "Main Window/Backgrnd");
							SkinDrawGlyph(dis->hDC, &r, &dis->rcItem, "World Time/Background");
						}
						
					} else {
						//tcol = DBGetContactSettingDword(NULL,"CLC","BkColour", CLCDEFAULT_BKCOLOUR);
						tcol = DBGetContactSettingDword(NULL, "WorldTime", "BgColour", GetSysColor(COLOR_3DFACE));
						SetBkColor(dis->hDC, tcol);
						FillRect(dis->hDC, &dis->rcItem, (ttbrush = CreateSolidBrush(tcol)));
					}
					
					//SendMessage(label, LB_GETITEMRECT, (WPARAM)dis->itemID, (LPARAM)&r);
					//FillRect(dis->hDC, &r, (ttbrush = CreateSolidBrush(tcol)));

					tcol = ContactFontColour;					
					SetTextColor(dis->hDC, tcol);
				}				
				
				SetBkMode(dis->hDC, TRANSPARENT);
				pItemData = &listbox_items[dis->itemID];
				{

					dis->rcItem.left += DBGetContactSettingWord(NULL, "WorldTime", "Indent", 0);
					if(show_icons) {
						//DrawIconEx(dis->hDC,dis->rcItem.left,(dis->rcItem.top+dis->rcItem.bottom-GetSystemMetrics(SM_CYSMICON))>>1, pItemData->icon, GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0,NULL,DI_NORMAL);
						DrawIconEx(dis->hDC,dis->rcItem.left,(dis->rcItem.top+dis->rcItem.bottom-16)>>1, pItemData->icon, 0, 0, 0, NULL, DI_NORMAL);
			
						GetTextExtentPoint32(dis->hDC,pItemData->pszText,lstrlen(pItemData->pszText),&textSize);
						TextOut(dis->hDC,dis->rcItem.left + 16 + 4,(dis->rcItem.top+dis->rcItem.bottom-textSize.cy)>>1,pItemData->pszText,lstrlen(pItemData->pszText));
					} else {
						GetTextExtentPoint32(dis->hDC,pItemData->pszText,lstrlen(pItemData->pszText),&textSize);
						TextOut(dis->hDC,dis->rcItem.left,(dis->rcItem.top+dis->rcItem.bottom-textSize.cy)>>1,pItemData->pszText,lstrlen(pItemData->pszText));
					}

					GetTextExtentPoint32(dis->hDC,pItemData->pszTimeText,lstrlen(pItemData->pszTimeText),&textSize);
					TextOut(dis->hDC,dis->rcItem.right - textSize.cx - 2,(dis->rcItem.top+dis->rcItem.bottom-textSize.cy)>>1,pItemData->pszTimeText,lstrlen(pItemData->pszTimeText));

					if(set_format) {
						int xSave = textSize.cx;

						GetTextExtentPoint32(dis->hDC,pItemData->pszDateText,lstrlen(pItemData->pszDateText),&textSize);
						TextOut(dis->hDC,dis->rcItem.right - textSize.cx - xSave - 4,(dis->rcItem.top+dis->rcItem.bottom-textSize.cy)>>1,pItemData->pszDateText,lstrlen(pItemData->pszDateText));
					}
				}

				SetBkMode(dis->hDC, OPAQUE);
				SelectObject(dis->hDC, oldFont);
			} else {
				if(ServiceExists(MS_SKIN_DRAWGLYPH)) {
					if(FrameIsFloating()) {
						//SkinDrawGlyph(dis->hDC, &r, &dis->rcItem, "Main Window/Backgrnd");
						SkinDrawGlyph(dis->hDC, &r, &dis->rcItem, "World Time/Background");
					} else {
						//SkinDrawWindowBack(label, dis->hDC, &dis->rcItem, "Main Window/Backgrnd");
						SkinDrawGlyph(dis->hDC, &r, &dis->rcItem, "World Time/Background");
					}
				} else {
					//tcol = DBGetContactSettingDword(NULL,"CLC","BkColour", CLCDEFAULT_BKCOLOUR);
					tcol = DBGetContactSettingDword(NULL, "WorldTime", "BgColour", GetSysColor(COLOR_3DFACE));
					SetBkColor(dis->hDC, tcol);
					FillRect(dis->hDC, &dis->rcItem, (ttbrush = CreateSolidBrush(tcol)));
				}
			}

			if(ttbrush) DeleteObject(ttbrush);
		}
		return TRUE;
	case WM_WTREFRESH:
		{
			bool need_repaint = false;

			SYSTEMTIME st, other_st;
			HANDLE hIcon;
			TCHAR buf[512], buf2[512];
			MyGetSystemTime(&st);

			for(int i = 0; i < listbox_items.getCount(); i++) {
				TIME_ZONE_INFORMATION tzi;
				tzi.Bias = timezone_list[listbox_items[i].timezone_list_index].TZI.Bias;
				tzi.DaylightBias = timezone_list[listbox_items[i].timezone_list_index].TZI.DaylightBias;
				tzi.DaylightDate = timezone_list[listbox_items[i].timezone_list_index].TZI.DaylightDate;
				tzi.StandardBias = timezone_list[listbox_items[i].timezone_list_index].TZI.StandardBias;
				tzi.StandardDate = timezone_list[listbox_items[i].timezone_list_index].TZI.StandardDate;

				MySystemTimeToTzSpecificLocalTime(&tzi, &st, &other_st);

				if(set_format) {
					GetTimeFormat(LOCALE_USER_DEFAULT, 0, &other_st, format_string, buf, 512);
					GetDateFormat(LOCALE_USER_DEFAULT, 0, &other_st, date_format_string, buf2, 512);
				} else
					GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &other_st, 0, buf, 512);

				int risediff = minutes_diff(&other_st, &listbox_items[i].sunrise),
					setdiff = minutes_diff(&other_st, &listbox_items[i].sunset);
				if(risediff >= 0 && setdiff < 0) {
					if(risediff < 30)
						hIcon = riseIcon;
					else
						hIcon = upIcon;
				} else {
					if(setdiff > 0 && setdiff < 30)
						hIcon = setIcon;
					else
						hIcon = downIcon;
				}

				if(_tcscmp(buf, listbox_items[i].pszTimeText) || (set_format && _tcscmp(buf2, listbox_items[i].pszDateText)) || listbox_items[i].icon != hIcon) {
					need_repaint = true;
					_tcscpy(listbox_items[i].pszTimeText, buf);
					if(set_format) _tcscpy(listbox_items[i].pszDateText, buf2);
					else listbox_items[i].pszDateText[0] = '\0';
					listbox_items[i].icon = (HICON)hIcon;
				}
			}
			if(need_repaint)
				InvalidateRect(label, 0, FALSE);
		}
		return TRUE;
	case WM_CTLCOLORLISTBOX:
		{
			if(ServiceExists(MS_SKIN_DRAWGLYPH)) return 0;

			if(tbrush) DeleteObject(tbrush);

			return (INT_PTR)(tbrush = CreateSolidBrush((COLORREF)DBGetContactSettingDword(NULL,"WorldTime","BgColour", GetSysColor(COLOR_3DFACE))));
		}
		break;
	case WM_ERASEBKGND: 
		/*
		{
//			HDC hDC = (HDC)wParam;
//			HBRUSH tempBrush = CreateSolidBrush((COLORREF)DBGetContactSettingDword(NULL,"CLC","BkColour", CLCDEFAULT_BKCOLOUR));
//			GetClientRect(hwnd, &r);
//			FillRect(hDC, &r, tempBrush);
//			DeleteObject(tempBrush);
			
			HDC hdc = GetDC(hwnd);
			RECT r;
			GetClientRect(hwnd, &r);
			SkinDrawWindowBack(hwnd, hdc, &r, "Ping/Background");
			ReleaseDC(hwnd, hdc);
		}
		*/
		//return DefWindowProc(hwnd, msg, wParam, lParam);
		{
			RECT r;
			GetClientRect(hwnd, &r);
			if(ServiceExists(MS_SKIN_DRAWGLYPH)) {
				if(FrameIsFloating()) {
					//SkinDrawGlyph((HDC)wParam, &r, &r, "Main Window/Backgrnd");
					SkinDrawGlyph((HDC)wParam, &r, &r, "World Time/Background");
				} else {
					//SkinDrawWindowBack(label, (HDC)wParam, &r, "Main Window/Backgrnd");
					SkinDrawGlyph((HDC)wParam, &r, &r, "World Time/Background");
				}
			} else {
				FillRect((HDC)wParam, &r, tbrush);
			}
		}
		return TRUE;


	case WM_SYSCOLORCHANGE:
		SendMessage(label,msg,wParam,lParam);
		break;

	case WM_CREATE:
		label=CreateWindow(_T("LISTBOX"), _T(""), (WS_VISIBLE | WS_CHILD | LBS_STANDARD | LBS_OWNERDRAWFIXED | LBS_NOTIFY) & ~WS_BORDER, 0, 0, 0, 0, hwnd, NULL, hInst,0);

		oldListWindowProc = (WNDPROC)SetWindowLongPtr(label, GWLP_WNDPROC, (LONG_PTR)SubclassedListWindowProc);

		if (DBGetContactSettingByte(NULL,"CList","Transparent",0))
		{
			if(ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) {

			} else {
				SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
				if (MySetLayeredWindowAttributes) MySetLayeredWindowAttributes(hwnd, RGB(0,0,0), (BYTE)DBGetContactSettingByte(NULL,"CList","Alpha",SETTING_ALPHA_DEFAULT), LWA_ALPHA);
			}
			//if (MySetLayeredWindowAttributes) MySetLayeredWindowAttributes(hwnd, RGB(0,0,0), 255, LWA_ALPHA);		
		}
		return FALSE;

	case WMU_INITIALIZE:
		{	
			//CreateWindow("button","button1",WS_VISIBLE|WS_CHILD	,120,4,60,22,hwnd,NULL,hInst,0);
			//label=CreateWindow("LISTBOX", "", (WS_VISIBLE | WS_CHILD | LBS_STANDARD | LBS_OWNERDRAWFIXED | LBS_NOTIFY) & ~WS_BORDER, 2, 2, 120, 60, hwnd, NULL, hInst,0);
			
			//SetTimer(hwnd,TIMER_ID,	1000,plug1TimerProc);
			SetTimer(hwnd,TIMER_ID2,1000,plug1TimerProc2);
			SetTimer(hwnd,TIMER_ID3,1000,plug1TimerProc3);
		
			

		}
		//PostMessage(hwnd, WM_WTREFRESH, 0, 0);
		//PostMessage(hwnd, WM_SIZE, 0, 0);
		PostMessage(hwnd, WMU_SIZELIST, 0, 0);
		return TRUE;


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
		break;

	case WM_SETCURSOR:
		if(DBGetContactSettingByte(NULL,"CList","Transparent",SETTING_TRANSPARENT_DEFAULT)) {
			if (!transparentFocus && GetForegroundWindow()!=hwnd && MySetLayeredWindowAttributes) {
				MySetLayeredWindowAttributes(hwnd, RGB(0,0,0), (BYTE)DBGetContactSettingByte(NULL,"CList","Alpha",SETTING_ALPHA_DEFAULT), LWA_ALPHA);
				transparentFocus=1;
				SetTimer(hwnd, TM_AUTOALPHA,250,NULL);
			}
		}
		break;

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
		DBWriteContactSettingByte(NULL, "WorldTime", "WindowVisible", wParam ? 1 : 0);

		PostMessage(hwnd, WMU_SIZELIST, 0, 0);
		InvalidateRect(hwnd, 0, TRUE);
		break;
		//return FALSE; //break;
	}

	case WM_CONTEXTMENU:
		{	
			OPENOPTIONSDIALOG ood = {0};
			ood.cbSize = sizeof(ood);
			ood.pszGroup = "Plugins";
			ood.pszPage = "World Time";

			CallService(MS_OPT_OPENOPTIONS, 0, (LPARAM)&ood);
		}
		return TRUE;

	case WM_RBUTTONDOWN:
		//CallService(MS_CLIST_MENUBUILDFRAMECONTEXT, Frameid, 0);
		break;


	case WM_COMMAND:
		if (HIWORD( wParam ) == LBN_DBLCLK) {
			sel = SendMessage(label, LB_GETCURSEL, 0, 0);
			if(sel != LB_ERR) {
				lp = SendMessage(label, LB_GETITEMDATA, sel, 0);
				if(lp != LB_ERR) {
					pItemData = (LISTITEM *)lp;

					// ??
				}
			}
		}
		break;
	case WM_PRINTCLIENT:
		{
			/*
			HDC hdc = (HDC)wParam;
			RECT r;
			GetClientRect(hwnd, &r);
			*/
		}
		return TRUE;

	case WM_PAINT:
		{
			RECT r;
			if(GetUpdateRect(hwnd, &r, FALSE)) {
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hwnd, &ps);
				SendMessage(hwnd, WM_PRINTCLIENT, (WPARAM)hdc, (LPARAM)(PRF_CLIENT | PRF_CHILDREN));
				EndPaint(hwnd, &ps);
			}
		}
		return TRUE;
		
	case WM_MOVE:			// needed for docked frames in clist_mw (not needed in clist_modern)
		if(FrameIsFloating()) 
			break;
	case WM_SIZE:
		//if(IsWindowVisible(hwnd))
			PostMessage(hwnd, WMU_SIZELIST, 0, 0);
		break;
	case WMU_SIZELIST:

		{
			//PostMessage(label, WM_SIZE, wParam, lParam);
			
			GetClientRect(hwnd,&rect);	

			{
				//char buff[256];
				//sprintf(buff, "WMU_SIZELIST: width = %d, height = %d", rect.right - rect.left, rect.bottom - rect.top);
				//PUShowMessage(buff, SM_NOTIFY);

			}

			int winheight = rect.bottom - rect.top,
				itemheight = SendMessage(label, LB_GETITEMHEIGHT, 0, 0),
				count = SendMessage(label, LB_GETCOUNT, 0, 0),
				height = min(winheight - winheight % itemheight, itemheight * count);
			SetWindowPos(label, 0, rect.left, rect.top, rect.right-rect.left, height, SWP_NOZORDER);
			InvalidateRect(label, 0, FALSE);
			//SetWindowPos(label, 0, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, SWP_NOZORDER);
		}
		if(Frameid != -1) {
			//CallService(MS_CLIST_FRAMES_UPDATEFRAME, (WPARAM)Frameid, (LPARAM)(FU_TBREDRAW | FU_FMREDRAW));
			CallService(MS_CLIST_FRAMES_UPDATEFRAME, (WPARAM)Frameid, (LPARAM)FU_TBREDRAW);
			//CallService(MS_CLIST_FRAMES_UPDATEFRAME, (WPARAM)Frameid, (LPARAM)(FU_TBREDRAW | FU_FMREDRAW | FU_FMPOS));
		}

		InvalidateRect(hwnd, 0, TRUE);
		return TRUE;


	case WM_DESTROY:
		if(tbrush) DeleteObject(tbrush);

		if(!ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) {
			GetWindowRect(hwnd, &r);

			DBWriteContactSettingDword(NULL, "WorldTime", "WindowX", r.left);
			DBWriteContactSettingDword(NULL, "WorldTime", "WindowY", r.top);
			DBWriteContactSettingDword(NULL, "WorldTime", "WindowWidth", r.right - r.left);
			DBWriteContactSettingDword(NULL, "WorldTime", "WindowHeight", r.bottom - r.top);
		}

		{
			KillTimer(hwnd,TIMER_ID);
			KillTimer(hwnd,TIMER_ID2);
		}
		DestroyWindow(label);
	
		break;

	case WM_CLOSE:
		if(!ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) {
			if(!hook_window_behaviour_to_clist)
				ShowWindow(hwnd, SW_HIDE);
			return TRUE;
		}
		break;

	};
	
	return DefWindowProc(hwnd, msg, wParam, lParam);
};

int FillList(WPARAM wParam, LPARAM lParam) {

	SendMessage(label, LB_RESETCONTENT, 0, 0);

	for(int i = 0; i < listbox_items.getCount(); i++) {
		SendMessage(label, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)listbox_items[i].pszText);
	}

	InvalidateRect(label, 0, TRUE);
	return 0;
}

static INT_PTR PlugShowWindow(WPARAM wParam, LPARAM lParam) {
	if(pluginwind && !hook_window_behaviour_to_clist) {
		ShowWindow(pluginwind, IsWindowVisible(pluginwind) ? SW_HIDE : SW_SHOW);
		DBWriteContactSettingByte(NULL, "WorldTime", "WindowVisible", IsWindowVisible(pluginwind) ? 1 : 0);
	}
	return 0;
}

int ReloadIcons(WPARAM wParam, LPARAM lParam)
{
	upIcon = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)"WorldTime_day");
	downIcon = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)"WorldTime_night");
	riseIcon = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)"WorldTime_sunrise");
	setIcon = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)"WorldTime_sunset");

	return 0;
}

int InitIcons(WPARAM wParam, LPARAM lParam) 
{
	SKINICONDESC sid = {0};

	TCHAR szFile[MAX_PATH];
	GetModuleFileName(hInst, szFile, SIZEOF(szFile));

	sid.cbSize = sizeof(SKINICONDESC);
	sid.pszSection = "WorldTime";
	sid.ptszDefaultFile = szFile;
	sid.flags = SIDF_PATH_TCHAR;

	sid.pszDescription = "Day";
	sid.pszName = "WorldTime_day";
	sid.iDefaultIndex = -IDI_ICON_SUN;
	CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

	sid.pszDescription = "Night";
	sid.pszName = "WorldTime_night";
	sid.iDefaultIndex = -IDI_ICON_MOON;
	CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

	sid.pszDescription = "Sunrise";
	sid.pszName = "WorldTime_sunrise";
	sid.iDefaultIndex = -IDI_ICON_SUNRISE;
	CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

	sid.pszDescription = "Sunset";
	sid.pszName = "WorldTime_sunset";
	sid.iDefaultIndex = -IDI_ICON_SUNSET;
	CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

	ReloadIcons(0, 0);
	return 0;
}

void load_listbox_items() {
	int num = DBGetContactSettingDword(0, "WorldTime", "NumEntries", 0);
	for(int i = 0; i < num; i++) {
		LISTITEM lsi = {0};
		char p1[20], p2[20], p3[20], p4[20], p5[20];

		mir_snprintf(p1, SIZEOF(p1), "Label%d", i);
		mir_snprintf(p2, SIZEOF(p2), "Index%d", i);
		mir_snprintf(p3, SIZEOF(p3), "Sunrise%d", i);
		mir_snprintf(p4, SIZEOF(p4), "Sunset%d", i);
		mir_snprintf(p5, SIZEOF(p5), "TZName%d", i);

		DBVARIANT dbv;
		DBGetContactSettingTString(0, "WorldTime", p1, &dbv);
		_tcsncpy(lsi.pszText, dbv.ptszVal, MAX_NAME_LENGTH);
		DBFreeVariant(&dbv);

		if(!DBGetContactSettingTString(0, "WorldTime", p5, &dbv)) {
			for(int j = 0; j < timezone_list.getCount(); j++) {
				if(!_tcscmp(timezone_list[j].tcName, dbv.ptszVal)) {
					lsi.timezone_list_index = timezone_list[j].list_index;
					break;
				}
			}
			DBFreeVariant(&dbv);
		} else
			lsi.timezone_list_index = (int)DBGetContactSettingDword(0, "WorldTime", p2, 0);

		GetSystemTime(&lsi.sunrise);
		GetSystemTime(&lsi.sunset);

		int sunrise_min = (int)DBGetContactSettingDword(0, "WorldTime", p3, 360), // 60 * 6 (6 am) = 360
			sunset_min = (int)DBGetContactSettingDword(0, "WorldTime", p4, 1080); // 60 * 18 (6 pm) = 1080

		lsi.sunrise.wHour = sunrise_min / 60;
		lsi.sunrise.wMinute = sunrise_min % 60;
		lsi.sunrise.wSecond = 0;
		lsi.sunset.wHour = sunset_min / 60;
		lsi.sunset.wMinute = sunset_min % 60;
		lsi.sunset.wSecond = 0;

		lsi.pszTimeText[0] = '\0';
		lsi.pszDateText[0] = '\0';
		lsi.icon = upIcon;

		listbox_items.insert(new LISTITEM(lsi));
	}
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

		if(db_write && !strcmp("CLC", db_write->szModule) && !strncmp("Font", db_write->szSetting, 4)) {
			DeleteObject(ContactFont);
			ContactFont = (HFONT)GetFont();
			InvalidateRect(label, 0, FALSE);
		}
	}

	return 0;
}

int RefreshWindow(WPARAM wParam, LPARAM lParam) {
	//InvalidateRect(hpwnd, 0, TRUE);
	InvalidateRect(label, 0, TRUE);
	return 0;
}

int SkinReload(WPARAM wParam, LPARAM lParam) {
	CreateGlyphedObjectDefColor("World Time/Background", DBGetContactSettingDword(NULL, "WorldTime", "BgColour", GetSysColor(COLOR_3DFACE)));
	CreateGlyphedObjectDefColor("World Time/Selection Background", DBGetContactSettingDword(NULL, "CLC", "SelBkColour", CLCDEFAULT_SELBKCOLOUR));
	RefreshWindow(0, 0);
	return 0;
}

int addmypluginwindow1(HWND parent)
{
	if (ServiceExists(MS_FONT_REGISTERT)) {
		font_id.cbSize = sizeof(font_id);
		_tcscpy(font_id.group, _T("Frames"));
		_tcscpy(font_id.name, _T("World Time"));
		strncpy(font_id.dbSettingsGroup, "WorldTime", sizeof(font_id.dbSettingsGroup));
		strncpy(font_id.prefix, "Font", sizeof(font_id.prefix));
		font_id.order = 0;

		CallService(MS_FONT_REGISTERT, (WPARAM)&font_id, 0);
		HookEvent(ME_FONT_RELOAD, ReloadFont);
	}

	if(ServiceExists(MS_SKIN_REGISTERDEFOBJECT)) {
		SkinReload(0, 0);
		HookEvent(ME_SKIN_SERVICESCREATED, SkinReload);
	}

	HMODULE hUserDll = GetModuleHandleA("user32");
	if (hUserDll) {
		MySetLayeredWindowAttributes = (BOOL (WINAPI *)(HWND,COLORREF,BYTE,DWORD))GetProcAddress(hUserDll, "SetLayeredWindowAttributes");
		MyAnimateWindow=(BOOL (WINAPI*)(HWND,DWORD,DWORD))GetProcAddress(hUserDll,"AnimateWindow");
	}

	hwnd_clist = parent;

	hook_window_behaviour_to_clist = (DBGetContactSettingByte(NULL, "WorldTime", "MinMax", DEFAULT_MINMAX ? 1 : 0) == 1);
	set_format = (DBGetContactSettingByte(NULL, "WorldTime", "EnableTimeFormat", 0) == 1);
	show_icons = (DBGetContactSettingByte(NULL, "WorldTime", "ShowIcons", 1) == 1);
	hide_menu = (DBGetContactSettingByte(NULL, "WorldTime", "HideMenu", 0) == 1);
	DBVARIANT dbv;
	if(!DBGetContactSettingTString(NULL, "WorldTime", "TimeFormat", &dbv)) {
		_tcsncpy(format_string, dbv.ptszVal, 512);
	}
	DBFreeVariant(&dbv);
	if(!DBGetContactSettingTString(NULL, "WorldTime", "DateFormat", &dbv)) {
		_tcsncpy(date_format_string, dbv.ptszVal, 512);
	}
	DBFreeVariant(&dbv);

	WNDCLASS wndclass;
	int font;

	InitIcons(0, 0);
	if(ServiceExists(MS_SKIN2_ADDICON)) {
		hIcoLibIconsChanged = HookEvent(ME_SKIN2_ICONSCHANGED, ReloadIcons);
	}

	load_listbox_items();

	//hIconsChangedEvent = HookEvent(ME_SKIN_ICONSCHANGED, LoadIcons);
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
    wndclass.lpszClassName = _T(PLUG);
	RegisterClass(&wndclass);

	if(ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) {
		pluginwind=CreateWindow(_T(PLUG),TranslateT("World Time"),
						WS_BORDER|WS_CHILD|WS_CLIPCHILDREN,
						0,0,0,0,parent,NULL,hInst,NULL);

		CLISTFrame Frame = {0};
		Frame.name=PLUG;
		Frame.cbSize=sizeof(CLISTFrame);
		Frame.hWnd=pluginwind;
		Frame.align=alBottom;
		Frame.Flags=F_VISIBLE|F_SHOWTB|F_SHOWTBTIP;
		Frame.height=30;
		Frame.TBname = Translate("World Time");
		
		Frameid=CallService(MS_CLIST_FRAMES_ADDFRAME,(WPARAM)&Frame,0);

	} else {
		CreateServiceFunction("WorldTime/ShowWindow", PlugShowWindow);

		int x, y, width, height;
		bool visible;

		x = (int)DBGetContactSettingDword(NULL, "WorldTime", "WindowX", DEFAULT_WINDOW_X);
		y = (int)DBGetContactSettingDword(NULL, "WorldTime", "WindowY", DEFAULT_WINDOW_Y);
		width = (int)DBGetContactSettingDword(NULL, "WorldTime", "WindowWidth", DEFAULT_WINDOW_WIDTH);
		height = (int)DBGetContactSettingDword(NULL, "WorldTime", "WindowHeight", DEFAULT_WINDOW_HEIGHT);


		if(hook_window_behaviour_to_clist) {
			visible = (DBGetContactSettingByte(NULL, "CList", "State", SETTING_STATE_NORMAL) == SETTING_STATE_NORMAL);
		} else {
			visible = ((int)DBGetContactSettingByte(NULL, "WorldTime", "WindowVisible", 1) == 1);
		}

		pluginwind=CreateWindowEx(WS_EX_TOOLWINDOW, _T(PLUG),TranslateT("World Time"),
						WS_POPUPWINDOW | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | (visible ? WS_VISIBLE : 0) | WS_CLIPCHILDREN,
						x,y,width,height,parent,NULL,hInst,NULL);

		if(!hide_menu) {
			CLISTMENUITEM mi;
			memset( &mi, 0, sizeof( mi ));
			mi.flags = 0;
			mi.popupPosition = 0;
			mi.pszPopupName = NULL;
			mi.cbSize = sizeof( mi );
			mi.position = 2000400000;
			mi.hIcon = 0;//LoadIcon( hInst, 0);
			mi.pszName = Translate( "*Hide/Show &World Time Window" );
			mi.pszService = "WorldTime/ShowWindow";
			mainMenuItem = (HANDLE)CallService( MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM)&mi );
		}
	}
	

	font=SendMessage(parent,WM_GETFONT,0,0);
	SendMessage(pluginwind,WM_SETFONT,font,0);

	TitleBarFont = CLUILoadTitleBarFont();
	ContactFont = GetFont();

	SendMessage(label,WM_SETFONT,(WPARAM)TitleBarFont,0);

	FillList(0, 0);

	SendMessage(pluginwind, WMU_INITIALIZE, 0, 0);

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

void CALLBACK sttMainThreadCallback(void* pParam)
{
	POPUPDATAT* ppd = (POPUPDATAT*)pParam;
	CallService( MS_POPUP_ADDPOPUPT, ( WPARAM )ppd, 0 );

	free( ppd );
}

void __stdcall	ShowPopup( const TCHAR* line1, const TCHAR* line2, int flags )
{
	if ( !ServiceExists( MS_POPUP_ADDPOPUPT )) {	
		MessageBox( NULL, line2, TranslateT("World Time"), MB_OK | MB_ICONINFORMATION );
		return;
	}

	POPUPDATAT* ppd = ( POPUPDATAT* )calloc( sizeof( POPUPDATAT ), 1 );

	ppd->lchContact = NULL;
	ppd->lchIcon = 0;
	_tcscpy( ppd->lptzContactName, line1 );
	_tcscpy( ppd->lptzText, line2 );

	ppd->colorBack = GetSysColor( COLOR_BTNFACE );
	ppd->colorText = GetSysColor( COLOR_WINDOWTEXT );
	ppd->iSeconds = 10;

	ppd->PluginWindowProc = ( WNDPROC )NullWindowProc;
	ppd->PluginData = NULL;

	CallFunctionAsync(sttMainThreadCallback, ppd);
}

void plugwin_cleanup() {
	UnhookEvent(hIcoLibIconsChanged);
	if(hIconsChangedEvent) UnhookEvent(hIconsChangedEvent);

	UnhookEvent(hDBChange);

	//KillTimer(pluginwind, TIMER_ID2);
	DestroyWindow(pluginwind);

	DeleteObject(ContactFont);
	DeleteObject(TitleBarFont);
}