#ifndef _PLUGWIN
#define _PLUGWIN

#define TIMER_ID	19191
#define TIMER_ID2	19192
#define TIMER_ID3	19193

#include "common.h"
#include "resource.h"

#include "timezone.h"
#include "time_convert.h"

#define PLUG		"WorldTime"

#define DEFAULT_MINMAX		false

#define DEFAULT_WINDOW_X	100
#define DEFAULT_WINDOW_Y	100
#define DEFAULT_WINDOW_WIDTH	200
#define DEFAULT_WINDOW_HEIGHT	100

#define MAX_NAME_LENGTH	256
#define MAX_TIME_LENGTH	256

typedef struct tagLISTITEM {
	int cbSize;
	TCHAR pszText[MAX_NAME_LENGTH];
	int timezone_list_index;
	SYSTEMTIME sunset;
	SYSTEMTIME sunrise;
	TCHAR pszTimeText[MAX_NAME_LENGTH];
	TCHAR pszDateText[MAX_NAME_LENGTH];
	HICON icon;
} LISTITEM;

typedef OBJLIST<LISTITEM> ITEMLIST;

extern HICON upIcon, downIcon;

extern ITEMLIST listbox_items;

extern TCHAR format_string[];

int addmypluginwindow1(HWND parent);
void plugwin_cleanup();


int WorldTimeOptInit(WPARAM wParam,LPARAM lParam);
void __stdcall	ShowPopup( const char* line1, const char* line2, int flags );

#define WinVerMajor()      LOBYTE(LOWORD(GetVersion()))
#define WinVerMinor()      HIBYTE(LOWORD(GetVersion()))
#define IsWinVer2000Plus() (WinVerMajor()>=5)

#define CLCDEFAULT_BKCOLOUR      GetSysColor(COLOR_3DFACE)
#define CLCDEFAULT_SELBKCOLOUR   GetSysColor(COLOR_HIGHLIGHT)
#define CLCDEFAULT_SELTEXTCOLOUR GetSysColor(COLOR_HIGHLIGHTTEXT)
#define CLCDEFAULT_TEXTCOLOUR	 GetSysColor(COLOR_WINDOWTEXT)

int InitIcons(WPARAM wParam, LPARAM lParam);
int LoadIcons(WPARAM wParam, LPARAM lParam);

extern HINSTANCE hInst;

extern ITEMLIST listbox_items;

extern TCHAR format_string[512], date_format_string[512];

extern HWND pluginwind, hwnd_clist;

extern bool hook_window_behaviour_to_clist;
extern bool set_format;
extern bool show_icons;
extern bool hide_menu;

extern HFONT ContactFont;
extern COLORREF ContactFontColour;

HFONT GetFont();

int FillList(WPARAM wParam, LPARAM lParam);



#endif
