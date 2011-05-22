#pragma once

#include "common.h"

class FieldInfo
{
	TCHAR m_fieldName[512];
	TCHAR m_fieldValue[2048];

public:
	FieldInfo(TCHAR *lpszFieldName, TCHAR *lpszFieldValue) {
		_tcsncpy(m_fieldName, lpszFieldName, 512);
		_tcsncpy(m_fieldValue, lpszFieldValue, 2048);
	}

	FieldInfo() {}
	FieldInfo(const FieldInfo &other) {
		memcpy(this, &other, sizeof(FieldInfo));
	}

	const TCHAR *GetFieldName() const { return &m_fieldName[0]; }
	const TCHAR *GetFieldValue() const { return &m_fieldValue[0]; }
};

//typedef std::vector< FieldInfo > FieldInfoCollection;
class FieldInfoList {
public:
	class ListNode {
	public:
		ListNode() {}
		FieldInfo info;
		ListNode *next;
	};

	FieldInfoList(): head(0) {}
	~FieldInfoList() {clear();}

	void clear() {
		ListNode *n = head;
		while(n) {
			n = n->next;
			delete head;
			head = n;
		}
	}

	void push_back(const FieldInfo &info) {
		ListNode *n = new ListNode;
		n->info = info;
		n->next = head;
		head = n;
	}

	ListNode *start() const {return head;}
protected:
	ListNode *head;

};
