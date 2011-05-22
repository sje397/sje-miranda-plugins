#pragma once

#include "common.h"

class UploadResults
{
	HRESULT m_hResult;
	DWORD m_dwStatusCode;
	
	char *szResponse;
	int cbResponseLength;

public:
	UploadResults(): m_hResult(S_OK), m_dwStatusCode(-1), szResponse(0), cbResponseLength(0) {}
	~UploadResults() {if(szResponse) free(szResponse);}

	void SetResult(HRESULT hr) { m_hResult = hr; }
	HRESULT GetResult() const { return m_hResult; }

	void SetStatusCode(DWORD dwStatusCode) { m_dwStatusCode = dwStatusCode; }
	DWORD GetStatusCode() const { return m_dwStatusCode; }

	bool GotHttpResult() const { return GetStatusCode() != -1; }
	bool IsHttpSuccess() const { return ((m_dwStatusCode / 100) == 2); }

	void AppendResponse(char *szData, DWORD cbSize) {
		int oldSize = cbResponseLength;
		cbResponseLength += cbSize;
		szResponse = (char *)realloc(szResponse, cbResponseLength + 1); // one last 0 byte for luck
		memcpy(szResponse + oldSize, szData, cbSize);
		szResponse[cbResponseLength] = 0;
	}

	int GetResponseLength() {return cbResponseLength;}
	char *GetResponse() {return szResponse;}
};

#define E_USER_CANCELLED MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x301)
