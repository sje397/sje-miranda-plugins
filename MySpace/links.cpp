/*
Copyright © 2003 Robert Rainwater, 2007 Scott Ellis
adapted for MySpace, 2007 Scott Ellis

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "common.h"
#include "links.h"
#include "server_con.h"

#include <malloc.h>
#include <cstdlib>

#define MYSPACEWATCHERCLASS _T("__MYSPACEWatcherClass__")

static ATOM aWatcherClass = 0;
static HWND hWatcher = NULL;

static LRESULT CALLBACK myspace_links_watcherwndproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
        case WM_COPYDATA:
        {
            char *szData, *s;
            COPYDATASTRUCT *cds = (COPYDATASTRUCT *) lParam;

            //LOG(LOG_DEBUG, "Links: WM_COPYDATA");
            // Check to see if link support is enabled
            // We shouldn't need this check since the class instance shouldn't be running
            // but lets be safe
            if (!DBGetContactSettingByte(NULL, MODULE, "EnableLinkHandling", 0))
                break;
            if (!(char *) cds->lpData)
                break;
            s = szData = strdup((char *) cds->lpData);

            s += 5;
			if (!_strnicmp(s, "addContact?", 11)) {
				s =strstr(s, "cID=");
				if(s) {
					s += 4;
					char *t = strchr(s, '&');
					if(t) *t = 0;
					
					HANDLE hContact;
					int uid;
					for(t = strtok(s, ","); t ;t = strtok(0, ",")) {
						uid = atoi(t);
						hContact = FindContact(uid);
						if(!hContact) {
							hContact = CreateContact(uid, 0, 0, true);
							LookupUID(uid);
						}
					}
				}
			}
            else if (_strnicmp(s, "sendIM?", 7) == 0 && ServiceExists(MS_MSG_SENDMESSAGE))
			{
				s=strrchr(s, '=');
				if(s) {
					s++;
					int uid = atoi(s);

					HANDLE hContact = FindContact(uid);
					if(!hContact) {
						hContact = CreateContact(uid, 0, 0, false);
						DBWriteContactSettingByte(hContact, "CList", "NotOnList", 1);
						DBWriteContactSettingByte(hContact, "CList", "Hidden", 1);
						LookupUID(uid);
					}
					CallService(MS_MSG_SENDMESSAGE, (WPARAM)hContact, 0);
				}
			}

			free(szData);
        }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

static void myspace_links_regwatcher()
{
    WNDCLASS wc;

    //LOG(LOG_DEBUG, "Links: regwatcher");
    if (hWatcher || aWatcherClass) {
        return;
    }
    memset(&wc, 0, sizeof(WNDCLASS));
    wc.lpfnWndProc = myspace_links_watcherwndproc;
    wc.hInstance = hInst;
    wc.lpszClassName = MYSPACEWATCHERCLASS;
    aWatcherClass = RegisterClass(&wc);
    hWatcher = CreateWindowEx(0, MYSPACEWATCHERCLASS, _T(""), 0, 0, 0, 0, 0, NULL, NULL, hInst, NULL);
    return;
}

static void myspace_links_unregwatcher()
{
    //LOG(LOG_DEBUG, "Links: unregwatcher");
    if (hWatcher) {
        DestroyWindow(hWatcher);
        hWatcher = NULL;
    }
    if (aWatcherClass) {
        UnregisterClass(MYSPACEWATCHERCLASS, hInst);
        aWatcherClass = 0;
    }
    return;
}

static BOOL CALLBACK myspace_linsk_enumthreadwindowsproc(HWND hwnd, LPARAM lParam)
{
    TCHAR szBuf[64];

    if (GetClassName(hwnd, szBuf, 64)) {
        if (!lstrcmp(szBuf, MYSPACEWATCHERCLASS)) {
			COPYDATASTRUCT cds;

            //LOG(LOG_DEBUG, "Links: enumthreadwindowsproc - found AIMWATCHERCLASS");
            cds.dwData = 1;
            cds.cbData = strlen((char *) lParam) + 1;
            cds.lpData = (char *) lParam;
            SendMessageTimeout(hwnd, WM_COPYDATA, (WPARAM) hwnd, (LPARAM) & cds, SMTO_ABORTIFHUNG | SMTO_BLOCK, 150, NULL);
        }
    };
    return TRUE;
}

static BOOL CALLBACK myspace_links_enumwindowsproc(HWND hwnd, LPARAM lParam)
{
    char szBuf[32];

    //LOG(LOG_DEBUG, "Links: enumwindowsproc");
    if (GetClassNameA(hwnd, szBuf, 32))
	{
        if (!strcmp(szBuf, MIRANDACLASS)) {
            //LOG(LOG_DEBUG, "Links: enumwindowsproc - found Miranda window");
            EnumThreadWindows(GetWindowThreadProcessId(hwnd, NULL), myspace_linsk_enumthreadwindowsproc, lParam);
        }
    }
    return TRUE;
}

static void myspace_links_register()
{
    HKEY hkey;
    char szBuf[MAX_PATH], szExe[MAX_PATH * 2], szShort[MAX_PATH];


    GetModuleFileNameA(hInst, szBuf, sizeof(szBuf));
    GetShortPathNameA(szBuf, szShort, sizeof(szShort));
    //LOG(LOG_DEBUG, "Links: register");
    if (RegCreateKeyA(HKEY_CLASSES_ROOT, "myim", &hkey) == ERROR_SUCCESS) {
        RegSetValueA(hkey, NULL, REG_SZ, "URL:MySpace IM Protocol", strlen("URL:MySpace IM Protocol"));
        RegSetValueExA(hkey, "URL Protocol", 0, REG_SZ, (PBYTE) "", 1);
        RegCloseKey(hkey);
    }
    else {
        //LOG(LOG_ERROR, "Links: register - unable to create registry key (root)");
        return;
    }
    if (RegCreateKeyA(HKEY_CLASSES_ROOT, "myim\\DefaultIcon", &hkey) == ERROR_SUCCESS) {
        char szIcon[MAX_PATH];

        mir_snprintf(szIcon, sizeof(szIcon), "%s,0", szShort);
        RegSetValueA(hkey, NULL, REG_SZ, szIcon, strlen(szIcon));
        RegCloseKey(hkey);
    }
    else {
        //LOG(LOG_ERROR, "Links: register - unable to create registry key (DefaultIcon)");
        return;
    }
    if (RegCreateKeyA(HKEY_CLASSES_ROOT, "myim\\shell\\open\\command", &hkey) == ERROR_SUCCESS) {
        // MSVC exports differently than gcc/mingw
#ifdef _MSC_VER
        mir_snprintf(szExe, sizeof(szExe), "RUNDLL32.EXE %s,_myspace_links_exec@16 %%1", szShort);
        //LOG(LOG_INFO, "Links: registering (%s)", szExe);
#else
        mir_snprintf(szExe, sizeof(szExe), "RUNDLL32.EXE %s,myspace_links_exec@16 %%1", szShort);
        //LOG(LOG_INFO, "Links: registering (%s)", szExe);
#endif
        RegSetValueA(hkey, NULL, REG_SZ, szExe, strlen(szExe));
        RegCloseKey(hkey);
    }
    else {
        //LOG(LOG_ERROR, "Links: register - unable to create registry key (command)");
        return;
    }

}

void myspace_links_unregister()
{
    //LOG(LOG_DEBUG, "Links: unregister");
    RegDeleteKeyA(HKEY_CLASSES_ROOT, "myim\\shell\\open\\command");
    RegDeleteKeyA(HKEY_CLASSES_ROOT, "myim\\shell\\open");
    RegDeleteKeyA(HKEY_CLASSES_ROOT, "myim\\shell");
    RegDeleteKeyA(HKEY_CLASSES_ROOT, "myim\\DefaultIcon");
    RegDeleteKeyA(HKEY_CLASSES_ROOT, "myim");
}

void myspace_links_init()
{
    //LOG(LOG_DEBUG, "Links: init");
    if (DBGetContactSettingByte(NULL, MODULE, "EnableLinkHandling", 0)) {
        //LOG(LOG_DEBUG, "Links: init - links support is on");
        myspace_links_register();
        myspace_links_regwatcher();
    }
}

void myspace_links_destroy()
{
   // LOG(LOG_DEBUG, "Links: destroy");
    myspace_links_unregwatcher();
}

extern "C" void __declspec(dllexport)
     CALLBACK myspace_links_exec(HWND /*hwnd*/, HINSTANCE /*hInst*/, char *lpszCmdLine, int /*nCmdShow*/)
{
    EnumWindows(myspace_links_enumwindowsproc, (LPARAM) lpszCmdLine);
}
