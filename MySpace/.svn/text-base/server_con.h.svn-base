#ifndef _SERVER_CON_INC
#define _SERVER_CON_INC

#include "NetMessage.h"
#include "m_protosvc.h"

extern int status;
extern int sesskey;
extern int req_id;
extern bool myspace_server_running;
extern int my_uid;

void SetServerStatus(int st);
void SetServerStatusMessage(char *msg);

HANDLE FindContact(int uid);
HANDLE CreateContact(int uid, char *nick, char *email, bool add_buddy);
int LookupUID(int uid);
void AddBuddyOnServer(int uid);

void AddAllContactsToServer();

void SendMessage(ClientNetMessage &msg);
void DownloadAvatar(HANDLE hContact, char *url);

typedef struct {
	PROTOSEARCHRESULT psr;
	int uid;
} MYPROTOSEARCHRESULT;

#endif
