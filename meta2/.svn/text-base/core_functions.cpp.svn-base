#include "common.h"
#include "core_functions.h"
#include "proto.h"
#include "api.h"
#include "priorities.h"

MetaMap metaMap;

HANDLE GetMetaHandle(DWORD id) {
	HANDLE hContact = (HANDLE)CallService( MS_DB_CONTACT_FINDFIRST, 0, 0);
	char *proto;
	while(hContact) {
		proto = ContactProto(hContact);
		if(proto && !strcmp(proto, MODULE)) {
			DWORD mid = DBGetContactSettingDword(hContact, MODULE, META_ID, (DWORD)-1);
			if(mid == id) return hContact;
		}

		hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
	}	
	return 0;
}

void Meta_Hide(bool hide_metas) {
	HANDLE hContact = (HANDLE)CallService( MS_DB_CONTACT_FINDFIRST, 0, 0);
	while(hContact != NULL) {
		if(IsMetacontact(hContact)) {
			DBWriteContactSettingByte(hContact, "CList", "Hidden", hide_metas ? 1 : 0);
		} else if(IsSubcontact(hContact)) {
			if(!meta_group_hack_disabled)
				DBWriteContactSettingByte(hContact, "CList", "Hidden", hide_metas ? 0 : 1);
			CallService(hide_metas ? MS_IGNORE_UNIGNORE : MS_IGNORE_IGNORE, (WPARAM)hContact, (WPARAM)IGNOREEVENT_USERONLINE);
		}

		hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
	}	
}

HANDLE Meta_GetActive(HANDLE hMeta) {
	HANDLE hActive = 0;
	if(DBGetContactSettingByte(hMeta, MODULE, "ForceDefault", 0))
		hActive = (HANDLE)MetaAPI_GetDefault((WPARAM)hMeta, 0);
	if(!hActive) {
		if((hActive = (HANDLE)DBGetContactSettingDword(hMeta, MODULE, "ForceSend", 0)) == 0) {
			if((hActive = (HANDLE)DBGetContactSettingDword(hMeta, MODULE, "TempDefault", 0)) == 0) {
				hActive = Meta_GetMostOnline(hMeta);
				if(ContactStatus(hActive, ContactProto(hActive)) == ID_STATUS_OFFLINE) {
					hActive = Meta_GetMostOnlineSupporting(hMeta, PFLAGNUM_4, PF4_IMSENDOFFLINE);
				}
			}
		}
	}
	return hActive;
}

HANDLE Meta_GetMostOnline(HANDLE hMeta) {
	return Meta_GetMostOnlineSupporting(hMeta, PFLAGNUM_1, PF1_IM);
}

HANDLE Meta_GetMostOnlineSupporting(HANDLE hMeta, int flag, int cap) {
	if(!metaMap.exists(hMeta)) return 0;

	HANDLE most_online = (HANDLE)MetaAPI_GetDefault((WPARAM)hMeta, 0);
	char *most_online_proto = ContactProto(most_online);
	int most_online_status = ID_STATUS_OFFLINE,
		most_online_prio = MAX_PRIORITY;
	if(most_online_proto) {
		char szService[256];
		mir_snprintf(szService, sizeof(szService), "%s%s", most_online_proto, PS_GETCAPS);
		if(CallService(szService, (WPARAM)flag, (LPARAM)0) & cap) {
			most_online_status = ContactStatus(most_online, most_online_proto);
			most_online_prio = GetRealPriority(most_online_proto, most_online_status);
		}
	}

	char *proto;
	int status, prio;
	SubcontactList::Iterator i = metaMap[hMeta].start();
	while(i.has_val()) {
		proto = ContactProto(i.val().handle());
		if(proto) {
			char szService[256];
			mir_snprintf(szService, sizeof(szService), "%s%s", proto, PS_GETCAPS);
			if(CallService(szService, (WPARAM)flag, 0) & cap) {
				status = ContactStatus(i.val().handle(), proto);
				if((prio = GetRealPriority(proto, status)) < most_online_prio) {
					most_online_status = status;
					most_online = i.val().handle();
					most_online_proto = proto;
					most_online_prio = prio;
				}
			}
		}
		i.next();
	}

	return most_online;
}

void Meta_CalcStatus(HANDLE hMeta) {
	HANDLE hSub = Meta_GetActive(hMeta);
	char *proto = ContactProto(hSub);
	DBWriteContactSettingWord(hMeta, MODULE, "Status", ContactStatus(hSub, proto));
	FireSubcontactsChanged(hMeta);
}

HANDLE Meta_Convert(HANDLE hSub) {
	HANDLE hMeta = NewMetaContact();

	DBWriteContactSettingByte(hMeta, MODULE, "Default", 0);
	Meta_Assign(hSub, hMeta);

	DBVARIANT dbv;
	if(!DBGetContactSettingUTF8String(hSub, "CList", "Group", &dbv)) {
		DBWriteContactSettingUTF8String(hMeta, "CList", "Group", dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	if(!DBGetContactSettingUTF8String(hSub, "CList", "MyHandle", &dbv)) {
		DBWriteContactSettingUTF8String(hMeta, "CList", "MyHandle", dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	
	char *subProto = ContactProto(hSub);
	if(subProto) {
		if(!DBGetContactSettingUTF8String(hSub, subProto, "Nick", &dbv)) {
			DBWriteContactSettingUTF8String(hMeta, MODULE, "Nick", dbv.pszVal);
			DBFreeVariant(&dbv);
		}
	}

	Meta_CalcStatus(hMeta);
	return hMeta;
}

void Meta_Assign(HANDLE hSub, HANDLE hMeta) {
	metaMap[hMeta].add(hSub);
	DBWriteContactSettingDword(hSub, MODULE, "ParentMetaID", DBGetContactSettingDword(hMeta, MODULE, META_ID, -1));
	DBWriteContactSettingDword(hSub, MODULE, "Handle", (DWORD)hMeta);
	DBWriteContactSettingByte(hSub, MODULE, "IsSubcontact", 1);
	if(MetaEnabled()) {
		if(!meta_group_hack_disabled)
			DBWriteContactSettingByte(hSub, "CList", "Hidden", 1);
	} else // shouldn't happen, as the menu option is hidden when metas are disabled...
		DBWriteContactSettingByte(hMeta, "CList", "Hidden", 1);

	CallService(MS_IGNORE_IGNORE, (WPARAM)hSub, (WPARAM)IGNOREEVENT_USERONLINE);

	DBVARIANT dbv;
	if(DBGetContactSettingUTF8String(hMeta, "CList", "MyHandle", &dbv)) {
		if(!DBGetContactSettingUTF8String(hSub, "CList", "MyHandle", &dbv)) {
			DBWriteContactSettingUTF8String(hMeta, "CList", "MyHandle", dbv.pszVal);
			DBFreeVariant(&dbv);
		}
	} else
		DBFreeVariant(&dbv);

	if(DBGetContactSettingUTF8String(hMeta, MODULE, "Nick", &dbv)) {
		char *subProto = ContactProto(hSub);
		if(subProto) {
			if(!DBGetContactSettingUTF8String(hSub, subProto, "Nick", &dbv)) {
				DBWriteContactSettingUTF8String(hMeta, MODULE, "Nick", dbv.pszVal);
				DBFreeVariant(&dbv);
			}
		}
	} else
		DBFreeVariant(&dbv);

	FireSubcontactsChanged(hMeta);
}

void Meta_Remove(HANDLE hSub) {
	HANDLE hMeta = (HANDLE)DBGetContactSettingDword(hSub, MODULE, "Handle", 0);
	if(hMeta) {
		DBDeleteContactSetting(hSub, MODULE, "ParentMetaID");
		// deleting these (resident) settings doesn't work :( [25/9/07]
		DBWriteContactSettingDword(hSub, MODULE, "Handle", 0);
		DBWriteContactSettingByte(hSub, MODULE, "IsSubcontact", 0);
		if(!meta_group_hack_disabled) DBWriteContactSettingByte(hSub, "CList", "Hidden", 0);

		CallService(MS_IGNORE_UNIGNORE, (WPARAM)hSub, (WPARAM)IGNOREEVENT_USERONLINE);

		metaMap[hMeta].remove(hSub);
		if(metaMap[hMeta].size() == 0) {
			CallService(MS_DB_CONTACT_DELETE, (WPARAM)hMeta, 0);
			metaMap.remove(hMeta);
		} else {
			int def = DBGetContactSettingByte(hMeta, MODULE, "Default", -1);
			if(def < 0 || def >= (int)metaMap[hMeta].size())
				DBWriteContactSettingByte(hMeta, MODULE, "Default", 0);
		}
		FireSubcontactsChanged(hMeta);
	}
}