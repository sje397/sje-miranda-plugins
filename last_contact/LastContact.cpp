/*
Miranda plugin template, originally by Richard Hughes
http://miranda-icq.sourceforge.net/

This file is placed in the public domain. Anybody is free to use or
modify it as they wish with no restriction.
There is no warranty.
*/

#include "common.h"
#include "options.h"

HINSTANCE hInst;
PLUGINLINK *pluginLink;
MM_INTERFACE mmi;

HANDLE hEventWindow;

//typedef LRESULT (CALLBACK *WNDPROC)(HWND, UINT, WPARAM, LPARAM);

#define WINDOW_DATA_STACK_SIZE		10
MessageWindowEventData windowDataStack[WINDOW_DATA_STACK_SIZE];
int stackTop = 0, stackBottom = 0;
ATOM hotkey_id;
DWORD hotkey_thread_id;

void stack_push(MessageWindowEventData *data) {
	stackTop = (stackTop + 1) % WINDOW_DATA_STACK_SIZE;
	windowDataStack[stackTop] = *data;
	if(stackBottom == stackTop)
		stackBottom = (stackBottom + 1) % WINDOW_DATA_STACK_SIZE;
}

bool stack_pop(MessageWindowEventData *data) {
	if(stackTop == stackBottom) return false; // empty stack

	*data = windowDataStack[stackTop];
	// leave data on stack if we're toggling visibility
	if(!options.hide_if_visible) {
		if(stackTop == 0) stackTop = WINDOW_DATA_STACK_SIZE - 1;
		else stackTop--;
	}

	return true;
}

void stack_update(MessageWindowEventData *data) {
	if(stackTop == stackBottom) return; // empty stack

	if(windowDataStack[stackTop].hContact == data->hContact)
		windowDataStack[stackTop] = *data;
}

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	MODULE,
	PLUGIN_MAKE_VERSION(0,0,2,4),
	"Re-open the last open message window using a configurable hot key.",
	"Scott Ellis",
	"mail@scottellis.com.au",
	"© 2006 Scott Ellis",
	"http://scottellis.com.au",
	0,		//not transient
	0,		//doesn't replace anything built-in
	{ 0x199db5f3, 0x888d, 0x42df, { 0x96, 0x5, 0xc1, 0x79, 0xd, 0x7b, 0xba, 0xbd } } // {199DB5F3-888D-42df-9605-C1790D7BBABD}
};

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	pluginInfo.cbSize = sizeof(PLUGININFO);
	return (PLUGININFO*)&pluginInfo;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

static const MUUID interfaces[] = {MIID_LASTCONTACT, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}


int WindowEvent(WPARAM wParam, LPARAM lParam) {
	MessageWindowEventData *wd = (MessageWindowEventData *)lParam;

	/*
	// ignore chat room contacts?
	char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)ed->hContact, 0);
	if(proto && DBGetContactSettingByte(ed->hContact, proto, "ChatRoom", 0)) return 0; 
	*/

	switch(wd->uType) {
	case MSG_WINDOW_EVT_OPENING:
		break;
	case MSG_WINDOW_EVT_OPEN:
		// update top of stack data
		if(options.hide_if_visible) 
			stack_update(wd);
		break;
	case MSG_WINDOW_EVT_CLOSING:
		break;
	case MSG_WINDOW_EVT_CLOSE:
		stack_push(wd);
		break;
	}

	return 0;
}

void HotkeyAction() {
	MessageWindowEventData wd;
	if(stack_pop(&wd)) {
		if(options.hide_if_visible && IsWindow(wd.hwndWindow) && IsWindowVisible(wd.hwndWindow)) {
			PostMessage(wd.hwndWindow, WM_COMMAND, 1023, 0);			// chat close tab
			PostMessage(wd.hwndWindow, WM_COMMAND, IDCANCEL, 0);		// srmm/scriver close session button
			PostMessage(wd.hwndWindow, WM_COMMAND, 1025, 0);			// tabsrmm close session button
		} else {
			CallServiceSync(MS_MSG_SENDMESSAGE, (WPARAM)wd.hContact, 0);
		}
	}
}

void MakeHotkey() {
	UINT mods = 
		(options.mod_alt ? MOD_ALT : 0) 
		| (options.mod_ctrl ? MOD_CONTROL : 0) 
		| (options.mod_shift ? MOD_SHIFT : 0)
		| (options.mod_win ? MOD_WIN : 0);

	RegisterHotKey(0, hotkey_id, mods, options.vk);
}

void EventLoop(HANDLE hEvent) {
	hotkey_thread_id = GetCurrentThreadId();

	MakeHotkey();

	MSG msg;
	while(GetMessage(&msg, 0, 0, 0) > 0 && !Miranda_Terminated()) {
		if(msg.message == WM_HOTKEY) {
			HotkeyAction();
		} else if(msg.message == WMU_RESET_HOTKEY) {
			UnregisterHotKey(0, hotkey_id);
			MakeHotkey();
		} else {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	UnregisterHotKey(0, hotkey_id);
}

int ModulesLoaded(WPARAM wParam,LPARAM lParam) {
	if(ServiceExists(MS_UPDATE_REGISTER)) {
		// register with updater
		Update update = {0};
		char szVersion[16];

		update.cbSize = sizeof(Update);

		update.szComponentName = pluginInfo.shortName;
		update.pbVersion = (BYTE *)CreateVersionString(pluginInfo.version, szVersion);
		update.cpbVersion = strlen((char *)update.pbVersion);
		update.szBetaChangelogURL = "https://server.scottellis.com.au/wsvn/mim_plugs/last_contact/?op=log&rev=0&sc=0&isdir=1";

		update.szUpdateURL = UPDATER_AUTOREGISTER;
		
		// these are the three lines that matter - the archive, the page containing the version string, and the text (or data) 
		// before the version that we use to locate it on the page
		// (note that if the update URL and the version URL point to standard file listing entries, the backend xml
		// data will be used to check for updates rather than the actual web page - this is not true for beta urls)
		update.szBetaUpdateURL = "http://www.scottellis.com.au/miranda_plugins/last_contact.zip";
		update.szBetaVersionURL = "http://www.scottellis.com.au/miranda_plugins/ver_last_contact.html";
		update.pbBetaVersionPrefix = (BYTE *)MODULE " version ";
		
		update.cpbBetaVersionPrefix = strlen((char *)update.pbBetaVersionPrefix);

		CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);
	}

	mir_forkthread(EventLoop, 0);
	hEventWindow = HookEvent(ME_MSG_WINDOWEVENT, WindowEvent);
	return 0;
}

int Shutdown(WPARAM wParam, LPARAM lParam) {
	if(hEventWindow) UnhookEvent(hEventWindow);
	return 0;
}

UINT_PTR timer_id;

extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
	pluginLink = link;

	mir_getMMI(&mmi);

	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, Shutdown);

	InitOptions();

	hotkey_id = GlobalAddAtom(MAKEINTATOM(1));

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	GlobalDeleteAtom(hotkey_id);
	return 0;
}

