#pragma once
#include <map>
#include "types.h"
class CWombatUtil
{
public:
	CWombatUtil() {}
	virtual ~CWombatUtil() {}
};

class CWombatLists
{
public:
	CWombatLists(void);
	~CWombatLists(void);
	int GetModListCount() {return m_ModList.GetCount();}
	UINT GetHashKey(LPCTSTR key) const {return m_ModList.HashKey(key);}
protected:
CMapStringToPtr	 m_ModList;
public:
	CString* GetModPtr(LPCTSTR pMod);
	CString	 m_strEmptyMod;
};