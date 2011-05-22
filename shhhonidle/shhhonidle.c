////////////////////////////////////
/// This Miranda plugin (http://www.miranda-im.org) is released under the General Public Licence,
/// available at http://www.gnu.org/copyleft/gpl.html
/// Copyright Scott Ellis 2005 (mail@scottellis.com.au .. www.scottellis.com.au)
////////////////////////////////////

#include "common.h"
#include "options.h"

HINSTANCE hInst;
PLUGINLINK *pluginLink;

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	"Shhh on Idle",
	PLUGIN_MAKE_VERSION(0,1,0,0),
	"Disable sounds (and optionally PopUps) when Miranda goes idle",
	"Scott Ellis",
	"mail@scottellis.com.au",
	"© 2005-2007 Scott Ellis",
	"http://www.scottellis.com.au/",
	0,		//not transient
	0,		//doesn't replace anything built-in
	{ 0xe36dca71, 0x7b2, 0x4b98, { 0x99, 0xab, 0xa1, 0x89, 0x2d, 0x8e, 0x5c, 0xfc } } // {E36DCA71-07B2-4b98-99AB-A1892D8E5CFC}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}

__declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	pluginInfo.cbSize = sizeof(PLUGININFO);
	return (PLUGININFO*)&pluginInfo;
}

__declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

static const MUUID interfaces[] = {MIID_SHHHONIDLE, MIID_LAST};
__declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}


HANDLE hook_idle = 0;
BOOL is_idle = FALSE;
UINT alarm_timer_id = 0;

//#define CONTROL_GLOBAL_STATUS
#define CONTROL_SOUND

#ifdef CONTROL_GLOBAL_STATUS
WORD saved_status;
#endif

#define ALARM_ON_TIME		(1000 * 60 * 10) // alow sound for 10 mins when an alarm with sound notification is triggered

void EnableSounds(BOOL enable) {
	//BOOL enabled = (BOOL)DBGetContactSettingByte(0, "Skin", "UseSound", 1);

	DBWriteContactSettingByte(0, "Skin", "UseSound", enable ? 1 : 0);
}

int OnIdleChanged(WPARAM wParam, LPARAM lParam) {
#ifdef CONTROL_GLOBAL_STATUS
	int st;
#endif

	is_idle = (lParam & IDF_ISIDLE);

#ifdef CONTROL_SOUND
	EnableSounds(!is_idle);
#endif

#ifdef CONTROL_GLOBAL_STATUS
	st = CallService(MS_CLIST_GETSTATUSMODE, 0, 0);
	if(is_idle && st != ID_STATUS_OFFLINE) {
		saved_status = st;
		CallService(MS_CLIST_SETSTATUSMODE, (WPARAM)ID_STATUS_AWAY, 0);	
	} else if(!is_idle && saved_status != ID_STATUS_OFFLINE) {
		CallService(MS_CLIST_SETSTATUSMODE, (WPARAM)saved_status, 0);
	}
#endif

	if(!is_idle && alarm_timer_id)
		KillTimer(0, alarm_timer_id);

	return 0;
}

VOID CALLBACK AlarmTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
	KillTimer(0, alarm_timer_id);

	if(is_idle) EnableSounds(FALSE);
}

// do not prevent alarm noise
// (i.e. re-enable sound on any alarm with sound, for a period of time)
int OnAlarm(WPARAM wParam, LPARAM lParam) {
	ALARMINFO *al = (ALARMINFO *)lParam;

	if(al && al->sound_num > 0) {
		EnableSounds(TRUE);
		if(alarm_timer_id) KillTimer(0, alarm_timer_id);
		alarm_timer_id = SetTimer(0, 0, ALARM_ON_TIME, AlarmTimerProc);
	}

	return 0;
}

int OnModulesLoaded(WPARAM wParam, LPARAM lParam) {
	hook_idle = HookEvent(ME_IDLE_CHANGED, OnIdleChanged);
	
#ifdef CONTROL_SOUND
	EnableSounds(TRUE);

	// do not prevent alarm noise
	HookEvent(ME_ALARMS_TRIGGERED, OnAlarm);
#endif

	return 0;
}

int __declspec(dllexport) Load(PLUGINLINK *link)
{
	pluginLink = link;
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);

	HookEvent(ME_OPT_INITIALISE, OptInit);
	LoadOptions();

	return 0;
}

int __declspec(dllexport) Unload(void)
{
	UnhookEvent(hook_idle);
	return 0;
}
