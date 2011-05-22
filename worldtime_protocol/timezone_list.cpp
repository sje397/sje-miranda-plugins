#include "common.h"
#include "timezone_list.h"

ITEMLIST listbox_items(5);

void load_listbox_items() {

	HANDLE hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
	LISTITEM pa;
	DBVARIANT dbv;
	char *proto;

	listbox_items.destroy();
	while ( hContact != NULL ) {
		proto = ( char* )CallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM )hContact,0 );
		if ( proto && !strcmp( PROTO, proto)) {
			pa.hContact = hContact;
			if(!DBGetContactSettingTString(pa.hContact, PROTO, "TZName", &dbv)) {
				for (int j = 0; j < timezone_list.getCount(); ++j) {
					if(!_tcscmp(timezone_list[j].tcName, dbv.ptszVal)) {
						pa.timezone_list_index = timezone_list[j].list_index;
						break;
					}
				}
				DBFreeVariant(&dbv);
			} else
				pa.timezone_list_index = DBGetContactSettingDword(pa.hContact, PROTO, "TimezoneListIndex", -1);
			if(!DBGetContactSettingTString(pa.hContact, PROTO, "Nick", &dbv)) {
				_tcsncpy(pa.pszText, dbv.ptszVal, MAX_NAME_LENGTH);
				DBFreeVariant(&dbv);
			}

			listbox_items.insert(new LISTITEM(pa));
		}

		hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
	}	
}

void save_listbox_items() {
	bool is_contact;


	for(int i = 0; i < listbox_items.getCount(); ++i) {
		is_contact = (int)CallService(MS_DB_CONTACT_IS, (WPARAM)listbox_items[i].hContact, 0) == 1;

		if(!is_contact) {
			listbox_items[i].hContact = (HANDLE)CallService(MS_DB_CONTACT_ADD, 0, 0);
			CallService( MS_PROTO_ADDTOCONTACT, ( WPARAM )listbox_items[i].hContact, ( LPARAM )PROTO );	
			CallService(MS_IGNORE_IGNORE, (WPARAM)listbox_items[i].hContact, (WPARAM)IGNOREEVENT_USERONLINE);
		}

		DBWriteContactSettingTString(listbox_items[i].hContact, PROTO, "Nick", listbox_items[i].pszText);
		DBWriteContactSettingDword(listbox_items[i].hContact, PROTO, "TimezoneListIndex", listbox_items[i].timezone_list_index);
		DBWriteContactSettingWord(listbox_items[i].hContact, PROTO, "Status", ID_STATUS_ONLINE);
		DBWriteContactSettingTString(listbox_items[i].hContact, PROTO, "TZName", timezone_list[listbox_items[i].timezone_list_index].tcName);
	}
	DBWriteContactSettingWord(0, PROTO, "DataVersion", 1);

	// remove contacts in DB that have been removed from the list
	HANDLE hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
	char *proto;
	bool found;
	while ( hContact != NULL ) {
		proto = ( char* )CallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM )hContact,0 );
		if ( proto && !strcmp( PROTO, proto)) {
			found = false;
			for(int i = 0; i < listbox_items.getCount(); i++) {
				if(listbox_items[i].hContact == hContact) {
					found = true;
				}
			}

			if(!found) {
				// remove prot first, so that our contact deleted event handler (if present) isn't activated
				HANDLE oldHContact = hContact;
				hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );

				CallService(MS_PROTO_REMOVEFROMCONTACT, (WPARAM)oldHContact, (LPARAM)PROTO);
				CallService(MS_DB_CONTACT_DELETE, (WPARAM)oldHContact, 0);
			} else
				hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
		} else
			hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
	}	
}

void copy_listbox_items(ITEMLIST &dest, ITEMLIST &src)
{
	dest.destroy();
	for (int i=0; i < src.getCount(); ++i)
		dest.insert(new LISTITEM(src[i]));
}


