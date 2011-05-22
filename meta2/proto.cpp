#include "common.h"
#include "proto.h"
#include "resource.h"
#include "core_functions.h"
#include "api.h"
#include "priorities.h"

bool firstSetOnline = true;
DWORD status = ID_STATUS_OFFLINE;
UINT_PTR setStatusTimerId = 0;

DWORD next_meta_id = 1;

#define EVENTTYPE_STATUSCHANGE          25368

HANDLE NewMetaContact() {
	HANDLE hMeta = (HANDLE) CallService(MS_DB_CONTACT_ADD, 0, 0);
	DBWriteContactSettingDword(hMeta, MODULE, META_ID, next_meta_id++);
	CallService( MS_PROTO_ADDTOCONTACT, ( WPARAM )hMeta, ( LPARAM )MODULE);
	return hMeta;
}


INT_PTR GetCaps(WPARAM wParam,LPARAM lParam) {
	INT_PTR ret = 0;
    switch (wParam) {
        case PFLAGNUM_1:
			ret = PF1_NUMERICUSERID | PF1_IM | PF1_MODEMSGRECV | PF1_FILESEND;
			break;
        case PFLAGNUM_2:
			ret = PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND
						| PF2_HEAVYDND | PF2_FREECHAT | PF2_OUTTOLUNCH | PF2_ONTHEPHONE;
			break;
        case PFLAGNUM_3:
			ret = PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND
					| PF2_HEAVYDND | PF2_FREECHAT | PF2_OUTTOLUNCH | PF2_ONTHEPHONE;
            break;
        case PFLAGNUM_4:
			ret = PF4_SUPPORTTYPING | PF4_AVATARS | PF4_SUPPORTIDLE | PF4_IMSENDUTF | PF4_IMSENDOFFLINE;
            break;
        case PFLAGNUM_5:
			ret = PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND
						| PF2_HEAVYDND | PF2_FREECHAT | PF2_OUTTOLUNCH | PF2_ONTHEPHONE;
			break;
        case PFLAG_UNIQUEIDTEXT:
            ret = (INT_PTR) Translate("Meta ID");
            break;
        case PFLAG_MAXLENOFMESSAGE:
            ret = 2048;
            break;
        case PFLAG_UNIQUEIDSETTING:
            ret = (INT_PTR) META_ID;
            break;
    }
    return ret;
}

INT_PTR GetName(WPARAM wParam,LPARAM lParam) {
	char *name = (char *)Translate(MODULE);
	size_t size = min(strlen(name),wParam-1);	// copy only the first size bytes.
	if(strncpy((char *)lParam,name,size)==NULL)
		return 1;
	((char *)lParam)[size]='\0';
	return 0;
}

INT_PTR LoadIcon(WPARAM wParam,LPARAM lParam) {
	
	UINT id;
    switch (wParam & 0xFFFF)
	{
        case PLI_PROTOCOL:
			id = IDI_MCMENU;
			break;
		case PLI_ONLINE:
			id = IDI_MCMENU;
			break;
		case PLI_OFFLINE:
			id = IDI_MCMENUOFF;
			break;
		default:
            return 0;
    }

    return (INT_PTR) LoadImage(hInst, MAKEINTRESOURCE(id), IMAGE_ICON,
						GetSystemMetrics(wParam & PLIF_SMALL ? SM_CXSMICON : SM_CXICON),
						GetSystemMetrics(wParam & PLIF_SMALL ? SM_CYSMICON : SM_CYICON), 0);
	return 0;
}

INT_PTR ProtoGetInfo(WPARAM wParam,LPARAM lParam) {
	CCSDATA *ccs = ( CCSDATA* )lParam;

	ccs->hContact = Meta_GetActive(ccs->hContact);
	char *proto = ContactProto(ccs->hContact);
	if(!proto) 
		return 1;

	char szServiceName[256];
	mir_snprintf(szServiceName, 256, "%s%s", proto, PSS_GETINFO); 
	if (ServiceExists(szServiceName)) {
		return CallContactService(ccs->hContact, PSS_GETINFO, ccs->wParam, ccs->lParam);
	}
	return 1;
}

INT_PTR ProtoGetAwayMsg(WPARAM wParam, LPARAM lParam) {
	CCSDATA *ccs = ( CCSDATA* )lParam;

	ccs->hContact = Meta_GetMostOnlineSupporting(ccs->hContact, PFLAGNUM_1, PF1_MODEMSGRECV);
	char *proto = ContactProto(ccs->hContact);
	if(!proto) 
		return 0;

	char szServiceName[256];
	mir_snprintf(szServiceName, 256, "%s%s", proto, PSS_GETAWAYMSG); 
	if (ServiceExists(szServiceName)) {
		return CallContactService(ccs->hContact, PSS_GETAWAYMSG, ccs->wParam, ccs->lParam);
	}
	return 0;
}

void CALLBACK SetStatusProc(HWND hWnd, UINT msg, UINT_PTR id, DWORD dw)
{
	int previousMode = status;
	status = (int)ID_STATUS_ONLINE;
	ProtoBroadcastAck(MODULE, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)previousMode, status);

	KillTimer(0, setStatusTimerId);
}

INT_PTR SetStatus(WPARAM wParam,LPARAM lParam)
{
	// firstSetOnline starts out true - used to delay metacontact's 'onlineness' to prevent double status notifications on startup
	if(status == ID_STATUS_OFFLINE && firstSetOnline) {
		// causes crash on exit if miranda is closed in under options.set_status_from_offline milliseconds!
		//CloseHandle( CreateThread( NULL, 0, SetStatusThread, (void *)wParam, 0, 0 ));		
		setStatusTimerId = SetTimer(0, 0, 10000, SetStatusProc);
		firstSetOnline = FALSE;
	} else {
		int previousMode = status;
		status = (int)wParam;
		ProtoBroadcastAck(MODULE, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)previousMode, status);
	}
	return 0;
}

INT_PTR GetStatus(WPARAM wParam,LPARAM lParam) {
	return status;
}

INT_PTR ProtoSendMessage(WPARAM wParam, LPARAM lParam) {
    CCSDATA *ccs = (CCSDATA *) lParam;
    char *message = (char *)ccs->lParam;
	int flags = ccs->wParam;
	char *buff = 0, *proto = 0;
	HANDLE hMeta = ccs->hContact;

	HANDLE hSendTo = (HANDLE)MetaAPI_GetActiveContact((WPARAM)hMeta, 0);
	proto = ContactProto(hSendTo);
	//DBWriteContactSettingDword(hMeta, MODULE, "TempDefault", (DWORD)hSendTo);

	char szServiceName[256];
	mir_snprintf(szServiceName, 256, "%s", PSS_MESSAGE); 
	if((flags & PREF_UNICODE) && proto != 0) { 
		char szTemp[256]; 
		mir_snprintf(szTemp, 256, "%s%sW", proto, PSS_MESSAGE); 
		if (ServiceExists(szTemp)) 
			strncpy(szServiceName, PSS_MESSAGE "W", sizeof(szServiceName)); 
	}
	// subcontact does not support utf - convert to old ascii/wide format
	if((flags & PREF_UTF) && (proto == 0 || !(CallProtoService(proto, PS_GETCAPS, PFLAGNUM_4, 0) & PF4_IMSENDUTF))) {
		ccs->wParam &= ~PREF_UTF;
		ccs->wParam |= PREF_UNICODE;
		wchar_t *unicode = mir_utf8decodeW(message);
		char *ascii = mir_u2a_cp(unicode, codepage);

		char *buff = new char[strlen(ascii) + 1 + (wcslen(unicode) + 1) * sizeof(wchar_t)];
		strcpy(buff, ascii);
		wcscpy((wchar_t *)(buff + strlen(ascii) + 1), unicode);
		mir_free(unicode);
		mir_free(ascii);
		ccs->lParam = (LPARAM)buff;
	}

	INT_PTR ret = (INT_PTR)CallContactService(hSendTo, szServiceName, ccs->wParam, ccs->lParam);
	if(buff) { // undo our utf8 conversion changes
		ccs->lParam = (LPARAM)message;
		delete[] buff;
	}
	return ret;
}

INT_PTR ProtoSendMessageW(WPARAM wParam, LPARAM lParam) {
    CCSDATA *ccs = (CCSDATA *) lParam;
	if(!(ccs->wParam & PREF_UTF))
		ccs->wParam |= PREF_UNICODE;

	return ProtoSendMessage(wParam, lParam);
}

INT_PTR ProtoRecvMessage(WPARAM wParam, LPARAM lParam) {
    CCSDATA *ccs = (CCSDATA *) lParam;
    PROTORECVEVENT *pre = (PROTORECVEVENT *) ccs->lParam;

	// use the subcontact's protocol to add to the db (AIMOSCAR removes HTML here!) 
	HANDLE most_online = Meta_GetActive(ccs->hContact);
	char *proto = ContactProto(most_online);
	if(proto) {
		char service[256];
		mir_snprintf(service, 256, "%s%s", proto, PSR_MESSAGE);
		if(ServiceExists(service))
			return CallService(service, wParam, lParam);
	}

	return 0;
}

int RedirectACKs(WPARAM wParam, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA*) lParam;
	HANDLE hMeta;

	if(ack->hContact == 0 || (hMeta = (HANDLE)DBGetContactSettingDword(ack->hContact, MODULE, "Handle", 0)) == 0)
		return 0;	// Can't find the MetaID, let through the protocol chain

	if(!strcmp(ack->szModule, MODULE)) {
		return 0; // don't rebroadcast our own acks
	}

	// if it's for something we don't support, ignore
	if(ack->type != ACKTYPE_MESSAGE && ack->type != ACKTYPE_CHAT && ack->type != ACKTYPE_FILE && ack->type != ACKTYPE_AWAYMSG
		&& ack->type != ACKTYPE_AVATAR && ack->type != ACKTYPE_GETINFO)		
	{
		return 0;
	}

	// change the hContact in the avatar info struct, if it's the avatar we're using - else drop it
	if(ack->type == ACKTYPE_AVATAR) {
		if(ack->result == ACKRESULT_SUCCESS || ack->result == ACKRESULT_FAILED || ack->result == ACKRESULT_STATUS) {
			if(ack->hContact == 0) {
				return 0;
			}

			if(ack->hProcess) {
				PROTO_AVATAR_INFORMATION AI;
				memcpy(&AI, (PROTO_AVATAR_INFORMATION *)ack->hProcess, sizeof(PROTO_AVATAR_INFORMATION));
				if(AI.hContact)
					AI.hContact = hMeta;
			
				return ProtoBroadcastAck(MODULE,hMeta,ack->type,ack->result, (HANDLE)&AI, ack->lParam);
			} else
				return ProtoBroadcastAck(MODULE,hMeta,ack->type,ack->result, 0, ack->lParam);
		}
	}

	return ProtoBroadcastAck(MODULE, hMeta, ack->type, ack->result, ack->hProcess, ack->lParam);
}

INT_PTR ProtoGetAvatarInfo(WPARAM wParam, LPARAM lParam) {
    PROTO_AVATAR_INFORMATION *AI = (PROTO_AVATAR_INFORMATION *) lParam;

	HANDLE hMeta = AI->hContact;

	// find the most online contact supporting avatars, according to priorities
	HANDLE most_online = (HANDLE)Meta_GetMostOnlineSupporting(hMeta, PFLAGNUM_4, PF4_AVATARS);
	char *most_online_proto = ContactProto(most_online);

	AI->hContact = most_online;
	char szServiceName[100];  
	mir_snprintf(szServiceName, sizeof(szServiceName), "%s%s", most_online_proto, PS_GETAVATARINFO); 
	int result = CallService(szServiceName, wParam, lParam);
	AI->hContact = hMeta;
	if (result != CALLSERVICE_NOTFOUND) return result;

	return GAIR_NOAVATAR; // fail
}

INT_PTR ProtoFileSend(WPARAM wParam, LPARAM lParam) {
    CCSDATA *ccs = (CCSDATA *) lParam;
	HANDLE hMeta = ccs->hContact;

	// find the most online contact supporting file send, according to priorities
	HANDLE most_online = (HANDLE)Meta_GetMostOnlineSupporting(hMeta, PFLAGNUM_1, PF1_FILESEND);
	if(ContactStatus(most_online, ContactProto(most_online)) == ID_STATUS_OFFLINE)
		most_online = (HANDLE)Meta_GetMostOnlineSupporting(hMeta, PFLAGNUM_4, PF4_OFFLINEFILES);

	int result = CallContactService(most_online, PSS_FILE, ccs->wParam, ccs->lParam);
	if(result != CALLSERVICE_NOTFOUND) return result;

	return 0; // fail
}

INT_PTR ProtoUserIsTyping(WPARAM wParam, LPARAM lParam) {
	HANDLE hMeta = (HANDLE)wParam;
	// find the most online contact supporting typing notifications, according to priorities
	HANDLE most_online = (HANDLE)Meta_GetMostOnlineSupporting(hMeta, PFLAGNUM_4, PF4_SUPPORTTYPING);
	char *most_online_proto = ContactProto(most_online);

	char szServiceName[256];  
	mir_snprintf(szServiceName, sizeof(szServiceName), "%s%s", most_online_proto, PSS_USERISTYPING); 
	CallService(szServiceName, (WPARAM)most_online, lParam);
	return 0;
}

int EventContactIsTyping(WPARAM wParam, LPARAM lParam) {
	HANDLE hMeta;

	if((hMeta = (HANDLE)DBGetContactSettingDword((HANDLE)wParam, MODULE, "Handle", 0)) != 0 && MetaEnabled()) {	
		// try to remove any clist events added for subcontact
		CallServiceSync(MS_CLIST_REMOVEEVENT, wParam, (LPARAM) 1);

		CallService(MS_PROTO_CONTACTISTYPING, (WPARAM)hMeta, lParam);

		// stop processing of event
		return 1;
	}

	return 0;
}

INT_PTR SendNudge(WPARAM wParam,LPARAM lParam)
{
	HANDLE hMeta = (HANDLE)wParam,
		hSubContact = Meta_GetActive(hMeta);

	char servicefunction[256];
	char *protoName = ContactProto(hSubContact);
	mir_snprintf(servicefunction, 256, "%s/SendNudge", protoName);

	return CallService(servicefunction, (WPARAM)hSubContact, lParam);
}

int ContactDeleted(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;
	if(IsSubcontact(hContact)) {
		Meta_Remove(hContact);
	} else if(IsMetacontact(hContact)) {
		SubcontactList::Iterator i = metaMap[hContact].start();
		HANDLE hSub;
		while(i.has_val()) {
			// same functionality as Meta_Remove - except the meta is not deleted when all subcontacts are removed here since it's already happening,
			// and the 'subcontacts changed' event isn't fired
			hSub = i.val().handle();
			DBDeleteContactSetting(hSub, MODULE, "ParentMetaID");
			// deleting these (resident) settings doesn't work :( [25/9/07]
			DBWriteContactSettingDword(hSub, MODULE, "Handle", 0);
			DBWriteContactSettingByte(hSub, MODULE, "IsSubcontact", 0);
			if(!meta_group_hack_disabled) DBWriteContactSettingByte(hSub, "CList", "Hidden", 0);

			CallService(MS_IGNORE_UNIGNORE, (WPARAM)hSub, (WPARAM)IGNOREEVENT_USERONLINE);

			i.next();
		}
		metaMap.remove(hContact);
	}
	return 0;
}

int SettingChanged(WPARAM wParam, LPARAM lParam) {
	if(wParam == 0 || !IsSubcontact((HANDLE)wParam)) return 0;

	DBCONTACTWRITESETTING *dcws = (DBCONTACTWRITESETTING *)lParam;
	if(strcmp(dcws->szSetting, "Status") == 0) {
		// subcontact status has changed
		Meta_CalcStatus((HANDLE)DBGetContactSettingDword((HANDLE)wParam, MODULE, "Handle", 0));
	}

	// keep subcontacts hidden if the clist doesn't do it for us
	if(strcmp(dcws->szSetting, "Hidden") == 0 && strcmp(dcws->szModule, "CList") == 0 && MetaEnabled() && !meta_group_hack_disabled) {
		if(dcws->value.type == DBVT_DELETED || dcws->value.bVal == 0) {
			DBWriteContactSettingByte((HANDLE)wParam, "CList", "Hidden", 1);
		}
	}

	return 0;
}

int MetaChanged(WPARAM wParam, LPARAM lParam) {
	Meta_CalcStatus((HANDLE)wParam);
	return 0;
}

int WindowEvent(WPARAM wParam, LPARAM lParam) {
	MessageWindowEventData *mwed = (MessageWindowEventData *)lParam;
	bool sub = IsSubcontact(mwed->hContact), meta = IsMetacontact(mwed->hContact);
	if(sub || meta) {
		if(mwed->uType == MSG_WINDOW_EVT_OPEN || mwed->uType == MSG_WINDOW_EVT_OPENING) {
			DBWriteContactSettingByte(mwed->hContact, MODULE, "WindowOpen", 1);
			if(meta)
				CallService(MS_CLIST_REMOVEEVENT, (WPARAM)mwed->hContact, (LPARAM)EVENTTYPE_MESSAGE);
		} else if(mwed->uType == MSG_WINDOW_EVT_CLOSE || mwed->uType == MSG_WINDOW_EVT_CLOSING) {
			DBWriteContactSettingByte(mwed->hContact, MODULE, "WindowOpen", 0);
			if(meta) {
				DBWriteContactSettingDword(mwed->hContact, MODULE, "TempDefault", 0);
				DBWriteContactSettingDword(mwed->hContact, MODULE, "ForceSend", 0);
			}
		}

	}
	return 0;
}

void RegisterProto() {
	PROTOCOLDESCRIPTOR pd = {0};
	pd.cbSize = PROTOCOLDESCRIPTOR_V3_SIZE; //sizeof(pd);
	pd.szName = MODULE;
	pd.type = PROTOTYPE_PROTOCOL; 
	CallService(MS_PROTO_REGISTERMODULE,0,(LPARAM)&pd);
}

// redirect events - works if the contact is both a sub and a meta :)
#define DBEF_META		0x2000
int EventFilterAdd(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;
	DBEVENTINFO *dbei = (DBEVENTINFO *)lParam;

	// prevent recursion
	if(dbei->flags & DBEF_META) return 0;
	dbei->flags |= DBEF_META;

	HANDLE hMeta;
	//events passed up to parent
	while((hMeta = (HANDLE)DBGetContactSettingDword(hContact, MODULE, "Handle", 0)) != 0) {
		// subcontact event being added (e.g. by subcontact proto) - add to meta
		CallService(MS_DB_EVENT_ADD, (WPARAM)hMeta, (LPARAM)dbei);

		if(dbei->eventType == EVENTTYPE_MESSAGE) {
			// set default
			int num = metaMap[hMeta].index_of(hContact);
			if(num != -1 && num != DBGetContactSettingByte(hMeta, MODULE, "Default", -1))
				MetaAPI_SetDefaultContactNum((WPARAM)hMeta, (LPARAM)num);

			// set meta nick	
			char *proto = ContactProto(hContact);
			DBVARIANT dbv;
			if(proto && !DBGetContactSettingUTF8String(0, proto, "Nick", &dbv)) {
				DBWriteContactSettingUTF8String(0, MODULE, "Nick", dbv.pszVal);
				DBFreeVariant(&dbv);
			}

			// set temp default
			DBWriteContactSettingDword(hMeta, MODULE, "TempDefault", (DWORD)hContact);
		}

		// mark as read if subcontact window not open
		if((dbei->flags & DBEF_READ) == 0 && (dbei->flags & DBEF_SENT) == 0 && DBGetContactSettingByte(hContact, MODULE, "WindowOpen", 0) == 0) {
			dbei->flags |= DBEF_READ;
		}

		hContact = hMeta;
	}

	// events passed down to children
	hContact = (HANDLE)wParam; // reset to the source event's contact
	HANDLE hSub;
	while((hSub = Meta_GetActive(hContact)) != 0) {
		CallService(MS_DB_EVENT_ADD, (WPARAM)hSub, (LPARAM)dbei);
		hContact = hSub;
	}

	// let original event through
	return 0;
}

#define NUM_HOOKS_INTERNAL 10
int hookIndex = 0;
HANDLE hHooksInternal[NUM_HOOKS_INTERNAL] = {0};

HANDLE hEventMessageWindow = 0;
int ModulesLoadedProto(WPARAM wParam, LPARAM lParam) {
	hHooksInternal[hookIndex++] = (HANDLE)HookEvent(ME_MSG_WINDOWEVENT, WindowEvent);

	Meta_Hide(DBGetContactSettingByte(0, MODULE, "Enabled", 1) == 0);

	// hooked here so we get it later (than e.g. OTR)
	hHooksInternal[hookIndex++] = (HANDLE)HookEvent(ME_DB_EVENT_FILTER_ADD, EventFilterAdd);
	return 0;
}

#define NUM_SERVICES 14
HANDLE hServices[NUM_SERVICES] = {0};

void InitProto() {
	RegisterProto();

	// create our services
	int i = 0;

	hServices[i++] = CreateProtoServiceFunction(MODULE, PS_GETCAPS, GetCaps);
	hServices[i++] = CreateProtoServiceFunction(MODULE, PS_GETNAME, GetName);
	hServices[i++] = CreateProtoServiceFunction(MODULE, PS_LOADICON, LoadIcon);
	hServices[i++] = CreateProtoServiceFunction(MODULE, PS_SETSTATUS, SetStatus);
	hServices[i++] = CreateProtoServiceFunction(MODULE, PS_GETSTATUS, GetStatus);

	hServices[i++] = CreateProtoServiceFunction(MODULE, PSS_GETINFO, ProtoGetInfo);

	hServices[i++] = CreateProtoServiceFunction(MODULE, PSS_MESSAGE, ProtoSendMessage);
	hServices[i++] = CreateProtoServiceFunction(MODULE, PSS_MESSAGE"W", ProtoSendMessageW);
	hServices[i++] = CreateProtoServiceFunction(MODULE, PSR_MESSAGE, ProtoRecvMessage);

	hServices[i++] = CreateProtoServiceFunction(MODULE, PSS_GETAWAYMSG, ProtoGetAwayMsg);

	hServices[i++] = CreateProtoServiceFunction(MODULE, PS_GETAVATARINFO, ProtoGetAvatarInfo);
	hServices[i++] = CreateProtoServiceFunction(MODULE, PSS_FILE, ProtoFileSend);
	hServices[i++] = CreateProtoServiceFunction(MODULE, PSS_USERISTYPING, ProtoUserIsTyping);

	// REMEMBER to modify the NUM_SERVICES #define above if you add more services!

	hServices[i++] = CreateProtoServiceFunction(MODULE, "/SendNudge", SendNudge);

	hHooksInternal[hookIndex++] = HookEvent(ME_DB_CONTACT_DELETED, ContactDeleted);
	hHooksInternal[hookIndex++] = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, SettingChanged);

	hHooksInternal[hookIndex++] = (HANDLE)HookEvent(ME_MC_DEFAULTTCHANGED, MetaChanged );
	hHooksInternal[hookIndex++] = (HANDLE)HookEvent(ME_MC_FORCESEND, MetaChanged );
	hHooksInternal[hookIndex++] = (HANDLE)HookEvent(ME_MC_UNFORCESEND, MetaChanged );
	hHooksInternal[hookIndex++] = (HANDLE)HookEvent(ME_PROTO_ACK, RedirectACKs);
	hHooksInternal[hookIndex++] = (HANDLE)HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoadedProto);
	hHooksInternal[hookIndex++] = (HANDLE)HookEvent(ME_PROTO_CONTACTISTYPING, EventContactIsTyping);
}

void DeinitProto() {
	for(int i = 0; i < NUM_SERVICES; i++)
		DestroyServiceFunction(hServices[i]);
	for(int j = 0; j < NUM_HOOKS_INTERNAL; j++)
		UnhookEvent(hHooksInternal[j]);

}