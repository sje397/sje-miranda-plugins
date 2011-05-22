// adapted 23/9/2004 from public domain code at http://tangentsoft.net/wskfaq/examples/dllping.html

#ifndef _ICMP_H
#define _ICMP_H

#include <windows.h>

// Structures required to use functions in ICMP.DLL

typedef struct {
    unsigned char Ttl;                         // Time To Live
    unsigned char Tos;                         // Type Of Service
    unsigned char Flags;                       // IP header flags
    unsigned char OptionsSize;                 // Size in bytes of options data
    unsigned char *OptionsData;                // Pointer to options data
} IP_OPTION_INFORMATION, * PIP_OPTION_INFORMATION;

typedef struct {
    DWORD Address;                             // Replying address
    unsigned long  Status;                     // Reply status
    unsigned long  RoundTripTime;              // RTT in milliseconds
    unsigned short DataSize;                   // Echo data size
    unsigned short Reserved;                   // Reserved for system use
    void *Data;                                // Pointer to the echo data
    IP_OPTION_INFORMATION Options;             // Reply options
	unsigned char ReplyData[8];
} IP_ECHO_REPLY, * PIP_ECHO_REPLY;


typedef HANDLE (WINAPI* pfnHV)(VOID);
typedef BOOL (WINAPI* pfnBH)(HANDLE);
typedef DWORD (WINAPI* pfnDHDPWPipPDD)(HANDLE, DWORD, LPVOID, WORD, PIP_OPTION_INFORMATION, LPVOID, DWORD, DWORD); // evil, no?

class ICMP {
protected:
	pfnHV pIcmpCreateFile;
    pfnBH pIcmpCloseHandle;
    pfnDHDPWPipPDD pIcmpSendEcho;
 	
	HMODULE hDLL;

	HANDLE hIP;

	unsigned int timeout;
	bool functions_loaded;

	// protected constructor - singleton class
	ICMP();
	static ICMP instance;

public:
	~ICMP();
	static ICMP *get_instance() {return &instance;}

	bool ping(char *host, IP_ECHO_REPLY &reply);
	
	void set_timeout(unsigned int t) {
		timeout = t;
	}

	void stop();

	unsigned int get_timeout() {return timeout;}

};
#endif //_ICMP_H
