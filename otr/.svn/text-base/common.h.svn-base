#ifndef _COMMON_INC
#define _COMMON_INC

#define _WIN32_WINNT	 0x0500
#define WINVER			 0x0500
#define _WIN32_IE		 0x0300

#include <windows.h>
//#include <process.h>
//#include <shlobj.h>
#include <stdio.h>
#include <commctrl.h>
#include <malloc.h>
#include <process.h>

#include "resource.h"

#include <newpluginapi.h>
#include <statusmodes.h>
#include <m_options.h>
#include <m_langpack.h>
#include <m_system.h>
#include <m_skin.h>
#include <m_netlib.h>
#include <m_database.h>
#include <m_protocols.h>
#include <m_protomod.h>
#include <m_protosvc.h>
#include <m_ignore.h>
#include <m_clist.h>
#include <m_clui.h>
#include <m_utils.h>
#include <m_message.h>

#include <m_updater.h>
#include <m_metacontacts.h>

#include <m_yapp.h>
#include <m_icolib.h>
#include <m_folders.h>

#define MODULE		"OTR"

extern "C" {
#include "proto.h"
#include "message.h"
#include "privkey.h"
};

// modified manual policy - so that users set to 'opportunistic' will automatically start OTR with users set to 'manual'
#define OTRL_POLICY_MANUAL_MOD		(OTRL_POLICY_MANUAL | OTRL_POLICY_WHITESPACE_START_AKE | OTRL_POLICY_ERROR_START_AKE)

// use the same filenames as the gaim plugin, for compatibility
#define PRIVATE_KEY_FILENAME 			"otr.private_key"
#define FINGERPRINT_STORE_FILENAME 		"otr.fingerprints"


extern HINSTANCE hInst;
extern HANDLE mainThread;
extern DWORD mainThreadId;

extern OtrlUserState otr_user_state;
extern char private_key_filename[MAX_PATH];
extern char fingerprint_store_filename[MAX_PATH];

extern char *metaproto;

// read the files named above
void ReadPrivkeyFiles();

// get human readable fingerprint for a contact
bool GetFingerprint(HANDLE hContact, char buff[45]);

#define MS_OTR_MENUSTART		"OTR/Start"
#define MS_OTR_MENUSTOP			"OTR/Stop"

#define INLINE_PREFIX			"[OTR Message] "
#define INLINE_PREFIX_LEN		14

#ifndef MIID_OTR
#define MIID_OTR	{0xade1b67c, 0xc164, 0x4817, { 0x94, 0x50, 0xc8, 0x9d, 0xcf, 0x59, 0x4c, 0x4b}}
#endif

#endif
