#pragma once

class CAboutDlg final : public CDialog
{
public:
	explicit CAboutDlg(CWnd* pParent = nullptr);
private:
	virtual BOOL OnInitDialog();
	void OnMouseMove(UINT nFlags, CPoint point);
	HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnDestroy();
	void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDIS);
	DECLARE_MESSAGE_MAP();
private:
	bool m_fMailtoUnderline { true };
	bool m_fGithubUnderline { true };
	HFONT m_fontNormal { };
	HFONT m_fontUnderline { };
	HCURSOR m_curHand { };
	HCURSOR m_curArrow { };
	HBRUSH m_hbrBlack { };
};