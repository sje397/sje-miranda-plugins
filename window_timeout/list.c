#include "windowtimeout.h"

#define BASE_TIMER_ID		10000

struct Entry *list_head = 0;

BOOL me_typing;

VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime);

struct Entry *get_entry(HANDLE hContact) {
	struct Entry *current = list_head;
	while(current) {
		if(current->hContact == hContact)
			return current;
		current = current->next;
	}
	return 0;
}

struct Entry *get_entry_for_timer(UINT timer_id) {
	struct Entry *current = list_head;
	char buff[128];
	sprintf(buff, "get_entry_for_timerid. timer_id = %d", timer_id);
	msg(buff);
	while(current) {
		sprintf(buff, "checking entry with timer_id %d", current->timer_id);
		msg(buff);
		if(current->timer_id == timer_id)
			return current;
		current = current->next;
	}
	return 0;
}

void add_entry(HANDLE hContact) {
	struct Entry *entry = get_entry(hContact);
	if(!entry) {
		char buff[128];
		entry = mir_alloc(sizeof(struct Entry));

		sprintf(buff, "added entry. hcontact = %d", hContact);
		msg(buff);
		
		entry->hContact = hContact;
		entry->hwnd = 0;

		entry->next = list_head;
		entry->prev = 0;
		if(list_head) list_head->prev = entry;
		list_head = entry;

		entry->timer_id = 0;
		entry->typing = FALSE;
	}
}

void set_window_handle(HANDLE hContact, HWND hwnd) {
	struct Entry *entry = get_entry(hContact);
	if(entry) {
		msg("set window handle");
		entry->hwnd = hwnd;
	}
}

void set_typing(BOOL typing) {
	me_typing = typing;
}

void remove_entry(HANDLE hContact) {
	struct Entry *entry = get_entry(hContact);
	if(entry) {
		msg("remove entry");
		if(entry->prev) entry->prev->next = entry->next;
		if(entry->next) entry->next->prev = entry->prev;

		if(list_head == entry) list_head = list_head->next;

		KillTimer(0, entry->timer_id);

		mir_free(entry);
	}
}

void reset_timer(HANDLE hContact) {
	struct Entry *entry = get_entry(hContact);
	if(entry) {
		msg("reset timer");
		if(entry->timer_id) KillTimer(0, entry->timer_id);
		entry->timer_id = SetTimer(0, 0, options.timeout * 1000, TimerProc);
	}
}

VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime) {
	struct Entry *entry;
	char buff[128];

	KillTimer(0, idEvent);
	entry = get_entry_for_timer(idEvent);

	sprintf(buff, "timer. id = %d", idEvent);
	msg(buff);
	if(entry && entry->hwnd != 0) {
		if(me_typing) {
			entry->timer_id = SetTimer(0, 0, TYPING_CHECK_DELAY, TimerProc);
		} else {
			msg("timer: close window");
			SendMessage(entry->hwnd, WM_CLOSE, 0, 0);
			remove_entry(entry->hContact);
		}
	}
}