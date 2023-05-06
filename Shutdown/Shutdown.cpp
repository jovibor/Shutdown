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

	const auto hMutex = CreateMutexW(nullptr, FALSE, m_pwszClassName);
	if (!hMutex || GetLastError() == ERROR_ALREADY_EXISTS) {
		const auto hWnd = FindWindowExW(nullptr, nullptr, nullptr, L"Shutdown");
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
	const WNDCLASSEXW stClass { sizeof(WNDCLASSEXW), 0, nullptr, 0, 0, AfxGetInstanceHandle(),
		nullptr, nullptr, nullptr, nullptr, m_pwszClassName, nullptr };
	RegisterClassExW(&stClass);
	CWnd wndMain;
	wndMain.CreateEx(0, m_pwszClassName, nullptr, WS_POPUP, 0, 0, 0, 0, nullptr, nullptr);

	CShutdownDlg dlg(&wndMain);
	m_pMainWnd = &dlg;
	dlg.DoModal();

	ReleaseMutex(hMutex);

	return FALSE;
}