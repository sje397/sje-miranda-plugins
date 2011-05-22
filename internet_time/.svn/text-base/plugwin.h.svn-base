#ifndef _PLUGWIN
#define _PLUGWIN

#define TIMER_ID2	1193
#define TIMER_ID3	1194

#include "../../include/newpluginapi.h"
#include "../../include/m_langpack.h"
#include "../../include/m_skin.h"
#include "../../include/m_database.h"
#include "../../include/m_options.h"
#include "../../include/m_popup.h"
#include "../../include/m_protosvc.h"
#include "../mwclist/CLUIFrames/m_cluiframes.h"
#include "../mwclist/m_clist.h"
#include "../mwclist/m_clui.h"
#include "../mwclist/m_clc.h"
#include "../mwclist/m_genmenu.h"
//#include "common.h"

#include <sstream>

#include "timezone.h"
#include "time_convert.h"


#define DEFAULT_MINMAX		true

#define DEFAULT_WINDOW_X	100
#define DEFAULT_WINDOW_Y	100
#define DEFAULT_WINDOW_WIDTH	200
#define DEFAULT_WINDOW_HEIGHT	100

#define MAX_NAME_LENGTH	256
#define MAX_TIME_LENGTH	256

typedef struct tagLISTITEM {
	int cbSize;
	char pszText[MAX_NAME_LENGTH];
	int timezone_list_index;
} LISTITEM;

#include <vector>
typedef std::vector<LISTITEM> ITEMLIST;

extern HANDLE upIcon, downIcon;

extern ITEMLIST listbox_items;

extern HANDLE   mainThread;

extern char format_string[];

int addmypluginwindow1(HWND parent);
void plugwin_cleanup();

#ifdef __cplusplus
extern "C" {
#endif

int InternetTimeOptInit(WPARAM wParam,LPARAM lParam);
void __stdcall	ShowPopup( const char* line1, const char* line2, int flags );

#ifdef __cplusplus
};
#endif

#define WinVerMajor()      LOBYTE(LOWORD(GetVersion()))
#define WinVerMinor()      HIBYTE(LOWORD(GetVersion()))
#define IsWinVer2000Plus() (WinVerMajor()>=5)

#define CLCDEFAULT_BKCOLOUR      GetSysColor(COLOR_3DFACE)
#define CLCDEFAULT_SELBKCOLOUR   GetSysColor(COLOR_HIGHLIGHT)
#define CLCDEFAULT_SELTEXTCOLOUR GetSysColor(COLOR_HIGHLIGHTTEXT)
#define CLCDEFAULT_TEXTCOLOUR	 GetSysColor(COLOR_WINDOWTEXT)

#endif
