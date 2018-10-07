#pragma once

class CShutdownDlg : public CDialog
{
public:
	CShutdownDlg(CWnd* pParent = nullptr) : CDialog(IDD_SHUTDOWN_DIALOG, pParent) {};
protected:
	HICON m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	CShutdownEdit m_stEditMinutes, m_stEditHours;
	CMenu m_stMenuSystray;
	CSpinButtonCtrl m_stSpinHours, m_stSpinMinutes;
	UINT m_unTimeTotal { };
	NOTIFYICONDATA m_stSystrayIcon { };
	NOTIFYICONIDENTIFIER m_stSystrayIconIdent { };
	RECT m_rectSystrayIcon { };
	HBRUSH m_hbrBlack { }, m_hbrWhite { };
	CLastminDlg m_stDlgLastmin;
	HWND m_hwndTooltip { };
	TOOLINFO m_stToolInfo { };
	BOOL m_fTooltip = FALSE, m_fToolTipFirstTime = TRUE;
	WCHAR m_strTooltip[20] { };
	const unsigned m_uTrayIconId { 1 };
	const UINT_PTR m_uTimerShutdown { 1 };
	const UINT_PTR m_uTimerToolTip { 2 };
	const WCHAR * const m_pTextNoTime { L"No time set" };

	DECLARE_MESSAGE_MAP()
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	void OnSystrayMenuShow();
	void OnSystrayMenuResetTimer();
	void OnSystrayMenuExit();
	void ShutdownPC();
	void UpdateTooltipText();
	void OnSetupButton();
	void OnTimer(UINT nIDEvent);
	void OnDestroy();
	void OnSysCommand(UINT nID, LPARAM lParam);
	LRESULT OnHotKey(WPARAM wParam, LPARAM lParam);
	LRESULT OnSystrayIconMessage(WPARAM wParam, LPARAM lParam);
	HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDIS);
	void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMIS);
	void ResetTimer();
	void OnMidnightButton();
};
