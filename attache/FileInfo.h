#pragma once

#include "common.h"
//#include <tchar.h>

class FileInfo
{
	TCHAR m_fieldName[512];
	TCHAR m_pathName[MAX_PATH];
	TCHAR m_contentType[512];

public:
	FileInfo(TCHAR *lpszFieldName, TCHAR *lpszPathName, TCHAR *lpszContentType) {
		_tcsncpy(m_fieldName, lpszFieldName, 512);
		_tcsncpy(m_pathName, lpszPathName, MAX_PATH);
		_tcsncpy(m_contentType, lpszContentType, 512);
	}

	FileInfo() {}
	FileInfo(const FileInfo &other) {
		memcpy(this, &other, sizeof(FileInfo));
	}

	const TCHAR *GetFieldName() const { return &m_fieldName[0]; }
	const TCHAR *GetPathName() const { return &m_pathName[0]; }
	const TCHAR *GetContentType() const { return &m_contentType[0]; }
};

//typedef std::vector< FileInfo > FileInfoCollection;
class FileInfoList {
public:
	class ListNode {
	public:
		ListNode() {}
		FileInfo info;
		ListNode *next;
	};

	FileInfoList(): head(0) {}
	~FileInfoList() {clear();}

	void clear() {
		ListNode *n = head;
		while(n) {
			n = n->next;
			delete head;
			head = n;
		}
	}


	void push_back(const FileInfo &info) {
		ListNode *n = new ListNode;
		n->info = info;
		n->next = head;
		head = n;
	}

	ListNode *start() const {return head;}
protected:
	ListNode *head;

};
