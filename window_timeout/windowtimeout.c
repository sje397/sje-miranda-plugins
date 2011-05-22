#include "windowtimeout.h"

HINSTANCE hInst;
PLUGINLINK *pluginLink;
struct MM_INTERFACE mmi;

PLUGININFO pluginInfo={
	sizeof(PLUGININFO),
	PLUG,
	PLUGIN_MAKE_VERSION(0,0,0,3),
	"Hides message windows after a specified period of inactivity",
	"Scott Ellis",
	"mail@scottellis.com.au",
	"© 2005 Scott Ellis",
	"http://www.scottellis.com.au/",
	0,		//not transient
	0		//doesn't replace anything built-in
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}

__declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	return &pluginInfo;
}

HANDLE hook_idle, hook_dbevent, hook_dbcontact, hook_window, hook_typing;

int OnIdleChanged(WPARAM wParam, LPARAM lParam) {
	return 0;
}

int OnWindowEvent(WPARAM wParam, LPARAM lParam) {
	MessageWindowEventData *mwed = (MessageWindowEventData *)lParam;
	if(mwed->uType == MSG_WINDOW_EVT_OPENING || mwed->uType == MSG_WINDOW_EVT_OPEN) {
		add_entry(mwed->hContact);
		set_window_handle(mwed->hContact, mwed->hwndWindow);
	} else if(mwed->uType == MSG_WINDOW_EVT_CLOSING || mwed->uType == MSG_WINDOW_EVT_CLOSE)
		remove_entry(mwed->hContact);
	return 0;
}

int OnDbEvent(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;
	if(options.monitor_recv_only == FALSE) {
		add_entry(hContact);
		reset_timer(hContact);
	} else {
		DBEVENTINFO *dbe = (HANDLE)lParam;
		if(dbe->flags & DBEF_SENT) {
			struct Entry *entry = get_entry(hContact);
			msg("sent event");
			if(entry && entry->timer_id == 0) {
				msg("closing window");
				SendMessage(entry->hwnd, WM_CLOSE, 0, 0);
			}
			return 0;
		}
		if(dbe->flags & DBEF_READ) {
			msg("read event");
			return 0;
		}

		add_entry(hContact);
		reset_timer(hContact);
	}
	return 0;
}


int OnTyping(WPARAM wParam, LPARAM lParam) {
	if(lParam) reset_timer((HANDLE)wParam);

	return 0;
}

int OnContactDeleted(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;
	remove_entry(hContact);
	return 0;
}

int OnModulesLoaded(WPARAM wParam, LPARAM lParam) {
	hook_idle = HookEvent(ME_IDLE_CHANGED, OnIdleChanged);
	hook_dbevent = HookEvent(ME_DB_EVENT_FILTER_ADD, OnDbEvent);
	hook_window = HookEvent(ME_MSG_WINDOWEVENT, OnWindowEvent);
	hook_dbcontact = HookEvent(ME_DB_CONTACT_DELETED, OnContactDeleted);
	hook_typing = HookEvent(ME_PROTO_CONTACTISTYPING, OnTyping);
	return 0;
}

int __declspec(dllexport) Load(PLUGINLINK *link)
{
	PROTOCOLDESCRIPTOR pd;
	HANDLE hContact;

	pluginLink = link;

	CallService(MS_SYSTEM_GET_MMI, 0, (LPARAM)&mmi);

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_OPT_INITIALISE, OptInit);

	load_options();
	return 0;
}

int __declspec(dllexport) Unload(void)
{
	if(hook_idle) UnhookEvent(hook_idle);
	if(hook_dbevent) UnhookEvent(hook_dbevent);
	if(hook_window) UnhookEvent(hook_window);
	if(hook_dbcontact) UnhookEvent(hook_dbcontact);
	if(hook_typing) UnhookEvent(hook_typing);
	return 0;
}


void msg(char *msg) {
	//MessageBox(0, msg, "Message", MB_OK);
}