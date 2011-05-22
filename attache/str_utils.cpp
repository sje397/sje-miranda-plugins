#include "common.h"
#include "str_utils.h"

int code_page = CP_ACP;

void set_codepage() {
	if(ServiceExists(MS_LANGPACK_GETCODEPAGE))
		code_page = CallService(MS_LANGPACK_GETCODEPAGE, 0, 0);
}

char *w2u(const wchar_t *ws) {
	if(ws) {
		int size = WideCharToMultiByte(CP_UTF8, 0, ws, -1, 0, 0, 0, 0);
		char *buff = (char *)malloc(size);
		WideCharToMultiByte(CP_UTF8, 0, ws, -1, buff, 2048, 0, 0);
		return buff;
	} else 
		return 0;
}

wchar_t *u2w(const char *utfs) {
	if(utfs) {
		int size = MultiByteToWideChar(CP_UTF8, 0, utfs, -1, 0, 0);
		wchar_t *buff = (wchar_t *)malloc(size * sizeof(wchar_t));
		MultiByteToWideChar(CP_UTF8, 0, utfs, -1, buff, size);
		return buff;
	} else
		return 0;
}

wchar_t *a2w(const char *as) {
	int code_page = CP_ACP;
	if(ServiceExists(MS_LANGPACK_GETCODEPAGE)) code_page = CallService(MS_LANGPACK_GETCODEPAGE, 0, 0);
	int size = MultiByteToWideChar(code_page, 0, as, -1, 0, 0);
	wchar_t *buff = (wchar_t *)malloc(size * sizeof(wchar_t));
	MultiByteToWideChar(code_page, 0, as, -1, buff, size);
	return buff;
}

char *w2a(const wchar_t *ws) {
	int code_page = CP_ACP;
	if(ServiceExists(MS_LANGPACK_GETCODEPAGE)) code_page = CallService(MS_LANGPACK_GETCODEPAGE, 0, 0);
	int size = WideCharToMultiByte(code_page, 0, ws, -1, 0, 0, 0, 0);
	char *buff = (char *)malloc(size);
	WideCharToMultiByte(code_page, 0, ws, -1, buff, 2048, 0, 0);
	return buff;
}

char *t2a(const TCHAR *ts) {
#ifdef _UNICODE
	return w2a(ts);
#else
	return _strdup(ts);
#endif
}

TCHAR *a2t(const char *as) {
#ifdef _UNICODE
	return a2w(as);
#else
	return _strdup(as);
#endif
}

TCHAR *u2t(const char *utfs) {
#ifdef _UNICODE
	return u2w(utfs);
#else
	wchar_t *ws = u2w(utfs);
	char *ret = w2a(ws);
	free(ws);
	return ret;
#endif
}

char *t2u(const TCHAR *ts) {
#ifdef _UNICODE
	return w2u(ts);
#else
	wchar_t *ws = a2w(ts);
	char *ret = w2u(ws);
	free(ws);
	return ret;
#endif
}

char *u2a(const char *utfs) {
	wchar_t *ws = u2w(utfs);
	char *ret = w2a(ws);
	free(ws);
	return ret;
}

char *a2u(const char *as) {
	wchar_t *ws = a2w(as);
	char *ret = w2u(ws);
	free(ws);
	return ret;
}
