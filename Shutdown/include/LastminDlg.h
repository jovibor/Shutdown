#pragma once

class CLastminDlg : public CDialog
{

public:
	CLastminDlg(CWnd* pParent = nullptr) : CDialog(IDD_LASTMIN_DIALOG, pParent) {};
	virtual ~CLastminDlg() {};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	DECLARE_MESSAGE_MAP()
	HBRUSH m_hbrBlack { };
	UINT m_uTimerSec { };

public:
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDIS);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnPostponeButton();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
