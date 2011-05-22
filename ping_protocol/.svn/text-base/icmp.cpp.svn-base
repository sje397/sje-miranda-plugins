#include "stdafx.h"
#include "icmp.h"

ICMP ICMP::instance;

ICMP::ICMP():
	timeout(2000),
	functions_loaded(false)
{
	hDLL = LoadLibrary("ICMP.DLL");
	if(!hDLL) return;

	pIcmpCreateFile = (pfnHV)GetProcAddress(hDLL, "IcmpCreateFile");
	pIcmpCloseHandle = (pfnBH)GetProcAddress(hDLL, "IcmpCloseHandle");
	pIcmpSendEcho = (pfnDHDPWPipPDD)GetProcAddress(hDLL, "IcmpSendEcho");
	if ((pIcmpCreateFile == 0) || (pIcmpCloseHandle == 0) || (pIcmpSendEcho == 0)) {
		return;
	}

    WSAData wsaData;
    if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {
		WSACleanup();
		FreeLibrary((HMODULE)hDLL);
		return;
    }

	functions_loaded = true;
}

void ICMP::stop() {
	if(hIP) {
		pIcmpCloseHandle(hIP);
		hIP = 0;
	}
}

ICMP::~ICMP() {
	if(hIP) pIcmpCloseHandle(hIP);
	WSACleanup();
	if(hDLL)
		FreeLibrary(hDLL);	
}

bool ICMP::ping(char *host, IP_ECHO_REPLY &reply) {
	if(!functions_loaded) return false;

	DWORD address;
	struct in_addr dest;
	HOSTENT *rec;
	IP_OPTION_INFORMATION ipoi;

	hIP = pIcmpCreateFile();
	if (hIP == INVALID_HANDLE_VALUE) {
		return false;
	}
   

	dest.s_addr = inet_addr(host);
	if (dest.s_addr == INADDR_NONE) {
		rec = gethostbyname(host);
		if(rec) address = *(DWORD *)(*rec->h_addr_list);
		else return false;
	} else {
		address = dest.s_addr;
	}

	ipoi.Ttl = 255;
	ipoi.Tos = 0;
	ipoi.Flags = 0;
	ipoi.OptionsSize = 0;
	ipoi.OptionsData = 0;
	
	reply.Status = 0;	

	pIcmpSendEcho(hIP, address, (void *)"PINGPONG", 8, &ipoi, &reply, sizeof(reply), timeout);

	if(hIP) {
		pIcmpCloseHandle(hIP);
		hIP = 0;
	}

	return (reply.Status == 0);
}
