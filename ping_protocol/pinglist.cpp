#include "stdafx.h"
#include "pinglist.h"

#include "options.h"

PINGLIST list_items;
CRITICAL_SECTION list_cs;
HANDLE reload_event_handle;


BOOL clist_handle_changing = FALSE;

BOOL changing_clist_handle() {
	return clist_handle_changing;
}

void set_changing_clist_handle(BOOL flag) {
	clist_handle_changing = flag;
}

// lParam is address of pointer to a std::list<PINGADDRESS> 
// copies data into this structure
int GetPingList(WPARAM wParam,LPARAM lParam)
{
	PINGLIST *pa = (PINGLIST *)lParam;

	EnterCriticalSection(&list_cs);
	*pa = list_items;
	LeaveCriticalSection(&list_cs);
	
	return 0;
}

void reset_myhandle() {
	EnterCriticalSection(&list_cs);
	set_changing_clist_handle(true);
	for(PINGLIST::iterator i = list_items.begin(); i != list_items.end(); i++) {
		DBWriteContactSettingString(i->hContact, "CList", "MyHandle", i->pszLabel);
	}
	set_changing_clist_handle(false);
	LeaveCriticalSection(&list_cs);
}


void write_ping_address(PINGADDRESS *i) {
	bool is_contact = (bool)(CallService(MS_DB_CONTACT_IS, (WPARAM)i->hContact, 0) != 0);

	if(!is_contact) {
		//MessageBox(0, "Creating contact", "Ping", MB_OK);
		i->hContact = (HANDLE)CallService(MS_DB_CONTACT_ADD, 0, 0);
		CallService( MS_PROTO_ADDTOCONTACT, ( WPARAM )i->hContact, ( LPARAM )PROTO );	
		CallService(MS_IGNORE_IGNORE, (WPARAM)i->hContact, (WPARAM)IGNOREEVENT_USERONLINE);
	}
	DBWriteContactSettingString(i->hContact, PROTO, "Address", i->pszName);
	set_changing_clist_handle(TRUE);
	DBWriteContactSettingString(i->hContact, "CList", "MyHandle", i->pszLabel);
	set_changing_clist_handle(FALSE);
	DBWriteContactSettingString(i->hContact, PROTO, "Nick", i->pszLabel);
	DBWriteContactSettingWord(i->hContact, PROTO, "Status", i->status);
	DBWriteContactSettingDword(i->hContact, PROTO, "Port", i->port);
	DBWriteContactSettingString(i->hContact, PROTO, "Proto", i->pszProto);
	if(strlen(i->pszCommand))
		DBWriteContactSettingString(i->hContact, PROTO, "Command", i->pszCommand);
	else
		DBDeleteContactSetting(i->hContact, PROTO, "Command");
	if(strlen(i->pszParams))
		DBWriteContactSettingString(i->hContact, PROTO, "CommandParams", i->pszParams);
	else
		DBDeleteContactSetting(i->hContact, PROTO, "CommandParams");
	DBWriteContactSettingWord(i->hContact, PROTO, "SetStatus", i->set_status);
	DBWriteContactSettingWord(i->hContact, PROTO, "GetStatus", i->get_status);
	DBWriteContactSettingWord(i->hContact, PROTO, "Index", i->index);
	if(strlen(i->pszCListGroup))
		DBWriteContactSettingString(i->hContact, "CList", "Group", i->pszCListGroup);
	else
		DBDeleteContactSetting(i->hContact, "CList", "Group");
}

// call with list_cs locked
void write_ping_addresses() {
	int index = 0;
	for(PINGLIST::iterator i = list_items.begin(); i != list_items.end(); i++, index++) {
		i->index = index;
		write_ping_address(&(*i));
	}
}

void read_ping_address(PINGADDRESS &pa) {
	DBVARIANT dbv;
	DBGetContactSetting(pa.hContact, PROTO, "Address", &dbv);
	strncpy(pa.pszName, dbv.pszVal, MAX_PINGADDRESS_STRING_LENGTH);
	DBFreeVariant(&dbv);
	if(!DBGetContactSetting(pa.hContact, PROTO, "Nick", &dbv)) {
		strncpy(pa.pszLabel, dbv.pszVal, MAX_PINGADDRESS_STRING_LENGTH);
		DBFreeVariant(&dbv);
		set_changing_clist_handle(TRUE);
		DBWriteContactSettingString(pa.hContact, "CList", "MyHandle", pa.pszLabel);
		set_changing_clist_handle(FALSE);
	} else {
		if(!DBGetContactSetting(pa.hContact, PROTO, "Label", &dbv)) {
			strncpy(pa.pszLabel, dbv.pszVal, MAX_PINGADDRESS_STRING_LENGTH);
			DBFreeVariant(&dbv);
			set_changing_clist_handle(TRUE);
			DBWriteContactSettingString(pa.hContact, "CList", "MyHandle", pa.pszLabel);
			set_changing_clist_handle(FALSE);
			DBWriteContactSettingString(pa.hContact, PROTO, "Nick", pa.pszLabel);
			DBDeleteContactSetting(pa.hContact, PROTO, "Label");
		} else {
			if(!DBGetContactSetting(pa.hContact, "CList", "MyHandle", &dbv)) {
				strncpy(pa.pszLabel, dbv.pszVal, MAX_PINGADDRESS_STRING_LENGTH);
				DBFreeVariant(&dbv);
			} else
				pa.pszLabel[0] = '\0';
		}
	}

	pa.status = DBGetContactSettingWord(pa.hContact, PROTO, "Status", options.nrstatus);
	pa.port = (int)DBGetContactSettingDword(pa.hContact, PROTO, "Port", (DWORD)-1);

	if(!DBGetContactSetting(pa.hContact, PROTO, "Proto", &dbv)) {
		strncpy(pa.pszProto, dbv.pszVal, MAX_PINGADDRESS_STRING_LENGTH);
		DBFreeVariant(&dbv);
	} else pa.pszProto[0] = '\0';

	if(!DBGetContactSetting(pa.hContact, PROTO, "Command", &dbv)) {
		strncpy(pa.pszCommand, dbv.pszVal, MAX_PATH);
		DBFreeVariant(&dbv);
	} else
		pa.pszCommand[0] = '\0';
	if(!DBGetContactSetting(pa.hContact, PROTO, "CommandParams", &dbv)) {
		strncpy(pa.pszParams, dbv.pszVal, MAX_PATH);
		DBFreeVariant(&dbv);
	} else
		pa.pszParams[0] = '\0';

	pa.set_status = DBGetContactSettingWord(pa.hContact, PROTO, "SetStatus", ID_STATUS_ONLINE);
	pa.get_status = DBGetContactSettingWord(pa.hContact, PROTO, "GetStatus", ID_STATUS_OFFLINE);

	pa.responding = false;
	pa.round_trip_time = 0;
	pa.miss_count = 0;
	pa.index = DBGetContactSettingWord(pa.hContact, PROTO, "Index", 0);

	if(!DBGetContactSetting(pa.hContact, "CList", "Group", &dbv)) {
		strncpy(pa.pszCListGroup, dbv.pszVal, MAX_PINGADDRESS_STRING_LENGTH);
		DBFreeVariant(&dbv);
	} else
		pa.pszCListGroup[0] = '\0';
}

// call with list_cs locked
void read_ping_addresses() {
	HANDLE hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
	PINGADDRESS pa;

	list_items.clear();
	while ( hContact != NULL ) {
		char *proto = ( char* )CallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM )hContact,0 );
		if ( proto && lstrcmp( PROTO, proto) == 0) {
			pa.hContact = hContact;
			read_ping_address(pa);
			list_items.push_back(pa);
		}

		hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
	}	
	std::sort(list_items.begin(), list_items.end(), SAscendingSort());
}

int LoadPingList(WPARAM wParam, LPARAM lParam) {
	EnterCriticalSection(&list_cs);
	read_ping_addresses();
	LeaveCriticalSection(&list_cs);
	NotifyEventHooks(reload_event_handle, 0, 0);	
	return 0;
}

// wParam is zero
// lParam is zero
int SavePingList(WPARAM wParam, LPARAM lParam) {
	EnterCriticalSection(&list_cs);
	write_ping_addresses();
	LeaveCriticalSection(&list_cs);
	NotifyEventHooks(reload_event_handle, 0, 0);
	
	return 0;
}

// wParam is address of a PINGLIST structure to replace the current one
// lParam is zero
int SetPingList(WPARAM wParam, LPARAM lParam) {
	PINGLIST *pli = (PINGLIST *)wParam;
	
	EnterCriticalSection(&list_cs);
	list_items = *pli;
	LeaveCriticalSection(&list_cs);
	NotifyEventHooks(reload_event_handle, 0, 0);
	
	return 0;
}

// wParam is address of a PINGLIST structure to replace the current one
// lParam is zero
int SetAndSavePingList(WPARAM wParam, LPARAM lParam) {
	PINGLIST *pli = (PINGLIST *)wParam;
	
	EnterCriticalSection(&list_cs);
	// delete items that aren't in the new list
	bool found;
	for(PINGLIST::iterator i = list_items.begin(); i != list_items.end(); i++) {
		found = false;
		for(PINGLIST::iterator j = pli->begin(); j != pli->end(); j++) {
			if(i->hContact == j->hContact) {
				found = true;
				break;
			}

		}
		if(!found) {
			//MessageBox(0, "Deleting contact", "Ping", MB_OK);
			// remove prot first, so that our contact deleted event handler isn't activated
			CallService(MS_PROTO_REMOVEFROMCONTACT, (WPARAM)i->hContact, (LPARAM)PROTO);
			CallService(MS_DB_CONTACT_DELETE, (WPARAM)i->hContact, 0);
		}
	}

	// set new list
	if(pli->size())
		list_items = *pli;
	else 
		list_items.clear();

	write_ping_addresses();
	LeaveCriticalSection(&list_cs);
	
	NotifyEventHooks(reload_event_handle, 0, 0);
	
	return 0;
}

int ClearPingList(WPARAM wParam, LPARAM lParam) {
	EnterCriticalSection(&list_cs);
	list_items.clear();
	LeaveCriticalSection(&list_cs);
	
	NotifyEventHooks(reload_event_handle, 0, 0);
	return 0;
}

