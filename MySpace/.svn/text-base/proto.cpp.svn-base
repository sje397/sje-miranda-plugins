#include "common.h"
#include "proto.h"
#include "server_con.h"
#include "resource.h"
#include "formatting.h"

#include <malloc.h>
#include <ctime>

// in case it's not in m_protosvc.h
#ifndef PF4_IMSENDOFFLINE
#define PF4_IMSENDOFFLINE 0x00000100 // protocol is able to send offline messages (v.0.8.0+ 2007/10/3)
#endif

#define FAILED_MESSAGE_HANDLE			99998

int msg_id = 1;

int GetCaps(WPARAM wParam,LPARAM lParam) {
	int ret = 0;
    switch (wParam) {
        case PFLAGNUM_1:
			ret = PF1_NUMERICUSERID | PF1_IM | PF1_BASICSEARCH | PF1_SEARCHBYEMAIL | PF1_SEARCHBYNAME | PF1_ADDSEARCHRES | PF1_SERVERCLIST | PF1_MODEMSG;
			break;
        case PFLAGNUM_2:
			ret = PF2_ONLINE | PF2_SHORTAWAY | PF2_INVISIBLE;
			break;
        case PFLAGNUM_3:
			ret = PF2_ONLINE | PF2_SHORTAWAY | PF2_INVISIBLE;
            break;
        case PFLAGNUM_4:
			ret = PF4_SUPPORTTYPING | PF4_AVATARS | PF4_SUPPORTIDLE | PF4_IMSENDUTF | PF4_IMSENDOFFLINE;
            break;
        case PFLAGNUM_5:
			//ret = PF2_INVISIBLE;
			break;
        case PFLAG_UNIQUEIDTEXT:
            ret = (int) Translate("UserID");
            break;
        case PFLAG_MAXLENOFMESSAGE:
            ret = MAX_MESSAGE_SIZE;
            break;
        case PFLAG_UNIQUEIDSETTING:
            ret = (int) "UID";
            break;
    }
    return ret;
}

int GetName(WPARAM wParam,LPARAM lParam) {
	mir_snprintf((char *)lParam, wParam, "%s", MODULE);
	return 0;
}

int LoadIcon(WPARAM wParam,LPARAM lParam) {
	
	UINT id;
    switch (wParam & 0xFFFF)
	{
        case PLI_PROTOCOL:
			id = IDI_MYSPACE;
			break;
		case PLI_ONLINE:
			id = IDI_MYSPACE;
			break;
		case PLI_OFFLINE:
			id = IDI_MYSPACE;
			break;
		default:
            return (int) (HICON) NULL;
    }

    return (int) LoadImage(hInst, MAKEINTRESOURCE(id), IMAGE_ICON,
						GetSystemMetrics(wParam & PLIF_SMALL ? SM_CXSMICON : SM_CXICON),
						GetSystemMetrics(wParam & PLIF_SMALL ? SM_CYSMICON : SM_CYICON), 0);
	return 0;
}

int GetInfo(WPARAM wParam,LPARAM lParam) {
	CCSDATA *ccs = ( CCSDATA* )lParam;
	int uid;
	if((uid = DBGetContactSettingDword(ccs->hContact, MODULE, "UID", 0)) == 0)
		return 1; // fail

	LookupUID(uid);
	return 0;
}

int SetStatus(WPARAM wParam,LPARAM lParam) {
	if(wParam == status) return 0;

	SetServerStatus(wParam);
	return 0;
}

int GetStatus(WPARAM wParam,LPARAM lParam) {
	return status;
}

void __cdecl sttFakeAckMessageSuccess(void *param) {
	HANDLE hContact = (HANDLE)param;

	SleepEx( 10, TRUE );
	ProtoBroadcastAck(MODULE, hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)1, 0 );
}

void __cdecl sttFakeAckMessageFailed(void *param) {
	HANDLE hContact = (HANDLE)param;

	SleepEx( 10, TRUE );
	ProtoBroadcastAck(MODULE, hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)FAILED_MESSAGE_HANDLE, 0);
}

int ProtoSendMessage(WPARAM wParam, LPARAM lParam) {
    CCSDATA *ccs = (CCSDATA *) lParam;
    char *message = (char *)ccs->lParam;
	char msg_utf[MAX_MESSAGE_SIZE + 1], msg_fmt[MAX_MESSAGE_SIZE + 128];
	int uid;

	if((uid = DBGetContactSettingDword(ccs->hContact, MODULE, "UID", 0)) == 0 || status == ID_STATUS_OFFLINE) {
		mir_forkthread(sttFakeAckMessageFailed, ccs->hContact);
		return FAILED_MESSAGE_HANDLE;
	}

	// TODO: process 'message' and/or 'messagew' below
	if(ccs->wParam & PREF_UTF) {
		strncpy(msg_utf, message, MAX_MESSAGE_SIZE);
	} else if(ccs->wParam & PREF_UNICODE) {
		wchar_t *messagew = (wchar_t *)&message[strlen(message)+1];
		WideCharToMultiByte(CP_UTF8, 0, messagew, -1, msg_utf, MAX_MESSAGE_SIZE, 0, 0);
	} else {
		strncpy(msg_utf, message, MAX_MESSAGE_SIZE);
	}
	msg_utf[MAX_MESSAGE_SIZE] = 0;

	entitize_xml(msg_utf, MAX_MESSAGE_SIZE + 1);
	encode_smileys(msg_utf, MAX_MESSAGE_SIZE + 1);
	mir_snprintf(msg_fmt, MAX_MESSAGE_SIZE + 1, "<p><f f='Times' h='16'><c v='black'><b v='white'>%s</b></c></f></p>", msg_utf);

	ClientNetMessage msg;
	msg.add_int("bm", 1);
	msg.add_int("sesskey", sesskey);
	msg.add_int("t", uid);
	msg.add_int("f", my_uid);
	msg.add_int("cv", CLIENT_VER);
	msg.add_string("msg", msg_fmt);
	SendMessage(msg);

	mir_forkthread(sttFakeAckMessageSuccess, ccs->hContact);
	return 1;
}

int ProtoSendMessageW(WPARAM wParam, LPARAM lParam) {
    CCSDATA *ccs = (CCSDATA *) lParam;
	ccs->wParam |= PREF_UNICODE;

	return ProtoSendMessage(wParam, lParam);
}

int ProtoRecvMessage(WPARAM wParam, LPARAM lParam) {
	DBEVENTINFO dbei;
	CCSDATA* ccs = (CCSDATA*)lParam;
	PROTORECVEVENT* pre = (PROTORECVEVENT*)ccs->lParam;

	DBDeleteContactSetting(ccs->hContact, "CList", "Hidden");

	ZeroMemory(&dbei, sizeof(dbei));
	dbei.cbSize = sizeof(dbei);
	dbei.szModule = MODULE;
	dbei.timestamp = pre->timestamp;
	dbei.flags = (pre->flags & PREF_CREATEREAD) ? DBEF_READ : 0;
	dbei.flags |= (pre->flags & PREF_RTL) ? DBEF_RTL : 0;
	dbei.flags |= (pre->flags & PREF_UTF) ? DBEF_UTF : 0;
	dbei.eventType = pre->lParam;
	dbei.cbBlob = strlen(pre->szMessage) + 1;
	dbei.pBlob = (PBYTE)pre->szMessage;

	CallService(MS_DB_EVENT_ADD, (WPARAM)ccs->hContact, (LPARAM)&dbei);
	return 0;

}

int BasicSearch(WPARAM wParam, LPARAM lParam) {
	if(status <= ID_STATUS_OFFLINE) return 0;
	char *szId = (char *)lParam;

	ClientNetMessage msg;
	msg.add_int("persist", 1);
	msg.add_int("sesskey", sesskey);
	msg.add_int("uid", DBGetContactSettingDword(0, MODULE, "UID", 0));

	msg.add_int("cmd", 1);
	msg.add_int("dsn", 4);
	msg.add_int("lid", 3);

	int req = req_id++;
	msg.add_int("rid", req);

	char body[512];
	mir_snprintf(body, 512, "UserID=%s", szId);
	msg.add_string("body", body);

	SendMessage(msg);

	return req;
}

int SearchByEmail(WPARAM wParam, LPARAM lParam) {
	if(status <= ID_STATUS_OFFLINE) return 0;
	char *email = (char*)lParam;

	ClientNetMessage msg;
	msg.add_int("persist", 1);
	msg.add_int("sesskey", sesskey);
	msg.add_int("uid", DBGetContactSettingDword(0, MODULE, "UID", 0));

	msg.add_int("cmd", 1);
	msg.add_int("dsn", 5);
	msg.add_int("lid", 7);

	int req = req_id++;
	msg.add_int("rid", req);

	char body[512];
	mir_snprintf(body, 512, "Email=%s", email);
	msg.add_string("body", body);

	SendMessage(msg);
	return req;
}

int SearchByName(WPARAM wParam, LPARAM lParam) {
	if(status <= ID_STATUS_OFFLINE) return 0;
	PROTOSEARCHBYNAME *sbn = (PROTOSEARCHBYNAME *)lParam;
	if(!sbn->pszNick) return 0;

	ClientNetMessage msg;
	msg.add_int("persist", 1);
	msg.add_int("sesskey", sesskey);
	msg.add_int("uid", DBGetContactSettingDword(0, MODULE, "UID", 0));

	msg.add_int("cmd", 1);
	msg.add_int("dsn", 5);
	msg.add_int("lid", 7);

	int req = req_id++;
	msg.add_int("rid", req);

	char body[512];
	mir_snprintf(body, 512, "UserName=%s", sbn->pszNick);
	msg.add_string("body", body);

	SendMessage(msg);
	return req;
}

int AddToList(WPARAM wParam, LPARAM lParam) {
	MYPROTOSEARCHRESULT *mpsr = (MYPROTOSEARCHRESULT *)lParam;
	bool temp = (wParam & PALF_TEMPORARY) != 0;

	HANDLE hContact = FindContact(mpsr->uid);
	if(!hContact) {
		hContact = CreateContact(mpsr->uid, mpsr->psr.nick, mpsr->psr.email, temp ? false : true);
	} else {
		if(!temp) {
			AddBuddyOnServer(mpsr->uid);

			/*
			ClientNetMessage msg_add;
			msg_add.add_string("addbuddy", "");
			msg_add.add_int("sesskey", sesskey);
			msg_add.add_int("newprofileid", DBGetContactSettingDword(hContact, MODULE, "UID", 0));
			msg_add.add_string("reason", "");

			SendMessage(msg_add);
			*/
		}
	}

	if(temp) {
		DBWriteContactSettingByte(hContact, "CList", "NotOnList", 1);
		DBWriteContactSettingByte(hContact, "CList", "Hidden", 1);
	} else {
		DBDeleteContactSetting(hContact, "CList", "NotOnList");
		DBDeleteContactSetting(hContact, "CList", "Hidden");

		LookupUID(mpsr->uid);
	}

	return (int)hContact;
}

void __cdecl sttRecvAway(void *param) {
	HANDLE hContact = (HANDLE)param;

	DBVARIANT dbv;
	char buff[512];
	buff[0] = 0;

	if(!DBGetContactSettingStringUtf(hContact, MODULE, "StatusMsg", &dbv) && strlen(dbv.pszVal)) {
		mir_snprintf(buff, 512, "%s", dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	ProtoBroadcastAck(MODULE, hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, (LPARAM)buff);
}


int GetAwayMsg(WPARAM wParam, LPARAM lParam) {
	CCSDATA* ccs = (CCSDATA*)lParam;
	if ( ccs == NULL )
		return 0;

	mir_forkthread(sttRecvAway, ccs->hContact);
	return 1;
}

int SetAwayMsg(WPARAM wParam, LPARAM lParam) {
	int status = (int)wParam;
	char *msg = (char *)lParam;

	// called *just after* SetStatus :(
	SetServerStatusMessage(msg);

	return 0;
}

int SendTyping(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;
	int flags = (int)lParam, uid;
	if(status != ID_STATUS_OFFLINE) {
		if((uid = DBGetContactSettingDword(hContact, MODULE, "UID", 0)) != 0) {
			ClientNetMessage msg;
			msg.add_int("bm", 121);
			msg.add_int("sesskey", sesskey);
			msg.add_int("t", uid);
			msg.add_int("f", my_uid);
			msg.add_int("cv", CLIENT_VER);
			msg.add_string("msg", (flags == PROTOTYPE_SELFTYPING_ON ? "%typing%" : "%stoptyping%"));
			SendMessage(msg);
		}
	}
	return 0;
}

bool FileExists(char *filename) {
	HANDLE hFile = CreateFileA(filename, 0, 0, 0, OPEN_EXISTING, 0, 0);
	if(hFile != INVALID_HANDLE_VALUE) {
		CloseHandle(hFile);
		return true;
	}
	return false;
}

int GetAvatarInfo(WPARAM wParam,LPARAM lParam) {
	if (DBGetContactSettingByte(0, MODULE, "EnableAvatars", 1) == 0)
		return GAIR_NOAVATAR;

	PROTO_AVATAR_INFORMATION* AI = ( PROTO_AVATAR_INFORMATION* )lParam;

	int ret = GAIR_NOAVATAR;
	DBVARIANT dbv, dbv2;

	bool url_exists = (DBGetContactSettingString(AI->hContact, MODULE, "ImageURL", &dbv) == 0);
	if(!url_exists) {
		DBFreeVariant(&dbv);
		return GAIR_NOAVATAR;
	}

	bool online = (status > ID_STATUS_OFFLINE);
	bool force = ((wParam & GAIF_FORCE) != 0);
	bool free_dbv2 = (DBGetContactSettingString(AI->hContact, MODULE, "AvatarFilename", &dbv2) == 0);
	bool file_exists = free_dbv2 && FileExists(dbv2.pszVal);

	if(file_exists) {
		strncpy(AI->filename, dbv2.pszVal, sizeof(AI->filename));
		char *ext = strrchr(AI->filename, '.');
		if(ext) ext++;
		if(ext) {
			AI->format = 
				(stricmp(ext, ".png") ? PA_FORMAT_PNG :
				(stricmp(ext, ".jpg") ? PA_FORMAT_JPEG :
				(stricmp(ext, ".jpeg") ? PA_FORMAT_JPEG :
				(stricmp(ext, ".gif") ? PA_FORMAT_GIF :
				(stricmp(ext, ".swf") ? PA_FORMAT_SWF : PA_FORMAT_UNKNOWN)))));
		} else
			AI->format = PA_FORMAT_UNKNOWN;

		ret = GAIR_SUCCESS;
	} else if(online) {
		DownloadAvatar(AI->hContact, dbv.pszVal);
		ret = GAIR_WAITFOR;
	} else {
		ret = GAIR_NOAVATAR;
	}

	DBFreeVariant(&dbv);
	if(free_dbv2) DBFreeVariant(&dbv2);
	return ret;
}


int GetMyAvatar(WPARAM wParam,LPARAM lParam) {
	char *buff = (char *)wParam;
	DBVARIANT dbv;
	if(!DBGetContactSettingString(0, MODULE, "AvatarFilename", &dbv)) {
		mir_snprintf(buff, (int)lParam, "%s", dbv.pszVal);
		DBFreeVariant(&dbv);
		return 0;
	}
	return 1;
}

int GetAvatarCaps(WPARAM wParam,LPARAM lParam) {
	switch(wParam) {
		case AF_MAXSIZE:
			{
				POINT *p = (POINT *)lParam;
				p->x = p->y = -1;
			}
			return 0;
		case AF_PROPORTION:
			return PIP_NONE;
		case AF_FORMATSUPPORTED:
			return 0;
		case AF_ENABLED:
			return 1;
		case AF_DONTNEEDDELAYS:
			return 1;
		case AF_MAXFILESIZE:
			return 0;
		case AF_DELAYAFTERFAIL:
			return 0;
	}
	return 0;
}

int ContactDeleted(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;
	int uid = DBGetContactSettingDword(hContact, MODULE, "UID", 0);
	if(uid) {
		if(DBGetContactSettingByte(hContact, "CList", "NotOnList", 0) == 0) {
			ClientNetMessage msg_del;
			msg_del.add_string("delbuddy", "");
			msg_del.add_int("sesskey", sesskey);
			msg_del.add_int("delprofileid", uid);
			SendMessage(msg_del);

			ClientNetMessage msg_persist;
			msg_persist.add_int("persist", 1);
			msg_persist.add_int("sesskey", sesskey);
			msg_persist.add_int("cmd", 515);
			msg_persist.add_int("dsn", 0);
			msg_persist.add_int("uid", DBGetContactSettingDword(0, MODULE, "UID", 0));
			msg_persist.add_int("lid", 8);
			msg_persist.add_int("rid", req_id++);
			char body[1024];
			mir_snprintf(body, 1024, "ContactID=%d", uid);
			msg_persist.add_string("body", body);
			SendMessage(msg_persist);

			ClientNetMessage msg_block;
			msg_block.add_string("blocklist", "");
			msg_block.add_int("sesskey", sesskey);
			char idlist[1024];
			mir_snprintf(idlist, 1024, "a-|%d|b-|%d", uid, uid);
			msg_block.add_string("idlist", idlist);
			SendMessage(msg_block);
		}
	}
	return 0;
}

void RegisterProto() {
	PROTOCOLDESCRIPTOR pd = {0};
	pd.cbSize = sizeof(pd);
	pd.szName = MODULE;
	pd.type = PROTOTYPE_PROTOCOL; 
	CallService(MS_PROTO_REGISTERMODULE,0,(LPARAM)&pd);
}

int IdleChanged(WPARAM wParam, LPARAM lParam) {
	if(lParam & IDF_PRIVACY) return 0;

	if((lParam & IDF_ISIDLE) && status == ID_STATUS_ONLINE) { // mimic official client - doesn't go idle when away (or invisible?)
		SetServerStatus(ID_STATUS_IDLE);
	}
	return 0;
}

int SettingChanged(WPARAM wParam, LPARAM lParam) {
	if(!wParam) return 0;

	DBCONTACTWRITESETTING *dbcws = (DBCONTACTWRITESETTING*)lParam;
	if(strcmp(dbcws->szModule, "CList") != 0) return 0;
	if(strcmp(dbcws->szSetting, "NotOnList") != 0) return 0;

	if(dbcws->value.type != DBVT_DELETED && dbcws->value.bVal != 0) return 0;

	HANDLE hContact = (HANDLE)wParam;
	char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	if(proto == 0 || strcmp(proto, MODULE) != 0) return 0;

	int uid = DBGetContactSettingDword(hContact, MODULE, "UID", 0);

	ClientNetMessage msg_add;
	msg_add.add_string("addbuddy", "");
	msg_add.add_int("sesskey", sesskey);
	msg_add.add_int("newprofileid", DBGetContactSettingDword(hContact, MODULE, "UID", 0));
	msg_add.add_string("reason", "");

	SendMessage(msg_add);
	return 0;
}

#define NUM_FILTER_SERVICES 19
HANDLE hServices[NUM_FILTER_SERVICES];

HANDLE hEventContactDeleted, hEventIdle, hEventSettingChanged;
void CreateProtoServices() {
	// create our services
	int i = 0;
	hServices[i++] = CreateProtoServiceFunction(MODULE, PSS_MESSAGE, ProtoSendMessage);
	hServices[i++] = CreateProtoServiceFunction(MODULE, PSS_MESSAGE"W", ProtoSendMessageW);
	hServices[i++] = CreateProtoServiceFunction(MODULE, PSR_MESSAGE, ProtoRecvMessage);

	hServices[i++] = CreateProtoServiceFunction(MODULE, PS_GETCAPS, GetCaps);
	hServices[i++] = CreateProtoServiceFunction(MODULE, PS_GETNAME, GetName);
	hServices[i++] = CreateProtoServiceFunction(MODULE, PS_LOADICON,LoadIcon);
	hServices[i++] = CreateProtoServiceFunction(MODULE, PSS_GETINFO,GetInfo);
	hServices[i++] = CreateProtoServiceFunction(MODULE, PS_SETSTATUS, SetStatus);
	hServices[i++] = CreateProtoServiceFunction(MODULE, PS_GETSTATUS, GetStatus);

	hServices[i++] = CreateProtoServiceFunction(MODULE, PS_BASICSEARCH, BasicSearch);
	hServices[i++] = CreateProtoServiceFunction(MODULE, PS_SEARCHBYEMAIL, SearchByEmail);
	hServices[i++] = CreateProtoServiceFunction(MODULE, PS_SEARCHBYNAME, SearchByName);
	hServices[i++] = CreateProtoServiceFunction(MODULE, PS_ADDTOLIST, AddToList);

	hServices[i++] = CreateProtoServiceFunction(MODULE, PSS_GETAWAYMSG, GetAwayMsg);
	hServices[i++] = CreateProtoServiceFunction(MODULE, PS_SETAWAYMSG, SetAwayMsg);

	hServices[i++] = CreateProtoServiceFunction(MODULE, PSS_USERISTYPING, SendTyping);

	hServices[i++] = CreateProtoServiceFunction(MODULE, PS_GETAVATARINFO, GetAvatarInfo);
	hServices[i++] = CreateProtoServiceFunction(MODULE, PS_GETMYAVATAR, GetMyAvatar);
	hServices[i++] = CreateProtoServiceFunction(MODULE, PS_GETAVATARCAPS, GetAvatarCaps);
	// remember to modify the NUM_FILTER_SERVICES #define above if you add more services!

	hEventContactDeleted = HookEvent(ME_DB_CONTACT_DELETED, ContactDeleted);
	hEventIdle = HookEvent(ME_IDLE_CHANGED, IdleChanged);
	hEventSettingChanged = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, SettingChanged);
}

void DeinitProto() {
	UnhookEvent(hEventIdle);
	UnhookEvent(hEventContactDeleted);
	for(int i = 0; i < NUM_FILTER_SERVICES; i++)
		DestroyServiceFunction(hServices[i]);
}
