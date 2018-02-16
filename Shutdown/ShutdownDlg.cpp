#include "stdafx.h"
#include "ShutdownDlg.h"
#include "Aboutdlg.h"
#include <thread>

#define WM_TRAYICON_MESSAGE (WM_APP + 1)
#define WM_TRAYICONID 1

#define TIMER_SHUTDOWN 1
#define TIMER_TOOLTIP 2

#define TEXT_NOTIME TEXT("No time set")

void CShutdownDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_EDIT_HOURS, m_edit_hours);
	DDX_Control(pDX, IDC_EDIT_MINUTES, m_edit_minutes);
}

BEGIN_MESSAGE_MAP(CShutdownDlg, CDialog)
	ON_BN_CLICKED(IDC_SETUP_BUTTON, OnSetupButton)
	ON_BN_CLICKED(IDC_MIDNIGHT_BUTTON, OnMidnightButton)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_SYSCOMMAND()
	ON_MESSAGE(WM_TRAYICON_MESSAGE, OnSystrayIconMessage)
	ON_MESSAGE(WM_HOTKEY, OnHotKey)
	ON_COMMAND(IDC_SYSTRAYMENU_SHOW, OnSystrayMenuShow)
	ON_COMMAND(IDC_SYSTRAYMENU_EXIT, OnSystrayMenuExit)
	ON_COMMAND(IDC_SYSTRAYMENU_RESETTIMER, OnSystrayMenuResetTimer)
	ON_WM_CTLCOLOR()
	ON_WM_DRAWITEM()
	ON_WM_MEASUREITEM()
END_MESSAGE_MAP()

BOOL CShutdownDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	RegisterHotKey(m_hWnd, 1, MOD_CONTROL | MOD_ALT, 0x53); //ALT+CTRL+S - to shutdown immediately

	m_systray_icon.cbSize = sizeof(NOTIFYICONDATA);
	m_systray_icon.hWnd = m_hWnd;
	m_systray_icon.uID = WM_TRAYICONID;
	m_systray_icon.uFlags = NIF_ICON | NIF_MESSAGE;
	m_systray_icon.uCallbackMessage = WM_TRAYICON_MESSAGE;
	m_systray_icon.hIcon = m_hIcon;

	Shell_NotifyIcon(NIM_ADD, &m_systray_icon);

	//For acquiring icon rect 
	m_systray_iconid.cbSize = sizeof(NOTIFYICONIDENTIFIER);
	m_systray_iconid.hWnd = m_hWnd;
	m_systray_iconid.uID = WM_TRAYICONID;
	m_systray_iconid.guidItem = GUID_NULL;

	//Tooltip window
	m_tooltip_hwnd = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
		TTS_BALLOON | TTS_NOPREFIX | TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, NULL, NULL);

	_tcscpy_s(m_tooltip_text, TEXT_NOTIME);

	//Initializing tooltip
	m_ti.cbSize = TTTOOLINFO_V1_SIZE;
	m_ti.uFlags = TTF_TRACK;
	m_ti.uId = WM_TRAYICONID;
	m_ti.lpszText = m_tooltip_text;

	//Prepearing tooltip appearance
	::SendMessage(m_tooltip_hwnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&m_ti);
	::SendMessage(m_tooltip_hwnd, TTM_SETTIPBKCOLOR, (WPARAM)RGB(0, 0, 0), 0);
	::SendMessage(m_tooltip_hwnd, TTM_SETTIPTEXTCOLOR, (WPARAM)RGB(255, 255, 255), 0);
	::SendMessage(m_tooltip_hwnd, TTM_SETTITLE, (WPARAM)TTI_NONE, (LPARAM)TEXT("Time to shutdown:"));

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
	m_systray_menu.AppendMenu(MF_OWNERDRAW, IDC_SYSTRAYMENU_SHOW, _TEXT("Main window"));
	m_systray_menu.AppendMenu(MF_OWNERDRAW | MF_DISABLED, IDC_SYSTRAYMENU_RESETTIMER, _TEXT("Reset timer"));
	m_systray_menu.AppendMenu(MF_OWNERDRAW, IDC_SYSTRAYMENU_EXIT, _TEXT("Exit"));

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
	m_time_total = (m_spin_hours.GetPos() * 60) + m_spin_minutes.GetPos();
	m_systray_menu.EnableMenuItem(IDC_SYSTRAYMENU_RESETTIMER, MF_ENABLED);
	ShowWindow(SW_HIDE);
	SetTimer(TIMER_SHUTDOWN, 60000, nullptr);

	OnTimer(TIMER_SHUTDOWN);
}

void CShutdownDlg::OnMidnightButton()
{
	time_t t = time(0);
	tm now;
	localtime_s(&now, &t);

	//minutes left to midnight
	m_time_total = (24 - now.tm_hour) * 60 - now.tm_min;

	m_systray_menu.EnableMenuItem(IDC_SYSTRAYMENU_RESETTIMER, MF_ENABLED);
	ShowWindow(SW_HIDE);
	SetTimer(TIMER_SHUTDOWN, 60000, nullptr);

	OnTimer(TIMER_SHUTDOWN);
}

void CShutdownDlg::OnTimer(UINT nIDEvent)
{
	if (nIDEvent == TIMER_SHUTDOWN)
	{
		UpdateTooltipText();

		switch (m_time_total) {//time in minutes to shutdown
		case 0:
			ShutdownPC();
			break;
		case 1:
		{
			m_time_total--;

			std::thread beep(Beep, 15000, 350); //calling Beep() in async state->
			beep.detach(); //->and detaching immidiately 

			INT_PTR dlg_rtrnvalue = m_lastmin_dlg.DoModal();

			if (dlg_rtrnvalue == IDCANCEL)
				ResetTimer();
			else if (dlg_rtrnvalue == IDC_POSTPONE_BUTTON)
			{
				m_time_total = 30;
				SetTimer(TIMER_SHUTDOWN, 60000, nullptr); //resetting the timer to 30 min.

				OnTimer(TIMER_SHUTDOWN);
			}
			break;
		}

		default:
			m_time_total--;
		}

	}
	else if (nIDEvent == TIMER_TOOLTIP)
	{
		POINT cur;
		GetCursorPos(&cur);
		Shell_NotifyIconGetRect(&m_systray_iconid, &m_systray_iconrect);

		//Checking if mouse pointer is within systrayicon rect
		//if not — killing tooltip window
		if (!(cur.x >= m_systray_iconrect.left && cur.x <= m_systray_iconrect.right && cur.y >= m_systray_iconrect.top && cur.y <= m_systray_iconrect.bottom))
		{
			KillTimer(TIMER_TOOLTIP);
			mf_tooltip = FALSE;
			::SendMessage(m_tooltip_hwnd, TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)(LPTOOLINFO)&m_ti);
		}
	}
}

LRESULT CShutdownDlg::OnSystrayIconMessage(WPARAM wParam, LPARAM lParam)
{
	switch (lParam)
	{
	case WM_LBUTTONDBLCLK:
		ShowWindow(SW_SHOW);
		break;
	case WM_RBUTTONDOWN:
		::SendMessage(m_tooltip_hwnd, TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)(LPTOOLINFO)&m_ti);//disabling tooltip
		break;
	case WM_RBUTTONUP:
		POINT cur;
		GetCursorPos(&cur);
		SetForegroundWindow();
		m_systray_menu.TrackPopupMenu(TPM_LEFTALIGN, cur.x, cur.y, this);
		break;
	case WM_MOUSEMOVE:
		//For some undiscovered reason when systray icon just created 
		//it sends WM_MOUSEMOVE message to main window (even if mouse is far away from systray)
		//which forces tooltip window to show up at the current cursor pos. 
		//This flag is just to prevent it. 
		if (mf_first_time_tooltip)
		{
			mf_first_time_tooltip = FALSE;
			return 0;
		}

		//Showing tooltip window if it's already not.
		//Then in timer proc checking whether cursor pos is out of systray icon rect.
		//Windows can not do it by itself.
		if (!mf_tooltip)
		{
			mf_tooltip = TRUE;

			Shell_NotifyIconGetRect(&m_systray_iconid, &m_systray_iconrect);

			//Position tooltip in icon's center
			::SendMessage(m_tooltip_hwnd, TTM_TRACKPOSITION, 0,
				(LPARAM)MAKELONG((m_systray_iconrect.right - m_systray_iconrect.left) / 2 + m_systray_iconrect.left,
				(m_systray_iconrect.bottom - m_systray_iconrect.top) / 2 + m_systray_iconrect.top));
			//Show tooltip window
			::SendMessage(m_tooltip_hwnd, TTM_TRACKACTIVATE, (WPARAM)TRUE, (LPARAM)(LPTOOLINFO)&m_ti);

			//every 200ms checking whether cursor is still hovering systray icon
			SetTimer(TIMER_TOOLTIP, 200, NULL);
		}
		break;
	}

	return 0;
}

void CShutdownDlg::OnSysCommand(UINT nID, LPARAM wParam)
{
	switch (nID)
	{
	case SC_MINIMIZE:
		ShowWindow(SW_HIDE);
		break;
	case IDC_MENU_ABOUT: {
		CAboutDlg about_dlg;
		about_dlg.DoModal();
		break;
	}
	default:
		CDialog::OnSysCommand(nID, wParam);
	}
}

void CShutdownDlg::OnSystrayMenuExit()
{
	DestroyWindow();
}

void CShutdownDlg::OnSystrayMenuResetTimer()
{
	ResetTimer();
	m_lastmin_dlg.EndDialog(-1); //destroying lastmin dlg if exist
}

void CShutdownDlg::OnSystrayMenuShow()
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
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		OnSetupButton();
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CShutdownDlg::OnDestroy()
{
	KillTimer(TIMER_SHUTDOWN);

	Shell_NotifyIcon(NIM_DELETE, &m_systray_icon);
	UnregisterHotKey(m_hWnd, 1);
	m_systray_menu.DestroyMenu();
	DeleteObject(m_hbr_black);
	DeleteObject(m_hbr_white);

	::DestroyWindow(m_tooltip_hwnd);
}

//************************************
// Method:    OnCtlColor
// Change Dlg background color here
//************************************
HBRUSH CShutdownDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// Check for edit-boxes first, we need their bkcolor as white,
	// the rest dialog is black.	
	if (pWnd->GetDlgCtrlID() == IDC_EDIT_HOURS || pWnd->GetDlgCtrlID() == IDC_EDIT_MINUTES)
		return m_hbr_white;

	pDC->SetTextColor(RGB(255, 255, 255));
	pDC->SetBkColor(RGB(0, 0, 0));

	return m_hbr_black;
}

void CShutdownDlg::UpdateTooltipText()
{
	if (m_time_total % 60 > 9)
		swprintf_s(m_tooltip_text, 20, TEXT("%u%s%u%s"), m_time_total / 60, TEXT(":"), m_time_total % 60, TEXT(" remaining"));
	else
		swprintf_s(m_tooltip_text, 20, TEXT("%u%s%u%s"), m_time_total / 60, TEXT(":0"), m_time_total % 60, TEXT(" remaining"));

	m_ti.lpszText = m_tooltip_text;
	::SendMessage(m_tooltip_hwnd, TTM_UPDATETIPTEXT, 0, (LPARAM)(LPTOOLINFO)&m_ti);
}

void CShutdownDlg::ResetTimer()
{
	KillTimer(TIMER_SHUTDOWN);

	_tcscpy_s(m_tooltip_text, TEXT_NOTIME);
	m_ti.lpszText = m_tooltip_text;
	::SendMessage(m_tooltip_hwnd, TTM_UPDATETIPTEXT, 0, (LPARAM)(LPTOOLINFO)&m_ti);

	m_systray_menu.EnableMenuItem(IDC_SYSTRAYMENU_RESETTIMER, MF_DISABLED);
}

void CShutdownDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDIS)
{
	CDC *pDC = CDC::FromHandle(lpDIS->hDC);

	switch (lpDIS->CtlType)
	{
	case ODT_MENU:
	{
		if (lpDIS->itemAction & ODA_DRAWENTIRE)
		{
			pDC->FillSolidRect(&lpDIS->rcItem, RGB(0, 0, 0));
			pDC->SetBkColor(RGB(0, 0, 0));

			if (lpDIS->itemState & ODS_DISABLED)
				pDC->SetTextColor(RGB(100, 100, 100));
			else
				pDC->SetTextColor(RGB(255, 255, 255));

			lpDIS->rcItem.left = 10;
			pDC->DrawText(reinterpret_cast<LPCTSTR>(lpDIS->itemData), &lpDIS->rcItem, DT_SINGLELINE | DT_VCENTER | DT_NOCLIP);
		}
		if ((lpDIS->itemAction & ODA_SELECT) && (lpDIS->itemState & ODS_SELECTED))
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

		if (lpDIS->itemState & ODS_FOCUS)       // If the button has focus
		{
			if (lpDIS->itemState & ODS_SELECTED)
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
	if (lpMIS->CtlType == ODT_MENU)
	{
		lpMIS->itemWidth = 85;
		lpMIS->itemHeight = 20;
	}
}