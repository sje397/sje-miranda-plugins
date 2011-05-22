#include "common.h"
#include "server_con.h"
#include "net.h"
#include "arc4.h"
#include "options.h"
#include "nick_dialog.h"
#include "formatting.h"
#include "notifications.h"

#include <malloc.h>
#include <ctime>
#include <cstdlib>

#define SERVER_READ_BUFFER_SIZE				(1024 * 32)

/* TODO: obtain IPs of network interfaces from user's machine, instead of
 * hardcoding these values below (used in msim_compute_login_response). 
 * This is not immediately
 * important because you can still connect and perform basic
 * functions of the protocol. There is also a high chance that the addreses
 * are RFC1918 private, so the servers couldn't do anything with them
 * anyways except make note of that fact. Probably important for any
 * kind of direct connection, or file transfer functionality.
 */

#define LOGIN_IP_LIST  "\x00\x00\x00\x00\x05\x7f\x00\x00\x01\x00\x00\x00\x00\x0a\x00\x00\x40\xc0\xa8\x58\x01\xc0\xa8\x3c\x01"
#define LOGIN_IP_LIST_LEN	25

#define NONCE_SIZE		0x20

int status = ID_STATUS_OFFLINE;
bool myspace_server_running = false, server_stop = false;
HANDLE server_connection = 0;
int sesskey = 0, req_id = 1, my_uid = 0;
int signon_status = ID_STATUS_ONLINE;
char signon_status_msg[512] = {0};
CRITICAL_SECTION write_cs;

bool WriteData(char *fn, int fn_size, char *data, int data_size) {
	char szTempPath[MAX_PATH], szTempFileName[MAX_PATH];

	if (GetTempPathA(sizeof(szTempPath), szTempPath) <= 0) lstrcpyA( szTempPath, ".\\" );

	mir_snprintf(szTempFileName, MAX_PATH, "%s%s", szTempPath, fn);
	mir_snprintf(fn, fn_size, "%s", szTempFileName);

	HANDLE hSaveFile = CreateFileA(fn, GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if(hSaveFile == INVALID_HANDLE_VALUE) return false;

	unsigned long bytes_written = 0;
	bool ret = false;
	if(WriteFile(hSaveFile, data, data_size, &bytes_written, NULL) == TRUE) ret = true;
	CloseHandle(hSaveFile);
	return ret;
}

int stat_mir_to_myspace(int mir_status) {
	switch(mir_status) {
		case ID_STATUS_INVISIBLE: return 0;
		case ID_STATUS_ONLINE: return 1;
		case ID_STATUS_AWAY: return 5;

		case ID_STATUS_IDLE: return 2;
	}
	return 0;
}

int stat_myspace_to_mir(int myspace_status) {
	switch(myspace_status) {
		case 0: return ID_STATUS_OFFLINE;
		case 1: return ID_STATUS_ONLINE;
		case 2: return ID_STATUS_IDLE;
		case 5: return ID_STATUS_AWAY;
	}
	return ID_STATUS_OFFLINE;
}

// translate request to change to mode into actual mode
int stat_mir_to_mir(int mir_status) {
	switch(mir_status) {
		case ID_STATUS_INVISIBLE: return ID_STATUS_INVISIBLE;
		case ID_STATUS_ONLINE: return ID_STATUS_ONLINE;
		case ID_STATUS_AWAY: return ID_STATUS_AWAY;

		case ID_STATUS_DND: return ID_STATUS_AWAY;
		case ID_STATUS_NA: return ID_STATUS_AWAY;
		case ID_STATUS_OCCUPIED: return ID_STATUS_AWAY;
		case ID_STATUS_ONTHEPHONE: return ID_STATUS_AWAY;
		case ID_STATUS_OUTTOLUNCH: return ID_STATUS_AWAY;

		case ID_STATUS_FREECHAT: return ID_STATUS_ONLINE;

		case ID_STATUS_IDLE: return ID_STATUS_IDLE;
	}
	return ID_STATUS_OFFLINE;
}

HANDLE FindContact(int uid) {
	char *proto;
	int cuid;
	HANDLE hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
	while ( hContact != NULL )
	{
		proto = ( char* )CallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM )hContact,0 );
		if ( proto && !strcmp( MODULE, proto)) {
			cuid = DBGetContactSettingDword(hContact, MODULE, "UID", (DWORD)0);
			if(cuid == uid) {
				return hContact;
			}
		}
		hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
	}	

	return 0;
}

int LookupUID(int uid) {
	int ret;
	// lookup user
	ClientNetMessage msg_lookup;
	msg_lookup.add_int("persist", 1);
	msg_lookup.add_int("sesskey", sesskey);
	msg_lookup.add_int("uid", my_uid);
	msg_lookup.add_int("cmd", 1);
	msg_lookup.add_int("dsn", 4);
	msg_lookup.add_int("lid", 3);
	msg_lookup.add_int("rid", ret = req_id++);
	char body[512];
	mir_snprintf(body, 512, "UserID=%d", uid);
	msg_lookup.add_string("body", body);
	SendMessage(msg_lookup);

	return ret;
}

void AddAllContactsToServer() {
	char *proto;
	int uid;
	HANDLE hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
	while ( hContact != NULL )
	{
		proto = ( char* )CallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM )hContact,0 );
		if ( proto && !strcmp( MODULE, proto)) {
			uid = DBGetContactSettingDword(hContact, MODULE, "UID", (DWORD)0);
			AddBuddyOnServer(uid);
		}
		hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
	}	
}

void AddBuddyOnServer(int uid) {
	ClientNetMessage msg_add;
	msg_add.add_string("addbuddy", "");
	msg_add.add_int("sesskey", sesskey);
	msg_add.add_int("newprofileid", uid);
	msg_add.add_string("reason", "");

	SendMessage(msg_add);

	// update block list
	ClientNetMessage msg_block;
	msg_block.add_string("blocklist", "");
	msg_block.add_int("sesskey", sesskey);
	
	char idlist[1024];
	mir_snprintf(idlist, 1024, "b-|%d|a+|%d", uid, uid);
	msg_block.add_string("idlist", idlist);

	SendMessage(msg_block);

	// update server side list?
	ClientNetMessage msg_srvlist;
	msg_srvlist.add_int("persist", 1);
	msg_srvlist.add_int("sesskey", sesskey);
	msg_srvlist.add_int("cmd", 514);
	msg_srvlist.add_int("dsn", 0);
	msg_srvlist.add_int("uid", my_uid);
	msg_srvlist.add_int("lid", 9);
	msg_srvlist.add_int("rid", 31);
	
	char body[1024];
	mir_snprintf(body, 1024, "ContactID=%d", uid);
	msg_srvlist.add_string("body", body);

	SendMessage(msg_srvlist);
}

HANDLE CreateContact(int uid, char *nick, char *email, bool add_buddy) {
	HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_ADD, 0, 0);

	if(hContact) {
		DBWriteContactSettingDword(hContact, MODULE, "UID", uid);
		if(nick && strlen(nick)) {
			DBWriteContactSettingStringUtf(hContact, MODULE, "Nick", nick);
		}
		if(email && strlen(email)) {
			DBWriteContactSettingStringUtf(hContact, MODULE, "email", email);
		}


		if(add_buddy) AddBuddyOnServer(uid);

		/*			 
		// update contact details?
		ClientNetMessage msg_persist;
		msg_persist.add_int("persist", 1);
		msg_persist.add_int("sesskey", sesskey);

		msg_persist.add_int("cmd", 514);
		msg_persist.add_int("dsn", 0);
		msg_persist.add_int("uid", DBGetContactSettingDword(0, MODULE, "UID", 0));
		msg_persist.add_int("lid", 9);
		msg_persist.add_int("rid", req_id++);
		char body[4096];
		mir_snprintf(body, 4096, "ContactID=%d\x1cGroupName=\x1cPosition=1000\x1cVisibility=2\x1cNickName=\x1cNameSelect=0", mpsr->uid);
		msg_persist.add_string("body", body);
		SendMessage(msg_persist);
		*/

		CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)MODULE);
	}

	return hContact;
}

void SendMessage(ClientNetMessage &msg) {
	char packet[4096];
	int packet_size = msg.make_packet(packet, 4096);
	if(server_connection) {
		EnterCriticalSection(&write_cs);
		Netlib_Send(server_connection, packet, packet_size, MSG_DUMPASTEXT);
		LeaveCriticalSection(&write_cs);
	}
}

void CALLBACK sttMainThreadStatusCallback( ULONG dwParam ) {
	if(status != dwParam) {
		int previous_status = status;
		status = dwParam;
		ProtoBroadcastAck(MODULE,NULL,ACKTYPE_STATUS,ACKRESULT_SUCCESS, (HANDLE)previous_status, status);
	}
}

int waitcallback(unsigned int *timeout) {
	//PUShowMessage("waitcallback", SM_NOTIFY);
	return server_stop ? 0 : 1;
}

int try_ports[9] = {1863,6660,6661,6662,6665,6668,6669,80,443};

void try_login(NetMessage &msg, HANDLE connection) {
	char nonce[NONCE_SIZE * 2 + 2], *nc1 = nonce, *nc2 = nonce + NONCE_SIZE;
	int size = NONCE_SIZE * 2 + 2;
	if(msg.get_data("nc", nonce, &size) && size == NONCE_SIZE * 2) {
		SHA1_INTERFACE sha1;
		mir_sha1_ctx sha1_ctx;
		mir_getSHA1I(&sha1);

		ARC4_INTERFACE arc4;
		mir_arc4_ctx arc4_ctx;
		mir_getARC4I(&arc4);
		
		char *ch_resp;
		mir_sha1_byte_t pw_hash[20];
		int ch_resp_size;
		mir_sha1_byte_t key[20];
		char email[256];

		// only use first 10 chars of pw
		wchar_t wpw[11];
		wpw[10] = 0;

#ifdef _UNICODE
		_tcsncpy(wpw, options.pw, 10);
		WideCharToMultiByte(CP_UTF8, 0, options.email, -1, email, 256, 0, 0);
#else
		strcpy(email, options.email);
		MultiByteToWideChar(code_page, 0, options.pw, -1, wpw, 10);
#endif
		// and make it lower case!?!?
		_wcslwr(wpw);

		sha1.sha1_hash((mir_sha1_byte_t*)wpw, wcslen(wpw) * sizeof(wchar_t), pw_hash);

		sha1.sha1_init(&sha1_ctx);
		sha1.sha1_append(&sha1_ctx, (mir_sha1_byte_t*)pw_hash, 20);
		sha1.sha1_append(&sha1_ctx, (mir_sha1_byte_t*)nc2, NONCE_SIZE);
		sha1.sha1_finish(&sha1_ctx, key);

		arc4.arc4_init(&arc4_ctx, (char *)key, 0x10);

		ch_resp_size = NONCE_SIZE + strlen(email) + LOGIN_IP_LIST_LEN;
		ch_resp = new char[ch_resp_size];
		memcpy(ch_resp, nc1, NONCE_SIZE);
		memcpy(ch_resp + NONCE_SIZE, email, strlen(email));
		memcpy(ch_resp + NONCE_SIZE + strlen(email), LOGIN_IP_LIST, LOGIN_IP_LIST_LEN);
		
		arc4.arc4_crypt(&arc4_ctx, ch_resp, ch_resp, ch_resp_size);

		ClientNetMessage reply;
		reply.add_int("login2", 196610);
		reply.add_string("username", email);
		reply.add_data("response", ch_resp, ch_resp_size);
		reply.add_int("clientver", CLIENT_VER);
		reply.add_int("reconn", 0);
		reply.add_int("status", 100);
		reply.add_int("id", 1);

		delete[] ch_resp;

		SendMessage(reply);
	} else {
		PUShowMessage("Nonce format error", SM_NOTIFY);
	}
}

int ParseStatusMessage(HANDLE hContact, PipedStringList &l) {
	int stat = stat_myspace_to_mir(l.get_int(1));
	if(stat == ID_STATUS_IDLE) {
		if(DBWriteContactSettingWord(hContact, MODULE, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE)
			DBWriteContactSettingWord(hContact, MODULE, "Status", ID_STATUS_ONLINE);
		DBWriteContactSettingDword(hContact, MODULE, "IdleTS", (DWORD)time(0));
	} else {
		DBWriteContactSettingWord(hContact, MODULE, "Status", stat);
		DBWriteContactSettingDword(hContact, MODULE, "IdleTS", 0);
	}
	char smsg[512];
	if(l.get_string(3, smsg, 512))
		DBWriteContactSettingStringUtf(hContact, MODULE, "StatusMsg", smsg);
	return stat;
}

void __cdecl CheckMailThreadFunc(void*) {
	while(server_connection) {
		ClientNetMessage chk_msg;
		chk_msg.add_int("persist", 1);
		chk_msg.add_int("sesskey", sesskey);
		chk_msg.add_int("uid", my_uid);
		chk_msg.add_int("cmd", 1);
		chk_msg.add_int("dsn", 7);
		chk_msg.add_int("lid", 18);
		chk_msg.add_int("rid", req_id++);
		chk_msg.add_string("body", "");
		SendMessage(chk_msg);
		SleepEx(3 * 60 * 1000, TRUE);
	}
}

void __cdecl ServerThreadFunc(void*) {
	InitializeCriticalSection(&write_cs);

	NETLIBOPENCONNECTION conn_data = {0};
	conn_data.cbSize = sizeof(NETLIBOPENCONNECTION);
	conn_data.flags = NLOCF_V2;
	conn_data.szHost = "im.myspace.akadns.net";
	conn_data.wPort = DBGetContactSettingDword(0, MODULE, "LastPort", try_ports[0]);
	conn_data.timeout = 10;
	conn_data.waitcallback = waitcallback;

	int conn_stat = ID_STATUS_CONNECTING;
	QueueUserAPC(sttMainThreadStatusCallback, mainThread, conn_stat);		

	myspace_server_running = true;

	char *recv_buffer = new char[SERVER_READ_BUFFER_SIZE + 1];
	int bytes = 0;
	char mt[256];
	int tries = 0;
	bool login = true;
	int bytes_read;

	HANDLE connection = 0;

	char *pbuff = recv_buffer, *end;
	int buffer_bytes = 0;
	recv_buffer[SERVER_READ_BUFFER_SIZE] = 0;
	while(!Miranda_Terminated() && !server_stop) {
		if(login) {
			if(connection) Netlib_CloseHandle(connection);
			QueueUserAPC(sttMainThreadStatusCallback, mainThread, conn_stat++);
			connection = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)hNetlibUser, (LPARAM)&conn_data);
		}
		if(connection) {
			bytes = Netlib_Recv(connection, (char *)pbuff, SERVER_READ_BUFFER_SIZE - (pbuff - recv_buffer), MSG_DUMPASTEXT);
			buffer_bytes += bytes;
			pbuff[bytes] = 0;
		}
		if(connection == 0 || bytes == 0 || bytes == SOCKET_ERROR) {
			//PUShowMessage("Connection closed", SM_NOTIFY);
			if(login && tries < 9) {
				conn_data.wPort = try_ports[tries++];
			} else {
				if(login) ProtoBroadcastAck(MODULE, 0, ACKTYPE_LOGIN, ACKRESULT_FAILED, (HANDLE)0, (LPARAM)LOGINERR_NOSERVER);
				break;
			}
		} else {
			if(login) {
				QueueUserAPC(sttMainThreadStatusCallback, mainThread, conn_stat++);
				login = false;
				server_connection = connection;
				DBWriteContactSettingDword(0, MODULE, "LastPort", conn_data.wPort);
			}

			while((end = strstr(pbuff, "\\final\\")) != 0) {
				//mir_snprintf(mt, 256, "recvd %d bytes", bytes);
				//PUShowMessage(mt, SM_NOTIFY);
				end += 7;
				if(end - pbuff > bytes) {
					//PUShowMessage("More than one packet", SM_NOTIFY);
				}
				if(pbuff != recv_buffer) {
					//PUShowMessage("Not first packet", SM_NOTIFY);
				}

				*(end - 1) = 0;
				NetMessage msg;
				msg.parse(pbuff, end - pbuff);
				if(msg.exists(NMString("error"))) {
					char errmsg[256];
					if(msg.get_string("errmsg", errmsg, 256)) {
						PUShowMessage(errmsg, SM_WARNING);
						int code = msg.get_int("err");
						if(code == 6) ProtoBroadcastAck(MODULE, 0, ACKTYPE_LOGIN, ACKRESULT_FAILED, (HANDLE)0, (LPARAM)LOGINERR_OTHERLOCATION);
						if(code == 259) ProtoBroadcastAck(MODULE, 0, ACKTYPE_LOGIN, ACKRESULT_FAILED, (HANDLE)0, (LPARAM)LOGINERR_BADUSERID);
						if(code == 260) ProtoBroadcastAck(MODULE, 0, ACKTYPE_LOGIN, ACKRESULT_FAILED, (HANDLE)0, (LPARAM)LOGINERR_WRONGPASSWORD);
					}
				} else if(msg.get_int("lc") == 1) {
					QueueUserAPC(sttMainThreadStatusCallback, mainThread, conn_stat++);
					try_login(msg, server_connection);
				} else if(msg.get_int("lc") == 2) {
					sesskey = msg.get_int("sesskey");
					DBWriteContactSettingDword(0, MODULE, "UID", my_uid = msg.get_int("userid"));
					char nick[256];
					if(msg.get_string("uniquenick", nick, 256))
						DBWriteContactSettingStringUtf(0, MODULE, "Nick", nick);
					QueueUserAPC(sttMainThreadStatusCallback, mainThread, signon_status);

					if(my_uid == msg.get_int("uniquenick")) {
						//  need to pick a nick
						ShowNickDialog();
					}
					
					
					// update our options on server
					Dictionary d;
					d.add_string("Sound", options.sound ? "True" : "False");
					d.add_int("PrivacyMode", options.privacy_mode);
					d.add_string("ShowOnlyToList", options.show_only_to_list ? "True" : "False");
					d.add_int("OfflineMessageMode", options.offline_message_mode);
					d.add_string("Headline", "");
					d.add_int("Alert", 1);
					d.add_string("ShowAvatar", options.show_avatar ? "True" : "False");
					d.add_string("IMName", options.im_name);

					ClientNetMessage cmsg;
					cmsg.add_int("persist", 1);
					cmsg.add_int("sesskey", sesskey);
					cmsg.add_int("uid", DBGetContactSettingDword(0, MODULE, "UID", 0));
					cmsg.add_int("cmd", 514);
					cmsg.add_int("dsn", 1);
					cmsg.add_int("lid", 10);
					cmsg.add_int("rid", req_id++);
					cmsg.add_dict("body", d);
					SendMessage(cmsg);

					// set blocklist
					ClientNetMessage msg_block;
					msg_block.add_string("blocklist", "");
					msg_block.add_int("sesskey", sesskey);
					msg_block.add_string("idlist", "w0|c0|a-|*|b-|*");
					SendMessage(msg_block);

					// set status
					ClientNetMessage msg_status;
					msg_status.add_int("status", stat_mir_to_myspace(signon_status));
					msg_status.add_int("sesskey", sesskey);
					msg_status.add_string("statstring", signon_status_msg);
					msg_status.add_string("locstring", "");
					SendMessage(msg_status);
					
					LookupUID(my_uid);

					/*
					// set login time?
					Dictionary ld;
					ld.add_int("ContactType", 1);
					ld.add_int("LastLogin", timestamp);
					ClientNetMessage msg_setinfo;
					msg_setinfo.add_string("setinfo", "");
					msg_block.add_int("sesskey", sesskey);
					msg_setinfo.add_dict("info", ld);
					SendMessage(msg_setinfo);
					*/

					// add all contacts to server (old versions didn't do it :| )
					if(DBGetContactSettingByte(0, MODULE, "ContactsOnServer", 0) == 0) {
						AddAllContactsToServer();
						DBWriteContactSettingByte(0, MODULE, "ContactsOnServer", 1);
					}
					
					// start mail checking thread
					mir_forkthread(CheckMailThreadFunc, 0);

				} else if(msg.get_int("bm") == 200) { // action message
					int uid = msg.get_int("f");
					if(uid) {
						HANDLE hContact = FindContact(uid);
						if(hContact) {
							char cmv[256];
							if(msg.get_string("msg", cmv, 256)) {
								DBWriteContactSettingStringUtf(hContact, MODULE, "MirVer", cmv);
							}
						}
					}
				} else if(msg.get_int("bm") == 100) { // status message
					int uid = msg.get_int("f");
					if(uid) {
						HANDLE hContact = FindContact(uid);
						if(!hContact) {
							hContact = CreateContact(uid, 0, 0, false);
							LookupUID(uid);
						}
						PipedStringList l = msg.get_list("msg");
						int old_status = DBGetContactSettingWord(hContact, MODULE, "Status", ID_STATUS_OFFLINE),
							new_status = ParseStatusMessage(hContact, l);
						if(status != ID_STATUS_INVISIBLE && old_status == ID_STATUS_OFFLINE && new_status != ID_STATUS_OFFLINE) {
							ClientNetMessage msg;
							msg.add_int("bm", 200);
							msg.add_int("sesskey", sesskey);
							msg.add_int("t", DBGetContactSettingDword(hContact, MODULE, "UID", 0));
							msg.add_int("f", my_uid);
							msg.add_int("cv", CLIENT_VER);
							msg.add_string("msg", mir_ver);
							SendMessage(msg);
						}
					}
				} else if(msg.get_int("bm") == 121) { // action message
					int uid = msg.get_int("f");
					if(uid) {
						HANDLE hContact = FindContact(uid);
						if(!hContact) {
							hContact = CreateContact(uid, 0, 0, false);
							DBWriteContactSettingByte(hContact, "CList", "NotOnList", 1);
							DBWriteContactSettingByte(hContact, "CList", "Hidden", 1);
							LookupUID(uid);
						}
						char smsg[1024];
						if(msg.get_string("msg", smsg, 1024)) {
							if(strcmp(smsg, "%typing%") == 0)
								CallService(MS_PROTO_CONTACTISTYPING, (WPARAM)hContact, (LPARAM)20);
							else if(strcmp(smsg, "%stoptyping%") == 0) {
								CallService(MS_PROTO_CONTACTISTYPING, (WPARAM)hContact, (LPARAM)0);
							} else if(strncmp(smsg, "!!!ZAP_SEND!!!=RTE_BTN_ZAPS_", 28) == 0) {
								// recvd a zap
								int zap_num = smsg[28] - '0';
								NotifyZapRecv(hContact, zap_num);
							}
						}
					}
				} else if(msg.get_int("bm") == 1) { // instant message
					int uid = msg.get_int("f");
					if(uid) {
						HANDLE hContact = FindContact(uid);
						if(!hContact) {
							hContact = CreateContact(uid, 0, 0, false);
							DBWriteContactSettingByte(hContact, "CList", "NotOnList", 1);
							DBWriteContactSettingByte(hContact, "CList", "Hidden", 1);
							LookupUID(uid);
						}
						char text[MAX_MESSAGE_SIZE + 1];
						if(msg.get_string("msg", text, MAX_MESSAGE_SIZE + 1)) {
							// handle dodgy client with wrong message type
							if(strcmp(text, "%typing%") == 0) {
								CallService(MS_PROTO_CONTACTISTYPING, (WPARAM)hContact, (LPARAM)20);
							} else if(strcmp(text, "%stoptyping%") == 0) {
								CallService(MS_PROTO_CONTACTISTYPING, (WPARAM)hContact, (LPARAM)0);
							} else {
								CallService(MS_PROTO_CONTACTISTYPING, (WPARAM)hContact, (LPARAM)0); // auto end typing

								strip_tags(text);
								decode_smileys(text);
								unentitize_xml(text);

								PROTORECVEVENT pre = {0};
								pre.flags = PREF_UTF;
								pre.szMessage = text;
								pre.timestamp = (DWORD)time(0);
								pre.lParam = EVENTTYPE_MESSAGE;

								CCSDATA css = {0};
								css.hContact = hContact;
								css.lParam = (LPARAM)&pre;
								css.szProtoService = PSR_MESSAGE;
								
								CallService(MS_PROTO_CHAINRECV, 0, (LPARAM)&css);
							}
						}
					}
				} else if(msg.exists(NMString("persistr"))) {
					int cmd, dsn, lid, req;
					cmd = msg.get_int("cmd") & 255;
					dsn = msg.get_int("dsn");
					lid = msg.get_int("lid");
					req = msg.get_int("rid");

					//mir_snprintf(mt, 256, "Peristr message: type is %d,%d,%d", cmd, dsn, lid);
					//PUShowMessage(mt, SM_NOTIFY);
					if(cmd == 1 && dsn == 5 && lid == 7) { // userinfo (lookup by username/email)
						Dictionary body = msg.get_dict("body");
						char email[256], nick[256];
						int uid = body.get_int("UserID");
						if(nick_dialog) PostMessage(nick_dialog, WMU_NICKEXISTS, (WPARAM)(uid != 0), msg.get_int("rid"));
						if(uid != 0) {
							MYPROTOSEARCHRESULT mpsr = {sizeof(mpsr)};

							if(body.get_string("UserName", nick, 256)) {
								mpsr.psr.nick = nick;
							} else if(body.get_string("DisplayName", nick, 256)) {
								mpsr.psr.nick = nick;
							}
							if(body.get_string("Email", email, 256))
								mpsr.psr.email = email;
							mpsr.uid = uid;

							ProtoBroadcastAck(MODULE, 0, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)req, (LPARAM)&mpsr);
						}
						ProtoBroadcastAck(MODULE, 0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)req, 0);
					} else
					if(cmd == 1 && dsn == 4 && lid == 3) { // userinfo(lookup by userid)
						Dictionary body = msg.get_dict("body");
						char errmsg[256];
						if(body.get_string("ErrorMessage", errmsg, 256)) {
							PUShowMessage(errmsg, SM_WARNING);
							ProtoBroadcastAck(MODULE, 0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)req, 0);
							ProtoBroadcastAck(MODULE, 0, ACKTYPE_GETINFO, ACKRESULT_FAILED, (HANDLE)0, 0);
						} else {
							char email[256], nick[256];
							int uid = body.get_int("UserID");
							if(uid != 0) {
								MYPROTOSEARCHRESULT mpsr = {sizeof(mpsr)};

								if(body.get_string("DisplayName", nick, 256)) {
									mpsr.psr.nick = nick;
								} else if(body.get_string("UserName", nick, 256)) {
									mpsr.psr.nick = nick;
								}

								if(body.get_string("Email", email, 256))
									mpsr.psr.email = email;
								mpsr.uid = uid;

								HANDLE hContact = 0;
								hContact = FindContact(uid);
								if(hContact || uid == my_uid) {
									if(mpsr.psr.nick) {
										DBWriteContactSettingStringUtf(hContact, MODULE, "Nick", mpsr.psr.nick);
										if(uid == my_uid) DBWriteContactSettingStringUtf(0, MODULE, "Nick", mpsr.psr.nick);
									}
									if(mpsr.psr.email) {
										DBWriteContactSettingStringUtf(hContact, MODULE, "email", mpsr.psr.email);
										if(uid == my_uid) DBWriteContactSettingStringUtf(0, MODULE, "email", mpsr.psr.email);
									}

									char band[256], song[256];
									band[0] = song[0] = 0;
									for(LinkedList<KeyValue>::Iterator i = body.start(); i.has_val(); i.next()) {
										KeyValue &kv = i.val();
										if(strcmp(kv.first.text, "Gender") == 0) {
											DBWriteContactSettingByte(hContact, MODULE, "Gender", kv.second.text[0]);
											if(uid == my_uid) DBWriteContactSettingByte(0, MODULE, "Gender", kv.second.text[0]);
										} else if(strcmp(kv.first.text, "ImageURL") == 0) {
											DBVARIANT dbv;
											if(!DBGetContactSettingStringUtf(hContact, MODULE, "ImageURL", &dbv)) {
												if(strcmp(kv.second.text, dbv.pszVal) != 0) {
													// avatar changed
													DBVARIANT dbv2;
													if(!DBGetContactSettingStringUtf(hContact, MODULE, "AvatarFilename", &dbv2)) {
														DeleteFileA(dbv2.pszVal);
														DBFreeVariant(&dbv2);
														DBDeleteContactSetting(hContact, MODULE, "AvatarFilename");
													}
													DBWriteContactSettingStringUtf(hContact, MODULE, kv.first.text, kv.second.text);
													DownloadAvatar(hContact, kv.second.text);
												}
												DBFreeVariant(&dbv);
											}else {
												DBWriteContactSettingStringUtf(hContact, MODULE, kv.first.text, kv.second.text);
												DownloadAvatar(hContact, kv.second.text);
											}
											if(uid == my_uid) { // same again for null hContact
												if(!DBGetContactSettingStringUtf(0, MODULE, "ImageURL", &dbv)) {
													if(strcmp(kv.second.text, dbv.pszVal) != 0) {
														// avatar changed
														DBVARIANT dbv2;
														if(!DBGetContactSettingStringUtf(0, MODULE, "AvatarFilename", &dbv2)) {
															DeleteFileA(dbv2.pszVal);
															DBFreeVariant(&dbv2);
															DBDeleteContactSetting(0, MODULE, "AvatarFilename");
														}
														DBWriteContactSettingStringUtf(0, MODULE, kv.first.text, kv.second.text);
														DownloadAvatar(0, kv.second.text);
													}
													DBFreeVariant(&dbv);
												}else {
													DBWriteContactSettingStringUtf(0, MODULE, kv.first.text, kv.second.text);
													DownloadAvatar(0, kv.second.text);
												}
											}
										} else {
											DBWriteContactSettingStringUtf(hContact, MODULE, kv.first.text, kv.second.text);
											if(uid == my_uid) DBWriteContactSettingStringUtf(0, MODULE, kv.first.text, kv.second.text);
											if(strcmp(kv.first.text, "BandName") == 0) strncpy(band, kv.second.text, 256);
											if(strcmp(kv.first.text, "SongName") == 0) strncpy(song, kv.second.text, 256);
										}
									}
									if(band[0] && song[0]) {
										char listenTo[512];
										mir_snprintf(listenTo, 512, "%s/%s", band, song);
										DBWriteContactSettingStringUtf(hContact, MODULE, "ListeningTo", listenTo);
										if(uid == my_uid) DBWriteContactSettingStringUtf(0, MODULE, "ListeningTo", listenTo);
									}

									ProtoBroadcastAck(MODULE, hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, (HANDLE)1, 0);
								}

								ProtoBroadcastAck(MODULE, 0, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)req, (LPARAM)&mpsr);
							}
							ProtoBroadcastAck(MODULE, 0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)req, 0);
						}
					} else
					if(cmd == 2 && dsn == 0 && lid == 9) {
						Dictionary body = msg.get_dict("body");
						char errmsg[256];
						if(body.get_string("ErrorMessage", errmsg, 256)) {
							PUShowMessage(errmsg, SM_WARNING);
						} else {
							char nick[256];
							int uid = body.get_int("ContactID");
							if(uid != 0) {
								if(body.get_string("NickName", nick, 256)) {
									DBWriteContactSettingStringUtf(0, MODULE, "Nick", nick);
								}
							}
						}
					} else
					if(cmd == 2 && dsn == 9 && lid == 14) {
						Dictionary body = msg.get_dict("body");
						char errmsg[256];
						if(body.get_string("ErrorMessage", errmsg, 256)) {
							PUShowMessage(errmsg, SM_WARNING);
						} else {
							int code = body.get_int("Code");
							if(nick_dialog) PostMessage(nick_dialog, WMU_CHANGEOK, (WPARAM)(code == 0), msg.get_int("rid"));						
						}
					} else 
					if(cmd == 1 && dsn == 7 && lid == 18) { // mail info?
						Dictionary body = msg.get_dict("body");
						char b[10];
						if(body.get_string("BlogComment", b, 10)) {
							NotifyBlogComment();
						}
						if(body.get_string("Mail", b, 10)) {
							NotifyMail();
						}
						if(body.get_string("ProfileComment", b, 10)) {
							NotifyProfileComment();
						}
						if(body.get_string("FriendRequest", b, 10)) {
							NotifyFriendRequest();
						}
						if(body.get_string("PictureComment", b, 10)) {
							NotifyPictureComment();
						}
						if(body.get_string("BlogSubscriptionPost", b, 10)) {
							NotifyBlogSubscriptPost();
						}
						if(DBGetContactSettingByte(0, MODULE, "UnknownNotify", 1)) {
							char pmsg[512];
							for(LinkedList<KeyValue>::Iterator i = body.start(); i.has_val(); i.next()) {
								KeyValue &kv = i.val();
								if(
									strcmp(kv.first.text, "BlogComment") != 0
									&& strcmp(kv.first.text, "Mail") != 0
									&& strcmp(kv.first.text, "ProfileComment") != 0
									&& strcmp(kv.first.text, "FriendRequest") != 0
									&& strcmp(kv.first.text, "PictureComment") != 0
									&& strcmp(kv.first.text, "BlogSubscriptionPost") != 0
									&& strcmp(kv.first.text, "Â€") != 0)
								{
									NotifyUnknown(kv.first.text, kv.second.text);
								}
							}
						}
					}
				}
				buffer_bytes -= (end - pbuff);
				memmove(recv_buffer, end, buffer_bytes);
				pbuff = recv_buffer; 
			}
		}
	}
	if(server_connection) {
		ClientNetMessage msg;
		msg.add_string("logout", "");
		msg.add_int("sesskey", sesskey);
		sesskey = 0;
		SendMessage(msg);
		Netlib_CloseHandle(server_connection);
		server_connection = 0;
	}
	delete recv_buffer;

	QueueUserAPC(sttMainThreadStatusCallback, mainThread, ID_STATUS_OFFLINE);	
	myspace_server_running = false;
	SetAllOffline();

	DeleteCriticalSection(&write_cs);
}

void StartThread() {
	if(!myspace_server_running) {
		server_stop = false;
		mir_forkthread(ServerThreadFunc, 0);
	}
}

void StopThread() {
	if(myspace_server_running) {
		if(sesskey) {
			ClientNetMessage msg;
			msg.add_string("logout", "");
			msg.add_int("sesskey", sesskey);
			SendMessage(msg);
		} else {
			if(server_connection) {
				Netlib_CloseHandle(server_connection);
				server_connection = 0;
			}
		}
	}
}

void SetServerStatus(int st) {
	if(st == ID_STATUS_OFFLINE) {
		StopThread();
	} else {
		st = stat_mir_to_mir(st);
		if(myspace_server_running && sesskey) {
			// set status
			ClientNetMessage msg_status;
			msg_status.add_int("status", stat_mir_to_myspace(st));
			msg_status.add_int("sesskey", sesskey);
			msg_status.add_string("statstring", "");
			msg_status.add_string("locstring", "");
			SendMessage(msg_status);

			QueueUserAPC(sttMainThreadStatusCallback, mainThread, st);
		} else {
			signon_status = st;
			StartThread();
		}
	}
}

void CALLBACK sttMainThreadStatusMessageCallback( ULONG dwParam ) {
	char *msg = (char *)dwParam;
	if(status != ID_STATUS_OFFLINE) {
		if(myspace_server_running && sesskey) {
			// set status

			ClientNetMessage msg_status;
			msg_status.add_int("status", stat_mir_to_myspace(status));
			msg_status.add_int("sesskey", sesskey);
			if(msg && msg[0]) {
				char buff[512];
				strncpy(buff, msg, 512);
				msg_status.add_string("statstring", buff);
			} else {
				msg_status.add_string("statstring", "");
			}
			msg_status.add_string("locstring", "");
			SendMessage(msg_status);
		} else {
			if(msg) strncpy(signon_status_msg, msg, 512);
			else signon_status_msg[0] = 0;
		}
	}
	if(msg) free(msg);
}

void SetServerStatusMessage(char *msg) {
	QueueUserAPC(sttMainThreadStatusMessageCallback, mainThread, (ULONG_PTR)(msg ? strdup(msg): 0));
}

int NetlibHttpRecvChunkHeader(HANDLE hConnection, BOOL first)
{
	char data[32], *peol1;

	int recvResult = Netlib_Recv(hConnection, data, 31, MSG_PEEK);
	data[recvResult] = 0;

	peol1 = strstr(data, "\r\n");
	if (peol1 != NULL)
	{
		char *peol2 = first ? peol1 : strstr(peol1 + 2, "\r\n");
		if (peol2 != NULL)
		{
			Netlib_Recv(hConnection, data, peol2 - data + 2, 0);
			return atoi(first ? data : peol1+2);
		}
	}

	return SOCKET_ERROR;
}

NETLIBHTTPREQUEST* NetlibHttpRecv(HANDLE hConnection, DWORD hflags, DWORD dflags)
{
	int dataLen = -1, i, chunkhdr;
	int chunked = FALSE;

	NETLIBHTTPREQUEST *nlhrReply = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_RECVHTTPHEADERS, (WPARAM)hConnection, hflags);
	if (nlhrReply==NULL) 
		return NULL;

	for(i=0;i<nlhrReply->headersCount;i++) 
	{
		if(!lstrcmpiA(nlhrReply->headers[i].szName, "Content-Length")) 
			dataLen = atoi(nlhrReply->headers[i].szValue);

		if(!lstrcmpiA(nlhrReply->headers[i].szName, "Transfer-Encoding") && 
			!lstrcmpiA(nlhrReply->headers[i].szValue, "chunked"))
		{
			chunked = TRUE;
			chunkhdr = i;
			break;
		}
	}

	if (nlhrReply->resultCode >= 200 && dataLen != 0)
	{
		int recvResult, chunksz = 0;
		int dataBufferAlloced = dataLen + 1;

		if (chunked)
		{
			chunksz = NetlibHttpRecvChunkHeader(hConnection, TRUE);
			if (chunksz == SOCKET_ERROR) 
			{
				CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)nlhrReply);
				return NULL;
			}
			dataLen = dataBufferAlloced = chunksz;
		}

		nlhrReply->pData = (char *)mir_realloc(nlhrReply->pData, dataBufferAlloced);

		do {
			for(;;) {
				if(dataBufferAlloced-nlhrReply->dataLength<1024 && dataLen == -1) {
					dataBufferAlloced+=2048;
					nlhrReply->pData=(char *)mir_realloc(nlhrReply->pData,dataBufferAlloced);
					if(nlhrReply->pData==NULL) {
						SetLastError(ERROR_OUTOFMEMORY);
						CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0,(LPARAM)nlhrReply);
						return NULL;
					}
				}
				recvResult=Netlib_Recv(hConnection,nlhrReply->pData+nlhrReply->dataLength,
					dataBufferAlloced-nlhrReply->dataLength-1, dflags);

				if(recvResult==0) break;
				if(recvResult==SOCKET_ERROR) {
					CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0,(LPARAM)nlhrReply);
					return NULL;
				}
				nlhrReply->dataLength += recvResult;

				if (dataLen > -1 && nlhrReply->dataLength >= dataLen)
					break;
			}

			if (chunked)
			{
				chunksz = NetlibHttpRecvChunkHeader(hConnection, FALSE);
				if (chunksz == SOCKET_ERROR) 
				{
					CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)nlhrReply);
					return NULL;
				}
				dataLen += chunksz;
				dataBufferAlloced += chunksz;

				nlhrReply->pData = (char *)mir_realloc(nlhrReply->pData, dataBufferAlloced);
			}
		} while (chunksz != 0);

		nlhrReply->pData[nlhrReply->dataLength]='\0';
	}

	if (chunked)
	{
		nlhrReply->headers[chunkhdr].szName = (char *)mir_realloc(nlhrReply->headers[chunkhdr].szName, 16);
		lstrcpyA(nlhrReply->headers[chunkhdr].szName, "Content-Length");

		nlhrReply->headers[chunkhdr].szValue = (char *)mir_realloc(nlhrReply->headers[chunkhdr].szValue, 16);
		mir_snprintf(nlhrReply->headers[chunkhdr].szValue, 16, "%u", nlhrReply->dataLength);
	}

	return nlhrReply;
}

typedef struct {
	HANDLE hContact;
	char *url;
} av_dl_info;

void __cdecl sttDownloadAvatar(void *param) {
	av_dl_info *info = (av_dl_info *)param;

	NETLIBHTTPREQUEST req = {0};
	req.cbSize = sizeof(req);
	req.requestType = REQUEST_GET;
	req.szUrl = info->url;
	req.flags = 0;
	NETLIBHTTPREQUEST *nlhrReply = 0;	
	int dlflags = 0;

	ProtoBroadcastAck(MODULE,info->hContact, ACKTYPE_AVATAR, ACKRESULT_CONNECTING, 0, 0);

	HANDLE hConnection = 0;
	{
		NETLIBOPENCONNECTION nloc={0};
		char szHost[128];
		char *ppath,*phost,*pcolon;

		phost=strstr(info->url,"://");
		if(phost==NULL) phost=info->url;
		else phost+=3;
		lstrcpynA(szHost,phost,sizeof(szHost));
		ppath=strchr(szHost,'/');
		if(ppath) *ppath='\0';
		nloc.cbSize=sizeof(nloc);
		nloc.szHost=szHost;
		pcolon=strrchr(szHost,':');
		if(pcolon) {
			*pcolon='\0';
			nloc.wPort=(WORD)atoi(pcolon+1);
		}
		else nloc.wPort=80;
		nloc.flags=NLOCF_HTTP;
		hConnection=(HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)hNetlibUser,(LPARAM)&nloc);
		if(hConnection==NULL) {
			ProtoBroadcastAck(MODULE,info->hContact,ACKTYPE_AVATAR,ACKRESULT_FAILED,0, 0);
			goto free_and_exit;
		}
	}
	ProtoBroadcastAck(MODULE,info->hContact,ACKTYPE_AVATAR,ACKRESULT_CONNECTED, 0, 0);

	if(!CallService(MS_NETLIB_SENDHTTPREQUEST, (WPARAM)hConnection, (LPARAM)&req)) {
		ProtoBroadcastAck(MODULE,info->hContact,ACKTYPE_AVATAR,ACKRESULT_FAILED,0, 0);
		goto free_and_exit;
	}

	ProtoBroadcastAck(MODULE,info->hContact,ACKTYPE_AVATAR,ACKRESULT_SENTREQUEST, 0, 0);

	dlflags = (req.flags&NLHRF_DUMPASTEXT?MSG_DUMPASTEXT:0) |
		(req.flags&NLHRF_NODUMP?MSG_NODUMP:(req.flags&NLHRF_DUMPPROXY?MSG_DUMPPROXY:0));

	nlhrReply = NetlibHttpRecv(hConnection, 0, dlflags);

	Netlib_CloseHandle(hConnection);

	if(nlhrReply) {
		ProtoBroadcastAck(MODULE,info->hContact,ACKTYPE_AVATAR,ACKRESULT_DATA, 0, 0);

		char *ext = strrchr(info->url, '.');
		if(ext) ext++;

		// write file
		int uid = DBGetContactSettingDword(info->hContact, MODULE, "UID", 0);

		char fn[MAX_PATH];
		char buff[128], tbuff[128];
		mir_snprintf(fn, MAX_PATH, "%s_%s.%s", _itoa(uid, buff, 10), _itoa(GetTickCount(), tbuff, 10), ext ? ext : "");

		if(WriteData(fn, MAX_PATH, nlhrReply->pData, nlhrReply->dataLength)) {
			CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)nlhrReply);
			DBWriteContactSettingStringUtf(info->hContact, MODULE, "AvatarFilename", fn);

			PROTO_AVATAR_INFORMATION AI = {0};
			AI.cbSize = sizeof(PROTO_AVATAR_INFORMATION);
			AI.hContact = info->hContact;
			strncpy(AI.filename, fn, MAX_PATH);
			if(ext) {
				AI.format = 
					(stricmp(ext, ".png") ? PA_FORMAT_PNG :
					(stricmp(ext, ".jpg") ? PA_FORMAT_JPEG :
					(stricmp(ext, ".jpeg") ? PA_FORMAT_JPEG :
					(stricmp(ext, ".gif") ? PA_FORMAT_GIF :
					(stricmp(ext, ".swf") ? PA_FORMAT_SWF : PA_FORMAT_UNKNOWN)))));
			} else
				AI.format = PA_FORMAT_UNKNOWN;

			ProtoBroadcastAck(MODULE,info->hContact,ACKTYPE_AVATAR,ACKRESULT_SUCCESS,(HANDLE) &AI, 0);
			if(!info->hContact)
				CallService(MS_AV_REPORTMYAVATARCHANGED, (WPARAM)MODULE, 0);
		} else 
			ProtoBroadcastAck(MODULE,info->hContact,ACKTYPE_AVATAR,ACKRESULT_FAILED, 0, 0);
	} else
		ProtoBroadcastAck(MODULE,info->hContact,ACKTYPE_AVATAR,ACKRESULT_FAILED, 0, 0);

free_and_exit:
	free(info->url);
	delete info;
}

void DownloadAvatar(HANDLE hContact, char *url) {
	av_dl_info *info = new av_dl_info;
	info->hContact = hContact;
	info->url = strdup(url);

	mir_forkthread(sttDownloadAvatar, info);
}