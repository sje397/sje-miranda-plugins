/* Replace "dll.h" with the name of your header */
/* Replace "dll.h" with the name of your header */

#include "common.h"
#include "dll.h"
#include "options.h"
#include "menu.h"
#include "utils.h"

#include "otr_private.h"

///////////////////////////////////////////////
// Common Plugin Stuff
///////////////////////////////////////////////
HINSTANCE hInst;
PLUGINLINK *pluginLink;

HANDLE mainThread;
DWORD mainThreadId;

HANDLE hEventDbEventAdded, hEventDbEventAddedFilter, hEventWindow, hEventIconPressed, hSettingChanged;

CRITICAL_SECTION lib_cs;

int code_page = CP_ACP;

#include <list>
std::list<void *> alloc_list;

char fingerprint_store_filename[MAX_PATH];
char private_key_filename[MAX_PATH];

OtrlUserState otr_user_state = 0;

UINT_PTR 		TIMER_ID_CLEAR_FINISHED = 0;

char *metaproto = 0;

// plugin stuff
PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	MODULE,
	PLUGIN_MAKE_VERSION(VER_MAJOR, VER_MINOR, VER_RELEASE, VER_BUILD),
	DESC_STRING,
	"Scott Ellis",
	"mail@scottellis.com.au",
	"© 2005 Scott Ellis",
	"http://www.scottellis.com.au/",
	0,		//not transient
	0,		//doesn't replace anything built-in
	{ 0x1b5b5c1b, 0x93a, 0x4765, { 0xa1, 0xfa, 0x4c, 0xc1, 0xda, 0x44, 0x52, 0x87 } } // {1B5B5C1B-093A-4765-A1FA-4CC1DA445287}
};

extern "C" BOOL APIENTRY DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved) {
	hInst=hinstDLL;
	return TRUE;
}

static const MUUID interfaces[] = {MIID_OTR, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void) {
	return interfaces;
}

extern "C" DLLIMPORT PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion) {
	return &pluginInfo;
}

extern "C" __declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	pluginInfo.cbSize = sizeof(PLUGININFO);
	return (PLUGININFO*)&pluginInfo;
}

/////////////////////////
// dodgy, INNEFFICIENT, utility function
////////////////////////
HANDLE get_contact(const char *protocol, const char *disp_name) {
	HANDLE hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
	while ( hContact != NULL )
	{
		char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
		if(proto && strcmp(proto, protocol) == 0) {
			char *name = (char *)CallService( MS_CLIST_GETCONTACTDISPLAYNAME, ( WPARAM )hContact, 0 );	
			if(name && strcmp(name, disp_name) == 0) {
				return hContact;
			}
		}
		hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
	}
	
	return 0;
}

bool GetFingerprint(HANDLE hContact, char buff[45]) {
	char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	char *uname = (char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, 0);
	if(proto == 0 || uname == 0) return false;
	
	ConnContext *context = otrl_context_find(otr_user_state, uname, MODULE, proto, FALSE, 0, 0, 0);
	if(context == 0) return false;
 	
	if(context->active_fingerprint != 0) {
		otrl_privkey_hash_to_human(buff, context->active_fingerprint->fingerprint);
		return true;
	} else if(context->fingerprint_root.next != 0){
		otrl_privkey_hash_to_human(buff, context->fingerprint_root.next->fingerprint);
		return true;
	}
	
	return false;
}

// wish there was an EVENTTYPE_INLINEMSG :)
void ShowMessageInline(const HANDLE hContact, const char *msg) {
	char buff[1024];
	mir_snprintf(buff, 1024, "%s%s", INLINE_PREFIX, msg);

	/*
	// bypasses filters etc - including metacontacts
	DBEVENTINFO dbei = {0};
	dbei.cbSize = sizeof(DBEVENTINFO);
	dbei.szModule = MODULE;
	dbei.timestamp = time(0);
	//dbei.flags = DBEF_READ;
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.cbBlob = strlen(buff);
	dbei.pBlob = (BYTE *)buff;

	CallService(MS_DB_EVENT_ADD, (WPARAM)hContact, (LPARAM)&dbei);
	*/
	
	PROTORECVEVENT pre = {0};
	pre.timestamp = time(0);
	pre.szMessage = buff;
	//CallContactService(hContact, PSR_MESSAGE, 0, (LPARAM)&pre);
	
	CCSDATA ccs = {0};
	ccs.hContact = hContact;
	ccs.szProtoService = PSR_MESSAGE;
	ccs.lParam = (LPARAM)&pre;
	
	CallService(MS_PROTO_CHAINRECV, 0, (LPARAM)&ccs);
}

// set SRMM icon status, if applicable
void SetEncryptionStatus(HANDLE hContact, bool encrypted) {
	//char dbg_msg[2048];
	//dbg_msg[0] = 0;
	
	//strcat(dbg_msg, "Set encyption status: ");
	//strcat(dbg_msg, (encrypted ? "true" : "false"));
	
	char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	bool chat_room = (proto && DBGetContactSettingByte(hContact, proto, "ChatRoom", 0));

	if(!chat_room) DBWriteContactSettingByte(hContact, MODULE, "Encrypted", (encrypted ? 1 : 0));
	
	if(ServiceExists(MS_MSG_MODIFYICON)) {
		//strcat(dbg_msg, "\nchanging icon");
		StatusIconData sid = {0};
		sid.cbSize = sizeof(sid);
		sid.szModule = MODULE;
		sid.dwId = 0;
		sid.flags = (chat_room ? MBF_HIDDEN : (encrypted ? 0 : MBF_DISABLED));
		CallService(MS_MSG_MODIFYICON, (WPARAM)hContact, (LPARAM)&sid);
		
		if(!chat_room && ServiceExists(MS_MC_GETMETACONTACT)) {
			HANDLE hMeta = (HANDLE)CallService(MS_MC_GETMETACONTACT, (WPARAM)hContact, 0);
			if(hMeta && hContact == (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM)hMeta, 0)) {
				//strcat(dbg_msg, "\nrecursing for meta");
				SetEncryptionStatus(hMeta, encrypted);
			}
		}
	}
	//PUShowMessage(dbg_msg, SM_NOTIFY);
}

///////////////////////////////////////////////
// OTR stuff
///////////////////////////////////////////////

void set_context_contact(void *data, ConnContext *context) {
	HANDLE hContact = get_contact(context->protocol, context->username);
	context->app_data = hContact;
}

void add_appdata(void *data, ConnContext *context) {
	if(context) {
		context->app_data = data; // data is hContact
	}
}

void lib_cs_lock() {
	EnterCriticalSection(&lib_cs);
}

void lib_cs_unlock() {
	LeaveCriticalSection(&lib_cs);
}


/* Return the OTR policy for the given context. */
extern "C" OtrlPolicy otr_gui_policy(void *opdata, ConnContext *context) {
	HANDLE hContact = (HANDLE)opdata;
	if(hContact) {
		WORD pol = DBGetContactSettingWord(hContact, MODULE, "Policy", CONTACT_DEFAULT_POLICY); 
		if(pol == CONTACT_DEFAULT_POLICY)
			return options.default_policy | OTRL_POLICY_NOHTML;
		else return pol | OTRL_POLICY_NOHTML;
	}

	return options.default_policy | OTRL_POLICY_NOHTML;
}

typedef struct {
	char *account_name;
	char *protocol;
} NewKeyData;

void CALLBACK newKeyAPC(DWORD data) {
//DWORD CALLBACK newKeyThread(VOID *data) {
	CallService(MS_SYSTEM_THREAD_PUSH, 0, 0);
	
	NewKeyData *nkd = (NewKeyData *)data;
	otrl_privkey_generate(otr_user_state, private_key_filename, nkd->account_name, nkd->protocol);
	
	free(nkd->account_name);
	free(nkd->protocol);
	free(nkd);

	CallService(MS_SYSTEM_THREAD_POP, 0, 0);
	//return 0;
}

BOOL CALLBACK NullDlgFunc(HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {
		case WM_INITDIALOG:
			TranslateDialogDefault(hWndDlg);
			break;
	}
	return FALSE;
}

/* Create a private key for the given accountname/protocol if
 * desired. */
extern "C" void otr_gui_create_privkey(void *opdata, const char *account_name, const char *protocol) {
	//if(MessageBox(0, Translate("Would you like to generate a new private key for this protocol?"), Translate("OTR"), MB_YESNO) == IDYES)
	//if(options.err_method == ED_POP) 
		//ShowPopup(Translate("Generating new private key."), 0 /*Translate("Please wait.")*/, 5);	
	
	//NewKeyData *nkd = (NewKeyData *)malloc(sizeof(NewKeyData));
	//nkd->account_name = strdup(account_name);
	//nkd->protocol = strdup(protocol);

	//DWORD tid;
	//CloseHandle(CreateThread(0, 0, newKeyThread, (VOID *)nkd, 0, &tid));
	//QueueUserAPC(newKeyAPC, mainThread, (DWORD)nkd);
	HWND hWndNotifyDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_GENKEYNOTIFY), GetDesktopWindow(), NullDlgFunc);
	SetClassLong(hWndNotifyDlg, GCL_HICON, (LONG)hLockIcon);
	ShowWindow(hWndNotifyDlg, SW_SHOW);
	UpdateWindow(hWndNotifyDlg);
	otrl_privkey_generate(otr_user_state, private_key_filename, account_name, protocol);
	DestroyWindow(hWndNotifyDlg);
		
}

/* Report whether you think the given user is online.  Return 1 if
 * you think he is, 0 if you think he isn't, -1 if you're not sure.
 *
 * If you return 1, messages such as heartbeats or other
 * notifications may be sent to the user, which could result in "not
 * logged in" errors if you're wrong. */
extern "C" int otr_gui_is_logged_in(void *opdata, const char *accountname, const char *protocol, const char *recipient) {
	HANDLE hContact = (HANDLE)opdata;
	if(hContact) {
		WORD status = DBGetContactSettingWord(hContact, (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0), "Status", ID_STATUS_OFFLINE);
		if(status == ID_STATUS_OFFLINE) return 0;
		else return 1;
	}
	
	return -1;
}

/* Send the given IM to the given recipient from the given
 * accountname/protocol. */
extern "C" void otr_gui_inject_message(void *opdata, const char *accountname, const char *protocol, const char *recipient, const char *message) {
	//MessageBox(0, message, "OTR Inject", MB_OK);
	HANDLE hContact = (HANDLE)opdata;
	
	// bypass filters (including this module, metacontacts, etc) -  go direct to proto level
	// mod for new account stuff - OTRSendMessage will ignore messages starting with "?OTR" - 'new' protocols expect args (flags, char *) instead of (0, CSSDATA*)
	if(protocol && DBGetContactSettingWord(hContact, protocol, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE) {
		/*
		CCSDATA ccs = {0};
		ccs.hContact = hContact;
		ccs.szProtoService = PSS_MESSAGE;
		ccs.lParam = (LPARAM)Translate(message);
		CallProtoService(protocol, PSS_MESSAGE, 0, (LPARAM)&ccs);
		*/
		CallContactService(hContact, PSS_MESSAGE, PREF_UTF, (LPARAM)Translate(message));
	}
}

/* Display a notification message for a particular accountname /
 * protocol / username conversation. */
extern "C" void otr_gui_notify(void *opdata, OtrlNotifyLevel level, const char *accountname, const char *protocol, const char *username, const char *title, const char *primary, const char *secondary) {
	char buff1[512], buff2[512];
	mir_snprintf(buff1, 512, "%s: %s", username, Translate(title));
	mir_snprintf(buff2, 512, "%s\n%s", Translate(primary), Translate(secondary));
	ShowPopup(buff1, buff2, 0);
}

/* Display an OTR control message for a particular accountname /
 * protocol / username conversation.  Return 0 if you are able to
 * successfully display it.  If you return non-0 (or if this
 * function is NULL), the control message will be displayed inline,
 * as a received message, or else by using the above notify()
 * callback. */
extern "C" int otr_gui_display_otr_message(void *opdata, const char *accountname, const char *protocol, const char *username, const char *msg) {
	if(options.msg_inline) {
		ShowMessageInline((HANDLE)opdata, Translate(msg));
	} else {
		char buff[512];
		mir_snprintf(buff, 512, Translate("%s (%s)"), protocol, username);
		ShowPopup(buff, msg, 0);	
	}
	return 0;
}

/* When the list of ConnContexts changes (including a change in
 * state), this is called so the UI can be updated. */
extern "C" void otr_gui_update_context_list(void *opdata) {
	//MessageBox(0, "Update Context List", "OTR Callback", MB_OK);
}

/* Return a newly-allocated string containing a human-friendly name
 * for the given protocol id */
extern "C" const char *otr_gui_protocol_name(void *opdata, const char *protocol) {
	//return strdup(protocol);
	return protocol;
}

/* Deallocate a string allocated by protocol_name */
extern "C" void otr_gui_protocol_name_free(void *opdata, const char *protocol_name) {
	//free((void *)protocol_name);
}

unsigned int CALLBACK trust_fp_thread(void *param) {
	CallService(MS_SYSTEM_THREAD_PUSH, 0, 0);

	Fingerprint *fp = (Fingerprint *)param;
	
	char hash[45];
    otrl_privkey_hash_to_human(hash, fp->fingerprint);
    
    char msg[1024];
    mir_snprintf(msg, 1024, Translate("A new fingerprint has been recieved from '%s'\n\n%s\n\nDo you trust it?"), fp->context->username, hash);
    if(MessageBox(0, msg, Translate("OTR New Fingerprint"), MB_YESNO) == IDYES) {
		lib_cs_lock();
		otrl_context_set_trust(fp, "verified");	
		lib_cs_unlock();
	} else {
		lib_cs_lock();
		otrl_context_set_trust(fp, "unknown");
		lib_cs_unlock();
	}

	CallService(MS_SYSTEM_THREAD_POP, 0, 0);
	return 0; 
}

/* A new fingerprint for the given user has been received. */
extern "C" void otr_gui_new_fingerprint(void *opdata, OtrlUserState us, const char *accountname, const char *protocol, const char *username, unsigned char fingerprint[20]) {
	//MessageBox(0, username, Translate("OTR New Fingerprint"), MB_OK);
    ConnContext *context = otrl_context_find(us, username, accountname, protocol, TRUE, 0, add_appdata, opdata);
    Fingerprint *fp = otrl_context_find_fingerprint(context, fingerprint, TRUE, 0);
   
	//CloseHandle((HANDLE)_beginthreadex(0, 0, trust_fp_thread, (void *)fp, 0, 0));

	otrl_context_set_trust(fp, "unknown");
    otrl_privkey_write_fingerprints(otr_user_state, fingerprint_store_filename);
}

/* The list of known fingerprints has changed.  Write them to disk. */
extern "C" void otr_gui_write_fingerprints(void *opdata) {
	//if(MessageBox(0, Translate("Would you like to save the current fingerprint list?"), Translate(MODULE), MB_YESNO) == IDYES)
	    otrl_privkey_write_fingerprints(otr_user_state, fingerprint_store_filename);
}

// forward dec
void Disconnect(ConnContext *context);

unsigned int CALLBACK verify_fp_thread(void *param) {
	CallService(MS_SYSTEM_THREAD_PUSH, 0, 0);

	ConnContext *context = (ConnContext *)param;
	Fingerprint *fp = context->active_fingerprint;
	
	char hash[45];
    otrl_privkey_hash_to_human(hash, fp->fingerprint);
	
	char buff[1024];
	mir_snprintf(buff, 1024, Translate("OTR encrypted session with '%s'.\n\nFingerprint is UNVERIFIED.\n\n%s\n\nVerify?"), context->username, hash);
	if(MessageBox(0, buff, Translate("OTR Information"), MB_YESNO) == IDNO) {
		//Disconnect(context);
		return 0;
	}

	lib_cs_lock();
	otrl_context_set_trust(context->active_fingerprint, "verified");	
	otrl_privkey_write_fingerprints(otr_user_state, fingerprint_store_filename);
	lib_cs_unlock();
	
	CallService(MS_SYSTEM_THREAD_POP, 0, 0);
	return 0;
}
/* A ConnContext has entered a secure state. */
extern "C" void otr_gui_gone_secure(void *opdata, ConnContext *context) {
	bool trusted = (context->active_fingerprint && context->active_fingerprint->trust && strcmp(context->active_fingerprint->trust, "verified") == 0);
	char buff[1024];
	if(trusted) {
		mir_snprintf(buff, 1024, Translate("Beginning OTR encrypted session with '%s'"), context->username);
		//MessageBox(0, buff, Translate("OTR Information"), MB_OK);
		if(options.msg_inline)
			ShowMessageInline((HANDLE)opdata, buff);
		else
			ShowPopup(Translate("OTR Information"), buff, 0);
		
		// opdata is hContact
		SetEncryptionStatus((HANDLE)opdata, true);
	} else {
		CloseHandle((HANDLE)_beginthreadex(0, 0, verify_fp_thread, context, 0, 0));
		
		mir_snprintf(buff, 1024, Translate("Beginning OTR encrypted session with '%s' (NOT VERIFIED)"), context->username);
		//MessageBox(0, buff, Translate("OTR Information"), MB_OK);
		if(options.msg_inline)
			ShowMessageInline((HANDLE)opdata, buff);
		else
			ShowPopup(Translate("OTR Information"), buff, 0);

		// opdata is hContact
		SetEncryptionStatus((HANDLE)opdata, true);
	}
}

/* A ConnContext has left a secure state. */
extern "C" void otr_gui_gone_insecure(void *opdata, ConnContext *context) {
	char buff[256];
	mir_snprintf(buff, 256, Translate("OTR encrypted session with '%s' has ended"), context->username);
	//MessageBox(0, buff, Translate("OTR Information"), MB_OK);
	if(options.msg_inline)
		ShowMessageInline((HANDLE)opdata, buff);
	else
		ShowPopup(Translate("OTR Information"), buff, 0);

	// opdata is hContact
	SetEncryptionStatus((HANDLE)opdata, false);
}

/* We have completed an authentication, using the D-H keys we
 * already knew.  is_reply indicates whether we initiated the AKE. */
extern "C" void otr_gui_still_secure(void *opdata, ConnContext *context, int is_reply) {
	if(is_reply) {
		char buff[256];
		mir_snprintf(buff, 256, Translate("OTR encrypted session with '%s' is being continued"), context->username);
		//MessageBox(0, buff, Translate("OTR Information"), MB_OK);
		if(options.msg_inline)
			ShowMessageInline((HANDLE)opdata, buff);
		else
			ShowPopup(Translate("OTR Information"), buff, 0);
	}

	// opdata is hContact
	SetEncryptionStatus((HANDLE)opdata, true);
}

/* Log a message.  The passed message will end in "\n". */
extern "C" void otr_gui_log_message(void *opdata, const char *message) {
	//MessageBox(0, message, Translate("OTR Log Message"), MB_OK);
	//ShowMessageInline((HANDLE)opdata, message);
}

extern "C" int max_message_size(void *opdata, ConnContext *context) {
	int s = CallProtoService(context->protocol, PS_GETCAPS, PFLAG_MAXLENOFMESSAGE, 0);
	return s;
}

extern "C" const char *account_name(void *opdata, const char *account, const char *protocol) {
	return protocol;
}

extern "C" void account_name_free(void *opdata, const char *account_name) {
}


OtrlMessageAppOps ops = {
	otr_gui_policy,
	otr_gui_create_privkey,
	otr_gui_is_logged_in,
	otr_gui_inject_message,
	otr_gui_notify,
	otr_gui_display_otr_message,
	otr_gui_update_context_list,
	otr_gui_protocol_name,
	otr_gui_protocol_name_free,
	otr_gui_new_fingerprint,
	otr_gui_write_fingerprints,
	otr_gui_gone_secure,
	otr_gui_gone_insecure,
	otr_gui_still_secure,
	otr_gui_log_message,
	max_message_size,
	account_name,
	account_name_free
};

void Disconnect(ConnContext *context) {
	otrl_message_disconnect(otr_user_state, &ops, 0, MODULE, context->protocol, context->username);
}

//////////////////////////////////////////////////
/// Miranda filter plugin stuff
//////////////////////////////////////////////////

void CALLBACK ClearFinishedSessions(HWND, UINT, UINT, DWORD) {
	KillTimer(0, TIMER_ID_CLEAR_FINISHED);
	
	lib_cs_lock();
	//PUShowMessage("Clearing finished sessions", SM_NOTIFY);
	HANDLE hContact;
	ConnContext *context = otr_user_state->context_root;
	while(context) {
		if(context->msgstate == OTRL_MSGSTATE_FINISHED) {
			hContact = context->app_data;
			//hContact = get_contact(context->protocol, context->username);
			if(hContact) {
				SetEncryptionStatus(hContact, false);		
				//PUShowMessage("Cleared a finished session", SM_NOTIFY);
			} //else
				//PUShowMessage("Cleared a finished session - but hCcontact is null", SM_NOTIFY);
			otrl_context_force_plaintext(context);
		}
		context = context->next;
	}
	lib_cs_unlock();
}

void ResetFinishedTimer() {
	//PUShowMessage("ResetFinishedTimer", SM_NOTIFY);
	if(TIMER_ID_CLEAR_FINISHED) KillTimer(0, TIMER_ID_CLEAR_FINISHED);
	TIMER_ID_CLEAR_FINISHED = SetTimer(0, 0, DBGetContactSettingDword(0, MODULE, "SessionTimeout", 10000), ClearFinishedSessions);
}

// if it's a protocol going offline, attempt to send terminate session to all contacts of that protocol
// (this would be hooked as the ME_CLIST_STATUSMODECHANGE handler except that event is sent *after* the proto goes offline)
int StatusModeChange(WPARAM wParam, LPARAM lParam) {
	
	if(Miranda_Terminated()) return 0;
	
	int status = (int)wParam;
	const char *proto = (char *)lParam;
	HANDLE hContact;
	
	lib_cs_lock();
	if(status == ID_STATUS_OFFLINE) {
		ConnContext *context = otr_user_state->context_root;
		while(context) {
			if(context->msgstate == OTRL_MSGSTATE_ENCRYPTED && (proto == 0 || strcmp(proto, context->protocol) == 0)) {
				hContact = context->app_data;
				
				char *uproto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
				char *uname = (char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, 0);
				if(uproto && uname)
					otrl_message_disconnect(otr_user_state, &ops, hContact, MODULE, uproto, uname);				
				
				//otrl_context_force_finished(context);
				
				SetEncryptionStatus(hContact, false);
				
			}
			context = context->next;
		}
	}
	lib_cs_unlock();
	
	return 0;
}

void FinishSession(HANDLE hContact) {
	char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	char *uname = (char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, 0);
	if(!proto || !uname) return; // error - just bail		

	lib_cs_lock();
	ConnContext *context = otrl_context_find(otr_user_state, uname, MODULE, proto, FALSE, 0, 0, 0);
	
	if(context && context->msgstate == OTRL_MSGSTATE_ENCRYPTED) {
		//otrl_message_disconnect(otr_user_state, &ops, hContact, MODULE, proto, uname);
		//SetEncryptionStatus(hContact, false);
		
		// FINISHED is not 'unencrypted', for e.g. srmm icon (still need to be able to 'stop OTR')
		otrl_context_force_finished(context);
		if(options.timeout_finished) ResetFinishedTimer();
		
		// removed - don't need a popup everytime an OTR user goes offline!
		//char buff[512];
		//mir_snprintf(buff, 512, Translate("User '%s' ended encrypted session"), uname);
		//ShowPopup(Translate("OTR Information"), buff, 0);

		// opdata is hContact
	}
	lib_cs_unlock();
}

void EndSession(HANDLE hContact) {
	HANDLE hSub;
	if(ServiceExists(MS_MC_GETMOSTONLINECONTACT) && (hSub = (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM)hContact, 0)) != 0) {
		hContact = hSub;
	}
	
	char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	char *uname = (char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, 0);
	if(!proto || !uname) return; // error

	lib_cs_lock();
	otrl_message_disconnect(otr_user_state, &ops, hContact, MODULE, proto, uname);
	lib_cs_unlock();
}

int SettingChanged(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *)lParam;

	// only care about contacts to which this filter is attached
	if(hContact && !CallService(MS_PROTO_ISPROTOONCONTACT, (WPARAM)hContact, (LPARAM)MODULE))
		return 0;
	
	// and who are changing status to offline
	if(strcmp(cws->szSetting, "Status") == 0 && cws->value.type != DBVT_DELETED && cws->value.wVal == ID_STATUS_OFFLINE) {
		if(!hContact) {
			// if it's a protocol going offline, attempt to send terminate session to all contacts of that protocol with active OTR sessions
			const char *proto = cws->szModule;
			StatusModeChange((WPARAM)ID_STATUS_OFFLINE, (LPARAM)proto);
			return 0;
		} else if(options.end_offline) {
			FinishSession(hContact);
		}
	}
	
	return 0;
}

int OTRSendMessage(WPARAM wParam,LPARAM lParam){
	if(!lParam) return 0;
	
    CCSDATA *ccs = (CCSDATA *) lParam;

	//MessageBox(0, (char *)ccs->lParam, "OTR - sending raw message", MB_OK);

	char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)ccs->hContact, 0);
	if(proto && metaproto && strcmp(proto, metaproto) == 0)  {// bypass for metacontacts
		//PUShowMessage("OTR bypassing metacontact", SM_NOTIFY);
		return CallService(MS_PROTO_CHAINSEND, wParam, lParam);
	}
	if(!proto) {
		//PUShowMessage("OTR error no proto", SM_NOTIFY);
		return 1; // error
	}

	gcry_error_t err;
    char *newmessage = 0;
    char *oldmessage = (char *)ccs->lParam;
    char *oldmessage_utf = 0;    

	//MessageBox(0, "Send message - converting to UTF-8", "msg", MB_OK);
	
	if(ccs->wParam & PREF_UTF) {
		oldmessage_utf = strdup(oldmessage);
	} else if(ccs->wParam & PREF_UNICODE) {
		int size = WideCharToMultiByte(CP_UTF8, 0, (wchar_t *)&oldmessage[strlen(oldmessage)+1], -1, 0, 0, 0, 0);
		oldmessage_utf = (char *)malloc(size);
		if(!oldmessage_utf) return 1;
		WideCharToMultiByte(CP_UTF8, 0, (wchar_t *)&oldmessage[strlen(oldmessage)+1], -1, oldmessage_utf, size, 0, 0);
	} else {
		wchar_t *temp = 0;
		int size_temp, size;
		size_temp = MultiByteToWideChar(code_page, 0, oldmessage, -1, 0, 0);
		temp = (wchar_t *)malloc(size_temp * sizeof(wchar_t));
		if(!temp) return 1;
		
		MultiByteToWideChar(code_page, 0, oldmessage, -1, temp, size_temp);
		
		size = WideCharToMultiByte(CP_UTF8, 0, temp, -1, 0, 0, 0, 0);
		oldmessage_utf = (char *)malloc(size);
		if(!oldmessage_utf) {
			free(temp);
			return 1;
		}
		WideCharToMultiByte(CP_UTF8, 0, temp, -1, oldmessage_utf, size, 0, 0);
		
		free(temp);
	}
	
	// don't filter OTR messages being sent (OTR messages should only happen *after* the otrl_message_sending call below)
	if(strncmp(oldmessage_utf, "?OTR", 4) == 0) {
		free(oldmessage_utf);
		PUShowMessage("OTR skipping otr message", SM_NOTIFY);
		return CallService(MS_PROTO_CHAINSEND, wParam, lParam);
	}

	char *username = (char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)ccs->hContact, 0);
	// check if we're waiting for the other side to accept a new key from us
	lib_cs_lock();
	/*
    ConnContext *context = otrl_context_find(otr_user_state, username, MODULE, proto, FALSE, 0, 0, 0);
	if(context && context->otr_offer == context::OFFER_SENT) {
		lib_cs_unlock();
		if(MessageBox(0, Translate("The receiver has not yet accepted or rejected your fingerprint.\nSend the message unencrypted?"), Translate("OTR Warning"), MB_ICONWARNING | MB_YESNO) == IDYES)
			return CallService(MS_PROTO_CHAINSEND, wParam, lParam);
		else
			return 1;
	}
	*/
	//MessageBox(0, "Send message - passing through OTR library", "msg", MB_OK);
    err = otrl_message_sending(otr_user_state, &ops, ccs->hContact, MODULE, proto, username, oldmessage_utf, 0, &newmessage, add_appdata, ccs->hContact);
	lib_cs_unlock();
	free(oldmessage_utf);
    if(err) {
		// failure to encrypt message!
		return 1;
	}
	
	if(newmessage) {
		char *remaining = 0;
		ConnContext *context = otrl_context_find(otr_user_state, username, MODULE, proto, FALSE, 0, 0, 0);

		if(context)
			otrl_message_fragment_and_send(&ops, ccs->hContact, context, newmessage, OTRL_FRAGMENT_SEND_ALL_BUT_LAST, &remaining);
		else
			remaining = newmessage;
		
		int ret;
		if(ccs->wParam & PREF_UTF) {
			ccs->lParam = (LPARAM)remaining;
			ret = CallService(MS_PROTO_CHAINSEND, wParam, lParam);			

			lib_cs_lock();
			otrl_message_free(newmessage);
			lib_cs_unlock();
		} else {
			//MessageBox(0, "Send message - message encoded - decoding UTF-8", "msg", MB_OK);
			// decode utf8 into unicode message 		
			wchar_t *temp;
			char *text;
		
			// forward message
			int size = MultiByteToWideChar(CP_UTF8, 0, (const char *)remaining, -1, 0, 0);
			temp = (wchar_t *)malloc(size * sizeof(wchar_t));
			if(!temp) return 1;
			MultiByteToWideChar(CP_UTF8, 0, (const char *)remaining, -1, temp, size);
	
			size = WideCharToMultiByte(code_page, 0, temp, -1, 0, 0, 0, 0);
			text = (char *)malloc(size);
			if(!text) {
				free(temp);
				return 1;
			}
			WideCharToMultiByte(code_page, 0, temp, -1, text, size, 0, 0);
			
			lib_cs_lock();
			otrl_message_free(newmessage);
			lib_cs_unlock();
	
			int tMsgBodyLen = strlen(text);
			int tRealBodyLen = wcslen(temp);
			
			int tMsgBufLen = (tMsgBodyLen+1) * sizeof(char) + (tRealBodyLen+1)*sizeof( wchar_t );
			char* tMsgBuf = ( char* )malloc( tMsgBufLen );
			if(!tMsgBuf) {
				free(temp);
				free(text);
				return 1;
			}
		
			char* p = tMsgBuf;
		
			strcpy(p, text);
			p += (tMsgBodyLen+1);
			free(text);
		
			WPARAM old_flags = ccs->wParam;
			if ( tRealBodyLen != 0 ) {
				wcscpy((wchar_t *)p, temp);
				ccs->wParam |= PREF_UNICODE;
			}
			free(temp);
			//MessageBox(0, "Send message - forwarding message", "msg", MB_OK);
			// forward new message
			ccs->lParam = (LPARAM)tMsgBuf;
			ret = CallService(MS_PROTO_CHAINSEND, wParam, lParam);			
			// free decrypted message and conversion buffer
			free(tMsgBuf);
	
			// restore state of chain message, for freeing, db, etc etc.
			ccs->wParam = old_flags;
		}
		//ccs->szProtoService=PSS_MESSAGE"W";
		ccs->lParam = (LPARAM)oldmessage;

		return ret;
	} //else
		//MessageBox(0, "Send message - no action", "msg", MB_OK);

	return CallService(MS_PROTO_CHAINSEND, wParam, lParam);
}

int OTRSendMessageW(WPARAM wParam, LPARAM lParam){
	if(!lParam) return 0;

    CCSDATA *ccs = (CCSDATA *) lParam;
	if(!(ccs->wParam & PREF_UTF))
		ccs->wParam |= PREF_UNICODE;
	
	return OTRSendMessage(wParam, lParam);
}

/*
#define MESSAGE_PREFIX			"(OTR) "
#define MESSAGE_PREFIXW			L"(OTR) "
#define MESSAGE_PREFIX_LEN		6
*/

int RecvMessage(WPARAM wParam,LPARAM lParam){
	//PUShowMessage("OTR Recv Message", SM_NOTIFY);
    CCSDATA *ccs = (CCSDATA *) lParam;
    PROTORECVEVENT *pre = (PROTORECVEVENT *) ccs->lParam;
	
	char MESSAGE_PREFIX[64];
	wchar_t MESSAGE_PREFIXW[64];
	MultiByteToWideChar(CP_UTF8, 0, options.prefix, -1, MESSAGE_PREFIXW, 64);
	WideCharToMultiByte(code_page, 0, MESSAGE_PREFIXW, -1, MESSAGE_PREFIX, 64, 0, 0);
	
	int ignore_message;
    char *newmessage = 0;

	char *oldmessage = pre->szMessage;
	char *oldmessage_utf = 0;
    OtrlTLV *tlvs = 0;
    OtrlTLV *tlv = 0;

	char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)ccs->hContact, 0);
	if(proto && metaproto && strcmp(proto, metaproto) == 0) // bypass for metacontacts
		return CallService(MS_PROTO_CHAINRECV, wParam, lParam);
	
	if(strlen(oldmessage) > INLINE_PREFIX_LEN && strncmp(oldmessage, INLINE_PREFIX, INLINE_PREFIX_LEN) == 0) // bypass for our inline messages
		return CallService(MS_PROTO_CHAINRECV, wParam, lParam);
	
	char *uname = (char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)ccs->hContact, 0);
	if(!proto || !uname) return 1; // error

	// convert oldmessage to utf-8
	if(pre->flags & PREF_UTF) {
		oldmessage_utf = strdup(oldmessage);
		if(!oldmessage_utf) return 1;
	} else if(pre->flags & PREF_UNICODE) {
		int size = WideCharToMultiByte(CP_UTF8, 0, (wchar_t *)&oldmessage[strlen(oldmessage)+1], -1, 0, 0, 0, 0);
		oldmessage_utf = (char *)malloc(size);
		if(!oldmessage_utf) return 1;
		WideCharToMultiByte(CP_UTF8, 0, (wchar_t *)&oldmessage[strlen(oldmessage)+1], -1, oldmessage_utf, size, 0, 0);
	} else {
		wchar_t *temp = 0;
		int size_temp, size;
		size_temp = MultiByteToWideChar(code_page, 0, oldmessage, -1, 0, 0);
		temp = (wchar_t *)malloc(size_temp * sizeof(wchar_t));
		if(!temp) return 1;
		MultiByteToWideChar(code_page, 0, oldmessage, -1, temp, size_temp);
		
		size = WideCharToMultiByte(CP_UTF8, 0, temp, -1, 0, 0, 0, 0);
		oldmessage_utf = (char *)malloc(size * sizeof(char));
		if(!oldmessage_utf) {
			free(temp);
			return 1;
		}
		WideCharToMultiByte(CP_UTF8, 0, temp, -1, oldmessage_utf, size, 0, 0);
		
		free(temp);
	}

	lib_cs_lock();
    ignore_message = otrl_message_receiving(otr_user_state, &ops, ccs->hContact, MODULE, proto, uname, oldmessage_utf, &newmessage, &tlvs, add_appdata, ccs->hContact);
	lib_cs_unlock();
    free(oldmessage_utf);
	
    tlv = otrl_tlv_find(tlvs, OTRL_TLV_DISCONNECTED);
    if (tlv) {
		char buff[256];
		mir_snprintf(buff, 256, Translate("'%s' has terminated the OTR session"), uname);
		//MessageBox(0, buff, Translate("OTR Information"), MB_OK);
		if(options.msg_inline)
			ShowMessageInline(ccs->hContact, buff);
		else
			ShowPopup(Translate("OTR Information"), buff, 0);

		if(options.timeout_finished) ResetFinishedTimer();
		
		return 1;	
	}
    
	if(tlvs) otrl_tlv_free(tlvs); // do we need these? (can be used to tell that the other side has terminated encrypted connection)
    
    if(ignore_message) // internal OTR library message - ignore it
    //if(ignore_message == 1) // internal OTR library message - ignore it
    	return 1;
    	
	if(newmessage) {
		int ret;
		// user should receive newmessage instead of message

		// it may not be encrypted however (e.g. tagged plaintext with tags stripped)
		ConnContext *context = otrl_context_find(otr_user_state, uname, MODULE, proto, FALSE, 0, 0, 0);	
		bool encrypted = context && context->msgstate != OTRL_MSGSTATE_PLAINTEXT;
		
		if(pre->flags & PREF_UTF) {
			char *temp;
			if(options.prefix_messages && encrypted) {
				int len = strlen(MESSAGE_PREFIX) + strlen(newmessage) + 1;
				temp = (char *)malloc(len);
				mir_snprintf(temp, len, "%s%s", MESSAGE_PREFIX, newmessage);
			} else
				temp = strdup(newmessage);
				
			pre->szMessage = temp;
			ret = CallService(MS_PROTO_CHAINRECV, wParam, lParam);	
			free(temp);
		} else {
			// decode utf8 into unicode message 		
			wchar_t *temp;
			char *text;
		
			int size = MultiByteToWideChar(CP_UTF8, 0, (const char *)newmessage, -1, 0, 0);
			temp = (wchar_t *)malloc(size * sizeof(wchar_t));
			if(!temp) return 1;
			MultiByteToWideChar(CP_UTF8, 0, (const char *)newmessage, -1, temp, size);
		
			size = WideCharToMultiByte(code_page, 0, temp, -1, 0, 0, 0, 0);
			text = (char *)malloc(size);
			if(!text) {
				free(temp);
				return 1;
			}
			WideCharToMultiByte(code_page, 0, temp, -1, text, size, 0, 0);
		
			int tMsgBodyLen = strlen(text);
			int tRealBodyLen = wcslen(temp);
		
			// it may not be encrypted however (e.g. tagged plaintext with tags stripped)
			ConnContext *context = otrl_context_find(otr_user_state, uname, MODULE, proto, FALSE, 0, 0, 0);	
			bool encrypted = context && context->msgstate != OTRL_MSGSTATE_PLAINTEXT;
		
			if(options.prefix_messages && encrypted) {
				tMsgBodyLen += strlen(MESSAGE_PREFIX);
				tRealBodyLen += wcslen(MESSAGE_PREFIXW);
			}
				
			int tMsgBufLen = (tMsgBodyLen+1) * sizeof(char) + (tRealBodyLen+1)*sizeof( wchar_t );
			char* tMsgBuf = ( char* )malloc( tMsgBufLen );
			if(!tMsgBuf) {
				free(temp);
				free(text);
				return 1;
			}
		
			char* p = tMsgBuf;
		
			if(options.prefix_messages && encrypted) {
				strcpy(p, MESSAGE_PREFIX);
				strcat(p + strlen(MESSAGE_PREFIX), text);
			} else
				strcpy(p, text);
			p += (tMsgBodyLen+1);
			free(text);
			
			WPARAM old_flags = pre->flags;
			if ( tRealBodyLen != 0 ) {
				if(options.prefix_messages && encrypted) {
					wcscpy((wchar_t *)p, MESSAGE_PREFIXW);
					wcscat((wchar_t *)p + wcslen(MESSAGE_PREFIXW), temp );
				} else
					wcscpy((wchar_t *)p, temp);
				pre->flags |= PREF_UNICODE;
			}
			free(temp);
			
			pre->szMessage = tMsgBuf;
			ret = CallService(MS_PROTO_CHAINRECV, wParam, lParam);			
			// free decrypted message and conversion buffer
			free(tMsgBuf);
			pre->flags = old_flags;
		}
		
		lib_cs_lock();
		otrl_message_free(newmessage);
		lib_cs_unlock();

		// restore old message, so it can be freed etc
		pre->szMessage = oldmessage; 
	
		return ret;
	}
	    
	return CallService(MS_PROTO_CHAINRECV, wParam, lParam);
}

// add prefix to sent messages
int OnDatabaseEventPreAdd(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;
	DBEVENTINFO *dbei = (DBEVENTINFO *)lParam;

	if(!options.prefix_messages) 
		return 0;
	
	if ((dbei->eventType != EVENTTYPE_MESSAGE) || !(dbei->flags & DBEF_SENT))
		return 0;

	if(dbei->cbBlob == 0 || dbei->pBlob == 0)
		return 0; // just to be safe

	char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	char *uname = (char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, 0);
	
	if(!proto || !uname || DBGetContactSettingByte(hContact, proto, "ChatRoom", 0) == 1) {
		return 0;
	}
	
	if(metaproto && strcmp(proto, metaproto) == 0) {
		HANDLE hSubContact = (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM)hContact, 0);
		if(!hSubContact)
			return 0;
		uname = (char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hSubContact, 0);
		proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hSubContact, 0);
	}

	ConnContext *context = otrl_context_find(otr_user_state, uname, MODULE, proto, FALSE, 0, 0, 0);	
	bool encrypted = context && context->msgstate != OTRL_MSGSTATE_PLAINTEXT;

	if(!encrypted) return 0;

	char MESSAGE_PREFIX[64];
	wchar_t MESSAGE_PREFIXW[64];
	MultiByteToWideChar(CP_UTF8, 0, options.prefix, -1, MESSAGE_PREFIXW, 64);
	WideCharToMultiByte(code_page, 0, MESSAGE_PREFIXW, -1, MESSAGE_PREFIX, 64, 0, 0);
	
	DBEVENTINFO my_dbei = *dbei; // copy the other event

	char *msg = (char *)dbei->pBlob;
	wchar_t *msgw = 0;
	unsigned int msglen = strlen(msg) + 1;
	
	if((msglen > strlen(MESSAGE_PREFIX) && strncmp(msg, MESSAGE_PREFIX, strlen(MESSAGE_PREFIX)) == 0) || (msglen > INLINE_PREFIX_LEN && strncmp(msg, INLINE_PREFIX, INLINE_PREFIX_LEN) == 0))
		return 0; // already contains prefix, or it's an inline message
	
	// here we detect the double-zero wide char zero terminator - in case of messages that are not unicode but have
	// other data after the message (e.g. metacontact copied messages with source identifier)
	bool dz = false;
	for(unsigned int i = msglen; i < dbei->cbBlob; i++) {
		if(msg[i] == 0 && msg[i - 1] == 0) { // safe since msglen + 1 above
			dz = true;
			break;
		}
	}

	// does blob contain unicode message?
	if(msglen < dbei->cbBlob && dz) {
		// yes
		msgw = (wchar_t *)&msg[msglen];
	} else {
		// no, convert to unciode (allocate stack memory)
		if(dbei->flags && DBEF_UTF) {
			int size = MultiByteToWideChar(CP_UTF8, 0, (char *) msg, -1, 0, 0);
			msgw = (wchar_t *) _alloca(sizeof(wchar_t) * size);
			MultiByteToWideChar(CP_UTF8, 0, msg, -1, msgw, size);
		} else {
			int size = MultiByteToWideChar(code_page, 0, (char *) msg, -1, 0, 0);
			msgw = (wchar_t *) _alloca(sizeof(wchar_t) * size);
			MultiByteToWideChar(code_page, 0, msg, -1, msgw, size);
		}
	}
	
	int tMsgBodyLen = strlen(msg) + strlen(MESSAGE_PREFIX);
	int tRealBodyLen = wcslen(msgw) + wcslen(MESSAGE_PREFIXW);

	int tMsgBufLen = (tMsgBodyLen+1) * sizeof(char) + (tRealBodyLen+1)*sizeof( wchar_t );
	char* tMsgBuf = ( char* )_alloca( tMsgBufLen );
	
	char* p = tMsgBuf;

	strcpy(p, MESSAGE_PREFIX);
	strcat(p + strlen(MESSAGE_PREFIX), msg);
	p += (tMsgBodyLen+1);

	if ( tRealBodyLen != 0 ) {
		wcscpy((wchar_t *)p, MESSAGE_PREFIXW);
		wcscat((wchar_t *)p + wcslen(MESSAGE_PREFIXW), msgw);
	}
	
	my_dbei.pBlob = (BYTE *)tMsgBuf;
	my_dbei.cbBlob = tMsgBufLen;
	my_dbei.flags &= ~(DBEF_UTF);

	CallService(MS_DB_EVENT_ADD, (WPARAM)hContact, (LPARAM)&my_dbei);
	
	// stop original event from being added
	return 1;
}


int OnDatabaseEventAdd(WPARAM wParam, LPARAM lParam) {
	// DISABLED - we should only remove sent events and received events when they are marked read - e.g. NoHistory plugin
	/*
	if(!options.delete_history) return 0;
	
	// remove the event - hopefully SRMM has received the event by now (since we hooked this in 'modules loaded')!
	HANDLE hContact = (HANDLE)wParam, hDbEvent = (HANDLE)lParam;

	char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	char *uname = (char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, 0);
	
	if(!proto || !uname || DBGetContactSettingByte(hContact, proto, "ChatRoom", 0) == 1) {
		return 0;
	}
	
	if(strcmp(proto, "MetaContacts") == 0) {
		HANDLE hSubContact = (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM)hContact, 0);
		if(!hSubContact)
			return 0;
		uname = (char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hSubContact, 0);
		proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hSubContact, 0);
	}

	ConnContext *context = otrl_context_find(otr_user_state, uname, MODULE, proto, FALSE, 0, 0, 0);	
	bool encrypted = context && context->msgstate == OTRL_MSGSTATE_ENCRYPTED;

	if(encrypted)
		CallService(MS_DB_EVENT_DELETE, (WPARAM)hContact, (LPARAM)hDbEvent);
	
	// continuing processing of this event could cause trouble
	return 1;
	*/
	
	return 0; 
}

////////////////////////////////
///////// Menu Services
///////////////////////
int StartOTR(WPARAM wParam, LPARAM lParam) {
	// prevent this filter from acting on injeceted messages for metas, when they are passed though the subcontact's proto send chain
	HANDLE hContact = (HANDLE)wParam, hSub;
	if(ServiceExists(MS_MC_GETMOSTONLINECONTACT) && (hSub = (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM)hContact, 0)) != 0) {
		hContact = hSub;
	}
	
	char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	char *uname = (char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, 0);
	if(!proto || !uname) return 1; // error
	
	WORD pol = DBGetContactSettingWord(hContact, MODULE, "Policy", CONTACT_DEFAULT_POLICY);
	if(pol == CONTACT_DEFAULT_POLICY) pol = options.default_policy | OTRL_POLICY_NOHTML;
	
	lib_cs_lock();
    char *msg = otrl_proto_default_query_msg(MODULE, pol);
    otr_gui_inject_message(hContact, MODULE, proto, uname, msg ? msg : "?OTRv2?");
	lib_cs_unlock();
    free(msg);
	
	return 0;
}

int StopOTR(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;
	
	// prevent this filter from acting on injeceted messages for metas, when they are passed though the subcontact's proto send chain
	EndSession(hContact);

	SetEncryptionStatus(hContact, false);

	char buff[256];
	mir_snprintf(buff, 256, Translate("OTR session terminated"), (char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, 0));
	//MessageBox(0, buff, Translate("OTR Information"), MB_OK);
	if(options.msg_inline)
		ShowMessageInline(hContact, buff);
	else
		ShowPopup(Translate("OTR Information"), buff, 0);

	return 0;
}


///////////////////////////////////////////////
/////// Plugin init and deinit
////////////////////////////////////////////////


int WindowEvent(WPARAM wParam, LPARAM lParam) {
	MessageWindowEventData *mwd = (MessageWindowEventData *)lParam;
	
	if(mwd->uType == MSG_WINDOW_EVT_CLOSE && options.end_window_close) {
		FinishSession(mwd->hContact);
		return 0;
	}
	
	if(mwd->uType != MSG_WINDOW_EVT_OPEN) return 0;
	if(!ServiceExists(MS_MSG_MODIFYICON)) return 0;
	
	HANDLE hContact = mwd->hContact, hTemp;
	if(ServiceExists(MS_MC_GETMOSTONLINECONTACT) && (hTemp = (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM)hContact, 0)) != 0)
		hContact = hTemp;
	
	if(!CallService(MS_PROTO_ISPROTOONCONTACT, (WPARAM)hContact, (LPARAM)MODULE))
		return 0;
	
	char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	char *uname = (char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, 0);
	if(!proto || !uname) return 0; // error - just bail		

	lib_cs_lock();
	ConnContext *context = otrl_context_find(otr_user_state, uname, MODULE, proto, FALSE, 0, 0, 0);
	bool encrypted = (context && context->msgstate != OTRL_MSGSTATE_PLAINTEXT);
	lib_cs_unlock();
	
	SetEncryptionStatus(hContact, encrypted);
	
	return 0;
}

int IconPressed(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;
	StatusIconClickData *sicd = (StatusIconClickData *)lParam;
	if(sicd->cbSize < (int)sizeof(StatusIconClickData))
		return 0;
	
	if(sicd->flags & MBCF_RIGHTBUTTON) return 0; // ignore right clicks
	
	char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	if(proto && DBGetContactSettingByte(hContact, proto, "ChatRoom", 0))
		return 0;
	

	if(strcmp(sicd->szModule, MODULE) == 0) {
		if(DBGetContactSettingByte(hContact, MODULE, "Encrypted", 0) == 1)
			StopOTR((WPARAM)hContact, 0);
		else
			StartOTR((WPARAM)hContact, 0);
	}
	
	return 0;
}

void ReadPrivkeyFiles() {
	lib_cs_lock();
	otrl_privkey_read(otr_user_state, private_key_filename);
	otrl_privkey_read_fingerprints(otr_user_state, fingerprint_store_filename, set_context_contact, 0);
	lib_cs_unlock();
}

int ModulesLoaded(WPARAM wParam, LPARAM lParam) {
	if(ServiceExists(MS_MC_GETPROTOCOLNAME))
		metaproto = (char *)CallService(MS_MC_GETPROTOCOLNAME, 0, 0);
	
	if(ServiceExists(MS_UPDATE_REGISTER)) {
		// register with updater
		Update update = {0};
		char szVersion[16];

		update.cbSize = sizeof(Update);

		update.szComponentName = pluginInfo.shortName;
		update.pbVersion = (BYTE *)CreateVersionString(pluginInfo.version, szVersion);
		update.cpbVersion = strlen((char *)update.pbVersion);
		update.szBetaChangelogURL = "https://server.scottellis.com.au/wsvn/mim_plugs/otr/?op=log&rev=0&sc=0&isdir=1";

		update.szUpdateURL = UPDATER_AUTOREGISTER;
		
		// these are the three lines that matter - the archive, the page containing the version string, and the text (or data) 
		// before the version that we use to locate it on the page
		// (note that if the update URL and the version URL point to standard file listing entries, the backend xml
		// data will be used to check for updates rather than the actual web page - this is not true for beta urls)
		update.szBetaUpdateURL = "http://www.scottellis.com.au/miranda_plugins/otr.zip";
		update.szBetaVersionURL = "http://www.scottellis.com.au/miranda_plugins/ver_otr.html";
		update.pbBetaVersionPrefix = (BYTE *)"OTR (Off The Record) encryption plugin, version ";
		
		update.cpbBetaVersionPrefix = strlen((char *)update.pbBetaVersionPrefix);

		CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);
	}

	InitUtils();	
	InitMenu();
	
	// hooked so we can remove OFF THE RECORD items from the database
	hEventDbEventAdded = HookEvent(ME_DB_EVENT_ADDED, OnDatabaseEventAdd);
	
	// add icon to srmm status icons
	if(ServiceExists(MS_MSG_ADDICON)) {
		StatusIconData sid = {0};
		sid.cbSize = sizeof(sid);
		sid.szModule = MODULE;
		sid.dwId = 0;
		sid.hIcon = hLockIcon;
		sid.hIconDisabled = hUnlockIcon;
		sid.flags = MBF_DISABLED;
		sid.szTooltip = Translate("OTR Encryption Status");
		CallService(MS_MSG_ADDICON, 0, (LPARAM)&sid);
		
		// hook the window events so that we can can change the status of the icon
		hEventWindow = HookEvent(ME_MSG_WINDOWEVENT, WindowEvent);
		hEventIconPressed = HookEvent(ME_MSG_ICONPRESSED, IconPressed);
	}
	
	//////////////
	/////// init OTR	
	OTRL_INIT;
	lib_cs_lock();
	otr_user_state = otrl_userstate_create();
	lib_cs_unlock();
	
	// this calls ReadPrivkeyFiles (above) to set filename values (also called on ME_FOLDERS_PATH_CHANGED)
	LoadOptions();
	HookEvent(ME_OPT_INITIALISE, OptInit);		

	// hook setting changed to monitor status
	hSettingChanged = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, SettingChanged);

	// hook status mode changes to terminate sessions when we go offline
	// (this would be hooked as the ME_CLIST_STATUSMODECHANGE handler except that event is sent *after* the proto goes offline)
	// (instead, it's called from the SettingChanged handler for protocol status db setting changes)
	//HookEvent(ME_CLIST_STATUSMODECHANGE, StatusModeChange);
	
	return 0;
}

int NewContact(WPARAM wParam, LPARAM lParam) {
	// add filter
	HANDLE hContact = (HANDLE)wParam;
	CallService( MS_PROTO_ADDTOCONTACT, ( WPARAM )hContact, ( LPARAM )MODULE );
	
	return 0;
}

extern "C" DLLIMPORT int Load(PLUGINLINK *link) {
	pluginLink=link;

	DuplicateHandle( GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &mainThread, THREAD_SET_CONTEXT, FALSE, 0 );
	mainThreadId = GetCurrentThreadId();

	InitializeCriticalSection(&lib_cs);

	INITCOMMONCONTROLSEX icex;
	// Ensure that the common control DLL is loaded (for listview)
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC  = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex); 	

	if(ServiceExists(MS_DB_SETSETTINGRESIDENT)) { // 0.6+
		CallService(MS_DB_SETSETTINGRESIDENT, TRUE, (LPARAM)(MODULE "/Encrypted"));
	}

	/////////////
	////// init plugin
	PROTOCOLDESCRIPTOR pd = {0};
	pd.cbSize = sizeof(pd);
	pd.szName = MODULE;
	pd.type = PROTOTYPE_ENCRYPTION;
	CallService(MS_PROTO_REGISTERMODULE,0,(LPARAM)&pd);

	// remove us as a filter to all contacts - fix filter type problem
	if(DBGetContactSettingByte(0, MODULE, "FilterOrderFix", 0) != 2) {
		HANDLE hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
		while ( hContact != NULL ) {
			CallService( MS_PROTO_REMOVEFROMCONTACT, ( WPARAM )hContact, ( LPARAM )MODULE );	
			hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
		}	
		DBWriteContactSettingByte(0, MODULE, "FilterOrderFix", 2);
	}

	// add us as a filter to all contacts	
	HANDLE hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
	while ( hContact != NULL ) {
		CallService( MS_PROTO_ADDTOCONTACT, ( WPARAM )hContact, ( LPARAM )MODULE );	
		hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
	}	
	HookEvent(ME_DB_CONTACT_ADDED, NewContact);
	
	// create our services
	CreateProtoServiceFunction(MODULE, PSS_MESSAGE, OTRSendMessage);
	CreateProtoServiceFunction(MODULE, PSS_MESSAGE"W", OTRSendMessageW);
	CreateProtoServiceFunction(MODULE, PSR_MESSAGE, RecvMessage);

	CreateServiceFunction(MS_OTR_MENUSTART, StartOTR);
	CreateServiceFunction(MS_OTR_MENUSTOP, StopOTR);

	if(ServiceExists(MS_LANGPACK_GETCODEPAGE))
		code_page = CallService(MS_LANGPACK_GETCODEPAGE, 0, 0);
	
	// hook modules loaded for updater support
	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	
	// filter added db events, so that we can add the prefix if the option is enabled
	hEventDbEventAddedFilter = HookEvent(ME_DB_EVENT_FILTER_ADD, OnDatabaseEventPreAdd);
	return 0;
}

extern "C" DLLIMPORT int Unload(void) {
	UnhookEvent(hSettingChanged);
	UnhookEvent(hEventWindow);
	UnhookEvent(hEventDbEventAddedFilter);
	UnhookEvent(hEventDbEventAdded);

	DeinitMenu();
	DeinitUtils();
	
	lib_cs_lock();
	otrl_userstate_free(otr_user_state);
	lib_cs_unlock();
	
	DeleteCriticalSection(&lib_cs);
	return 0;
}
