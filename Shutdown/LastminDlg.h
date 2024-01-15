#pragma once

class CLastminDlg : public CDialog {
public:
	explicit CLastminDlg(CWnd* pParent = nullptr);
private:
	void DoDataExchange(CDataExchange* pDX)override;
	BOOL OnInitDialog()override;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDIS);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnPostponeButton();
	BOOL PreTranslateMessage(MSG* pMsg)override;
	DECLARE_MESSAGE_MAP();
private:
	HBRUSH m_hbrBlack { };
	UINT m_uTimerSec { };
};