#include "common.h"
#include "NetMessage.h"
#include "formatting.h"
#include <cstdlib>

NMString::NMString(): text(0), len(-1) {
}

NMString::NMString(const NMString &other) {
	len = strlen(other.text);
	text = new char[len + 1];
	mir_snprintf(text, len + 1, "%s", other.text);
}

NMString::NMString(char *t) {
	len = strlen(t);
	text = new char[len + 1];
	mir_snprintf(text, len + 1, "%s", t);
}

const bool NMString::operator<(const NMString &other) const {
	return strcmp(text, other.text) < 0;
}

const bool NMString::operator==(const NMString &other) const {
	return strcmp(text, other.text) == 0;
}

const bool NMString::operator==(const char *str) const {
	return strcmp(text, str) == 0;
}

NMString &NMString::operator=(const NMString &other) {
	int olen = strlen(other.text);
	if(olen > len) {
		delete[] text;
		text = new char[olen + 1];
	}
	len = olen;
	mir_snprintf(text, len + 1, "%s", other.text);

	return *this;
}

NMString::~NMString() {
	delete[] text;
}

StringList::StringList(char *sep): LinkedList<NMString>(), seperators(sep) {
}

StringList::~StringList() {
}

int StringList::parse(char *data, int size) {
	int i = 0, tl, pos = 0;
	bool first = true;
	while(pos < size) {
		tl = strcspn(data + pos, seperators);
		if(first && tl == 0) {
			first = false;
			pos += 1;
		} else {
			if(pos + tl < size) {
				*(data + pos + tl) = 0;
				add(NMString(unescape_inplace(data + pos)));
				i++;
			}
			pos += tl + 1;
		}
	}
	return i;
}

bool StringList::get_string(int index, char *buff, int buffsize) {
	int i = 0;
	ListNode<NMString> *n = head;
	while(n && i < index) {
		if(i == index) break;
		n = n->next;
		i++;
	}
	if(n) {
		strncpy(buff, n->val.text, buffsize);
		return true;
	}
	return false;
}

int StringList::get_int(int index) {
	int i = 0;
	ListNode<NMString> *n = head;
	while(n && i < index) {
		if(i == index) break;
		n = n->next;
		i++;
	}
	if(n) {
		return atoi(n->val.text);
	}
	return 0;
}

void StringList::add_string(char *buff) {
	add(NMString(buff));
}

void StringList::add_int(int i) {
	char buff[128];
	_itoa(i, buff, 10);
	add(NMString(buff));
}

int StringList::make_body(char *buff, int size, bool sep_at_start) {
	int len = 0;
	bool first = true;
	char *val;
	for(LinkedList<NMString>::Iterator i = start(); i.has_val() && len < size; i.next()) {
		NMString &val = i.val();
		if(first && sep_at_start == false) {
			first = false;
		} else {
			if(len < size) buff[len++] = seperators[0];
			if(len < size) buff[len] = 0;
		}
		char *temp = escape(val.text);
		strncat(buff, temp, size - len);
		len += strlen(temp);
		delete[] temp;
	}
	if(len < size) buff[len] = 0;
	return len;
}

char *StringList::escape(char *val) {
	int len = strlen(val);
	char *buff = new char[len + 1];
	strncpy(buff, val, strlen(val));
	return buff;
}

char *StringList::unescape_inplace(char *val) {
	return val;
}

PipedStringList::PipedStringList(): StringList() {
}

PipedStringList::~PipedStringList() {
}

char *PipedStringList::escape(char *val) {
	int len = strlen(val);
	char *buff = new char[len * 2 + 1];
	int read_pos = 0, write_pos = 0;
	while(read_pos < len) {
		if(val[read_pos] == '|') {
			buff[write_pos++] = '/';
			buff[write_pos++] = '3';
			read_pos++;
		} else {
			buff[write_pos++] = val[read_pos++];
		}
	}
	buff[write_pos] = 0;
	return buff;
}

char *PipedStringList::unescape_inplace(char *val) {
	int read_pos = 0, write_pos = 0, len = strlen(val);
	while(read_pos < len) {
		if(val[read_pos] == '/' && read_pos + 1 < len) {
			if(val[read_pos + 1] == '3') {
				read_pos += 2;
				val[write_pos++] = '|';
			} else {
				val[write_pos++] = val[read_pos++];
			}
		} else {
			val[write_pos++] = val[read_pos++];
		}
	}
	val[write_pos] = 0;

	return val;
}


Dictionary::Dictionary(): LinkedList< KeyValue >() {
}

Dictionary::~Dictionary() {
}

int Dictionary::parse(char *data, int size) {
	int i = 0, tl, il, pos = 0;
	bool first = true;
	char *key, *value;
	while(pos < size) {
		tl = strcspn(data + pos, "\x1c");
		if(first && tl == 0) {
			first = false;
			pos += 1;
		} else {
			if(pos + tl < size) {
				*(data + pos + tl) = 0;
				il = strcspn(data + pos, "=");
				*(data + pos + il) = 0;
				key = unescape_inplace(data + pos);
				value = unescape_inplace(data + pos + il + 1);
				KeyValue kv = KeyValue(NMString(key), NMString(value));
				add(kv);
			}
			pos += tl + 1;

		}
	}
	return i;
}

bool Dictionary::get_string(char *key, char *buff, int buffsize) {
	NMString strkey(key);
	for(Iterator i = start(); i.has_val(); i.next()) {
		KeyValue kv = i.val();
		if(kv.first == strkey) {
			mir_snprintf(buff, buffsize, "%s", kv.second.text);
			return true;
		}
	}
	return false;
}

int Dictionary::get_int(char *key) {
	NMString strkey(key);
	for(Iterator i = start(); i.has_val(); i.next()) {
		KeyValue kv = i.val();
		if(kv.first == strkey) {
			return atoi(kv.second.text);
		}
	}
	return 0;
}

void Dictionary::add_string(char *key, char *buff) {
	KeyValue dat = KeyValue(NMString(key), NMString(buff));
	add(dat);
}

void Dictionary::add_int(char *key, int i) {
	char buff[254];
	KeyValue dat(NMString(key), NMString(_itoa(i, buff, 10)));
	add(dat);
}

int Dictionary::make_body(char *buff, int size) {
	int pos = 0, key_len, val_len, len;
	char *ekey, *eval;
	bool first = true;
	for(Iterator i = start(); i.has_val() && pos < size; i.next()) {
		KeyValue &v = i.val();

		ekey = escape(v.first.text);
		eval = escape(v.second.text);
		key_len = strlen(ekey);
		val_len = strlen(eval);
		len = key_len + val_len + 2;

		if(size - pos - len > 0) {
			if(first) {
				mir_snprintf(buff + pos, size - pos, "%s=%s", ekey, eval);
				first = false;
			}  else
				mir_snprintf(buff + pos, size - pos, "\x1c%s=%s", ekey, eval);
			pos += len;
		}
		delete[] ekey;
		delete[] eval;
	}

	return pos;
}

char *Dictionary::escape(char *val) {
	int len = strlen(val);
	char *buff = new char[len * 10 + 1]; // allow for every char to be a large html entity (e.g. '&thetasym;' is 10 chars!)
	strcpy(buff, val);
	entitize_html(buff, len * 10 + 1);
	return buff;
}

char *Dictionary::unescape_inplace(char *val) {
	unentitize_html(val);
	return val;
}

NetMessage::NetMessage(): Map<NMString, NMString>() {
}

NetMessage::~NetMessage()
{
}

//static
char *NetMessage::unescape_inplace(char *val) {
	int read_pos = 0, write_pos = 0, len = strlen(val);
	while(read_pos < len) {
		if(val[read_pos] == '/' && read_pos + 1 < len) {
			if(val[read_pos + 1] == '1') val[write_pos++] = '/';
			else if(val[read_pos + 1] == '2') val[write_pos++] = '\\';
			else {
				val[write_pos++] = val[read_pos];
				val[write_pos++] = val[read_pos + 1];
			}
			read_pos+= 2;
		} else {
			val[write_pos++] = val[read_pos++];
		}
	}
	val[write_pos] = 0;

	return val;
}

int NetMessage::parse(char *data, int size) {
	StringList sl("\\");
	sl.parse(data, size);
	for(StringList::Iterator i = sl.start(); i.has_val(); i.next()) {
		NMString &key = i.val();
		i.next();
		if(i.has_val()) {
			NMString &val = i.val();
			put(key, val);
		}
	}
	return this->size();
}

bool NetMessage::get_string(char *key, char *buff, int buffsize) {
	NMString val;
	if(get(NMString(key), val)) {
		mir_snprintf(buff, buffsize, "%s", val.text);
		unescape_inplace(buff);
		return true;
	}
	return false;
}

int NetMessage::get_int(char *key) {
	NMString val;
	if(get(NMString(key), val)) {
		return atoi(val.text);
	}
	return 0;
}

bool NetMessage::get_data(char *key, char *buff, int *size) {
	NMString val;
	if(get(NMString(key), val)) {
		NETLIBBASE64 nbd = {0};
		nbd.pszEncoded = val.text;
		nbd.cchEncoded = strlen(nbd.pszEncoded);
		nbd.pbDecoded = (BYTE *)buff;
		nbd.cbDecoded = *size;

		//*size = Netlib_GetBase64DecodedBufferSize(nbd.cchEncoded);
		if(CallService(MS_NETLIB_BASE64DECODE, 0, (LPARAM)&nbd)) {
			*size = nbd.cbDecoded;
			return true;
		}
	}
	return false;
}

Dictionary NetMessage::get_dict(char *key) {
	Dictionary d;
	char t[4096];
	if(get_string(key, t, 4096)) {
		d.parse(t, strlen(t) + 1);
	}
	return d;
}

PipedStringList NetMessage::get_list(char *key) {
	PipedStringList l;
	char t[4096];
	if(get_string(key, t, 4096)) {
		l.parse(t, strlen(t) + 1);
	}
	return l;
}

ClientNetMessage::ClientNetMessage(): Dictionary() {
}

ClientNetMessage::~ClientNetMessage() {
}

bool ClientNetMessage::add_data(char *key, char *data, int size) {
	int len = Netlib_GetBase64EncodedBufferSize(size);
	char *buff = new char[len];

	NETLIBBASE64 nbd = {0};
	nbd.pszEncoded = buff;
	nbd.cchEncoded = len;
	nbd.pbDecoded = (BYTE *)data;
	nbd.cbDecoded = size;

	if(CallService(MS_NETLIB_BASE64ENCODE, 0, (LPARAM)&nbd)) {
		KeyValue dat = KeyValue(NMString(key), NMString(buff));
		add(dat);
		delete[] buff;
		return true;
	}

	delete[] buff;
	return false;
}

void ClientNetMessage::add_string(char *key, char *buff) {
	char *temp = escape(buff);
	KeyValue dat = KeyValue(NMString(key), NMString(temp));
	delete[] temp;
	add(dat);
}

void ClientNetMessage::add_dict(char *key, Dictionary &d) {
	char t[4096];
	if(d.make_body(t, 4096)) {
		KeyValue dat = KeyValue(NMString(key), NMString(t));
		add(dat);
	}
}

void ClientNetMessage::add_list(char *key, PipedStringList &list) {
	char t[4096];
	if(list.make_body(t, 4096)) {
		KeyValue dat = KeyValue(NMString(key), NMString(t));
		add(dat);
	}
}

//static 
char *ClientNetMessage::escape(char *val) {
	int len = strlen(val);
	char *buff = new char[len * 2 + 1];
	int read_pos = 0, write_pos = 0;
	while(read_pos < len) {
		if(val[read_pos] == '/') {
			buff[write_pos++] = '/';
			buff[write_pos++] = '1';
			read_pos++;
		} else if(val[read_pos] == '\\') {
			buff[write_pos++] = '/';
			buff[write_pos++] = '2';
			read_pos++;
		} else {
			buff[write_pos++] = val[read_pos++];
		}
	}
	buff[write_pos] = 0;
	return buff;
}

int ClientNetMessage::make_packet(char *buff, int size) {
	int pos = 0, key_len, val_len, len;
	for(Iterator i = start(); i.has_val() && pos < size; i.next()) {
		KeyValue v = i.val();

		key_len = strlen(v.first.text);
		val_len = strlen(v.second.text);
		len = key_len + val_len + 2;

		if(size - pos - len > 0) {
			mir_snprintf(buff + pos, size - pos, "\\%s\\%s", v.first.text, v.second.text);
			pos += len;
		} else
			return -1;
	}

	if(size - pos > 7) {
		mir_snprintf(buff + pos, size - pos, "\\final\\");
		pos += 7;
		return pos;
	}
	return -1;
}
