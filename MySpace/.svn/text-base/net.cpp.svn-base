#include "common.h"
#include "net.h"

HANDLE hNetlibUser = 0;

void InitNetlib() {
	NETLIBUSER nl_user = {0};
	nl_user.cbSize = sizeof(nl_user);
	nl_user.szSettingsModule = MODULE;
	nl_user.flags = NUF_OUTGOING | NUF_HTTPCONNS;

	char buff[512];
	mir_snprintf(buff, 512, "%s connections", MODULE);
	nl_user.szDescriptiveName = buff;

	hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nl_user);
}

void DeinitNetlib() {
	if(hNetlibUser)
		CallService(MS_NETLIB_CLOSEHANDLE, (WPARAM)hNetlibUser, 0);
}

