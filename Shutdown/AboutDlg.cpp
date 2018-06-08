#include "stdafx.h"
#include "AboutDlg.h"

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	ON_WM_MOUSEMOVE()
	ON_WM_CTLCOLOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_DESTROY()
	ON_WM_DRAWITEM()
END_MESSAGE_MAP()

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetClassLongPtr(m_hWnd, GCL_HCURSOR, 0); //to prevent cursor from blinking

	m_fontNormal = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));

	LOGFONT logFont;
	GetObject(m_fontNormal, sizeof(logFont), &logFont);
	logFont.lfUnderline = TRUE;

	m_fontUnderline = CreateFontIndirect(&logFont);

	m_curHand = LoadCursor(nullptr, IDC_HAND);
	m_curArrow = LoadCursor(nullptr, IDC_ARROW);

	m_hbrBlack = CreateSolidBrush(RGB(0, 0, 0));

	return TRUE;
}

void CAboutDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	CWnd* pWnd = ChildWindowFromPoint(point);

	if (pWnd == nullptr) //mouse pointer is out of window
		return;

	if (m_fMailtoUnderline == (pWnd->GetDlgCtrlID() == IDC_MAILTO)) //is mouse pointer hovering IDC_MAILTO?
	{
		m_fMailtoUnderline = !m_fMailtoUnderline;
		::InvalidateRect(GetDlgItem(IDC_MAILTO)->m_hWnd, nullptr, FALSE);
		SetCursor(m_fMailtoUnderline ? m_curArrow : m_curHand);
	}
	if (m_fGithubUnderline == (pWnd->GetDlgCtrlID() == IDC_HTTPGITHUB))
	{
		m_fGithubUnderline = !m_fGithubUnderline;
		::InvalidateRect(GetDlgItem(IDC_HTTPGITHUB)->m_hWnd, nullptr, FALSE);
		SetCursor(m_fGithubUnderline ? m_curArrow : m_curHand);
	}
}

HBRUSH CAboutDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	pDC->SetBkColor(RGB(0, 0, 0));

	switch (pWnd->GetDlgCtrlID())
	{
	case IDC_MAILTO:
		pDC->SetTextColor(RGB(0, 255, 50));
		pDC->SelectObject(m_fMailtoUnderline ? m_fontNormal : m_fontUnderline);
		break;
	case IDC_HTTPGITHUB:
		pDC->SetTextColor(RGB(0, 255, 50));
		pDC->SelectObject(m_fGithubUnderline ? m_fontNormal : m_fontUnderline);
		break;
	default:
		pDC->SetTextColor(RGB(255, 255, 255));
	}

	return m_hbrBlack;
}

void CAboutDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	CWnd* pWnd = ChildWindowFromPoint(point);

	if (pWnd == nullptr)
		return;

	if (pWnd->GetDlgCtrlID() == IDC_MAILTO)
		ShellExecute(nullptr, TEXT("open"), TEXT("mailto:eaxedx@gmail.com"), nullptr, nullptr, NULL);

	if (pWnd->GetDlgCtrlID() == IDC_HTTPGITHUB)
		ShellExecute(nullptr, TEXT("open"), TEXT("https://github.com/jovibor/Shutdown"), nullptr, nullptr, NULL);
}

void CAboutDlg::OnDestroy()
{
	DeleteObject(m_fontUnderline);
	DeleteObject(m_hbrBlack);
}

void CAboutDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDIS)
{
	CDC *pDC = CDC::FromHandle(lpDIS->hDC);

	switch (lpDIS->CtlType)
	{
	case ODT_BUTTON:
	{
		TCHAR buff[128];
		::GetWindowText(lpDIS->hwndItem, buff, 128);

		pDC->FillSolidRect(&lpDIS->rcItem, RGB(0, 0, 0)); //Button color
		pDC->DrawEdge(&lpDIS->rcItem, EDGE_RAISED, BF_RECT);
		pDC->SetTextColor(RGB(255, 255, 255));
		pDC->DrawText(buff, &lpDIS->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

		if (lpDIS->itemState & ODS_FOCUS)       // If the button has focus
		{
			if (lpDIS->itemState & ODS_SELECTED)
				pDC->DrawEdge(&lpDIS->rcItem, EDGE_SUNKEN, BF_RECT);    // Draw a sunken face

			lpDIS->rcItem.top += 4;	lpDIS->rcItem.left += 4;
			lpDIS->rcItem.right -= 4; lpDIS->rcItem.bottom -= 4;

			pDC->DrawFocusRect(&lpDIS->rcItem);
		}
		break;
	}
	}
}