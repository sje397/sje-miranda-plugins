#include "stdafx.h"
#include "pingproto.h"

HINSTANCE hInst;
PLUGINLINK *pluginLink;

HANDLE hNetlibUser = 0;
HANDLE hFillListEvent = 0, hContactDeleted = 0, hContactDblClick = 0, hContactSettingChanged;

#define NUM_SERVICES	22
HANDLE hServices[NUM_SERVICES] = {0};

int previousMode,
	mcStatus = ID_STATUS_OFFLINE;

// protocol related services
int GetCaps(WPARAM wParam,LPARAM lParam)
{
	int ret = 0;
    switch (wParam) {
        case PFLAGNUM_1:
			//ret = PF1_IM | PF1_URL | PF1_FILE | PF1_MODEMSG | PF1_AUTHREQ | PF1_ADDED;
			//ret = PF1_IMSEND | PF1_URLSEND | PF1_FILESEND | PF1_MODEMSGSEND;
			//ret = 0;
			break;
        case PFLAGNUM_2:
			if(!options.hide_proto) {
				//ret = PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND
				//	| PF2_HEAVYDND | PF2_FREECHAT | PF2_OUTTOLUNCH | PF2_ONTHEPHONE;			//}
				//ret = options.nrstatus | options.off_status | options.rstatus;
				ret = Proto_Status2Flag(options.rstatus) | Proto_Status2Flag(options.nrstatus) | Proto_Status2Flag(options.tstatus);
				if(options.off_status != ID_STATUS_OFFLINE)
					ret |= Proto_Status2Flag(options.off_status);
			}
			break;
        case PFLAGNUM_3:
            //ret = PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND;
            //ret =	PF2_ONLINE | PF2_LIGHTDND | PF2_HEAVYDND;
			//ret = PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND
				//| PF2_HEAVYDND | PF2_FREECHAT | PF2_OUTTOLUNCH | PF2_ONTHEPHONE;			//}
			//ret = options.nrstatus | options.off_status | options.rstatus;
			//ret = Proto_Status2Flag(options.rstatus) | Proto_Status2Flag(options.off_status);
			ret = Proto_Status2Flag(options.nrstatus) | Proto_Status2Flag(options.tstatus);
            break;
        case PFLAGNUM_4:
            break;
        case PFLAG_UNIQUEIDTEXT:
            ret = (int) Translate(PROTO);
            break;
        case PFLAG_MAXLENOFMESSAGE:
            ret = 2000;
            break;
        case PFLAG_UNIQUEIDSETTING:
            ret = (int) "Address";
            break;
    }
    return ret;
}

/** Copy the name of the protocole into lParam
* @param wParam :	max size of the name
* @param lParam :	reference to a char *, which will hold the name
*/
int GetName(WPARAM wParam,LPARAM lParam)
{
	char *name = (char *)Translate(PROTO);
#ifdef min
	size_t size = min(strlen(name),wParam-1);	// copy only the first size bytes.
#else
	size_t size = std::min(strlen(name),wParam-1);	// copy only the first size bytes.
#endif
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
int LoadIcon(WPARAM wParam,LPARAM lParam) 
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
            return (int) (HICON) NULL;
    }

    return (int) LoadImage(hInst, MAKEINTRESOURCE(id), IMAGE_ICON,
						GetSystemMetrics(wParam & PLIF_SMALL ? SM_CXSMICON : SM_CXICON),
						GetSystemMetrics(wParam & PLIF_SMALL ? SM_CYSMICON : SM_CYICON), 0);
	return 0;
}


/** Changes the status and notifies everybody
* @param wParam :	The new mode
* @param lParam :	Allways set to 0.
*/
int SetStatus(WPARAM wParam,LPARAM lParam)
{
	int new_status = (int)wParam;

	if(new_status == options.rstatus || options.nrstatus || options.off_status) {
		previousMode = mcStatus;
		mcStatus = new_status;
		ProtoBroadcastAck(PROTO,NULL,ACKTYPE_STATUS,ACKRESULT_SUCCESS, (HANDLE)previousMode, mcStatus);

		if(new_status == options.off_status)
			PingDisableAll(0, 0);
		else if(previousMode == options.off_status)
			PingEnableAll(0, 0);
	}

	return 0;
}

/** Returns the current status
*/
int GetStatus(WPARAM wParam,LPARAM lParam)
{
	return mcStatus;
}

// plugin stuff
PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	"Ping Protocol",
	PLUGIN_MAKE_VERSION(0, 5, 0, 2),
	"Ping labelled IP addresses or domain names.",
	"Scott Ellis",
	"mail@scottellis.com.au",
	"© 2005 Scott Ellis",
	"http://www.scottellis.com.au/",
	0,		//not transient
	0,		//doesn't replace anything built-in
	{ 0x9eb87960, 0x61ab, 0x4625, { 0x94, 0xc7, 0xa0, 0xee, 0xd, 0x9, 0x38, 0x85 } } // {9EB87960-61AB-4625-94C7-A0EE0D093885}
};

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}

extern "C" PINGPROTO_API PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" PINGPROTO_API PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	pluginInfo.cbSize = sizeof(PLUGININFO);
	return (PLUGININFO*)&pluginInfo;
}

static const MUUID interfaces[] = {MIID_PROTOCOL, MIID_PING, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}


int ContactDeleted(WPARAM wParam, LPARAM lParam) {
	if ( CallService(MS_PROTO_ISPROTOONCONTACT, wParam, (LPARAM)PROTO) ) {
		CallService(MS_PROTO_REMOVEFROMCONTACT, wParam, (LPARAM)PROTO);
		HWND hGraphWnd = (HWND)DBGetContactSettingDword((HANDLE)wParam, PROTO, "WindowHandle", 0);
		if(hGraphWnd) PostMessage(hGraphWnd, WM_CLOSE, 0, 0);
		CallService(PROTO "/LoadPingList", 0, 0);
	}

	return 0;
}

int ContactSettingChanged(WPARAM wParam, LPARAM lParam) {
	DBCONTACTWRITESETTING *dcws = (DBCONTACTWRITESETTING *)lParam;

	if ( CallService(MS_PROTO_ISPROTOONCONTACT, wParam, (LPARAM)PROTO) ) {
		if(!strcmp(dcws->szModule, "CList") && !strcmp(dcws->szSetting, "MyHandle")) {
			if(!changing_clist_handle()) {
				if(!dcws->value.type == DBVT_DELETED) {
					DBWriteContactSettingString((HANDLE)wParam, PROTO, "Nick", dcws->value.pszVal);
					CallService(PROTO "/LoadPingList", 0, 0);
				}
			}
		}
	}

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

int GetInfo(WPARAM wParam, LPARAM lParam) {
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

void CreatePluginServices() {
	int i = 0;
	// protocol
	hServices[i++] = CreateProtoServiceFunction(PROTO,PS_GETCAPS, GetCaps);
	hServices[i++] = CreateProtoServiceFunction(PROTO,PS_GETNAME, GetName);
	hServices[i++] = CreateProtoServiceFunction(PROTO,PS_SETSTATUS, SetStatus);
	hServices[i++] = CreateProtoServiceFunction(PROTO,PS_GETSTATUS, GetStatus);
	hServices[i++] = CreateProtoServiceFunction(PROTO,PS_LOADICON,LoadIcon);
	hServices[i++] = CreateProtoServiceFunction(PROTO,PSS_GETINFO,GetInfo);

	// general
	hServices[i++] = CreateServiceFunction(PROTO "/Ping", PluginPing);

	// list
	hServices[i++] = CreateServiceFunction(PROTO "/ClearPingList", ClearPingList);
	hServices[i++] = CreateServiceFunction(PROTO "/GetPingList", GetPingList);
	hServices[i++] = CreateServiceFunction(PROTO "/SetPingList", SetPingList);
	hServices[i++] = CreateServiceFunction(PROTO "/SetAndSavePingList", SetAndSavePingList);
	hServices[i++] = CreateServiceFunction(PROTO "/LoadPingList", LoadPingList);
	hServices[i++] = CreateServiceFunction(PROTO "/SavePingList", SavePingList);

	reload_event_handle = CreateHookableEvent(PROTO "/ListReload");
	
	//log
	hServices[i++] = CreateServiceFunction(PROTO "/Log", Log);
	hServices[i++] = CreateServiceFunction(PROTO "/ViewLogData", ViewLogData);
	hServices[i++] = CreateServiceFunction(PROTO "/GetLogFilename", GetLogFilename);
	hServices[i++] = CreateServiceFunction(PROTO "/SetLogFilename", SetLogFilename);

	// menu
	hServices[i++] = CreateServiceFunction(PROTO "/DisableAll", PingDisableAll);
	hServices[i++] = CreateServiceFunction(PROTO "/EnableAll", PingEnableAll);
	hServices[i++] = CreateServiceFunction(PROTO "/ToggleEnabled", ToggleEnabled);
	hServices[i++] = CreateServiceFunction(PROTO "/ShowGraph", ShowGraph);
	hServices[i++] = CreateServiceFunction(PROTO "/Edit", EditContact);

}

void DestroyPluginServices() {
	/*
	DestroyServiceFunction(PROTO PS_GETCAPS);
	DestroyServiceFunction(PROTO PS_GETNAME);
	DestroyServiceFunction(PROTO PS_SETSTATUS);
	DestroyServiceFunction(PROTO PS_GETSTATUS);
	DestroyServiceFunction(PROTO PS_LOADICON);
	DestroyServiceFunction(PROTO PSS_GETINFO);

	DestroyServiceFunction(PROTO "/Ping");

	DestroyServiceFunction(PROTO "/ClearPingList");
	DestroyServiceFunction(PROTO "/GetPingList");
	DestroyServiceFunction(PROTO "/SetPingList");
	DestroyServiceFunction(PROTO "/SetAndSavePingList");
	DestroyServiceFunction(PROTO "/LoadPingList");
	DestroyServiceFunction(PROTO "/SavePingList");


	DestroyServiceFunction(PROTO "/Log");
	DestroyServiceFunction(PROTO "/ViewLogData");
	DestroyServiceFunction(PROTO "/GetLogFilename");
	DestroyServiceFunction(PROTO "/SetLogFilename");

	// menu
	DestroyServiceFunction(PROTO "/DisableAll");
	DestroyServiceFunction(PROTO "/EnableAll");
	DestroyServiceFunction(PROTO "/ToggleEnabled");
	DestroyServiceFunction(PROTO "/ShowGraph");
	DestroyServiceFunction(PROTO "/Edit");
	*/
	for(int i = 0; i < NUM_SERVICES; i++)
		if(hServices[i]) DestroyServiceFunction(hServices[i]);
	
	DestroyHookableEvent(reload_event_handle);

}

int OnShutdown(WPARAM wParam, LPARAM lParam) {
	graphs_cleanup();
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
		update.cpbVersion = strlen((char *)update.pbVersion);

		update.szUpdateURL = UPDATER_AUTOREGISTER;
		
		// these are the three lines that matter - the archive, the page containing the version string, and the text (or data) 
		// before the version that we use to locate it on the page
		// (note that if the update URL and the version URL point to standard file listing entries, the backend xml
		// data will be used to check for updates rather than the actual web page - this is not true for beta urls)
		update.szBetaUpdateURL = "http://www.scottellis.com.au/miranda_plugins/pingproto.zip";
		update.szBetaVersionURL = "http://www.scottellis.com.au/miranda_plugins/ver_pingproto.html";
		update.pbBetaVersionPrefix = (BYTE *)"Ping Protocol version ";
		
		update.cpbBetaVersionPrefix = strlen((char *)update.pbBetaVersionPrefix);

		CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);
	}

	
	// register for autoupdates
	CallService(MS_UPDATE_REGISTERFL, (WPARAM)1826, (LPARAM)&pluginInfo);

	NETLIBUSER nl_user = {0};
	nl_user.cbSize = sizeof(nl_user);
	nl_user.szSettingsModule = PROTO;
	//nl_user.flags = NUF_OUTGOING | NUF_HTTPGATEWAY | NUF_NOOPTIONS;
	//nl_user.flags = NUF_OUTGOING | NUF_NOOPTIONS;
	nl_user.flags = NUF_OUTGOING | NUF_HTTPCONNS;
	nl_user.szDescriptiveName = "Ping Protocol Plugin";
	nl_user.szHttpGatewayHello = 0;
	nl_user.szHttpGatewayUserAgent = 0;
	nl_user.pfnHttpGatewayInit = 0;
	nl_user.pfnHttpGatewayWrapSend = 0;
	nl_user.pfnHttpGatewayUnwrapRecv = 0;

	hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nl_user);

	InitMenus();
	
	hFillListEvent = HookEvent(PROTO "/ListReload", FillList);

	if(!DBGetContactSettingByte(0, PROTO, "PingPlugImport", 0)) {
		if(DBGetContactSettingDword(0, "PingPlug", "NumEntries", 0)) {
			if(MessageBox(0, Translate("Old PingPlug data detected in database.\nWould you like to import?"), Translate(PROTO "PingPlug Import"), MB_YESNO) == IDYES) {
				import_ping_addresses();
				DBWriteContactSettingByte(0, PROTO, "PingPlugImport", 1);
			} else {
				if(MessageBox(0, Translate("Should I ask this question again\nnext time you start Miranda?"), Translate(PROTO "PingPlug Import"), MB_YESNO) == IDNO) {
					DBWriteContactSettingByte(0, PROTO, "PingPlugImport", 1);
				}
			}
		}
	}
	CallService(PROTO "/LoadPingList", 0, 0);	
	
	//ignore status for all PING contacts
	{
		char *proto;
		HANDLE hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
		while ( hContact != NULL )
		{
			proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
			if (proto && lstrcmp(PROTO, proto) == 0) {
				CallService(MS_IGNORE_IGNORE, (WPARAM)hContact, (WPARAM)IGNOREEVENT_USERONLINE);
				DBDeleteContactSetting(hContact, PROTO, "WindowHandle"); // bugfix - windows left open when miranda closed never open again!
			}

			hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
		}	
	}
	
	hContactDeleted = HookEvent(ME_DB_CONTACT_DELETED, ContactDeleted);
	hContactSettingChanged = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, ContactSettingChanged);

	hContactDblClick = HookEvent(ME_CLIST_DOUBLECLICKED, ContactDblClick);

	graphs_init();

	start_ping_thread();

	if(options.logging) CallService(PROTO "/Log", (WPARAM)"start", 0);

	return 0;
}

extern "C" PINGPROTO_API int Load(PLUGINLINK *link)
{
	pluginLink=link;

	if(init_raw_ping()) {
		//MessageBox(0, Translate("Failed to initialize. Plugin disabled."), Translate("PING Protocol"), MB_OK | MB_ICONERROR);
		//return 1;
		use_raw_ping = false;
	}
	DBWriteContactSettingByte(0, PROTO, "UsingRawSockets", (BYTE)use_raw_ping);

	DuplicateHandle( GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &mainThread, THREAD_SET_CONTEXT, FALSE, 0 );
	hWakeEvent = CreateEvent(NULL, FALSE, FALSE, "Local\\ThreadWaitEvent");

	previousMode = mcStatus = ID_STATUS_OFFLINE;

	InitializeCriticalSection(&list_cs);
	InitializeCriticalSection(&thread_finished_cs);
	InitializeCriticalSection(&list_changed_cs);
	InitializeCriticalSection(&data_list_cs);
	
	PROTOCOLDESCRIPTOR pd = {0};
	pd.cbSize=sizeof(pd);
	pd.szName=PROTO;
	pd.type = PROTOTYPE_PROTOCOL;
	CallService(MS_PROTO_REGISTERMODULE,0,(LPARAM)&pd);

	// create services before loading options - so we can have the 'getlogfilename' service!
	CreatePluginServices();

	LoadOptions();

	//set all contacts to 'not responding' if not 'disabled'
	{
		char *proto;
		HANDLE hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
		while ( hContact != NULL )
		{
			proto = ( char* )CallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM )hContact,0 );
			if ( proto && !lstrcmp( PROTO, proto)) {
				if(DBGetContactSettingWord( hContact, PROTO, "Status", options.nrstatus) != options.off_status)
					DBWriteContactSettingWord( hContact, PROTO, "Status", options.nrstatus);
			}

			hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
		}	
	}

	SkinAddNewSound("PingProtoTimeout", "Ping Timout", 0);
	SkinAddNewSound("PingProtoReply", "Ping Reply", 0);

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);	

	HookEvent(ME_OPT_INITIALISE, PingOptInit );

	//HookEvent(ME_SYSTEM_SHUTDOWN, OnShutdown);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, OnShutdown);

	return 0;
}

extern "C" PINGPROTO_API int Unload(void)
{
	UnhookEvent(hContactDblClick);
	UnhookEvent(hFillListEvent);
	UnhookEvent(hContactDeleted);
	UnhookEvent(hContactSettingChanged);
	stop_ping_thread();

	DestroyPluginServices();

	DeleteCriticalSection(&list_cs);
	DeleteCriticalSection(&thread_finished_cs);
	DeleteCriticalSection(&list_changed_cs);
	DeleteCriticalSection(&data_list_cs);

	CloseHandle( mainThread );

	if(use_raw_ping)
		cleanup_raw_ping();
	else
		ICMP::get_instance()->stop();

	if(options.logging) CallService(PROTO "/Log", (WPARAM)"stop", 0);

	return 0;
}
