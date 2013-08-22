#include <Windows.h>
#include "lsapi.h"
#include <strsafe.h>
#include "ufmod.h"

// Constants
const TCHAR g_rcsRevision[]		= L"1.1";
const TCHAR g_szAppName[]		= L"LSXMPlayer";
const TCHAR g_szMsgHandler[]	= L"LSXMPlayerManager";
const TCHAR g_szAuthor[]		= L"Alurcard2";
const DWORD g_dwStyle			= WS_VISIBLE | WS_POPUP;
const DWORD g_dwExStyle			= WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW;

const int g_lsMessages[] = {LM_GETREVID, 0};

// Function declarations
extern "C" __declspec( dllexport ) int initModuleW(HWND hwndParent, HINSTANCE hDllInstance, LPCWSTR szPath);
extern "C" __declspec( dllexport ) void quitModule(HINSTANCE hDllInstance);
bool CreateMessageHandlers(HINSTANCE hInst);
LRESULT WINAPI MessageHandlerProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void BangPauseXM(HWND hwndCaller, LPCTSTR pszArgs);
void BangPlayXM(HWND hwndCaller, LPCSTR pszArgs);
void BangPlayXMNoLoop(HWND hwndCaller, LPCSTR pszArgs);
void BangResumeXM(HWND hwndCaller, LPCTSTR pszArgs);
void BangRestartXM(HWND hwndCalled, LPCTSTR pszArgs);
void BangSetXMVolume(HWND hwndCaller, LPCTSTR pszArgs);
void BangStopXM(HWND hwndCaller, LPCTSTR pszArgs);
void BangXMGoto(HWND hwndCaller, LPCTSTR pszArgs);

// Variables
HWND g_hwndMessageHandler = NULL;

// Implementation
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
int initModuleW(HWND, HINSTANCE hDllInstance, LPCWSTR)
{
	// Create window classes and the main window
	if (!CreateMessageHandlers(hDllInstance))
	{
		return 1;
	}

	// Register bangs
	AddBangCommand(L"!PauseXM", BangPauseXM);
	AddBangCommandA("!PlayXM", BangPlayXM);
	AddBangCommandA("!PlayXMNoLoop", BangPlayXMNoLoop);
	//AddBangCommand(L"!RestartXM", BangRestartXM);
	AddBangCommand(L"!ResumeXM", BangResumeXM);
	AddBangCommand(L"!SetXMVolume", BangSetXMVolume);
	AddBangCommand(L"!StopXM", BangStopXM);
	//AddBangCommand(L"!XMGoto", BangXMGoto);
	
	uFMOD_SetVolume(GetRCInt(L"XMPlayerVolume", 25));

	return 0; // Initialized succesfully
}

void quitModule(HINSTANCE hDllInstance)
{
	// Unregister bangs
	RemoveBangCommand(L"!PauseXM");
	RemoveBangCommandA("!PlayXM");
	RemoveBangCommandA("!PlayXMNoLoop");
	//RemoveBangCommand(L"!RestartXM");
	RemoveBangCommand(L"!ResumeXM");
	RemoveBangCommand(L"!SetXMVolume");
	RemoveBangCommand(L"!StopXM");
	//RemoveBangCommand(L"!XMGoto");

	// Stop any playing XM song
	uFMOD_StopSong();
	
	if (g_hwndMessageHandler)
	{
		// Unregister for LiteStep messages
		SendMessage(GetLitestepWnd(), LM_UNREGISTERMESSAGE, (WPARAM)g_hwndMessageHandler, (LPARAM)g_lsMessages);

		// Destroy message handling windows
		DestroyWindow(g_hwndMessageHandler);

		g_hwndMessageHandler = NULL;
	}

	// Unregister window classes
	UnregisterClass(g_szMsgHandler, hDllInstance);
}

void BangPlayXM(HWND, LPCSTR pszArgs)
{
	uFMOD_PlaySong((void*)pszArgs, NULL, XM_FILE);
}

void BangPlayXMNoLoop(HWND, LPCSTR pszArgs)
{
	uFMOD_PlaySong((void*)pszArgs, NULL, XM_FILE | XM_NOLOOP);
}


void BangPauseXM(HWND, LPCTSTR)
{
	uFMOD_Pause();
	Sleep(0);
}

void BangResumeXM(HWND, LPCTSTR)
{
	uFMOD_Resume();
	Sleep(0);
}

void BangRestartXM(HWND, LPCTSTR)
{
	uFMOD_Rewind();
}

void BangSetXMVolume(HWND, LPCTSTR pszArgs)
{
	uFMOD_SetVolume(_wtoi(pszArgs));
}

void BangStopXM(HWND, LPCTSTR)
{
	uFMOD_StopSong();
}

void BangXMGoto(HWND, LPCTSTR pszArgs)
{
	uFMOD_Jump2Pattern(_wtoi(pszArgs));
}

bool CreateMessageHandlers(HINSTANCE hInst)
{
	WNDCLASSEX wc = {0};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_NOCLOSE;
	wc.lpfnWndProc = MessageHandlerProc;
	wc.hInstance = hInst;
	wc.lpszClassName = g_szMsgHandler;
	wc.hIconSm = 0;

	if (!RegisterClassEx(&wc))
		return false; // Failed to register message handler window class

	g_hwndMessageHandler = CreateWindowEx(WS_EX_TOOLWINDOW, g_szMsgHandler, 0, WS_POPUP, 0, 0, 0, 0, 0, 0, hInst, 0);

	if (!g_hwndMessageHandler)
		return false; // Failed to create message handler window

	// Register with LiteStep to receive LM_ messages
	SendMessage(GetLitestepWnd(), LM_REGISTERMESSAGE, (WPARAM)g_hwndMessageHandler, (LPARAM)g_lsMessages);

	return true;
}

LRESULT WINAPI MessageHandlerProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case LM_GETREVID:
		{
			size_t uLength;
			StringCchPrintf((LPTSTR)lParam, 64, L"%s: %s", g_szAppName, g_rcsRevision);
			if (SUCCEEDED(StringCchLength((LPTSTR)lParam, 64, &uLength)))
				return uLength;
			lParam = NULL;
			return 0;
		}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}