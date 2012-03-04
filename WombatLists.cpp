#include "StdAfx.h"
#include "emule.h"
#include "wombatlists.h"
#include "otherfunctions.h"
#include "HttpDownloadDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CWombatLists::CWombatLists(void)
{
	m_ModList.InitHashTable(331);
	m_strEmptyMod.Empty();
}

CWombatLists::~CWombatLists(void)
{
	CString key;
	CString* pointer;

	POSITION pos = m_ModList.GetStartPosition();
	while (pos)
	{
		m_ModList.GetNextAssoc(pos, key, (void*&)pointer);
		delete pointer;
	}
	m_ModList.RemoveAll();
}

CString* CWombatLists::GetModPtr(LPCTSTR pMod)
{
	CString* pointer;
	if (m_ModList.Lookup(pMod,(void*&)pointer))
		return pointer; //Already inserted
	pointer=new CString(pMod);
	m_ModList.SetAt(pMod,(void*&)pointer);
	return pointer;
}