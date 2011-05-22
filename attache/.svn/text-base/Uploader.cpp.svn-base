#include "common.h"
#include "Uploader.h"
#include "UploadSettings.h"
#include "UploadResults.h"
#include "UploadProgress.h"

#include "str_utils.h"
#include "version.h"

#include <stdlib.h>

TCHAR * Uploader::GenerateMultipartBoundary()
{
	TCHAR *multipartBoundary = (TCHAR *)malloc(512 * sizeof(TCHAR));

	// We need 12 hex digits.
	int r0 = rand() & 0xffff;
	int r1 = rand() & 0xffff;
	int r2 = rand() & 0xffff;
	mir_sntprintf(multipartBoundary, 512, _T("---------------------------%04X%04X%04X"), r0, r1, r2);

	return multipartBoundary;
}

TCHAR * Uploader::GenerateContentTypeHeader(const TCHAR * strBoundary)
{
	TCHAR *contentTypeHeader = (TCHAR *)malloc(512 * sizeof(TCHAR));
	mir_sntprintf(contentTypeHeader, 512, _T("Content-Type: multipart/form-data; boundary=%s"), strBoundary);

	return contentTypeHeader;
}

char * Uploader::GenerateFileHeader(const TCHAR * strBoundary, const TCHAR * lpszFieldName, const TCHAR * lpszPathName, const TCHAR * lpszContentType)
{
	char *boundary = t2a(strBoundary);
	char *fieldName = t2a(lpszFieldName);
	char *pathName = t2a(lpszPathName);
	char *contentType = t2a(lpszContentType);

	char contentDispositionHeader[MAX_PATH];
	mir_snprintf(contentDispositionHeader, MAX_PATH, "Content-Disposition: form-data; name=\"%hs\"; filename=\"%hs\"", fieldName, pathName);

	char contentTypeHeader[512];
	mir_snprintf(contentTypeHeader, 512, "Content-Type: %hs", contentType);

	char *fileHeader = (char *)malloc(2048);
	mir_snprintf(fileHeader, 2048, "--%hs\r\n%s\r\n%s\r\n\r\n", boundary, contentDispositionHeader, contentTypeHeader);

	free(boundary);
	free(fieldName);
	free(pathName);
	free(contentType);

	return fileHeader;
}

const char *Uploader::GenerateFileTrailer()
{
	return "\r\n";
}

char *Uploader::GenerateBodyTrailer(const TCHAR *strBoundary)
{
	char *boundary = t2a(strBoundary);
	char *bodyTrailer = (char *)malloc(512);
	mir_snprintf(bodyTrailer, 512, "--%s--\r\n", boundary);
	free(boundary);
	return bodyTrailer;
}

char *Uploader::GenerateFieldHeader(const TCHAR *strBoundary, const TCHAR *lpszFieldName)
{
	char *boundary = t2a(strBoundary);
	char *fieldName = t2a(lpszFieldName);

	char contentDispositionHeader[512];
	mir_snprintf(contentDispositionHeader, 512, "Content-Disposition: form-data; name=\"%hs\"", fieldName);

	char *fieldHeader = (char *)malloc(1024);
	mir_snprintf(fieldHeader, 1024, "--%hs\r\n%s\r\n\r\n", boundary, contentDispositionHeader);

	free(boundary);
	free(fieldName);

	return fieldHeader;
}

const char *Uploader::GenerateFieldTrailer()
{
	return "\r\n";
}

DWORD Uploader::CalculateContentLength(const TCHAR * strBoundary, const TCHAR * lpszFieldName, const TCHAR * lpszFieldValue)
{
	char *fieldHeader = GenerateFieldHeader(strBoundary, lpszFieldName);
	const char *fieldTrailer = GenerateFieldTrailer();
	char *fieldValue = t2a(lpszFieldValue);

	DWORD contentLength = strlen(fieldHeader) + strlen(fieldValue) + strlen(fieldTrailer);
	
	free(fieldHeader);
	free(fieldValue);

	return contentLength;
}

DWORD Uploader::CalculateContentLength(const TCHAR * strBoundary, const TCHAR * lpszFieldName, const TCHAR * lpszPathName, const TCHAR * lpszContentType, DWORD *pdwOverallBytesTotal)
{
	char *fileHeader = GenerateFileHeader(strBoundary, lpszFieldName, lpszPathName, lpszContentType);

	HANDLE hFile = CreateFile(lpszPathName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	DWORD fileLength = GetFileSize(hFile, NULL);
	CloseHandle(hFile);

	*pdwOverallBytesTotal += fileLength;

	const char *fileTrailer = GenerateFileTrailer();

	DWORD contentLength = strlen(fileHeader) + fileLength + strlen(fileTrailer);
	free(fileHeader);
	return contentLength;
}

DWORD Uploader::CalculateContentLength(const TCHAR * strBoundary, DWORD *pdwOverallBytesTotal)
{
	DWORD contentLength = 0;
	*pdwOverallBytesTotal = 0;

	FieldInfoList fields;
	m_pSettings->GetFields(&fields);

	for(FieldInfoList::ListNode *n = fields.start(); n; n = n->next)
		contentLength += CalculateContentLength(strBoundary, n->info.GetFieldName(), n->info.GetFieldValue());

	FileInfoList files;
	m_pSettings->GetFiles(&files);

	for(FileInfoList::ListNode *n = files.start(); n; n = n->next)
		contentLength += CalculateContentLength(strBoundary, n->info.GetFieldName(), n->info.GetPathName(), n->info.GetContentType(), pdwOverallBytesTotal);

	const char *bodyTrailer = GenerateBodyTrailer(strBoundary);
	contentLength += strlen(bodyTrailer);

	return contentLength;
}

HRESULT Uploader::DoUpload(UploadSettings *pSettings, UploadResults *pResults) {
	m_pSettings = pSettings;
	m_pResults = pResults;

	const TCHAR *lpszAgent = _T(MODULE);
	DWORD dwOpenInternetFlags = 0;//INTERNET_FLAG_ASYNC;
	HINTERNET hInternet = InternetOpen(lpszAgent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, dwOpenInternetFlags);
	if(!hInternet) {
		return HRESULT_FROM_WIN32(GetLastError());
	}

	InternetSetStatusCallback(hInternet, StaticInternetStatusCallback);

	INTERNET_PORT nServerPort = INTERNET_DEFAULT_HTTP_PORT;
	LPCTSTR lpszUserName = NULL;
	LPCTSTR lpszPassword = NULL;
	DWORD dwConnectFlags = 0;
	DWORD_PTR dwConnectContext = reinterpret_cast<DWORD_PTR>(this);
	HINTERNET hConnect = InternetConnect(hInternet, m_pSettings->GetHostName(), nServerPort,
											lpszUserName, lpszPassword,
											INTERNET_SERVICE_HTTP,
											dwConnectFlags, dwConnectContext);
	if(!hConnect) {
		InternetCloseHandle(hInternet);
		//TRACE("InternetConnect failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
		return HRESULT_FROM_WIN32(GetLastError());
	}

	LPCTSTR lpszVerb = _T("POST");
	LPCTSTR lpszVersion = NULL;			// Use default.
	LPCTSTR lpszReferrer = NULL;		// No referrer.
	LPCTSTR *lplpszAcceptTypes = NULL;	// Whatever the server wants to give us.
	DWORD dwOpenRequestFlags = INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP |
								INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS | 
								INTERNET_FLAG_KEEP_CONNECTION |
								INTERNET_FLAG_NO_AUTH |
								//INTERNET_FLAG_NO_AUTO_REDIRECT |
								INTERNET_FLAG_NO_COOKIES |
								INTERNET_FLAG_NO_UI |
								INTERNET_FLAG_RELOAD;
	DWORD_PTR dwOpenRequestContext = reinterpret_cast<DWORD_PTR>(this);
	HINTERNET hRequest = HttpOpenRequest(hConnect, lpszVerb, m_pSettings->GetUrlPath(), lpszVersion,
											lpszReferrer, lplpszAcceptTypes,
											dwOpenRequestFlags, dwOpenRequestContext);
	if(!hRequest) {
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
		return HRESULT_FROM_WIN32(GetLastError());
	}

	TCHAR *strBoundary = GenerateMultipartBoundary();
	TCHAR *contentTypeHeader = GenerateContentTypeHeader(strBoundary);
	BOOL bResult = HttpAddRequestHeaders(hRequest, contentTypeHeader, -1, HTTP_ADDREQ_FLAG_ADD);
	free(contentTypeHeader);
	if(!bResult) {
		free(strBoundary);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
		return HRESULT_FROM_WIN32(GetLastError());
	}

	DWORD dwOverallBytesTotal = 0;
	DWORD contentLength = CalculateContentLength(strBoundary, &dwOverallBytesTotal);

	INTERNET_BUFFERS buffersIn;
	memset(&buffersIn, 0, sizeof(INTERNET_BUFFERS));
	buffersIn.dwStructSize = sizeof(INTERNET_BUFFERS);
	buffersIn.dwBufferTotal = contentLength;

	bResult = HttpSendRequestEx(hRequest, &buffersIn, NULL, HSR_INITIATE, reinterpret_cast<DWORD_PTR>(this));
	if(!bResult) {
		// If GetLastError() == 6 (ERROR_INVALID_HANDLE), then this often means that
		// the server wasn't actually up.  Unfortunately, I don't know of a better
		// way to get more information.
		free(strBoundary);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
		return HRESULT_FROM_WIN32(GetLastError());
	}

	FieldInfoList fields;
	m_pSettings->GetFields(&fields);

	for(FieldInfoList::ListNode *n = fields.start(); n; n = n->next) {
		HRESULT hr = UploadField(hRequest, strBoundary, n->info.GetFieldName(), n->info.GetFieldValue());
		if(FAILED(hr)) {
			free(strBoundary);
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hInternet);
			return hr;
		}
	}

	FileInfoList files;
	m_pSettings->GetFiles(&files);

	DWORD dwOverallBytesSent = 0;
	for (FileInfoList::ListNode *n = files.start(); n; n = n->next) {
		HRESULT hr = UploadFile(hRequest, strBoundary, n->info.GetFieldName(), n->info.GetPathName(), n->info.GetContentType(), &dwOverallBytesSent, dwOverallBytesTotal);
		if(FAILED(hr)) {
			free(strBoundary);
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hInternet);
			return hr;
		}
	}

	// After the last file:
	char *bodyTrailer = GenerateBodyTrailer(strBoundary);
	DWORD dwBytesWritten;
	bResult = InternetWriteFile(hRequest, bodyTrailer, strlen(bodyTrailer), &dwBytesWritten);
	free(bodyTrailer);
	if(!bResult) {
		free(strBoundary);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
		return HRESULT_FROM_WIN32(GetLastError());
	}

	free(strBoundary);

	bResult = HttpEndRequest(hRequest, NULL, 0, reinterpret_cast<DWORD_PTR>(this));
	if (!bResult) {
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
		// TODO: return S_OK anyway?  It's probably too late to do anything about it now.
		return HRESULT_FROM_WIN32(GetLastError());
	}

	DWORD dwStatusCode;
	ReadStatusCode(hRequest, &dwStatusCode);
	m_pResults->SetStatusCode(dwStatusCode);

	ReadResponseHeaders(hRequest);
	ReadResponseBody(hRequest);

	InternetCloseHandle(hRequest);
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hInternet);

	return S_OK;
}

HRESULT Uploader::ReadStatusCode(HINTERNET hRequest, DWORD *pStatusCode)
{
	DWORD dwStatusCodeSize = sizeof(DWORD);
	DWORD dwStatusCode;
	if (!HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &dwStatusCode, &dwStatusCodeSize, NULL))
		return HRESULT_FROM_WIN32(GetLastError());

	*pStatusCode = dwStatusCode;
	return S_OK;
}

HRESULT Uploader::ReadResponseHeaders(HINTERNET hRequest)
{
	DWORD dwInfoLevel = HTTP_QUERY_RAW_HEADERS_CRLF;
	DWORD dwInfoBufferLength = 10;
	BYTE *pInfoBuffer = (BYTE *)malloc(dwInfoBufferLength+1);
	while (!HttpQueryInfo(hRequest, dwInfoLevel, pInfoBuffer, &dwInfoBufferLength, NULL))
	{
		DWORD dwError = GetLastError();
		if (dwError == ERROR_INSUFFICIENT_BUFFER)
		{
			free(pInfoBuffer);
			pInfoBuffer = (BYTE *)malloc(dwInfoBufferLength+1);
		}
		else
		{
			return HRESULT_FROM_WIN32(GetLastError());
		}
	}

	pInfoBuffer[dwInfoBufferLength] = '\0';
	//TRACE("%s", pInfoBuffer);
	free(pInfoBuffer);

	return S_OK;
}

HRESULT Uploader::ReadResponseBody(HINTERNET hRequest)
{
	DWORD dwBytesAvailable;
	while (InternetQueryDataAvailable(hRequest, &dwBytesAvailable, 0, 0))
	{
		BYTE *pMessageBody = (BYTE *)malloc(dwBytesAvailable+1);

		DWORD dwBytesRead;
		BOOL bResult = InternetReadFile(hRequest, pMessageBody, dwBytesAvailable, &dwBytesRead);
		if (!bResult)
		{
			free(pMessageBody);
			return HRESULT_FROM_WIN32(GetLastError());
		}

		if (dwBytesRead == 0)
		{
			free(pMessageBody);
			break;	// End of File.
		}

		pMessageBody[dwBytesRead] = '\0';
		//TRACE("%hs", pMessageBody);
		m_pResults->AppendResponse((char *)pMessageBody, dwBytesAvailable);

		free(pMessageBody);
	}

	return S_OK;
}

HRESULT Uploader::UploadFile(HINTERNET hRequest, const TCHAR * strBoundary, const TCHAR * lpszFieldName, const TCHAR * lpszPathName, const TCHAR * lpszContentType, DWORD *pdwOverallBytesSent, DWORD dwOverallBytesTotal)
{
	char *fileHeader = GenerateFileHeader(strBoundary, lpszFieldName, lpszPathName, lpszContentType);

	DWORD dwBytesWritten;
	if (!InternetWriteFile(hRequest, fileHeader, strlen(fileHeader), &dwBytesWritten))
	{
		free(fileHeader);
		return HRESULT_FROM_WIN32(GetLastError());
	}
	free(fileHeader);

	HANDLE hFile = CreateFile(lpszPathName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	if(m_pProgress) m_pProgress->OnFileBegin(lpszPathName);
	HRESULT hr = UploadFileContent(hRequest, lpszPathName, hFile, pdwOverallBytesSent, dwOverallBytesTotal);
	if (FAILED(hr))
	{
		CloseHandle(hFile);
		if(m_pProgress) m_pProgress->OnFileComplete(lpszPathName, hr);
		return hr;
	}

	if(m_pProgress) m_pProgress->OnFileComplete(lpszPathName, S_OK);
	CloseHandle(hFile);

	// After the file:
	const char *fileTrailer = GenerateFileTrailer();
	if (!InternetWriteFile(hRequest, fileTrailer, strlen(fileTrailer), &dwBytesWritten))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	return S_OK;
}

HRESULT Uploader::UploadField(HINTERNET hRequest, const TCHAR * strBoundary, const TCHAR * lpszFieldName, const TCHAR * lpszFieldValue)
{
	char *fieldHeader = GenerateFieldHeader(strBoundary, lpszFieldName);

	DWORD dwBytesWritten;
	if (!InternetWriteFile(hRequest, fieldHeader, strlen(fieldHeader), &dwBytesWritten))
	{
		free(fieldHeader);
		return HRESULT_FROM_WIN32(GetLastError());
	}
	free(fieldHeader);

	char *fieldValue = t2a(lpszFieldValue);
	if (!InternetWriteFile(hRequest, fieldValue, strlen(fieldValue), &dwBytesWritten))
	{
		free(fieldValue);
		return HRESULT_FROM_WIN32(GetLastError());
	}
	free(fieldValue);

	const char *fieldTrailer = GenerateFieldTrailer();
	if (!InternetWriteFile(hRequest, fieldTrailer, strlen(fieldTrailer), &dwBytesWritten))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	return S_OK;
}

HRESULT Uploader::UploadFileContent(HINTERNET hRequest, const TCHAR * lpszPathName, HANDLE hFile, DWORD *pdwOverallBytesSent, DWORD dwOverallBytesTotal)
{
	const int PROGRESS_INTERVAL = 500;	// Report progress twice a second.

	DWORD dwLocalFileSize = GetFileSize(hFile, NULL);
	DWORD dwFileBytesSent = 0;
	DWORD dwBytesPerSecond = m_pSettings->GetCachedBytesPerSecond();
	DWORD dwTimeStarted = GetTickCount() / PROGRESS_INTERVAL;
	DWORD dwTimeLast = dwTimeStarted;

	/* The buffer size is a tradeoff.  Too small and the transfer rate will drop.
	 * Too large and each write will take too long, and you'll not get useful progress
	 * reporting.
	 * What would have been nice is if the InternetStatusCallback was notified
	 * on partial writes, but it's only notified when the entire buffer is flushed.
	 * This makes it useless for adding progress to a large buffer.
	 * Instead, we'll dynamically adjust the buffer size, so that each write is
	 * taking about a second.
	 */

	// If a write takes less time than this, then increase the buffer size.
	const DWORD LOW_WRITE_THRESHOLD = 500;
	// If a write takes more time than this, then decrease the buffer size.
	const DWORD HIGH_WRITE_THRESHOLD = 1250;

	const size_t MIN_BUFFER_SIZE = 8192;
	const size_t INITIAL_BUFFER_SIZE = 32768;
	const size_t MAX_BUFFER_SIZE = 16 * 1024 * 1024;

	DWORD cbBuffer = INITIAL_BUFFER_SIZE;
	BYTE *pBuffer = (BYTE *)malloc(cbBuffer);

	DWORD dwSecondsToFileCompletion = dwLocalFileSize / dwBytesPerSecond;
	DWORD dwSecondsToOverallCompletion = dwOverallBytesTotal / dwBytesPerSecond;
	if(m_pProgress) m_pProgress->OnFileProgress(lpszPathName, 0, dwLocalFileSize, dwSecondsToFileCompletion, *pdwOverallBytesSent, dwOverallBytesTotal, dwSecondsToOverallCompletion, dwBytesPerSecond);
	for (;;)
	{
		if (m_pProgress && m_pProgress->CheckCancel())
		{
			free(pBuffer);
			return E_USER_CANCELLED;
		}
		DWORD dwBytesRead;
		if (!ReadFile(hFile, pBuffer, cbBuffer, &dwBytesRead, NULL))
		{
			free(pBuffer);
			return HRESULT_FROM_WIN32(GetLastError());
		}

		if (dwBytesRead == 0)
			break;

		DWORD dwWriteStartTime = GetTickCount();

		// Write that to the other end:
		DWORD dwBytesWritten;
		if (!InternetWriteFile(hRequest, pBuffer, dwBytesRead, &dwBytesWritten))
		{
			free(pBuffer);

			/* One cause I've found for InternetWriteFile failing, when writing to a PHP script,
			 * is that max_execution_time is too small, and the file is too large.
			 *
			 * The PHP script gets killed, and we see ERROR_INTERNET_CONNECTION_ABORTED.
			 */
			//TRACE("InternetWriteFile failed, error = %d (0x%x)\n", GetLastError(), GetLastError());

			// InternetGetLastResponseInfo doesn't do anything useful
			// for HTTP connections at this point.
			return HRESULT_FROM_WIN32(GetLastError());
		}

		DWORD dwWriteEndTime = GetTickCount();
		DWORD dwWriteTicks = dwWriteEndTime - dwWriteStartTime;

		if (dwWriteTicks < LOW_WRITE_THRESHOLD)
		{
			//TRACE("Writing %d bytes took %d ms.\n", cbBuffer, dwWriteTicks);
			//TRACE("Increasing buffer size.\n");
			// Increase the buffer size for increased performance.
			if (cbBuffer < MAX_BUFFER_SIZE)
				cbBuffer *= 2;
			free(pBuffer);
			pBuffer = (BYTE *)malloc(cbBuffer);
		}
		else if (dwWriteTicks > HIGH_WRITE_THRESHOLD)
		{
			//TRACE("Writing %d bytes took %d ms.\n", cbBuffer, dwWriteTicks);
			//TRACE("Decreasing buffer size.\n");

			// Decrease the buffer size for more regular progress reporting.
			cbBuffer /= 2;
			if (cbBuffer < INITIAL_BUFFER_SIZE)
				cbBuffer = INITIAL_BUFFER_SIZE;

			free(pBuffer);
			pBuffer = (BYTE *)malloc(cbBuffer);
		}

		dwFileBytesSent += dwBytesWritten;
		*pdwOverallBytesSent += dwBytesWritten;
		
		// Uncomment this to slow it down a bit, if you're testing against a local server:
		Sleep(50);

		// Avoid reporting progress too often.
		DWORD dwTimeNow = GetTickCount() / PROGRESS_INTERVAL;
		if (dwTimeNow != dwTimeLast)
		{
			DWORD dwSecondsTaken = dwTimeNow - dwTimeStarted;
			dwBytesPerSecond = dwFileBytesSent / dwSecondsTaken;

			DWORD dwFileBytesRemaining = dwLocalFileSize - dwFileBytesSent;
			dwSecondsToFileCompletion = dwFileBytesRemaining / dwBytesPerSecond;

			DWORD dwOverallBytesRemaining = dwOverallBytesTotal - *pdwOverallBytesSent;
			dwSecondsToOverallCompletion = dwOverallBytesRemaining / dwBytesPerSecond;

			if(m_pProgress) m_pProgress->OnFileProgress(lpszPathName, dwFileBytesSent, dwLocalFileSize, dwSecondsToFileCompletion, *pdwOverallBytesSent, dwOverallBytesTotal, dwSecondsToOverallCompletion, dwBytesPerSecond);
			dwTimeLast = dwTimeNow;
		}
	}

	m_pSettings->SetCachedBytesPerSecond(dwBytesPerSecond);
	if(m_pProgress) m_pProgress->OnFileProgress(lpszPathName, dwLocalFileSize, dwLocalFileSize, dwSecondsToFileCompletion, *pdwOverallBytesSent, dwOverallBytesTotal, dwSecondsToOverallCompletion, dwBytesPerSecond);
	free(pBuffer);
	return S_OK;
}

/* static */
void CALLBACK Uploader::StaticInternetStatusCallback(HINTERNET hInternet, DWORD_PTR dwContext,
														 DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength)
{
//	TRACE("StaticInternetStatusCallback(hInternet = %p, dwContext = %p, dwInternetStatus = %d, lpvStatusInformation = %p, dwStatusInformationLength = %d)\n",
//				hInternet, dwContext, dwInternetStatus, lpvStatusInformation, dwStatusInformationLength);

	Uploader *pThis = reinterpret_cast<Uploader *>(dwContext);
	pThis->InternetStatusCallback(hInternet, dwInternetStatus, lpvStatusInformation, dwStatusInformationLength);
}

void Uploader::InternetStatusCallback(HINTERNET hInternet, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength)
{
	switch (dwInternetStatus)
	{
	case INTERNET_STATUS_RESOLVING_NAME:
		// lpvStatusInformation is the server name.
		if(m_pProgress) m_pProgress->OnResolvingName((LPCTSTR)lpvStatusInformation);
		break;
	case INTERNET_STATUS_NAME_RESOLVED:
		if(m_pProgress) m_pProgress->OnNameResolved((LPCTSTR)lpvStatusInformation);
		break;
	case INTERNET_STATUS_CONNECTING_TO_SERVER:
		// The documentation says that lpvStatusInformation is a SOCKADDR structure.  It lies.
		if(m_pProgress) m_pProgress->OnConnectingToServer((LPCTSTR)lpvStatusInformation);
		break;
	case INTERNET_STATUS_CONNECTED_TO_SERVER:
		// The documentation says that lpvStatusInformation is a SOCKADDR structure.  It lies.
		if(m_pProgress) m_pProgress->OnConnectedToServer((LPCTSTR)lpvStatusInformation);
		break;
	case INTERNET_STATUS_SENDING_REQUEST:
		// lpvStatusInformation is NULL.
		if(m_pProgress) m_pProgress->OnSendingRequest();
		break;
	case INTERNET_STATUS_REQUEST_SENT:
		{
			DWORD dwBytesSent = *(DWORD *)lpvStatusInformation;
//			TRACE("INTERNET_STATUS_REQUEST_SENT: %d bytes\n", dwBytesSent);
			if(m_pProgress) m_pProgress->OnRequestSent(dwBytesSent);
		}
		break;
	case INTERNET_STATUS_RECEIVING_RESPONSE:
		// lpvStatusInformation is NULL.
		if(m_pProgress) m_pProgress->OnReceivingResponse();
		break;
	case INTERNET_STATUS_RESPONSE_RECEIVED:
		{
			DWORD dwBytesReceived = *(DWORD *)lpvStatusInformation;
			if(m_pProgress) m_pProgress->OnResponseReceived(dwBytesReceived);
		}
		break;
	case INTERNET_STATUS_CTL_RESPONSE_RECEIVED:
		//TRACE("INTERNET_STATUS_CTL_RESPONSE_RECEIVED: Not implemented.\n");
		break;
	case INTERNET_STATUS_PREFETCH:
		//TRACE("INTERNET_STATUS_PREFETCH:\n");
		break;
	case INTERNET_STATUS_CLOSING_CONNECTION:
		// lpvStatusInformation is NULL.
		if(m_pProgress) m_pProgress->OnClosingConnection();
		break;
	case INTERNET_STATUS_CONNECTION_CLOSED:
		// lpvStatusInformation is NULL.
		if(m_pProgress) m_pProgress->OnConnectionClosed();
		break;
	case INTERNET_STATUS_HANDLE_CREATED:
		{
			HINTERNET hCreated = *(HINTERNET *)lpvStatusInformation;
			//TRACE("INTERNET_STATUS_HANDLE_CREATED: %p\n", hCreated);
		}
		break;
	case INTERNET_STATUS_HANDLE_CLOSING:
		{
			HINTERNET hClosing = *(HINTERNET *)lpvStatusInformation;
			//TRACE("INTERNET_STATUS_HANDLE_CLOSING: %p\n", hClosing);
		}
		break;
	case INTERNET_STATUS_DETECTING_PROXY:
		//TRACE("INTERNET_STATUS_DETECTING_PROXY: Notifies the client application that a proxy has been detected.\n");
		break;
	case INTERNET_STATUS_REQUEST_COMPLETE:
		//TRACE("INTERNET_STATUS_REQUEST_COMPLETE:\n");
		break;
	case INTERNET_STATUS_REDIRECT:
		//TRACE("INTERNET_STATUS_REDIRECT:\n");
		break;
	case INTERNET_STATUS_INTERMEDIATE_RESPONSE:
		//TRACE("INTERNET_STATUS_INTERMEDIATE_RESPONSE:\n");
		break;
	case INTERNET_STATUS_USER_INPUT_REQUIRED:
		//TRACE("INTERNET_STATUS_USER_INPUT_REQUIRED:\n");
		break;
	case INTERNET_STATUS_STATE_CHANGE:
		//TRACE("INTERNET_STATUS_STATE_CHANGE:\n");
		break;
	case INTERNET_STATUS_COOKIE_SENT:
		//TRACE("INTERNET_STATUS_COOKIE_SENT:\n");
		break;
	case INTERNET_STATUS_COOKIE_RECEIVED:
		//TRACE("INTERNET_STATUS_COOKIE_RECEIVED:\n");
		break;
	case INTERNET_STATUS_PRIVACY_IMPACTED:
		//TRACE("INTERNET_STATUS_PRIVACY_IMPACTED:\n");
		break;
	case INTERNET_STATUS_P3P_HEADER:
		//TRACE("INTERNET_STATUS_P3P_HEADER:\n");
		break;
	case INTERNET_STATUS_P3P_POLICYREF:
		//TRACE("INTERNET_STATUS_P3P_POLICYREF:\n");
		break;
	case INTERNET_STATUS_COOKIE_HISTORY:
		//TRACE("INTERNET_STATUS_COOKIE_HISTORY:\n");
		break;
	}
}
