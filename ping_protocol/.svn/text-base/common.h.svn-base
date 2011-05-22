#ifndef _COMMON_H
#define _COMMON_H

#define MAX_HISTORY		(1440)		// 12 hrs at 30 sec intervals

#define PROTO	"PING"

#define DEFAULT_PING_PERIOD				30
#define DEFAULT_PING_TIMEOUT			2
#define DEFAULT_SHOW_POPUP				false
#define DEFAULT_SHOW_POPUP2				false
#define DEFAULT_BLOCK_REPS				true
#define DEFAULT_LOGGING_ENABLED			false
#define DEFAULT_LOG_FILENAME			"ping_log.txt"
#define DEFAULT_NO_TEST_STATUS			true
#define DEFAULT_HIDE_PROTOCOL			false
#define DEFAULT_USE_STATUS_MESSAGE		false


#define MAX_PINGADDRESS_STRING_LENGTH	256

#include <stdio.h>

#include "../../include/newpluginapi.h"
#include "../../include/statusmodes.h"
#include "../../include/m_options.h"
#include "../../include/m_langpack.h"
#include "../../include/m_popup.h"
#include "../../include/m_system.h"
#include "../../include/m_skin.h"
#include "../../include/m_netlib.h"
#include "../../include/m_database.h"
#include "../../include/m_protocols.h"
#include "../../include/m_protomod.h"
#include "../../include/m_protosvc.h"
#include "../../include/m_ignore.h"
#include "../../include/m_clist.h"
#include "../../include/m_clui.h"
#include "../../include/m_utils.h"

#include "../updater/m_updater.h"

typedef struct {
	int ping_period, ping_timeout;
	bool show_popup, show_popup2, block_reps, logging;
	char log_filename[MAX_PATH];
	WORD rstatus, nrstatus, tstatus, off_status;
	bool no_test_status;
	bool hide_proto;
	int retries;
	bool use_status_msg;
} PingOptions;

#include <vector>
// deque of pairs - round trip time and timestamp
typedef std::vector<std::pair<short, DWORD> > HistoryList;

typedef struct PINGADDRESS_tag {
	
	/*
	PINGADDRESS_tag() {
		pszName[0] = 0;
		pszLabel[0] = 0;
		pszProto[0] = 0;
		pszCommand[0] = 0;
		pszParams[0] = 0;
	}

	PINGADDRESS_tag(const PINGADDRESS_tag &src) {
	}
	*/
	const PINGADDRESS_tag operator=(const PINGADDRESS_tag &src) {
		cbSize = src.cbSize;
		hContact = src.hContact;
		strcpy(pszName, src.pszName);
		strcpy(pszLabel, src.pszLabel);
		responding = src.responding;
		status = src.status;
		round_trip_time = src.round_trip_time;
		miss_count = src.miss_count;
		port = src.port;
		strcpy(pszProto, src.pszProto);
		strcpy(pszCommand, src.pszCommand);
		strcpy(pszParams, src.pszParams);
		get_status = src.get_status;
		set_status = src.set_status;
		if(src.history.size())
			history = src.history;
		else
			history.clear();
		index = src.index;
		strcpy(pszCListGroup, src.pszCListGroup);

		return *this;
	}

	int cbSize;											//size in bytes of this structure
	HANDLE hContact;
	char pszName[MAX_PINGADDRESS_STRING_LENGTH];		//IP address or domain name
	char pszLabel[MAX_PINGADDRESS_STRING_LENGTH];
	bool responding;
	int status;
	short round_trip_time;
	int miss_count;
	int port; // -1 for ICMP, non-zero for TCP
	char pszProto[MAX_PINGADDRESS_STRING_LENGTH];
	char pszCommand[MAX_PATH];
	char pszParams[MAX_PATH];
	unsigned int get_status;		// on success, if status equals this
	unsigned int set_status;	// set it to this
	HistoryList history;
	int index;
	char pszCListGroup[MAX_PINGADDRESS_STRING_LENGTH];
} PINGADDRESS;

extern HANDLE hNetlibUser;
extern HINSTANCE hInst;

extern int previousMode;
extern int mcStatus;

#ifndef MIID_PING
#define MIID_PING	{0x9cd1684e, 0xc520, 0x4b58, { 0x9a, 0x52, 0xae, 0x3d, 0x7a, 0x72, 0x4, 0x46}}
#endif

#endif
