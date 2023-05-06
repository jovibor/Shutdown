#include "stdafx.h"
#include "Resource.h"
#include "ShutdownDlg.h"
#include "AboutDlg.h"
#include <thread>

constexpr auto WM_TRAY_ICON_MSG = WM_APP + 1;
constexpr UINT_PTR ID_TIMER_SHUTDOWN { 0x01 };
constexpr UINT_PTR ID_TIMER_TT_CHECK { 0x10 };
constexpr UINT_PTR ID_TIMER_TT_ACTIVATE { 0x11 };

BEGIN_MESSAGE_MAP(CShutdownEdit, CEdit)
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

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

void CShutdownDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_EDIT_HOURS, m_stEditHours);
	DDX_Control(pDX, IDC_EDIT_MINUTES, m_stEditMinutes);
}

BOOL CShutdownDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	RegisterHotKey(m_hWnd, 1, MOD_CONTROL | MOD_ALT | MOD_SHIFT, 0x53); //ALT+CTRL+SHIFT+S - to shutdown immediately.

	constexpr auto uTrayIconID { 1UL };
	auto hIcon = AfxGetApp()->LoadIconW(IDR_MAINFRAME);

	m_stSystrayIcon.cbSize = sizeof(NOTIFYICONDATA);
	m_stSystrayIcon.hWnd = m_hWnd;
	m_stSystrayIcon.uID = uTrayIconID;
	m_stSystrayIcon.uFlags = NIF_ICON | NIF_MESSAGE;
	m_stSystrayIcon.uCallbackMessage = WM_TRAY_ICON_MSG;
	m_stSystrayIcon.hIcon = hIcon;

	Shell_NotifyIconW(NIM_ADD, &m_stSystrayIcon);

	//For acquiring icon rect.
	m_stSystrayIconIdent.cbSize = sizeof(NOTIFYICONIDENTIFIER);
	m_stSystrayIconIdent.hWnd = m_hWnd;
	m_stSystrayIconIdent.uID = uTrayIconID;
	m_stSystrayIconIdent.guidItem = GUID_NULL;

	//Tooltip window.
	m_hwndTooltip = CreateWindowExW(WS_EX_TOPMOST, TOOLTIPS_CLASS, nullptr,
		TTS_BALLOON | TTS_NOPREFIX | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		nullptr, nullptr, nullptr, nullptr);

	wcscpy_s(m_strTooltip, m_pTextNoTime);

	//Initializing tooltip.
	m_stToolInfo.cbSize = sizeof(TTTOOLINFOW);
	m_stToolInfo.uFlags = TTF_TRACK;
	m_stToolInfo.uId = uTrayIconID;
	m_stToolInfo.lpszText = m_strTooltip;
	::SendMessageW(m_hwndTooltip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&m_stToolInfo);
	::SendMessageW(m_hwndTooltip, TTM_SETTIPBKCOLOR, (WPARAM)RGB(0, 0, 0), 0);
	::SendMessageW(m_hwndTooltip, TTM_SETTIPTEXTCOLOR, (WPARAM)RGB(255, 255, 255), 0);
	::SendMessageW(m_hwndTooltip, TTM_SETTITLE, (WPARAM)TTI_NONE, (LPARAM)L"Time to shutdown:");

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

	UDACCEL stAccel[2];
	stAccel[0].nSec = 0; stAccel[0].nInc = 1;
	stAccel[1].nSec = 1; stAccel[1].nInc = 8;
	m_stSpinMinutes.SetAccel(2, stAccel);	//acceleration of minutes_spin

	const auto pSysMenu = GetSystemMenu(FALSE);
	pSysMenu->DeleteMenu(0, MF_BYPOSITION);
	pSysMenu->DeleteMenu(1, MF_BYPOSITION);
	pSysMenu->DeleteMenu(2, MF_BYPOSITION);
	pSysMenu->AppendMenu(MF_SEPARATOR);
	pSysMenu->AppendMenu(MF_ENABLED, IDC_MENU_ABOUT, L"About...");

	m_stMenuSystray.CreatePopupMenu();
	m_stMenuSystray.AppendMenu(MF_OWNERDRAW, IDC_SYSTRAYMENU_SHOW, L"Main window");
	m_stMenuSystray.AppendMenu(MF_OWNERDRAW | MF_DISABLED, IDC_SYSTRAYMENU_RESETTIMER, L"Reset timer");
	m_stMenuSystray.AppendMenu(MF_OWNERDRAW, IDC_SYSTRAYMENU_EXIT, L"Exit");

	m_hbrBlack = CreateSolidBrush(RGB(0, 0, 0));
	m_hbrWhite = CreateSolidBrush(RGB(255, 255, 255));

	SetIcon(hIcon, TRUE);
	SetIcon(hIcon, FALSE);

	//Parsing cmdLine params:
	//If "-m" then going to midnight mode and minimizing main window.
	if (wcsstr(AfxGetApp()->m_lpCmdLine, L"-m")) {
		OnMidnightButton();
		PostMessageW(WM_SYSCOMMAND, SC_MINIMIZE, 0);
	}

	return TRUE;
}

void CShutdownDlg::ShutdownPC()
{
	TOKEN_PRIVILEGES tkp { };
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	LookupPrivilegeValueW(nullptr, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);

	HANDLE hToken;
	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, nullptr, nullptr);
	InitiateSystemShutdownExW(nullptr, nullptr, NULL, TRUE, FALSE, SHTDN_REASON_FLAG_PLANNED);

	DestroyWindow();
}

void CShutdownDlg::OnSetupButton()
{
	m_unTimeTotal = (m_stSpinHours.GetPos() * 60) + m_stSpinMinutes.GetPos();
	m_stMenuSystray.EnableMenuItem(IDC_SYSTRAYMENU_RESETTIMER, MF_ENABLED);
	ShowWindow(SW_HIDE);
	SetTimer(ID_TIMER_SHUTDOWN, 60000, nullptr);

	OnTimer(ID_TIMER_SHUTDOWN);
}

void CShutdownDlg::OnMidnightButton()
{
	const time_t t = time(nullptr);
	tm now;
	localtime_s(&now, &t);

	//minutes left to midnight.
	m_unTimeTotal = (24 - now.tm_hour) * 60 - now.tm_min;

	m_stMenuSystray.EnableMenuItem(IDC_SYSTRAYMENU_RESETTIMER, MF_ENABLED);

	SetTimer(ID_TIMER_SHUTDOWN, 60000, nullptr);
	OnTimer(ID_TIMER_SHUTDOWN);
	ShowWindow(SW_HIDE);
}

void CShutdownDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent) {
	case ID_TIMER_SHUTDOWN:
	{
		UpdateTooltipText();

		switch (m_unTimeTotal) { //Time in minutes to shutdown.
		case 0:
			ShutdownPC();
			break;
		case 1:
		{
			--m_unTimeTotal;
			std::thread beep(Beep, 15000, 350); //calling Beep() in async state->
			beep.detach(); //->and detaching immediately.

			const INT_PTR iReturn = m_stDlgLastMin.DoModal();

			if (iReturn == IDCANCEL)
				ResetTimer();
			else if (iReturn == IDC_POSTPONE_BUTTON) {
				m_unTimeTotal = 30;
				SetTimer(ID_TIMER_SHUTDOWN, 60000, nullptr); //resetting the timer to 30 min.
				OnTimer(ID_TIMER_SHUTDOWN);
			}
		}
		break;
		default:
			--m_unTimeTotal;
		}

	}
	break;
	case ID_TIMER_TT_CHECK:
	{
		POINT ptCursor;
		GetCursorPos(&ptCursor);
		Shell_NotifyIconGetRect(&m_stSystrayIconIdent, &m_rcIcon);

		//Checking if mouse pointer is within systrayicon rect.
		//if not — killing tooltip window.
		if (!m_rcIcon.PtInRect(ptCursor)) {
			KillTimer(ID_TIMER_TT_CHECK);
			m_fTooltip = false;
			::SendMessageW(m_hwndTooltip, TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)(LPTOOLINFO)&m_stToolInfo);
		}
	}
	break;
	case ID_TIMER_TT_ACTIVATE:
	{
		POINT ptCursor;
		GetCursorPos(&ptCursor);
		Shell_NotifyIconGetRect(&m_stSystrayIconIdent, &m_rcIcon);

		if (m_rcIcon.PtInRect(ptCursor)) {
			//Showing tooltip window if it's already not.
			//Then in timer proc checking whether cursor pos is out of systray icon rect.

			//Position tooltip in icon's center
			::SendMessageW(m_hwndTooltip, TTM_TRACKPOSITION, 0, (LPARAM)MAKELONG((m_rcIcon.Width()) / 2 + m_rcIcon.left,
				(m_rcIcon.Height()) / 2 + m_rcIcon.top));
			//Show tooltip window
			::SendMessageW(m_hwndTooltip, TTM_TRACKACTIVATE, (WPARAM)TRUE, (LPARAM)(LPTOOLINFO)&m_stToolInfo);

			//every 200ms checking whether cursor is still hovering systray icon
			SetTimer(ID_TIMER_TT_CHECK, 200, nullptr);
		}
		else {
			m_fTooltip = false;
		}

		KillTimer(ID_TIMER_TT_ACTIVATE);
	}
	}
}

LRESULT CShutdownDlg::OnSystrayIconMessage(WPARAM /*wParam*/, LPARAM lParam)
{
	switch (lParam) {
	case WM_LBUTTONDBLCLK:
		ShowWindow(SW_SHOW);
		break;
	case WM_RBUTTONDOWN:
		::SendMessageW(m_hwndTooltip, TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)(LPTOOLINFO)&m_stToolInfo);//disabling tooltip
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
		if (m_fToolTipFirstTime) {
			m_fToolTipFirstTime = false;
			return 0;
		}

		if (!m_fTooltip) {
			SetTimer(ID_TIMER_TT_ACTIVATE, 200, nullptr);
			m_fTooltip = true;
		}
		break;
	}

	return 0;
}

void CShutdownDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	switch (nID) {
	case SC_MINIMIZE:
		ShowWindow(SW_HIDE);
		break;
	case IDC_MENU_ABOUT:
	{
		CAboutDlg about_dlg;
		about_dlg.DoModal();
	}
	break;
	default:
		CDialog::OnSysCommand(nID, lParam);
		break;
	}
}

void CShutdownDlg::OnSystrayMenuExit()
{
	DestroyWindow();
}

void CShutdownDlg::OnSystrayMenuResetTimer()
{
	ResetTimer();
	m_stDlgLastMin.EndDialog(-1); //destroying lastmin dlg if exist
}

void CShutdownDlg::OnSystrayMenuShow()
{
	ShowWindow(SW_SHOW);
}

LRESULT CShutdownDlg::OnHotKey(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	ShutdownPC();
	return 0;
}

CShutdownDlg::CShutdownDlg(CWnd * pParent) : CDialog(IDD_SHUTDOWN_DIALOG, pParent)
{
}

BOOL CShutdownDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) {
		OnSetupButton();
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CShutdownDlg::OnDestroy()
{
	KillTimer(ID_TIMER_SHUTDOWN);

	Shell_NotifyIconW(NIM_DELETE, &m_stSystrayIcon);
	UnregisterHotKey(m_hWnd, 1);
	m_stMenuSystray.DestroyMenu();
	DeleteObject(m_hbrBlack);
	DeleteObject(m_hbrWhite);
	::DestroyWindow(m_hwndTooltip);
}

HBRUSH CShutdownDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT /*nCtlColor*/)
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
	::SendMessageW(m_hwndTooltip, TTM_UPDATETIPTEXT, 0, (LPARAM)(LPTOOLINFO)&m_stToolInfo);
}

void CShutdownDlg::ResetTimer()
{
	KillTimer(ID_TIMER_SHUTDOWN);

	wcscpy_s(m_strTooltip, m_pTextNoTime);
	m_stToolInfo.lpszText = m_strTooltip;
	::SendMessageW(m_hwndTooltip, TTM_UPDATETIPTEXT, 0, (LPARAM)(LPTOOLINFO)&m_stToolInfo);

	m_stMenuSystray.EnableMenuItem(IDC_SYSTRAYMENU_RESETTIMER, MF_DISABLED);
}

void CShutdownDlg::OnDrawItem(int /*nIDCtl*/, LPDRAWITEMSTRUCT lpDIS)
{
	auto *pDC = CDC::FromHandle(lpDIS->hDC);

	switch (lpDIS->CtlType) {
	case ODT_MENU:
	{
		if (lpDIS->itemAction & ODA_DRAWENTIRE) {
			pDC->FillSolidRect(&lpDIS->rcItem, RGB(0, 0, 0));
			pDC->SetBkColor(RGB(0, 0, 0));

			if (lpDIS->itemState & ODS_DISABLED)
				pDC->SetTextColor(RGB(100, 100, 100));
			else
				pDC->SetTextColor(RGB(255, 255, 255));

			lpDIS->rcItem.left = 10;
			pDC->DrawTextW(reinterpret_cast<LPCTSTR>(lpDIS->itemData), &lpDIS->rcItem, DT_SINGLELINE | DT_VCENTER | DT_NOCLIP);
		}
		if ((lpDIS->itemAction & ODA_SELECT) && (lpDIS->itemState & ODS_SELECTED)) {
			auto brush = CBrush(RGB(255, 255, 255));
			pDC->FrameRect(&lpDIS->rcItem, &brush);
		}
		else {
			auto brush = CBrush(RGB(0, 0, 0));
			pDC->FrameRect(&lpDIS->rcItem, &brush);
		}
		break;
	}
	case ODT_BUTTON:
	{
		WCHAR buff[128];
		::GetWindowTextW(lpDIS->hwndItem, buff, 128);

		pDC->FillSolidRect(&lpDIS->rcItem, RGB(0, 0, 0)); //Button color
		pDC->DrawEdge(&lpDIS->rcItem, EDGE_RAISED, BF_RECT);
		pDC->SetTextColor(RGB(255, 255, 255));
		pDC->DrawTextW(buff, &lpDIS->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

		if (lpDIS->itemState & ODS_FOCUS) {
			if (lpDIS->itemState & ODS_SELECTED)
				pDC->DrawEdge(&lpDIS->rcItem, EDGE_SUNKEN, BF_RECT); // Draw a sunken face

			lpDIS->rcItem.top += 4;	lpDIS->rcItem.left += 4;
			lpDIS->rcItem.right -= 4; lpDIS->rcItem.bottom -= 4;

			pDC->DrawFocusRect(&lpDIS->rcItem);
		}
		break;
	}
	}
}

void CShutdownDlg::OnMeasureItem(int /*nIDCtl*/, LPMEASUREITEMSTRUCT lpMIS)
{
	if (lpMIS->CtlType == ODT_MENU) {
		lpMIS->itemWidth = 85;
		lpMIS->itemHeight = 20;
	}
}