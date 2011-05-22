#include "common.h"
#include "worldtimeproto.h"

HINSTANCE hInst;
PLUGINLINK *pluginLink;

HANDLE hContactDblClick = 0;
HANDLE hMenuEdit;

MM_INTERFACE mmi;
LIST_INTERFACE li;

int previousMode,
	mcStatus = ID_STATUS_OFFLINE;

UINT_PTR id_timer = 0;

// protocol related services
INT_PTR GetCaps(WPARAM wParam,LPARAM lParam)
{
	int ret = 0;
    switch (wParam) {
        case PFLAGNUM_1:
			break;
        case PFLAGNUM_2:
			if(!hide_proto)
				ret =	PF2_ONLINE | PF2_LONGAWAY;
			break;
        case PFLAGNUM_3:
			if(!hide_proto)
	            ret =	PF2_ONLINE | PF2_LONGAWAY;
            break;
        case PFLAGNUM_4:
            break;
        case PFLAGNUM_5:
	        ret =	PF2_ONLINE | PF2_LONGAWAY;
			break;
        case PFLAG_UNIQUEIDTEXT:
            ret = (INT_PTR) Translate(PROTO);
            break;
        case PFLAG_MAXLENOFMESSAGE:
            ret = 2000;
            break;
        case PFLAG_UNIQUEIDSETTING:
            ret = (INT_PTR) "Nick";
            break;
    }
    return ret;
}

/** Copy the name of the protocole into lParam
* @param wParam :	max size of the name
* @param lParam :	reference to a char *, which will hold the name
*/
INT_PTR GetName(WPARAM wParam,LPARAM lParam)
{
	char *name = (char *)Translate(PROTO);
	size_t size = min(strlen(name),wParam-1);	// copy only the first size bytes.
	if(strncpy((char *)lParam,name,size)==NULL)
		return 1;
	((char *)lParam)[size]='\0';
	return 0;
}

/** Loads the icon corresponding to the status
* Called by the CList when the status changes.
* @param wParam :	one of the following values : \n
					<tt>PLI_PROTOCOL | PLI_ONLINE | PLI_OFFLINE</tt>
* @return			an \c HICON in which the icon has been loaded.
*/
INT_PTR LoadIcon(WPARAM wParam,LPARAM lParam) 
{
	
	UINT id;
    switch (wParam & 0xFFFF)
	{
        case PLI_PROTOCOL:
			id = IDI_ICON_PROTO;
			break;
		case PLI_ONLINE:
			id = IDI_ICON_PROTO;
			break;
		case PLI_OFFLINE:
			id = IDI_ICON_PROTO;
			break;
		default:
            return 0;
    }

    return (INT_PTR) LoadImage(hInst, MAKEINTRESOURCE(id), IMAGE_ICON,
						GetSystemMetrics(wParam & PLIF_SMALL ? SM_CXSMICON : SM_CXICON),
						GetSystemMetrics(wParam & PLIF_SMALL ? SM_CYSMICON : SM_CYICON), 0);
	return 0;
}


/** Changes the status and notifies everybody
* @param wParam :	The new mode
* @param lParam :	Allways set to 0.
*/
INT_PTR SetStatus(WPARAM wParam,LPARAM lParam)
{
	previousMode = mcStatus;
	mcStatus = (int)wParam;
	ProtoBroadcastAck(PROTO,NULL,ACKTYPE_STATUS,ACKRESULT_SUCCESS, (HANDLE)previousMode, mcStatus);

	return 0;
}

/** Returns the current status
*/
INT_PTR GetStatus(WPARAM wParam,LPARAM lParam)
{
	return mcStatus;
}

// plugin stuff
PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	"World Time Protocol",
	PLUGIN_MAKE_VERSION(0, 1, 2, 0),
	"Adds 'contacts' showing the time at specified locations around the world.",
	"Scott Ellis",
	"mail@scottellis.com.au",
	"© 2005 Scott Ellis",
	"http://www.scottellis.com.au/",
	0,		//not transient
	0,		//doesn't replace anything built-in
	{0x2d27ffb5, 0x7117, 0x4445, { 0xa4, 0x23, 0x1d, 0xeb, 0x5a, 0xb, 0x29, 0xf2}} // {2D27FFB5-7117-4445-A423-1DEB5A0B29F2}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}

extern "C" WORLDTIMEPROTO_API PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" WORLDTIMEPROTO_API PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	pluginInfo.cbSize = sizeof(PLUGININFO);
	return (PLUGININFO*)&pluginInfo;
}

static const MUUID interfaces[] = {MIID_PROTOCOL, MIID_WORLDTIME, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

int ContactDblClick(WPARAM wParam, LPARAM lParam) {
	char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0);
	if(proto && strcmp(proto, PROTO) == 0)
		show_time((HANDLE)wParam);
	return 0;
}

//////////////////////////////////////////////////////////
/// Copied from MSN plugin - sent acks need to be from different thread :(
//////////////////////////////////////////////////////////
typedef struct tag_TFakeAckParams
{
	HANDLE	hEvent;
	HANDLE	hContact;
} TFakeAckParams;


static DWORD CALLBACK sttFakeAckInfoSuccess( LPVOID param )
{
	TFakeAckParams *tParam = ( TFakeAckParams* )param;
	WaitForSingleObject( tParam->hEvent, INFINITE );

	Sleep( 100 );
	ProtoBroadcastAck(PROTO, tParam->hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, ( HANDLE )1, 0 );

	CloseHandle( tParam->hEvent );
	free(tParam);
	return 0;
}

INT_PTR GetInfo(WPARAM wParam, LPARAM lParam) {
    CCSDATA *ccs = (CCSDATA *) lParam;
	DWORD dwThreadId;
	HANDLE hEvent;
	TFakeAckParams *tfap;

	hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	tfap = (TFakeAckParams *)malloc(sizeof(TFakeAckParams));
	tfap->hContact = ccs->hContact;
	tfap->hEvent = hEvent;

	CloseHandle( CreateThread( NULL, 0, sttFakeAckInfoSuccess, tfap, 0, &dwThreadId ));
	SetEvent( hEvent );
	
	return 0;
}

INT_PTR EditContact(WPARAM wParam, LPARAM lParam) {
	LISTITEM pa;
	HWND hwndList = (HWND)CallService(MS_CLUI_GETHWND, 0, 0);
	DBVARIANT dbv;

	pa.hContact = (HANDLE)wParam;
	pa.timezone_list_index = DBGetContactSettingDword(pa.hContact, PROTO, "TimezoneListIndex", -1);
	if(!DBGetContactSettingTString(pa.hContact, PROTO, "Nick", &dbv)) {
		_tcsncpy(pa.pszText, dbv.ptszVal, MAX_NAME_LENGTH);
		DBFreeVariant(&dbv);
	}
	add_edit_item = pa;
	if(DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG2), hwndList, DlgProcOptsEdit) == IDOK) {
		pa = add_edit_item;

		DBWriteContactSettingTString(pa.hContact, PROTO, "Nick", pa.pszText);
		DBWriteContactSettingDword(pa.hContact, PROTO, "TimezoneListIndex", pa.timezone_list_index);
		DBWriteContactSettingWord(pa.hContact, PROTO, "Status", ID_STATUS_ONLINE);
		return 0;
	}
	return 1;
}

HANDLE hServices[7];

void CreatePluginServices() {
	// protocol
	hServices[0] = CreateProtoServiceFunction(PROTO,PS_GETCAPS, GetCaps);
	hServices[1] = CreateProtoServiceFunction(PROTO,PS_GETNAME, GetName);
	hServices[2] = CreateProtoServiceFunction(PROTO,PS_SETSTATUS, SetStatus);
	hServices[3] = CreateProtoServiceFunction(PROTO,PS_GETSTATUS, GetStatus);
	hServices[4] = CreateProtoServiceFunction(PROTO,PS_LOADICON,LoadIcon);
	hServices[5] = CreateProtoServiceFunction(PROTO,PSS_GETINFO,GetInfo);

	// menu
	hServices[6] = CreateServiceFunction(PROTO "/Edit", EditContact);
}

void DestroyServiceFunctions() {
	for(int i = 0; i < 7; i++)
		DestroyServiceFunction(hServices[i]);
}


void replaceStr(TCHAR* str, size_t nchar, TCHAR* rstr)
{
	size_t lenr = _tcslen(rstr);
	size_t lenNew = (_tcslen(str) - nchar + 1) + lenr;

	memmove(str + lenr, str + nchar, (lenNew - lenr) * sizeof(TCHAR));
	memcpy(str, rstr, lenr * sizeof(TCHAR));
}



VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
	SYSTEMTIME st, other_st;
	TCHAR date_buf[512], time_buf[512];
	
	MyGetSystemTime(&st);

	TIME_ZONE_INFORMATION here_tzi = {0}, tzi = {0};
	LISTITEM pa;
	DBVARIANT dbv;
	WORD status;
	char *proto;

	BOOL daylight_here = (GetTimeZoneInformation(&here_tzi) == TIME_ZONE_ID_DAYLIGHT);

	HANDLE hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
	while ( hContact != NULL ) {
		proto = ( char* )CallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM )hContact,0 );
		if ( proto && !strcmp( PROTO, proto)) {
			pa.hContact = hContact;
			pa.timezone_list_index = DBGetContactSettingDword(pa.hContact, PROTO, "TimezoneListIndex", -1);
			if(!DBGetContactSettingTString(pa.hContact, PROTO, "Nick", &dbv)) {
				_tcsncpy(pa.pszText, dbv.ptszVal, MAX_NAME_LENGTH);
				DBFreeVariant(&dbv);
			}
			status = DBGetContactSettingWord(pa.hContact, PROTO, "Status", ID_STATUS_ONLINE);

			tzi.Bias = timezone_list[pa.timezone_list_index].TZI.Bias;
			tzi.DaylightBias = timezone_list[pa.timezone_list_index].TZI.DaylightBias;
			tzi.DaylightDate = timezone_list[pa.timezone_list_index].TZI.DaylightDate;
			tzi.StandardBias = timezone_list[pa.timezone_list_index].TZI.StandardBias;
			tzi.StandardDate = timezone_list[pa.timezone_list_index].TZI.StandardDate;

			MySystemTimeToTzSpecificLocalTime(&tzi, &st, &other_st);

			if(set_format) {
				GetTimeFormat(LOCALE_USER_DEFAULT, 0, &other_st, format_string, time_buf, 512);
				GetDateFormat(LOCALE_USER_DEFAULT, 0, &other_st, date_format_string, date_buf, 512);
			} else {
				GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &other_st, 0, time_buf, 512);
				GetDateFormat(LOCALE_USER_DEFAULT, 0, &other_st, 0, date_buf, 512);
			}

			
			TCHAR buf[1024];
			TCHAR* pos;
			_tcscpy(buf, clist_format_string);
			pos = _tcsstr(buf, _T("%n"));
			if (pos != NULL)
				replaceStr(pos, 2, pa.pszText);
			pos = _tcsstr(buf, _T("%t"));
			if (pos != NULL)
				replaceStr(pos, 2, time_buf);
			pos = _tcsstr(buf, _T("%d"));
			if (pos != NULL)
				replaceStr(pos, 2, date_buf);

			pos = _tcsstr(buf, _T("%o"));
			if(pos != NULL)
			{
				int other_offset, offset;
				
				if(tzi.StandardDate.wMonth) {
					if(tzi.DaylightDate.wMonth < tzi.StandardDate.wMonth) {
						if(LocalGreaterThanTransition(&other_st, &tzi.DaylightDate)
							&& !LocalGreaterThanTransition(&other_st, &tzi.StandardDate))
						{
							other_offset = tzi.Bias + tzi.DaylightBias;
						} else
							other_offset = tzi.Bias;

					} else {
						if(!LocalGreaterThanTransition(&other_st, &tzi.StandardDate)
							|| LocalGreaterThanTransition(&other_st, &tzi.DaylightDate))
						{
							other_offset = tzi.Bias + tzi.DaylightBias;
						} else
							other_offset = tzi.Bias;
					}
				} else
					other_offset = tzi.Bias;

				TCHAR temp_buf[8];

				unsigned char icq_offset = (unsigned char)((other_offset / 60) * 2) + (other_offset % 60) / 30;

				if(DBGetContactSettingByte(pa.hContact, PROTO, "Timezone", 256) != icq_offset)
					DBWriteContactSettingByte(pa.hContact, PROTO, "Timezone", icq_offset);

				offset = here_tzi.Bias + (daylight_here ? here_tzi.DaylightBias : 0) - other_offset;
				//if(offset > 12 * 60) offset -= 24 * 60;
				//if(offset < -12 * 60) offset += 24 * 60;

				mir_sntprintf(temp_buf, SIZEOF(temp_buf), _T("%+g"), offset/60.0);
				replaceStr(pos, 2, temp_buf);
			}

			if(!DBGetContactSettingTString(pa.hContact, "CList", "MyHandle", &dbv)) {
				// only write if times are different
				if(_tcscmp(dbv.ptszVal, buf))			
					DBWriteContactSettingTString(pa.hContact, "CList", "MyHandle", buf);
				DBFreeVariant(&dbv);
			} else
				DBWriteContactSettingTString(pa.hContact, "CList", "MyHandle", buf);

			if(!DBGetContactSettingTString(pa.hContact, PROTO, "FirstName", &dbv)) {
				// only write if times are different
				if(_tcscmp(dbv.ptszVal, time_buf))			
					DBWriteContactSettingTString(pa.hContact, PROTO, "FirstName", time_buf);
				DBFreeVariant(&dbv);
			} else
				DBWriteContactSettingTString(pa.hContact, PROTO, "FirstName", time_buf);

			if(set_format) {
				if(!DBGetContactSettingTString(pa.hContact, PROTO, "LastName", &dbv)) {
					// only write if times are different
					if(_tcscmp(dbv.ptszVal, date_buf))			
						DBWriteContactSettingTString(pa.hContact, PROTO, "LastName", date_buf);
					DBFreeVariant(&dbv);
				} else
					DBWriteContactSettingTString(pa.hContact, PROTO, "LastName", date_buf);
			} else
				DBDeleteContactSetting(pa.hContact, PROTO, "LastName");

			if(other_st.wHour >= 6 && other_st.wHour < 18) {
				if(status != ID_STATUS_ONLINE)
					DBWriteContactSettingWord(pa.hContact, PROTO, "Status", ID_STATUS_ONLINE);
			} else {
				if(status != ID_STATUS_NA)
					DBWriteContactSettingWord(pa.hContact, PROTO, "Status", ID_STATUS_NA);
			}
		}
		hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
	}
}

int OnShutdown(WPARAM wParam, LPARAM lParam) {
	time_windows_cleanup();
	return 0;
}

int OnModulesLoaded(WPARAM wParam, LPARAM lParam) {
	if(ServiceExists(MS_UPDATE_REGISTER)) {
		// register with updater
		Update update = {0};
		char szVersion[16];

		update.cbSize = sizeof(Update);

		update.szComponentName = pluginInfo.shortName;
		update.pbVersion = (BYTE *)CreateVersionString(pluginInfo.version, szVersion);
		update.cpbVersion = (int)strlen((char *)update.pbVersion);

		update.szUpdateURL = UPDATER_AUTOREGISTER;
		
		// these are the three lines that matter - the archive, the page containing the version string, and the text (or data) 
		// before the version that we use to locate it on the page
		// (note that if the update URL and the version URL point to standard file listing entries, the backend xml
		// data will be used to check for updates rather than the actual web page - this is not true for beta urls)
		update.szBetaUpdateURL = "http://www.scottellis.com.au/miranda_plugins/worldtimeproto.zip";
		update.szBetaVersionURL = "http://www.scottellis.com.au/miranda_plugins/ver_worldtimeproto.html";
		update.pbBetaVersionPrefix = (BYTE *)"World Time Protocol version ";
		
		update.cpbBetaVersionPrefix = (int)strlen((char *)update.pbBetaVersionPrefix);

		CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);
	}

	
	// for database editor++ ver 3+
	if(ServiceExists("DBEditorpp/RegisterSingleModule"))
		CallService("DBEditorpp/RegisterSingleModule",(WPARAM)PROTO,0);

	//ignore status for all WORLD TIME contacts
	{
		HANDLE hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
		char *proto;
		while ( hContact != NULL )
		{
			proto = ( char* )CallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM )hContact,0 );
			if ( proto && !strcmp( PROTO, proto)) {
				CallService(MS_IGNORE_IGNORE, (WPARAM)hContact, (WPARAM)IGNOREEVENT_USERONLINE);
				DBDeleteContactSetting(hContact, PROTO, "WindowHandle");
			}

			hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
		}	
	}
	
	
	hContactDblClick = HookEvent(ME_CLIST_DOUBLECLICKED, ContactDblClick);

	
	{
		CLISTMENUITEM menu;

		ZeroMemory(&menu,sizeof(menu));
		menu.cbSize=sizeof(menu);
		menu.flags = CMIM_ALL;

		menu.hIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON_PROTO), IMAGE_ICON, 0, 0, 0);

		menu.position = -200000;
		menu.pszName = "Edit...";
		menu.pszService=PROTO "/Edit";
		menu.pszContactOwner = PROTO;
		hMenuEdit = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&menu);

		DestroyIcon(menu.hIcon);
	}
	
	build_timezone_list();
	time_windows_init();

	id_timer = SetTimer(0, 0, 950, TimerProc);

	return 0;
}

HANDLE hEventModulesLoaded, hEventOptInit, hEventPreShutdown;
extern "C" WORLDTIMEPROTO_API int Load(PLUGINLINK *link)
{
	pluginLink=link;

	mir_getMMI(&mmi);
	mir_getLI(&li);

	previousMode = mcStatus = ID_STATUS_OFFLINE;

	PROTOCOLDESCRIPTOR pd = {0};
	pd.cbSize=sizeof(pd);
	pd.szName=PROTO;
	pd.type = PROTOTYPE_PROTOCOL;
	CallService(MS_PROTO_REGISTERMODULE,0,(LPARAM)&pd);

	CreatePluginServices();

	LoadOptions();

	hEventModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);	

	hEventOptInit = HookEvent(ME_OPT_INITIALISE, OptInit);

	hEventPreShutdown = HookEvent(ME_SYSTEM_PRESHUTDOWN, OnShutdown);

	return 0;
}

extern "C" WORLDTIMEPROTO_API int Unload(void)
{
	UnhookEvent(hEventModulesLoaded);
	UnhookEvent(hEventOptInit);
	UnhookEvent(hEventPreShutdown);

	KillTimer(0, id_timer);

	DestroyServiceFunctions();

	UnhookEvent(hContactDblClick);
	return 0;
}
