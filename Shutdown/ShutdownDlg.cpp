#include "stdafx.h"
#include "ShutdownDlg.h"
#include "Aboutdlg.h"
#include <thread>

#define WM_TRAY_ICON_MSG (WM_APP + 1)

void CShutdownDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_EDIT_HOURS, m_stEditHours);
	DDX_Control(pDX, IDC_EDIT_MINUTES, m_stEditMinutes);
}

BEGIN_MESSAGE_MAP(CShutdownDlg, CDialog)
	ON_BN_CLICKED(IDC_SETUP_BUTTON, OnSetupButton)
	ON_BN_CLICKED(IDC_MIDNIGHT_BUTTON, OnMidnightButton)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_SYSCOMMAND()
	ON_MESSAGE(WM_TRAY_ICON_MSG, OnSystrayIconMessage)
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

	m_stSystrayIcon.cbSize = sizeof(NOTIFYICONDATA);
	m_stSystrayIcon.hWnd = m_hWnd;
	m_stSystrayIcon.uID = m_uTrayIconId;
	m_stSystrayIcon.uFlags = NIF_ICON | NIF_MESSAGE;
	m_stSystrayIcon.uCallbackMessage = WM_TRAY_ICON_MSG;
	m_stSystrayIcon.hIcon = m_hIcon;

	Shell_NotifyIcon(NIM_ADD, &m_stSystrayIcon);

	//For acquiring icon rect 
	m_stSystrayIconIdent.cbSize = sizeof(NOTIFYICONIDENTIFIER);
	m_stSystrayIconIdent.hWnd = m_hWnd;
	m_stSystrayIconIdent.uID = m_uTrayIconId;
	m_stSystrayIconIdent.guidItem = GUID_NULL;

	//Tooltip window
	m_hwndTooltip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
		TTS_BALLOON | TTS_NOPREFIX | TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, NULL, NULL);

	_tcscpy_s(m_strTooltip, m_pTextNoTime);

	//Initializing tooltip
	m_stToolInfo.cbSize = TTTOOLINFO_V1_SIZE;
	m_stToolInfo.uFlags = TTF_TRACK;
	m_stToolInfo.uId = m_uTrayIconId;
	m_stToolInfo.lpszText = m_strTooltip;

	//Prepearing tooltip appearance
	::SendMessage(m_hwndTooltip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&m_stToolInfo);
	::SendMessage(m_hwndTooltip, TTM_SETTIPBKCOLOR, (WPARAM)RGB(0, 0, 0), 0);
	::SendMessage(m_hwndTooltip, TTM_SETTIPTEXTCOLOR, (WPARAM)RGB(255, 255, 255), 0);
	::SendMessage(m_hwndTooltip, TTM_SETTITLE, (WPARAM)TTI_NONE, (LPARAM)L"Time to shutdown:");

	m_stSpinHours.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | UDS_ALIGNRIGHT | UDS_SETBUDDYINT | UDS_ARROWKEYS | UDS_WRAP,
		CRect(0, 0, 0, 0), this, IDC_SPIN_HOURS);
	m_stSpinHours.SetBuddy(&m_stEditHours);
	m_stSpinHours.SetRange(0, 23);
	m_stSpinHours.SetPos(1);

	m_stSpinMinutes.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | UDS_ALIGNRIGHT | UDS_SETBUDDYINT | UDS_ARROWKEYS | UDS_WRAP,
		CRect(0, 0, 0, 0), this, IDC_SPIN_MINUTES);
	m_stSpinMinutes.SetBuddy(&m_stEditMinutes);
	m_stSpinMinutes.SetRange(0, 59);
	m_stSpinMinutes.SetPos(0);

	UDACCEL _stAccel[2];
	_stAccel[0].nSec = 0; _stAccel[0].nInc = 1;
	_stAccel[1].nSec = 1; _stAccel[1].nInc = 8;
	m_stSpinMinutes.SetAccel(2, _stAccel);	//acceleration of minutes_spin

	CMenu* _pSysMenu = GetSystemMenu(FALSE);
	_pSysMenu->DeleteMenu(0, MF_BYPOSITION);
	_pSysMenu->DeleteMenu(1, MF_BYPOSITION);
	_pSysMenu->DeleteMenu(2, MF_BYPOSITION);
	_pSysMenu->AppendMenu(MF_SEPARATOR);
	_pSysMenu->AppendMenu(MF_ENABLED, IDC_MENU_ABOUT, L"About...");

	m_stMenuSystray.CreatePopupMenu();
	m_stMenuSystray.AppendMenu(MF_OWNERDRAW, IDC_SYSTRAYMENU_SHOW, L"Main window");
	m_stMenuSystray.AppendMenu(MF_OWNERDRAW | MF_DISABLED, IDC_SYSTRAYMENU_RESETTIMER, L"Reset timer");
	m_stMenuSystray.AppendMenu(MF_OWNERDRAW, IDC_SYSTRAYMENU_EXIT, L"Exit");

	m_hbrBlack = CreateSolidBrush(RGB(0, 0, 0));
	m_hbrWhite = CreateSolidBrush(RGB(255, 255, 255));

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	// Parsing cmnLine params
	// If "-m" then going to midnight mode
	// and minimizing main window.
	if (wcsstr(AfxGetApp()->m_lpCmdLine, L"-m"))
	{
		OnMidnightButton();
		PostMessageW(WM_SYSCOMMAND, SC_MINIMIZE, 0);
	}

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
	m_unTimeTotal = (m_stSpinHours.GetPos() * 60) + m_stSpinMinutes.GetPos();
	m_stMenuSystray.EnableMenuItem(IDC_SYSTRAYMENU_RESETTIMER, MF_ENABLED);
	ShowWindow(SW_HIDE);
	SetTimer(m_uTimerShutdown, 60000, nullptr);

	OnTimer(m_uTimerShutdown);
}

void CShutdownDlg::OnMidnightButton()
{
	time_t t = time(0);
	tm now;
	localtime_s(&now, &t);

	//minutes left to midnight
	m_unTimeTotal = (24 - now.tm_hour) * 60 - now.tm_min;

	m_stMenuSystray.EnableMenuItem(IDC_SYSTRAYMENU_RESETTIMER, MF_ENABLED);

	SetTimer(m_uTimerShutdown, 60000, nullptr);
	OnTimer(m_uTimerShutdown);
	ShowWindow(SW_HIDE);
}

void CShutdownDlg::OnTimer(UINT nIDEvent)
{
	if (nIDEvent == m_uTimerShutdown)
	{
		UpdateTooltipText();

		switch (m_unTimeTotal) {//time in minutes to shutdown
		case 0:
			ShutdownPC();
			break;
		case 1:
		{
			m_unTimeTotal--;

			std::thread beep(Beep, 15000, 350); //calling Beep() in async state->
			beep.detach(); //->and detaching immidiately 

			INT_PTR dlg_rtrnvalue = m_stDlgLastmin.DoModal();

			if (dlg_rtrnvalue == IDCANCEL)
				ResetTimer();
			else if (dlg_rtrnvalue == IDC_POSTPONE_BUTTON)
			{
				m_unTimeTotal = 30;
				SetTimer(m_uTimerShutdown, 60000, nullptr); //resetting the timer to 30 min.

				OnTimer(m_uTimerShutdown);
			}
			break;
		}

		default:
			m_unTimeTotal--;
		}

	}
	else if (nIDEvent == m_uTimerToolTip)
	{
		POINT cur;
		GetCursorPos(&cur);
		Shell_NotifyIconGetRect(&m_stSystrayIconIdent, &m_rectSystrayIcon);

		//Checking if mouse pointer is within systrayicon rect
		//if not — killing tooltip window
		if (!(cur.x >= m_rectSystrayIcon.left && cur.x <= m_rectSystrayIcon.right && cur.y >= m_rectSystrayIcon.top && cur.y <= m_rectSystrayIcon.bottom))
		{
			KillTimer(m_uTimerToolTip);
			m_fTooltip = FALSE;
			::SendMessage(m_hwndTooltip, TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)(LPTOOLINFO)&m_stToolInfo);
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
		::SendMessage(m_hwndTooltip, TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)(LPTOOLINFO)&m_stToolInfo);//disabling tooltip
		break;
	case WM_RBUTTONUP:
		POINT cur;
		GetCursorPos(&cur);
		SetForegroundWindow();
		m_stMenuSystray.TrackPopupMenu(TPM_LEFTALIGN, cur.x, cur.y, this);
		break;
	case WM_MOUSEMOVE:
		//For some undiscovered reason when systray icon just created 
		//it sends WM_MOUSEMOVE message to main window (even if mouse is far away from systray)
		//which forces tooltip window to show up at the current cursor pos. 
		//This flag is just to prevent it. 
		if (m_fToolTipFirstTime)
		{
			m_fToolTipFirstTime = FALSE;
			return 0;
		}

		//Showing tooltip window if it's already not.
		//Then in timer proc checking whether cursor pos is out of systray icon rect.
		//Windows can not do it by itself.
		if (!m_fTooltip)
		{
			m_fTooltip = TRUE;

			Shell_NotifyIconGetRect(&m_stSystrayIconIdent, &m_rectSystrayIcon);

			//Position tooltip in icon's center
			::SendMessage(m_hwndTooltip, TTM_TRACKPOSITION, 0,
				(LPARAM)MAKELONG((m_rectSystrayIcon.right - m_rectSystrayIcon.left) / 2 + m_rectSystrayIcon.left,
				(m_rectSystrayIcon.bottom - m_rectSystrayIcon.top) / 2 + m_rectSystrayIcon.top));
			//Show tooltip window
			::SendMessage(m_hwndTooltip, TTM_TRACKACTIVATE, (WPARAM)TRUE, (LPARAM)(LPTOOLINFO)&m_stToolInfo);

			//every 200ms checking whether cursor is still hovering systray icon
			SetTimer(m_uTimerToolTip, 200, NULL);
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
	m_stDlgLastmin.EndDialog(-1); //destroying lastmin dlg if exist
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
	KillTimer(m_uTimerShutdown);

	Shell_NotifyIcon(NIM_DELETE, &m_stSystrayIcon);
	UnregisterHotKey(m_hWnd, 1);
	m_stMenuSystray.DestroyMenu();
	DeleteObject(m_hbrBlack);
	DeleteObject(m_hbrWhite);

	::DestroyWindow(m_hwndTooltip);
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
		return m_hbrWhite;

	pDC->SetTextColor(RGB(255, 255, 255));
	pDC->SetBkColor(RGB(0, 0, 0));

	return m_hbrBlack;
}

void CShutdownDlg::UpdateTooltipText()
{
	if (m_unTimeTotal % 60 > 9)
		swprintf_s(m_strTooltip, 20, L"%u%s%u%s", m_unTimeTotal / 60, L":", m_unTimeTotal % 60, L" remaining");
	else
		swprintf_s(m_strTooltip, 20, L"%u%s%u%s", m_unTimeTotal / 60, L":0", m_unTimeTotal % 60, L" remaining");

	m_stToolInfo.lpszText = m_strTooltip;
	::SendMessage(m_hwndTooltip, TTM_UPDATETIPTEXT, 0, (LPARAM)(LPTOOLINFO)&m_stToolInfo);
}

void CShutdownDlg::ResetTimer()
{
	KillTimer(m_uTimerShutdown);

	_tcscpy_s(m_strTooltip, m_pTextNoTime);
	m_stToolInfo.lpszText = m_strTooltip;
	::SendMessage(m_hwndTooltip, TTM_UPDATETIPTEXT, 0, (LPARAM)(LPTOOLINFO)&m_stToolInfo);

	m_stMenuSystray.EnableMenuItem(IDC_SYSTRAYMENU_RESETTIMER, MF_DISABLED);
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