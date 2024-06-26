#pragma once
#include "LastminDlg.h"
#include <string>

class CShutdownEdit : public CEdit {
private:
	BOOL OnSetCursor(CWnd*, UINT, UINT) { return TRUE; };
	void OnLButtonDown(UINT, CPoint) { };
	void OnRButtonDown(UINT, CPoint) { };
	void OnSetFocus(CWnd*) { };
	DECLARE_MESSAGE_MAP();
};

class CShutdownDlg : public CDialog {
public:
	explicit CShutdownDlg(CWnd* pParent = nullptr);
private:
	BOOL PreTranslateMessage(MSG* pMsg)override;
	void DoDataExchange(CDataExchange* pDX)override;
	BOOL OnInitDialog()override;
	void OnSystrayMenuShow();
	void OnSystrayMenuResetTimer();
	void OnSystrayMenuExit();
	void ShutdownPC();
	void UpdateTooltipText();
	void OnSetupButton();
	void OnTimer(UINT_PTR nIDEvent);
	void OnDestroy();
	void OnSysCommand(UINT nID, LPARAM lParam);
	LRESULT OnSystrayIconMessage(WPARAM wParam, LPARAM lParam);
	HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDIS);
	void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMIS);
	void ResetTimer();
	void OnMidnightButton();
	DECLARE_MESSAGE_MAP();
private:
	static constexpr auto m_pTextNoTime { L"No time set" };
	CShutdownEdit m_stEditMinutes;
	CShutdownEdit m_stEditHours;
	CSpinButtonCtrl m_stSpinHours;
	CSpinButtonCtrl m_stSpinMinutes;
	CLastminDlg m_stDlgLastMin;
	CMenu m_stMenuSystray;
	UINT m_unTimeTotal { };
	NOTIFYICONDATAW m_stSystrayIcon { };
	NOTIFYICONIDENTIFIER m_stSystrayIconIdent { };
	CRect m_rcIcon;
	HBRUSH m_hbrBlack { };
	HBRUSH m_hbrWhite { };
	HWND m_hwndTooltip { };
	TTTOOLINFOW m_stToolInfo { };
	std::wstring m_strTooltip { };
	bool m_fTooltip { false };
	bool m_fToolTipFirstTime { true };
};