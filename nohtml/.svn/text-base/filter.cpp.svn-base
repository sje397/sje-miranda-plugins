#include "common.h"
#include "filter.h"
#include "options.h"
#include "conv.h"

#define MIR_VER_FILTERS_FIXED		0x0007001A

void LOG(char *message) {
#ifdef _DEBUG
	PUShowMessage(message, SM_NOTIFY);
#endif
}

void LOGW(wchar_t *message) {
#ifdef _DEBUG
	PUShowMessageW(message, SM_NOTIFY);
#endif
}

bool ContactIsOTREncrypted(HANDLE hContact) {
	return (DBGetContactSettingByte(hContact, "OTR", "Encrypted", 0) != 0);
}

INT_PTR FilterSendMessage(WPARAM wParam, LPARAM lParam) {
	CCSDATA *ccs = (CCSDATA *) lParam;
	char *old_message = (char *)ccs->lParam;
	char *buf = 0;

	if(options.filter_out) {
		if((options.apply_to & ATF_ALL) 
			|| ((options.apply_to & ATF_OTR) && ContactIsOTREncrypted(ccs->hContact)))
		{
			if(ccs->wParam & PREF_UTF) {
				int size = MultiByteToWideChar(CP_UTF8, 0, old_message, -1, 0, 0);
				wchar_t *messagew = (wchar_t *)malloc(size * sizeof(wchar_t));
				MultiByteToWideChar(CP_UTF8, 0, old_message, -1, messagew, size);
				wchar_t* smsg=strip_carrots(messagew);
				wchar_t* html_msg;
				if(options.html) {
					html_msg=bbcodes_to_html(smsg);
					delete[] smsg;
				} else
					html_msg = smsg;

				free(messagew);
				size = WideCharToMultiByte(CP_UTF8, 0, html_msg, -1, 0, 0, 0, 0);
				buf = (char *)malloc(size);
				WideCharToMultiByte(CP_UTF8, 0, html_msg, -1, buf, size, 0, 0);
				delete[] html_msg;
			} else if(ccs->wParam & PREF_UNICODE) {
				wchar_t *messagew = (wchar_t *)&old_message[strlen(old_message)+1];
				wchar_t* smsg=strip_carrots(messagew);
				wchar_t* html_msg;
				if(options.html) {
					html_msg=bbcodes_to_html(smsg);
					delete[] smsg;
				} else
					html_msg = smsg;

				buf=(char *)malloc(wcslen(html_msg)*3+3);
				WideCharToMultiByte( codepage, 0,html_msg, -1,buf,wcslen(html_msg)+1, NULL, NULL);
				memcpy(&buf[strlen(buf)+1],html_msg,lstrlen(buf)*2+2);
				delete[] html_msg;
			} else {
				char* smsg=strip_carrots(old_message);
				char* html_msg;
				if(options.html) {
					html_msg=bbcodes_to_html(smsg);
					delete[] smsg;
				} else
					html_msg = smsg;

				buf = strdup(html_msg);
				delete[] html_msg;
			}
			ccs->lParam = (LPARAM)buf;
		}
	}

	INT_PTR ret = CallService(MS_PROTO_CHAINSEND, wParam, lParam);
	ccs->lParam = (LPARAM)old_message;
	if(buf) free(buf);
	return ret;
}

INT_PTR FilterSendMessageW(WPARAM wParam, LPARAM lParam) {
    CCSDATA *ccs = (CCSDATA *) lParam;
	if(!(ccs->wParam & PREF_UTF))
		ccs->wParam |= PREF_UNICODE;

	return FilterSendMessage(wParam, lParam);
}

INT_PTR FilterRecvMessage(WPARAM wParam, LPARAM lParam) {
	CCSDATA *ccs = (CCSDATA *) lParam;
	PROTORECVEVENT *pre = (PROTORECVEVENT *) ccs->lParam;
	char *old_message = pre->szMessage;
	char* buf = 0;

	if(options.filter_in) {
		if((options.apply_to & ATF_ALL) 
			|| ((options.apply_to & ATF_OTR) && ContactIsOTREncrypted(ccs->hContact)))
		{
			if(pre->flags & PREF_UTF) {
				LOG("Recieved a utf8 message.");
				int size = MultiByteToWideChar(CP_UTF8, 0, old_message, -1, 0, 0);
				wchar_t *messagew = (wchar_t *)malloc(size * sizeof(wchar_t));
				MultiByteToWideChar(CP_UTF8, 0, old_message, -1, messagew, size);

				LOGW(messagew);
				wchar_t* st_wbuf;
				if(options.bbcodes)
				{
					LOG("Converting from html to bbcodes then stripping leftover html.(U)");
					wchar_t* bbuf=html_to_bbcodes(messagew);
					st_wbuf=strip_html(bbuf);
					delete[] bbuf;
				}
				else
				{
					LOG("Stripping html.(U)");
					st_wbuf=strip_html(messagew);
				}
				free(messagew);

				//delete[] pre->szMessage; not necessary - done in server.cpp
				buf=(char *)malloc(wcslen(st_wbuf)*3+3);
				WideCharToMultiByte(CP_UTF8, 0, st_wbuf, -1,buf,wcslen(st_wbuf)+1, NULL, NULL);
				memcpy(&buf[strlen(buf)+1], st_wbuf, lstrlen(buf)*2+2);
				delete[] st_wbuf;
				pre->szMessage = buf;
			} else if(pre->flags & PREF_UNICODE) {
				LOG("Recieved a unicode message.");
				wchar_t *messagew = (wchar_t *)&old_message[strlen(old_message)+1];
				//wchar_t* wbuf=(wchar_t*)&pre->szMessage[lstrlen(pre->szMessage)+1];
				LOGW(messagew);
				wchar_t* st_wbuf;
				if(options.bbcodes)
				{
					LOG("Converting from html to bbcodes then stripping leftover html.(U)");
					wchar_t* bbuf=html_to_bbcodes(messagew);
					st_wbuf=strip_html(bbuf);
					delete[] bbuf;
				}
				else
				{
					LOG("Stripping html.(U)");
					st_wbuf=strip_html(messagew);
				}
				//delete[] pre->szMessage; not necessary - done in server.cpp
				buf=(char *)malloc(wcslen(st_wbuf)*3+3);
				WideCharToMultiByte(codepage, 0, st_wbuf, -1,buf,wcslen(st_wbuf)+1, NULL, NULL);
				memcpy(&buf[strlen(buf)+1],st_wbuf,lstrlen(buf)*2+2);
				delete[] st_wbuf;
				pre->szMessage = buf;
			} else {
				LOG("Recieved a non-unicode message.");
				if(options.bbcodes)
				{
					LOG("Converting from html to bbcodes then stripping leftover html.");
					char* bbuf=html_to_bbcodes(pre->szMessage);
					buf=strip_html(bbuf);
					delete[] bbuf;
				}
				else
				{
					LOG("Stripping html.");
					buf=strip_html(pre->szMessage);
				}
				pre->szMessage = buf;
			}
		}
	}
	
	INT_PTR ret = CallService(MS_PROTO_CHAINRECV, wParam, lParam);
	pre->szMessage = old_message;
	if(buf) free(buf);
	return ret;
}

int NewContact(WPARAM wParam, LPARAM lParam) {
	// add filter
	HANDLE hContact = (HANDLE)wParam;
	CallService( MS_PROTO_ADDTOCONTACT, ( WPARAM )hContact, ( LPARAM )MODULE );
	
	return 0;
}

void RegisterFilter() {
	PROTOCOLDESCRIPTOR pd = {0};
	pd.cbSize = PROTOCOLDESCRIPTOR_V3_SIZE;//sizeof(pd);
	pd.szName = MODULE "Filter";	
	pd.type = PROTOTYPE_FILTER; 

	CallService(MS_PROTO_REGISTERMODULE,0,(LPARAM)&pd);
}

HANDLE hEventNewContact;
#define NUM_FILTER_SERVICES 3
HANDLE hServices[NUM_FILTER_SERVICES];

void AddFilterToContacts() {
	if(DBGetContactSettingByte(0, MODULE, "FilterOrderFix", 0) != 2) {
		// remove filter so it can be added again in the right position
		HANDLE hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
		while ( hContact != NULL ) {
			CallService( MS_PROTO_REMOVEFROMCONTACT, ( WPARAM )hContact, ( LPARAM )(MODULE "Filter"));	
			hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
		}	
		DBWriteContactSettingByte(0, MODULE, "FilterOrderFix", 2);
	}

	// this adds us as a filter to all existing and new contacts
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