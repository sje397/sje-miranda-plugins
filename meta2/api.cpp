#include "common.h"
#include "api.h"
#include "proto.h"
#include "core_functions.h"

HANDLE hEventDefaultChanged = 0, hEventForceSend = 0, hEventUnforceSend = 0, hSubcontactsChanged = 0;

void FireSubcontactsChanged(HANDLE hMeta) {
	NotifyEventHooks(hSubcontactsChanged, (WPARAM)hMeta, 0);
}

//get the handle for a contact's parent metacontact
//wParam=(HANDLE)hSubContact
//lParam=0
//returns a handle to the parent metacontact, or null if this contact is not a subcontact
INT_PTR MetaAPI_GetMeta(WPARAM wParam, LPARAM lParam) {
	return (INT_PTR)(HANDLE)DBGetContactSettingDword((HANDLE)wParam, MODULE, "Handle", 0);
}

//gets the handle for the default contact
//wParam=(HANDLE)hMetaContact
//lParam=0
//returns a handle to the default contact, or null on failure
INT_PTR MetaAPI_GetDefault(WPARAM wParam, LPARAM lParam) {
	HANDLE hMeta = (HANDLE)wParam;
	if(metaMap.exists(hMeta) == false || metaMap[hMeta].size() == 0) return 0;
	
	int def = DBGetContactSettingByte(hMeta, MODULE, "Default", -1);
	if(def < 0 || def >= (int)metaMap[hMeta].size()) return 0;

	return (INT_PTR)metaMap[hMeta][def].handle();
}

//gets the contact number for the default contact
//wParam=(HANDLE)hMetaContact
//lParam=0
//returns an int contact number, or -1 on failure
INT_PTR MetaAPI_GetDefaultNum(WPARAM wParam, LPARAM lParam) {
	HANDLE hMeta = (HANDLE)wParam;
	if(metaMap.exists(hMeta) == false || metaMap[hMeta].size() == 0) return -1;

	INT_PTR def = DBGetContactSettingByte(hMeta, MODULE, "Default", -1);
	if(def < 0 || def >= metaMap[hMeta].size()) return -1;
	return def;
}

//gets the handle for the 'most online' contact
//wParam=(HANDLE)hMetaContact
//lParam=0
//returns a handle to the 'most online' contact
INT_PTR MetaAPI_GetMostOnline(WPARAM wParam, LPARAM lParam) {
	return (INT_PTR)Meta_GetMostOnline((HANDLE)wParam);
}

//gets the handle for the 'active' messaging contact (the one new messages will be sent to,
//taking into account temporary conversation settings and 'force state' below)
//wParam=(HANDLE)hMetaContact
//lParam=0
//returns a handle to the 'most online' contact
//added with meta2 (2007/10/4)
INT_PTR MetaAPI_GetActiveContact(WPARAM wParam, LPARAM lParam) {
	return (INT_PTR)Meta_GetActive((HANDLE)wParam);
}

//gets the number of subcontacts for a metacontact
//wParam=(HANDLE)hMetaContact
//lParam=0
//returns an int representing the number of subcontacts for the given metacontact, or -1 on failure
INT_PTR MetaAPI_GetNumContacts(WPARAM wParam, LPARAM lParam) {
	HANDLE hMeta = (HANDLE)wParam;
	if(metaMap.exists(hMeta)) return metaMap[hMeta].size();
	return -1;
}

//gets the handle of a subcontact, using the subcontact's number
//wParam=(HANDLE)hMetaContact
//lParam=(int)contact number
//returns a handle to the specified subcontact
INT_PTR MetaAPI_GetContact(WPARAM wParam, LPARAM lParam) {
	HANDLE hMeta = (HANDLE)wParam;
	if(metaMap.exists(hMeta) && lParam >= 0 && metaMap[hMeta].size() > lParam) return (INT_PTR)metaMap[hMeta][lParam].handle();
	return 0;
}

//sets the default contact, using the subcontact's contact number
//wParam=(HANDLE)hMetaContact
//lParam=(int)contact number
//returns 0 on success
INT_PTR MetaAPI_SetDefaultContactNum(WPARAM wParam, LPARAM lParam) {
	HANDLE hMeta = (HANDLE)wParam;
	DWORD num_contacts = metaMap.exists(hMeta) ? metaMap[hMeta].size() : -1;
	if(num_contacts < 0)
		return 1;
	if((DWORD)lParam >= num_contacts || (DWORD)lParam < 0)
		return 1;
	if(DBWriteContactSettingByte((HANDLE)wParam, MODULE, "Default", (DWORD)lParam))
		return 1;

	DBWriteContactSettingDword(hMeta, MODULE, "TempDefault", 0);

	NotifyEventHooks(hEventDefaultChanged, wParam, (LPARAM)metaMap[hMeta][(int)lParam].handle());
	return 0;
}

//sets the default contact, using the subcontact's handle
//wParam=(HANDLE)hMetaContact
//lParam=(HANDLE)hSubcontact
//returns 0 on success
INT_PTR MetaAPI_SetDefaultContact(WPARAM wParam, LPARAM lParam) {
	HANDLE hMeta = (HANDLE)wParam, hSub = (HANDLE)lParam;
	HANDLE hMetaFromSub = (HANDLE)DBGetContactSettingDword(hSub, MODULE, "Handle", 0);
	if(hMetaFromSub != hMeta || metaMap.exists(hMeta) == false) 
		return 1;

	int contact_num = metaMap[hMeta].index_of(hSub);
	if(contact_num == -1) return 1;

	if(DBWriteContactSettingByte(hMeta, MODULE, "Default", contact_num))
		return 1;

	DBWriteContactSettingDword(hMeta, MODULE, "TempDefault", 0);
	
	NotifyEventHooks(hEventDefaultChanged, wParam, lParam);
	return 0;
}

//temporarily forces the metacontact to send using a specific subcontact, using the subcontact's contact number
//wParam=(HANDLE)hMetaContact
//lParam=(int)contact number
//returns 0 on success
INT_PTR MetaAPI_ForceSendContactNum(WPARAM wParam, LPARAM lParam) {
	HANDLE hMeta = (HANDLE)wParam;
	unsigned contact_num = (unsigned)lParam;
	if(!hMeta || (int)metaMap.exists(hMeta) == false || contact_num < 0 || contact_num >= metaMap[hMeta].size() || DBGetContactSettingByte(hMeta, MODULE, "ForceDefault", 0)) 
		return 1;
	
	HANDLE hSub = metaMap[hMeta][contact_num].handle();
	HANDLE hMetaFromSub = (HANDLE)DBGetContactSettingDword(hSub, MODULE, "Handle", 0);
	if(hMetaFromSub != hMeta) 
		return 1;

	DBWriteContactSettingDword(hMeta, MODULE, "ForceSend", (DWORD)hSub);

	NotifyEventHooks(hEventForceSend, wParam, (LPARAM)hSub);
	return 0;
}

//temporarily forces the metacontact to send using a specific subcontact, using the subcontact's handle
//wParam=(HANDLE)hMetaContact
//lParam=(HANDLE)hSubcontact
//returns 0 on success (will fail if 'force default' is in effect)
INT_PTR MetaAPI_ForceSendContact(WPARAM wParam, LPARAM lParam) {
	HANDLE hMeta = (HANDLE)wParam, hSub = (HANDLE)lParam;
	HANDLE hMetaFromSub = (HANDLE)DBGetContactSettingDword(hSub, MODULE, "Handle", 0);
	if(!hSub || !hMeta || hMetaFromSub != hMeta || !metaMap.exists(hMeta) || metaMap[hMeta].index_of(hSub) == -1 || DBGetContactSettingByte(hMeta, MODULE, "ForceDefault", 0)) 
		return 1;

	DBWriteContactSettingDword(hMeta, MODULE, "ForceSend", (DWORD)hSub);

	NotifyEventHooks(hEventForceSend, wParam, lParam);
	return 0;
}

//'unforces' the metacontact to send using a specific subcontact
//wParam=(HANDLE)hMetaContact
//lParam=0
//returns 0 on success (will fail if 'force default' is in effect)
INT_PTR MetaAPI_UnforceSendContact(WPARAM wParam, LPARAM lParam) {
	if(DBGetContactSettingByte((HANDLE)wParam, MODULE, "ForceDefault", 0) == 0)
		return 1;

	DBWriteContactSettingDword((HANDLE)wParam, MODULE, "ForceSend", 0);
		
	NotifyEventHooks(hEventUnforceSend, wParam, lParam);
	return 0;
}


//'forces' or 'unforces' (i.e. toggles) the metacontact to send using it's default contact
// overrides 'force send' above, and will even force use of offline contacts
// will send ME_MC_FORCESEND event
//wParam=(HANDLE)hMetaContact
//lParam=0
//returns 1(true) or 0(false) representing new state of 'force default'
INT_PTR MetaAPI_ForceDefault(WPARAM wParam, LPARAM lParam) {
	int old_state = DBGetContactSettingByte((HANDLE)wParam, MODULE, "ForceDefault", 0);
	DBWriteContactSettingByte((HANDLE)wParam, MODULE, "ForceDefault", old_state ? 0 : 1);
	return old_state ? 0 : 1;
}
	
// method to get state of 'force' for a metacontact
// wParam=(HANDLE)hMetaContact
// lParam= (int)&contact_number or NULL
// if lparam supplied, the contact_number of the contatct 'in force' will be copied to the address it points to,
// or if none is in force, the value (DWORD)-1 will be copied
// (v0.8.0.8+ returns 1 if 'force default' is true with *lParam == default contact number, else returns 0 with *lParam as above)
INT_PTR MetaAPI_GetForceState(WPARAM wParam, LPARAM lParam) {
	HANDLE hMeta = (HANDLE)wParam;
	HANDLE hContact;
	
	if(!hMeta || !metaMap.exists(hMeta)) return 0;
	
	if(DBGetContactSettingByte(hMeta, MODULE, "ForceDefault", 0)) {
		if(lParam) *(DWORD *)lParam = DBGetContactSettingByte((HANDLE)wParam, MODULE, "Default", -1);
		return 1;
	}

	hContact = (HANDLE)DBGetContactSettingDword(hMeta, MODULE, "ForceSend", 0);

	if(!hContact) {
		if(lParam) *(DWORD *)lParam = -1;
	} else {
		if(lParam) *(DWORD *)lParam = (DWORD)metaMap[hMeta].index_of(hContact);
	}

	return 0;
}

// method to get protocol name - used to be sure you're dealing with a "real" metacontacts plugin :)
// wParam=lParam=0
INT_PTR MetaAPI_GetProtoName(WPARAM wParam, LPARAM lParam) {
	return (INT_PTR)MODULE;
}

// added 0.9.5.0 (22/3/05)
// wParam=(HANDLE)hContact
// lParam=0
// convert a given contact into a metacontact
INT_PTR MetaAPI_ConvertToMeta(WPARAM wParam, LPARAM lParam) {
	return (INT_PTR)Meta_Convert((HANDLE)wParam);
}

// added 0.9.5.0 (22/3/05)
// wParam=(HANDLE)hContact
// lParam=(HANDLE)hMeta
// add an existing contact to a metacontact
INT_PTR MetaAPI_AddToMeta(WPARAM wParam, LPARAM lParam) {
	Meta_Assign((HANDLE)wParam, (HANDLE)lParam);
	return 0;
}

// added 0.9.5.0 (22/3/05)
// wParam=0
// lParam=(HANDLE)hContact
// remove a contact from a metacontact
INT_PTR MetaAPI_RemoveFromMeta(WPARAM wParam, LPARAM lParam) {
	Meta_Remove((HANDLE)lParam);
	return 0;
}

// added 0.9.13.2 (6/10/05)
// wParam=(BOOL)disable
// lParam=0
// enable/disable the 'hidden group hack' - for clists that support subcontact hiding using 'IsSubcontact' setting
// should be called once in your 'onmodulesloaded' event handler
bool meta_group_hack_disabled = false;	
INT_PTR MetaAPI_DisableHiddenGroup(WPARAM wParam, LPARAM lParam) {
	meta_group_hack_disabled = wParam != 0;
	return 0;
}

int ModulesLoadedAPI(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)CallService( MS_DB_CONTACT_FINDFIRST, 0, 0);
	HANDLE hMeta;
	while(hContact != NULL) {
		DWORD id = DBGetContactSettingDword(hContact, MODULE, "ParentMetaID", (DWORD)-1);
		if(id != (DWORD)-1) {
			hMeta = GetMetaHandle(id);
			if(hMeta) {
				if(id >= next_meta_id) next_meta_id = id + 1;
				metaMap[hMeta].add(hContact);
				DBWriteContactSettingDword(hContact, MODULE, "Handle", (DWORD)hMeta);
				DBWriteContactSettingByte(hContact, MODULE, "IsSubcontact", 1);
			} else
				DBDeleteContactSetting(hContact, MODULE, "ParentMetaID");
		}

		hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
	}	

	hContact = (HANDLE)CallService( MS_DB_CONTACT_FINDFIRST, 0, 0);
	while(hContact != NULL) {
		if(IsMetacontact(hContact))
			Meta_CalcStatus(hContact);
		hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
	}	

	return 0;
}

#define NUM_API_SERVICES 18
HANDLE hServicesAPI[NUM_API_SERVICES] = {0};
HANDLE hEventModulesLoadedAPI = 0;
void InitAPI() {
	int i = 0;
	hServicesAPI[i++] = CreateServiceFunction(MS_MC_GETMETACONTACT, MetaAPI_GetMeta);
	hServicesAPI[i++] = CreateServiceFunction(MS_MC_GETDEFAULTCONTACT, MetaAPI_GetDefault);
	hServicesAPI[i++] = CreateServiceFunction(MS_MC_GETDEFAULTCONTACTNUM, MetaAPI_GetDefaultNum);
	hServicesAPI[i++] = CreateServiceFunction(MS_MC_GETMOSTONLINECONTACT, MetaAPI_GetMostOnline);
	hServicesAPI[i++] = CreateServiceFunction(MS_MC_GETACTIVECONTACT, MetaAPI_GetActiveContact);
	hServicesAPI[i++] = CreateServiceFunction(MS_MC_GETNUMCONTACTS, MetaAPI_GetNumContacts);
	hServicesAPI[i++] = CreateServiceFunction(MS_MC_GETSUBCONTACT, MetaAPI_GetContact);
	hServicesAPI[i++] = CreateServiceFunction(MS_MC_SETDEFAULTCONTACTNUM, MetaAPI_SetDefaultContactNum);
	hServicesAPI[i++] = CreateServiceFunction(MS_MC_SETDEFAULTCONTACT, MetaAPI_SetDefaultContact);
	hServicesAPI[i++] = CreateServiceFunction(MS_MC_FORCESENDCONTACTNUM, MetaAPI_ForceSendContactNum);
	hServicesAPI[i++] = CreateServiceFunction(MS_MC_FORCESENDCONTACT, MetaAPI_ForceSendContact);
	hServicesAPI[i++] = CreateServiceFunction(MS_MC_UNFORCESENDCONTACT, MetaAPI_UnforceSendContact);
	hServicesAPI[i++] = CreateServiceFunction(MS_MC_GETPROTOCOLNAME, MetaAPI_GetProtoName);
	hServicesAPI[i++] = CreateServiceFunction(MS_MC_GETFORCESTATE, MetaAPI_GetForceState);

	hServicesAPI[i++] = CreateServiceFunction(MS_MC_CONVERTTOMETA, MetaAPI_ConvertToMeta);
	hServicesAPI[i++] = CreateServiceFunction(MS_MC_ADDTOMETA, MetaAPI_AddToMeta);
	hServicesAPI[i++] = CreateServiceFunction(MS_MC_REMOVEFROMMETA, MetaAPI_RemoveFromMeta);

	hServicesAPI[i++] = CreateServiceFunction(MS_MC_DISABLEHIDDENGROUP, MetaAPI_DisableHiddenGroup);

	hEventDefaultChanged = CreateHookableEvent(ME_MC_DEFAULTTCHANGED);
	hEventForceSend = CreateHookableEvent(ME_MC_FORCESEND);
	hEventUnforceSend = CreateHookableEvent(ME_MC_UNFORCESEND);
	hSubcontactsChanged = CreateHookableEvent(ME_MC_SUBCONTACTSCHANGED);

	hEventModulesLoadedAPI = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoadedAPI);
	//ModulesLoadedAPI(0, 0);
}

void DeinitAPI() {
	if(hEventModulesLoadedAPI) UnhookEvent(hEventModulesLoadedAPI);
	DestroyHookableEvent(hSubcontactsChanged);
	DestroyHookableEvent(hEventUnforceSend);
	DestroyHookableEvent(hEventForceSend);
	DestroyHookableEvent(hEventDefaultChanged);

	for(int i = 0; i < NUM_API_SERVICES; i++)
		DestroyServiceFunction(hServicesAPI[i]);
}