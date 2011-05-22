#include "common.h"

bool ContactHandle::operator==(const ContactHandle &other) const {
	return hContact == other.handle();
}

bool ContactHandle::operator<(const ContactHandle &other) const {
	// these don't work, 'cause the comparison is not constant
	//return CallService(MS_CLIST_CONTACTSCOMPARE, (WPARAM)hContact, (LPARAM)other.handle()) < 0;
	//return _tcscmp(ContactName(hContact), ContactName(other.handle()));

	// will produce a *very* unbalanced binary tree (metaMap) when contacts are added in order of handle - which they most likely are
	return hContact < other.handle();
}

