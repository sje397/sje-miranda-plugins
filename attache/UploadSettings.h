#pragma once
#include "FileInfo.h"
#include "FieldInfo.h"

class UploadSettings
{
	TCHAR m_strHostName[1024];
	TCHAR m_strUrlPath[MAX_PATH];

	FileInfoList m_files;
	FieldInfoList m_fields;

	DWORD m_dwCachedBytesPerSecond;

	// A random guess: 56Kbps modem (ish)
	static const DWORD INITIAL_ESTIMATED_TRANSFER_RATE = 5600;

public:
	UploadSettings()
		: m_dwCachedBytesPerSecond(INITIAL_ESTIMATED_TRANSFER_RATE)
	{
	}

	bool SetAddress(TCHAR *lpszAddress);

	TCHAR *GetAddress() const;
	const TCHAR *GetHostName() const { return &m_strHostName[0]; }
	const TCHAR *GetUrlPath() const { return &m_strUrlPath[0]; }

	void ClearFiles();
	void AddFile(TCHAR *lpszFieldName, TCHAR *lpszFileName, TCHAR *lpszContentType);
	void GetFiles(FileInfoList *pFiles) const;

	void ClearFields();
	void AddField(TCHAR *lpszFieldName, TCHAR *lpszFieldValue);
	void GetFields(FieldInfoList *pFields) const;

	DWORD GetCachedBytesPerSecond() const { return m_dwCachedBytesPerSecond; }
	void SetCachedBytesPerSecond(DWORD dwBytesPerSecond) { m_dwCachedBytesPerSecond = dwBytesPerSecond; }
};
