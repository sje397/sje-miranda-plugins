#ifndef _COMMON_INC
#define _COMMON_INC

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#if defined( UNICODE ) && !defined( _UNICODE )
#define _UNICODE
#endif

#include <tchar.h>

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <commctrl.h>

#define MIRANDA_VER		0x0800

#include <newpluginapi.h>
#include <m_plugins.h>
#include <m_system.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_protomod.h>
#include <m_protosvc.h>
#include <m_clist.h>
#include <m_clc.h>
#include <m_clui.h>
#include <m_message.h>
#include <m_icolib.h>
#include <m_skin.h>
#include <m_utils.h>
#include <m_ignore.h>

#include <m_popup.h>
#include <m_updater.h>

#include "m_metacontacts.h"
#include "collection.h"

////////////
// included for backward compatibility
#ifndef CMIF_UNICODE
#define CMIF_UNICODE        512      //will return TCHAR* instead of char*
#if defined( _UNICODE )
	#define CMIF_TCHAR       CMIF_UNICODE      //will return TCHAR* instead of char*
#else
	#define CMIF_TCHAR       0       //will return char*, as usual
#endif
#endif
////////////

#define MODULE						"meta2"

extern HINSTANCE hInst;
extern PLUGINLINK *pluginLink;
extern MM_INTERFACE mmi;
extern UTF8_INTERFACE utfi;
extern HANDLE metaMainThread;
extern DWORD next_meta_id;
extern int codepage;

#ifndef MIID_META2
#define MIID_META2	{ 0x4415A85D, 0xD6DA, 0x4551, { 0xB2, 0xB8, 0x9B, 0xDD, 0x82, 0xE2, 0x4B, 0x50 } }
#endif

#define MAX_SUBCONTACTS		20

#define META_ID		"MetaID"

inline bool MetaEnabled() {
	return DBGetContactSettingByte(0, MODULE, "Enabled", 1) == 1;
}

inline bool IsMetacontact(HANDLE hContact) {
	return DBGetContactSettingDword(hContact, MODULE, META_ID, (DWORD)-1) != (DWORD)-1;
}

inline bool IsSubcontact(HANDLE hContact) {
	return DBGetContactSettingByte(hContact, MODULE, "IsSubcontact", 0) == 1;
}

inline char *ContactProto(HANDLE hContact) {
	return (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
}

inline TCHAR *ContactName(HANDLE hContact) {
	return (TCHAR *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR);
}

inline WORD ContactStatus(HANDLE hContact, char *proto) {
	if(!proto) return ID_STATUS_OFFLINE;
	return DBGetContactSettingWord(hContact, proto, "Status", ID_STATUS_OFFLINE);
}

class ContactHandle {
public:
	ContactHandle(const HANDLE &hCon): hContact(hCon) {}
	virtual ~ContactHandle() {}

	HANDLE handle() const {return hContact;}
	//operator HANDLE () const {return hContact;}

	bool operator==(const ContactHandle &other) const;
	bool operator<(const ContactHandle &other) const;
	const ContactHandle &operator=(HANDLE h) {hContact = h;}
protected:
	HANDLE hContact;
};

class SubcontactList: public SortedDynamicArray<ContactHandle> {
public:
	
	void add(HANDLE h) {
		SortedDynamicArray<ContactHandle>::add(ContactHandle(h));
	}

	void remove(HANDLE h) {
		SortedDynamicArray<ContactHandle>::remove(ContactHandle(h));
	}
	
};


#endif
