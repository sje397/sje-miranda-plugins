#ifndef _POPUPS_INC
#define _POPUPS_INC

#include <m_popup.h>

#include "options.h"
#include "mywindowlist.h"

void InitUtils();
void DeinitUtils();

void ShowPopup(HANDLE hContact, const char* line1, const char* line2, int flags = 0);
void ShowWarning(char *msg);
void ShowError(char *msg);

bool IsUnicodePopupsEnabled();
void ShowPopupW(HANDLE hContact, const wchar_t* line1, const wchar_t* line2, int flags = 0);

#define WMU_CLOSEPOPUP		(WM_USER + 200)

#endif
