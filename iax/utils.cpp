#include "common.h"
#include "utils.h"

LRESULT CALLBACK NullWindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
	switch( message ) {
		case UM_INITPOPUP:
			{
			}
			return 0;
		case WM_COMMAND:
			PUDeletePopUp( hWnd );
			return TRUE;

		case WM_CONTEXTMENU:
			PUDeletePopUp( hWnd );
			return TRUE;

		case UM_FREEPLUGINDATA: 
			{
			}
			return TRUE;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}


void ShowError(char *fmt, ...) {
    va_list args;
	char buff[MAX_SECONDLINE];
    va_start(args, fmt);
    _vsnprintf(buff, MAX_SECONDLINE - 1, fmt, args);
    va_end(args);
	buff[MAX_SECONDLINE - 1] = 0;

	if(ServiceExists(MS_POPUP_ADDPOPUPEX)) {
		POPUPDATAEX pd = {0};
		pd.colorBack = RGB(0xff, 0x00, 0x00);
		pd.colorText = RGB(0xff, 0xff, 0xff);
		pd.lchIcon = LoadIcon(0, IDI_ERROR);
		strncpy(pd.lptzContactName, Translate(MODULE " Error"), MAX_CONTACTNAME);
		strncpy(pd.lptzText, buff, MAX_SECONDLINE);
		pd.iSeconds = -1;
		pd.PluginWindowProc = NullWindowProc;
		PUAddPopUpEx(&pd);
	} else {
		MIRANDASYSTRAYNOTIFY stn = {0};
		stn.cbSize = sizeof(stn);
		stn.dwInfoFlags = NIIF_ERROR; // NIIF_INFO, NIIF_WARNING, NIIF_ERROR
		stn.szInfo = buff;
		stn.szInfoTitle = Translate(MODULE " Error");
		stn.szProto = MODULE;
		stn.uTimeout = 10;
		CallService(MS_CLIST_SYSTRAY_NOTIFY, 0, (LPARAM)&stn);
	}
}

void ShowWarning(char *fmt, ...) {
    va_list args;
	char buff[MAX_SECONDLINE];
    va_start(args, fmt);
    _vsnprintf(buff, MAX_SECONDLINE - 1, fmt, args);
    va_end(args);
	buff[MAX_SECONDLINE - 1] = 0;

	if(ServiceExists(MS_POPUP_ADDPOPUPEX)) {
		POPUPDATAEX pd = {0};
		pd.colorBack = RGB(0xff, 0xff, 0x00);
		pd.colorText = 0;
		pd.lchIcon = LoadIcon(0, IDI_WARNING);
		strncpy(pd.lptzContactName, Translate(MODULE " Warning"), MAX_CONTACTNAME);
		strncpy(pd.lptzText, buff, MAX_SECONDLINE);
		pd.iSeconds = -1;
		pd.PluginWindowProc = NullWindowProc;
		PUAddPopUpEx(&pd);
	} else {
		MIRANDASYSTRAYNOTIFY stn = {0};
		stn.cbSize = sizeof(stn);
		stn.dwInfoFlags = NIIF_WARNING; // NIIF_INFO, NIIF_WARNING, NIIF_ERROR
		stn.szInfo = buff;
		stn.szInfoTitle = Translate(MODULE" Warning");
		stn.szProto = MODULE;
		stn.uTimeout = 10;
		CallService(MS_CLIST_SYSTRAY_NOTIFY, 0, (LPARAM)&stn);
	}
}

void ShowInfo(char *fmt, ...) {
    va_list args;
	char buff[MAX_SECONDLINE];
    va_start(args, fmt);
    _vsnprintf(buff, MAX_SECONDLINE - 1, fmt, args);
    va_end(args);
	buff[MAX_SECONDLINE - 1] = 0;

	if(ServiceExists(MS_POPUP_ADDPOPUPEX)) {
		POPUPDATAEX pd = {0};
		pd.colorBack = RGB(0xff, 0xff, 0xff);
		pd.colorText = 0;
		pd.lchIcon = LoadIcon(0, IDI_INFORMATION);
		strncpy(pd.lptzContactName, Translate(MODULE " Information"), MAX_CONTACTNAME);
		strncpy(pd.lptzText, buff, MAX_SECONDLINE);
		pd.iSeconds = 10;
		pd.PluginWindowProc = NullWindowProc;
		PUAddPopUpEx(&pd);
	} else {
		MIRANDASYSTRAYNOTIFY stn = {0};
		stn.cbSize = sizeof(stn);
		stn.dwInfoFlags = NIIF_INFO; // NIIF_INFO, NIIF_WARNING, NIIF_ERROR
		stn.szInfo = buff;
		stn.szInfoTitle = Translate(MODULE " Information");
		stn.szProto = MODULE;
		stn.uTimeout = 10;
		CallService(MS_CLIST_SYSTRAY_NOTIFY, 0, (LPARAM)&stn);
	}
}

#define WMP_PAUSE	0x8028 //32808
#define WMP_NEXT	0x497B //18811

void WMPPause() {
	HWND hWnd = FindWindow(0, "Windows Media Player");
	if(hWnd)
		PostMessage(hWnd, WM_COMMAND, WMP_PAUSE, 0);
}

