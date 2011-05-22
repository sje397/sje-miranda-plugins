#include "stdafx.h"
#include "pingthread.h"

int upCount, total = 0;

HANDLE mainThread;
HANDLE hWakeEvent = 0; 

// thread protected variables
CRITICAL_SECTION thread_finished_cs, list_changed_cs, data_list_cs;
bool thread_finished = false, list_changed = false;
PINGLIST data_list;

HANDLE status_update_thread = 0;

bool get_thread_finished() {
	Lock(&thread_finished_cs, "get_thread_finished");
	bool retval = thread_finished;
	Unlock(&thread_finished_cs);
	return retval;
}

void set_thread_finished(bool f) {
	Lock(&thread_finished_cs, "set_thread_finished");
	thread_finished = f;
	Unlock(&thread_finished_cs);
}

bool get_list_changed() {
	Lock(&list_changed_cs, "get_list_changed");
	bool retval = list_changed;
	Unlock(&list_changed_cs);
	return retval;
}

void set_list_changed(bool f) {
	Lock(&list_changed_cs, "set_list_changed");
	list_changed = f;
	Unlock(&list_changed_cs);
}

void SetProtoStatus(char *pszLabel, char *pszProto, int if_status, int new_status) {
	if(strcmp(pszProto, Translate("<all>")) == 0) {
		int num_protocols;
		PROTOCOLDESCRIPTOR **pppDesc;

		CallService(MS_PROTO_ENUMPROTOCOLS, (LPARAM)&num_protocols, (WPARAM)&pppDesc);
		for(int i = 0; i < num_protocols; i++) {
			if(pppDesc[i]->type == PROTOTYPE_PROTOCOL)
				SetProtoStatus(pszLabel, pppDesc[i]->szName, if_status, new_status);
		}
	} else {
		std::ostringstream get_status, set_status;
		get_status << pszProto << PS_GETSTATUS;
		if(ServiceExists(get_status.str().c_str())) {
			set_status << pszProto << PS_SETSTATUS;
	
			if(CallService(get_status.str().c_str(), 0, 0) == if_status) {
				if(options.logging) {
					std::ostringstream oss2;
					oss2 << pszLabel << " - setting status of protocol '" << pszProto << "' (" << new_status << ")";
					CallService(PROTO "/Log", (WPARAM)oss2.str().c_str(), 0);
				}
				CallService(set_status.str().c_str(), new_status, 0);
			}
		}
	}
}

DWORD WINAPI sttCheckStatusThreadProc( void *vp)
{
	clock_t start_t = clock(), end_t;
	while(!get_thread_finished()) {
	
		end_t = clock();

		int wait = (int)((options.ping_period - ((end_t - start_t) / (double)CLOCKS_PER_SEC)) * 1000);
		if(wait > 0)
			WaitForSingleObjectEx(hWakeEvent, wait, TRUE);

		if(get_thread_finished()) break;

		start_t = clock();

		bool timeout = false;
		bool reply = false;
		int count = 0;

		PINGADDRESS pa;
		//HANDLE next = 0;

		std::pair<short, DWORD> history_entry;

		Lock(&data_list_cs, "ping thread loop start");
		set_list_changed(false);
		int size = data_list.size();
		Unlock(&data_list_cs);
		
		int ind;
		for(ind = 0; ind < size; ind++) {
			Lock(&data_list_cs, "ping thread loop start");
			int c = 0;
			for(PINGLIST::iterator i = data_list.begin(); i != data_list.end() && c <= ind; i++, c++) {
				if(c == ind) {
					// copy just what we need - i.e. not history, not command
					pa.get_status = i->get_status;
					pa.hContact = i->hContact;
					pa.miss_count = i->miss_count;
					pa.port = i->port;
					strcpy(pa.pszLabel, i->pszLabel);
					strcpy(pa.pszName, i->pszName);
					strcpy(pa.pszProto, i->pszProto);
					pa.set_status = i->set_status;
					pa.status = i->status;
					break;
				}
					
			}
			Unlock(&data_list_cs);

			if(get_thread_finished()) break;
			if(get_list_changed()) break;

			if(pa.status != options.off_status) {
				if(CallService(MS_DB_CONTACT_IS, (WPARAM)pa.hContact, 0)) {
					if(!options.no_test_status)
						DBWriteContactSettingWord(pa.hContact, PROTO, "Status", options.tstatus);

					CallService(PROTO "/Ping", 0, (LPARAM)&pa);	
				} else
					break;

				if(get_thread_finished()) break;
				if(get_list_changed()) break;

				Lock(&data_list_cs, "ping thread loop start");
				for(PINGLIST::iterator i = data_list.begin(); i != data_list.end(); i++) {
					if(i->hContact == pa.hContact) {
						i->responding = pa.responding;
						i->round_trip_time = pa.round_trip_time;
	
						history_entry.first = i->round_trip_time;
						history_entry.second = (DWORD)time(0);
						i->history.push_back(history_entry);
						// maintain history (-1 represents no response)
						while(i->history.size() >= MAX_HISTORY) {
							i->history.erase(i->history.begin());
							//i->history.pop_front();
						}

						if(pa.responding) {
							if(pa.miss_count > 0)
								pa.miss_count = -1;
							else
								pa.miss_count--;
							pa.status = options.rstatus;
						}
						else {
							if(pa.miss_count < 0)
								pa.miss_count = 1;
							else
								pa.miss_count++;
							pa.status = options.nrstatus;
						}
						i->miss_count = pa.miss_count;
						i->status = pa.status;

						break;
					}
				}
				Unlock(&data_list_cs);
				
				if(pa.responding) {	
					count++;

					if(pa.miss_count == -1 - options.retries || 
						(((-pa.miss_count) % (options.retries + 1)) == 0 && !options.block_reps))
					{
						reply = true;
						if(options.show_popup2 && ServiceExists(MS_POPUP_SHOWMESSAGE)) {
							ShowPopup("Ping Reply", pa.pszLabel, 1);
						}
						if(DBGetContactSettingWord(pa.hContact, PROTO, "Status", ID_STATUS_OFFLINE) != pa.status)
							DBWriteContactSettingWord(pa.hContact, PROTO, "Status", pa.status);
					}
					if(pa.miss_count == -1 - options.retries && options.logging) {
						std::ostringstream oss2;
						oss2 << pa.pszLabel << " - reply";
						CallService(PROTO "/Log", (WPARAM)oss2.str().c_str(), 0);
					}
					SetProtoStatus(pa.pszLabel, pa.pszProto, pa.get_status, pa.set_status);
					if(options.use_status_msg) {
						std::ostringstream msg;
						char *mode_desc = (char *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, options.rstatus, 0);
						msg << (mode_desc ? mode_desc : Translate("Uknnown")) << " (" << pa.round_trip_time << ")";
						DBWriteContactSettingString(pa.hContact, "CList", "StatusMsg", msg.str().c_str());
					} else {
						std::ostringstream handle;
						handle << pa.pszLabel << " (" << pa.round_trip_time << ")";
						set_changing_clist_handle(TRUE);
						DBWriteContactSettingString(pa.hContact, "CList", "MyHandle", handle.str().c_str());
						set_changing_clist_handle(FALSE);
					}
				} else {
					
					if(pa.miss_count == 1 + options.retries || 
						((pa.miss_count % (options.retries + 1)) == 0 && !options.block_reps))
					{
						timeout = true;
						if(options.show_popup && ServiceExists(MS_POPUP_SHOWMESSAGE)) {
							ShowPopup("Ping Timeout", pa.pszLabel, 0);
						}
						if(!timeout) timeout = true;
						if(DBGetContactSettingWord(pa.hContact, PROTO, "Status", ID_STATUS_OFFLINE) != pa.status)
							DBWriteContactSettingWord(pa.hContact, PROTO, "Status", pa.status);
					}
					if(pa.miss_count == 1 + options.retries && options.logging) {
						std::ostringstream oss2;
						oss2 << pa.pszLabel << " - timeout";
						CallService(PROTO "/Log", (WPARAM)oss2.str().c_str(), 0);
					}
					if(options.use_status_msg) {
						std::ostringstream msg;
						char *mode_desc = (char *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, options.nrstatus, 0);
						msg << (mode_desc ? mode_desc : Translate("Uknnown")) << " [" << pa.miss_count << "]";
						DBWriteContactSettingString(pa.hContact, "CList", "StatusMsg", msg.str().c_str());
					} else {
						std::ostringstream handle;
						handle << pa.pszLabel << " [" << pa.miss_count << "]";
						set_changing_clist_handle(TRUE);
						DBWriteContactSettingString(pa.hContact, "CList", "MyHandle", handle.str().c_str());
						set_changing_clist_handle(FALSE);
					}
				}
				if(!options.no_test_status && DBGetContactSettingWord(pa.hContact, PROTO, "Status", ID_STATUS_OFFLINE) != pa.status)
					DBWriteContactSettingWord(pa.hContact, PROTO, "Status", pa.status);
			}			
		}

		if(timeout) SkinPlaySound("PingProtoTimeout");
		if(reply) SkinPlaySound("PingProtoReply");

		if(!get_list_changed()) {
			upCount = count;
			total = ind;

			if(DBGetContactSettingByte(0, PROTO, "SetProtoStatus", 0)) {
				if(upCount == total)
					CallService(PROTO PS_SETSTATUS, options.rstatus, 0);
				else if(upCount == 0)
					CallService(PROTO PS_SETSTATUS, options.nrstatus, 0);
			}
		} else {
			total = 0;
		}
	}

	return 0;
}

void start_ping_thread() {
	DWORD tid;

	if(status_update_thread) CloseHandle(status_update_thread);
	status_update_thread = CreateThread(0, 0, sttCheckStatusThreadProc, 0, 0, &tid);
}

void stop_ping_thread() {
	set_thread_finished(true);
	SetEvent(hWakeEvent);
	//ICMP::get_instance()->stop();
	WaitForSingleObject(status_update_thread, 2000);
	TerminateThread(status_update_thread, 0);
	CloseHandle(status_update_thread);
	status_update_thread = 0;
}

int FillList(WPARAM wParam, LPARAM lParam) {

	if(options.logging)
		CallService(PROTO "/Log", (WPARAM)"ping address list reload", 0);

	PINGLIST pl;
	CallService(PROTO "/GetPingList", 0, (LPARAM)&pl);

	Lock(&data_list_cs, "fill_list");

	// copy history
	for(PINGLIST::iterator i = data_list.begin(); i != data_list.end(); i++) {
		for(PINGLIST::iterator j = pl.begin(); j != pl.end(); j++) {
			if(i->hContact == j->hContact) {
				if(i->history.size())
					j->history = i->history;
				else
					j->history.clear();
			}
		}
	}
	
	if(pl.size())
		data_list = pl;

	set_list_changed(true);
	Unlock(&data_list_cs);

	SetEvent(hWakeEvent);
	return 0;
}

