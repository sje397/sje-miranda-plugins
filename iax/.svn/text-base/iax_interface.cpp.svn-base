#include "common.h"
#include "iax_interface.h"
#include "options.h"
#include "services.h"
#include "utils.h"
#include "resource.h"
#include "dial_dlg.h"
#include "icons.h"

#include <process.h>
#include <time.h>

#define NUM_LINES	3

typedef struct {
	HWND hwndPopup;
	bool ring_thread;
	int state;
	char cid_name[256];
	char cid_num[256];
} LineInfo;

LineInfo line_info[NUM_LINES] = {0};

int reg_id = 0;
float silence_threshold = -99;
//double silence_threshold = 0;

void SendDTMF(char c) {
	iaxc_send_dtmf(c);	
}

void Call(char *number) {
	char buff[512];
	mir_snprintf(buff, 512, "%s:%s@%s/%s", options.username, options.password, options.host, number);
	iaxc_call(buff);
}

void HangUp() {
	iaxc_reject_call();
}

void SelectLine(int line) {
	iaxc_select_call(line);
}

void SetLineNo(int line) {
	iaxc_select_call(line);
}

int GetLineNo() {
	return iaxc_selected_call();
}

int GetLineState(int line) {
	return line_info[line].state;
}

void AnswerCall(int callNo) {
	WMPPause();
	iaxc_answer_call(callNo);
	iaxc_select_call(callNo);
}

char *GetLineCIDNum(int callNo) {
	return line_info[callNo].cid_num;
}

char *GetLineCIDName(int callNo) {
	return line_info[callNo].cid_name;
}

int text_event_callback(iaxc_ev_text text) {
	switch(text.type) {
		case IAXC_TEXT_TYPE_STATUS:
			ShowInfo(Translate("Status: %s"), Translate(text.message));
			break;
		case IAXC_TEXT_TYPE_NOTICE:
			ShowInfo(Translate("Notice: %s"), Translate(text.message));
			break;
		case IAXC_TEXT_TYPE_ERROR:
			ShowError(Translate(text.message));
			break;
		case IAXC_TEXT_TYPE_FATALERROR:
			ShowError(Translate("Fatal: %s"), Translate(text.message));
			break;
		case IAXC_TEXT_TYPE_IAX:
			ShowInfo(Translate("Text: %s"), text.message);
			break;
	}
	return 0;
}

LRESULT CALLBACK CallPopupWindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
	switch( message ) {
		case UM_INITPOPUP:
			{
				int callNo = CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd,0);
				line_info[callNo].hwndPopup = hWnd;
			}
			return 0;
		case WM_COMMAND:
			{
				int callNo = CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd,0);
				iaxc_answer_call(callNo);
			}
			PUDeletePopUp( hWnd );
			return TRUE;

		case WM_CONTEXTMENU:
			{
				int callNo = CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd,0);
				iaxc_reject_call_number(callNo);
			}
			PUDeletePopUp( hWnd );
			return TRUE;

		case UM_FREEPLUGINDATA: 
			{
			}
			return TRUE;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

unsigned int CALLBACK RingThread(void *param) {
	int callNo = (int)param;
	while((GetLineState(callNo) & IAXC_CALL_STATE_RINGING) && !Miranda_Terminated()) {
		if(GetLineState(callNo) & IAXC_CALL_STATE_OUTGOING)
			SkinPlaySound("iax_ring_out");
		else
			SkinPlaySound("iax_ring_in");
		Sleep(3000);
	}
	line_info[callNo].ring_thread = false;
	return 0;
}

void AddClistIcon(HANDLE hContact, int line_no) {
	CLISTEVENT cli = {0};
	cli.cbSize = sizeof(CLISTEVENT);
	cli.flags = CLEF_URGENT;
	cli.hContact = hContact;
	cli.hDbEvent = (HANDLE)line_no;
	cli.hIcon = hIconRing;
	cli.pszService = MODULE "/ClistIconClick";
	cli.pszTooltip = (line_info[line_no].state & IAXC_CALL_STATE_OUTGOING) ? Translate("Ringing (OUT)") : Translate("Ringing (IN)");
	CallService(MS_CLIST_ADDEVENT, 0, (LPARAM)&cli);
}

int ClistIconClick(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;
	CLISTEVENT *cli = (CLISTEVENT *)lParam;
	int line_no = (int)cli->hDbEvent;

	if(!(line_info[line_no].state & IAXC_CALL_STATE_RINGING))
		return 0;
	/*
	if(line_info[line_no].state & IAXC_CALL_STATE_OUTGOING) {
		// ask if user wants to hang up or leave it ringing
	} else {
		// ask if user wants to answer call or reject call or leave it ringing
	}
	*/

	ShowDialDialog(true);
	//AddClistIcon(hContact, line_no);
	//CallServiceSync(MS_CLIST_CONTACTDOUBLECLICKED, (WPARAM)hContact, 0);
	return 0;
}

int state_event_callback(iaxc_ev_call_state call){

	//ShowInfo("Line %d, name = %s", call.callNo, call.remote_name);

	line_info[call.callNo].state = call.state;
	if(call.remote_name) strncpy(line_info[call.callNo].cid_name, call.remote_name, 256);
	else line_info[call.callNo].cid_name[0] = 0;
	if(call.remote) strncpy(line_info[call.callNo].cid_num, call.remote, 256);
	else line_info[call.callNo].cid_num[0] = 0;

	HANDLE hContact = FindContact(call.remote); 

	RefreshLineInfo();

	bool active = (call.state & IAXC_CALL_STATE_ACTIVE) != 0;
	bool ringing = (call.state & IAXC_CALL_STATE_RINGING) != 0;
	bool outgoing = (call.state & IAXC_CALL_STATE_OUTGOING) != 0;

	if(!active) {
		DBWriteContactSettingWord(hContact, MODULE, "Status", ID_STATUS_ONLINE);
		DWORD current_time = (DWORD)time(0), last_call = DBGetContactSettingDword(hContact, MODULE, "LastCall", current_time);
		if(current_time - last_call > 0) {
			DBWriteContactSettingDword(hContact, MODULE, "LastCallDuration", current_time - last_call);
		}
		DBWriteContactSettingByte(hContact, MODULE, "LineNo", 0xff);
	}  else {
		if(ringing) {

			if(!line_info[call.callNo].ring_thread) {
				CloseHandle((HANDLE)_beginthreadex(0, 0, RingThread, (void *)call.callNo, 0, 0));
				line_info[call.callNo].ring_thread = true;
			}

			if(!hContact) hContact = CreateContact(call.remote, call.remote, true); //call.remote_name);
			DBWriteContactSettingByte(hContact, MODULE, "LineNo", call.callNo);
			if(outgoing) {
				DBWriteContactSettingWord(hContact, MODULE, "Status", ID_STATUS_NA);
				if(options.pop_dial_out) ShowDialDialog(false);
			} else {
				DBWriteContactSettingWord(hContact, MODULE, "Status", ID_STATUS_FREECHAT);
				if(options.pop_dial_in) ShowDialDialog(false);
			}

		} else if(hContact) {
			DBWriteContactSettingWord(hContact, MODULE, "Status", ID_STATUS_ONTHEPHONE);
			DBWriteContactSettingDword(hContact, MODULE, "LastCall", (DWORD)time(0));
			DBWriteContactSettingByte(hContact, MODULE, "LineNo", call.callNo);
		}
	}

    return 0;
}

int registration_event_callback(iaxc_ev_registration reg) {
	if(reg.reply == IAXC_REGISTRATION_REPLY_ACK) {
		SetStatusAndACK(ID_STATUS_ONLINE);
		if(reg.msgcount > 0) ShowInfo(Translate("You have %d messages."), reg.msgcount);
		SetContactStatus(ID_STATUS_ONLINE);
	} else if(reg.reply == IAXC_REGISTRATION_REPLY_REJ) {
		if(reg_id) ShowError(Translate("Registration rejected"));
		Disconnect();
		SetStatusAndACK(ID_STATUS_OFFLINE);
	} else if(reg.reply == IAXC_REGISTRATION_REPLY_TIMEOUT) {
		if(reg_id) ShowError(Translate("Registration timeout"));
		Disconnect();
		SetStatusAndACK(ID_STATUS_OFFLINE);
	}
	return 0;
}

int levels_callback(float input, float output) {
	SetLevels(input, output);
	return 0;
}

int iaxc_callback(iaxc_event e)
{
    switch(e.type) {
        case IAXC_EVENT_LEVELS:
            return levels_callback(e.ev.levels.input, e.ev.levels.output);
        case IAXC_EVENT_NETSTAT:
            //return netstat_callback(e.ev.netstats);
			return 0;
        case IAXC_EVENT_TEXT:
            return text_event_callback(e.ev.text);
        case IAXC_EVENT_STATE:
            return state_event_callback(e.ev.call);
		case IAXC_EVENT_REGISTRATION:
			return registration_event_callback(e.ev.reg);
		case IAXC_EVENT_VIDEO:
			return 0;
        default:
            return 0;  // not handled
    }
}


void Connect() {
	if(reg_id) Disconnect();

	if(options.username && options.password && options.host) {
		if(options.cid_name)
			iaxc_set_callerid(options.cid_name, options.cid_number);
		reg_id = iaxc_register(options.username, options.password, options.host);	
		SetStatusAndACK(ID_STATUS_CONNECTING);
	} else
		SetStatusAndACK(ID_STATUS_OFFLINE);
}

void Disconnect() {
	if(reg_id) {
		iaxc_unregister(reg_id);
		reg_id = 0;
		Sleep(500);
	}
}

HANDLE hServiceIconClist;
bool InitIAXInterface() {
	SkinAddNewSoundEx("iax_ring_in", Translate("IAX"), Translate("Ring (Inbound)"));
	SkinAddNewSoundEx("iax_ring_out", Translate("IAX"), Translate("Ring (Outbound)"));
	SkinAddNewSoundEx("iax_keypad", Translate("IAX"), Translate("Key Press"));

	hServiceIconClist = CreateServiceFunction(MODULE "/ClistIconClick", ClistIconClick);

	iaxc_set_preferred_source_udp_port(DBGetContactSettingDword(0, MODULE, "Port", 4569));
	if(iaxc_initialize(NUM_LINES)) return false;
	
	iaxc_set_formats(options.codecs_preferred, options.codecs_allowed);
	//iaxc_video_format_set_cap(IAXC_FORMAT_THEORA, IAXC_FORMAT_THEORA);
	iaxc_video_format_set(IAXC_FORMAT_THEORA, IAXC_FORMAT_THEORA, 10, 16 * 1024 * 8, 320, 240, 8000);
	//iaxc_set_filters(IAXC_FILTER_CN);
	
	iaxc_set_speex_settings(1,-1,-1,0,8000,3);

	iaxc_set_silence_threshold(silence_threshold);
	
	iaxc_set_event_callback(iaxc_callback); 

	iaxc_start_processing_thread();
	
	return true;
}

void DeinitIAXInterface() {
	DestroyServiceFunction(hServiceIconClist);

	iaxc_dump_call();
	iaxc_millisleep(1000);
	if (reg_id) iaxc_unregister(reg_id);
	iaxc_stop_processing_thread();
	iaxc_shutdown();
}


