#ifndef _MYWINDOWLIST_INC
#define _MYWINDOWLIST_INC

bool InList(HANDLE hContact, HWND hWnd);
void AddToWindowList(HANDLE hContact, HWND hWnd);
void RemoveFromWindowList(HANDLE hContact, HWND hWnd);

void PostMessageWindowList(UINT msg, WPARAM wParam, LPARAM lParam);
void PostMessageWindowListContact(HANDLE hContact, UINT msg, WPARAM wParam, LPARAM lParam);


int CountList(HANDLE hContact);
bool EmptyList(HANDLE hContact);

void InitWindowList();
void DeinitWindowList();

#endif
