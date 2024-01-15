#include "stdafx.h"
#include "ShutdownDlg.h"

class CShutdownApp : public CWinApp {
public:
	BOOL InitInstance()override;
private:
	//"Unique" class name, also used as a CLASS in the Shutdown.rc
	LPCWSTR m_pwszClassName { L"Shutdown{34754674}" };
};

CShutdownApp theApp;

BOOL CShutdownApp::InitInstance()
{
	//Allowing only one App instance.
	if (const auto hWnd = FindWindowExW(nullptr, nullptr, m_pwszClassName, nullptr); hWnd) {
		ShowWindow(hWnd, SW_SHOWNORMAL);
		SetForegroundWindow(hWnd);
		return FALSE;
	}

	WNDCLASSEX wndcls { sizeof(WNDCLASSEX) };
	::GetClassInfoExW(NULL, MAKEINTRESOURCEW(32770), &wndcls);
	wndcls.lpszClassName = m_pwszClassName;
	if (!::RegisterClassExW(&wndcls)) {
		_ASSERTE("RegisterClassExW");
		return FALSE;
	}

	CShutdownDlg dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();

	return FALSE;
}