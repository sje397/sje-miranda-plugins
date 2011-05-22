#include "stdafx.h"
#include "log.h"

int Log(WPARAM wParam, LPARAM lParam) {

	char buf[1024];
	CallService(PROTO "/GetLogFilename", (WPARAM)1024, (LPARAM)buf);

	char TBcapt[255];
	SYSTEMTIME systime;
	
	GetLocalTime(&systime);
	
	wsprintf(TBcapt,"%02d:%02d:%02d",systime.wHour,systime.wMinute,systime.wSecond);

	std::string timestring = TBcapt;

	std::string line = (char *)wParam;

	std::ofstream out(buf, std::ios_base::out | std::ios_base::app);
	if(out.is_open()) {
		out << timestring << ": " << line << std::endl;
		out.close();
	}

	return 0;
}

int GetLogFilename(WPARAM wParam, LPARAM lParam) {
	DBVARIANT dbv;
	std::string filename;
	if(DBGetContactSetting(0, PROTO, "LogFilename", &dbv)) {
		char buf[MAX_PATH];
		CallService(MS_DB_GETPROFILEPATH, (WPARAM)MAX_PATH, (LPARAM)buf);
		filename = buf;
		filename += "\\";
		filename += "ping_log.txt";
	} else {
		filename = dbv.pszVal;
	}

	strncpy((char *)lParam, filename.c_str(), (size_t)wParam);

	DBFreeVariant(&dbv);
	return 0;
}

int SetLogFilename(WPARAM wParam, LPARAM lParam) {
	DBWriteContactSettingString(0, PROTO, "LogFilename", (char *)lParam);
	return 0;
}

int ViewLogData(WPARAM wParam, LPARAM lParam) {
	char buf[1024];
	CallService(PROTO "/GetLogFilename", (WPARAM)MAX_PATH, (LPARAM)buf);
	return (int)ShellExecute((HWND)wParam, "edit", buf, "", "", SW_SHOW);	
}

