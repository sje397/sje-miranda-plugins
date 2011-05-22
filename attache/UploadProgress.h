#pragma once

// The UploadProgress interface allows an observer to be notified
// about the upload.
class UploadProgress
{
public:
	virtual void OnResolvingName(LPCTSTR lpsz) = 0;
	virtual void OnNameResolved(LPCTSTR lpsz) = 0;
	virtual void OnConnectingToServer(LPCTSTR lpsz) = 0;
	virtual void OnConnectedToServer(LPCTSTR lpsz) = 0;
	virtual void OnSendingRequest() = 0;
	virtual void OnRequestSent(DWORD dwBytesSent) = 0;
	virtual void OnReceivingResponse() = 0;
	virtual void OnResponseReceived(DWORD dwBytesReceived) = 0;
	virtual void OnClosingConnection() = 0;
	virtual void OnConnectionClosed() = 0;

	virtual void OnFileBegin(LPCTSTR lpszPathName) = 0;
	virtual void OnFileProgress(LPCTSTR lpszPathName, DWORD dwFileBytesSent, DWORD dwFileBytesTotal, DWORD dwSecondsToFileCompletion, DWORD dwOverallBytesSent, DWORD dwOverallBytesTotal, DWORD dwSecondsToOverallCompletion, DWORD dwBytesPerSecond) = 0;
	virtual void OnFileComplete(LPCTSTR lpszPathName, HRESULT hr) = 0;

	virtual bool CheckCancel() = 0;
};

// The BackgroundUploadProgress interface adds other notification
// methods that are important when doing the upload from a
// background thread.
class BackgroundUploadProgress : public UploadProgress
{
public:
	virtual void OnBackgroundUploadBegin() = 0;
	virtual void OnBackgroundUploadComplete(HRESULT hr) = 0;
};
