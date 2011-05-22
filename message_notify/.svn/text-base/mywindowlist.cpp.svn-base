#include "common.h"
#include "mywindowlist.h"
#include "collection.h"

class WindowList: public LinkedList<HWND> {
public:
	WindowList(): LinkedList<HWND>() {}
	virtual ~WindowList() {}

	void PostListMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
		ListNode<HWND> *n = head;
		while(n) {
			PostMessage(n->val, msg, wParam, lParam);
			n = n->next;
		}
	}
};

class WindowMap: public Map<HANDLE, WindowList> {
public:
	WindowMap(): Map<HANDLE, WindowList>() {}
	virtual ~WindowMap() {}

	void Add(HANDLE hContact, HWND hWnd) {
		(*this)[hContact].add(hWnd);
	}

	void Remove(HANDLE hContact, HWND hWnd) {
		if(contains(hContact)) {
			(*this)[hContact].remove(hWnd);
			if((*this)[hContact].size() == 0)
				remove(hContact);
		}
	}

	void PostMapMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
		for(Iterator i = start(); i.has_val();i.next())
			i.val().second.PostListMessage(msg, wParam, lParam);
	}

};

WindowMap window_map;

CRITICAL_SECTION list_cs;

void AddToWindowList(HANDLE hContact, HWND hWnd) {
	EnterCriticalSection(&list_cs);
	
	window_map.Add(hContact, hWnd);
		
	LeaveCriticalSection(&list_cs);
}

void RemoveFromWindowList(HANDLE hContact, HWND hWnd) {
	EnterCriticalSection(&list_cs);

	window_map.Remove(hContact, hWnd);

	LeaveCriticalSection(&list_cs);
}

void PostMessageWindowList(UINT msg, WPARAM wParam, LPARAM lParam) {
	EnterCriticalSection(&list_cs);

	window_map.PostMapMessage(msg, wParam, lParam);

	LeaveCriticalSection(&list_cs);
}

void PostMessageWindowListContact(HANDLE hContact, UINT msg, WPARAM wParam, LPARAM lParam) {
	EnterCriticalSection(&list_cs);

	if(window_map.contains(hContact))
		window_map[hContact].PostListMessage(msg, wParam, lParam);

	LeaveCriticalSection(&list_cs);
}

bool InList(HANDLE hContact, HWND hWnd) {
	bool ret = false;
	EnterCriticalSection(&list_cs);

	if(window_map.contains(hContact) && window_map[hContact].contains(hWnd))
		ret = true;

	LeaveCriticalSection(&list_cs);
	return ret;
}

int CountList(HANDLE hContact) {
	int count = 0;
	EnterCriticalSection(&list_cs);

	if(window_map.contains(hContact))
		count = window_map[hContact].size();

	LeaveCriticalSection(&list_cs);
	return count;
}

bool EmptyList(HANDLE hContact) {
	EnterCriticalSection(&list_cs);

	bool ret = !window_map.contains(hContact);

	LeaveCriticalSection(&list_cs);
	return ret;
}

void InitWindowList() {
	InitializeCriticalSection(&list_cs);
}

void DeinitWindowList() {
	DeleteCriticalSection(&list_cs);
}

