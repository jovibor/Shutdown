#pragma once

class CShutdownEdit : public CEdit
{
public:
	CShutdownEdit(){};
	virtual ~CShutdownEdit(){};
protected:
	DECLARE_MESSAGE_MAP()
	BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message){ return TRUE; };
	void OnLButtonDown(UINT nFlags, CPoint point){};
	void OnRButtonDown(UINT nFlags, CPoint){};
	void OnSetFocus(CWnd* pOldWnd){};
};