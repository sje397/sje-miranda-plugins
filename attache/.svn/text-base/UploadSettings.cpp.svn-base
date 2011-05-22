#include "common.h"
#include "UploadSettings.h"

bool UploadSettings::SetAddress(TCHAR * lpszAddress)
{
	TCHAR szScheme[MAX_PATH];
	TCHAR szUserName[MAX_PATH];
	TCHAR szPassword[MAX_PATH];
	TCHAR szHostName[MAX_PATH];
	TCHAR szUrlPath[MAX_PATH];
	TCHAR szExtraInfo[MAX_PATH];

	URL_COMPONENTS url;
	memset(&url, 0, sizeof(URL_COMPONENTS));
	url.dwStructSize = sizeof(URL_COMPONENTS);
	url.lpszScheme = szScheme;
	url.dwSchemeLength = MAX_PATH;
	url.lpszHostName = szHostName;
	url.dwHostNameLength = MAX_PATH;
	url.lpszUserName = szUserName;
	url.dwUserNameLength = MAX_PATH;
	url.lpszPassword = szPassword;
	url.dwPasswordLength = MAX_PATH;
	url.lpszUrlPath = szUrlPath;
	url.dwUrlPathLength = MAX_PATH;
	url.lpszExtraInfo = szExtraInfo;
	url.dwExtraInfoLength = MAX_PATH;

	if (!InternetCrackUrl(lpszAddress, 0, 0, &url))
		return false;

	// Verify that none of the other bits got filled in:
	if (_tcscmp(url.lpszScheme, _T("http")) != 0)
		return false;
	if (url.nPort != INTERNET_DEFAULT_HTTP_PORT)
		return false;	// Not supported yet.
	if (url.lpszUserName[0])
		return false;
	if (url.lpszPassword[0])
		return false;
	if (url.lpszExtraInfo[0])
		return false;

	_tcsncpy(m_strHostName, szHostName, 1024);
	_tcsncpy(m_strUrlPath,szUrlPath, MAX_PATH);

	return true;
}

TCHAR *UploadSettings::GetAddress() const
{
	// TODO: Use InternetCreateUrl to put it back together.
	TCHAR *strAddress = (TCHAR *)malloc(1024 * sizeof(TCHAR)); 
	mir_sntprintf(strAddress, 1024, _T("http://%s%s"), m_strHostName, m_strUrlPath);

	return strAddress;
}

void UploadSettings::AddFile(TCHAR * lpszFieldName, TCHAR * lpszPathName, TCHAR * lpszContentType)
{
	// TODO: Open the file here, so that we can guarantee it's valid all the way through?
	m_files.push_back(FileInfo(lpszFieldName, lpszPathName, lpszContentType));
}

void UploadSettings::ClearFiles()
{
	m_files.clear();
}

void UploadSettings::GetFiles(FileInfoList *pFiles) const
{
	// TODO: If we start storing file handles in the FileInfo object, we ought to use DuplicateHandle?
	pFiles->clear();
	for(FileInfoList::ListNode *n = m_files.start(); n; n = n->next)
		pFiles->push_back(n->info);
	//std::copy(m_files.begin(), m_files.end(), std::back_inserter(*pFiles));
}

void UploadSettings::ClearFields()
{
	m_fields.clear();
}

void UploadSettings::GetFields(FieldInfoList *pFields) const
{
	pFields->clear();
	for(FieldInfoList::ListNode *n = m_fields.start(); n; n = n->next)
		pFields->push_back(n->info);
	//std::copy(m_fields.begin(), m_fields.end(), std::back_inserter(*pFields));
}

void UploadSettings::AddField(TCHAR * lpszFieldName, TCHAR *lpszFieldValue)
{
	m_fields.push_back(FieldInfo(lpszFieldName, lpszFieldValue));
}
