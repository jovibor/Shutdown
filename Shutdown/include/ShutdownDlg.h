#pragma once

class CShutdownDlg : public CDialog
{
public:
	CShutdownDlg(CWnd* pParent = nullptr) : CDialog(IDD_SHUTDOWN_DIALOG, pParent){}; 
protected:
	HICON m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	CShutdownEdit m_edit_minutes, m_edit_hours;
	CMenu m_systray_menu;
	CSpinButtonCtrl m_spin_hours, m_spin_minutes;
	UINT m_time_total;
	NOTIFYICONDATA m_systray_icon = { 0 };
	NOTIFYICONIDENTIFIER m_systray_iconid;
	RECT m_systray_iconrect;
	HBRUSH m_hbr_black, m_hbr_white;
	CLastminDlg m_lastmin_dlg;
	HWND m_tooltip_hwnd;
	TOOLINFO m_ti = { 0 };
	BOOL mf_tooltip = FALSE, mf_first_time_tooltip = TRUE;
	WCHAR m_tooltip_text[20];
	
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
};
