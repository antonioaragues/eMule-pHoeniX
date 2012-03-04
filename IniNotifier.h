#pragma once
#include "ini2.h"
#include "notifierSkin.h"

class CIniNotifier :
	public CIni, public CNotifierSkin
{
public:
	CIniNotifier(void);
	~CIniNotifier(void);
	CIniNotifier(const CIniNotifier&);
	bool Open(CString skinFile, bool readOnly);
	bool Save(CString skinFile);

protected:
	uint8 m_nIniVersion;
	CString m_strSkinDir;

private:
	void LoadConfiguration();
	void SaveConfiguration();

};
