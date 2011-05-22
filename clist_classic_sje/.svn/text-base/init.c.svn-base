/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2005 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#include "commonheaders.h"

HINSTANCE g_hInst = 0;
PLUGINLINK *pluginLink;
CLIST_INTERFACE* pcli = NULL;
HIMAGELIST himlCListClc = NULL;

extern int currentDesiredStatusMode;

struct MM_INTERFACE mmi;
BOOL(WINAPI * MySetLayeredWindowAttributes) (HWND, COLORREF, BYTE, DWORD) = NULL;

/////////////////////////////////////////////////////////////////////////////////////////
// external functions

void InitCustomMenus( void );
void PaintClc(HWND hwnd, struct ClcData *dat, HDC hdc, RECT * rcPaint);

int ClcOptInit(WPARAM wParam, LPARAM lParam);
int CluiOptInit(WPARAM wParam, LPARAM lParam);
int CListOptInit(WPARAM wParam, LPARAM lParam);

void ReplaceCListWndProc(CLIST_INTERFACE *pcli);
void AddMetaMods(CLIST_INTERFACE *pcli);

void InitFrames();
int FramesModulesLoaded(WPARAM wParam, LPARAM lParam);
void DeinitFrames();

/////////////////////////////////////////////////////////////////////////////////////////
// dll stub

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwReason, LPVOID reserved)
{
	g_hInst = hInstDLL;
	DisableThreadLibraryCalls(g_hInst);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// returns the plugin information

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	#if defined( _UNICODE )
	"Classic contact list (Unicode) [sje mod]",
	#else
	"Classic contact list [sje mod]",
	#endif
	PLUGIN_MAKE_VERSION(0, 3, 0, 0),
	"Display contacts, event notifications, protocol status",
	"Miranda IM project",
	"mail@scottellis.com.au",
	"Copyright 2000-2007 Miranda IM project",
	"http://www.scottellis.com.au",
	UNICODE_AWARE,
	DEFMOD_CLISTALL,
	#if defined( _UNICODE )
    //{0x240a91dc, 0x9464, 0x457a, { 0x97, 0x87, 0xff, 0x1e, 0xa8, 0x8e, 0x77, 0xe3 }} //{240A91DC-9464-457a-9787-FF1EA88E77E3}
	{ 0x345c2f5b, 0x3fd3, 0x4cae, { 0xa9, 0xc0, 0xa, 0x1f, 0x5, 0x38, 0xa3, 0xb1 } } // {345C2F5B-3FD3-4cae-A9C0-0A1F0538A3B1}
	#else
    //{0x552cf71a, 0x249f, 0x4650, { 0xbb, 0x2b, 0x7c, 0xdb, 0x1f, 0xe7, 0xd1, 0x78 }} //{552CF71A-249F-4650-BB2B-7CDB1FE7D178}
	{ 0x789c7d9b, 0x45b4, 0x4768, { 0x9c, 0xfc, 0x50, 0x10, 0x25, 0x2c, 0x21, 0xa4 } } // {789C7D9B-45B4-4768-9CFC-5010252C21A4}
	#endif
};

__declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion)
{
	if (mirandaVersion < PLUGIN_MAKE_VERSION(0, 4, 3, 0))
		return NULL;
	return &pluginInfo;
}

__declspec(dllexport) PLUGININFO *MirandaPluginInfo(DWORD mirandaVersion)
{
	if (mirandaVersion < PLUGIN_MAKE_VERSION(0, 4, 3, 0))
		return NULL;
	pluginInfo.cbSize = sizeof(PLUGININFO);
	return (PLUGININFO *)&pluginInfo;
}



static const MUUID interfaces[] = {MIID_CLIST, MIID_LAST};
__declspec(dllexport) const MUUID * MirandaPluginInterfaces(void)
{
	return interfaces;
}

/////////////////////////////////////////////////////////////////////////////////////////
// called when all modules got loaded

static int OnModulesLoaded( WPARAM wParam, LPARAM lParam )
{

	himlCListClc = (HIMAGELIST) CallService(MS_CLIST_GETICONSIMAGELIST, 0, 0);

	if(ServiceExists(MS_MC_DISABLEHIDDENGROUP))
		CallService(MS_MC_DISABLEHIDDENGROUP, 1, 0);

	//HookEvent(ME_SYSTEM_MODULESLOADED, FramesModulesLoaded);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// options iniatialization

static int OnOptsInit(WPARAM wParam, LPARAM lParam)
{
	ClcOptInit(wParam, lParam);
	CluiOptInit(wParam, lParam);
	CListOptInit(wParam, lParam);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// menu status services

static int GetStatusMode(WPARAM wParam, LPARAM lParam)
{
	return pcli->currentDesiredStatusMode;
}

/////////////////////////////////////////////////////////////////////////////////////////
// main clist initialization routine

int __declspec(dllexport) CListInitialise(PLUGINLINK * link)
{
	int rc = 0;
	pluginLink = link;
	#ifdef _DEBUG
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif

	// get the internal malloc/free()
	mir_getMMI( &mmi );

	pcli = ( CLIST_INTERFACE* )CallService(MS_CLIST_RETRIEVE_INTERFACE, 0, (LPARAM)g_hInst);
	if ( (int)pcli == CALLSERVICE_NOTFOUND ) {
LBL_Error:
		MessageBoxA( NULL, "This version of plugin requires Miranda IM 0.7.0.8 or later", "Fatal error", MB_OK );
		return 1;
	}
	if ( pcli->version < 4 )
		goto LBL_Error;

	pcli->pfnPaintClc = PaintClc;

	ReplaceCListWndProc(pcli);
	AddMetaMods(pcli);
	//InitFrames();

	MySetLayeredWindowAttributes = (BOOL(WINAPI *) (HWND, COLORREF, BYTE, DWORD)) GetProcAddress(
		LoadLibraryA("user32.dll"), "SetLayeredWindowAttributes");

	CreateServiceFunction(MS_CLIST_GETSTATUSMODE, GetStatusMode);

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_OPT_INITIALISE, OnOptsInit);

	InitCustomMenus();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// a plugin loader aware of CList exports will never call this.

int __declspec(dllexport) Load(PLUGINLINK * link)
{
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// a plugin unloader

int __declspec(dllexport) Unload(void)
{
	//DeinitFrames();
	return 0;
}
