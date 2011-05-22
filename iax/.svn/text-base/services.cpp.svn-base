#include "common.h"
#include "services.h"
#include "resource.h"
#include "iax_interface.h"

#define NUM_SERVICES 6
HANDLE hServices[NUM_SERVICES];

int current_status = ID_STATUS_OFFLINE;

HANDLE FindContact(char *num) {
	char *proto;
	DBVARIANT dbv;
	HANDLE hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 ), ret = 0;
	while ( hContact != NULL )
	{
		proto = ( char* )CallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM )hContact,0 );
		if ( proto && !strcmp( MODULE, proto)) {
			if(!DBGetContactSetting(hContact, MODULE, "Number", &dbv)) {
				if(strcmp(num, dbv.pszVal) == 0) {
					DBFreeVariant(&dbv);
					return hContact;
				}
				DBFreeVariant(&dbv);
			}
		}
		hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
	}

	return 0;
}

int ClistDblClick(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;
	char *proto = ( char* )CallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM )hContact,0 );
	if ( proto && !strcmp( MODULE, proto)) {
		// if contact has active line
		BYTE line_no = DBGetContactSettingByte(hContact, MODULE, "LineNo", 0xff);
		if(line_no != 0xff) {

			// if ringing
			if(GetLineState(line_no) & IAXC_CALL_STATE_RINGING) {
				SelectLine(line_no); // calling this could change result of GetLineState call above
				// if outgoing, hang up
				if(GetLineState(line_no) & IAXC_CALL_STATE_OUTGOING) {
					HangUp();
				} else {
				// if incoming, answer
					AnswerCall(line_no);
				}
			} else {
				SelectLine(line_no);
			// else
				// hang up
				HangUp();
			}
		} else {
		// else
			// call number
			DBVARIANT dbv;
			if(!DBGetContactSettingTString(hContact, MODULE, "Number", &dbv)) {
				Call(dbv.pszVal);
				DBFreeVariant(&dbv);
			}
		}
	}

	return 0;
}

HANDLE CreateContact(char *num, char *name, bool temp) {
	HANDLE hContact = FindContact(num);
	if(!hContact) {
		hContact = (HANDLE)CallService(MS_DB_CONTACT_ADD, 0, 0);
		DBWriteContactSettingTString(hContact, MODULE, "Number", num);
		CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)MODULE);
		DBWriteContactSettingTString(hContact, MODULE, "Nick", name);
		if(temp) DBWriteContactSettingByte(hContact, "CList", "NotOnList", 1);
		DBWriteContactSettingWord(hContact, MODULE, "Status", ID_STATUS_ONLINE);
		CallService(MS_IGNORE_IGNORE, (WPARAM)hContact, (WPARAM)IGNOREEVENT_USERONLINE);
	} else {
		if(name && name[0]) DBWriteContactSettingTString(hContact, MODULE, "Nick", name);
		if(DBGetContactSettingByte(hContact, "CList", "Hidden", 0))
			DBWriteContactSettingByte(hContact, "CList", "Hidden", 0);
	}
	return hContact;
}

void SetContactStatus(int status) {
	char *proto;
	HANDLE hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
	while ( hContact != NULL )
	{
		proto = ( char* )CallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM )hContact,0 );
		if ( proto && !strcmp( MODULE, proto)) {
			if(!DBGetContactSettingByte(hContact, MODULE, "ChatRoom", 0)) {
				DBWriteContactSettingWord( hContact, MODULE, "Status", status);
				if(status == ID_STATUS_OFFLINE) {
					DBWriteContactSettingDword(hContact, MODULE, "IdleTS", 0);
					DBDeleteContactSetting(hContact, MODULE, "LineNo");
				}
			}
		}
		hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
	}	
}


// protocol related services
int GetCaps(WPARAM wParam,LPARAM lParam)
{
	int ret = 0;
    switch (wParam) {
        case PFLAGNUM_1:
			ret = 0;
			break;
        case PFLAGNUM_2:
			ret = PF2_ONLINE | PF2_FREECHAT | PF2_LONGAWAY | PF2_ONTHEPHONE;
			break;
        case PFLAGNUM_3:
            break;
        case PFLAGNUM_4:
			ret = 0;
            break;
        case PFLAGNUM_5:
			// states contacts can have but we can't
			ret = 
				PF2_FREECHAT  // incomming call
				| PF2_LONGAWAY  // outgoing call
				| PF2_ONTHEPHONE; // active call;
            break;
        case PFLAG_UNIQUEIDTEXT:
			ret = (int) Translate("Ph#");
            break;
        case PFLAG_MAXLENOFMESSAGE:
            ret = 0;
            break;
        case PFLAG_UNIQUEIDSETTING:
            ret = (int) "Number";
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
	char *name = (char *)Translate(MODULE);
	if(strncpy((char *)lParam,name,wParam)==NULL)
		return 1;
	((char *)lParam)[wParam-1]='\0';
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
			id = IDI_PROTO;
			break;
		case PLI_ONLINE:
			id = IDI_PROTO;
			break;
		case PLI_OFFLINE:
			id = IDI_PROTO;
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

void SetStatusAndACK(int new_status) {
	int prev_status = current_status;
	current_status = new_status;

	ProtoBroadcastAck(MODULE,NULL,ACKTYPE_STATUS,ACKRESULT_SUCCESS, (HANDLE)prev_status, current_status);
}

int SetStatusService(WPARAM wParam,LPARAM lParam)
{
	int new_status = (int)wParam;
	// we only support online and offline
	if(new_status != ID_STATUS_OFFLINE) new_status = ID_STATUS_ONLINE;

	if(new_status == current_status) return 0;

	if(current_status == ID_STATUS_OFFLINE && new_status != ID_STATUS_OFFLINE) {
		Connect();
	} else {
		if(new_status == ID_STATUS_OFFLINE && current_status != ID_STATUS_OFFLINE) {
			Disconnect();
			//SetContactStatus(ID_STATUS_OFFLINE);
		}
	
		SetStatusAndACK(new_status);
	} 

	return 0;
}

void SetStatus(int new_status) {
	SetStatusService((WPARAM)new_status, 0);
}

/** Returns the current status
*/
int GetStatus(WPARAM wParam,LPARAM lParam)
{
	return current_status;
}

typedef struct tag_TFakeAckParams
{
	HANDLE	hEvent;
	HANDLE	hContact;
	LPARAM lParam;
} TFakeAckParams;

static DWORD CALLBACK sttFakeAckInfoSuccess( LPVOID param )
{
	TFakeAckParams *tParam = ( TFakeAckParams* )param;
	WaitForSingleObject( tParam->hEvent, INFINITE );

	Sleep( 100 );
	ProtoBroadcastAck(MODULE, tParam->hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, ( HANDLE )1, 0 );

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


HANDLE hEventClistDblClick;

void InitServices() {
	int i = 0;
	// protocol
	hServices[i++] = CreateProtoServiceFunction(MODULE, PS_GETCAPS, (MIRANDASERVICE)GetCaps);
	hServices[i++] = CreateProtoServiceFunction(MODULE, PS_GETNAME, (MIRANDASERVICE)GetName);
	hServices[i++] = CreateProtoServiceFunction(MODULE, PS_SETSTATUS, (MIRANDASERVICE)SetStatusService);
	hServices[i++] = CreateProtoServiceFunction(MODULE, PS_GETSTATUS, (MIRANDASERVICE)GetStatus);
	hServices[i++] = CreateProtoServiceFunction(MODULE, PS_LOADICON, (MIRANDASERVICE)LoadIcon);
	hServices[i++] = CreateProtoServiceFunction(MODULE, PSS_GETINFO, (MIRANDASERVICE)GetInfo);

	hEventClistDblClick = HookEvent(ME_CLIST_DOUBLECLICKED, ClistDblClick);
}

void DeinitServices() {
	UnhookEvent(hEventClistDblClick);

	for(int i = 0; i < NUM_SERVICES; i++)
		if(hServices[i]) DestroyServiceFunction(hServices[i]);
}