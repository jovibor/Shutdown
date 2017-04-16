#include "stdafx.h"
#include "ShutdownDlg.h"
#include "ShutdownAboutdlg.h"

#define WM_TRAYICON_MESSAGE (WM_APP + 1)

void CShutdownDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_EDIT_HOURS, m_edit_hours);
	DDX_Control(pDX, IDC_EDIT_MINUTES, m_edit_minutes);
}

BEGIN_MESSAGE_MAP(CShutdownDlg, CDialog)
	ON_BN_CLICKED(IDC_SETUP_BUTTON, OnSetupButton)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_SYSCOMMAND()
	ON_MESSAGE(WM_TRAYICON_MESSAGE, OnSystrayIconMessage)
	ON_MESSAGE(WM_HOTKEY, OnHotKey)
	ON_COMMAND(IDC_POPUP_SHOW, OnPopupShow)
	ON_COMMAND(IDC_POPUP_EXIT, OnPopupExit)
	ON_COMMAND(IDC_POPUP_RESETTIMER, OnPopupResetTimer)
	ON_WM_CTLCOLOR()
	ON_WM_DRAWITEM()
	ON_WM_MEASUREITEM()
END_MESSAGE_MAP()

BOOL CShutdownDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	RegisterHotKey(m_hWnd, 1, MOD_CONTROL | MOD_ALT, 0x53); //ALT+CTRL+S - to shutdown immediately

	memset(&m_systray_icon, 0, sizeof(m_systray_icon));
	m_systray_icon.cbSize = sizeof(NOTIFYICONDATA);
	m_systray_icon.hIcon = m_hIcon;
	m_systray_icon.hWnd = m_hWnd;
	m_systray_icon.uCallbackMessage = WM_TRAYICON_MESSAGE;
	m_systray_icon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	m_systray_icon.uID = 1;
	_tcscpy_s(m_systray_icon.szTip, TEXT("No time set"));
	Shell_NotifyIcon(NIM_ADD, &m_systray_icon);

	m_spin_hours.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | UDS_ALIGNRIGHT | UDS_SETBUDDYINT | UDS_ARROWKEYS | UDS_WRAP,
		CRect(0, 0, 0, 0), this, IDC_SPIN_HOURS);
	m_spin_hours.SetBuddy(&m_edit_hours);
	m_spin_hours.SetRange(0, 23);
	m_spin_hours.SetPos(1);

	m_spin_minutes.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | UDS_ALIGNRIGHT | UDS_SETBUDDYINT | UDS_ARROWKEYS | UDS_WRAP,
		CRect(0, 0, 0, 0), this, IDC_SPIN_MINUTES);
	m_spin_minutes.SetBuddy(&m_edit_minutes);
	m_spin_minutes.SetRange(0, 59);
	m_spin_minutes.SetPos(0);

	UDACCEL accel[2];
	accel[0].nSec = 0; accel[0].nInc = 1;
	accel[1].nSec = 1; accel[1].nInc = 8;
	m_spin_minutes.SetAccel(2, accel);	//acceleration of minutes_spin

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	pSysMenu->DeleteMenu(0, MF_BYPOSITION);
	pSysMenu->DeleteMenu(1, MF_BYPOSITION);
	pSysMenu->DeleteMenu(2, MF_BYPOSITION);
	pSysMenu->AppendMenu(MF_SEPARATOR);
	pSysMenu->AppendMenu(MF_ENABLED, IDC_MENU_ABOUT, TEXT("About..."));

	m_systray_menu.CreatePopupMenu();
	m_systray_menu.AppendMenu(MF_OWNERDRAW, IDC_POPUP_SHOW, _TEXT("Main window"));
	m_systray_menu.AppendMenu(MF_OWNERDRAW | MF_DISABLED, IDC_POPUP_RESETTIMER, _TEXT("Reset timer"));
	m_systray_menu.AppendMenu(MF_OWNERDRAW, IDC_POPUP_EXIT, _TEXT("Exit"));

	m_hbr_black = CreateSolidBrush(RGB(0, 0, 0));
	m_hbr_white = CreateSolidBrush(RGB(255, 255, 255));

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	return TRUE;
}

void CShutdownDlg::ShutdownPC()
{
	TOKEN_PRIVILEGES tkp;
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	LookupPrivilegeValue(nullptr, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);

	HANDLE hToken;
	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, nullptr, nullptr);
	InitiateSystemShutdownEx(nullptr, nullptr, NULL, TRUE, FALSE, SHTDN_REASON_FLAG_PLANNED);
	DestroyWindow();
}

void CShutdownDlg::OnSetupButton()
{
	KillTimer(m_timer_id);

	if((m_time_total = (m_spin_hours.GetPos() * 60) + m_spin_minutes.GetPos()) == 0)
	{
		ShutdownPC();
		return;
	}

	m_timer_id = SetTimer(1, 60000, nullptr);
	m_systray_menu.EnableMenuItem(IDC_POPUP_RESETTIMER, MF_ENABLED);
	ShowWindow(SW_HIDE);
	UpdateTooltipText();
}

void CShutdownDlg::OnTimer(UINT nIDEvent)
{
	if(--m_time_total == 0)
	{
		ShutdownPC();
		return;
	}

	UpdateTooltipText();

	if(m_time_total == 1)	//warn user for one minute remain to shut down 
	{
		Beep(15000, 350);
		if(IDCANCEL == MessageBox(TEXT("Warning! Your system is about to shut down in less than 1 minute!\n Press \"Cancel\" to prevent it."),
			TEXT("SHUTDOWN Warning!!!"), MB_OKCANCEL | MB_ICONWARNING | MB_TOPMOST | MB_SETFOREGROUND))
			OnPopupResetTimer();
	}
}

LRESULT CShutdownDlg::OnSystrayIconMessage(WPARAM wParam, LPARAM lParam)
{
	switch(lParam)
	{
	case WM_LBUTTONDBLCLK:
		ShowWindow(SW_SHOW);
		break;
	case WM_RBUTTONUP:
		POINT cur;
		GetCursorPos(&cur);
		SetForegroundWindow();
		m_systray_menu.TrackPopupMenu(TPM_LEFTALIGN, cur.x, cur.y, this);
		break;
	}
	return 0;
}

void CShutdownDlg::OnSysCommand(UINT nID, LPARAM wParam)
{
	switch(nID)
	{
	case SC_MINIMIZE:
		ShowWindow(SW_HIDE);
		break;
	case IDC_MENU_ABOUT:{
		CShutdownAboutDlg about_dlg;
		about_dlg.DoModal();
		break;
	}
	default:
		CDialog::OnSysCommand(nID, wParam);
	}
}

void CShutdownDlg::OnPopupExit()
{
	DestroyWindow();
}

void CShutdownDlg::OnPopupResetTimer()
{
	KillTimer(m_timer_id);
	_tcscpy_s(m_systray_icon.szTip, TEXT("No time set"));
	Shell_NotifyIcon(NIM_MODIFY, &m_systray_icon);
	m_systray_menu.EnableMenuItem(IDC_POPUP_RESETTIMER, MF_DISABLED);
}

void CShutdownDlg::OnPopupShow()
{
	ShowWindow(SW_SHOW);
}

LRESULT CShutdownDlg::OnHotKey(WPARAM wParam, LPARAM lParam)
{
	ShutdownPC();
	return 0;
}

BOOL CShutdownDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		OnSetupButton();
		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CShutdownDlg::OnDestroy()
{
	Shell_NotifyIcon(NIM_DELETE, &m_systray_icon);
	UnregisterHotKey(m_hWnd, 1);
	m_systray_menu.DestroyMenu();
	DeleteObject(m_hbr_black);
	DeleteObject(m_hbr_white);
}

//************************************
// Method:    OnCtlColor
// Change Dlg background color here
//************************************
HBRUSH CShutdownDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// Check for edit-boxes first, we need their bkcolor be white,
	// the rest dialog is black.	
	if(pWnd->GetDlgCtrlID() == IDC_EDIT_HOURS || pWnd->GetDlgCtrlID() == IDC_EDIT_MINUTES)
		return m_hbr_white;

	pDC->SetTextColor(RGB(255, 255, 255));
	pDC->SetBkColor(RGB(0, 0, 0));

	return m_hbr_black;
}

void CShutdownDlg::UpdateTooltipText()
{
	if(m_time_total / 60 > 0)
		swprintf_s(m_systray_icon.szTip, 128, TEXT("%u%s%u%s"), m_time_total / 60, TEXT(" hours, "), m_time_total % 60, TEXT(" minutes to shutdown"));
	else
		swprintf_s(m_systray_icon.szTip, 128, TEXT("%u%s"), m_time_total, TEXT(" minutes to shutdown"));

	Shell_NotifyIcon(NIM_MODIFY, &m_systray_icon);
}

void CShutdownDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDIS)
{
	CDC *pDC = CDC::FromHandle(lpDIS->hDC);

	switch(lpDIS->CtlType)
	{
	case ODT_MENU:
	{
		if(lpDIS->itemAction & ODA_DRAWENTIRE)
		{
			pDC->FillSolidRect(&lpDIS->rcItem, RGB(0, 0, 0));
			pDC->SetBkColor(RGB(0, 0, 0));

			if(lpDIS->itemState & ODS_DISABLED)
				pDC->SetTextColor(RGB(100, 100, 100));
			else
				pDC->SetTextColor(RGB(255, 255, 255));

			lpDIS->rcItem.left = 10;
			pDC->DrawText(reinterpret_cast<LPCTSTR>(lpDIS->itemData), &lpDIS->rcItem, DT_SINGLELINE | DT_VCENTER | DT_NOCLIP);
		}
		if((lpDIS->itemAction & ODA_SELECT) && (lpDIS->itemState & ODS_SELECTED))
			pDC->FrameRect(&lpDIS->rcItem, &CBrush(RGB(255, 255, 255)));
		else
			pDC->FrameRect(&lpDIS->rcItem, &CBrush(RGB(0, 0, 0)));
		break;
	}
	case ODT_BUTTON:
	{
		TCHAR buff[128];
		::GetWindowText(lpDIS->hwndItem, buff, 128);

		pDC->FillSolidRect(&lpDIS->rcItem, RGB(0, 0, 0)); //Button color
		pDC->DrawEdge(&lpDIS->rcItem, EDGE_RAISED, BF_RECT);
		pDC->SetTextColor(RGB(255, 255, 255));
		pDC->DrawText(buff, &lpDIS->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

		if(lpDIS->itemState & ODS_FOCUS)       // If the button has focus
		{
			if(lpDIS->itemState & ODS_SELECTED)
				pDC->DrawEdge(&lpDIS->rcItem, EDGE_SUNKEN, BF_RECT);    // Draw a sunken face

			lpDIS->rcItem.top += 4;	lpDIS->rcItem.left += 4;
			lpDIS->rcItem.right -= 4; lpDIS->rcItem.bottom -= 4;

			pDC->DrawFocusRect(&lpDIS->rcItem);
		}
		break;
	}
	}
}

void CShutdownDlg::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMIS)
{
	if(lpMIS->CtlType == ODT_MENU)
	{
		lpMIS->itemWidth = 85;
		lpMIS->itemHeight = 20;
	}
}