//this file is part of eMule
//Copyright (C)2002 Merkur ( devs@emule-project.net / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#include "stdafx.h"
#include "emule.h"
#include "emuledlg.h"
#include "SharedFilesCtrl.h"
#include "OtherFunctions.h"
#include "CommentDialog.h"
#include "FileInfoDialog.h"
#include "MetaDataDlg.h"
#include "Modeless.h"	// [TPT] - SLUGFILLER: modelessDialogs
#include "commentdialoglst.h"	// [TPT] - SLUGFILLER: showComments
#include "PreferencesDlg.h" // [TPT] - itsonlyme: virtualDirs
#include "KnownFile.h"
#include "MapKey.h"
#include "SharedFileList.h"
#include "MemDC.h"
#include "PartFile.h"
#include "MenuCmds.h"
#include "IrcWnd.h"
#include "SharedFilesWnd.h"
#include "Opcodes.h"
#include "InputBox.h"
#include "WebServices.h"
#include "TransferWnd.h"
#include "ClientList.h"
#include "UpDownClient.h"
#include "ED2kLinkDlg.h"
#include "HighColorTab.hpp"
#include "MenuXP.h"// [TPT] - New Menu Styles
#include "mod_version.h" // [TPT]
//[TPT] - Webacache
#include "Preferences.h" //JP webcache release
#include "UserMsgs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif



//////////////////////////////////////////////////////////////////////////////
// CSharedFileDetailsSheet

class CSharedFileDetailsSheet : public CModelessPropertySheet	// [TPT] - SLUGFILLER: modelessDialogs
{
	DECLARE_DYNAMIC(CSharedFileDetailsSheet)

public:
	CSharedFileDetailsSheet(CTypedPtrList<CPtrList, CKnownFile*>& aFiles, UINT uPshInvokePage = 0, CListCtrlItemWalk* pListCtrl = NULL);
	virtual ~CSharedFileDetailsSheet();

protected:
	CFileInfoDialog m_wndMediaInfo;
	CMetaDataDlg m_wndMetaData;
	CED2kLinkDlg		m_wndFileLink;
	CCommentDialogLst	m_wndComments;	// SLUGFILLER: showComments
	CCommentDialog		m_wndFileComments;

	UINT m_uPshInvokePage;
	static LPCTSTR m_pPshStartPage;

	void UpdateTitle();

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg LRESULT OnDataChanged(WPARAM, LPARAM);
};

LPCTSTR CSharedFileDetailsSheet::m_pPshStartPage;

IMPLEMENT_DYNAMIC(CSharedFileDetailsSheet, CModelessPropertySheet)	// SLUGFILLER: modelessDialogs

BEGIN_MESSAGE_MAP(CSharedFileDetailsSheet, CModelessPropertySheet)	// SLUGFILLER: modelessDialogs
	ON_WM_DESTROY()
	ON_MESSAGE(UM_DATA_CHANGED, OnDataChanged)
END_MESSAGE_MAP()

CSharedFileDetailsSheet::CSharedFileDetailsSheet(CTypedPtrList<CPtrList, CKnownFile*>& aFiles, UINT uPshInvokePage, CListCtrlItemWalk* pListCtrl)
	: CModelessPropertySheet(pListCtrl)	// SLUGFILLER: modelessDialogs
{
	m_uPshInvokePage = uPshInvokePage;
	POSITION pos = aFiles.GetHeadPosition();
	while (pos)
		m_aItems.Add(aFiles.GetNext(pos));
	m_psh.dwFlags &= ~PSH_HASHELP;
	
	m_wndMediaInfo.m_psp.dwFlags &= ~PSP_HASHELP;
	m_wndMediaInfo.m_psp.dwFlags |= PSP_USEICONID;
	m_wndMediaInfo.m_psp.pszIcon = _T("MEDIAINFO");
	m_wndMediaInfo.SetFiles(&m_aItems);
	AddPage(&m_wndMediaInfo);

	m_wndMetaData.m_psp.dwFlags &= ~PSP_HASHELP;
	m_wndMetaData.m_psp.dwFlags |= PSP_USEICONID;
	m_wndMetaData.m_psp.pszIcon = _T("METADATA");
	if (m_aItems.GetSize() == 1 && thePrefs.IsExtControlsEnabled()) {
		m_wndMetaData.SetFiles(&m_aItems);
		AddPage(&m_wndMetaData);
	}

	m_wndFileLink.m_psp.dwFlags &= ~PSP_HASHELP;
	m_wndFileLink.m_psp.dwFlags |= PSP_USEICONID;
	m_wndFileLink.m_psp.pszIcon = _T("ED2KLINK");
	m_wndFileLink.SetFiles(&m_aItems);
	AddPage(&m_wndFileLink);

	// [TPT] - SLUGFILLER: showComments
	if (m_aItems.GetSize() == 1)
	{
		m_wndComments.m_psp.dwFlags &= ~PSP_HASHELP;
		m_wndComments.m_psp.dwFlags |= PSP_USEICONID;
		m_wndComments.m_psp.pszIcon = _T("COMMENTS");
		m_wndComments.SetFiles(&m_aItems);
		AddPage(&m_wndComments);
	}	
	// [TPT] - SLUGFILLER: showComments
	
	m_wndFileComments.m_psp.dwFlags &= ~PSP_HASHELP;
	m_wndFileComments.m_psp.dwFlags |= PSP_USEICONID;
	m_wndFileComments.m_psp.pszIcon = _T("FileComments");
	m_wndFileComments.SetFiles(&m_aItems);
	AddPage(&m_wndFileComments);

	LPCTSTR pPshStartPage = m_pPshStartPage;
	if (m_uPshInvokePage != 0)
		pPshStartPage = MAKEINTRESOURCE(m_uPshInvokePage);
	for (int i = 0; i < m_pages.GetSize(); i++)
	{
		CPropertyPage* pPage = GetPage(i);
		if (pPage->m_psp.pszTemplate == pPshStartPage)
		{
			m_psh.nStartPage = i;
			break;
		}
	}
}

CSharedFileDetailsSheet::~CSharedFileDetailsSheet()
{
}

void CSharedFileDetailsSheet::OnDestroy()
{
	if (m_uPshInvokePage == 0)
		m_pPshStartPage = GetPage(GetActiveIndex())->m_psp.pszTemplate;
	CListViewWalkerPropertySheet::OnDestroy();
}

BOOL CSharedFileDetailsSheet::OnInitDialog()
{		
	EnableStackedTabs(FALSE);
	BOOL bResult = CModelessPropertySheet::OnInitDialog();	// SLUGFILLER: modelessDialogs
	HighColorTab::UpdateImageList(*this);
	InitWindowStyles(this);
	EnableSaveRestore(_T("SharedFileDetailsSheet")); // call this after(!) OnInitDialog
	UpdateTitle();
	return bResult;
}

LRESULT CSharedFileDetailsSheet::OnDataChanged(WPARAM, LPARAM)
{
	UpdateTitle();
	return 1;
}

void CSharedFileDetailsSheet::UpdateTitle()
{
	if (m_aItems.GetSize() == 1)
		SetWindowText(GetResString(IDS_DETAILS) + _T(": ") + STATIC_DOWNCAST(CKnownFile, m_aItems[0])->GetFileName());
	else
		SetWindowText(GetResString(IDS_DETAILS));
	}

BOOL CSharedFileDetailsSheet::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_APPLY_NOW)
	{
		CSharedFilesCtrl* pSharedFilesCtrl = DYNAMIC_DOWNCAST(CSharedFilesCtrl, m_pListCtrl->GetListCtrl());
		if (pSharedFilesCtrl)
		{
			for (int i = 0; i < m_aItems.GetSize(); i++) {
				// so, and why does this not(!) work while the sheet is open ??
				pSharedFilesCtrl->UpdateFile(DYNAMIC_DOWNCAST(CKnownFile, m_aItems[i]));
		}
	}
}

	return CListViewWalkerPropertySheet::OnCommand(wParam, lParam);
}


// [TPT] - SLUGFILLER: modelessDialogs
//////////////////////////////////////////////////////////////////////////////
// CSharedFileDetailsSheetInterface

CSharedFileDetailsSheetInterface::CSharedFileDetailsSheetInterface(CKnownFile* owner)
	: CModelessPropertySheetInterface(STATIC_DOWNCAST(CObject, owner))
{
}

void CSharedFileDetailsSheetInterface::OpenDetailsSheet(CTypedPtrList<CPtrList, CKnownFile*>& aFiles, UINT uPshInvokePage, CListCtrlItemWalk* pListCtrl)
{
	CSimpleArray<CModelessPropertySheetInterface*> aInterfaces;
	POSITION pos = aFiles.GetHeadPosition();
	while (pos)
		aInterfaces.Add(STATIC_DOWNCAST(CModelessPropertySheetInterface, aFiles.GetNext(pos)->GetDetailsSheetInterface()));
	OpenPropertySheet(&aInterfaces, &aFiles, uPshInvokePage, pListCtrl);
}

CModelessPropertySheet* CSharedFileDetailsSheetInterface::CreatePropertySheet(va_list args)
{
	CTypedPtrList<CPtrList, CKnownFile*>* aFiles = (CTypedPtrList<CPtrList, CKnownFile*>*)va_arg(args, LPVOID);
	UINT uPshInvokePage = va_arg(args, UINT);
	CListCtrlItemWalk* pListCtrl = va_arg(args, CListCtrlItemWalk*);
	return STATIC_DOWNCAST(CModelessPropertySheet, new CSharedFileDetailsSheet(*aFiles, uPshInvokePage, pListCtrl));
}
// [TPT] - SLUGFILLER: modelessDialogs


//////////////////////////////////////////////////////////////////////////////
// CSharedFilesCtrl

IMPLEMENT_DYNAMIC(CSharedFilesCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CSharedFilesCtrl, CMuleListCtrl)
	ON_WM_SYSCOLORCHANGE()
	ON_WM_CONTEXTMENU()
	ON_WM_MEASUREITEM()// [TPT] - New Menu Styles
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

CSharedFilesCtrl::CSharedFilesCtrl()
	: CListCtrlItemWalk(this)
{
	MEMZERO(&sortstat, sizeof(sortstat));
	nAICHHashing = 0;
}

CSharedFilesCtrl::~CSharedFilesCtrl()
{
}

void CSharedFilesCtrl::Init()
{
	CImageList ilDummyImageList; //dummy list for getting the proper height of listview entries
	ilDummyImageList.Create(1, theApp.GetSmallSytemIconSize().cy,theApp.m_iDfltImageListColorFlags|ILC_MASK, 1, 1); 
	SetImageList(&ilDummyImageList, LVSIL_SMALL);
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) == 0 );
	ilDummyImageList.Detach();

	SetExtendedStyle(LVS_EX_FULLROWSELECT);
	ModifyStyle(LVS_SINGLESEL,0);
	SetDoubleBufferStyle(); //[TPT] - Double buffer style in lists

	InsertColumn(0, GetResString(IDS_DL_FILENAME) ,LVCFMT_LEFT,250,0);
	InsertColumn(1,GetResString(IDS_DL_SIZE),LVCFMT_LEFT,100,1);
	InsertColumn(2,GetResString(IDS_TYPE),LVCFMT_LEFT,50,2);
	InsertColumn(3,GetResString(IDS_PRIORITY),LVCFMT_LEFT,70,3);
	InsertColumn(4,GetResString(IDS_FILEID),LVCFMT_LEFT,220,4);
	InsertColumn(5,GetResString(IDS_SF_REQUESTS),LVCFMT_LEFT,100,5);
	InsertColumn(6,GetResString(IDS_SF_ACCEPTS),LVCFMT_LEFT,100,6);
	InsertColumn(7,GetResString(IDS_SF_TRANSFERRED),LVCFMT_LEFT,120,7);
	InsertColumn(8,GetResString(IDS_UPSTATUS),LVCFMT_LEFT,100,8);
	InsertColumn(9,GetResString(IDS_FOLDER),LVCFMT_LEFT,200,9);
	InsertColumn(10,GetResString(IDS_COMPLSOURCES),LVCFMT_LEFT,100,10);
	InsertColumn(11,GetResString(IDS_SHAREDTITLE),LVCFMT_LEFT,200,11);
	InsertColumn(12,GetResString(IDS_PERMISSION),LVCFMT_LEFT,100,4); // [TPT] - xMule_MOD: showSharePermissions
	InsertColumn(13,GetResString(IDS_VDS_VIRTDIRTITLE),LVCFMT_LEFT,200,16);	// [TPT] - itsonlyme: virtualDirs
	InsertColumn(14,GetResString(IDS_POWERSHARE_COLUMN_LABEL),LVCFMT_LEFT,70,13);// [TPT] - Powershare
	InsertColumn(15,GetResString(IDS_HIDEOS),LVCFMT_LEFT,100,18);

	if (thePrefs.IsSpreadBarsEnable())
	{
		// [TPT] - SLUGFILLER: Spreadbars
		InsertColumn(16,GetResString(IDS_SF_UPLOADED_PARTS),LVCFMT_LEFT,170,11);	// SF
		InsertColumn(17,GetResString(IDS_SF_TURN_PART),LVCFMT_LEFT,100,12);	// SF
		InsertColumn(18,GetResString(IDS_SF_TURN_SIMPLE),LVCFMT_LEFT,100,13);	// VQB
		InsertColumn(19,GetResString(IDS_SF_FULLUPLOAD),LVCFMT_LEFT,100,14);	// SF
		// [TPT] - SLUGFILLER: Spreadbars
	}
	SetAllIcons();	// [TPT] - SLUGFILLER: showComments

	//CreateMenues(); // [TPT] - New Menu Styles

	LoadSettings(CPreferences::tableShared);

	// Barry - Use preferred sort order from preferences
	int sortItem = thePrefs.GetColumnSortItem(CPreferences::tableShared);
	bool sortAscending = thePrefs.GetColumnSortAscending(CPreferences::tableShared);
	SetSortArrow(sortItem, sortAscending);
	// [TPT] - SLUGFILLER: multiSort - load multiple params
	for (int i = thePrefs.GetColumnSortCount(CPreferences::tableShared); i > 0; ) {
		i--;
		sortItem = thePrefs.GetColumnSortItem(CPreferences::tableShared, i);
		sortAscending = thePrefs.GetColumnSortAscending(CPreferences::tableShared, i);
	SortItems(SortProc, sortItem + (sortAscending ? 0:20));
	}
	// [TPT] - SLUGFILLER: multiSort
}

// [TPT] - SLUGFILLER: showComments
void CSharedFilesCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
}

void CSharedFilesCtrl::SetAllIcons()
{
	m_ImageList.DeleteImageList();
	m_ImageList.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	m_ImageList.SetBkColor(CLR_NONE);
	m_ImageList.Add(CTempIconLoader(_T("EMPTY")));
	m_ImageList.Add(CTempIconLoader(_T("FileSharedServer"), 16, 16));
	m_ImageList.Add(CTempIconLoader(_T("FileSharedKad"), 16, 16));
	// [TPT] - SLUGFILLER: showComments
	m_ImageList.Add(CTempIconLoader(_T("RatingReceived")));
	m_ImageList.Add(CTempIconLoader(_T("BadRatingReceived")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_NotRated")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Fake")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Poor")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Good")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Fair")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Excellent")));
	// [TPT] - SLUGFILLER: showComments
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("FileCommentsOvl"))), 1);
}

void CSharedFilesCtrl::Localize() 
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
	CString strRes;

	strRes = GetResString(IDS_DL_FILENAME);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(0, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_DL_SIZE);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(1, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_TYPE);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(2, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_PRIORITY);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(3, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_FILEID);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(4, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_SF_REQUESTS);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(5, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_SF_ACCEPTS);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(6, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_SF_TRANSFERRED);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(7, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_UPSTATUS);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(8, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_FOLDER);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(9, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_COMPLSOURCES);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(10, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_SHAREDTITLE);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(11, &hdi);
	strRes.ReleaseBuffer();

	// [TPT] - xMule_MOD: showSharePermissions
	strRes = GetResString(IDS_PERMISSION);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(12, &hdi);
	strRes.ReleaseBuffer();
	// [TPT] - xMule_MOD: showSharePermissions

	// [TPT] - itsonlyme: virtualDirs
	strRes = GetResString(IDS_VDS_VIRTDIRTITLE);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(13, &hdi);
	strRes.ReleaseBuffer();
	// [TPT] - itsonlyme: virtualDirs
	
	// [TPT] - Powershare
	strRes = GetResString(IDS_POWERSHARE_COLUMN_LABEL);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(14, &hdi);
	strRes.ReleaseBuffer();
	

	strRes.ReleaseBuffer();
	strRes = GetResString(IDS_HIDEOS);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(15, &hdi);
	strRes.ReleaseBuffer();
	// [TPT] - Powershare

	if (thePrefs.IsSpreadBarsEnable())
	{
		// [TPT] - SLUGFILLER: Spreadbars
		strRes = GetResString(IDS_SF_UPLOADED_PARTS);
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(16, &hdi);
		strRes.ReleaseBuffer();

		strRes = GetResString(IDS_SF_TURN_PART);
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(17, &hdi);
		strRes.ReleaseBuffer();

		strRes = GetResString(IDS_SF_TURN_SIMPLE);
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(18, &hdi);
		strRes.ReleaseBuffer();
		
		strRes = GetResString(IDS_SF_FULLUPLOAD);
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(19, &hdi);
		strRes.ReleaseBuffer();
		// [TPT] - SLUGFILLER: Spreadbars
	}
	//CreateMenues(); // [TPT] - New Menu Styles

	int iItems = GetItemCount();
	for (int i = 0; i < iItems; i++)
		Update(i);
}

void CSharedFilesCtrl::AddFile(const CKnownFile* file)
{
	if (!theApp.emuledlg->IsRunning())
		return;
	if (FindFile(file) != -1)
		return;
	int iItem = InsertItem(LVIF_TEXT|LVIF_PARAM, GetItemCount(), LPSTR_TEXTCALLBACK, 0, 0, 0, (LPARAM)file);
	if (iItem >= 0)
		Update(iItem);
}

void CSharedFilesCtrl::RemoveFile(const CKnownFile* file)
{
	int iItem = FindFile(file);
	if (iItem != -1)
	{
		DeleteItem(iItem);
		ShowFilesCount();
	}
}

void CSharedFilesCtrl::UpdateFile(const CKnownFile* file)
{
	if(!file || !theApp.emuledlg->IsRunning())
		return;
	//[TPT] - MORPH START - SiRoB, Don't Refresh item if not needed
	if( theApp.emuledlg->activewnd != theApp.emuledlg->sharedfileswnd)
		return;
	//[TPT] - MORPH END   - SiRoB, Don't Refresh item if not needed
	int iItem = FindFile(file);
	if (iItem != -1)
	{
		Update(iItem);
		if (GetItemState(iItem, LVIS_SELECTED))
			theApp.emuledlg->sharedfileswnd->ShowSelectedFilesSummary();
	}
}

int CSharedFilesCtrl::FindFile(const CKnownFile* pFile)
{
	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)pFile;
	return FindItem(&find);
}

void CSharedFilesCtrl::ReloadFileList()
{
	DeleteAllItems();
	theApp.emuledlg->sharedfileswnd->ShowSelectedFilesSummary();
	
	CCKey bufKey;
	CKnownFile* cur_file;
	for (POSITION pos = theApp.sharedfiles->m_Files_map.GetStartPosition(); pos != 0; ){
		theApp.sharedfiles->m_Files_map.GetNextAssoc(pos, bufKey, cur_file);
		AddFile(cur_file);
	}
	ShowFilesCount();
}

void CSharedFilesCtrl::ShowFilesCount()
{
	CString str;
	if (theApp.sharedfiles->GetHashingCount() + nAICHHashing)
		str.Format(_T(" (%i, %s %i)"), theApp.sharedfiles->GetCount(), GetResString(IDS_HASHING), theApp.sharedfiles->GetHashingCount() + nAICHHashing);
	else
		str.Format(_T(" (%i)"), theApp.sharedfiles->GetCount());
	theApp.emuledlg->sharedfileswnd->GetDlgItem(IDC_TRAFFIC_TEXT)->SetWindowText(GetResString(IDS_SF_FILES) + str);
	// [TPT] - MoNKi: -Downloaded History-
	if(!theApp.emuledlg->sharedfileswnd->historylistctrl.IsWindowVisible())
		theApp.emuledlg->sharedfileswnd->GetDlgItem(IDC_BTN_CHANGEVIEW)->SetWindowText(GetResString(IDS_SF_FILES)+str);
	// [TPT] - MoNKi: -Downloaded History-

}

#define DLC_DT_TEXT (DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS)

void CSharedFilesCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if( !theApp.emuledlg->IsRunning() )
		return;
	if( !lpDrawItemStruct->itemData)
		return;

	// [TPT] - Morph. Don't draw hidden Rect
	RECT clientRect;
	GetClientRect(&clientRect);
	RECT cur_rec = lpDrawItemStruct->rcItem;
	if (cur_rec.top >= clientRect.bottom || cur_rec.bottom <= clientRect.top)
		return;
	// [TPT] - Morph. Don't draw hidden Rect

	CDC* odc = CDC::FromHandle(lpDrawItemStruct->hDC);
	BOOL bCtrlFocused = ((GetFocus() == this ) || (GetStyle() & LVS_SHOWSELALWAYS));

	CKnownFile* file = (CKnownFile*)lpDrawItemStruct->itemData;	// [TPT] - Moved here // [TPT] - SLUGFILLER: showComments - not really const, may reload local comments
		
	if( odc && (lpDrawItemStruct->itemAction | ODA_SELECT) && (lpDrawItemStruct->itemState & ODS_SELECTED )){
		if(bCtrlFocused)
			odc->SetBkColor(m_crHighlight);
		else
			odc->SetBkColor(m_crNoHighlight);
	}
	else
		// [TPT] - POWERSHARE
		if(file->GetPowerShared())	
			odc->SetBkColor(m_crPhoenix);
		else
		// [TPT] - POWERSHARE
		odc->SetBkColor(m_crWindow);

	CMemDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem);
	CFont* pOldFont = dc.SelectObject(GetFont());
	//RECT cur_rec = lpDrawItemStruct->rcItem; // [TPT] - Morph. Don't draw hidden Rect
	COLORREF crOldTextColor = dc.SetTextColor(m_crWindowText);

	int iOldBkMode;
	if (m_crWindowTextBk == CLR_NONE){
		DefWindowProc(WM_ERASEBKGND, (WPARAM)(HDC)dc, 0);
		iOldBkMode = dc.SetBkMode(TRANSPARENT);
	}
	else
		iOldBkMode = OPAQUE;


	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();
	const int iMarginX = 4;
	cur_rec.right = cur_rec.left - iMarginX*2;
	cur_rec.left += iMarginX;

	CString buffer;
	int iIconDrawWidth = theApp.GetSmallSytemIconSize().cx + 3;
	for(int iCurrent = 0; iCurrent < iCount; iCurrent++){
		int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
		if( !IsColumnHidden(iColumn) ){
			UINT uDTFlags = DLC_DT_TEXT;
			int next_left = cur_rec.left + GetColumnWidth(iColumn);	// [TPT] - SLUGFILLER: showComments - some modular coding
			cur_rec.right += GetColumnWidth(iColumn);
			// [TPT] - MORPH START - Added by SiRoB, Don't draw hidden columns
			if (cur_rec.left < clientRect.right && cur_rec.right > clientRect.left)
			{
				switch(iColumn){
					case 0:{
						// [TPT] - itsonlyme: displayOptions START
						if (thePrefs.ShowFileSystemIcon())
						{
						int iImage = theApp.GetFileTypeSystemImageIdx(file->GetFileName());
						if (theApp.GetSystemImageList() != NULL)
							::ImageList_Draw(theApp.GetSystemImageList(), iImage, dc->GetSafeHdc(), cur_rec.left, cur_rec.top, ILD_NORMAL|ILD_TRANSPARENT);
						if (!thePrefs.ShowLocalRating() && (!file->GetFileComment().IsEmpty() || file->GetFileRating()))	// SLUGFILLER: showComments
							m_ImageList.Draw(dc, 0, CPoint(cur_rec.left, cur_rec.top), ILD_NORMAL | ILD_TRANSPARENT | INDEXTOOVERLAYMASK(1));
						cur_rec.left += iIconDrawWidth;
						}
						// [TPT] - itsonlyme: displayOptions END
						// [TPT] - SLUGFILLER: showComments
						if (thePrefs.ShowLocalRating() && (!file->GetFileComment().IsEmpty() || file->GetFileRating()))
						{
							m_ImageList.Draw(dc, 5+file->GetFileRating(), CPoint(cur_rec.left, cur_rec.top), ILD_NORMAL | ILD_TRANSPARENT | (file->GetFileComment().IsEmpty()?0:INDEXTOOVERLAYMASK(1)));
							cur_rec.left+=19;
						}
						if (thePrefs.ShowRatingIndicator() && (file->HasRating() || file->HasComment()))
						{
							m_ImageList.Draw(dc, file->HasBadRating()?4:3, CPoint(cur_rec.left, cur_rec.top), ILD_NORMAL);
							cur_rec.left+=11;
						}
						// [TPT] - SLUGFILLER: showComments
						// [TPT] - xMule_MOD: showSharePermissions, modified by itsonlyme
						// display not finished files in navy, blocked files in red and friend-only files in orange
						if (file->GetPermissions() == PERM_NOONE)
							dc->SetTextColor((COLORREF)RGB(240,0,0));
						else if (file->GetPermissions() == PERM_FRIENDS)
							dc->SetTextColor((COLORREF)RGB(208,128,0));
						else if (file->IsPartFile())
							dc->SetTextColor((COLORREF)RGB(0,0,192));
						// [TPT] - xMule_MOD: showSharePermissions
						buffer = file->GetFileName();
						break;
					}
					case 1:
						buffer = CastItoXBytes(file->GetFileSize(), false, false);
						uDTFlags |= DT_RIGHT;
						break;
					case 2:
						buffer = file->GetFileTypeDisplayStr();
						break;
					case 3:{
						switch (file->GetUpPriority()) {
							case PR_VERYLOW :
								buffer = GetResString(IDS_PRIOVERYLOW);
								break;
							case PR_LOW :
								if( file->IsAutoUpPriority() )
									buffer = GetResString(IDS_PRIOAUTOLOW);
								else
									buffer = GetResString(IDS_PRIOLOW);
								break;
							case PR_NORMAL :
								if( file->IsAutoUpPriority() )
									buffer = GetResString(IDS_PRIOAUTONORMAL);
								else
									buffer = GetResString(IDS_PRIONORMAL);
								break;
							case PR_HIGH :
								if( file->IsAutoUpPriority() )
									buffer = GetResString(IDS_PRIOAUTOHIGH);
								else
									buffer = GetResString(IDS_PRIOHIGH);
								break;
							case PR_VERYHIGH :
								buffer = GetResString(IDS_PRIORELEASE);
								break;
							default:
								buffer.Empty();	
							}
						// [TPT] - Powershare
						if(file->GetPowerShared()) {
							CString tempString = GetResString(IDS_POWERSHARE_PREFIX);
							tempString.Append(_T(" "));
							tempString.Append(buffer);
							buffer.Empty();
							buffer = tempString;
						}
						// [TPT] - Powershare end
						break;
					}
					case 4:
						buffer = md4str(file->GetFileHash());
						break;
					case 5:
	                    buffer.Format(_T("%u (%u)"), file->statistic.GetRequests(), file->statistic.GetAllTimeRequests());
						break;
					case 6:
						buffer.Format(_T("%u (%u)"), file->statistic.GetAccepts(), file->statistic.GetAllTimeAccepts());
						break;
					case 7:
						buffer.Format(_T("%s (%s)"), CastItoXBytes(file->statistic.GetTransferred(), false, false), CastItoXBytes(file->statistic.GetAllTimeTransferred(), false, false));
						break;
					case 8:
						if( file->GetPartCount()){
							cur_rec.bottom--;
							cur_rec.top++;
							file->DrawShareStatusBar(dc,&cur_rec,false,thePrefs.UseFlatBar());
							cur_rec.bottom++;
							cur_rec.top--;
						}
						break;
					case 9:
						buffer = file->GetPath();
						PathRemoveBackslash(buffer.GetBuffer());
						buffer.ReleaseBuffer();
						break;
					case 10:{
						if (file->m_nCompleteSourcesCountLo == 0){
							buffer.Format(_T("< %u"), file->m_nCompleteSourcesCountHi);	
						}
						else if (file->m_nCompleteSourcesCountLo == file->m_nCompleteSourcesCountHi)
							buffer.Format(_T("%u"), file->m_nCompleteSourcesCountLo);
						else
							buffer.Format(_T("%u - %u"), file->m_nCompleteSourcesCountLo, file->m_nCompleteSourcesCountHi);
						// [TPT] - Powershare
						CString buffer2;
						buffer2.Format(_T(" (%u)"),file->m_nVirtualCompleteSourcesCount);
						buffer.Append(buffer2);
						// [TPT] - Powershare End					
						break;}
					case 11:{
						CPoint pt(cur_rec.left, cur_rec.top);
					m_ImageList.Draw(dc, file->GetPublishedED2K() ? 1 : 0, pt, ILD_NORMAL | ILD_TRANSPARENT);
					pt.x += 16;
						bool bSharedInKad;
						if ((uint32)time(NULL) < file->GetLastPublishTimeKadSrc())
						{
							if (theApp.IsFirewalled() && theApp.IsConnected())
							{
								if (theApp.clientlist->GetBuddy() && (file->GetLastPublishBuddy() == theApp.clientlist->GetBuddy()->GetIP()))
									bSharedInKad = true;
								else
									bSharedInKad = false;
							}
							else
								bSharedInKad = true;
						}
						else
							bSharedInKad = false;
					m_ImageList.Draw(dc, bSharedInKad ? 2 : 0, pt, ILD_NORMAL | ILD_TRANSPARENT);
					buffer.Empty();
						break;
					}
					case 12:
						// [TPT] - xMule_MOD: showSharePermissions
						switch (file->GetPermissions())
						{
							case PERM_NOONE: 
								buffer = GetResString(IDS_HIDDEN); 
								break;
							case PERM_FRIENDS: 
								buffer = GetResString(IDS_FSTATUS_FRIENDSONLY); 
								break;
							default: 
								buffer = GetResString(IDS_FSTATUS_PUBLIC);
								break;
						}
						break;
						// [TPT] - xMule_MOD: showSharePermissions
					// [TPT] - itsonlyme: virtualDirs
					case 13:
	
						buffer.Format(_T("%s"),file->GetPath(true));
						break;
					// [TPT] - itsonlyme: virtualDirs
					// [TPT] - Powershare
					case 14:{
						int powersharemode;
						bool powershared = file->GetPowerShared();
						buffer = _T("[") + GetResString((powershared)?IDS_POWERSHARE_ON_LABEL:IDS_POWERSHARE_OFF_LABEL) + _T("] ");
						if (file->GetPowerSharedMode()>=0)
							powersharemode = file->GetPowerSharedMode();
						else {
							powersharemode = thePrefs.GetPowerShareMode();
							buffer.Append(_T(" ") + ((CString)GetResString(IDS_DEFAULT)).Left(1) + _T(". "));
						}
						if(powersharemode == 2)
							buffer.Append(GetResString(IDS_POWERSHARE_AUTO_LABEL));
						else if (powersharemode == 1)
							buffer.Append(GetResString(IDS_POWERSHARE_ACTIVATED_LABEL));
						else if (powersharemode == 3) {
							buffer.Append(GetResString(IDS_POWERSHARE_LIMITED));
							if (file->GetPowerShareLimit()<0)
								buffer.AppendFormat(_T(" %s. %i"), ((CString)GetResString(IDS_DEFAULT)).Left(1), thePrefs.GetPowerShareLimit());
							else
								buffer.AppendFormat(_T(" %i"), file->GetPowerShareLimit());
						}
						else
							buffer.Append(GetResString(IDS_POWERSHARE_DISABLED_LABEL));
						buffer.Append(_T(" ("));
						if (file->IsPartFile())
							buffer.Append(GetResString(IDS_DOWNLOADING));
						else if (file->GetPowerShareAuto())
							buffer.Append(GetResString(IDS_POWERSHARE_ADVISED_LABEL));
						else if (file->GetPowerShareLimited() && (powersharemode == 3))
							buffer.Append(GetResString(IDS_POWERSHARE_LIMITED));
						else if (file->GetPowerShareAuthorized())
							buffer.Append(GetResString(IDS_POWERSHARE_AUTHORIZED_LABEL));
						else
							buffer.Append(GetResString(IDS_POWERSHARE_DENIED_LABEL));
						buffer.Append(_T(")"));
						break;
							}
					
					case 15:
						if(file->GetHideOS()>=0)
							if (file->GetHideOS()){
								buffer.Format(_T("%i"), file->GetHideOS());
								if (file->GetSelectiveChunk()>=0)
									if (file->GetSelectiveChunk())
										buffer.AppendFormat(_T(" + %s") ,GetResString(IDS_SELECTIVESHARE));
							}else
								buffer = GetResString(IDS_DISABLED);
						else
							buffer = GetResString(IDS_DEFAULT);
						break;
					// [TPT] - Powershare END
					// [TPT] - SLUGFILLER: Spreadbars
					case 16:
						{
							cur_rec.bottom--;
							cur_rec.top++;
							file->statistic.DrawSpreadBar(dc,&cur_rec,thePrefs.UseFlatBar());
							cur_rec.bottom++;
							cur_rec.top--;
							cur_rec.left = next_left;
						}
						continue;
					case 17:
						buffer.Format(_T("%.2f"),file->statistic.GetSpreadSortValue());
						break;
					case 18:
						if (file->GetFileSize())
							buffer.Format(_T("%.2f"),((float)file->statistic.GetAllTimeTransferred())/((float)file->GetFileSize()));
						else
							buffer.Format(_T("%.2f"),0.0f);
						break;
					case 19:
						buffer.Format(_T("%.2f"),file->statistic.GetFullSpreadCount());
						break;
					// [TPT] - SLUGFILLER: Spreadbars
				}
	
				if( iColumn != 8 && iColumn != 16)
					dc->DrawText(buffer, buffer.GetLength(),&cur_rec,uDTFlags);
			}
			// [TPT] - MORPH END   - Added by SiRoB, Don't draw hidden coloms
			cur_rec.left = next_left;	// [TPT] - SLUGFILLER: showComments - some modular coding
		}
	}
	ShowFilesCount();
	if ((lpDrawItemStruct->itemAction | ODA_SELECT) && (lpDrawItemStruct->itemState & ODS_SELECTED))
	{
		RECT outline_rec = lpDrawItemStruct->rcItem;

		outline_rec.top--;
		outline_rec.bottom++;
		dc->FrameRect(&outline_rec, &CBrush(m_crWindow));
		outline_rec.top++;
		outline_rec.bottom--;
		outline_rec.left++;
		outline_rec.right--;

		if (lpDrawItemStruct->itemID > 0 && GetItemState(lpDrawItemStruct->itemID - 1, LVIS_SELECTED))
			outline_rec.top--;

		if (lpDrawItemStruct->itemID + 1 < (UINT)GetItemCount() && GetItemState(lpDrawItemStruct->itemID + 1, LVIS_SELECTED))
			outline_rec.bottom++;

		if(bCtrlFocused)
			dc->FrameRect(&outline_rec, &CBrush(m_crFocusLine));
		else
			dc->FrameRect(&outline_rec, &CBrush(m_crNoFocusLine));
	}
	
	if (m_crWindowTextBk == CLR_NONE)
		dc.SetBkMode(iOldBkMode);
	dc.SelectObject(pOldFont);
	dc.SetTextColor(crOldTextColor);
}


// [TPT] - New Menu Styles BEGIN
void CSharedFilesCtrl::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	HMENU hMenu = AfxGetThreadState()->m_hTrackingMenu;
	CMenu	*pMenu = CMenu::FromHandle(hMenu);
	pMenu->MeasureItem(lpMeasureItemStruct);
	
	CWnd::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}


void CSharedFilesCtrl::OnContextMenu(CWnd* pWnd, CPoint point)
{
	//[TPT] - Webcache
	bool uWCReleaseItem = true; //JP webcache release
	bool uGreyOutWCRelease = true; //JP webcache release
	bool bFirstItem = true;
	int iSelectedItems = GetSelectedCount();
	int iCompleteFileSelected = -1;
	int iPowerShareLimit = -1; 
	int iHideOS = -1;
	bool bVirtRemove = false;	// [TPT] - itsonlyme: virtualDirs

	// Main menu
	CMenuXP *pMenu = new CMenuXP;
	pMenu->CreatePopupMenu();
	pMenu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
	pMenu->AddSideBar(new CMenuXPSideBar(17, MOD_VERSION));
	pMenu->SetSideBarStartColor(RGB(255,0,0));
	pMenu->SetSideBarEndColor(RGB(255,128,0));
	pMenu->SetSelectedBarColor(RGB(242,120,114));
	//[TPT] - Bitmap in menus
	if(thePrefs.GetShowBitmapInMenus())
	{
		pMenu->SetBackBitmap(_T("FONDOCOMPARTIDOS"), _T("JPG"));
	}

	// PowerShare Menu
	CMenuXP *pPowershareMenu = new CMenuXP;
	pPowershareMenu->CreatePopupMenu();
	pPowershareMenu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
	pPowershareMenu->SetSelectedBarColor(RGB(242,120,114));

	// Priority Menu
	CMenuXP *pPrioMenu = new CMenuXP;
	pPrioMenu->CreatePopupMenu();
	pPrioMenu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
	pPrioMenu->SetSelectedBarColor(RGB(242,120,114));

	// Virtual dirs
	CMenuXP *pVirtualMenu = new CMenuXP;
	pVirtualMenu->CreatePopupMenu();
	pVirtualMenu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
	pVirtualMenu->SetSelectedBarColor(RGB(242,120,114));
	pVirtualMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_IOM_VIRTFILE,GetResString(IDS_VDS_MFILE)));
	pVirtualMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_IOM_VIRTDIR,GetResString(IDS_VDS_MDIR)));
	pVirtualMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_IOM_VIRTSUBDIR,GetResString(IDS_VDS_MSDIR)));
	pVirtualMenu->AppendSeparator();
	pVirtualMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_IOM_VIRTREMOVE,GetResString(IDS_VDS_REMOVE)));
	pVirtualMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_IOM_VIRTPREFS,GetResString(IDS_VDS_ADVANCED)));


	// Permissions Menu
	CMenuXP *pPermMenu = new CMenuXP;
	pPermMenu->CreatePopupMenu();
	pPermMenu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
	pPermMenu->SetSelectedBarColor(RGB(242,120,114));

	// Web services Menu
	CMenuXP *pWebMenu = new CMenuXP;
	pWebMenu->CreatePopupMenu();
	pWebMenu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
	pWebMenu->SetSelectedBarColor(RGB(242,120,114));

	// Hide OS Menu
	CMenuXP *pHideOSMenu = new CMenuXP;
	pHideOSMenu->CreatePopupMenu();
	pHideOSMenu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
	pHideOSMenu->SetSelectedBarColor(RGB(242,120,114));

	// Selective Chunck Sharing Menu
	CMenuXP *pSelectiveChunkMenu = new CMenuXP;
	pSelectiveChunkMenu->CreatePopupMenu();
	pSelectiveChunkMenu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
	pSelectiveChunkMenu->SetSelectedBarColor(RGB(242,120,114));

	// PowerShareLimit 
	CMenuXP *pPowershareLimitMenu = new CMenuXP;
	pPowershareLimitMenu->CreatePopupMenu();
	pPowershareLimitMenu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
	pPowershareLimitMenu->SetSelectedBarColor(RGB(242,120,114));

	UINT uPrioMenuItem = 0;
	UINT uPermMenuItem = 0; 
	UINT uPowershareMenuItem = 0; 
	UINT uPowerShareLimitMenuItem = 0; 

	UINT uHideOSMenuItem = 0; 
	UINT uSelectiveChunkMenuItem = 0; 
	const CKnownFile* pSingleSelFile = NULL;
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos)
	{
		const CKnownFile* pFile = (CKnownFile*)GetItemData(GetNextSelectedItem(pos));
		if (bFirstItem)
			pSingleSelFile = pFile;
		else
			pSingleSelFile = NULL;

		int iCurCompleteFile = pFile->IsPartFile() ? 0 : 1;

		if (bFirstItem)
			iCompleteFileSelected = iCurCompleteFile;
		else if (iCompleteFileSelected != iCurCompleteFile)
			iCompleteFileSelected = -1;

		UINT uCurPrioMenuItem = 0;
		if (pFile->IsAutoUpPriority())
			uCurPrioMenuItem = MP_PRIOAUTO;
		else if (pFile->GetUpPriority() == PR_VERYLOW)
			uCurPrioMenuItem = MP_PRIOVERYLOW;
		else if (pFile->GetUpPriority() == PR_LOW)
			uCurPrioMenuItem = MP_PRIOLOW;
		else if (pFile->GetUpPriority() == PR_NORMAL)
			uCurPrioMenuItem = MP_PRIONORMAL;
		else if (pFile->GetUpPriority() == PR_HIGH)
			uCurPrioMenuItem = MP_PRIOHIGH;
		else if (pFile->GetUpPriority() == PR_VERYHIGH)
			uCurPrioMenuItem = MP_PRIOVERYHIGH;
		else
			ASSERT(0);

		if (bFirstItem)
			uPrioMenuItem = uCurPrioMenuItem;
		else if (uPrioMenuItem != uCurPrioMenuItem)
			uPrioMenuItem = 0;
			
// [TPT] - WebCache ////////////////////////////////////////////////////////////////////////////////////
		//jp webcache release start
		if (!pFile->ReleaseViaWebCache)
			uWCReleaseItem = false;
		if (!pFile->IsPartFile())
			uGreyOutWCRelease = false;
		//jp webcache release end			

		// [TPT] - xMule_MOD: showSharePermissions
		UINT uCurPermMenuItem = 0;
		if (pFile->GetPermissions() == PERM_ALL)
			uCurPermMenuItem = MP_PERMALL;
		else if (pFile->GetPermissions() == PERM_FRIENDS)
			uCurPermMenuItem = MP_PERMFRIENDS;
		else if (pFile->GetPermissions() == PERM_NOONE)
			uCurPermMenuItem = MP_PERMNONE;
		else
			ASSERT(0);

		if (bFirstItem)
			uPermMenuItem = uCurPermMenuItem;
		else if (uPermMenuItem != uCurPermMenuItem)
			uPermMenuItem = 0;


		UINT uCurPowershareMenuItem = 0;
		if (pFile->GetPowerSharedMode()==-1)
			uCurPowershareMenuItem = MP_POWERSHARE_DEFAULT;
		else
			uCurPowershareMenuItem = MP_POWERSHARE_DEFAULT+1 + pFile->GetPowerSharedMode();

		if (bFirstItem)
			uPowershareMenuItem = uCurPowershareMenuItem;
		else if (uPowershareMenuItem != uCurPowershareMenuItem)
			uPowershareMenuItem = 0;

		UINT uCurPowerShareLimitMenuItem = 0;
		int iCurPowerShareLimit = pFile->GetPowerShareLimit();
		if (iCurPowerShareLimit==-1)
			uCurPowerShareLimitMenuItem = MP_POWERSHARE_LIMIT;
		else
			uCurPowerShareLimitMenuItem = MP_POWERSHARE_LIMIT_SET;

		if (bFirstItem)
		{
			uPowerShareLimitMenuItem = uCurPowerShareLimitMenuItem;
			iPowerShareLimit = iCurPowerShareLimit;
		}
		else if (uPowerShareLimitMenuItem != uCurPowerShareLimitMenuItem || iPowerShareLimit != iCurPowerShareLimit)
		{
			uPowerShareLimitMenuItem = 0;
			iPowerShareLimit = -1;
		}


		UINT uCurHideOSMenuItem = 0;
		int iCurHideOS = pFile->GetHideOS();
		if (iCurHideOS == -1)
			uCurHideOSMenuItem = MP_HIDEOS_DEFAULT;
		else
			uCurHideOSMenuItem = MP_HIDEOS_SET;
		if (bFirstItem)
		{
			uHideOSMenuItem = uCurHideOSMenuItem;
			iHideOS = iCurHideOS;
		}
		else if (uHideOSMenuItem != uCurHideOSMenuItem || iHideOS != iCurHideOS)
		{
			uHideOSMenuItem = 0;
			iHideOS = -1;
		}

		UINT uCurSelectiveChunkMenuItem = 0;
		if (pFile->GetSelectiveChunk() == -1)
			uCurSelectiveChunkMenuItem = MP_SELECTIVE_CHUNK;
		else
			uCurSelectiveChunkMenuItem = MP_SELECTIVE_CHUNK+1 + pFile->GetSelectiveChunk();
		if (bFirstItem)
			uSelectiveChunkMenuItem = uCurSelectiveChunkMenuItem;
		else if (uSelectiveChunkMenuItem != uCurSelectiveChunkMenuItem)
			uSelectiveChunkMenuItem = 0;

		// [TPT] - itsonlyme: virtualDirs
		if (!bVirtRemove) {
			CString virt, fileID;
				fileID.Format(_T("%i:%s"), pFile->GetFileSize(), EncodeBase16(pFile->GetFileHash(),16));
			bVirtRemove = thePrefs.GetFileToVDirMap()->Lookup(fileID, virt);
			if (!bVirtRemove) {
				CString path = pFile->GetPath();
				path.MakeLower();
				path.TrimRight(_T("\\"));
				bVirtRemove = thePrefs.GetDirToVDirMap()->Lookup(fileID, virt);
				if (!bVirtRemove)
					bVirtRemove = thePrefs.GetSubDirToVDirMap()->Lookup(fileID, virt);
			}
		}
		// [TPT] - itsonlyme: virtualDirs

		bFirstItem = false;
	}

	bool bSingleCompleteFileSelected = (iSelectedItems == 1 && iCompleteFileSelected == 1);
	UINT uInsertedMenuItem = 0;
	static const TCHAR _szSkinPkgSuffix[] = _T(".") EMULSKIN_BASEEXT _T(".zip");
	if (bSingleCompleteFileSelected 
		&& pSingleSelFile 
		&& pSingleSelFile->GetFilePath().Right(ARRSIZE(_szSkinPkgSuffix)-1).CompareNoCase(_szSkinPkgSuffix) == 0)
	{
		MENUITEMINFO mii = {0};
		mii.cbSize = sizeof mii;
		mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
		mii.fType = MFT_STRING;
		mii.fState = MFS_ENABLED;
		mii.wID = MP_INSTALL_SKIN;
		CString strBuff(GetResString(IDS_INSTALL_SKIN));
		mii.dwTypeData = const_cast<LPTSTR>((LPCTSTR)strBuff);
		if (pMenu->InsertMenuItem(MP_OPENFOLDER, &mii, FALSE))
			uInsertedMenuItem = mii.wID;
	}
	
	pMenu->AppendODMenu(bSingleCompleteFileSelected ? MF_ENABLED : MF_GRAYED, new CMenuXPText(MP_OPEN, GetResString(IDS_OPENFILE)));
	pMenu->AppendODMenu(bSingleCompleteFileSelected ? MF_ENABLED : MF_GRAYED, new CMenuXPText(MP_OPENFOLDER, GetResString(IDS_OPENFOLDER)));
	pMenu->AppendODMenu(bSingleCompleteFileSelected ? MF_ENABLED : MF_GRAYED, new CMenuXPText(MP_RENAME, GetResString(IDS_RENAME) + _T("...")));
	pMenu->AppendODMenu(iCompleteFileSelected > 0 ? MF_ENABLED : MF_GRAYED, new CMenuXPText(MP_REMOVE,GetResString(IDS_DELETE), theApp.LoadIcon(_T("delete"), 16, 16)));
	pMenu->SetDefaultItem(bSingleCompleteFileSelected ? MP_OPEN : -1);

	pMenu->AppendSeparator();
	pMenu->AppendODPopup(iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED, pPermMenu, new CMenuXPText(0, GetResString(IDS_PERMISSION),theApp.LoadIcon(_T("permission"), 16, 16)));

	pPermMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_PERMNONE,GetResString(IDS_HIDDEN)));
	pPermMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_PERMFRIENDS,GetResString(IDS_FSTATUS_FRIENDSONLY)));
	pPermMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_PERMALL,GetResString(IDS_FSTATUS_PUBLIC)));
	pPermMenu->CheckMenuRadioItem(MP_PERMALL, MP_PERMNONE, uPermMenuItem, 0);
	// [TPT] - Powershare
	CString buffer;
	pMenu->AppendODPopup((iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED), pPowershareMenu, new CMenuXPText(0, GetResString(IDS_POWERSHARE), theApp.LoadIcon(_T("picon"),16,16)));
	switch (thePrefs.GetPowerShareMode())
	{
	case 0:
		buffer.Format(_T(" (%s)"),GetResString(IDS_POWERSHARE_DISABLED));
		break;
	case 1:
		buffer.Format(_T(" (%s)"),GetResString(IDS_POWERSHARE_ACTIVATED));
		break;
	case 2:
		buffer.Format(_T(" (%s)"),GetResString(IDS_POWERSHARE_AUTO));
		break;
	case 3:
		buffer.Format(_T(" (%s)"),GetResString(IDS_POWERSHARE_LIMITED));
		break;
	default:
		buffer = _T(" (?)");
		break;
	}
	pPowershareMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_POWERSHARE_DEFAULT,GetResString(IDS_DEFAULT)+buffer));
	pPowershareMenu->AppendSeparator();
	pPowershareMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_POWERSHARE_OFF,GetResString(IDS_POWERSHARE_OFF_LABEL)));
	pPowershareMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_POWERSHARE_ON,GetResString(IDS_POWERSHARE_ON_LABEL)));
	pPowershareMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_POWERSHARE_AUTO,GetResString(IDS_POWERSHARE_AUTO)));
	pPowershareMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_POWERSHARE_LIMITED,GetResString(IDS_POWERSHARE_LIMITED)));
	pPowershareMenu->CheckMenuRadioItem(MP_POWERSHARE_DEFAULT, MP_POWERSHARE_LIMITED, uPowershareMenuItem, 0);

	pPowershareMenu->AppendSeparator();
	pPowershareMenu->AppendODPopup((iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED), pPowershareLimitMenu, new CMenuXPText(0, GetResString(IDS_POWERSHARE_LIMIT)));

	if (iPowerShareLimit==0)
		buffer.Format(_T(" (%s)"),GetResString(IDS_DISABLED));
	else
		buffer.Format(_T(" (%u)"),thePrefs.GetPowerShareLimit());
	pPowershareLimitMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_POWERSHARE_LIMIT,GetResString(IDS_DEFAULT)+buffer));
	if (iPowerShareLimit==-1)
		buffer = GetResString(IDS_SET);
	else if (iPowerShareLimit==0)
		buffer = GetResString(IDS_DISABLED);
	else
		buffer.Format(_T("[%i]"),iPowerShareLimit);
	pPowershareLimitMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_POWERSHARE_LIMIT_SET, buffer));
	pPowershareLimitMenu->CheckMenuRadioItem(MP_POWERSHARE_LIMIT, MP_POWERSHARE_LIMIT_SET, uPowerShareLimitMenuItem, 0);

	pMenu->AppendODPopup((iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED), pHideOSMenu,  new CMenuXPText(0, GetResString(IDS_HIDEOS)));

	if (thePrefs.GetHideOvershares()==0)
		buffer.Format(_T(" (%s)"),GetResString(IDS_DISABLED));
	else
		buffer.Format(_T(" (%u)"),thePrefs.GetHideOvershares());
	pHideOSMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_HIDEOS_DEFAULT, GetResString(IDS_DEFAULT)+buffer));

	if (iHideOS==-1)
		buffer = GetResString(IDS_SET);
	else if (iHideOS==0)
		buffer = GetResString(IDS_DISABLED);
	else
		buffer.Format(_T("[%i]"), iHideOS);
	pHideOSMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_HIDEOS_SET, buffer));
	pHideOSMenu->CheckMenuRadioItem(MP_HIDEOS_DEFAULT, MP_HIDEOS_SET, uHideOSMenuItem, 0);
	
	pHideOSMenu->AppendSeparator();
	pHideOSMenu->AppendODPopup((iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED), pSelectiveChunkMenu, new CMenuXPText(0, GetResString(IDS_SELECTIVESHARE)));
	

	// Selective Chunk 
	buffer.Format(_T(" (%s)"),thePrefs.IsSelectiveShareEnabled()? GetResString(IDS_ENABLED):GetResString(IDS_DISABLED));
	pSelectiveChunkMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_SELECTIVE_CHUNK,GetResString(IDS_DEFAULT)+buffer));
	pSelectiveChunkMenu->AppendSeparator();
	pSelectiveChunkMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_SELECTIVE_CHUNK_0,GetResString(IDS_DISABLED)));
	pSelectiveChunkMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_SELECTIVE_CHUNK_1,GetResString(IDS_ENABLED)));
	pSelectiveChunkMenu->CheckMenuRadioItem(MP_SELECTIVE_CHUNK, MP_SELECTIVE_CHUNK_1, uSelectiveChunkMenuItem, 0);

	//PrioMenu
	pMenu->AppendODPopup(iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED, pPrioMenu, new CMenuXPText(0, GetResString(IDS_PRIORITY),theApp.LoadIcon(_T("PRIORITY"), 16, 16)));
	pPrioMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_PRIOVERYLOW,GetResString(IDS_PRIOVERYLOW)));
	pPrioMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_PRIOLOW,GetResString(IDS_PRIOLOW)));
	pPrioMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_PRIONORMAL,GetResString(IDS_PRIONORMAL)));
	pPrioMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_PRIOHIGH,GetResString(IDS_PRIOHIGH)));
	pPrioMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_PRIOVERYHIGH,GetResString(IDS_PRIORELEASE)));
	pPrioMenu->AppendSeparator();
	pPrioMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_PRIOAUTO,GetResString(IDS_PRIOAUTO)));
	pPrioMenu->AppendSeparator();
	pPrioMenu->AppendODMenu(MF_STRING | (thePrefs.UpdateWebcacheReleaseAllowed() && !uGreyOutWCRelease) ? MF_ENABLED : MF_GRAYED, new CMenuXPText(MP_PRIOWCRELEASE,GetResString(IDS_WC_RELEASE)));
	if (uWCReleaseItem && thePrefs.IsWebcacheReleaseAllowed()) //JP webcache release
		pPrioMenu->CheckMenuItem(MP_PRIOWCRELEASE, MF_CHECKED);
	else
		pPrioMenu->CheckMenuItem(MP_PRIOWCRELEASE, MF_UNCHECKED);

	pPrioMenu->CheckMenuRadioItem(MP_PRIOVERYLOW, MP_PRIOAUTO, uPrioMenuItem, 0);
	pMenu->AppendODPopup(MF_STRING | (iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED), pVirtualMenu, new CMenuXPText(0, GetResString(IDS_VDS_VIRTDIRTITLE),theApp.LoadIcon(_T("virtualDir"), 16, 16)));
	// [TPT] - itsonlyme: virtualDirs	
	pVirtualMenu->EnableMenuItem(MP_IOM_VIRTREMOVE, bVirtRemove ? MF_ENABLED : MF_GRAYED);
	// [TPT] - itsonlyme: virtualDirs	
	pMenu->AppendSeparator();
	pMenu->AppendODMenu(MF_STRING | (iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_CMT,GetResString(IDS_CMT_ADD))); // [TPT] - SLUGFILLER: batchComment	
	pMenu->AppendODMenu(MF_STRING | (iSelectedItems == 1 ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_VIEWFILECOMMENTS,GetResString(IDS_CMT_SHOWALL), theApp.LoadIcon(_T("comments"), 16, 16)));	
	pMenu->AppendODMenu(MF_STRING | (iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_DETAIL, GetResString(IDS_SHOWDETAILS), theApp.LoadIcon(_T("information"), 16, 16)));
	pMenu->AppendSeparator();	
	
	if (thePrefs.GetShowCopyEd2kLinkCmd())
		pMenu->AppendODMenu(MF_STRING | (iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_GETED2KLINK, GetResString(IDS_COPY), theApp.LoadIcon(_T("ED2KLINK"), 16, 16)));
	else
		pMenu->AppendODMenu(MF_STRING | (iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_SHOWED2KLINK, GetResString(IDS_DL_SHOWED2KLINK), theApp.LoadIcon(_T("COPY"), 16, 16)));
	
	//JOHNTODO: Not for release as we need kad lowID users in the network to see how well this work work. Also, we do not support these links yet.
	#if defined(_DEBUG)	
	if (iSelectedItems > 0 && theApp.IsConnected() && theApp.IsFirewalled() && theApp.clientlist->GetBuddy())
		pMenu->AppendODMenu(MF_STRING | MF_ENABLED, new CMenuXPText(MP_GETKADSOURCELINK,  _T("Copy eD2K Links To Clipboard (Kad)")));
	else
		pMenu->AppendODMenu(MF_STRING | MF_GRAYED, new CMenuXPText(MP_GETKADSOURCELINK,  _T("Copy eD2K Links To Clipboard (Kad)")));
	#endif

	pMenu->AppendSeparator();	
	pMenu->AppendODMenu(iSelectedItems == 1 && theApp.emuledlg->ircwnd->IsConnected() ? MF_ENABLED : MF_GRAYED, new CMenuXPText(Irc_SetSendLink,GetResString(IDS_IRC_ADDLINKTOIRC)));
	pMenu->AppendSeparator();

	// Web services
	int iWebMenuEntries = theWebServices.GetFileMenuEntries(pWebMenu);
	UINT flag2 = (iWebMenuEntries == 0 || iSelectedItems != 1) ? MF_GRAYED : MF_STRING;
	pMenu->AppendODPopup(MF_STRING | flag2 | MF_POPUP, pWebMenu, new CMenuXPText(0,GetResString(IDS_WEBSERVICES),theApp.LoadIcon(_T("webServices"), 16, 16)));


	pMenu->TrackPopupMenu(TPM_LEFTBUTTON, point.x, point.y, this);
	
	delete pVirtualMenu;	
	delete pPermMenu;
	delete pPrioMenu;
	delete pWebMenu;
	delete pPowershareLimitMenu;
	delete pPowershareMenu;
	delete pSelectiveChunkMenu;
	delete pHideOSMenu;
	if (uInsertedMenuItem)
		pMenu->RemoveMenu(uInsertedMenuItem, MF_BYCOMMAND);
	delete pMenu;

}

// [TPT] - New Menu Styles END

BOOL CSharedFilesCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	CTypedPtrList<CPtrList, CKnownFile*> selectedList;
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos != NULL){
		int index = GetNextSelectedItem(pos);
		if (index >= 0)
			selectedList.AddTail((CKnownFile*)GetItemData(index));
	}

	if (selectedList.GetCount() > 0)
	{
		CKnownFile* file = NULL;
		if (selectedList.GetCount() == 1)
			file = selectedList.GetHead();

		switch (wParam){
			case Irc_SetSendLink:
				if (file)
					theApp.emuledlg->ircwnd->SetSendFileString(CreateED2kLink(file));
				break;
			case MP_GETED2KLINK:{
				CString str;
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL)
				{
					file = selectedList.GetNext(pos);
					if (!str.IsEmpty())
						str += _T("\r\n");
					str += CreateED2kLink(file);
				}
				theApp.CopyTextToClipboard(str);
				break;
			}
			#if defined(_DEBUG)
			//JOHNTODO: Not for release as we need kad lowID users in the network to see how well this work work. Also, we do not support these links yet.
			case MP_GETKADSOURCELINK:{
				CString str;
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL)
				{
					file = selectedList.GetNext(pos);
					if (!str.IsEmpty())
						str += _T("\r\n");
					str += theApp.CreateKadSourceLink(file);
				}
				theApp.CopyTextToClipboard(str);
				break;
			}
			#endif			
			// file operations
			case MP_OPEN:
				if (file && !file->IsPartFile())
					OpenFile(file);
				break; 
			case MP_INSTALL_SKIN:
				if (file && !file->IsPartFile())
					InstallSkin(file->GetFilePath());
				break;
			case MP_OPENFOLDER:
				if (file && !file->IsPartFile()){
					CString path = file->GetPath();
					int bspos = path.ReverseFind(_T('\\'));
					ShellExecute(NULL, _T("open"), path.Left(bspos), NULL, NULL, SW_SHOW);
				}
				break; 
			case MP_RENAME:
			case MPG_F2:
				if (file && !file->IsPartFile()){
					InputBox inputbox;
					CString title = GetResString(IDS_RENAME);
					title.Remove(_T('&'));
					inputbox.SetLabels(title, GetResString(IDS_DL_FILENAME), file->GetFileName());
					inputbox.SetEditFilenameMode();
					inputbox.DoModal();
					CString newname = inputbox.GetInput();
					if (!inputbox.WasCancelled() && newname.GetLength()>0)
					{
						// at least prevent users from specifying something like "..\dir\file"
						static const TCHAR _szInvFileNameChars[] = _T("\\/:*?\"<>|");
						if (newname.FindOneOf(_szInvFileNameChars) != -1){
							AfxMessageBox(GetErrorMessage(ERROR_BAD_PATHNAME));
							break;
						}

						CString newpath;
						PathCombine(newpath.GetBuffer(MAX_PATH), file->GetPath(), newname);
						newpath.ReleaseBuffer();
						if (_trename(file->GetFilePath(), newpath) != 0){
							CString strError;
							strError.Format(GetResString(IDS_ERR_RENAMESF), file->GetFilePath(), newpath, _tcserror(errno));
							AfxMessageBox(strError);
							break;
						}
						
						if (file->IsKindOf(RUNTIME_CLASS(CPartFile)))
							file->SetFileName(newname);
						else
						{
							theApp.sharedfiles->RemoveKeywords(file);
							file->SetFileName(newname);
							theApp.sharedfiles->AddKeywords(file);
						}
						file->SetFilePath(newpath);
						UpdateFile(file);
					}
				}
				else if (wParam == MPG_F2)
					MessageBeep((UINT)-1);
				break;
			case MP_REMOVE:
			case MPG_DELETE:{
				if (IDNO == AfxMessageBox(GetResString(IDS_CONFIRM_FILEDELETE),MB_ICONWARNING | MB_ICONQUESTION | MB_DEFBUTTON2 | MB_YESNO))
					return TRUE;

				SetRedraw(FALSE);
				bool bRemovedItems = false;
				while (!selectedList.IsEmpty())
				{
					CKnownFile* myfile = selectedList.RemoveHead();
					if (!myfile || myfile->IsPartFile())
						continue;
					
					bool delsucc = false;
					if (!PathFileExists(myfile->GetFilePath()))
						delsucc = true;
					else{
						// Delete
						if (!thePrefs.GetRemoveToBin()){
							delsucc = DeleteFile(myfile->GetFilePath());
						}
						else{
							// delete to recycle bin :(
							TCHAR todel[MAX_PATH+1];
							MEMZERO(todel, sizeof todel);
							_tcsncpy(todel, myfile->GetFilePath(), ARRSIZE(todel)-2);

							SHFILEOPSTRUCT fp = {0};
							fp.wFunc = FO_DELETE;
							fp.hwnd = theApp.emuledlg->m_hWnd;
							fp.pFrom = todel;
							fp.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT;// | FOF_NOERRORUI
							delsucc = (SHFileOperation(&fp) == 0);
						}
					}
					if (delsucc){
						theApp.sharedfiles->RemoveFile(myfile);
						bRemovedItems = true;
						if (myfile->IsKindOf(RUNTIME_CLASS(CPartFile)))
							theApp.emuledlg->transferwnd->downloadlistctrl.ClearCompleted(static_cast<CPartFile*>(myfile));
					}
					else{
						CString strError;
						strError.Format(_T("Failed to delete file \"%s\"\r\n\r\n%s"), myfile->GetFilePath(), GetErrorMessage(GetLastError()));
						AfxMessageBox(strError);
					}
				}
				SetRedraw(TRUE);
				if (bRemovedItems)
					AutoSelectItem();
				break; 
			}
			case MP_CMT:
				ShowFileDialog(selectedList, IDD_COMMENT);
				break; 
            // [TPT] - SLUGFILLER: showComments
			case MP_VIEWFILECOMMENTS:
				if (file)
					ShowFileDialog(selectedList, IDD_COMMENTLST);
                		break; 
			// [TPT] - SLUGFILLER: showComments
			case MPG_ALTENTER:
			case MP_DETAIL:
				ShowFileDialog(selectedList);
				break;
			case MP_SHOWED2KLINK:
				ShowFileDialog(selectedList, IDD_ED2KLINK);
				break;
			case MP_PRIOVERYLOW:
			case MP_PRIOLOW:
			case MP_PRIONORMAL:
			case MP_PRIOHIGH:
			case MP_PRIOVERYHIGH:
			// [TPT] - WebCache
			case MP_PRIOWCRELEASE: //JP webcache release
			case MP_PRIOAUTO:
				{
					// [TPT] - WebCache
					//jp webcache release START 
					// check if a click on MP_PRIOWCRELEASE should activate WC-release
					bool activateWCRelease = false;
					POSITION pos2 = selectedList.GetHeadPosition();
					CKnownFile* cur_file = NULL;
					while (pos2 != NULL)
					{
						cur_file = selectedList.GetNext(pos2);
						if (!cur_file->ReleaseViaWebCache)
							activateWCRelease = true;
					}
					//jp webcache release END
					// [TPT] - WebCache

					POSITION pos = selectedList.GetHeadPosition();
					while (pos != NULL)
					{
						CKnownFile* file = selectedList.GetNext(pos);
						switch (wParam) {
							case MP_PRIOVERYLOW:
								file->SetAutoUpPriority(false);
								file->SetUpPriority(PR_VERYLOW);
								UpdateFile(file);
								break;
							case MP_PRIOLOW:
								file->SetAutoUpPriority(false);
								file->SetUpPriority(PR_LOW);
								UpdateFile(file);
								break;
							case MP_PRIONORMAL:
								file->SetAutoUpPriority(false);
								file->SetUpPriority(PR_NORMAL);
								UpdateFile(file);
								break;
							case MP_PRIOHIGH:
								file->SetAutoUpPriority(false);
								file->SetUpPriority(PR_HIGH);
								UpdateFile(file);
								break;
							case MP_PRIOVERYHIGH:
								file->SetAutoUpPriority(false);
								file->SetUpPriority(PR_VERYHIGH);
								UpdateFile(file);
								break;	
							case MP_PRIOAUTO:
								file->SetAutoUpPriority(true);
								file->UpdateAutoUpPriority();
								UpdateFile(file); 
								break;
							// [TPT] - WebCache
							//jp webcache release start
							case MP_PRIOWCRELEASE:
								if (!file->IsPartFile())
									file->SetReleaseViaWebCache(activateWCRelease);
								else
									file->SetReleaseViaWebCache(false);
								break;
							//jp webcache release end
							// [TPT] - WebCache
						}
					}
					break;
				}
				// [TPT] - Powershare 
			case MP_POWERSHARE_ON:
			case MP_POWERSHARE_OFF:
			case MP_POWERSHARE_DEFAULT:
			case MP_POWERSHARE_AUTO:
			case MP_POWERSHARE_LIMITED: //MORPH - Added by SiRoB, POWERSHARE Limit
				{
				SetRedraw(FALSE);
					POSITION pos = selectedList.GetHeadPosition();
					while (pos != NULL)
					{
						file = selectedList.GetNext(pos);
						switch (wParam) {
							case MP_POWERSHARE_DEFAULT:
								file->SetPowerShared(-1);
								break;
							case MP_POWERSHARE_ON:
								file->SetPowerShared(1);
								break;
							case MP_POWERSHARE_OFF:
								file->SetPowerShared(0);
								break;
							case MP_POWERSHARE_AUTO:
								file->SetPowerShared(2);
								break;
								//MORPH START - Added by SiRoB, POWERSHARE Limit
							case MP_POWERSHARE_LIMITED:
								file->SetPowerShared(3);
								break;
								//MORPH END   - Added by SiRoB, POWERSHARE Limit
						}
						UpdateFile(file);
					}
					SetRedraw(TRUE);
					break;
				}
			case MP_POWERSHARE_LIMIT:
			case MP_POWERSHARE_LIMIT_SET:
				{
					POSITION pos = selectedList.GetHeadPosition();
					int newPowerShareLimit = -1;
					if (wParam==MP_POWERSHARE_LIMIT_SET)
					{
						InputBox inputbox;
						CString title=GetResString(IDS_POWERSHARE);
						CString currPowerShareLimit;
						if (file)
							currPowerShareLimit.Format(_T("%i"), (file->GetPowerShareLimit()>=0)?file->GetPowerShareLimit():thePrefs.GetPowerShareLimit());
						else
							currPowerShareLimit = _T("0");
						inputbox.SetLabels(GetResString(IDS_POWERSHARE), GetResString(IDS_POWERSHARE_LIMIT), currPowerShareLimit);
						inputbox.SetNumber(true);
						int result = inputbox.DoModal();
						if (result == IDCANCEL || (newPowerShareLimit = inputbox.GetInputInt()) < 0)
							break;
					}
					SetRedraw(FALSE);
					while (pos != NULL)
					{
						file = selectedList.GetNext(pos);
						if  (newPowerShareLimit == file->GetPowerShareLimit()) break;
						file->SetPowerShareLimit(newPowerShareLimit);
					if (file->IsPartFile())
						((CPartFile*)file)->UpdatePartsInfo();
					else
						file->UpdatePartsInfo();
						UpdateFile(file);
					}
					SetRedraw(TRUE);
					break;
				}
			case MP_HIDEOS_DEFAULT:
			case MP_HIDEOS_SET:
				{
					POSITION pos = selectedList.GetHeadPosition();
					int newHideOS = -1;
					if (wParam==MP_HIDEOS_SET)
					{
						InputBox inputbox;
						CString title=GetResString(IDS_HIDEOS);
						CString currHideOS;
						if (file)
							currHideOS.Format(_T("%i"), (file->GetHideOS()>=0)?file->GetHideOS():thePrefs.GetHideOvershares());
						else
							currHideOS = _T("0");
						inputbox.SetLabels(GetResString(IDS_HIDEOS), GetResString(IDS_HIDEOVERSHARES), currHideOS);
						inputbox.SetNumber(true);
						int result = inputbox.DoModal();
						if (result == IDCANCEL || (newHideOS = inputbox.GetInputInt()) < 0)
							break;
					}
					SetRedraw(FALSE);
					while (pos != NULL)
					{
						file = selectedList.GetNext(pos);
						if  (newHideOS == file->GetHideOS()) break;
						file->SetHideOS(newHideOS);
						UpdateFile(file);
					}
					SetRedraw(TRUE);
					break;
				}
				// [TPT] - Powershare end
			// [TPT] - xMule_MOD: showSharePermissions
			// with itsonlyme's sorting fix
			case MP_PERMNONE:
			case MP_PERMFRIENDS:
			case MP_PERMALL: 
			{
				SetRedraw(FALSE);
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL)
				{
					CKnownFile* file = selectedList.GetNext(pos);
					switch (wParam)
					{
						case MP_PERMNONE:
							file->SetPermissions(PERM_NOONE);
							break;
						case MP_PERMFRIENDS:
							file->SetPermissions(PERM_FRIENDS);
							break;
						default : // case MP_PERMALL:
							file->SetPermissions(PERM_ALL);
							break;
					}
					UpdateFile(file);
				}
				SetRedraw(TRUE);
				Invalidate();
				break;
			}
			// [TPT] - xMule_MOD: showSharePermissions
			// [TPT] - itsonlyme: virtualDirs
			case MP_IOM_VIRTFILE:
			case MP_IOM_VIRTDIR: 
			case MP_IOM_VIRTSUBDIR:
				{
				InputBox input;
				CString title;
					file = selectedList.GetHead();
					switch (wParam) {
					case MP_IOM_VIRTFILE:
						title.Format(GetResString(IDS_VDS_CHANGEMAP), file->GetFileName());
							input.SetLabels(title,GetResString(IDS_VDS_VIRTUALFILE),selectedList.GetHead()->GetPath(true));
						break;
					case MP_IOM_VIRTDIR:
						title.Format(GetResString(IDS_VDS_CHANGEMAP), file->GetPath());
							input.SetLabels(title,GetResString(IDS_VDS_VIRTUALDIR),selectedList.GetHead()->GetPath(true));
						break;
					case MP_IOM_VIRTSUBDIR:
						title.Format(GetResString(IDS_VDS_CHANGEMAP), file->GetPath());
							input.SetLabels(title,GetResString(IDS_VDS_VIRTUALSUBDIR),selectedList.GetHead()->GetPath(true));
						break;
				}
				input.DoModal();
				CString output = input.GetInput();
					if (!input.WasCancelled() && output.GetLength()>0) {
						output.MakeLower();
						output.TrimRight(_T('\\'));
					POSITION pos = selectedList.GetHeadPosition();
					while (pos != NULL)
					{
							CKnownFile* file = selectedList.GetNext(pos);
							CString fileID;
							CString path = file->GetPath();
							path.MakeLower();
							path.TrimRight(_T('\\'));
						fileID.Format(_T("%i:%s"), file->GetFileSize(), EncodeBase16(file->GetFileHash(),16));
						if (wParam == MP_IOM_VIRTFILE)
							thePrefs.GetFileToVDirMap()->SetAt(fileID, output);
						else if (wParam == MP_IOM_VIRTDIR)
							thePrefs.GetDirToVDirMap()->SetAt(path, output);
						else if (wParam == MP_IOM_VIRTSUBDIR)
							thePrefs.GetSubDirToVDirMap()->SetAt(path, output);
					}
				}
					theApp.emuledlg->sharedfileswnd->Invalidate(false);
					theApp.emuledlg->sharedfileswnd->UpdateWindow();
					if (theApp.sharedfiles) 
						theApp.sharedfiles->ShowLocalFilesDialog(true);
				break;
			}
			case MP_IOM_VIRTREMOVE:
				{
					POSITION pos = selectedList.GetHeadPosition();
					while (pos != NULL)
					{
						CKnownFile* file = selectedList.GetNext(pos);
						CString virt, fileID;
						CString path = file->GetPath();
						path.MakeLower();
						path.TrimRight(_T('\\'));
						fileID.Format(_T("%i:%s"), file->GetFileSize(), EncodeBase16(file->GetFileHash(),16));
						if (thePrefs.GetFileToVDirMap()->Lookup(fileID, virt))
							thePrefs.GetFileToVDirMap()->RemoveKey(fileID);
						if (thePrefs.GetDirToVDirMap()->Lookup(path, virt))
							thePrefs.GetDirToVDirMap()->RemoveKey(path);
						if (thePrefs.GetSubDirToVDirMap()->Lookup(path, virt))
							thePrefs.GetSubDirToVDirMap()->RemoveKey(path);
				}
					theApp.emuledlg->sharedfileswnd->Invalidate(false);
					theApp.emuledlg->sharedfileswnd->UpdateWindow();
					if (theApp.sharedfiles) 
						theApp.sharedfiles->ShowLocalFilesDialog(true);
				break;
			}
			case MP_IOM_VIRTPREFS:
				theApp.emuledlg->ShowPreferences(IDD_PPG_VIRTUAL);
				break;
			// [TPT] - itsonlyme: virtualDirs
			default:
				// [TPT] - Powershare
				while (pos != NULL)
				{
					file = selectedList.GetNext(pos);
					if (wParam>=MP_WEBURL && wParam<=MP_WEBURL+256){
						theWebServices.RunURL(file, wParam);
					}
					else if (wParam>=MP_SELECTIVE_CHUNK && wParam<=MP_SELECTIVE_CHUNK_1){
						file->SetSelectiveChunk(wParam==MP_SELECTIVE_CHUNK?-1:wParam-MP_SELECTIVE_CHUNK_0);
						UpdateFile(file);
					}
				}
				break;
				// [TPT] - Powershare end
		}
	}
	return TRUE;
}

void CSharedFilesCtrl::OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// Barry - Store sort order in preferences
	// Determine ascending based on whether already sorted on this column
	int sortItem = thePrefs.GetColumnSortItem(CPreferences::tableShared);
	bool m_oldSortAscending = thePrefs.GetColumnSortAscending(CPreferences::tableShared);
	bool sortAscending = (sortItem != pNMListView->iSubItem) ? true : !m_oldSortAscending;

	// Item is column clicked
	sortItem = pNMListView->iSubItem;

	// Save new preferences
	thePrefs.SetColumnSortItem(CPreferences::tableShared, sortItem);
	thePrefs.SetColumnSortAscending(CPreferences::tableShared, sortAscending);

	// Ornis 4-way-sorting
	int adder=0;
	if (pNMListView->iSubItem>=5 && pNMListView->iSubItem<=7)
	{
		ASSERT( pNMListView->iSubItem - 5 < ARRSIZE(sortstat) );
		if (!sortAscending)
			sortstat[pNMListView->iSubItem - 5] = !sortstat[pNMListView->iSubItem - 5];
		adder = sortstat[pNMListView->iSubItem-5] ? 0 : 100;
	}
	else if (pNMListView->iSubItem==11)
	{
		ASSERT( 3 < ARRSIZE(sortstat) );
		if (!sortAscending)
			sortstat[3] = !sortstat[3];
		adder = sortstat[3] ? 0 : 100;
	}

	// Sort table
	if (adder==0)	
		SetSortArrow(sortItem, sortAscending); 
	else
		SetSortArrow(sortItem, sortAscending ? arrowDoubleUp : arrowDoubleDown);
	SortItems(SortProc, sortItem + adder + (sortAscending ? 0:20));

	*pResult = 0;
}

int CSharedFilesCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	const CKnownFile* item1 = (CKnownFile*)lParam1;
	const CKnownFile* item2 = (CKnownFile*)lParam2;	
	switch(lParamSort){
		case 0: //filename asc
			return CompareLocaleStringNoCase(item1->GetFileName(),item2->GetFileName());
		case 20: //filename desc
			return CompareLocaleStringNoCase(item2->GetFileName(),item1->GetFileName());

		case 1: //filesize asc
			return item1->GetFileSize()==item2->GetFileSize()?0:(item1->GetFileSize()>item2->GetFileSize()?1:-1);

		case 21: //filesize desc
			return item1->GetFileSize()==item2->GetFileSize()?0:(item2->GetFileSize()>item1->GetFileSize()?1:-1);


		case 2: //filetype asc
			return item1->GetFileTypeDisplayStr().Compare(item2->GetFileTypeDisplayStr());
		case 22: //filetype desc
			return item2->GetFileTypeDisplayStr().Compare(item1->GetFileTypeDisplayStr());

		case 3: //prio asc
			// [TPT] - Powershare
			if (item1->GetPowerShared() == false && item2->GetPowerShared() == true)
				return -1;			
			else if (item1->GetPowerShared() == true && item2->GetPowerShared() == false)
				return 1;
			else			
				if(item1->GetUpPriority() == PR_VERYLOW && item2->GetUpPriority() != PR_VERYLOW)
					return -1;
				else if (item1->GetUpPriority() != PR_VERYLOW && item2->GetUpPriority() == PR_VERYLOW)
					return 1;
				else
					return item1->GetUpPriority()-item2->GetUpPriority();
		case 23: //prio desc
			if (item2->GetPowerShared() == false && item1->GetPowerShared() == true)
				return -1;			
			else if (item2->GetPowerShared() == true && item1->GetPowerShared() == false)
				return 1;
			else		
				if(item2->GetUpPriority() == PR_VERYLOW && item1->GetUpPriority() != PR_VERYLOW )
					return -1;
				else if (item2->GetUpPriority() != PR_VERYLOW && item1->GetUpPriority() == PR_VERYLOW)
					return 1;
				else
					return item2->GetUpPriority()-item1->GetUpPriority();
				// [TPT] - Powershare end

		case 4: //fileID asc
			return memcmp(item1->GetFileHash(), item2->GetFileHash(), 16);
		case 24: //fileID desc
			return memcmp(item2->GetFileHash(), item1->GetFileHash(), 16);

		case 5: //requests asc
			return item1->statistic.GetRequests() - item2->statistic.GetRequests();
		case 25: //requests desc
			return item2->statistic.GetRequests() - item1->statistic.GetRequests();
		
		case 6: //acc requests asc
			return item1->statistic.GetAccepts() - item2->statistic.GetAccepts();
		case 26: //acc requests desc
			return item2->statistic.GetAccepts() - item1->statistic.GetAccepts();
		
		case 7: //all transferred asc
			return item1->statistic.GetTransferred()==item2->statistic.GetTransferred()?0:(item1->statistic.GetTransferred()>item2->statistic.GetTransferred()?1:-1);
		case 27: //all transferred desc
			return item1->statistic.GetTransferred()==item2->statistic.GetTransferred()?0:(item2->statistic.GetTransferred()>item1->statistic.GetTransferred()?1:-1);

		case 9: //folder asc
			return CompareLocaleStringNoCase(item1->GetPath(),item2->GetPath());
		case 29: //folder desc
			return CompareLocaleStringNoCase(item2->GetPath(),item1->GetPath());

		case 10: //complete sources asc
			return CompareUnsigned(item1->m_nCompleteSourcesCount, item2->m_nCompleteSourcesCount);
		case 30: //complete sources desc
			return CompareUnsigned(item2->m_nCompleteSourcesCount, item1->m_nCompleteSourcesCount);

		case 11: //ed2k shared asc
			return item1->GetPublishedED2K() - item2->GetPublishedED2K();
		case 31: //ed2k shared desc
			return item2->GetPublishedED2K() - item1->GetPublishedED2K();

		case 12: //permission asc
			return item2->GetPermissions()-item1->GetPermissions();
		case 32: //permission desc
			return item1->GetPermissions()-item2->GetPermissions();

			
		// [TPT] - itsonlyme: virtualDirs
		case 13:
			return item1->GetPath(true).CompareNoCase(item2->GetPath(true));
		case 33:
			return item2->GetPath(true).CompareNoCase(item1->GetPath(true));
		// [TPT] - itsonlyme: virtualDirs
		// [TPT] - Powershare
		case 14:
			if (item1->GetPowerShared() == false && item2->GetPowerShared() == true)
				return -1;
			else if (item1->GetPowerShared() == true && item2->GetPowerShared() == false)
				return 1;
			else
				if (item1->GetPowerSharedMode() != item2->GetPowerSharedMode())
					return item1->GetPowerSharedMode() - item2->GetPowerSharedMode();
				else
					if (item1->GetPowerShareAuthorized() == false && item2->GetPowerShareAuthorized() == true)
						return -1;
					else if (item1->GetPowerShareAuthorized() == true && item2->GetPowerShareAuthorized() == false)
						return 1;
					else
						if (item1->GetPowerShareAuto() == false && item2->GetPowerShareAuto() == true)
							return -1;
						else if (item1->GetPowerShareAuto() == true && item2->GetPowerShareAuto() == false)
							return 1;
						else
							//MORPH START - Added by SiRoB, POWERSHARE Limit
							if (item1->GetPowerShareLimited() == false && item2->GetPowerShareLimited() == true)
								return -1;
							else if (item1->GetPowerShareLimited() == true && item2->GetPowerShareLimited() == false)
								return 1;
							else
								//MORPH END   - Added by SiRoB, POWERSHARE Limit
								return 0;
		case 34:
			if (item2->GetPowerShared() == false && item1->GetPowerShared() == true)
				return -1;
			else if (item2->GetPowerShared() == true && item1->GetPowerShared() == false)
				return 1;
			else
				if (item2->GetPowerSharedMode() == 0 && item1->GetPowerSharedMode() != 0)
					return -1;
				else if (item2->GetPowerSharedMode() == 1 && item1->GetPowerSharedMode() != 1)
					return 1;
				else if (item2->GetPowerSharedMode() == 2 && item1->GetPowerSharedMode() != 2)
					return 1-item1->GetPowerSharedMode();
				else
					if (item2->GetPowerShareAuthorized() == false && item1->GetPowerShareAuthorized() == true)
						return -1;
					else if (item2->GetPowerShareAuthorized() == true && item1->GetPowerShareAuthorized() == false)
						return 1;
					else
						if (item2->GetPowerShareAuto() == false && item1->GetPowerShareAuto() == true)
							return -1;
						else if (item2->GetPowerShareAuto() == true && item1->GetPowerShareAuto() == false)
							return 1;
						else
							//MORPH START - Added by SiRoB, POWERSHARE Limit
							if (item2->GetPowerShareLimited() == false && item1->GetPowerShareLimited() == true)
								return -1;
							else if (item2->GetPowerShareLimited() == true && item1->GetPowerShareLimited() == false)
								return 1;
							else
								//MORPH END   - Added by SiRoB, POWERSHARE Limit
								return 0;		
		// [TPT] - Powershare End
		case 15:
			if (item1->GetHideOS() == item2->GetHideOS())
				return item1->GetSelectiveChunk() - item2->GetSelectiveChunk();
			else
				return item1->GetHideOS() - item2->GetHideOS();
		case 35:
			if (item2->GetHideOS() == item1->GetHideOS())
				return item2->GetSelectiveChunk() - item1->GetSelectiveChunk();
			else
				return item2->GetHideOS() - item1->GetHideOS();

		// [TPT] - SLUGFILLER: Spreadbars
		case 16: //spread asc
		case 17:
			return 10000*(item1->statistic.GetSpreadSortValue()-item2->statistic.GetSpreadSortValue());
		case 36: //spread desc
		case 37:
			return 10000*(item2->statistic.GetSpreadSortValue()-item1->statistic.GetSpreadSortValue());

		case 18: // VQB:  Simple UL asc
		case 38: //VQB:  Simple UL desc
			{
				float x1 = ((float)item1->statistic.GetAllTimeTransferred())/((float)item1->GetFileSize());
				float x2 = ((float)item2->statistic.GetAllTimeTransferred())/((float)item2->GetFileSize());
				if (lParamSort == 14) return 10000*(x1-x2); else return 10000*(x2-x1);
			}
		case 19: // SF:  Full Upload Count asc
			return 10000*(item1->statistic.GetFullSpreadCount()-item2->statistic.GetFullSpreadCount());
		case 39: // SF:  Full Upload Count desc
			return 10000*(item2->statistic.GetFullSpreadCount()-item1->statistic.GetFullSpreadCount());
		// [TPT] - SLUGFILLER: Spreadbars

		case 105: //all requests asc
			return CompareUnsigned(item1->statistic.GetAllTimeRequests(), item2->statistic.GetAllTimeRequests());
		case 125: //all requests desc
			return CompareUnsigned(item2->statistic.GetAllTimeRequests(), item1->statistic.GetAllTimeRequests());
		case 106: //all acc requests asc
			return CompareUnsigned(item1->statistic.GetAllTimeAccepts(), item2->statistic.GetAllTimeAccepts());
		case 126: //all acc requests desc
			return CompareUnsigned(item2->statistic.GetAllTimeAccepts(), item1->statistic.GetAllTimeAccepts());
		case 107: //all transferred asc
			return item1->statistic.GetAllTimeTransferred()==item2->statistic.GetAllTimeTransferred()?0:(item1->statistic.GetAllTimeTransferred()>item2->statistic.GetAllTimeTransferred()?1:-1);
		case 127: //all transferred desc
			return item1->statistic.GetAllTimeTransferred()==item2->statistic.GetAllTimeTransferred()?0:(item2->statistic.GetAllTimeTransferred()>item1->statistic.GetAllTimeTransferred()?1:-1);

		case 111:{ //kad shared asc
			uint32 tNow = time(NULL);
			int i1 = (tNow < item1->GetLastPublishTimeKadSrc()) ? 1 : 0;
			int i2 = (tNow < item2->GetLastPublishTimeKadSrc()) ? 1 : 0;
			return i1 - i2;
		}
		case 131:{ //kad shared desc
			uint32 tNow = time(NULL);
			int i1 = (tNow < item1->GetLastPublishTimeKadSrc()) ? 1 : 0;
			int i2 = (tNow < item2->GetLastPublishTimeKadSrc()) ? 1 : 0;
			return i2 - i1;
		}
		default: 
			return 0;
	}
}

void CSharedFilesCtrl::OpenFile(const CKnownFile* file)
{
	ShellOpenFile(file->GetFilePath(), NULL);
}

void CSharedFilesCtrl::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1)
	{
		CKnownFile* file = (CKnownFile*)GetItemData(iSel);
		if (file)
		{
			if (GetKeyState(VK_MENU) & 0x8000 || file->IsPartFile())
			{
				CTypedPtrList<CPtrList, CKnownFile*> aFiles;
				aFiles.AddHead(file);
				ShowFileDialog(aFiles);
			}
			else if (!file->IsPartFile())
				OpenFile(file);
		}
	}
	*pResult = 0;
}
// [TPT] - New Menu Styles
/*
void CSharedFilesCtrl::CreateMenues()
{
	if (m_PrioMenu) VERIFY( m_PrioMenu.DestroyMenu() );
	if (m_SharedFilesMenu) VERIFY( m_SharedFilesMenu.DestroyMenu() );

	m_PrioMenu.CreateMenu();
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOVERYLOW,GetResString(IDS_PRIOVERYLOW));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOLOW,GetResString(IDS_PRIOLOW));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIONORMAL,GetResString(IDS_PRIONORMAL));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOHIGH, GetResString(IDS_PRIOHIGH));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOVERYHIGH, GetResString(IDS_PRIORELEASE));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOAUTO, GetResString(IDS_PRIOAUTO));//UAP

	m_SharedFilesMenu.CreatePopupMenu();
	m_SharedFilesMenu.AddMenuTitle(GetResString(IDS_SHAREDFILES));

	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_OPEN, GetResString(IDS_OPENFILE));
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_OPENFOLDER, GetResString(IDS_OPENFOLDER));
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_RENAME, GetResString(IDS_RENAME) + _T("..."));
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_REMOVE, GetResString(IDS_DELETE));

	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR);
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_PrioMenu.m_hMenu, GetResString(IDS_PRIORITY) + _T(" (") + GetResString(IDS_PW_CON_UPLBL) + _T(")"));
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR);
	
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_DETAIL, GetResString(IDS_SHOWDETAILS));
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_SHOWED2KLINK, GetResString(IDS_DL_SHOWED2KLINK) );
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_CMT, GetResString(IDS_CMT_ADD)); 
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR); 

#if defined(_DEBUG)
	//JOHNTODO: Not for release as we need kad lowID users in the network to see how well this work work. Also, we do not support these links yet.
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_GETKADSOURCELINK, _T("Copy eD2K Links To Clipboard (Kad)"));
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR); 
#endif

	
	m_SharedFilesMenu.AppendMenu(MF_STRING,Irc_SetSendLink,GetResString(IDS_IRC_ADDLINKTOIRC));
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR); 
}*/

void CSharedFilesCtrl::ShowComments(CKnownFile* file)
{
	if (file)
		{
		CTypedPtrList<CPtrList, CKnownFile*> aFiles;
		aFiles.AddHead(file);
		ShowFileDialog(aFiles, IDD_COMMENT);
		}
	}

void CSharedFilesCtrl::OnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);

	if (theApp.emuledlg->IsRunning()){
		// Although we have an owner drawn listview control we store the text for the primary item in the listview, to be
		// capable of quick searching those items via the keyboard. Because our listview items may change their contents,
		// we do this via a text callback function. The listview control will send us the LVN_DISPINFO notification if
		// it needs to know the contents of the primary item.
		//
		// But, the listview control sends this notification all the time, even if we do not search for an item. At least
		// this notification is only sent for the visible items and not for all items in the list. Though, because this
		// function is invoked *very* often, no *NOT* put any time consuming code here in.

		if (pDispInfo->item.mask & LVIF_TEXT){
			const CKnownFile* pFile = reinterpret_cast<CKnownFile*>(pDispInfo->item.lParam);
			if (pFile != NULL){
				switch (pDispInfo->item.iSubItem){
					case 0:
						if (pDispInfo->item.cchTextMax > 0){
							_tcsncpy(pDispInfo->item.pszText, pFile->GetFileName(), pDispInfo->item.cchTextMax);
							pDispInfo->item.pszText[pDispInfo->item.cchTextMax-1] = _T('\0');
						}
						break;
					default:
						// shouldn't happen
						pDispInfo->item.pszText[0] = _T('\0');
						break;
				}
			}
		}
	}
	*pResult = 0;
}

void CSharedFilesCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == 'C' && (GetKeyState(VK_CONTROL) & 0x8000))
	{
		// Ctrl+C: Copy listview items to clipboard
		SendMessage(WM_COMMAND, MP_GETED2KLINK);
		return;
	}
	else if (nChar == VK_F5)
		ReloadFileList();

	CMuleListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CSharedFilesCtrl::ShowFileDialog(CTypedPtrList<CPtrList, CKnownFile*>& aFiles, UINT uPshInvokePage)
{
	if (aFiles.GetSize() > 0)
	{
		aFiles.GetHead()->GetDetailsSheetInterface()->OpenDetailsSheet(aFiles, uPshInvokePage, this);	// SLUGFILLER: modelessDialogs
	}
}
//[TPT] - Double buffer style in lists
//TODO: I have done in this way because in future could be an option
void CSharedFilesCtrl::SetDoubleBufferStyle()
{
	if((_AfxGetComCtlVersion() >= MAKELONG(0, 6)) && thePrefs.GetDoubleBufferStyle())	
		SetExtendedStyle(GetExtendedStyle() | 0x00010000 /*LVS_EX_DOUBLEBUFFER*/);
	else
		if((GetExtendedStyle() & 0x00010000 /*LVS_EX_DOUBLEBUFFER*/) != 0)
			SetExtendedStyle(GetExtendedStyle() ^ 0x00010000);//XOR: delete the style if present
}
