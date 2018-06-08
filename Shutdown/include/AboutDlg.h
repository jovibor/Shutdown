#pragma once

class CAboutDlg : public CDialog
{
public:
	CAboutDlg(CWnd* pParent = nullptr) : CDialog(IDD_ABOUT_DIALOG, pParent) {};
protected:
	BOOL m_fMailtoUnderline = TRUE, m_fGithubUnderline = TRUE;
	HFONT m_fontNormal = nullptr, m_fontUnderline = nullptr;
	HCURSOR m_curHand = nullptr, m_curArrow = nullptr;
	HBRUSH m_hbrBlack = nullptr;

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	void OnMouseMove(UINT nFlags, CPoint point);
	HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnDestroy();
	void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDIS);
};