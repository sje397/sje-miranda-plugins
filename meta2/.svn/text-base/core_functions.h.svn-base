class MetaMap: public Map<ContactHandle, SubcontactList> {
public:
	SubcontactList &operator[](HANDLE h) {
		return Map<ContactHandle, SubcontactList>::operator[](ContactHandle(h));
	}

	const bool exists(HANDLE h) const {
		return Map<ContactHandle, SubcontactList>::exists(ContactHandle(h));
	}

	const bool remove(HANDLE h) {
		return Map<ContactHandle, SubcontactList>::remove(ContactHandle(h));
	}
};

extern MetaMap metaMap;

HANDLE GetMetaHandle(DWORD id);
void Meta_Hide(bool hide);
HANDLE Meta_GetActive(HANDLE hMeta);
HANDLE Meta_GetMostOnline(HANDLE hMeta);
HANDLE Meta_GetMostOnlineSupporting(HANDLE hMeta, int flag, int cap);
HANDLE Meta_Convert(HANDLE hSub);
void Meta_CalcStatus(HANDLE hMeta);
void Meta_Assign(HANDLE hSub, HANDLE hMeta);
void Meta_Remove(HANDLE hSub);