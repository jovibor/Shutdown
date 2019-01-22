#include "stdafx.h"
#include "Shutdown.h"
#include "ShutdownDlg.h"

CShutdownApp theApp;

BOOL CShutdownApp::InitInstance()
{
	//==================================================================
	//Creating mutex for only one instance allowing at time.
	//If mutex already exist - find window and show it, then return.
	//==================================================================

	CreateMutex(nullptr, FALSE, L"ShutDown-34754674");

	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		HWND hWnd = FindWindow(nullptr, L"Shutdown");
		ShowWindow(hWnd, SW_SHOWNORMAL);
		SetForegroundWindow(hWnd);
		return FALSE;
	}

	//===================================================================
	//Creating invisible - "main" - window and making it as parent
	//to our dialog window - to prevent dialog from having taskbar entry.
	//Dialog box must have WS_EX_APPWINDOW style set to false.
	//===================================================================
	CWnd wndMain;
	wndMain.CreateEx(0, nullptr, nullptr, 0, CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr);

	CShutdownDlg dlg(&wndMain);
	m_pMainWnd = &dlg;

	dlg.DoModal();

	return FALSE;
}