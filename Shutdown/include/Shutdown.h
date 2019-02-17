#pragma once

class CShutdownApp : public CWinApp
{
public:
	CShutdownApp() {};
	virtual BOOL InitInstance();
private:
	LPCWSTR m_lpszClassName { L"ShutDown - 34754674" };
	ATOM m_ClassAtom { };
};