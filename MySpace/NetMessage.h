#ifndef _NET_MESSAGE_INC
#define _NET_MESSAGE_INC

#include "collection.h"

class NMString {
public:
	NMString();
	NMString(const NMString &other);
	NMString(char *t);
	virtual ~NMString();

	const bool operator<(const NMString &other) const;
	const bool operator==(const NMString &other) const;
	const bool operator==(const char *str) const;
	NMString &operator=(const NMString &other);

	char *text;
	int len;
};

class StringList: public LinkedList<NMString> {
public:
	StringList(char *sep = "|");
	virtual ~StringList();

	int parse(char *data, int size);

	bool get_string(int index, char *buff, int buffsize);
	int get_int(int index);

	void add_string(char *buff);
	void add_int(int i);

	int make_body(char *buff, int size, bool sep_at_start = false);
protected:
	virtual char *escape(char *val);
	virtual char *unescape_inplace(char *val);

	char* seperators;
};

class PipedStringList: public StringList {
public:
	PipedStringList();
	virtual ~PipedStringList();
protected:
	char *escape(char *val);
	char *unescape_inplace(char *val);
};

class KeyValue: public Pair<NMString, NMString> {
public:
	KeyValue(const NMString &k, const NMString &v): Pair<NMString, NMString>(k, v) {}
	KeyValue &operator=(const KeyValue &other) {
		first = other.first;
		second = other.second;
		return *this;
	}
};

class Dictionary: public LinkedList< KeyValue > {
public:
	Dictionary();
	virtual ~Dictionary();

	virtual int parse(char *data, int size);

	bool get_string(char *key, char *buff, int buffsize);
	int get_int(char *key);

	virtual void add_string(char *key, char *buff);
	void add_int(char *key, int i);

	int make_body(char *buff, int size);

	static char *escape(char *val);
	static char *unescape_inplace(char *val);
};

class NetMessage: public Map<NMString, NMString> {
public:
	NetMessage();
	virtual ~NetMessage();

	int parse(char *data, int size);

	bool get_string(char *key, char *buff, int buffsize);
	int get_int(char *key);
	bool get_data(char *key, char *buff, int *size);
	Dictionary get_dict(char *key);
	PipedStringList get_list(char *key);

	static char *unescape_inplace(char *val);
};

class ClientNetMessage: public Dictionary {
public:
	ClientNetMessage();
	virtual ~ClientNetMessage();

	int make_packet(char *buff, int size);

	void add_string(char *key, char *buff);
	bool add_data(char *key, char *buff, int size);
	void add_dict(char *key, Dictionary &d);
	void add_list(char *key, PipedStringList &list);

	static char *escape(char *val);
};

#endif
