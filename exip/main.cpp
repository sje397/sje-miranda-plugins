#include "common.h"
#include "options.h"
#include "getip.h"

HINSTANCE hInst;
PLUGINLINK *pluginLink;

HANDLE hNetlibUser = 0;
HANDLE mainThread;

// plugin stuff
PLUGININFO pluginInfo={
	sizeof(PLUGININFO),
	"ExIP",
	PLUGIN_MAKE_VERSION(0, 0, 0, 1),
	"Retreive external IP address",
	"Scott Ellis",
	"mail@scottellis.com.au",
	"© 2005 Scott Ellis",
	"http://www.scottellis.com.au/",
	0,		//not transient
	0		//doesn't replace anything built-in
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	return &pluginInfo;
}





unsigned long __stdcall GetIPThread(void *param) {
	RetreiveIP();
	return 0;
}

/*
bool power_message = false;
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message) 
	{
		case WM_COMMAND:
			break;
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code here...
			RECT rt;
			GetClientRect(hWnd, &rt);
			if(power_message)
				DrawText(hdc, "Power message!", strlen("Power message!"), &rt, DT_CENTER);
			else
				DrawText(hdc, "Hello world!", strlen("Hello world!"), &rt, DT_CENTER);
			EndPaint(hWnd, &ps);
			break;
		case WM_DESTROY:
		case WM_CLOSE:
			DestroyWindow(hWnd);
			break;

		case WM_POWERBROADCAST:
			power_message = true;
			InvalidateRect(hWnd, 0, TRUE);
			break;


		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= 0; //LoadIcon(hInstance, (LPCTSTR)IDI_SCREENDUMP);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= 0;//(LPCSTR)IDC_SCREENDUMP;
	wcex.lpszClassName	= "TestPowerMessagesWindowClass";
	wcex.hIconSm		= 0;//LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

DWORD CALLBACK MessagePumpThread(LPVOID param) {
	CallService(MS_SYSTEM_THREAD_PUSH, 0, 0);

	MyRegisterClass(hInst);

	HWND hWnd = CreateWindow("TestPowerMessagesWindowClass", "Test", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInst, NULL);
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	MSG msg;
	// GetMessage returns -1 when hwndProgress is an invalid handle - so it exits when that window is destroyed
	while(GetMessage(&msg, hWnd, 0, 0) > 0 && !Miranda_Terminated()) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	CallService(MS_SYSTEM_THREAD_POP, 0, 0);
	return 0;
}
*/

int OnModulesLoaded(WPARAM wParam, LPARAM lParam) {

	NETLIBUSER nl_user = {0};
	nl_user.cbSize = sizeof(nl_user);
	nl_user.szSettingsModule = MODULE;
	nl_user.szDescriptiveName = "External IP";
	nl_user.flags = NUF_OUTGOING | NUF_HTTPCONNS;

	hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nl_user);

	DWORD tid;
	CloseHandle(CreateThread(0, 0, GetIPThread, 0, 0, &tid));

	//CloseHandle(CreateThread(0, 0, MessagePumpThread, 0, 0, &tid));

	return 0;
}

extern "C" __declspec(dllexport) int Load(PLUGINLINK *link)
{
	pluginLink=link;
	DuplicateHandle( GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &mainThread, THREAD_SET_CONTEXT, FALSE, 0 );

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);	

	HookEvent(ME_OPT_INITIALISE, OptInit );

	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	CallService(MS_NETLIB_CLOSEHANDLE, (WPARAM)hNetlibUser, 0);
	return 0;
}
