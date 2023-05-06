#include "stdafx.h"
#include "LastminDlg.h"
#include "Resource.h"

constexpr auto TIME_LASTMIN = 1;

BEGIN_MESSAGE_MAP(CLastminDlg, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_DRAWITEM()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_POSTPONE_BUTTON, &CLastminDlg::OnPostponeButton)
END_MESSAGE_MAP()

CLastminDlg::CLastminDlg(CWnd* pParent) : CDialog(IDD_LASTMIN_DIALOG, pParent)
{
}

void CLastminDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

HBRUSH CLastminDlg::OnCtlColor(CDC* pDC, CWnd* /*pWnd*/, UINT /*nCtlColor*/)
{
	pDC->SetTextColor(RGB(255, 255, 255));
	pDC->SetBkColor(RGB(0, 0, 0));

	return  m_hbrBlack;
}

BOOL CLastminDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_hbrBlack = CreateSolidBrush(RGB(0, 0, 0));
	m_uTimerSec = 60;

	SetTimer(TIME_LASTMIN, 1000, nullptr);
	OnTimer(TIME_LASTMIN);

	SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), FALSE);
	SetFocus();

	return TRUE;
}

void CLastminDlg::OnDrawItem(int /*nIDCtl*/, LPDRAWITEMSTRUCT lpDIS)
{
	CDC *pDC = CDC::FromHandle(lpDIS->hDC);

	switch (lpDIS->CtlType)
	case ODT_BUTTON:
	{
		WCHAR buff[128];
		::GetWindowTextW(lpDIS->hwndItem, buff, 128);

		pDC->FillSolidRect(&lpDIS->rcItem, RGB(0, 0, 0)); //Button color
		pDC->DrawEdge(&lpDIS->rcItem, EDGE_RAISED, BF_RECT);
		pDC->SetTextColor(RGB(255, 255, 255));
		pDC->DrawTextW(buff, &lpDIS->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

		if (lpDIS->itemState & ODS_FOCUS) { // If the button has focus
			if (lpDIS->itemState & ODS_SELECTED)
				pDC->DrawEdge(&lpDIS->rcItem, EDGE_SUNKEN, BF_RECT);    // Draw a sunken face

			lpDIS->rcItem.top += 4;	lpDIS->rcItem.left += 4;
			lpDIS->rcItem.right -= 4; lpDIS->rcItem.bottom -= 4;

			pDC->DrawFocusRect(&lpDIS->rcItem);
		}
		break;
	}
}

void CLastminDlg::OnTimer(UINT_PTR /*nIDEvent*/)
{
	WCHAR timer_text[5];
	if (m_uTimerSec > 9)
		swprintf_s(timer_text, 5, L"%s%u", L"0:", m_uTimerSec);
	else
		swprintf_s(timer_text, 5, L"%s%u", L"0:0", m_uTimerSec);

	SetDlgItemTextW(IDC_LASTMIN_TIMER, timer_text);

	if (m_uTimerSec-- == 0) {
		KillTimer(TIME_LASTMIN);
	}
}

void CLastminDlg::OnDestroy()
{
	KillTimer(TIME_LASTMIN);
}

void CLastminDlg::OnPostponeButton()
{
	EndDialog(IDC_POSTPONE_BUTTON);
}

BOOL CLastminDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) {
		EndDialog(GetFocus()->GetDlgCtrlID()); //Ending dialog with active (in focus) button exit code.

		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}