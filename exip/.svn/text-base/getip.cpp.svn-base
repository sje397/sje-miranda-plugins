#include "getip.h"

const char* data_prefix =			"Current IP Address: ";
const int data_prefix_len =			20;

bool RetreiveIP() {
	NETLIBHTTPREQUEST *resp = 0;
	NETLIBHTTPREQUEST req = {0};

	req.cbSize = sizeof(req);
	req.requestType = REQUEST_GET;
	req.szUrl = "http://checkip.dyndns.org";
	req.flags = NLHRF_DUMPASTEXT;

	resp = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)hNetlibUser, (LPARAM)&req);

	if(!resp) {
		DBWriteContactSettingString(0, MODULE, "ExternalIP", Translate("Unknown"));
		return false;
	}

	if(resp->resultCode != 200) {
		DBWriteContactSettingString(0, MODULE, "ExternalIP", Translate("Unknown"));
		return false;
	}

	int i1 = 0, i2 = 0;
	char *ip = 0;
	while(i1 < resp->dataLength) {
		while(i1 + i2 < resp->dataLength && resp->pData[i1 + i2] == data_prefix[i2] && i2 < data_prefix_len) i2++;
		if(i2 == data_prefix_len) {
			// found prefix
			i1 += i2;
			ip = &resp->pData[i1];

			i2 = 0;
			while(resp->pData[i1 + i2] != '<' && i1 + i2 < resp->dataLength) i2++;
			if(resp->pData[i1 + i2] == '<') resp->pData[i1 + i2] = 0;

			// ip now points to ip address
			DBWriteContactSettingString(0, MODULE, "ExternalIP", ip);
			break;
		}
		i1++;
	}

	if(!ip) DBWriteContactSettingString(0, MODULE, "ExternalIP", Translate("Unknown"));

	CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);

	return true;
}