// Modeless.cpp : implementation file
//

#include "stdafx.h"
#include "Modeless.h"
#include "emule.h"
#include "emuleDlg.h"
#include "PreferencesDlg.h"
#include "UserMsgs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// SLUGFILLER: modelessDialogs

// CModelessPropertySheet

IMPLEMENT_DYNAMIC(CModelessTreePropSheet, CTreePropSheet)

BEGIN_MESSAGE_MAP(CModelessTreePropSheet, CTreePropSheet)
	ON_COMMAND(IDOK, OnOK)
	ON_COMMAND(IDCANCEL, OnCancel)
END_MESSAGE_MAP()

CModelessTreePropSheet::CModelessTreePropSheet(BOOL bDeleteOnClose)
{
	m_bDeleteOnClose = bDeleteOnClose;
	m_bActive = FALSE;
}

void CModelessTreePropSheet::OpenDialog()
{
	if (!m_bActive) {
		m_bActive = TRUE;
		Create(theApp.emuledlg, WS_SYSMENU | WS_POPUP | WS_CAPTION | DS_MODALFRAME | DS_CONTEXTHELP | WS_VISIBLE | WS_MINIMIZEBOX);
	}
	ShowWindow(SW_SHOW);
	SetFocus();
}

void CModelessTreePropSheet::CloseDialog()
{
	if (m_bActive)
		DestroyWindow();
}

bool CModelessTreePropSheet::IsDialogOpen()
{
	return m_bActive;
}

BOOL CModelessTreePropSheet::OnInitDialog()
{
	// Modeless property sheets don't have Ok and Cancel buttons by default
	// This little trick fulls the property sheet into thinking it's modal
	// during it's init, so that it doesn't disable the Ok and Cancel buttons
	BOOL old_bModeless = m_bModeless;
	UINT old_nFlags = m_nFlags;
	m_bModeless = FALSE;
	m_nFlags |= WF_CONTINUEMODAL;
	BOOL bResult = CTreePropSheet::OnInitDialog();
	m_bModeless = old_bModeless;
	m_nFlags = old_nFlags;
	return bResult;
}

void CModelessTreePropSheet::OnOK()
{
	SendMessage(WM_COMMAND, ID_APPLY_NOW);
	UpdateData();
	DestroyWindow();
}

void CModelessTreePropSheet::OnCancel()
{
	DestroyWindow();
}

void CModelessTreePropSheet::PostNcDestroy()
{
	m_bActive = FALSE;
	if (m_bDeleteOnClose)
		delete this;
}


static CMap<CObject*, CObject*, CModelessPropertySheetInterface*, CModelessPropertySheetInterface*> object_interface_map;

// CModelessPropertySheet

IMPLEMENT_DYNAMIC(CModelessPropertySheet, CListViewWalkerPropertySheet)

BEGIN_MESSAGE_MAP(CModelessPropertySheet, CListViewWalkerPropertySheet)
	ON_COMMAND(IDOK, OnOK)
	ON_COMMAND(IDCANCEL, OnCancel)
END_MESSAGE_MAP()

CModelessPropertySheet::CModelessPropertySheet(CListCtrlItemWalk* pListCtrl, BOOL bDeleteOnClose)
	: CListViewWalkerPropertySheet(pListCtrl)
{
	m_bDeleteOnClose = bDeleteOnClose;
	m_bActive = FALSE;
}

CModelessPropertySheet::~CModelessPropertySheet()
{
	if (m_bActive)
		DestroyWindow();
	for (int i = 0; i < m_interfaces.GetSize(); i++)
		m_interfaces[i]->m_propertySheets.Remove(this);
}

void CModelessPropertySheet::OpenDialog()
{
	if (!m_bActive) {
		m_bActive = TRUE;
		Create(theApp.emuledlg, WS_SYSMENU | WS_POPUP | WS_CAPTION | DS_MODALFRAME | DS_CONTEXTHELP | WS_VISIBLE | WS_MINIMIZEBOX);
	}
	ShowWindow(SW_SHOW);
	SetFocus();
}

void CModelessPropertySheet::CloseDialog()
{
	if (m_bActive)
		DestroyWindow();
}

BOOL CModelessPropertySheet::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == UM_DATA_CHANGED)
	{
		for (int i = 0; i < m_interfaces.GetSize(); i++)
			m_interfaces[i]->m_propertySheets.Remove(this);
		m_interfaces.RemoveAll();
		for (int i = 0; i < m_aItems.GetSize(); i++) {
			CModelessPropertySheetInterface* cur_interface;
			if (!object_interface_map.Lookup(m_aItems[i], cur_interface)) {
				ASSERT(FALSE);
				continue;
			}
			cur_interface->m_propertySheets.Add(this);
			m_interfaces.Add(cur_interface);
		}
}

	return __super::PreTranslateMessage(pMsg);
}

BOOL CModelessPropertySheet::OnInitDialog()
{
	// Modeless property sheets don't have Ok and Cancel buttons by default
	// This little trick fulls the property sheet into thinking it's modal
	// during it's init, so that it doesn't disable the Ok and Cancel buttons
	BOOL old_bModeless = m_bModeless;
	UINT old_nFlags = m_nFlags;
	m_bModeless = FALSE;
	m_nFlags |= WF_CONTINUEMODAL;
	BOOL bResult = CListViewWalkerPropertySheet::OnInitDialog();
	m_bModeless = old_bModeless;
	m_nFlags = old_nFlags;
	return bResult;
}

void CModelessPropertySheet::OnOK()
{
	SendMessage(WM_COMMAND, ID_APPLY_NOW);
	UpdateData();
	DestroyWindow();
}

void CModelessPropertySheet::OnCancel()
{
	DestroyWindow();
}

void CModelessPropertySheet::PostNcDestroy()
{
	m_bActive = FALSE;
	if (m_bDeleteOnClose)
		delete this;
}

void CModelessPropertySheet::RemoveData(CObject* toremove)
{
	if (!m_aItems.Remove(toremove))
		return;
	SendMessage(UM_DATA_CHANGED);

	for (int iPage = 0; iPage < GetPageCount(); iPage++)
	{
		CPropertyPage* pPage = GetPage(iPage);
		if (pPage && pPage->m_hWnd)
		{
			pPage->SendMessage(UM_DATA_CHANGED);
			pPage->SetModified(FALSE);
		}
	}
	GetActivePage()->OnSetActive();
}


// CModelessPropertySheetInterface

IMPLEMENT_DYNAMIC(CModelessPropertySheetInterface, CObject)

CModelessPropertySheetInterface::CModelessPropertySheetInterface(CObject* owner)
{
	m_owner = owner;
	object_interface_map.SetAt(m_owner, this);
}

CModelessPropertySheetInterface::~CModelessPropertySheetInterface()
{
	// Detach interface from all dialogs
	// A multi-object dialog may remain open if it has other items
	for (int i = 0; i < m_propertySheets.GetSize(); i++) {
		CModelessPropertySheet* propertySheet = m_propertySheets[i];
		propertySheet->m_interfaces.Remove(this);
		if (propertySheet->m_interfaces.GetSize())
			propertySheet->RemoveData(m_owner);
		else
			propertySheet->CloseDialog();
	}
	object_interface_map.RemoveKey(m_owner);
}

bool CModelessPropertySheetInterface::IsDialogOpen() const
{
	return m_propertySheets.GetSize();
}

void CModelessPropertySheetInterface::OpenPropertySheet(const CSimpleArray<CModelessPropertySheetInterface*>* paOthers, ...)
{
	if (paOthers->Find(this) == -1) {
		ASSERT(FALSE);
		return;
	}
	for (int i = 0; i < m_propertySheets.GetSize(); i++) {	// Do not double-open if avoidable
		CModelessPropertySheet* propertySheet = m_propertySheets[i];
		bool valid = false;
		// Since there's no add data, only accept a perfect match
		if (propertySheet->m_interfaces.GetSize() == paOthers->GetSize()) {
			valid = true;
			for (int i = 0; i < propertySheet->m_interfaces.GetSize(); i++)
				if (paOthers->Find(propertySheet->m_interfaces[i]) == -1) {
					valid = false;
					break;
				}
		}
		if (valid) {
			propertySheet->OpenDialog();	// Show dialog
			return;
		}
	}
	va_list args;
	va_start(args, paOthers);
	CModelessPropertySheet* propertySheet = CreatePropertySheet(args);
	va_end(args);
	for (int i = 0; i < paOthers->GetSize(); i++) {
		(*paOthers)[i]->m_propertySheets.Add(propertySheet);
		propertySheet->m_interfaces.Add((*paOthers)[i]);
	}
	propertySheet->OpenDialog();
}

CObject* CModelessPropertySheetInterface::GetOwner() const
{
	return m_owner;
}

int	CModelessPropertySheetInterface::GetPropertySheetCount() const
{
	return m_propertySheets.GetSize();
}

CModelessPropertySheet*	CModelessPropertySheetInterface::GetPropertySheet(int i) const
{
	return m_propertySheets[i];
}
