#include "stdafx.h"
#include "Shutdown.h"
#include "ShutdownDlg.h"

CShutdownApp theApp;

BOOL CShutdownApp::InitInstance()
{
	//==================================================================
	//Creating mutex for allowing only one instance at the time.
	//If mutex already exists - find window and show it, then return.
	//==================================================================

	CreateMutexW(nullptr, FALSE, m_lpszClassName);
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		HWND hWnd = FindWindowW((LPCWSTR)m_ClassAtom, L"Shutdown");
		if (hWnd) {
			ShowWindow(hWnd, SW_SHOWNORMAL);
			SetForegroundWindow(hWnd);
		}
		return FALSE;
	}

	//===================================================================
	//Creating invisible - "main" - window and making it as parent
	//to our dialog window - to prevent dialog from having taskbar entry.
	//Dialog box must have WS_EX_APPWINDOW style set to false.
	//===================================================================
	WNDCLASSEXW stClass { sizeof(WNDCLASSEXW), 0, 0, 0, 0, AfxGetInstanceHandle(), 0, 0, 0, 0, m_lpszClassName, 0 };
	m_ClassAtom = RegisterClassExW(&stClass);
	CWnd wndMain;
	wndMain.CreateEx(0, (LPCWSTR)m_ClassAtom, nullptr, 0, 0, 0, 0, 0, nullptr, nullptr);

	CShutdownDlg dlg(&wndMain);
	m_pMainWnd = &dlg;
	dlg.DoModal();

	return FALSE;
}