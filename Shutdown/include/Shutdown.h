#pragma once

class CShutdownApp : public CWinApp
{
public:
	CShutdownApp()noexcept {};
	BOOL InitInstance()override;
private:
	const LPCWSTR m_pwszClassName { L"ShutDown - 34754674" };
};