#pragma once

class UploadProgress;
class UploadSettings;
class UploadResults;

class Uploader
{
	UploadProgress *m_pProgress;
	UploadSettings *m_pSettings;
	UploadResults *m_pResults;

public:
	Uploader(UploadProgress *pProgress)
		: m_pProgress(pProgress), m_pSettings(NULL), m_pResults(NULL)
	{
	}

	HRESULT DoUpload(UploadSettings *pSettings, UploadResults *pResults);

private:
	TCHAR *GenerateMultipartBoundary();
	TCHAR *GenerateContentTypeHeader(const TCHAR *strBoundary);
	DWORD CalculateContentLength(const TCHAR *strBoundary, DWORD *pdwOverallBytesTotal);

	char *GenerateFieldHeader(const TCHAR *strBoundary, const TCHAR *lpszFieldName);
	DWORD CalculateContentLength(const TCHAR *strBoundary, const TCHAR *lpszFieldName, const TCHAR *lpszFieldValue);
	HRESULT UploadField(HINTERNET hRequest, const TCHAR *strBoundary, const TCHAR *lpszFieldName, const TCHAR *lpszFieldValue);
	const char *GenerateFieldTrailer();
	
	DWORD CalculateContentLength(const TCHAR *strBoundary, const TCHAR * lpszFieldName, const TCHAR * lpszPathName, const TCHAR * lpszContentType, DWORD *pdwOverallBytesTotal);
	char *GenerateFileHeader(const TCHAR * strBoundary, const TCHAR * lpszFieldName, const TCHAR * lpszPathName, const TCHAR * lpszContentType);
	HRESULT UploadFile(HINTERNET hRequest, const TCHAR * strBoundary, const TCHAR * lpszFieldName, const TCHAR * lpszPathName, const TCHAR * lpszContentType, DWORD *pdwOverallBytesSent, DWORD dwOverallBytesTotal);
	HRESULT UploadFileContent(HINTERNET hRequest, const TCHAR * lpszPathName, HANDLE hFile, DWORD *pdwOverallBytesSent, DWORD dwOverallBytesTotal);
	const char *GenerateFileTrailer();

	char *GenerateBodyTrailer(const TCHAR * strBoundary);

	HRESULT ReadStatusCode(HINTERNET hRequest, DWORD *pStatusCode);
	HRESULT ReadResponseHeaders(HINTERNET hRequest);
	HRESULT ReadResponseBody(HINTERNET hRequest);

private:
	static void CALLBACK StaticInternetStatusCallback(HINTERNET hInternet, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength);
	void InternetStatusCallback(HINTERNET hInternet, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength);
};
