#include "common.h"
#include "DebugTools.h"
#include "MiniDump.h"
#include "../../include/m_versioninfo.h"
#include "str_utils.h"
#include "options.h"
#include "resource.h"
#include "EnforceFilter.h"

#include <crtdbg.h>
#include <dbghelp.h>

#include "bzip2-1.0.3/bzlib.h"
extern "C" void compressStream ( FILE *stream, FILE *zStream );

#pragma comment ( lib, "dbghelp.lib" )

static char profile_path[MAX_PATH];

static char *version_info = 0;

// used to get plugin version
typedef PLUGININFO * (__cdecl * Miranda_Plugin_Info) ( DWORD mirandaVersion );

/////////////////////////////
///// HTTP multipart POST upload support
#include "Uploader.h"
#include "UploadSettings.h"
#include "UploadResults.h"

TCHAR *PHPErrorMessage(int code) {
	TCHAR *error = 0;
	switch(code) {
		case 1: //UPLOAD_ERR_INI_SIZE
			error = _T("The uploaded file exceeds the upload_max_filesize directive in php.ini. ");
			break;
		case 2: //UPLOAD_ERR_FORM_SIZE
			error = _T("The uploaded file exceeds the MAX_FILE_SIZE directive that was specified in the HTML form.");
			break;
		case 3: //UPLOAD_ERR_PARTIAL
			error = _T("The uploaded file was only partially uploaded.");
			break;
		case 4: //UPLOAD_ERR_NO_FILE
			error = _T("No file was uploaded.");
			break;
		case 5:
			error = _T("Server specific error.");
			break;
		case 6: //UPLOAD_ERR_NO_TMP_DIR
			error = _T("Missing a temporary folder.");
			break;
		case 7: //UPLOAD_ERR_CANT_WRITE
			error = _T("Failed to write file to disk.");
			break;
		default:
			error = _T("Unknown PHP error.");
			break;
	}
	return error;
}

BOOL CALLBACK CPDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {
		case WM_INITDIALOG:
			{
				TranslateDialogDefault(hwndDlg);
				SetWindowText(GetDlgItem(hwndDlg, IDC_LNK_BUGS), _T("http://bugs.miranda-im.org"));

				TCHAR *dump_url = (TCHAR *)lParam; // passed via lParam in DialogBoxParam call
				SetWindowText(GetDlgItem(hwndDlg, IDC_LNK_DUMP), dump_url);
			}
			return 0;
		case WM_COMMAND:
			if(HIWORD(wParam) == BN_CLICKED) {
				switch(LOWORD(wParam)) {
					case IDYES:
					case IDNO:
						EndDialog(hwndDlg, LOWORD(wParam));
						break;
					case IDC_LNK_BUGS:
					case IDC_LNK_DUMP:
						{
							char buff[512];
							GetDlgItemTextA(hwndDlg, LOWORD(wParam), buff, 512);
							CallService(MS_UTILS_OPENURL, 1, (WPARAM)buff);
						}
						break;
				}
				return TRUE;
			}
			break;

			
	}
	return 0;
}


bool HttpUpload(TCHAR *filename, TCHAR *module_filename, TCHAR *module_filename2, TCHAR *module_version, TCHAR *module_version2, PVOID address, PVOID address2) {
	
	UploadSettings settings;
	UploadResults results;

	Uploader uploader(0);

	settings.SetAddress(_T(UPLOAD_URL));
	settings.AddFile(_T("file"), filename, _T("application/x-bzip2"));
	// module signature (filename)
	if(module_filename && module_filename[0]) settings.AddField(_T("module_signature"), module_filename);
	// module signature 2 (filename)
	if(module_filename2 && module_filename2[0]) settings.AddField(_T("trace_module_signature"), module_filename2);
	// module version (from pluginInfo)
	if(module_version && module_version[0]) settings.AddField(_T("module_version"), module_version);
	// module version (from pluginInfo)
	if(module_version2 && module_version2[0]) settings.AddField(_T("trace_module_version"), module_version2);
	// reporter_id
	if(options.reporter_id[0]) settings.AddField(_T("reporter_id"), options.reporter_id);

	/*
	if(version_info) {
		TCHAR *stzVersionInfo = a2t(version_info);
		settings.AddField(_T("version_info"), stzVersionInfo);
		free(stzVersionInfo);
	}
	*/
	{
		// address
		TCHAR buff[512];
		mir_sntprintf(buff, 512, "%x", address);
		settings.AddField(_T("module_address"), buff);
		if(address2) {
			mir_sntprintf(buff, 512, "%x", address2);
			settings.AddField(_T("trace_module_address"), buff);
		}
	}
	{
		// event timestamp
		SYSTEMTIME st;
		GetSystemTime(&st);
		TCHAR buff_dt[512];
		GetDateFormat(0, 0, &st, _T("yyyy-MM-dd' '"), buff_dt, 512);
		GetTimeFormat(0, 0, &st, _T("HH:mm:ss"), buff_dt + _tcslen(buff_dt), 512 - _tcslen(buff_dt));
		settings.AddField(_T("datetime_event"), buff_dt);
	}
	{
		// windows version
		OSVERSIONINFOEX vi = {0};
		vi.dwOSVersionInfoSize = sizeof(vi);
		GetVersionEx((OSVERSIONINFO *)&vi);

		TCHAR buff[512];
		//mir_sntprintf(buff, 512, _T("%d.%d.%d-%d.%d"), vi.dwMajorVersion, vi.dwMinorVersion, vi.dwBuildNumber, vi.wServicePackMajor, vi.wServicePackMinor);
		mir_sntprintf(buff, 512, _T("%d.%d.%d"), vi.dwMajorVersion, vi.dwMinorVersion, vi.dwBuildNumber);
		settings.AddField(_T("version_os"), buff);
	}

	{
		// miranda version
		TCHAR buff[32];
		mir_sntprintf(buff, 32, _T("%d.%d.%d.%d"), (mirandaVersion >> 24) & 255, (mirandaVersion >> 16) & 255, (mirandaVersion >> 8) & 255, mirandaVersion & 255);
		settings.AddField(_T("version_miranda"), buff);

		if(ServiceExists(MS_SYSTEM_GETVERSIONTEXT)) {
			char szVer[512];
			CallService(MS_SYSTEM_GETVERSIONTEXT, 512, (LPARAM)szVer);
			settings.AddField(_T("is_unicode"), strstr(szVer, "Unicode") == 0 ? _T("0") : _T("1"));
		}
	}

	{
		// attache version
		TCHAR buff[32];
		mir_sntprintf(buff, 32, _T("%d.%d.%d.%d"), __MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM);
		settings.AddField(_T("version_attache"), buff);
	}

	uploader.DoUpload(&settings, &results);

	TCHAR *error = 0;
	if(results.GetResult() != S_OK) {
		TCHAR buff[1024];
		mir_sntprintf(buff, 1024, TranslateT("HTTP upload failed. Status code: %d"), results.GetStatusCode());
		MessageBox(0, buff, TranslateT("MiniDump Error"), MB_OK | MB_ICONERROR);
		return false;
	}
		
	int len = results.GetResponseLength();
	if(len == 0) {
		MessageBox(0, TranslateT("Upload succeeded, but there was no response from the server."), TranslateT("MiniDump Error"), MB_OK | MB_ICONERROR);
		return false;
	}

	// response format is:
	// "n URL" or "n errormsg"
	// where n is php upload error code (0-7, 5 for server specific), URL is the new url of the uploaded file (not necessarily publically accessible), and errormsg is a text error message
	unsigned long code = 0;
	char *msg;
	int i = 0;
	while(i < len && results.GetResponse()[i] >= '0' && results.GetResponse()[i] <= '9') {
		code = code * 10 + (results.GetResponse()[i++] - '0');
	}
	if(i + 1 < len) {
		msg = &results.GetResponse()[i + 1];
	} else {
		TCHAR buff[1024];
		mir_sntprintf(buff, 1024, TranslateT("Server error code: %d"), code);
		MessageBox(0, buff, TranslateT("MiniDump Error"), MB_OK | MB_ICONERROR);
		return false;
	}

	TCHAR *tmsg = a2t(msg);
	if(code != 0) {
		MessageBox(0, TranslateTS(tmsg), TranslateT("MiniDump Error"), MB_OK | MB_ICONERROR);
		free(tmsg);
		return false;
	}

	//TCHAR buff[1024];
	//mir_sntprintf(buff, 1024, TranslateT("Your dump has been successfully stored.\n\"%s\"\n\nCopy URL to clipboard?"), tmsg);
	//if(MessageBox(0, buff, TranslateT("MiniDump Upload Succeeded"), MB_YESNO) == IDNO) 
	if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CP), 0, CPDlgProc, (LPARAM)tmsg) == IDNO)
		return true;

	// copy URL to clipboard
	if(!OpenClipboard(0)) {
		MessageBox(0, TranslateT("Failed to copy URL to clipboard."), TranslateT("MiniDump Error"), MB_OK | MB_ICONERROR);
		return false;
	}
	EmptyClipboard(); 

	// get URL
	int cch = _tcslen(tmsg);

	// Allocate a global memory object for the text. 
	HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (cch + 1) * sizeof(TCHAR)); 
	if (hglbCopy == NULL) { 
		free(tmsg);
		CloseClipboard(); 
		MessageBox(0, TranslateT("Failed to copy URL to clipboard."), TranslateT("MiniDump Error"), MB_OK | MB_ICONERROR);
		return false;
	}
	// Lock the handle and copy the text to the buffer. 
	LPTSTR lptstrCopy = (LPTSTR)GlobalLock(hglbCopy); 
	memcpy(lptstrCopy, tmsg, cch * sizeof(TCHAR)); 
	lptstrCopy[cch] = (TCHAR) 0;    // null character 
	GlobalUnlock(hglbCopy); 

	free(tmsg);

	// Place the handle on the clipboard. 
	SetClipboardData(CF_TEXT, hglbCopy); 
	CloseClipboard();

	return true;
}
//////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function declarations 
//
// Author: Oleg Starodumov (www.debuginfo.com)

LONG CreateMiniDump( EXCEPTION_POINTERS* pep ); 

LONG WINAPI MyUnhandledExceptionFilter(struct _EXCEPTION_POINTERS *lpTopLevelExceptionFilter) {
	return CreateMiniDump(lpTopLevelExceptionFilter);
}

int DebugModulesLoaded(WPARAM wParam, LPARAM lParam) {
	if(ServiceExists(MS_VERSIONINFO_GETINFO)) {
		CallService(MS_VERSIONINFO_GETINFO, TRUE, (LPARAM)&version_info);
	}
	return 0;
}

void InitializeDebug() {
	// in case folders plugin unavailable
	CallService(MS_DB_GETPROFILEPATH, MAX_PATH, (WPARAM)profile_path);
	strcat(profile_path, "\\");

	SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);
	EnforceFilter(true);

	HookEvent(ME_SYSTEM_MODULESLOADED, DebugModulesLoaded);
}

void UninitializeDebug() {	
	EnforceFilter(false);
	if(version_info) {
		mmi.mmi_free(version_info);
		version_info = 0;
	}
}

#include <crtdbg.h>


///////////////////////////////////////////////////////////////////////////////
// Directives 
//

///////////////////////////////////////////////////////////////////////////////
// Custom minidump callback 
//
// Author: Oleg Starodumov (www.debuginfo.com)

BOOL CALLBACK MyMiniDumpCallback(PVOID pParam, const PMINIDUMP_CALLBACK_INPUT pInput, PMINIDUMP_CALLBACK_OUTPUT pOutput) {
	BOOL bRet = FALSE; 

	// Check parameters 
	if( pInput == 0 ) 
		return FALSE; 
	if( pOutput == 0 ) 
		return FALSE; 

	// Process the callbacks 
	switch( pInput->CallbackType ) {
		case IncludeModuleCallback: 
			// Include the module into the dump 
			bRet = TRUE; 
			break; 

		case IncludeThreadCallback: 
			// Include the thread into the dump 
			bRet = TRUE; 
			break; 

		case ModuleCallback: 
			// Does the module have ModuleReferencedByMemory flag set ? 
			if( !(pOutput->ModuleWriteFlags & ModuleReferencedByMemory) ) {
				// No, it does not - exclude it 
				wprintf( L"Excluding module: %s \n", pInput->Module.FullPath ); 
				pOutput->ModuleWriteFlags &= (~ModuleWriteModule); 
			}

			bRet = TRUE; 
			break; 

		case ThreadCallback: 
			// Include all thread information into the minidump 
			bRet = TRUE;  
			break; 

		case ThreadExCallback: 
			// Include this information 
			bRet = TRUE;  
			break; 

		case MemoryCallback: 
			// We do not include any information here -> return FALSE 
			// check pOutput base address to see if it's our crashing plugin?
			bRet = FALSE; 
			break; 

		//case CancelCallback: 
			//break; 
		default:
			break;
	}
	return bRet; 
}

///////////////////////////////////////////////////////////////////////////////
// Minidump creation function 
//
// Author: Oleg Starodumov (www.debuginfo.com)

LONG CreateMiniDump( EXCEPTION_POINTERS* pep ) {
	// Open the file 
	char szDumpFileName[MAX_PATH];
	char szDate[128], szTime[128];

	SYSTEMTIME st;
	GetSystemTime(&st);
	GetDateFormatA(LOCALE_INVARIANT, 0, &st, "yyMMdd", szDate, 128);
	GetTimeFormatA(LOCALE_INVARIANT, TIME_NOTIMEMARKER, &st, "HHmmss", szTime, 128);

	if(ServiceExists(MS_FOLDERS_GET_PATH)) {
		char *adp = t2a(stzDumpPath);
		strcpy(szDumpFileName, adp);
		strcat(szDumpFileName, "\\");
	} else {
		strcpy(szDumpFileName, profile_path);
	}

	strcat(szDumpFileName, "mim-");
	strcat(szDumpFileName, szDate);
	strcat(szDumpFileName, szTime);
	strcat(szDumpFileName, ".dmp");

	// get the filename and version of the module that had the exception
	TCHAR szModuleFilePath[MAX_PATH], szModuleFileName[MAX_PATH], szModuleFileName2[MAX_PATH], szVersion[32], szVersion2[32];
	szModuleFileName[0] = 0;
	szModuleFileName2[0] = 0;
	szVersion[0] = 0;
	szVersion2[0] = 0;
	PVOID address = pep->ExceptionRecord->ExceptionAddress, address2 = 0;

	HINSTANCE hInstance; 
	MEMORY_BASIC_INFORMATION mbi;
	if(VirtualQuery( pep->ExceptionRecord->ExceptionAddress, &mbi, sizeof(mbi) )) {
		hInstance = (HINSTANCE)(mbi.AllocationBase);

		if(GetModuleFileName(hInstance, szModuleFilePath, MAX_PATH)) {
			// strip path
			TCHAR *p = _tcsrchr(szModuleFilePath, _T('\\'));
			if(p && *p) {
				p += 1;
				_tcscpy(szModuleFileName, p);
			} else
				_tcscpy(szModuleFileName, szModuleFilePath);
		}

		// get plugin version if possible
		Miranda_Plugin_Info Info = (Miranda_Plugin_Info) GetProcAddress(hInstance, "MirandaPluginInfo");
		if(Info) {
			PLUGININFO * pi = Info(mirandaVersion);
			if(pi) {
				mir_sntprintf(szVersion, 32, "%d.%d.%d.%d", (pi->version >> 24) & 255, (pi->version >> 16) & 255, (pi->version >> 8) & 255, pi->version & 255);
			}
		}
	}

	// couldn't get a plugin version from the address at the end of the stack trace - walk up the stack to see if we can get one earlier
	if(szVersion[0] == 0) {
		struct _EXCEPTION_RECORD *ex = pep->ExceptionRecord->ExceptionRecord;
		while(ex && VirtualQuery( ex->ExceptionAddress, &mbi, sizeof(mbi) )) {
			hInstance = (HINSTANCE)(mbi.AllocationBase);

			if(GetModuleFileName(hInstance, szModuleFilePath, MAX_PATH)) {
				// strip path
				TCHAR *p = _tcsrchr(szModuleFilePath, _T('\\'));
				if(p && *p) {
					p += 1;
					_tcscpy(szModuleFileName2, p);
				} else
					_tcscpy(szModuleFileName2, szModuleFilePath);
			}

			// get plugin version if possible
			Miranda_Plugin_Info Info = (Miranda_Plugin_Info) GetProcAddress(hInstance, "MirandaPluginInfo");
			if(Info) {
				PLUGININFO * pi = Info(mirandaVersion);
				if(pi) {
					address2 = ex->ExceptionAddress;
					mir_sntprintf(szVersion2, 32, "%d.%d.%d.%d", (pi->version >> 24) & 255, (pi->version >> 16) & 255, (pi->version >> 8) & 255, pi->version & 255);
					break;
				}
			}

			ex = ex->ExceptionRecord;
		}
	}

	HANDLE hFile = CreateFileA(szDumpFileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ); 

	if( ( hFile != NULL ) && ( hFile != INVALID_HANDLE_VALUE ) ) {
		// Create the minidump 
		MINIDUMP_EXCEPTION_INFORMATION mdei; 
		mdei.ThreadId           = GetCurrentThreadId(); 
		mdei.ExceptionPointers  = pep; 
		mdei.ClientPointers     = FALSE; 

		MINIDUMP_CALLBACK_INFORMATION mci; 
		mci.CallbackRoutine     = (MINIDUMP_CALLBACK_ROUTINE)MyMiniDumpCallback; 
		mci.CallbackParam       = 0; 

		MINIDUMP_TYPE mdt       = (MINIDUMP_TYPE)(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory | MiniDumpFilterMemory); 

		if(!MiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(), hFile, mdt, (pep != 0) ? &mdei : 0, 0, &mci )) {
			TCHAR buff[512];
			mir_sntprintf(buff, 512, _T("Crash in module: %s\nMiniDumpWriteDump failed. Error: %u"), szModuleFileName, GetLastError());
			MessageBox(0, buff, _T("MiniDump Error"), MB_OK | MB_ICONWARNING);
			CloseHandle( hFile ); 
			return EXCEPTION_EXECUTE_HANDLER;
		}

		// Close the file 
		CloseHandle( hFile ); 

		// bzip the file
		char szBzipFilename[MAX_PATH];
		strcpy(szBzipFilename, szDumpFileName);
		strcat(szBzipFilename, ".bz2");
		FILE *fin = fopen(szDumpFileName, "rb"), *fout = fopen(szBzipFilename, "w+b");
		if(fin) {
			if(fout) {
				compressStream(fin, fout);
				DeleteFileA(szDumpFileName);
			} else
				fclose(fin);
		}

		TCHAR *stzBzipFilename = a2t(szBzipFilename);

		TCHAR msg[1024];
		mir_sntprintf(msg, 1024, TranslateT("MiniDump crash dump created.\nCrash in module: %s\n\nUpload to miranda-im.org?"), szModuleFileName, stzBzipFilename);

		// upload?
		if(options.upload && (!options.confirm_upload || 
			MessageBox(0, msg, TranslateT("Attache - Crash Dump"), MB_YESNO | MB_ICONERROR) == IDYES)) 
		{
			// yes...
			if(HttpUpload(stzBzipFilename, szModuleFileName, szModuleFileName2, szVersion, szVersion2, address, address2) && options.delete_uploaded)
				DeleteFile(stzBzipFilename);
		} else {
			mir_sntprintf(msg, 1024, TranslateT("Crash in module: %s\nDump file: %s"), szModuleFileName, stzBzipFilename);
			MessageBox(0, msg, TranslateT("MiniDump - Crash Dump Created"), MB_OK | MB_ICONERROR);
		}

		free(stzBzipFilename);
	} else {
		TCHAR buff[512];
		mir_sntprintf(buff, 512, _T("Crash in module: %s\nMiniDumpWriteDump failed. Failed to create dump file."), szModuleFileName, GetLastError());
		MessageBox(0, buff, _T("MiniDump Error"), MB_OK | MB_ICONWARNING); 
		CloseHandle( hFile ); 
	}

	return EXCEPTION_EXECUTE_HANDLER;
}
