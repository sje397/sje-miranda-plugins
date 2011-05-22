#include "common.h"
#include "filter.h"

int FilterSendMessage(WPARAM wParam, LPARAM lParam) {
    CCSDATA *ccs = (CCSDATA *) lParam;
    char *message = (char *)ccs->lParam;

	// TODO: process 'message' and/or 'messagew' below
	if(ccs->wParam & PREF_UNICODE) {
		wchar_t *messagew = (wchar_t *)&message[strlen(message)+1];
	} else if(ccs->wParam & PREF_UTF) {
		// message is utf8 encoded - you can use mir_utf8decode (m_system.h) to make it wide
	} else {
	}

	return CallService(MS_PROTO_CHAINSEND, wParam, lParam);
}

int FilterSendMessageW(WPARAM wParam, LPARAM lParam) {
    CCSDATA *ccs = (CCSDATA *) lParam;
	ccs->wParam |= PREF_UNICODE;

	return FilterSendMessage(wParam, lParam);
}

int FilterRecvMessage(WPARAM wParam, LPARAM lParam) {
    CCSDATA *ccs = (CCSDATA *) lParam;
    PROTORECVEVENT *pre = (PROTORECVEVENT *) ccs->lParam;

	char *message = pre->szMessage;

	// TODO: process 'message' and/or 'messagew' below
	if(pre->flags & PREF_UNICODE) {
		wchar_t *messagew = (wchar_t *)&message[strlen(message)+1];
	} else if(pre->flags & PREF_UTF) {
		// message is utf8 encoded - you can use mir_utf8decode (m_system.h) to make it wide
	}
	
	return CallService(MS_PROTO_CHAINRECV, wParam, lParam);
}

int NewContact(WPARAM wParam, LPARAM lParam) {
	// add filter
	HANDLE hContact = (HANDLE)wParam;
	CallService( MS_PROTO_ADDTOCONTACT, ( WPARAM )hContact, ( LPARAM )MODULE );
	
	return 0;
}

void RegisterFilter() {
	PROTOCOLDESCRIPTOR pd = {0};
	pd.cbSize = sizeof(pd);
	pd.szName = MODULE "Filter";
	// TODO: modify this to reflect the purpose of your plugin
	pd.type = PROTOTYPE_FILTER; 
	CallService(MS_PROTO_REGISTERMODULE,0,(LPARAM)&pd);
}

HANDLE hEventNewContact;
#define NUM_FILTER_SERVICES 3
HANDLE hServices[NUM_FILTER_SERVICES];

void AddFilterToContacts() {
	// this adds us as a filter to all existing and new contacts
	// TODO: modify this (and the NewContact function above) to include only the contacts required
	HANDLE hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
	while ( hContact != NULL ) {
		CallService( MS_PROTO_ADDTOCONTACT, ( WPARAM )hContact, ( LPARAM )(MODULE "Filter"));	
		hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
	}	

	hEventNewContact = HookEvent(ME_DB_CONTACT_ADDED, NewContact);
}

void CreateFilterServices() {
	// create our services
	int i = 0;
	hServices[i++] = CreateProtoServiceFunction(MODULE "Filter", PSS_MESSAGE, FilterSendMessage);
	hServices[i++] = CreateProtoServiceFunction(MODULE "Filter", PSS_MESSAGE"W", FilterSendMessageW);
	hServices[i++] = CreateProtoServiceFunction(MODULE "Filter", PSR_MESSAGE, FilterRecvMessage);

	// remember to modify the NUM_FILTER_SERVICES #define above if you add more services!
}

void DeinitFilter() {
	UnhookEvent(hEventNewContact);

	for(int i = 0; i < NUM_FILTER_SERVICES; i++)
		DestroyServiceFunction(hServices[i]);
}