#include "common.h"
#include "notifications.h"
#include "resource.h"
#include "server_con.h"

#include <malloc.h>
#include <ctime>

#define URL_MAIL		"http://messaging.myspace.com/index.cfm?fuseaction=mail.inbox"
#define URL_BLOG		"http://blog.myspace.com/index.cfm?fuseaction=blog.controlcenter"
#define URL_PROFILE		"http://comment.myspace.com/index.cfm?fuseaction=user.viewComments&friendID=%d"
#define URL_FRIEND		"http://messaging.myspace.com/index.cfm?fuseaction=mail.friendRequests"
#define URL_PICTURE		"http://viewmorepics.myspace.com/index.cfm?fuseaction=user.viewPictureComments&friendID=%d"
#define URL_BLOGSUB		"http://blog.myspace.com/index.cfm?fuseaction=blog.controlcenter"

#define URL_HOME		"http://home.myspace.com/index.cfm?fuseaction=user"

char *zap_array[11] = {
	"zap", "whack", "torch", "smooch", "hug", "bslap", "goose", "hi-five", "punk'd", "raspberry", 0
};

bool bWndMailPopup = false, bWndBlogPopup = false, bWndProfilePopup = false, bWndFriendPopup = false, bWndPicPopup = false,
	bWndBlogSubPopup = false, bWndUnknownPopup = false;
typedef struct {
	char *url;
	bool *bWnd;
} PopupProcData;

char popup_class_name[128];

LRESULT CALLBACK NotifyPopupWindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message ) {
		case UM_INITPOPUP:
			{
				PopupProcData *pd = (PopupProcData *)PUGetPluginData(hWnd);
				if(pd) *pd->bWnd = true;
			}
			break;

		case WM_COMMAND:
			{
				PopupProcData *pd = (PopupProcData *)PUGetPluginData(hWnd);
				if(pd) {
					CallService(MS_UTILS_OPENURL, (WPARAM)TRUE, (LPARAM)pd->url); //"http://messaging.myspace.com/index.cfm?fuseaction=mail.inbox"
					*pd->bWnd = false;
				}
			}

			PUDeletePopUp( hWnd );
			return TRUE;

		case WM_CONTEXTMENU:
			{
				PopupProcData *pd = (PopupProcData *)PUGetPluginData(hWnd);
				if(pd) *pd->bWnd = false;
			}
			PUDeletePopUp( hWnd );
			return TRUE;


		case UM_FREEPLUGINDATA:
			{
				PopupProcData *pd = (PopupProcData *)PUGetPluginData(hWnd);
				if(pd) {
					free(pd->url);
					delete pd;
				}
			}
			return TRUE;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}


void NotifyMail() {
	if (!ServiceExists( MS_POPUP_ADDPOPUPT)) return;
	if(bWndMailPopup) return;
	if(!DBGetContactSettingByte(0, MODULE, "NotifyMail", 1)) return;

	POPUPDATAT ppd = {0};
	TCHAR wproto[256];
#ifndef _UNICODE
	strncpy(wproto, MODULE, 256);
#else
	MultiByteToWideChar(code_page, 0, MODULE, -1, wproto, 256);
#endif

	PopupProcData *pd = new PopupProcData;
	pd->url = strdup(URL_MAIL);
	pd->bWnd = &bWndMailPopup;

	if(ServiceExists(MS_YAPP_CLASSINSTANCE)) {
		PopupClassInstance d = {sizeof(d), popup_class_name};
		d.ptszTitle = wproto;
		d.ptszText = TranslateT("You have unread mail");
		d.opaque = (void *)pd;
		CallService(MS_YAPP_CLASSINSTANCE, 0, (LPARAM)&d);
	} else {
		_tcsncpy(ppd.lptzContactName, wproto, 256);
		_tcscpy(ppd.lptzText, TranslateT("You have unread mail"));
		ppd.lchIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_MYSPACE), IMAGE_ICON, SM_CXSMICON, SM_CYSMICON, 0);
		ppd.PluginWindowProc = ( WNDPROC )NotifyPopupWindowProc;
		ppd.PluginData = (void *)pd;
		ppd.iSeconds = -1;

		PUAddPopUpT(&ppd);
	}

	char temp[512];
	mir_snprintf(temp, 512, "%s/Mail", MODULE);
	SkinPlaySound(temp);
}


void NotifyBlogComment() {
	if (!ServiceExists( MS_POPUP_ADDPOPUPT)) return;
	if(bWndBlogPopup) return;
	if(!DBGetContactSettingByte(0, MODULE, "NotifyBlogComment", 1)) return;

	POPUPDATAT ppd = {0};
	TCHAR wproto[256];
#ifndef _UNICODE
	strncpy(wproto, MODULE, 256);
#else
	MultiByteToWideChar(code_page, 0, MODULE, -1, wproto, 256);
#endif

	PopupProcData *pd = new PopupProcData;
	pd->url = strdup(URL_BLOG);
	pd->bWnd = &bWndBlogPopup;

	if(ServiceExists(MS_YAPP_CLASSINSTANCE)) {
		PopupClassInstance d = {sizeof(d), popup_class_name};
		d.ptszTitle = wproto;
		d.ptszText = TranslateT("You have unread blog comments");
		d.opaque = (void *)pd;
		CallService(MS_YAPP_CLASSINSTANCE, 0, (LPARAM)&d);
	} else {
		_tcsncpy(ppd.lptzContactName, wproto, 256);
		_tcscpy(ppd.lptzText, TranslateT("You have unread blog comments"));
		ppd.lchIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_MYSPACE), IMAGE_ICON, SM_CXSMICON, SM_CYSMICON, 0);
		ppd.PluginWindowProc = ( WNDPROC )NotifyPopupWindowProc;
		ppd.PluginData = (void *)pd;
		ppd.iSeconds = -1;

		PUAddPopUpT(&ppd);
	}

	char temp[512];
	mir_snprintf(temp, 512, "%s/BlogComment", MODULE);
	SkinPlaySound(temp);
}

void NotifyProfileComment() {
	if (!ServiceExists( MS_POPUP_ADDPOPUPT)) return;
	if(bWndProfilePopup) return;
	if(!DBGetContactSettingByte(0, MODULE, "NotifyProfileComment", 1)) return;

	POPUPDATAT ppd = {0};
	TCHAR wproto[256];
#ifndef _UNICODE
	strncpy(wproto, MODULE, 256);
#else
	MultiByteToWideChar(code_page, 0, MODULE, -1, wproto, 256);
#endif

	PopupProcData *pd = new PopupProcData;
	char url[1024];
	mir_snprintf(url, 1024, URL_PROFILE, DBGetContactSettingDword(0, MODULE, "UID", 0));
	pd->url = strdup(url);
	pd->bWnd = &bWndProfilePopup;

	if(ServiceExists(MS_YAPP_CLASSINSTANCE)) {
		PopupClassInstance d = {sizeof(d), popup_class_name};
		d.ptszTitle = wproto;
		d.ptszText = TranslateT("You have unread profile comments");
		d.opaque = (void *)pd;
		CallService(MS_YAPP_CLASSINSTANCE, 0, (LPARAM)&d);
	} else {
		_tcsncpy(ppd.lptzContactName, wproto, 256);
		_tcscpy(ppd.lptzText, TranslateT("You have unread profile comments"));
		ppd.lchIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_MYSPACE), IMAGE_ICON, SM_CXSMICON, SM_CYSMICON, 0);
		ppd.PluginWindowProc = ( WNDPROC )NotifyPopupWindowProc;
		ppd.PluginData = (void *)pd;
		ppd.iSeconds = -1;

		PUAddPopUpT(&ppd);
	}

	char temp[512];
	mir_snprintf(temp, 512, "%s/ProfileComment", MODULE);
	SkinPlaySound(temp);
}

void NotifyFriendRequest() {
	if (!ServiceExists( MS_POPUP_ADDPOPUPT)) return;
	if(bWndFriendPopup) return;
	if(!DBGetContactSettingByte(0, MODULE, "NotifyFriendRequest", 1)) return;

	POPUPDATAT ppd = {0};
	TCHAR wproto[256];
#ifndef _UNICODE
	strncpy(wproto, MODULE, 256);
#else
	MultiByteToWideChar(code_page, 0, MODULE, -1, wproto, 256);
#endif

	PopupProcData *pd = new PopupProcData;
	pd->url = strdup(URL_FRIEND);
	pd->bWnd = &bWndFriendPopup;

	if(ServiceExists(MS_YAPP_CLASSINSTANCE)) {
		PopupClassInstance d = {sizeof(d), popup_class_name};
		d.ptszTitle = wproto;
		d.ptszText = TranslateT("You have new friend requests");
		d.opaque = (void *)pd;
		CallService(MS_YAPP_CLASSINSTANCE, 0, (LPARAM)&d);
	} else {
		_tcsncpy(ppd.lptzContactName, wproto, 256);
		_tcscpy(ppd.lptzText, TranslateT("You have new friend requests"));
		ppd.lchIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_MYSPACE), IMAGE_ICON, SM_CXSMICON, SM_CYSMICON, 0);
		ppd.PluginWindowProc = ( WNDPROC )NotifyPopupWindowProc;
		ppd.PluginData = (void *)pd;
		ppd.iSeconds = -1;

		PUAddPopUpT(&ppd);
	}

	char temp[512];
	mir_snprintf(temp, 512, "%s/FriendRequest", MODULE);
	SkinPlaySound(temp);
}

void NotifyPictureComment() {
	if (!ServiceExists( MS_POPUP_ADDPOPUPT)) return;
	if(bWndPicPopup) return;
	if(!DBGetContactSettingByte(0, MODULE, "NotifyPicComment", 1)) return;

	POPUPDATAT ppd = {0};
	TCHAR wproto[256];
#ifndef _UNICODE
	strncpy(wproto, MODULE, 256);
#else
	MultiByteToWideChar(code_page, 0, MODULE, -1, wproto, 256);
#endif

	PopupProcData *pd = new PopupProcData;
	char url[1024];
	mir_snprintf(url, 1024, URL_PICTURE, DBGetContactSettingDword(0, MODULE, "UID", 0));
	pd->url = strdup(url);
	pd->bWnd = &bWndPicPopup;

	if(ServiceExists(MS_YAPP_CLASSINSTANCE)) {
		PopupClassInstance d = {sizeof(d), popup_class_name};
		d.ptszTitle = wproto;
		d.ptszText = TranslateT("You have unread picture comments");
		d.opaque = (void *)pd;
		CallService(MS_YAPP_CLASSINSTANCE, 0, (LPARAM)&d);
	} else {
		_tcsncpy(ppd.lptzContactName, wproto, 256);
		_tcscpy(ppd.lptzText, TranslateT("You have unread picture comments"));
		ppd.lchIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_MYSPACE), IMAGE_ICON, SM_CXSMICON, SM_CYSMICON, 0);
		ppd.PluginWindowProc = ( WNDPROC )NotifyPopupWindowProc;
		ppd.PluginData = (void *)pd;
		ppd.iSeconds = -1;

		PUAddPopUpT(&ppd);
	}

	char temp[512];
	mir_snprintf(temp, 512, "%s/PictureComment", MODULE);
	SkinPlaySound(temp);
}

void NotifyBlogSubscriptPost() {
	if (!ServiceExists( MS_POPUP_ADDPOPUPT)) return;
	if(bWndBlogSubPopup) return;
	if(!DBGetContactSettingByte(0, MODULE, "NotifyBlogSubscription", 1)) return;

	POPUPDATAT ppd = {0};
	TCHAR wproto[256];
#ifndef _UNICODE
	strncpy(wproto, MODULE, 256);
#else
	MultiByteToWideChar(code_page, 0, MODULE, -1, wproto, 256);
#endif

	PopupProcData *pd = new PopupProcData;
	pd->url = strdup(URL_BLOGSUB);
	pd->bWnd = &bWndBlogSubPopup;

	if(ServiceExists(MS_YAPP_CLASSINSTANCE)) {
		PopupClassInstance d = {sizeof(d), popup_class_name};
		d.ptszTitle = wproto;
		d.ptszText = TranslateT("You have unread blog subscription posts");
		d.opaque = (void *)pd;
		CallService(MS_YAPP_CLASSINSTANCE, 0, (LPARAM)&d);
	} else {
		_tcsncpy(ppd.lptzContactName, wproto, 256);
		_tcscpy(ppd.lptzText, TranslateT("You have unread log subscription posts"));
		ppd.lchIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_MYSPACE), IMAGE_ICON, SM_CXSMICON, SM_CYSMICON, 0);
		ppd.PluginWindowProc = ( WNDPROC )NotifyPopupWindowProc;
		ppd.PluginData = (void *)pd;
		ppd.iSeconds = -1;

		PUAddPopUpT(&ppd);
	}

	char temp[512];
	mir_snprintf(temp, 512, "%s/BlogSubscriptionPost", MODULE);
	SkinPlaySound(temp);
}

void NotifyUnknown(char *name, char *value) {
	if (!ServiceExists( MS_POPUP_ADDPOPUPT)) return;
	if(bWndUnknownPopup) return;
	if(!DBGetContactSettingByte(0, MODULE, "NotifyUnknown", 1)) return;

	POPUPDATAT ppd = {0};
	TCHAR wproto[256];
#ifndef _UNICODE
	strncpy(wproto, MODULE, 256);
#else
	MultiByteToWideChar(code_page, 0, MODULE, -1, wproto, 256);
#endif

	PopupProcData *pd = new PopupProcData;
	pd->url = strdup(URL_HOME);
	pd->bWnd = &bWndUnknownPopup;

	TCHAR tname[256], tvalue[512];
#ifdef _UNICODE
	MultiByteToWideChar(CP_UTF8, 0, name, -1, tname, 256);
	MultiByteToWideChar(CP_UTF8, 0, value, -1, tvalue, 512);
#else
	strncpy(tname, name, 256);
	strncpy(tvalue, value, 512);
#endif

	TCHAR text[512];
	mir_sntprintf(text, 512, TranslateT("Unknown event:\n%s = %s\n\nClick here to go to your homepage"), tname, tvalue);

	if(ServiceExists(MS_YAPP_CLASSINSTANCE)) {
		PopupClassInstance d = {sizeof(d), popup_class_name};
		d.ptszTitle = wproto;
		d.ptszText = text;
		d.opaque = (void *)pd;
		CallService(MS_YAPP_CLASSINSTANCE, 0, (LPARAM)&d);
	} else {
		_tcsncpy(ppd.lptzContactName, wproto, 256);
		_tcscpy(ppd.lptzText, text);
		ppd.lchIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_MYSPACE), IMAGE_ICON, SM_CXSMICON, SM_CYSMICON, 0);
		ppd.PluginWindowProc = ( WNDPROC )NotifyPopupWindowProc;
		ppd.PluginData = (void *)pd;
		ppd.iSeconds = -1;

		PUAddPopUpT(&ppd);
	}
}

void NotifyZapRecv(HANDLE hContact, int zap_num) {
	char rmsg[512];
	mir_snprintf(rmsg, 512, Translate("You received a ZAP: %s"), Translate(zap_array[zap_num]));

	PROTORECVEVENT pre = {0};
	pre.flags = PREF_UTF;
	pre.szMessage = rmsg;
	pre.timestamp = (DWORD)time(0);
	pre.lParam = EVENTTYPE_ZAP;

	CCSDATA css = {0};
	css.hContact = hContact;
	css.lParam = (LPARAM)&pre;
	css.szProtoService = PSR_MESSAGE;
	
	CallService(MS_PROTO_CHAINRECV, 0, (LPARAM)&css);

	char zap_sound_name[512];
	mir_snprintf(zap_sound_name, 512, "%s/Zap/%s", MODULE, zap_array[zap_num]);
	SkinPlaySound(zap_sound_name);
}

void NotifyZapSend(HANDLE hContact, int zap_num) {
	char rmsg[512];
	mir_snprintf(rmsg, 512, "!!!ZAP_SEND!!!=RTE_BTN_ZAPS_%d", zap_num);

	ClientNetMessage msg;
	msg.add_int("bm", 121);
	msg.add_int("sesskey", sesskey);
	msg.add_int("t", DBGetContactSettingDword(hContact, MODULE, "UID", 0));
	msg.add_int("f", my_uid);
	msg.add_int("cv", CLIENT_VER);
	msg.add_string("msg", rmsg);
	SendMessage(msg);

	mir_snprintf(rmsg, 512, "You sent a ZAP: %s", zap_array[zap_num]);
	DBEVENTINFO dbe;
	dbe.cbSize = sizeof(dbe);
	dbe.szModule = MODULE;
	dbe.timestamp = (DWORD)time(0);
	dbe.flags = DBEF_SENT;
	dbe.eventType = EVENTTYPE_ZAP;
	dbe.pBlob = (BYTE*)rmsg;
	dbe.cbBlob = strlen(rmsg) + 1;

	CallService(MS_DB_EVENT_ADD,(WPARAM)hContact,(LPARAM)&dbe);
	/*
	// not required for new metas
	HANDLE hMeta;
	if(ServiceExists(MS_MC_GETMETACONTACT) && (hMeta = (HANDLE)CallService(MS_MC_GETMETACONTACT, (WPARAM)hContact, 0)) != 0) // add to metacontact too
		CallService(MS_DB_EVENT_ADD,(WPARAM)hMeta,(LPARAM)&dbe);
	*/

	char zap_sound_name[512];
	mir_snprintf(zap_sound_name, 512, "%s/Zap/%s", MODULE, zap_array[zap_num]);
	SkinPlaySound(zap_sound_name);
}

HICON hProtoIcon;
void InitNotifications() {
	SKINSOUNDDESCEX sd = {0};
	sd.cbSize = sizeof(sd);
	char sec[512], name[512], zap_desc[512];
	mir_snprintf(sec, 512, "%s Notifications", MODULE);
	sd.pszSection = sec;
	
	mir_snprintf(name, 512, "%s/Mail", MODULE);
	sd.pszName = name;
	sd.pszDescription = "Mail";
	CallService(MS_SKIN_ADDNEWSOUND, 0, (LPARAM)&sd);
	
	mir_snprintf(name, 512, "%s/BlogComment", MODULE);
	sd.pszName = name;
	sd.pszDescription = "New blog comment";
	CallService(MS_SKIN_ADDNEWSOUND, 0, (LPARAM)&sd);
	
	mir_snprintf(name, 512, "%s/ProfileComment", MODULE);
	sd.pszName = name;
	sd.pszDescription = "New profile comment";
	CallService(MS_SKIN_ADDNEWSOUND, 0, (LPARAM)&sd);
	
	mir_snprintf(name, 512, "%s/FriendRequest", MODULE);
	sd.pszName = name;
	sd.pszDescription = "New friend request";
	CallService(MS_SKIN_ADDNEWSOUND, 0, (LPARAM)&sd);

	mir_snprintf(name, 512, "%s/PictureComment", MODULE);
	sd.pszName = name;
	sd.pszDescription = "New picture comment";
	CallService(MS_SKIN_ADDNEWSOUND, 0, (LPARAM)&sd);

	mir_snprintf(name, 512, "%s/BlogSubscriptionPost", MODULE);
	sd.pszName = name;
	sd.pszDescription = "New blog subscription post";
	CallService(MS_SKIN_ADDNEWSOUND, 0, (LPARAM)&sd);

	for(int i = 0; zap_array[i]; i++) {
		mir_snprintf(name, 512, "%s/Zap/%s", MODULE, zap_array[i]);
		sd.pszName = name;
		mir_snprintf(zap_desc, 512, "Zap: %s", zap_array[i]);
		sd.pszDescription = zap_desc;
		CallService(MS_SKIN_ADDNEWSOUND, 0, (LPARAM)&sd);
	}

	hProtoIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_MYSPACE), IMAGE_ICON, SM_CXSMICON, SM_CYSMICON, 0);
	if(ServiceExists(MS_YAPP_REGISTERCLASS)) {
		TCHAR tmod[128];
#ifdef _UNICODE
		MultiByteToWideChar(code_page, 0, MODULE, -1, tmod, 128);
#else
		strncpy(tmod, MODULE, 128);
#endif

		mir_snprintf(popup_class_name, 128, "%s/Notify", MODULE);

		PopupClass test = {0};
		test.cbSize = sizeof(test);
		test.flags = PCF_TCHAR;
		test.hIcon = hProtoIcon;
		test.timeout = -1;
		test.ptszDescription = tmod;
		test.pszName = popup_class_name;
		test.windowProc = NotifyPopupWindowProc;
		CallService(MS_YAPP_REGISTERCLASS, 0, (WPARAM)&test);
	}
}

void DeinitNotifications() {
	DestroyIcon(hProtoIcon);
}


