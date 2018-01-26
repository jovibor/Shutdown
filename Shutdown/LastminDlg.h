#pragma once

class CLastminDlg : public CDialog
{
	DECLARE_DYNAMIC(CLastminDlg)

public:
	CLastminDlg(CWnd* pParent = nullptr) : CDialog(IDD_LASTMIN_DIALOG, pParent) {};
	virtual ~CLastminDlg() {};

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LASTMIN_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	DECLARE_MESSAGE_MAP()

protected:
	HBRUSH m_hbr_black;
	UINT_PTR m_timer_id = 0;
	UINT m_timer_sec;
public:
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDIS);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedPostponeButton();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
