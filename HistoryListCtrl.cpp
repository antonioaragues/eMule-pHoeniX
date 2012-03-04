//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
//
// HistoryListCtrl. emulEspaña Mod: Added by MoNKi
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

// HistoryListCtrl.cpp: archivo de implementación
//

#include "stdafx.h"
#include "emule.h"
#include "HistoryListCtrl.h"
#include "CommentDialog.h"
#include "FileInfoDialog.h"
#include "MetaDataDlg.h"
#include "ResizableLib/ResizableSheet.h"
#include "Preferences.h"
#include "KnownFileList.h"
#include "EmuleDlg.h"
#include "memdc.h"
#include "menucmds.h"
#include "IrcWnd.h"
#include "MenuXP.h"// [TPT] - New Menu Styles
#include "WebServices.h"
#include "mod_version.h" // [TPT]
#include "SharedFileList.h"
#include "log.h"
#include "ListViewWalkerPropertySheet.h"
#include "HighColorTab.hpp"
#include "UserMsgs.h"
#include "ED2kLinkDlg.h"
#include "SharedFilesWnd.h"
#include "Modeless.h"
#include "KnownFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////////
// CHistoryFileDetailsSheet

class CHistoryFileDetailsSheet : public CModelessPropertySheet
{
	DECLARE_DYNAMIC(CHistoryFileDetailsSheet)

public:
	CHistoryFileDetailsSheet(CTypedPtrList<CPtrList, CKnownFile*>& aFiles, UINT uPshInvokePage = 0, CListCtrlItemWalk* pListCtrl = NULL);
	virtual ~CHistoryFileDetailsSheet();

protected:
	CFileInfoDialog m_wndMediaInfo;
	CMetaDataDlg m_wndMetaData;
	CED2kLinkDlg		m_wndFileLink;
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

LPCTSTR CHistoryFileDetailsSheet::m_pPshStartPage;

IMPLEMENT_DYNAMIC(CHistoryFileDetailsSheet, CModelessPropertySheet)

BEGIN_MESSAGE_MAP(CHistoryFileDetailsSheet, CModelessPropertySheet)
	ON_WM_DESTROY()
	ON_MESSAGE(UM_DATA_CHANGED, OnDataChanged)
END_MESSAGE_MAP()

CHistoryFileDetailsSheet::CHistoryFileDetailsSheet(CTypedPtrList<CPtrList, CKnownFile*>& aFiles, UINT uPshInvokePage, CListCtrlItemWalk* pListCtrl)
	: CModelessPropertySheet(pListCtrl)
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

CHistoryFileDetailsSheet::~CHistoryFileDetailsSheet()
{
}

void CHistoryFileDetailsSheet::OnDestroy()
{
	if (m_uPshInvokePage == 0)
		m_pPshStartPage = GetPage(GetActiveIndex())->m_psp.pszTemplate;
	CListViewWalkerPropertySheet::OnDestroy();
}

BOOL CHistoryFileDetailsSheet::OnInitDialog()
{		
	EnableStackedTabs(FALSE);
	BOOL bResult = CModelessPropertySheet::OnInitDialog();
	HighColorTab::UpdateImageList(*this);
	InitWindowStyles(this);
	EnableSaveRestore(_T("HistoryFileDetailsSheet")); // call this after(!) OnInitDialog
	UpdateTitle();
	return bResult;
}

LRESULT CHistoryFileDetailsSheet::OnDataChanged(WPARAM, LPARAM)
{
	UpdateTitle();
	return 1;
}

void CHistoryFileDetailsSheet::UpdateTitle()
{
	if (m_aItems.GetSize() == 1)
		SetWindowText(GetResString(IDS_DETAILS) + _T(": ") + STATIC_DOWNCAST(CKnownFile, m_aItems[0])->GetFileName());
	else
		SetWindowText(GetResString(IDS_DETAILS));
}

BOOL CHistoryFileDetailsSheet::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_APPLY_NOW)
		{
		CHistoryListCtrl* pHistoryListCtrl = DYNAMIC_DOWNCAST(CHistoryListCtrl, m_pListCtrl->GetListCtrl());
		if (pHistoryListCtrl)
			{
			for (int i = 0; i < m_aItems.GetSize(); i++) {
				// so, and why does this not(!) work while the sheet is open ??
				pHistoryListCtrl->UpdateFile(DYNAMIC_DOWNCAST(CKnownFile, m_aItems[i]));
			}
		}
	}

	return CListViewWalkerPropertySheet::OnCommand(wParam, lParam);
}

// SLUGFILLER: modelessDialogs
//////////////////////////////////////////////////////////////////////////////
// CHistoryListDetailsSheetInterface

CHistoryListDetailsSheetInterface::CHistoryListDetailsSheetInterface(CKnownFile* owner)
	: CModelessPropertySheetInterface(STATIC_DOWNCAST(CObject, owner))
{
}

void CHistoryListDetailsSheetInterface::OpenDetailsSheet(CTypedPtrList<CPtrList, CKnownFile*>& aFiles, UINT uPshInvokePage, CListCtrlItemWalk* pListCtrl)
{
	CSimpleArray<CModelessPropertySheetInterface*> aInterfaces;
	POSITION pos = aFiles.GetHeadPosition();
	while (pos)
		aInterfaces.Add(STATIC_DOWNCAST(CModelessPropertySheetInterface, aFiles.GetNext(pos)->GetDetailsSheetInterface()));
	OpenPropertySheet(&aInterfaces, &aFiles, uPshInvokePage, pListCtrl);
}

CModelessPropertySheet* CHistoryListDetailsSheetInterface::CreatePropertySheet(va_list args)
{
	CTypedPtrList<CPtrList, CKnownFile*>* aFiles = (CTypedPtrList<CPtrList, CKnownFile*>*)va_arg(args, LPVOID);
	UINT uPshInvokePage = va_arg(args, UINT);
	CListCtrlItemWalk* pListCtrl = va_arg(args, CListCtrlItemWalk*);
	return STATIC_DOWNCAST(CModelessPropertySheet, new CHistoryFileDetailsSheet(*aFiles, uPshInvokePage, pListCtrl));
}
// SLUGFILLER: modelessDialogs


//////////////////////////////
// CHistoryListCtrl



BEGIN_MESSAGE_MAP(CHistoryListCtrl, CMuleListCtrl)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_WM_CONTEXTMENU()
	ON_WM_MEASUREITEM()// [TPT] - New Menu Styles
END_MESSAGE_MAP()

#define MLC_DT_TEXT (DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER | DT_END_ELLIPSIS)


IMPLEMENT_DYNAMIC(CHistoryListCtrl, CListCtrl)
CHistoryListCtrl::CHistoryListCtrl()
	: CListCtrlItemWalk(this)
{
}

CHistoryListCtrl::~CHistoryListCtrl()
{
}

void CHistoryListCtrl::Init(void)
{
	CImageList ilDummyImageList; //dummy list for getting the proper height of listview entries
	ilDummyImageList.Create(1, theApp.GetSmallSytemIconSize().cy,theApp.m_iDfltImageListColorFlags|ILC_MASK, 1, 1); 
	SetImageList(&ilDummyImageList, LVSIL_SMALL);
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) == 0 );
	ilDummyImageList.Detach();

	SetExtendedStyle(LVS_EX_FULLROWSELECT);
	SetDoubleBufferStyle();
	ModifyStyle(LVS_SINGLESEL,0);

	InsertColumn(0,GetResString(IDS_DL_FILENAME),LVCFMT_LEFT, 260);
	InsertColumn(1,GetResString(IDS_DL_SIZE),LVCFMT_RIGHT,70);
	InsertColumn(2,GetResString(IDS_TYPE),LVCFMT_LEFT,100);
	InsertColumn(3,GetResString(IDS_FILEID),LVCFMT_LEFT, 220);
	InsertColumn(4,GetResString(IDS_DATE),LVCFMT_LEFT, 120);
	InsertColumn(5,GetResString(IDS_DOWNHISTORY_SHARED),LVCFMT_LEFT, 65);
	InsertColumn(6,GetResString(IDS_COMMENT),LVCFMT_LEFT, 260);

	Reload();

	LoadSettings(CPreferences::tableHistory);

	// Barry - Use preferred sort order from preferences
	int sortItem = thePrefs.GetColumnSortItem(CPreferences::tableHistory);
	bool sortAscending = thePrefs.GetColumnSortAscending(CPreferences::tableHistory);
	SetSortArrow(sortItem, sortAscending);
	// [TPT] - SLUGFILLER: multiSort - load multiple params
	for (int i = thePrefs.GetColumnSortCount(CPreferences::tableHistory); i > 0; ) {
		i--;
		sortItem = thePrefs.GetColumnSortItem(CPreferences::tableHistory, i);
		sortAscending = thePrefs.GetColumnSortAscending(CPreferences::tableHistory, i);
		SortItems(SortProc, sortItem + (sortAscending ? 0:20));
	}
	// [TPT] - SLUGFILLER: multiSort
}

void CHistoryListCtrl::AddFile(CKnownFile* toadd){
	uint32 itemnr = GetItemCount();

	LVFINDINFO info;
	info.flags = LVFI_PARAM;
	info.lParam = (LPARAM)toadd;
	int nItem = FindItem(&info);
	if(nItem == -1){
		if(!theApp.sharedfiles->IsFilePtrInList(toadd) || (theApp.sharedfiles->IsFilePtrInList(toadd) && thePrefs.GetShowSharedInHistory()))
			InsertItem(LVIF_PARAM|LVIF_TEXT,itemnr,toadd->GetFileName(),0,0,0,(LPARAM)toadd);
	}
}

void CHistoryListCtrl::Reload(void)
{
	CKnownFile * cur_file;

	SetRedraw(false);

	DeleteAllItems();
	
	if(thePrefs.GetShowSharedInHistory()){
		POSITION pos = theApp.knownfiles->GetKnownFiles().GetStartPosition();					
		while(pos){
			CCKey key;
			theApp.knownfiles->GetKnownFiles().GetNextAssoc( pos, key, cur_file );
			InsertItem(LVIF_PARAM|LVIF_TEXT,GetItemCount(),cur_file->GetFileName(),0,0,0,(LPARAM)cur_file);
		}		
	}
	else{
		CKnownFilesMap *files = NULL;
		files=theApp.knownfiles->GetDownloadedFiles();
		POSITION pos = files->GetStartPosition();					
		while(pos){
			CCKey key;
			files->GetNextAssoc( pos, key, cur_file );
			InsertItem(LVIF_PARAM|LVIF_TEXT,GetItemCount(),cur_file->GetFileName(),0,0,0,(LPARAM)cur_file);
	}
		delete files;
	}

	SetRedraw(true);
}

void CHistoryListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
	if( !theApp.emuledlg->IsRunning() )
		return;
	if( !lpDrawItemStruct->itemData)
		return;

	//set up our ficker free drawing
	CRect rcItem(lpDrawItemStruct->rcItem);
	CDC *oDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	COLORREF crOldDCBkColor = oDC->SetBkColor(m_crWindow);
	CMemDC pDC(oDC, &rcItem);	
	CFont *pOldFont = pDC->SelectObject(GetFont());
	COLORREF crOldTextColor;
	CKnownFile* file = (CKnownFile*)lpDrawItemStruct->itemData;
	
	if(m_bCustomDraw)
		crOldTextColor = pDC->SetTextColor(m_lvcd.clrText);
	else
		crOldTextColor = pDC->SetTextColor(m_crWindowText);

	int iOffset = pDC->GetTextExtent(_T(" "), 1 ).cx*2;
	int iItem = lpDrawItemStruct->itemID;
	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();

	//gets the item image and state info
	LV_ITEM lvi;
	lvi.mask = LVIF_IMAGE | LVIF_STATE;
	lvi.iItem = iItem;
	lvi.iSubItem = 0;
	lvi.stateMask = LVIS_DROPHILITED | LVIS_FOCUSED | LVIS_SELECTED;
	GetItem(&lvi);

	//see if the item be highlighted
	BOOL bHighlight = ((lvi.state & LVIS_DROPHILITED) || (lvi.state & LVIS_SELECTED));
	BOOL bCtrlFocused = ((GetFocus() == this) || (GetStyle() & LVS_SHOWSELALWAYS));

	//get rectangles for drawing
	CRect rcBounds, rcLabel, rcIcon;
	GetItemRect(iItem, rcBounds, LVIR_BOUNDS);
	GetItemRect(iItem, rcLabel, LVIR_LABEL);
	GetItemRect(iItem, rcIcon, LVIR_ICON);
	CRect rcCol(rcBounds);

	//the label!
	CString sLabel = GetItemText(iItem, 0);
	//labels are offset by a certain amount
	//this offset is related to the width of a space character
	CRect rcHighlight;
	CRect rcWnd;

	//should I check (GetExtendedStyle() & LVS_EX_FULLROWSELECT) ?
	rcHighlight.top    = rcBounds.top;
	rcHighlight.bottom = rcBounds.bottom;
	rcHighlight.left   = rcBounds.left  + 1;
	rcHighlight.right  = rcBounds.right - 1;

	COLORREF crOldBckColor;
	//draw the background color
	if(bHighlight) 
	{
		if(bCtrlFocused) 
		{
			pDC->FillRect(rcHighlight, &CBrush(m_crHighlight));
			crOldBckColor = pDC->SetBkColor(m_crHighlight);
		} 
		else 
		{
			pDC->FillRect(rcHighlight, &CBrush(m_crNoHighlight));
			crOldBckColor = pDC->SetBkColor(m_crNoHighlight);
		}
	} 
	else 
	{
		pDC->FillRect(rcHighlight, &CBrush(m_crWindow));
		crOldBckColor = pDC->SetBkColor(GetBkColor());
	}

	//update column
	rcCol.right = rcCol.left + GetColumnWidth(0);

	//draw the item's icon
	int iImage = theApp.GetFileTypeSystemImageIdx( file->GetFileName() );
	if (theApp.GetSystemImageList() != NULL)
		::ImageList_Draw(theApp.GetSystemImageList(), iImage, pDC, rcCol.left + 4, rcCol.top, ILD_NORMAL|ILD_TRANSPARENT);

	//draw item label (column 0)
	sLabel = file->GetFileName();
	rcLabel.left += 16;
	rcLabel.left += iOffset / 2;
	rcLabel.right -= iOffset;
	pDC->DrawText(sLabel, -1, rcLabel, MLC_DT_TEXT | DT_LEFT | DT_NOCLIP);

	//draw labels for remaining columns
	LV_COLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH;
	rcBounds.right = rcHighlight.right > rcBounds.right ? rcHighlight.right : rcBounds.right;

	int iCount = pHeaderCtrl->GetItemCount();
	for(int iCurrent = 0; iCurrent < iCount; iCurrent++) 
	{
		int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);

		if(iColumn == 0)
			continue;

		GetColumn(iColumn, &lvc);
		//don't draw anything with 0 width
		if(lvc.cx == 0)
			continue;

		rcCol.left = rcCol.right;
		rcCol.right += lvc.cx;

		switch(iColumn){
			case 1:
				sLabel = CastItoXBytes(file->GetFileSize());
				break;
			case 2:
				sLabel = file->GetFileTypeDisplayStr();
				break;
			case 3:
				sLabel = EncodeBase16(file->GetFileHash(),16);
				break;
			case 4:
				sLabel = file->GetUtcCFileDate().Format("%x %X");
				break;
			case 5:
				if (theApp.sharedfiles->IsFilePtrInList(file))
					sLabel=GetResString(IDS_YES);
				else
					sLabel=GetResString(IDS_NO);
				break;
			case 6:
				sLabel = file->GetFileComment();
				break;
		}
		if (sLabel.GetLength() == 0)
			continue;

		//get the text justification
		UINT nJustify = DT_LEFT;
		switch(lvc.fmt & LVCFMT_JUSTIFYMASK) 
		{
			case LVCFMT_RIGHT:
				nJustify = DT_RIGHT;
				break;
			case LVCFMT_CENTER:
				nJustify = DT_CENTER;
				break;
			default:
				break;
		}

		rcLabel = rcCol;
		rcLabel.left += iOffset;
		rcLabel.right -= iOffset;

		pDC->DrawText(sLabel, -1, rcLabel, MLC_DT_TEXT | nJustify);
	}

	//draw focus rectangle if item has focus
	if((lvi.state & LVIS_FOCUSED) && (bCtrlFocused || (lvi.state & LVIS_SELECTED))) 
	{
		if(!bCtrlFocused || !(lvi.state & LVIS_SELECTED))
			pDC->FrameRect(rcHighlight, &CBrush(m_crNoFocusLine));
		else
			pDC->FrameRect(rcHighlight, &CBrush(m_crFocusLine));
	}

	pDC->Flush();
	//restore old font
	pDC->SelectObject(pOldFont);
	pDC->SetTextColor(crOldTextColor);
	pDC->SetBkColor(crOldBckColor);
	oDC->SetBkColor(crOldDCBkColor);
}

void CHistoryListCtrl::OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult){
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// Barry - Store sort order in preferences
	// Determine ascending based on whether already sorted on this column
	int sortItem = thePrefs.GetColumnSortItem(CPreferences::tableHistory);
	bool m_oldSortAscending = thePrefs.GetColumnSortAscending(CPreferences::tableHistory);
	bool sortAscending = (sortItem != pNMListView->iSubItem) ? true : !m_oldSortAscending;

	// Item is column clicked
	sortItem = pNMListView->iSubItem;

	// Save new preferences
	thePrefs.SetColumnSortItem(CPreferences::tableHistory, sortItem);
	thePrefs.SetColumnSortAscending(CPreferences::tableHistory, sortAscending);

	// Sort table
	SetSortArrow(sortItem, sortAscending);
	SortItems(SortProc, sortItem + (sortAscending ? 0:20));

	*pResult = 0;
}

int CHistoryListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort){
	CKnownFile* item1 = (CKnownFile*)lParam1;
	CKnownFile* item2 = (CKnownFile*)lParam2;	
	switch(lParamSort){
		case 0: //filename asc
			return _tcsicmp(item1->GetFileName(),item2->GetFileName());
		case 20: //filename desc
			return _tcsicmp(item2->GetFileName(),item1->GetFileName());

		case 1: //filesize asc
			return item1->GetFileSize()==item2->GetFileSize()?0:(item1->GetFileSize()>item2->GetFileSize()?1:-1);

		case 21: //filesize desc
			return item1->GetFileSize()==item2->GetFileSize()?0:(item2->GetFileSize()>item1->GetFileSize()?1:-1);

		case 2: //filetype asc
			return item1->GetFileType().CompareNoCase(item2->GetFileType());
		case 22: //filetype desc
			return item2->GetFileType().CompareNoCase(item1->GetFileType());

		case 3: //file ID
			return memcmp(item1->GetFileHash(),item2->GetFileHash(),16);
		case 23:
			return memcmp(item2->GetFileHash(),item1->GetFileHash(),16);

		case 4: //date
			return CompareUnsigned(item1->GetUtcFileDate(),item2->GetUtcFileDate());
		case 24:
			return CompareUnsigned(item2->GetUtcFileDate(),item1->GetUtcFileDate());

		case 5: //Shared?
			{
				bool shared1, shared2;
				shared1 = theApp.sharedfiles->IsFilePtrInList(item1);
				shared2 = theApp.sharedfiles->IsFilePtrInList(item2);
				return shared1==shared2 ? 0 : (shared1 && !shared2 ? 1 : -1);
			}
		case 25:
			{
				bool shared1, shared2;
				shared1 = theApp.sharedfiles->IsFilePtrInList(item1);
				shared2 = theApp.sharedfiles->IsFilePtrInList(item2);
				return shared1==shared2 ? 0 : (shared2 && !shared1 ? 1 : -1);
			}
		case 6: //comment
			return _tcsicmp(item1->GetFileComment(),item2->GetFileComment());
		case 26:
			return _tcsicmp(item2->GetFileComment(),item1->GetFileComment());

		default: 
			return 0;
	}
}

void CHistoryListCtrl::Localize() {
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
	CString strRes;

	for(int i=0; i<=6;i++){
		switch(i){
			case 0:
				strRes = GetResString(IDS_DL_FILENAME);
				break;
			case 1:
				strRes = GetResString(IDS_DL_SIZE);
				break;
			case 2:
				strRes = GetResString(IDS_TYPE);
				break;
			case 3:
				strRes = GetResString(IDS_FILEID);
				break;
			case 4:
				strRes = GetResString(IDS_DATE);
				break;
			case 5:
				strRes = GetResString(IDS_DOWNHISTORY_SHARED);
				break;
			case 6:
				strRes = GetResString(IDS_COMMENT);
				break;
			default:
				strRes = "No Text!!";
		}

		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(i, &hdi);
		strRes.ReleaseBuffer();
	}

	//CreateMenues(); // [TPT] - New Menu Styles
}


// [TPT] - New Menu Styles BEGIN
void CHistoryListCtrl::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CKnownFile* file = NULL;

	int iSelectedItems = GetSelectedCount();
	if (GetSelectionMark()!=-1) 
		file=(CKnownFile*)GetItemData(GetSelectionMark());


	//Menu Configuration
	CMenuXP	*pMenu = new CMenuXP;
	pMenu->CreatePopupMenu();
	pMenu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
	pMenu->AddSideBar(new CMenuXPSideBar(17, MOD_VERSION));
	pMenu->SetSideBarStartColor(RGB(255,0,0));
	pMenu->SetSideBarEndColor(RGB(255,128,0));
	pMenu->SetSelectedBarColor(RGB(242,120,114));


	//SubMenu Acciones
	CMenuXP	*pActionMenu = new CMenuXP;
	pActionMenu->CreatePopupMenu();
	pActionMenu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
	pActionMenu->SetSelectedBarColor(RGB(242,120,114));
	pActionMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_CLEARHISTORY, GetResString(IDS_DOWNHISTORY_CLEAR)));
	//pActionMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_MERGEHISTORY, GetResString(IDS_DOWNHISTORY_MERGE))); // [TPT] - Due to SLUGFILLER: mergeKnown

	//SubMenu Servicios Web
	CMenuXP *pWebMenu = new CMenuXP;
	pWebMenu->CreatePopupMenu();
	pWebMenu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
	pWebMenu->SetSelectedBarColor(RGB(242,120,114));
	
	pMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_OPEN, GetResString(IDS_OPENFILE)));
	pMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_CMT, GetResString(IDS_CMT_ADD)));
	pMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_DETAIL,GetResString(IDS_SHOWDETAILS), theApp.LoadIcon(_T("information"), 16, 16)));
	pMenu->AppendSeparator();

	if (thePrefs.GetShowCopyEd2kLinkCmd())
	{
		pMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_GETED2KLINK,GetResString(IDS_DL_LINK1), theApp.LoadIcon(_T("copy"), 16, 16)));
		pMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_GETHTMLED2KLINK, GetResString(IDS_DL_LINK2)));
		pMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_GETBBCODEED2KLINK, GetResString(IDS_CREATEBBCODELINK)));
	}
	else
		pMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_SHOWED2KLINK,GetResString(IDS_DL_SHOWED2KLINK), theApp.LoadIcon(_T("copy"), 16, 16)));

		
	pMenu->AppendSeparator();
	pMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_REMOVESELECTED,GetResString(IDS_DOWNHISTORY_REMOVE), theApp.LoadIcon(_T("delete"), 16, 16)));
	pMenu->AppendSeparator();


	//This menu option is is for testing..
	if(theApp.emuledlg->ircwnd->IsConnected()){
		pMenu->AppendODMenu(MF_STRING | (iSelectedItems == 1 && theApp.emuledlg->ircwnd->IsConnected()) ? MF_ENABLED : MF_GRAYED, new CMenuXPText(Irc_SetSendLink, GetResString(IDS_IRC_ADDLINKTOIRC)));
		pMenu->AppendSeparator();
	}

	pMenu->AppendODPopup(MF_STRING | (iSelectedItems>0)? MF_ENABLED : MF_GRAYED, pActionMenu,new CMenuXPText(0,GetResString(IDS_DOWNHISTORY_ACTIONS), theApp.LoadIcon(_T("misc"), 16, 16)));

	int iWebMenuEntries = theWebServices.GetAllMenuEntries(pWebMenu);
	UINT flag2 = (iWebMenuEntries == 0 ) ? MF_GRAYED : MF_STRING;
	pMenu->AppendODPopup(MF_STRING | flag2 | MF_POPUP, pWebMenu, new CMenuXPText(0,GetResString(IDS_WEBSERVICES), theApp.LoadIcon(_T("webServices"), 16, 16)));

	if(file && theApp.sharedfiles->IsFilePtrInList(file)){
		pMenu->EnableMenuItem(MP_OPEN, MF_ENABLED);
		pMenu->EnableMenuItem(MP_REMOVESELECTED, MF_GRAYED);
	}
	else {
		pMenu->EnableMenuItem(MP_OPEN, MF_GRAYED);
		if(file && iSelectedItems>0){
			pMenu->EnableMenuItem(MP_REMOVESELECTED, MF_ENABLED);
			pMenu->EnableMenuItem(MP_DETAIL, MF_ENABLED);
		}
		else{
			pMenu->EnableMenuItem(MP_REMOVESELECTED, MF_GRAYED);
			pMenu->EnableMenuItem(MP_DETAIL, MF_GRAYED);}
	}

	if(thePrefs.GetShowSharedInHistory())
			pMenu->EnableMenuItem(MP_VIEWSHAREDFILES, MF_CHECKED);
	else
			pMenu->EnableMenuItem(MP_VIEWSHAREDFILES, MF_UNCHECKED);

	pMenu->TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);

	delete pWebMenu;
	delete pActionMenu;

	delete pMenu;

}
// [TPT] - New Menu Styles END


// [TPT] - New Menu Styles BEGIN
void CHistoryListCtrl::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	HMENU hMenu = AfxGetThreadState()->m_hTrackingMenu;
	CMenu	*pMenu = CMenu::FromHandle(hMenu);
	pMenu->MeasureItem(lpMeasureItemStruct);

	CWnd::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}
// [TPT] - New Menu Styles END

void CHistoryListCtrl::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1)
	{
		CKnownFile* file = (CKnownFile*)GetItemData(iSel);
		if (file)
		{
			if (GetKeyState(VK_MENU) & 0x8000)
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

BOOL CHistoryListCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	UINT selectedCount = this->GetSelectedCount(); 
	int iSel = GetSelectionMark();

	CTypedPtrList<CPtrList, CKnownFile*> selectedList;
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos != NULL){
		int index = GetNextSelectedItem(pos);
		if (index >= 0)
			selectedList.AddTail((CKnownFile*)GetItemData(index));
	}

	if (selectedCount>0){
		CKnownFile* file = (CKnownFile*)GetItemData(iSel);
		if (wParam>=MP_WEBURL && wParam<=MP_WEBURL+256) {
			theWebServices.RunURL(file, wParam);
		}

		switch (wParam){
			case Irc_SetSendLink:
			{
				theApp.emuledlg->ircwnd->SetSendFileString(CreateED2kLink(file));
				break;
			}
			case MP_GETED2KLINK:{
				if(selectedCount > 1)
				{
					CString str;
					POSITION pos = this->GetFirstSelectedItemPosition();
					while( pos != NULL )
					{
						file = (CKnownFile*)this->GetItemData(GetNextSelectedItem(pos));
						str.Append(CreateED2kLink(file) + _T("\n"));
					}

					theApp.CopyTextToClipboard(str);					
					break; 
				}
				theApp.CopyTextToClipboard(CreateED2kLink(file));
				break;
			}
			case MP_GETHTMLED2KLINK:
				if(selectedCount > 1)
				{
					CString str;
					POSITION pos = this->GetFirstSelectedItemPosition();
					while( pos != NULL )
					{
						file = (CKnownFile*)this->GetItemData(GetNextSelectedItem(pos));
						str += CreateHTMLED2kLink(file) + _T("\n"); 
					}
					theApp.CopyTextToClipboard(str);					
					break; 
				} 
				theApp.CopyTextToClipboard(CreateHTMLED2kLink(file));
				break;
			// [TPT] - Announ: -Copy BBCode ed2k links-
			case MP_GETBBCODEED2KLINK:
			{
				if(selectedCount > 1)
				{
					CString str;
					POSITION pos = this->GetFirstSelectedItemPosition();
					while( pos != NULL )
					{
						file = (CKnownFile*)this->GetItemData(GetNextSelectedItem(pos));
						str += theApp.CreateBBCodeED2kLink(file) + _T("\n"); 
					}
					theApp.CopyTextToClipboard(str);					
					break; 
				} 
				CString strLink = theApp.CreateBBCodeED2kLink(file);
				if (!strLink.IsEmpty())
					theApp.CopyTextToClipboard(strLink);
				break;
			}
			// [TPT] - Announ: -Copy BBCode ed2k links-		
			case MP_OPEN:
				if(theApp.sharedfiles->IsFilePtrInList(file))
					OpenFile(file);
				break; 
			//For Comments
			case MP_SHOWED2KLINK:
				{
					ShowFileDialog(selectedList, IDD_ED2KLINK);
					break;
				}
			case MP_CMT: 
				ShowFileDialog(selectedList, IDD_COMMENT);
                		break; 
			case MPG_ALTENTER:
			case MP_DETAIL:{
				ShowFileDialog(selectedList);
				break;
				}
			case MP_REMOVESELECTED:
				{
					UINT i, uSelectedCount = GetSelectedCount();
					int  nItem = -1;

					if (uSelectedCount > 1)
					{
						if(MessageBox(GetResString(IDS_DOWNHISTORY_REMOVE_QUESTION_MULTIPLE),NULL,MB_YESNO) == IDYES){
							for (i=0;i < uSelectedCount;i++)
							{
								nItem = GetNextItem(nItem, LVNI_SELECTED);
								ASSERT(nItem != -1);
								CKnownFile *item_File = (CKnownFile *)GetItemData(nItem);
								if(item_File && !theApp.sharedfiles->IsFilePtrInList(item_File)){
									RemoveFile(item_File);
									nItem--;
								}
							}
						}
					}
					else
					{
						if(file){
							CString msg;
							msg.Format(GetResString(IDS_DOWNHISTORY_REMOVE_QUESTION),file->GetFileName());
							if(MessageBox(msg,NULL,MB_YESNO) == IDYES)
								RemoveFile(file);
						}
					}
				}
				break;
		}
	}

	switch(wParam){
		case MP_CLEARHISTORY:
			ClearHistory();
			break;
		// [TPT] - Due to SLUGFILLER: mergeKnown
		/*case MP_MERGEHISTORY:
			RemoveDuplicates();
			break;*/
		case MP_VIEWSHAREDFILES:
			thePrefs.SetShowSharedInHistory(!thePrefs.GetShowSharedInHistory());
			Reload();
			break;
	}

	return true;
}

void CHistoryListCtrl::ShowComments(CKnownFile* file) {
	if (file)
	{
		CTypedPtrList<CPtrList, CKnownFile*> aFiles;
		aFiles.AddHead(file);
		ShowFileDialog(aFiles, IDD_COMMENT);
	}
}

void CHistoryListCtrl::OpenFile(CKnownFile* file){
	TCHAR* buffer = new TCHAR[MAX_PATH];
	_sntprintf(buffer,MAX_PATH,_T("%s\\%s"),file->GetPath(),file->GetFileName());
	AddLogLine( false, _T("%s\\%s"),file->GetPath(),file->GetFileName());
	ShellOpenFile(buffer, NULL);
	delete[] buffer;
}

void CHistoryListCtrl::RemoveFile(CKnownFile *toRemove) {
	if(theApp.sharedfiles->IsFilePtrInList(toRemove))
		return;
	
	if(theApp.knownfiles->RemoveKnownFile(toRemove)){
	LVFINDINFO info;
	info.flags = LVFI_PARAM;
	info.lParam = (LPARAM)toRemove;
	int nItem = FindItem(&info);
	if(nItem != -1)
		DeleteItem(nItem);		
}
}

void CHistoryListCtrl::ClearHistory() {
	if(MessageBox(GetResString(IDS_DOWNHISTORY_CLEAR_QUESTION),GetResString(IDS_DOWNHISTORY),MB_YESNO)==IDYES)
	{
		theApp.knownfiles->ClearHistory();
		Reload();
	}
}


void CHistoryListCtrl::UpdateFile(const CKnownFile* file)
{
	if (!file || !theApp.emuledlg->IsRunning())
		return;
	int iItem = FindFile(file);
	if (iItem != -1)
	{
		Update(iItem);
		if (GetItemState(iItem, LVIS_SELECTED))
			theApp.emuledlg->sharedfileswnd->ShowSelectedFilesSummary();
	}
}

int CHistoryListCtrl::FindFile(const CKnownFile* pFile)
{
	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)pFile;
	return FindItem(&find);
}

void CHistoryListCtrl::ShowFileDialog(CTypedPtrList<CPtrList, CKnownFile*>& aFiles, UINT uPshInvokePage)
{
	if (aFiles.GetSize() > 0)
	{
		CHistoryFileDetailsSheet dialog(aFiles, uPshInvokePage, this);
		dialog.DoModal();
	}
}
//[TPT] - Double buffer style in lists
//TODO: I have done in this way because in future could be an option
void CHistoryListCtrl::SetDoubleBufferStyle()
{
	if((_AfxGetComCtlVersion() >= MAKELONG(0, 6)) && thePrefs.GetDoubleBufferStyle())	
		SetExtendedStyle(GetExtendedStyle() | 0x00010000 /*LVS_EX_DOUBLEBUFFER*/);
	else
		if((GetExtendedStyle() & 0x00010000 /*LVS_EX_DOUBLEBUFFER*/) != 0)
			SetExtendedStyle(GetExtendedStyle() ^ 0x00010000);//XOR: delete the style if present
}
