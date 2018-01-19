#pragma once

class CAboutDlg : public CDialog
{
public:
	CAboutDlg(CWnd* pParent = nullptr) : CDialog(IDD_ABOUT_DIALOG, pParent){};
protected:
	HICON m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	BOOL mb_mailto_underline = TRUE, mb_httpgithub_underline = TRUE;
	HFONT m_font_normal, m_font_underline;
	HCURSOR m_cur_hand, m_cur_arrow;
	HBRUSH m_hbr_black;

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	void OnMouseMove(UINT nFlags, CPoint point);
	HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnDestroy();
	void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDIS);
};