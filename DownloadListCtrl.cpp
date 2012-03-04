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
#include "DownloadListCtrl.h"
#include "otherfunctions.h" 
#include "updownclient.h"
#include "MenuCmds.h"
#include "ClientDetailDialog.h"
#include "FileDetailDialog.h"
#include "commentdialoglst.h"
#include "MetaDataDlg.h"
#include "InputBox.h"
#include "CommentDialog.h"	// [TPT] - SLUGFILLER: showComments
#include "KademliaWnd.h"
#include "emuledlg.h"
#include "DownloadQueue.h"
#include "FriendList.h"
#include "PartFile.h"
#include "ClientCredits.h"
#include "MemDC.h"
#include "ChatWnd.h"
#include "TransferWnd.h"
#include "MenuXP.h"// [TPT] - New Menu Styles
#include "Kademlia/Kademlia/Kademlia.h"
#include "Kademlia/Kademlia/Prefs.h"
#include "Kademlia/net/KademliaUDPListener.h"
#include "WebServices.h"
#include "Preview.h"
#include "StringConversion.h"
#include "AddSourceDlg.h"
// [TPT] - IP Country
#include "CxImage/xImage.h"
#include "ip2country.h"
// [TPT] - IP Country
#include "mod_version.h" // [TPT]
#include "HardLimitDlg.h" // [TPT] - Sivka AutoHL
#include "log.h"
#include "SR13-ImportParts.h"//[TPT] - SR13: Importparts

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif


// CDownloadListCtrl

#define DLC_DT_TEXT (DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS)
#define DLC_BARUPDATE 512

#define	FILE_ITEM_MARGIN_X	4
#define RATING_ICON_WIDTH	8


IMPLEMENT_DYNAMIC(CtrlItem_Struct, CObject)

IMPLEMENT_DYNAMIC(CDownloadListCtrl, CListBox)

BEGIN_MESSAGE_MAP(CDownloadListCtrl, CMuleListCtrl)
	ON_WM_MEASUREITEM()// [TPT] - New Menu Styles
	ON_WM_CONTEXTMENU()
	ON_WM_SYSCOLORCHANGE()
	ON_NOTIFY_REFLECT(LVN_ITEMACTIVATE, OnItemActivate)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnListModified)
	ON_NOTIFY_REFLECT(LVN_INSERTITEM, OnListModified)
	ON_NOTIFY_REFLECT(LVN_DELETEITEM, OnListModified)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclkDownloadlist)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
	// [TPT] - MFCK [addon] - New Tooltips [Rayita]
	ON_NOTIFY_REFLECT(LVN_KEYDOWN, OnLvnKeydown)
	ON_NOTIFY_REFLECT(NM_CLICK, OnNMClick)
	// [TPT] - MFCK [addon] - New Tooltips [Rayita]	
END_MESSAGE_MAP()

CDownloadListCtrl::CDownloadListCtrl()
	: CDownloadListListCtrlItemWalk(this)
{
}

CDownloadListCtrl::~CDownloadListCtrl(){
	// [TPT] - New Menu Styles 
	//if (m_PrioMenu) VERIFY( m_PrioMenu.DestroyMenu() );
	//if (m_A4AFMenu) VERIFY( m_A4AFMenu.DestroyMenu() );
	//if (m_PermMenu) VERIFY( m_PermMenu.DestroyMenu() );	// [TPT] - xMule_MOD: showSharePermissions
	//if (m_FileMenu) VERIFY( m_FileMenu.DestroyMenu() );
	while(m_ListItems.empty() == false){
		delete m_ListItems.begin()->second; // second = CtrlItem_Struct*
		m_ListItems.erase(m_ListItems.begin());
	}
}

void CDownloadListCtrl::Init()
{
	CImageList ilDummyImageList; //dummy list for getting the proper height of listview entries
	ilDummyImageList.Create(1, theApp.GetSmallSytemIconSize().cy, theApp.m_iDfltImageListColorFlags|ILC_MASK, 1, 1); 
	SetImageList(&ilDummyImageList, LVSIL_SMALL);
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) == 0 );
	ilDummyImageList.Detach();

	SetStyle();
	ModifyStyle(LVS_SINGLESEL,0);
	
	// [TPT] - MFCK [addon] - New Tooltips [Rayita]
	/*CToolTipCtrl* tooltip = GetToolTips();
	if (tooltip){
		tooltip->ModifyStyle(0, TTS_NOPREFIX);
		tooltip->SetDelayTime(TTDT_AUTOPOP, 20000);
		tooltip->SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay()*1000);
	}*/

	InsertColumn(0,GetResString(IDS_DL_FILENAME),LVCFMT_LEFT, 260);
	InsertColumn(1,GetResString(IDS_DL_SIZE),LVCFMT_LEFT, 60);
	InsertColumn(2,GetResString(IDS_DL_TRANSF),LVCFMT_LEFT, 65);
	InsertColumn(3,GetResString(IDS_DL_TRANSFCOMPL),LVCFMT_LEFT, 65);
	InsertColumn(4,GetResString(IDS_DL_SPEED),LVCFMT_LEFT, 65);
	InsertColumn(5,GetResString(IDS_DL_PROGRESS),LVCFMT_LEFT, 170);
	InsertColumn(6,GetResString(IDS_DL_SOURCES),LVCFMT_LEFT, 50);
	InsertColumn(7,GetResString(IDS_PRIORITY),LVCFMT_LEFT, 55);
	InsertColumn(8,GetResString(IDS_STATUS),LVCFMT_LEFT, 70);
	InsertColumn(9,GetResString(IDS_DL_REMAINS),LVCFMT_LEFT, 110);
	CString lsctitle=GetResString(IDS_LASTSEENCOMPL);
	lsctitle.Remove(':');
	InsertColumn(10, lsctitle,LVCFMT_LEFT, 220);
	lsctitle=GetResString(IDS_FD_LASTCHANGE);
	lsctitle.Remove(':');
	InsertColumn(11, lsctitle,LVCFMT_LEFT, 220);
	//InsertColumn(12, GetResString(IDS_CAT) ,LVCFMT_LEFT, 100);
	// [TPT] - khaos::categorymod+ Two new ResStrings, too.
	InsertColumn(12, GetResString(IDS_CAT_COLCATEGORY),LVCFMT_LEFT,60);
	InsertColumn(13, GetResString(IDS_CAT_COLORDER),LVCFMT_LEFT,60);
	// [TPT] - khaos::categorymod-
	// [TPT] - WebCache
	InsertColumn(14, GetResString(IDS_WEBCACHE_COLUMN),LVCFMT_LEFT, 100); //JP Webcache column	
	// [TPT] - WebCache

	SetAllIcons();
	Localize();
	LoadSettings(CPreferences::tableDownload);
	curTab=0;

	/*
	if (thePrefs.GetShowActiveDownloadsBold())
	{*/
		CFont* pFont = GetFont();
		LOGFONT lfFont = {0};
		pFont->GetLogFont(&lfFont);
		lfFont.lfWeight = FW_BOLD;
		m_fontBold.CreateFontIndirect(&lfFont);
	//}

	// Barry - Use preferred sort order from preferences
	m_bRemainSort=thePrefs.TransferlistRemainSortStyle();

	int sortItem = thePrefs.GetColumnSortItem(CPreferences::tableDownload);
	bool sortAscending = thePrefs.GetColumnSortAscending(CPreferences::tableDownload);

	uint8 adder=0;
	if (sortItem!=9 || !m_bRemainSort)
		SetSortArrow(sortItem, sortAscending);
	else {
        SetSortArrow(sortItem, sortAscending?arrowDoubleUp : arrowDoubleDown);
		adder=81;
	}
	SortItems(SortProc, 0x8000);	// [TPT] - SLUGFILLER: DLsortFix - uses multi-sort for fall-back
	// [TPT] - SLUGFILLER: multiSort - load multiple params
	if (m_bRemainSort)
		adder=81;
	for (int i = thePrefs.GetColumnSortCount(CPreferences::tableDownload); i > 0; ) {
		i--;
		sortItem = thePrefs.GetColumnSortItem(CPreferences::tableDownload, i);
		sortAscending = thePrefs.GetColumnSortAscending(CPreferences::tableDownload, i);
		if (sortItem!=9)
			SortItems(SortProc, sortItem + (sortAscending ? 0:100));
		else
			SortItems(SortProc, sortItem + (sortAscending ? 0:100) + adder);
	}
	// [TPT] - SLUGFILLER: multiSort

}

void CDownloadListCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
	//CreateMenues(); // [TPT] - New Menu Style
}

void CDownloadListCtrl::SetAllIcons()
{
	m_ImageList.DeleteImageList();
	m_ImageList.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	m_ImageList.SetBkColor(CLR_NONE);
	m_ImageList.Add(CTempIconLoader(_T("SrcDownloading")));//0
	m_ImageList.Add(CTempIconLoader(_T("SrcOnQueue")));//1
	m_ImageList.Add(CTempIconLoader(_T("SrcConnecting")));//2
	m_ImageList.Add(CTempIconLoader(_T("SrcNNPQF")));//3
	m_ImageList.Add(CTempIconLoader(_T("SrcUnknown")));//4
	m_ImageList.Add(CTempIconLoader(_T("ClientCompatible")));//5
	m_ImageList.Add(CTempIconLoader(_T("Friend")));//6
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkey")));//7
	m_ImageList.Add(CTempIconLoader(_T("ClientMLDonkey")));//8
	m_ImageList.Add(CTempIconLoader(_T("RatingReceived")));//9
	m_ImageList.Add(CTempIconLoader(_T("BadRatingReceived")));//10
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkeyHybrid")));//11
	m_ImageList.Add(CTempIconLoader(_T("ClientShareaza")));//12
	m_ImageList.Add(CTempIconLoader(_T("Server")));//13
	m_ImageList.Add(CTempIconLoader(_T("ClientAMule")));//14
	m_ImageList.Add(CTempIconLoader(_T("ClientLPhant")));//15
	m_ImageList.Add(CTempIconLoader(_T("ClientCompatiblePlus")));//16		// [TPT] - VQB: ownCredits
	// [TPT] - SLUGFILLER: showComments
	m_ImageList.Add(CTempIconLoader(_T("Rating_NotRated")));//17
	m_ImageList.Add(CTempIconLoader(_T("Rating_Fake")));//18
	m_ImageList.Add(CTempIconLoader(_T("Rating_Poor")));//19
	m_ImageList.Add(CTempIconLoader(_T("Rating_Good")));//20
	m_ImageList.Add(CTempIconLoader(_T("Rating_Fair")));//21
	m_ImageList.Add(CTempIconLoader(_T("Rating_Excellent")));//22
	m_ImageList.Add(CTempIconLoader(_T("EMPTY")));//23
	// [TPT] - SLUGFILLER: showComments	
	m_ImageList.Add(CTempIconLoader(_T("IDI_SNAFU"))); //24 // [TPT] - eWombat SNAFU v2
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("ClientSecureOvl"))), 1);//25
	// [TPT] - Own icon
	m_ImageList.Add(CTempIconLoader(_T("PHOENIX")));//26
	m_ImageList.Add(CTempIconLoader(_T("PHOENIXPLUS")));//27
	// [TPT] - Own icon
	m_ImageList.Add(CTempIconLoader(_T("WEBCACHE"))); // [TPT] - WebCache	// jp webcacheclient icon //28
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("FileCommentsOvl"))), 2);	// 29
}

void CDownloadListCtrl::Localize()
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;

	if(pHeaderCtrl->GetItemCount() != 0) {
	CString strRes;

	strRes = GetResString(IDS_DL_FILENAME);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(0, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_DL_SIZE);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(1, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_DL_TRANSF);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(2, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_DL_TRANSFCOMPL);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(3, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_DL_SPEED);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(4, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_DL_PROGRESS);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(5, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_DL_SOURCES);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(6, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_PRIORITY);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(7, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_STATUS);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(8, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_DL_REMAINS);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(9, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_LASTSEENCOMPL);
	strRes.Remove(':');
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(10, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_FD_LASTCHANGE);
	strRes.Remove(':');
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(11, &hdi);
	strRes.ReleaseBuffer();
	
	// [TPT] - khaos::categorymod+
	strRes = GetResString(IDS_CAT_COLCATEGORY);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(12, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_CAT_COLORDER);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(13, &hdi);
	strRes.ReleaseBuffer();
	// [TPT] - khaos::categorymod-

	// [TPT] - WebCache 
	strRes = GetResString(IDS_WEBCACHE_COLUMN);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(14, &hdi);
	strRes.ReleaseBuffer();
	// [TPT] - WebCache 
}
	//CreateMenues(); // [TPT] - New Menu Style

	ShowFilesCount();
}

void CDownloadListCtrl::AddFile(CPartFile* toadd)
{
	// Create new Item
    CtrlItem_Struct* newitem = new CtrlItem_Struct;
    uint16 itemnr = GetItemCount();
    newitem->owner = NULL;
    newitem->type = FILE_TYPE;
    newitem->value = toadd;
    newitem->parent = NULL;
	newitem->dwUpdated = 0; 

	// The same file shall be added only once
	ASSERT(m_ListItems.find(toadd) == m_ListItems.end());
	m_ListItems.insert(ListItemsPair(toadd, newitem));

	if (toadd->CheckShowItemInGivenCat(curTab))
		InsertItem(LVIF_PARAM|LVIF_TEXT,itemnr,LPSTR_TEXTCALLBACK,0,0,0,(LPARAM)newitem);

	ShowFilesCount();
}

void CDownloadListCtrl::AddSource(CPartFile* owner, CUpDownClient* source, bool notavailable)
{
	// Create new Item
    CtrlItem_Struct* newitem = new CtrlItem_Struct;
    newitem->owner = owner;
    newitem->type = (notavailable) ? UNAVAILABLE_SOURCE : AVAILABLE_SOURCE;
    newitem->value = source;
	newitem->dwUpdated = 0; 

	// Update cross link to the owner
	ListItems::const_iterator ownerIt = m_ListItems.find(owner);
	ASSERT(ownerIt != m_ListItems.end());
	CtrlItem_Struct* ownerItem = ownerIt->second;
	ASSERT(ownerItem->value == owner);
	newitem->parent = ownerItem;

	// The same source could be added a few time but only one time per file 
	{
		// Update the other instances of this source
		bool bFound = false;
		std::pair<ListItems::const_iterator, ListItems::const_iterator> rangeIt = m_ListItems.equal_range(source);
		for(ListItems::const_iterator it = rangeIt.first; it != rangeIt.second; it++){
			CtrlItem_Struct* cur_item = it->second;

			// Check if this source has been already added to this file => to be sure
			if(cur_item->owner == owner){
				// Update this instance with its new setting
				cur_item->type = newitem->type;
				cur_item->dwUpdated = 0;
				bFound = true;
			}
			else if(notavailable == false){
				// The state 'Available' is exclusive
				cur_item->type = UNAVAILABLE_SOURCE;
				cur_item->dwUpdated = 0;
			}
		}

		if(bFound == true){
			delete newitem; 
			return;
		}
	}
	m_ListItems.insert(ListItemsPair(source, newitem));

	if (owner->srcarevisible) {
		// find parent from the CListCtrl to add source
		LVFINDINFO find;
		find.flags = LVFI_PARAM;
		find.lParam = (LPARAM)ownerItem;
		sint16 result = FindItem(&find);
		if(result != (-1))
			InsertItem(LVIF_PARAM|LVIF_TEXT,result+1,LPSTR_TEXTCALLBACK,0,0,0,(LPARAM)newitem);
	}
}

void CDownloadListCtrl::RemoveSource(CUpDownClient* source, CPartFile* owner)
{
	if (!theApp.emuledlg->IsRunning())
		return;

	// Retrieve all entries matching the source
	std::pair<ListItems::iterator, ListItems::iterator> rangeIt = m_ListItems.equal_range(source);
	for(ListItems::iterator it = rangeIt.first; it != rangeIt.second; ){
		CtrlItem_Struct* delItem  = it->second;
		if(owner == NULL || owner == delItem->owner){
			// Remove it from the m_ListItems			
			it = m_ListItems.erase(it);

			// Remove it from the CListCtrl
 			LVFINDINFO find;
			find.flags = LVFI_PARAM;
			find.lParam = (LPARAM)delItem;
			sint16 result = FindItem(&find);
			if(result != (-1)){
				DeleteItem(result);
			}

			// finally it could be delete
			delete delItem;
		}
		else{
			it++;
		}
	}
}

bool CDownloadListCtrl::RemoveFile(const CPartFile* toremove)
{
	bool bResult = false;
	if (!theApp.emuledlg->IsRunning())
		return bResult;
	// Retrieve all entries matching the File or linked to the file
	// Remark: The 'asked another files' clients must be removed from here
	ASSERT(toremove != NULL);
	for(ListItems::iterator it = m_ListItems.begin(); it != m_ListItems.end(); ){
		CtrlItem_Struct* delItem = it->second;
		if(delItem->owner == toremove || delItem->value == (void*)toremove){
			// Remove it from the m_ListItems
			it = m_ListItems.erase(it);

			// Remove it from the CListCtrl
			LVFINDINFO find;
			find.flags = LVFI_PARAM;
			find.lParam = (LPARAM)delItem;
			sint16 result = FindItem(&find);
			if(result != (-1)){
				DeleteItem(result);
			}

			// finally it could be delete
			delete delItem;
			bResult = true;
		}
		else {
			it++;
		}
	}
	ShowFilesCount();
	return bResult;
}

void CDownloadListCtrl::UpdateItem(void* toupdate)
{
	if (!theApp.emuledlg->IsRunning())
		return;
	// [TPT] - MORPH START - SiRoB, Don't Refresh item if not needed
	if( theApp.emuledlg->activewnd != theApp.emuledlg->transferwnd  || theApp.emuledlg->transferwnd->downloadlistctrl.IsWindowVisible() == false )
		return;
	// [TPT] - MORPH START - SiRoB, Don't Refresh item if not needed

	// Retrieve all entries matching the source
	std::pair<ListItems::const_iterator, ListItems::const_iterator> rangeIt = m_ListItems.equal_range(toupdate);
	for(ListItems::const_iterator it = rangeIt.first; it != rangeIt.second; it++){
		CtrlItem_Struct* updateItem  = it->second;

		// Find entry in CListCtrl and update object
 		LVFINDINFO find;
		find.flags = LVFI_PARAM;
		find.lParam = (LPARAM)updateItem;
		sint16 result = FindItem(&find);
		if (result != -1){
			updateItem->dwUpdated = 0;
			Update(result);
		}
	}
}

void CDownloadListCtrl::DrawFileItem(CDC *dc, int nColumn, LPCRECT lpRect, CtrlItem_Struct *lpCtrlItem)
{
	if(lpRect->left < lpRect->right)
	{
		CString buffer;
		CPartFile *lpPartFile = (CPartFile*)lpCtrlItem->value;
		// [TPT] - Morph. Don't draw hidden Rect
		if (thePrefs.GetCatColor(lpPartFile->GetCategory()) > 0)
			dc->SetTextColor(thePrefs.GetCatColor(lpPartFile->GetCategory()));
		// [TPT] - Morph. Don't draw hidden Rect
		switch(nColumn)
		{
		case 0:{		// file name
			CRect rcDraw(lpRect);			
			// [TPT] - itsonlyme: displayOptions START
			if (thePrefs.ShowFileSystemIcon())
			{
				int iImage = theApp.GetFileTypeSystemImageIdx(lpPartFile->GetFileName());
				if (theApp.GetSystemImageList() != NULL)
					::ImageList_Draw(theApp.GetSystemImageList(), iImage, dc->GetSafeHdc(), rcDraw.left, rcDraw.top, ILD_NORMAL|ILD_TRANSPARENT);
				// [TPT] - SLUGFILLER: showComments
				if (!thePrefs.ShowLocalRating() && (!lpPartFile->GetFileComment().IsEmpty() || lpPartFile->GetFileRating()))
					m_ImageList.Draw(dc, 23, CPoint(rcDraw.left, rcDraw.top), ILD_NORMAL | ILD_TRANSPARENT | INDEXTOOVERLAYMASK(2));
				// [TPT] - SLUGFILLER: showComments
				rcDraw.left += theApp.GetSmallSytemIconSize().cx;
				}
			// [TPT] - itsonlyme: displayOptions END

			// [TPT] - SLUGFILLER: showComments
			if (thePrefs.ShowLocalRating() && (!lpPartFile->GetFileComment().IsEmpty() || lpPartFile->GetFileRating())){
				m_ImageList.Draw(dc, 17+lpPartFile->GetFileRating(), rcDraw.TopLeft(), ILD_NORMAL | ILD_TRANSPARENT | (lpPartFile->GetFileComment().IsEmpty()?0:INDEXTOOVERLAYMASK(2)));
				rcDraw.left += 16;
			}
			// [TPT] - SLUGFILLER: showComments
			if ( thePrefs.ShowRatingIndicator() && ( lpPartFile->HasComment() || lpPartFile->HasRating() )){
				// [TPT] - SLUGFILLER: showComments
				if (lpRect->left != rcDraw.left)
					rcDraw.left+=2;		// space between comment icons
				// [TPT] - SLUGFILLER: showComments
				m_ImageList.Draw(dc, (lpPartFile->HasRating() && lpPartFile->HasBadRating()) ? 10 : 9, rcDraw.TopLeft(), ILD_NORMAL);
				rcDraw.left += 8;
			}
			
			rcDraw.left += 3;
				dc->DrawText(lpPartFile->GetFileName(), lpPartFile->GetFileName().GetLength(), &rcDraw, DLC_DT_TEXT);
			break;
		}

		case 1:		// size
			buffer = CastItoXBytes(lpPartFile->GetFileSize(), false, false);
			dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT | DT_RIGHT);
			break;

		case 2:		// transferred
			buffer = CastItoXBytes(lpPartFile->GetTransferred(), false, false);
			dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT | DT_RIGHT);
			break;

		case 3:		// transferred complete
			buffer = CastItoXBytes(lpPartFile->GetCompletedSize(), false, false);
			dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT | DT_RIGHT);
			break;
		case 4:		// speed
			if (lpPartFile->GetTransferringSrcCount())
				// [TPT]
				buffer.Format(_T("%s"), CastItoXBytes(lpPartFile->GetDownloadFileDatarate(), false, true)); // Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
			dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT | DT_RIGHT);
			break;

		case 5:		// progress
			{
				CRect rcDraw(*lpRect);
				rcDraw.bottom--;
				rcDraw.top++;

				// added
				int iWidth = rcDraw.Width();
				int iHeight = rcDraw.Height();
				if (lpCtrlItem->status == (HBITMAP)NULL)
					VERIFY(lpCtrlItem->status.CreateBitmap(1, 1, 1, 8, NULL)); 
				CDC cdcStatus;
				HGDIOBJ hOldBitmap;
				cdcStatus.CreateCompatibleDC(dc);
				int cx = lpCtrlItem->status.GetBitmapDimension().cx; 
				DWORD dwTicks = GetTickCount();
				if(lpCtrlItem->dwUpdated + DLC_BARUPDATE < dwTicks || cx !=  iWidth || !lpCtrlItem->dwUpdated) {
					lpCtrlItem->status.DeleteObject(); 
					lpCtrlItem->status.CreateCompatibleBitmap(dc,  iWidth, iHeight); 
					lpCtrlItem->status.SetBitmapDimension(iWidth,  iHeight); 
					hOldBitmap = cdcStatus.SelectObject(lpCtrlItem->status); 

					RECT rec_status; 
					rec_status.left = 0; 
					rec_status.top = 0; 
					rec_status.bottom = iHeight; 
					rec_status.right = iWidth; 
					lpPartFile->DrawStatusBar(&cdcStatus,  &rec_status, thePrefs.UseFlatBar()); 

					lpCtrlItem->dwUpdated = dwTicks + (rand() % 128); 
				} else 
					hOldBitmap = cdcStatus.SelectObject(lpCtrlItem->status); 

				dc->BitBlt(rcDraw.left, rcDraw.top, iWidth, iHeight,  &cdcStatus, 0, 0, SRCCOPY); 
				cdcStatus.SelectObject(hOldBitmap);
				//added end

				if (thePrefs.GetUseDwlPercentage()) {
					// HoaX_69: BEGIN Display percent in progress bar
					COLORREF oldclr = dc->SetTextColor(RGB(255,255,255));
					int iOMode = dc->SetBkMode(TRANSPARENT);
					buffer.Format(_T("%.1f%%"), lpPartFile->GetPercentCompleted());
					dc->DrawText(buffer, buffer.GetLength(), &rcDraw, (DLC_DT_TEXT & ~DT_LEFT) | DT_CENTER);
					dc->SetBkMode(iOMode);
					dc->SetTextColor(oldclr);
					// HoaX_69: END
				}
			}
			break;
		// [TPT] - valid sources/sources+A4AF sources[hard limit] (transfering sources)
		// [TPT] - Sivka AutoHL
		case 6:		// sources
			{
				uint16 sc = lpPartFile->GetSourceCount();
				uint16 ncsc = lpPartFile->GetNotCurrentSourcesCount();				
// ZZ:DownloadManager -->
                if(!(lpPartFile->GetStatus() == PS_PAUSED && sc == 0) && lpPartFile->GetStatus() != PS_COMPLETE) {
                    buffer.Format(_T("%i"), sc-ncsc);
				    if(ncsc>0) buffer.AppendFormat(_T("/%i"), sc);
                    if(thePrefs.IsExtControlsEnabled() && lpPartFile->GetSrcA4AFCount() > 0) buffer.AppendFormat(_T("+%i"), lpPartFile->GetSrcA4AFCount());
				    if(lpPartFile->GetTransferringSrcCount() > 0) buffer.AppendFormat(_T(" (%i)"), lpPartFile->GetTransferringSrcCount());
				    buffer.AppendFormat(_T(" [%i]"), lpPartFile->GetMaxSourcesPerFile());				    
                } else {
                    buffer = _T("");
				}
// <-- ZZ:DownloadManager
				dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT | DT_RIGHT);
			}
			break;
		// [TPT] - Sivka AutoHL end
		case 7:{		// prio
			CString fullPriority;
			switch(lpPartFile->GetDownPriority()) 
			{
				case PR_LOW:
					if( lpPartFile->IsAutoDownPriority() )
						fullPriority = GetResString(IDS_PRIOAUTOLOW);
					else
						fullPriority = GetResString(IDS_PRIOLOW);
					break;
				case PR_NORMAL:
					if( lpPartFile->IsAutoDownPriority() )
						fullPriority = GetResString(IDS_PRIOAUTONORMAL);
					else
						fullPriority = GetResString(IDS_PRIONORMAL);
					break;
				case PR_HIGH:
					if( lpPartFile->IsAutoDownPriority() )
						fullPriority = GetResString(IDS_PRIOAUTOHIGH);
					else
						fullPriority = GetResString(IDS_PRIOHIGH);
					break;
			}
			
			//[TPT]- Show upload priority in downloadlist
			//no change in compare, we mantain this sort behaviour
			if(thePrefs.GetShowUpPrioInDownloadList())
			{
				fullPriority += _T(" | ");
				switch(lpPartFile->GetUpPriority())
				{
					case PR_VERYLOW :
						fullPriority += GetResString(IDS_PRIOVERYLOW);
						break;
					case PR_LOW :
						if( lpPartFile->IsAutoUpPriority() )
							fullPriority += GetResString(IDS_PRIOAUTOLOW);
						else
							fullPriority += GetResString(IDS_PRIOLOW);
						break;
					case PR_NORMAL :
						if( lpPartFile->IsAutoUpPriority() )
							fullPriority += GetResString(IDS_PRIOAUTONORMAL);
						else
							fullPriority += GetResString(IDS_PRIONORMAL);
						break;
					case PR_HIGH :
						if( lpPartFile->IsAutoUpPriority() )
							fullPriority += GetResString(IDS_PRIOAUTOHIGH);
						else
							fullPriority += GetResString(IDS_PRIOHIGH);
						break;
					case PR_VERYHIGH :
						fullPriority += GetResString(IDS_PRIORELEASE);
						break;
					default:
						fullPriority.Empty();	
				}
				if(lpPartFile->GetPowerShared()) 
				{
					CString tempString = GetResString(IDS_POWERSHARE_PREFIX);
					tempString.Append(_T(" "));
					tempString.Append(fullPriority);
					fullPriority.Empty();
					fullPriority = tempString;
				}
			}
			dc->DrawText(fullPriority,fullPriority.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
			break;
		}
		case 8:		// <<--9/21/02
			buffer = lpPartFile->getPartfileStatus();
			dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
			break;

		case 9:		// remaining time & size
			{
				if (lpPartFile->GetStatus()!=PS_COMPLETING && lpPartFile->GetStatus()!=PS_COMPLETE ){
					// time 
					time_t restTime;
					if (!thePrefs.UseSimpleTimeRemainingComputation())
						restTime = lpPartFile->getTimeRemaining();
					else
						restTime = lpPartFile->getTimeRemainingSimple();

					buffer.Format(_T("%s (%s)"), CastSecondsToHM(restTime), CastItoXBytes((lpPartFile->GetFileSize() - lpPartFile->GetCompletedSize()), false, false));
				}
				dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
			}
			break;
		case 10: // last seen complete
			{
				CString tempbuffer;
				if (lpPartFile->m_nCompleteSourcesCountLo == 0)
				{
					tempbuffer.Format(_T("< %u"), lpPartFile->m_nCompleteSourcesCountHi);
				}
				else if (lpPartFile->m_nCompleteSourcesCountLo == lpPartFile->m_nCompleteSourcesCountHi)
				{
					tempbuffer.Format(_T("%u"), lpPartFile->m_nCompleteSourcesCountLo);
				}
				else
				{
					tempbuffer.Format(_T("%u - %u"), lpPartFile->m_nCompleteSourcesCountLo, lpPartFile->m_nCompleteSourcesCountHi);
				}
				if (lpPartFile->lastseencomplete==NULL)
					buffer.Format(_T("%s (%s)"),GetResString(IDS_NEVER),tempbuffer);
				else
					buffer.Format(_T("%s (%s)"),lpPartFile->lastseencomplete.Format( thePrefs.GetDateTimeFormat()),tempbuffer);
				dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
			}
			break;
		case 11: // last receive
			if (!IsColumnHidden(11)) {
				if(lpPartFile->GetFileDate()!=NULL && lpPartFile->GetRealFileSize()>0)
					buffer=lpPartFile->GetCFileDate().Format( thePrefs.GetDateTimeFormat());
				else
					buffer.Format(_T("%s"),GetResString(IDS_NEVER));

				dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
			}
			break;
		/*case 12: // cat
			if (!IsColumnHidden(12)) {
				buffer=(lpPartFile->GetCategory()!=0)?
					thePrefs.GetCategory(lpPartFile->GetCategory())->title:_T("");
				dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
			}
			break;
		}*/
		// [TPT] - khaos::categorymod+
		case 12: // Category
			{
				if (!thePrefs.ShowCatNameInDownList())
					buffer.Format(_T("%u"), lpPartFile->GetCategory());
				else
					buffer.Format(_T("%s"), thePrefs.GetCategory(lpPartFile->GetCategory())->title);
				dc->DrawText(buffer, (int) _tcslen(buffer), const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
				break;
			}
		case 13: // Resume Mod
			{
				buffer.Format(_T("%u"), lpPartFile->GetCatResumeOrder());
				dc->DrawText(buffer, (int) _tcslen(buffer), const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
				break;
			}
		// [TPT] - khaos::categorymod-
		// [TPT] - WebCache 
		//JP Webcache START
		//JP added code from Gnaddelwarz
		case 14:
			{
				uint16 wcsc = lpPartFile->GetWebcacheSourceCount();
				uint16 wcsc_our;
				uint16 wcsc_not_our;
				if(thePrefs.IsExtControlsEnabled())
				{
				wcsc_our = lpPartFile->GetWebcacheSourceOurProxyCount();
				wcsc_not_our = lpPartFile->GetWebcacheSourceNotOurProxyCount();
				}

				uint16 sc = lpPartFile->GetSourceCount();
				double PercentWCClients;
				if (sc !=0)
					PercentWCClients = (double) 100 * wcsc / sc;
				else
					PercentWCClients = 0;
                if(wcsc > 0 && !(lpPartFile->GetStatus() == PS_PAUSED && wcsc == 0) && lpPartFile->GetStatus() != PS_COMPLETE) {
					if(thePrefs.IsExtControlsEnabled())
						buffer.Format(_T("%i/%i/%i (%1.1f%%)"), wcsc, wcsc_our, wcsc_not_our, PercentWCClients);
					else 
						buffer.Format(_T("%i (%1.1f%%)"), wcsc, PercentWCClients);
                } else {
                    buffer = _T("");
				}
				dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT | DT_RIGHT);
			}
			break;
		//JP Webcache END
		}		
		// [TPT] - WebCache
	}
}

void CDownloadListCtrl::DrawSourceItem(CDC *dc, int nColumn, LPCRECT lpRect, CtrlItem_Struct *lpCtrlItem) {
	if(lpRect->left < lpRect->right) { 

		CString buffer;
		CUpDownClient *lpUpDownClient = (CUpDownClient*)lpCtrlItem->value;
		switch(nColumn) {

		case 0:		// icon, name, status
			{
				COLORREF crOldTxtColor;

				RECT cur_rec = *lpRect;
				POINT point = {cur_rec.left, cur_rec.top+1};
				if (lpCtrlItem->type == AVAILABLE_SOURCE){
					switch (lpUpDownClient->GetDownloadState()) {
					case DS_CONNECTING:
						m_ImageList.Draw(dc, 2, point, ILD_NORMAL);
						crOldTxtColor = dc->SetTextColor((COLORREF)RGB(210,210,10));
						break;
					case DS_CONNECTED:
						m_ImageList.Draw(dc, 2, point, ILD_NORMAL);
						crOldTxtColor = dc->SetTextColor((COLORREF)RGB(210,210,10));
						break;
					case DS_WAITCALLBACKKAD:
					case DS_WAITCALLBACK:
						m_ImageList.Draw(dc, 2, point, ILD_NORMAL);
						crOldTxtColor = dc->SetTextColor((COLORREF)RGB(210,210,10));
						break;
					case DS_ONQUEUE:
						if(lpUpDownClient->IsRemoteQueueFull())
							m_ImageList.Draw(dc, 3, point, ILD_NORMAL);
						else
							m_ImageList.Draw(dc, 1, point, ILD_NORMAL);
						crOldTxtColor = dc->SetTextColor(GetSysColor(COLOR_GRAYTEXT));
						break;
					case DS_DOWNLOADING:
						m_ImageList.Draw(dc, 0, point, ILD_NORMAL);
						crOldTxtColor = dc->SetTextColor((COLORREF)RGB(192,0,0));
						break;
					case DS_REQHASHSET:
						m_ImageList.Draw(dc, 0, point, ILD_NORMAL);
						crOldTxtColor = dc->SetTextColor((COLORREF)RGB(245,240,100));
						break;
					case DS_NONEEDEDPARTS:
						m_ImageList.Draw(dc, 3, point, ILD_NORMAL);
						crOldTxtColor = dc->SetTextColor((COLORREF)RGB(30,200,240));
						break;
					case DS_ERROR:
						m_ImageList.Draw(dc, 3, point, ILD_NORMAL);
						crOldTxtColor = dc->SetTextColor((COLORREF)RGB(255,0,0));
						break;
					case DS_TOOMANYCONNS:
					case DS_TOOMANYCONNSKAD:
						m_ImageList.Draw(dc, 2, point, ILD_NORMAL);
						crOldTxtColor = dc->SetTextColor((COLORREF)RGB(135,135,135)); 
						break;
					default:
						m_ImageList.Draw(dc, 4, point, ILD_NORMAL);
						crOldTxtColor = dc->SetTextColor(GetSysColor(COLOR_GRAYTEXT));

					}
				}
				else {

					m_ImageList.Draw(dc, 3, point, ILD_NORMAL);
				}
				cur_rec.left += 20;
				UINT uOvlImg = ((lpUpDownClient->Credits() && lpUpDownClient->Credits()->GetCurrentIdentState(lpUpDownClient->GetIP()) == IS_IDENTIFIED) ? INDEXTOOVERLAYMASK(1) : 0);
				// [TPT] - eWombat SNAFU v2
				if (lpUpDownClient->IsSnafu()){
					POINT point2= {cur_rec.left,cur_rec.top+1};
					m_ImageList.Draw(dc, 24, point2, ILD_NORMAL | uOvlImg);
					crOldTxtColor = dc->SetTextColor((COLORREF)RGB(255,0,0));			
				}
				// [TPT] - eWombat SNAFU v2
				else if (lpUpDownClient->IsFriend()){
				POINT point2= {cur_rec.left,cur_rec.top+1};
					m_ImageList.Draw(dc, 6, point2, ILD_NORMAL | uOvlImg);
				}
				else
				{
					POINT point2= {cur_rec.left,cur_rec.top+1};
					if (lpUpDownClient->GetClientSoft() == SO_EDONKEYHYBRID)
					m_ImageList.Draw(dc, 11, point2, ILD_NORMAL | uOvlImg);
				else if (lpUpDownClient->GetClientSoft() == SO_MLDONKEY)
					m_ImageList.Draw(dc, 8, point2, ILD_NORMAL | uOvlImg);
				else if (lpUpDownClient->GetClientSoft() == SO_SHAREAZA)
					m_ImageList.Draw(dc, 12, point2, ILD_NORMAL | uOvlImg);
				else if (lpUpDownClient->GetClientSoft() == SO_URL)
					m_ImageList.Draw(dc, 13, point2, ILD_NORMAL | uOvlImg);
				else if (lpUpDownClient->GetClientSoft() == SO_AMULE)
					m_ImageList.Draw(dc, 14, point2, ILD_NORMAL | uOvlImg);
				else if (lpUpDownClient->GetClientSoft() == SO_LPHANT)
					m_ImageList.Draw(dc, 15, point2, ILD_NORMAL | uOvlImg);
				// [TPT] - WebCache						
				// jp webcacheclient icon START 
				else if (lpUpDownClient->GetClientSoft() == SO_WEBCACHE)
					m_ImageList.Draw(dc, 28, point2, ILD_NORMAL | uOvlImg);
				// jp webcacheclient icon END
				// [TPT] - WebCache	
				// [TPT] - Own icon
				else if (lpUpDownClient->ExtProtocolAvailable())
				{
					// [TPT] - VQB: ownCredits - show clients we have net credits
					if (lpUpDownClient->Credits() && lpUpDownClient->Credits()->GetMyScoreRatio(lpUpDownClient->GetIP()) > 1)
						m_ImageList.Draw(dc, (lpUpDownClient->GetpHoeniXClient())? 27 : 16, point2, ILD_NORMAL | uOvlImg);
					else
					// [TPT] - VQB: ownCredits
						m_ImageList.Draw(dc, (lpUpDownClient->GetpHoeniXClient())? 26 : 5, point2, ILD_NORMAL | uOvlImg);
				}
				// [TPT] - Own icon
				else
					m_ImageList.Draw(dc, 7, point2, ILD_NORMAL | uOvlImg);
				}
				cur_rec.left += 20;

				// [TPT] - IP Country					
				if(thePrefs.GetEnableShowCountryFlags())
				{
					CxImage imagen = theApp.ip2country->GetCountryFlag(lpUpDownClient->GetCountryIP());
					if (imagen.IsEnabled())
					{
						imagen.Draw(*dc, cur_rec.left, cur_rec.top+1, 16, 16);
						cur_rec.left +=20;
					}
				}
				// [TPT] - IP Country	


				if (!lpUpDownClient->GetUserName())
					buffer = "?";
				else
					buffer = lpUpDownClient->GetUserName();
				dc->DrawText(buffer,buffer.GetLength(),&cur_rec, DLC_DT_TEXT);
				dc->SetTextColor(crOldTxtColor);
			}
			break;

		case 1:		// size
			switch(lpUpDownClient->GetSourceFrom()){
				case SF_SERVER:
					buffer = "eD2K Server";
					break;
				case SF_KADEMLIA:
					buffer = GetResString(IDS_KADEMLIA);
					break;
				case SF_SOURCE_EXCHANGE:
					buffer = GetResString(IDS_SE);
					break;
				case SF_PASSIVE:
					buffer = GetResString(IDS_PASSIVE);
					break;
				case SF_LINK:
					buffer = GetResString(IDS_SW_LINK);
					break;
				case SF_SLS:
					buffer = _T("SLS");
					break;
			}
			dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
			break;

		case 2:// transferred
			//[TPT] - Download/Upload
			/*
			if ( !IsColumnHidden(3)) {
				dc->DrawText(_T(""),0,const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
				break;
			}
			*/
			if(lpUpDownClient->Credits() && (lpUpDownClient->Credits()->GetUploadedTotal() || lpUpDownClient->Credits()->GetDownloadedTotal())){
				buffer.Format( _T("%s/%s"),
				CastItoXBytes((float)lpUpDownClient->Credits()->GetDownloadedTotal(), false, false),
				CastItoXBytes((float)lpUpDownClient->Credits()->GetUploadedTotal(), false, false));
				dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
			}
			break;
			//[TPT] - Download/Upload end
		case 3:// completed
			if (lpCtrlItem->type == AVAILABLE_SOURCE && lpUpDownClient->GetTransferredDown()) {
				buffer = CastItoXBytes(lpUpDownClient->GetTransferredDown(), false, false);
				dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT | DT_RIGHT);
			}
			break;

		case 4:		// speed
			if (lpCtrlItem->type == AVAILABLE_SOURCE && lpUpDownClient->GetDownloadDatarate()){
				if (lpUpDownClient->GetDownloadDatarate())
					buffer.Format(_T("%s"), CastItoXBytes(lpUpDownClient->GetDownloadDatarate(), false, true));
				dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT | DT_RIGHT);
			}
			break;

		case 5:		// file info
			{
				CRect rcDraw(*lpRect);
				rcDraw.bottom--; 
				rcDraw.top++; 

				int iWidth = rcDraw.Width();
				int iHeight = rcDraw.Height();
				if (lpCtrlItem->status == (HBITMAP)NULL)
					VERIFY(lpCtrlItem->status.CreateBitmap(1, 1, 1, 8, NULL)); 
				CDC cdcStatus;
				HGDIOBJ hOldBitmap;
				cdcStatus.CreateCompatibleDC(dc);
				int cx = lpCtrlItem->status.GetBitmapDimension().cx;
				DWORD dwTicks = GetTickCount();
				if(lpCtrlItem->dwUpdated + DLC_BARUPDATE < dwTicks || cx !=  iWidth  || !lpCtrlItem->dwUpdated) { 
					lpCtrlItem->status.DeleteObject(); 
					lpCtrlItem->status.CreateCompatibleBitmap(dc,  iWidth, iHeight); 
					lpCtrlItem->status.SetBitmapDimension(iWidth,  iHeight); 
					hOldBitmap = cdcStatus.SelectObject(lpCtrlItem->status); 

					RECT rec_status; 
					rec_status.left = 0; 
					rec_status.top = 0; 
					rec_status.bottom = iHeight; 
					rec_status.right = iWidth; 
					lpUpDownClient->DrawStatusBar(&cdcStatus,  &rec_status,(lpCtrlItem->type == UNAVAILABLE_SOURCE), thePrefs.UseFlatBar()); 

					lpCtrlItem->dwUpdated = dwTicks + (rand() % 128); 
				} else 
					hOldBitmap = cdcStatus.SelectObject(lpCtrlItem->status); 

				dc->BitBlt(rcDraw.left, rcDraw.top, iWidth, iHeight,  &cdcStatus, 0, 0, SRCCOPY); 
				cdcStatus.SelectObject(hOldBitmap);
			}
			break;

		case 6:		// sources
		{
			buffer = lpUpDownClient->DbgGetFullClientSoftVer(); // [TPT]
			if (buffer.IsEmpty())
				buffer = GetResString(IDS_UNKNOWN);
			CRect rc(lpRect);
			dc->DrawText(buffer, buffer.GetLength(), &rc, DLC_DT_TEXT);
			break;
		}

		case 7:	{	// prio 
			// [TPT] - EatsShare: Addeded by TAHO, last asked time
			uint32 lastAskedTime = lpUpDownClient->GetLastAskedTime();
			if ( lastAskedTime )
				buffer = CastSecondsToHM(( ::GetTickCount() - lastAskedTime) /1000);
			else
				buffer = "?";
			dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), (DLC_DT_TEXT | DT_RIGHT) & ~DT_LEFT);
			// [TPT] - EatsShare: Addeded by TAHO, last asked time
			}
			break;

		case 8:	{	// status
			if (lpCtrlItem->type == AVAILABLE_SOURCE){
				buffer = lpUpDownClient->GetDownloadStateDisplayString();
			}
			else {
				buffer = GetResString(IDS_ASKED4ANOTHERFILE);

// ZZ:DownloadManager -->
                if(thePrefs.IsExtControlsEnabled()) {
                    if(lpUpDownClient->IsInNoNeededList(lpCtrlItem->owner)) {
                        buffer += _T(" (") + GetResString(IDS_NONEEDEDPARTS) + _T(")");
                    } else if(lpUpDownClient->GetDownloadState() == DS_DOWNLOADING) {
                        buffer += _T(" (") + GetResString(IDS_TRANSFERRING) + _T(")");
                    } else if(lpUpDownClient->IsSwapSuspended(lpUpDownClient->GetRequestFile())) {
                        buffer += _T(" (") + GetResString(IDS_SOURCESWAPBLOCKED) + _T(")");
                    }

                    if (lpUpDownClient && lpUpDownClient->GetRequestFile() && lpUpDownClient->GetRequestFile()->GetFileName()){
                        buffer.AppendFormat(_T(": \"%s\""),lpUpDownClient->GetRequestFile()->GetFileName());
                    }
                }
			}

            if(thePrefs.IsExtControlsEnabled() && !lpUpDownClient->m_OtherRequests_list.IsEmpty()) {
                buffer.Append(_T("*"));
            }
// ZZ:DownloadManager <--

			COLORREF crOldTxtColor; // [TPT] - EastShare - Moddified by TAHO, color
			if (lpCtrlItem->type == AVAILABLE_SOURCE){
				switch (lpUpDownClient->GetDownloadState()) {
					case DS_CONNECTING:
						crOldTxtColor = dc->SetTextColor((COLORREF)RGB(210,210,10));
						break;
					case DS_CONNECTED:
						crOldTxtColor = dc->SetTextColor((COLORREF)RGB(210,210,10));
						break;
					case DS_WAITCALLBACK:
						crOldTxtColor = dc->SetTextColor((COLORREF)RGB(210,210,10));
						break;
					case DS_ONQUEUE:
						if( lpUpDownClient->IsRemoteQueueFull() )
						{
							crOldTxtColor = dc->SetTextColor((COLORREF)RGB(10,130,160));
						}
						else {
							uint16 nRemoteQueueRank = lpUpDownClient->GetRemoteQueueRank();
							if (nRemoteQueueRank)
							{
								uint16 nDifference = lpUpDownClient->GetDifference();
								if (nDifference==nRemoteQueueRank)
								{
									crOldTxtColor = dc->SetTextColor((COLORREF)RGB(0,0,0));
								}      
								else if (nDifference<nRemoteQueueRank)
								{  
									crOldTxtColor = dc->SetTextColor((COLORREF)RGB(190,60,60));
									if (nDifference==0)
										{
											crOldTxtColor = dc->SetTextColor((COLORREF)RGB(5,65,195));
										}
								} 
								else 
								{
									crOldTxtColor = dc->SetTextColor((COLORREF)RGB(10,160,70));
								}
							}
							else{
								crOldTxtColor = dc->SetTextColor((COLORREF)RGB(50,80,140));
							}
						}
						break;
					case DS_DOWNLOADING:
						crOldTxtColor = dc->SetTextColor((COLORREF)RGB(192,0,0));
						break;
					case DS_REQHASHSET:
						crOldTxtColor = dc->SetTextColor((COLORREF)RGB(245,240,100));
						break;
					case DS_NONEEDEDPARTS:
						crOldTxtColor = dc->SetTextColor((COLORREF)RGB(30,200,240)); 
						break;
					case DS_LOWTOLOWIP:
						crOldTxtColor = dc->SetTextColor((COLORREF)RGB(135,135,135)); 
						break;
					case DS_TOOMANYCONNS:
						crOldTxtColor = dc->SetTextColor((COLORREF)RGB(135,135,135)); 
						break;
					default:
						crOldTxtColor = dc->SetTextColor((COLORREF)RGB(135,135,135)); 
				}
			}
			else {
				crOldTxtColor = dc->SetTextColor((COLORREF)RGB(200,80,200));
			}
			// [TPT] - EastShare END - Added by TAHO, color
			dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
			dc->SetTextColor(crOldTxtColor);
			break;
				}
		case 9:	 {	// [TPT] - IP Country				
			if (thePrefs.GetEnableShowCountryNames())
			{
				RECT cur_rec;
				MEMCOPY(&cur_rec, lpRect, sizeof(RECT));						
				CString Sbuffer;		
				Sbuffer = theApp.ip2country->GetCountryFromIP(lpUpDownClient->GetCountryIP());
				dc->DrawText(Sbuffer,Sbuffer.GetLength(),&cur_rec,DLC_DT_TEXT);			
			}
			// [TPT] - IP Country			
			break;
		}
		case 10:	// last seen complete
			break;
		case 11:	// last received
			break;
		case 12:	// category
			break;
// [TPT] - WebCache ////////////////////////////////////////////////////////////////////////////////////
		//JP Webcache START
		case 14: {
			if (lpUpDownClient->SupportsWebCache())
				{
					buffer = lpUpDownClient->GetWebCacheName();
					if (lpUpDownClient->IsBehindOurWebCache())
						dc->SetTextColor(RGB(0, 180, 0)); //if is behind our webcache display green
					else if (buffer != _T(""))
						dc->SetTextColor(RGB(255, 0, 0)); // if webcache info is there but not our own set red
					else
						buffer = _T("no proxy set");	// if no webcache info colour is black
				}
				else
					buffer = "";
				dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
 				dc->SetTextColor(RGB(0, 0, 0));
				break;
			}
		//JP Webcache END
		}
	}
}

void CDownloadListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (!theApp.emuledlg->IsRunning())
		return;
	if (!lpDrawItemStruct->itemData)
		return;

	// [TPT] - Morph. Don't draw hidden Rect
	RECT clientRect;
	GetClientRect(&clientRect);
	RECT cur_rec = lpDrawItemStruct->rcItem;
	if (cur_rec.top >= clientRect.bottom || cur_rec.bottom <= clientRect.top)
		return;
	// [TPT] - Morph. Don't draw hidden Rect

	CDC* odc = CDC::FromHandle(lpDrawItemStruct->hDC);
	CtrlItem_Struct* content = (CtrlItem_Struct*)lpDrawItemStruct->itemData;
	BOOL bCtrlFocused = ((GetFocus() == this) || (GetStyle() & LVS_SHOWSELALWAYS));

	if ((lpDrawItemStruct->itemAction | ODA_SELECT) && (lpDrawItemStruct->itemState & ODS_SELECTED)) {
		if(bCtrlFocused)
			odc->SetBkColor(m_crHighlight);
		else
			odc->SetBkColor(m_crNoHighlight);
	}
	else
		odc->SetBkColor(GetBkColor());

	CMemDC dc(odc, &lpDrawItemStruct->rcItem);
	CFont *pOldFont;
	if (m_fontBold.m_hObject && thePrefs.GetShowActiveDownloadsBold()){
		if (content->type == FILE_TYPE){
			if (((const CPartFile*)content->value)->GetTransferringSrcCount())
				pOldFont = dc->SelectObject(&m_fontBold);
			else
				pOldFont = dc->SelectObject(GetFont());
		}
		else if (content->type == UNAVAILABLE_SOURCE || content->type == AVAILABLE_SOURCE){
			if (((const CUpDownClient*)content->value)->GetDownloadState() == DS_DOWNLOADING)
				pOldFont = dc->SelectObject(&m_fontBold);
			else
				pOldFont = dc->SelectObject(GetFont());
		}
		else
			pOldFont = dc->SelectObject(GetFont());
	}
	else
		pOldFont = dc->SelectObject(GetFont());
	COLORREF crOldTextColor = dc->SetTextColor(m_crWindowText);

	int iOldBkMode;
	if (m_crWindowTextBk == CLR_NONE){
		DefWindowProc(WM_ERASEBKGND, (WPARAM)(HDC)dc, 0);
		iOldBkMode = dc.SetBkMode(TRANSPARENT);
	}
	else
		iOldBkMode = OPAQUE;

	BOOL notLast = lpDrawItemStruct->itemID + 1 != GetItemCount();
	BOOL notFirst = lpDrawItemStruct->itemID != 0;
	int tree_start=0;
	int tree_end=0;

	// RECT cur_rec = lpDrawItemStruct->rcItem; // [TPT] - Morph. Don't draw hidden Rect

	//offset was 4, now it's the standard 2 spaces
	int iTreeOffset = dc->GetTextExtent(_T(" "), 1 ).cx*2;
	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();
	cur_rec.right = cur_rec.left;
	cur_rec.right -= FILE_ITEM_MARGIN_X;
	cur_rec.left += FILE_ITEM_MARGIN_X;

	if (content->type == FILE_TYPE){
		for(int iCurrent = 0; iCurrent < iCount; iCurrent++) {

			int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
			int cx = CListCtrl::GetColumnWidth(iColumn);
			if(iColumn == 5) {
				int iNextLeft = cur_rec.left + cx;
				//set up tree vars
				cur_rec.left = cur_rec.right + iTreeOffset;
				cur_rec.right = cur_rec.left + min(8, cx);
				tree_start = cur_rec.left + 1;
				tree_end = cur_rec.right;
				//normal column stuff
				cur_rec.left = cur_rec.right + 1;
				cur_rec.right = tree_start + cx - iTreeOffset;
				// [TPT] - MORPH START - Added by SiRoB, Don't draw hidden columns
				if (cur_rec.left < clientRect.right && cur_rec.right > clientRect.left)
				// [TPT] - MORPH END   - Added by SiRoB, Don't draw hidden columns
					DrawFileItem(dc, 5, &cur_rec, content);
				cur_rec.left = iNextLeft;
			} else {
				cur_rec.right += cx;
				// [TPT] - MORPH START - Added by SiRoB, Don't draw hidden columns
				if (cur_rec.left < clientRect.right && cur_rec.right > clientRect.left)
				// [TPT] - MORPH END   - Added by SiRoB, Don't draw hidden columns
					DrawFileItem(dc, iColumn, &cur_rec, content);
				cur_rec.left += cx;
			}

		}

	}
	else if (content->type == UNAVAILABLE_SOURCE || content->type == AVAILABLE_SOURCE){

		for(int iCurrent = 0; iCurrent < iCount; iCurrent++) {

			int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
			int cx = CListCtrl::GetColumnWidth(iColumn);

			if(iColumn == 5) {
				int iNextLeft = cur_rec.left + cx;
				//set up tree vars
				cur_rec.left = cur_rec.right + iTreeOffset;
				cur_rec.right = cur_rec.left + min(8, cx);
				tree_start = cur_rec.left + 1;
				tree_end = cur_rec.right;
				//normal column stuff
				cur_rec.left = cur_rec.right + 1;
				cur_rec.right = tree_start + cx - iTreeOffset;
				DrawSourceItem(dc, 5, &cur_rec, content);
				cur_rec.left = iNextLeft;
			} else {
				cur_rec.right += cx;
				DrawSourceItem(dc, iColumn, &cur_rec, content);
				cur_rec.left += cx;
			}
		}
	}

	//draw rectangle around selected item(s)
	if ((lpDrawItemStruct->itemAction | ODA_SELECT) &&
		(lpDrawItemStruct->itemState & ODS_SELECTED) &&
		(content->type == FILE_TYPE))
	{
		RECT outline_rec = lpDrawItemStruct->rcItem;

		outline_rec.top--;
		outline_rec.bottom++;
		dc->FrameRect(&outline_rec, &CBrush(GetBkColor()));
		outline_rec.top++;
		outline_rec.bottom--;
		outline_rec.left++;
		outline_rec.right--;

		if(notFirst && (GetItemState(lpDrawItemStruct->itemID - 1, LVIS_SELECTED))) {
			CtrlItem_Struct* prev = (CtrlItem_Struct*)this->GetItemData(lpDrawItemStruct->itemID - 1);
			if(prev->type == FILE_TYPE)
				outline_rec.top--;
		} 

		if(notLast && (GetItemState(lpDrawItemStruct->itemID + 1, LVIS_SELECTED))) {
			CtrlItem_Struct* next = (CtrlItem_Struct*)this->GetItemData(lpDrawItemStruct->itemID + 1);
			if(next->type == FILE_TYPE)
				outline_rec.bottom++;
		} 

		if(bCtrlFocused)
			dc->FrameRect(&outline_rec, &CBrush(m_crFocusLine));
		else
			dc->FrameRect(&outline_rec, &CBrush(m_crNoFocusLine));
	}
	//draw focus rectangle around non-highlightable items when they have the focus
	else if (((lpDrawItemStruct->itemState & ODS_FOCUS) == ODS_FOCUS) && (GetFocus() == this))
	{
		RECT focus_rec;
		focus_rec.top    = lpDrawItemStruct->rcItem.top;
		focus_rec.bottom = lpDrawItemStruct->rcItem.bottom;
		focus_rec.left   = lpDrawItemStruct->rcItem.left + 1;
		focus_rec.right  = lpDrawItemStruct->rcItem.right - 1;
		dc->FrameRect(&focus_rec, &CBrush(m_crNoFocusLine));
	}

	//draw tree last so it draws over selected and focus (looks better)
	if(tree_start < tree_end) {
		//set new bounds
		RECT tree_rect;
		tree_rect.top    = lpDrawItemStruct->rcItem.top;
		tree_rect.bottom = lpDrawItemStruct->rcItem.bottom;
		tree_rect.left   = tree_start;
		tree_rect.right  = tree_end;
		dc->SetBoundsRect(&tree_rect, DCB_DISABLE);

		//gather some information
		BOOL hasNext = notLast &&
			((CtrlItem_Struct*)this->GetItemData(lpDrawItemStruct->itemID + 1))->type != FILE_TYPE;
		BOOL isOpenRoot = hasNext && content->type == FILE_TYPE;
		BOOL isChild = content->type != FILE_TYPE;
		//BOOL isExpandable = !isChild && ((CPartFile*)content->value)->GetSourceCount() > 0;
		//might as well calculate these now
		int treeCenter = tree_start + 3;
		int middle = (cur_rec.top + cur_rec.bottom + 1) / 2;

		//set up a new pen for drawing the tree
		CPen pn, *oldpn;
		pn.CreatePen(PS_SOLID, 1, m_crWindowText);
		oldpn = dc->SelectObject(&pn);

		if(isChild) {
			//draw the line to the status bar
			dc->MoveTo(tree_end, middle);
			dc->LineTo(tree_start + 3, middle);

			//draw the line to the child node
			if(hasNext) {
				dc->MoveTo(treeCenter, middle);
				dc->LineTo(treeCenter, cur_rec.bottom + 1);
			}
		} else if(isOpenRoot) {
			//draw circle
			RECT circle_rec;
			COLORREF crBk = dc->GetBkColor();
			circle_rec.top    = middle - 2;
			circle_rec.bottom = middle + 3;
			circle_rec.left   = treeCenter - 2;
			circle_rec.right  = treeCenter + 3;
			dc->FrameRect(&circle_rec, &CBrush(m_crWindowText));
			dc->SetPixelV(circle_rec.left,      circle_rec.top,    crBk);
			dc->SetPixelV(circle_rec.right - 1, circle_rec.top,    crBk);
			dc->SetPixelV(circle_rec.left,      circle_rec.bottom - 1, crBk);
			dc->SetPixelV(circle_rec.right - 1, circle_rec.bottom - 1, crBk);
			//draw the line to the child node
			if(hasNext) {
				dc->MoveTo(treeCenter, middle + 3);
				dc->LineTo(treeCenter, cur_rec.bottom + 1);
			}
		} /*else if(isExpandable) {
			//draw a + sign
			dc->MoveTo(treeCenter, middle - 2);
			dc->LineTo(treeCenter, middle + 3);
			dc->MoveTo(treeCenter - 2, middle);
			dc->LineTo(treeCenter + 3, middle);
		}*/

		//draw the line back up to parent node
		if(notFirst && isChild) {
			dc->MoveTo(treeCenter, middle);
			dc->LineTo(treeCenter, cur_rec.top - 1);
		}

		//put the old pen back
		dc->SelectObject(oldpn);
		pn.DeleteObject();
	}

	//put the original objects back
	if (m_crWindowTextBk == CLR_NONE)
		dc.SetBkMode(iOldBkMode);
	dc->SelectObject(pOldFont);
	dc->SetTextColor(crOldTextColor);
}

void CDownloadListCtrl::HideSources(CPartFile* toCollapse)
{
	SetRedraw(false);
	int pre = 0;
	int post = 0;
	for (int i = 0; i < GetItemCount(); i++)
	{
		CtrlItem_Struct* item = (CtrlItem_Struct*)GetItemData(i);
		if (item->owner == toCollapse)
		{
			pre++;
			item->dwUpdated = 0;
			item->status.DeleteObject();
			DeleteItem(i--);
			post++;
		}
	}
	if (pre - post == 0)
		toCollapse->srcarevisible = false;
	SetRedraw(true);
}

void CDownloadListCtrl::ExpandCollapseItem(int iItem, int iAction, bool bCollapseSource)
{
	if (iItem == -1)
		return;
	CtrlItem_Struct* content = (CtrlItem_Struct*)GetItemData(iItem);

	// to collapse/expand files when one of its source is selected
	if (bCollapseSource && content->parent != NULL)
	{
		content=content->parent;
		
 		LVFINDINFO find;
		find.flags = LVFI_PARAM;
		find.lParam = (LPARAM)content;
		iItem = FindItem(&find);
		if (iItem == -1)
			return;
	}

	if (!content || content->type != FILE_TYPE)
		return;
	
	CPartFile* partfile = reinterpret_cast<CPartFile*>(content->value);
	if (!partfile)
		return;

	if (partfile->GetStatus()==PS_COMPLETE) {
		ShellOpenFile(partfile->GetFullName(), NULL);
		return;
	}

	// Check if the source branch is disable
	if (!partfile->srcarevisible)
	{
		if (iAction > COLLAPSE_ONLY)
		{
			SetRedraw(false);
			
			// Go throught the whole list to find out the sources for this file
			// Remark: don't use GetSourceCount() => UNAVAILABLE_SOURCE
			for (ListItems::const_iterator it = m_ListItems.begin(); it != m_ListItems.end(); it++)
			{
				const CtrlItem_Struct* cur_item = it->second;
				if (cur_item->owner == partfile)
				{
					partfile->srcarevisible = true;
					InsertItem(LVIF_PARAM|LVIF_TEXT, iItem+1, LPSTR_TEXTCALLBACK, 0, 0, 0, (LPARAM)cur_item);
				}
			}

			SetRedraw(true);
		}
	}
	else {
		if (iAction == EXPAND_COLLAPSE || iAction == COLLAPSE_ONLY)
		{
			if (GetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED) != (LVIS_SELECTED | LVIS_FOCUSED))
			{
				SetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				SetSelectionMark(iItem);
			}
			HideSources(partfile);
		}
	}
}

void CDownloadListCtrl::OnItemActivate(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	if (thePrefs.IsDoubleClickEnabled() || pNMIA->iSubItem > 0)
		ExpandCollapseItem(pNMIA->iItem, EXPAND_COLLAPSE);
	*pResult = 0;
}

// [TPT] - New Menu Styles BEGIN
void CDownloadListCtrl::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	HMENU hMenu = AfxGetThreadState()->m_hTrackingMenu;
	CMenu	*pMenu = CMenu::FromHandle(hMenu);
	pMenu->MeasureItem(lpMeasureItemStruct);
	
	CWnd::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}
// [TPT] - New Menu Styles END

// [TPT] - New Menu Styles BEGIN
void CDownloadListCtrl::OnContextMenu(CWnd* pWnd, CPoint point)
{

	// Main menu
	CMenuXP *pFileMenu = new CMenuXP;
	pFileMenu->CreatePopupMenu();
	pFileMenu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
	pFileMenu->AddSideBar(new CMenuXPSideBar(17, MOD_VERSION));
	pFileMenu->SetSideBarStartColor(RGB(255,0,0));
	pFileMenu->SetSideBarEndColor(RGB(255,128,0));
	pFileMenu->SetSelectedBarColor(RGB(242,120,114));
	//[TPT] - Show Bitmap background in menus
	//If we extend this feature to more menus...we should rework the menuXP class
	if(thePrefs.GetShowBitmapInMenus())
	{
		pFileMenu->SetBackBitmap(_T("FONDODOWNLOAD"), _T("JPG"));
	}

	// Priority
	CMenuXP *pPrioMenu = new CMenuXP;
	pPrioMenu->CreatePopupMenu();
	pPrioMenu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
	pPrioMenu->SetSelectedBarColor(RGB(242,120,114));
	pPrioMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_PRIOLOW,GetResString(IDS_PRIOLOW)));
	pPrioMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_PRIONORMAL,GetResString(IDS_PRIONORMAL)));
	pPrioMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_PRIOHIGH,GetResString(IDS_PRIOHIGH)));
	pPrioMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_PRIOAUTO,GetResString(IDS_PRIOAUTO)));
	
	// A4AF
	CMenuXP *pA4AFMenu = new CMenuXP;
	pA4AFMenu->CreatePopupMenu();
	pA4AFMenu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
	pA4AFMenu->SetSelectedBarColor(RGB(242,120,114));		
	pA4AFMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_ALL_A4AF_AUTO,GetResString(IDS_ALL_A4AF_AUTO)));

	// Permissions
	CMenuXP *pPermMenu = new CMenuXP;
	pPermMenu->CreatePopupMenu();
	pPermMenu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
	pPermMenu->SetSelectedBarColor(RGB(242,120,114));
	pPermMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_PERMNONE,GetResString(IDS_HIDDEN)));
	pPermMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_PERMFRIENDS,GetResString(IDS_FSTATUS_FRIENDSONLY)));
	pPermMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_PERMALL,GetResString(IDS_FSTATUS_PUBLIC)));

	// Web Services
	CMenuXP *pWebMenu = new CMenuXP;
	pWebMenu->CreatePopupMenu();
	pWebMenu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
	pWebMenu->SetSelectedBarColor(RGB(242,120,114));
	
	// Categories
	CMenuXP *pCatMenu = new CMenuXP;
	pCatMenu->CreatePopupMenu();
	pCatMenu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
	pCatMenu->SetSelectedBarColor(RGB(242,120,114));
	
	// end of create menues
	
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1)
	{
		const CtrlItem_Struct* content = (CtrlItem_Struct*)GetItemData(iSel);
		if (content->type == FILE_TYPE)
		{
			// get merged settings
			bool bFirstItem = true;
			int iSelectedItems = 0;
			int iFilesNotDone = 0;
			int iFilesToPause = 0;
			int iFilesToStop = 0;
			int iFilesToResume = 0;
			int iFilesToOpen = 0;
            int iFilesGetPreviewParts = 0;
            int iFilesPreviewType = 0;
			int iFilesToPreview = 0;
			int iFilesA4AFAuto = 0;
			UINT uPrioMenuItem = 0;
			UINT uPermMenuItem = 0; // [TPT] - xMule_MOD: showSharePermissions
			const CPartFile* file1 = NULL;
			POSITION pos = GetFirstSelectedItemPosition();
			while (pos)
			{
				const CtrlItem_Struct* pItemData = (CtrlItem_Struct*)GetItemData(GetNextSelectedItem(pos));
				if (pItemData->type != FILE_TYPE)
					continue;
				const CPartFile* pFile = (CPartFile*)pItemData->value;
				if (bFirstItem)
					file1 = pFile;
				iSelectedItems++;

				bool bFileDone = (pFile->GetStatus()==PS_COMPLETE || pFile->GetStatus()==PS_COMPLETING);
				iFilesNotDone += !bFileDone ? 1 : 0;
				iFilesToStop += pFile->CanStopFile() ? 1 : 0;
				iFilesToPause += pFile->CanPauseFile() ? 1 : 0;
				iFilesToResume += pFile->CanResumeFile() ? 1 : 0;
				iFilesToOpen += pFile->CanOpenFile() ? 1 : 0;
                iFilesGetPreviewParts += pFile->GetPreviewPrio() ? 1 : 0;
                iFilesPreviewType += pFile->IsPreviewableFileType() ? 1 : 0;
				iFilesToPreview += pFile->IsReadyForPreview() ? 1 : 0;
				iFilesA4AFAuto += (!bFileDone && pFile->IsA4AFAuto()) ? 1 : 0;

				UINT uCurPrioMenuItem = 0;
				if (pFile->IsAutoDownPriority())
					uCurPrioMenuItem = MP_PRIOAUTO;
				else if (pFile->GetDownPriority() == PR_HIGH)
					uCurPrioMenuItem = MP_PRIOHIGH;
				else if (pFile->GetDownPriority() == PR_NORMAL)
					uCurPrioMenuItem = MP_PRIONORMAL;
				else if (pFile->GetDownPriority() == PR_LOW)
					uCurPrioMenuItem = MP_PRIOLOW;
				else
					ASSERT(0);

				if (bFirstItem)
					uPrioMenuItem = uCurPrioMenuItem;
				else if (uPrioMenuItem != uCurPrioMenuItem)
					uPrioMenuItem = 0;

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
				// [TPT] - xMule_MOD: showSharePermissions
				
				bFirstItem = false;
			}

			pFileMenu->AppendODPopup(MF_STRING | (iFilesNotDone > 0 ? MF_ENABLED : MF_GRAYED), pPrioMenu, new CMenuXPText(0, GetResString(IDS_PRIORITY), theApp.LoadIcon(_T("priority"), 16, 16)));
			pPrioMenu->CheckMenuRadioItem(MP_PRIOLOW, MP_PRIOAUTO, uPrioMenuItem, 0);
		
			// [TPT] - xMule_MOD: showSharePermissions
			pFileMenu->AppendODPopup(MF_STRING | (iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED), pPermMenu, new CMenuXPText(0, GetResString(IDS_PERMISSION), theApp.LoadIcon(_T("permission"), 16, 16)));			
			pPermMenu->CheckMenuRadioItem(MP_PERMALL, MP_PERMNONE, uPermMenuItem, 0);
			// [TPT] - xMule_MOD: showSharePermissions



			pFileMenu->AppendODMenu(MF_STRING | (iFilesNotDone > 0 ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_CANCEL, GetResString(IDS_MAIN_BTN_CANCEL), theApp.LoadIcon(_T("delete"), 16, 16)));
			pFileMenu->AppendODMenu(MF_STRING | (iFilesToStop > 0 ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_STOP, GetResString(IDS_DL_STOP), theApp.LoadIcon(_T("stop"), 16, 16)));
			pFileMenu->AppendODMenu(MF_STRING | (iFilesToPause > 0 ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_PAUSE, GetResString(IDS_DL_PAUSE), theApp.LoadIcon(_T("pause"), 16, 16)));
			pFileMenu->AppendODMenu(MF_STRING | (iFilesToResume > 0 ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_RESUME, GetResString(IDS_DL_RESUME), theApp.LoadIcon(_T("start"), 16, 16)));
			pFileMenu->AppendSeparator();
			bool bOpenEnabled = (iSelectedItems == 1 && iFilesToOpen == 1);
			pFileMenu->AppendODMenu(MF_STRING | (bOpenEnabled ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_OPEN, GetResString(IDS_DL_OPEN)));
			if(thePrefs.IsExtControlsEnabled() && !thePrefs.GetPreviewPrio()) {
			    pFileMenu->AppendODMenu(MF_STRING | (iSelectedItems == 1 && iFilesPreviewType == 1 && iFilesToPreview == 0 && iFilesNotDone == 1) ? MF_ENABLED : MF_GRAYED, new CMenuXPText(MP_TRY_TO_GET_PREVIEW_PARTS, GetResString(IDS_DL_TRY_TO_GET_PREVIEW_PARTS)) );
			    pFileMenu->CheckMenuItem(MP_TRY_TO_GET_PREVIEW_PARTS, (iSelectedItems == 1 && iFilesGetPreviewParts == 1) ? MF_CHECKED : MF_UNCHECKED);
            }
			pFileMenu->AppendODMenu(MF_STRING | ((iSelectedItems == 1 && iFilesToPreview == 1) ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_PREVIEW, GetResString(IDS_DL_PREVIEW), theApp.LoadIcon(_T("preview"), 16, 16)));

			// Preview Menu
			CMenuXP *pPreviewMenu = new CMenuXP;
			pPreviewMenu->CreatePopupMenu();
			pPreviewMenu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
			pPreviewMenu->SetSelectedBarColor(RGB(242,120,114));

			int iPreviewMenuEntries = thePreviewApps.GetAllMenuEntries(pPreviewMenu, (iSelectedItems == 1) ? file1 : NULL);
			if (iPreviewMenuEntries)
				pFileMenu->AppendODPopup(MF_STRING | (iSelectedItems == 1 ? MF_ENABLED : MF_GRAYED), pPreviewMenu, new CMenuXPText(0, GetResString(IDS_DL_PREVIEW)));
			bool bDetailsEnabled = (iSelectedItems > 0);
			pFileMenu->AppendODMenu(MF_STRING | (bDetailsEnabled ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_METINFO, GetResString(IDS_DL_INFO), theApp.LoadIcon(_T("DETAILS"), 16,16)));
					
			if (thePrefs.IsDoubleClickEnabled() && bOpenEnabled)
			pFileMenu->SetDefaultItem(MP_OPEN);
			else if (!thePrefs.IsDoubleClickEnabled() && bDetailsEnabled)
			pFileMenu->SetDefaultItem(MP_METINFO);
			else
			pFileMenu->SetDefaultItem((UINT)-1);

			// [TPT] - SLUGFILLER: showComments
			pFileMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_CMT, GetResString(IDS_CMT_ADD), theApp.LoadIcon(_T("FILECOMMENTS"), 16,16)));
			pFileMenu->AppendODMenu(MF_STRING | (iSelectedItems == 1 ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_VIEWFILECOMMENTS, GetResString(IDS_CMT_SHOWALL), theApp.LoadIcon(_T("comments"), 16, 16)));
			// [TPT] - SLUGFILLER: showComments
			pFileMenu->AppendSeparator();
			pFileMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_FAKECHECK1, _T("FakeCheck -> DonkeyFakes.Gambri.Net"), theApp.LoadIcon(_T("fakes"), 16, 16)));
			//[TPT] - SR13: Import Parts
			pFileMenu->AppendODMenu(MF_STRING | ((iSelectedItems == 1 && iFilesNotDone == 1) ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_SR13_ImportParts, GetResString(IDS_IMPORTPARTS), theApp.LoadIcon(_T("IMPORT"), 16,16)));
			//pFileMenu->EnableMenuItem(MP_SR13_InitiateRehash, (iSelectedItems == 1 && iFilesNotDone == 1) ? MF_ENABLED : MF_GRAYED);
			//[TPT] - SR13: Import Parts			
			pFileMenu->AppendSeparator();
			int total;
			pFileMenu->AppendODMenu(MF_STRING | (GetCompleteDownloads(curTab, total) > 0 ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_CLEARCOMPLETED, GetResString(IDS_DL_CLEAR), theApp.LoadIcon(_T("clean"), 16, 16)));
			
			// [TPT] - Sivka AutoHL
			pFileMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_HARD_LIMIT, GetResString(IDS_AUTOHLMENU)));
			
			if (thePrefs.IsExtControlsEnabled())
				pFileMenu->AppendODPopup(MF_STRING | ((iSelectedItems == 1 && iFilesNotDone == 1) ? MF_ENABLED : MF_GRAYED), pA4AFMenu, new CMenuXPText(0, GetResString(IDS_A4AF)));
			pA4AFMenu->CheckMenuItem(MP_ALL_A4AF_AUTO, (iSelectedItems == 1 && iFilesNotDone == 1 && iFilesA4AFAuto == 1) ? MF_CHECKED : MF_UNCHECKED);
			if (thePrefs.IsExtControlsEnabled())
				pFileMenu->AppendODMenu(MF_STRING | (iSelectedItems == 1 && iFilesToStop == 1) ? MF_ENABLED : MF_GRAYED, new CMenuXPText(MP_ADDSOURCE, GetResString(IDS_ADDSRCMANUALLY)));
				
			pFileMenu->AppendSeparator();


			if(thePrefs.GetShowCopyEd2kLinkCmd())
				pFileMenu->AppendODMenu(MF_STRING | (iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_GETED2KLINK, GetResString(IDS_COPY), theApp.LoadIcon(_T("COPY"), 16, 16)));
			else
				pFileMenu->AppendODMenu(MF_STRING | (iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_SHOWED2KLINK, GetResString(IDS_DL_SHOWED2KLINK), theApp.LoadIcon(_T("COPY"), 16, 16)));
			pFileMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_SAVEED2KLINK, GetResString(IDS_DL_SAVELINK), theApp.LoadIcon(_T("diskStore"), 16, 16)));
			pFileMenu->AppendODMenu(MF_STRING | (theApp.IsEd2kFileLinkInClipboard() ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_PASTE, GetResString(IDS_SW_DIRECTDOWNLOAD)));		
			pFileMenu->AppendSeparator();

			int iWebMenuEntries = theWebServices.GetFileMenuEntries(pWebMenu);
			UINT flag = (iWebMenuEntries == 0 || iSelectedItems != 1) ? MF_GRAYED : MF_ENABLED;
			pFileMenu->AppendODPopup(MF_STRING | MF_POPUP | flag, pWebMenu, new CMenuXPText(0, GetResString(IDS_WEBSERVICES), theApp.LoadIcon(_T("webServices"), 16, 16)));

			// [TPT] - khaos::categorymod+
			flag = (thePrefs.GetCatCount() == 1) ? MF_GRAYED : MF_ENABLED;
			CString catTitle;
			if (thePrefs.GetCatCount()>1)
			{
				for (int i = 0; i < thePrefs.GetCatCount(); i++)
				{
				//[TPT] - Fix
				catTitle = thePrefs.GetCategory(i)->title;
				catTitle.Replace(_T("&"), _T("&&") );
				pCatMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_ASSIGNCAT+i,(i==0)?GetResString(IDS_CAT_UNASSIGN):catTitle));
				}
			}

			pFileMenu->AppendODPopup(MF_STRING | MF_POPUP | flag, pCatMenu, new CMenuXPText(0, GetResString(IDS_TOCAT), theApp.LoadIcon(_T("category"), 16, 16)));
									
			CMenuXP	*pMenuOrder = new CMenuXP;
			pMenuOrder->CreatePopupMenu();
			pMenuOrder->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
			pMenuOrder->SetSelectedBarColor(RGB(242,120,114));
			if (this->GetSelectedCount() > 1) {												
				pMenuOrder->AppendODMenu(MF_STRING,new CMenuXPText(MP_CAT_ORDERAUTOINC, GetResString(IDS_CAT_MNUAUTOINC)));
				pMenuOrder->AppendODMenu(MF_STRING,new CMenuXPText(MP_CAT_ORDERSTEPTHRU, GetResString(IDS_CAT_MNUSTEPTHRU)));
				pMenuOrder->AppendODMenu(MF_STRING,new CMenuXPText(MP_CAT_ORDERALLSAME, GetResString(IDS_CAT_MNUALLSAME)));	
				pFileMenu->AppendODPopup(MF_STRING | MF_POPUP, pMenuOrder, new CMenuXPText(0, GetResString(IDS_CAT_SETORDER) ));							
				
		}
			else {
				pFileMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_CAT_SETRESUMEORDER, GetResString(IDS_CAT_SETORDER)));					
			}
			// [TPT] - khaos::categorymod-
			
			pFileMenu->TrackPopupMenu(TPM_LEFTBUTTON, point.x, point.y, this);			
			
			delete pPreviewMenu;
			delete pMenuOrder;
			
		}
		else {

			// [TPT] - Moved here to avoid creating menu if no client exists
			CUpDownClient* client = (CUpDownClient*)content->value;
			if (client == NULL)
				return;

			// Main menu
			CMenuXP *pClientMenu = new CMenuXP;
			pClientMenu->CreatePopupMenu();
			pClientMenu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
			pClientMenu->AddSideBar(new CMenuXPSideBar(17, MOD_VERSION));
			pClientMenu->SetSideBarStartColor(RGB(255,0,0));
			pClientMenu->SetSideBarEndColor(RGB(255,128,0));
			pClientMenu->SetSelectedBarColor(RGB(242,120,114));


			//Add items
			pClientMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_DETAIL, GetResString(IDS_SHOWDETAILS), theApp.LoadIcon(_T("details"), 16, 16)));
			pClientMenu->SetDefaultItem(MP_DETAIL);
			// [TPT] - itsonlyme:clientDetails
			pClientMenu->AppendODMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetViewSharedFilesSupport()) ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_SHOWLIST, GetResString(IDS_VIEWFILES), theApp.LoadIcon(_T("seeshared2"), 16, 16)));
			pClientMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_LIST_REQUESTED_FILES, GetResString(IDS_LIST_REQ_FILES), theApp.LoadIcon(_T("requestedFiles2"), 16, 16)));
			pClientMenu->AppendSeparator();
			// [TPT] - itsonlyme:clientDetails
			pClientMenu->AppendODMenu(MF_STRING | ((client && client->IsEd2kClient() && !client->IsFriend()) ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_ADDFRIEND, GetResString(IDS_ADDFRIEND), theApp.LoadIcon(_T("friend2"), 16, 16)));
			pClientMenu->AppendODMenu(MF_STRING | ((client && client->IsEd2kClient()) ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_MESSAGE, GetResString(IDS_SEND_MSG), theApp.LoadIcon(_T("sendMessage"), 16, 16)));

			pClientMenu->AppendSeparator();
			
			if (Kademlia::CKademlia::isRunning() && !Kademlia::CKademlia::isConnected())
				pClientMenu->AppendODMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetKadPort()!=0) ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_BOOT, GetResString(IDS_BOOTSTRAP), theApp.LoadIcon(_T("boostrap2"), 16, 16)));


			// A4AF2
			CMenuXP *pA4AF2Menu = new CMenuXP;
			pA4AF2Menu->CreatePopupMenu();
			pA4AF2Menu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
			pA4AF2Menu->SetSelectedBarColor(RGB(242,120,114));

			if (thePrefs.IsExtControlsEnabled()) 
			{
				// ZZ:DownloadManager -->				
				if (content->type == UNAVAILABLE_SOURCE)
					pA4AF2Menu->AppendODMenu(MF_STRING,new CMenuXPText(MP_A4AF_CHECK_THIS_NOW, GetResString(IDS_A4AF_CHECK_THIS_NOW)));				
				pClientMenu->AppendODPopup(MF_STRING | MF_POPUP, pA4AF2Menu, new CMenuXPText(0, GetResString(IDS_A4AF)));
			}
			// <-- ZZ:DownloadManager

			
			//<<< [TPT] - eWombat SNAFU v2			
			if (client->IsSnafu() || client->IsSUIFailed())
			{					
				if (client->GetDownloadState()!=DS_NONE)
				{   
					pClientMenu->AppendSeparator();
					pClientMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_REMOVEDL, GetResString(IDS_REMOVEDL)));
				}
			}
			
			//>>> eWombat [SNAFU]

			pClientMenu->TrackPopupMenu(TPM_LEFTBUTTON, point.x, point.y, this);
			
			delete pA4AF2Menu;			
			delete pClientMenu;
		}
	}
	else{
		int total;
		pFileMenu->AppendODPopup(MF_STRING | MF_GRAYED, pPrioMenu, new CMenuXPText(0, GetResString(IDS_PRIORITY), theApp.LoadIcon(_T("priority"), 16, 16)));
		pFileMenu->AppendODMenu(MF_STRING | MF_GRAYED, new CMenuXPText(MP_CANCEL, GetResString(IDS_MAIN_BTN_CANCEL), theApp.LoadIcon(_T("delete"), 16, 16)));
		
		pFileMenu->AppendODMenu(MF_STRING | MF_GRAYED, new CMenuXPText(MP_PAUSE, GetResString(IDS_DL_PAUSE), theApp.LoadIcon(_T("pause"), 16, 16)));
		pFileMenu->AppendODMenu(MF_STRING | MF_GRAYED, new CMenuXPText(MP_STOP, GetResString(IDS_DL_STOP), theApp.LoadIcon(_T("stop"), 16, 16)));
		pFileMenu->AppendODMenu(MF_STRING | MF_GRAYED, new CMenuXPText(MP_RESUME, GetResString(IDS_DL_RESUME), theApp.LoadIcon(_T("start"), 16, 16)));
		pFileMenu->AppendSeparator();
		pFileMenu->AppendODMenu(MF_STRING | MF_GRAYED, new CMenuXPText(MP_OPEN, GetResString(IDS_DL_OPEN)));
		if(thePrefs.IsExtControlsEnabled() && !thePrefs.GetPreviewPrio()) {
			pFileMenu->AppendODMenu(MF_STRING | MF_GRAYED, new CMenuXPText(MP_TRY_TO_GET_PREVIEW_PARTS, GetResString(IDS_DL_TRY_TO_GET_PREVIEW_PARTS)));
			pFileMenu->CheckMenuItem(MP_TRY_TO_GET_PREVIEW_PARTS, MF_UNCHECKED);
        }
		pFileMenu->AppendODMenu(MF_STRING | MF_GRAYED, new CMenuXPText(MP_PREVIEW, GetResString(IDS_DL_PREVIEW), theApp.LoadIcon(_T("preview"), 16, 16)));
		pFileMenu->AppendODMenu(MF_STRING | MF_GRAYED, new CMenuXPText(MP_METINFO, GetResString(IDS_DL_INFO)));
		pFileMenu->AppendODMenu(MF_STRING | MF_GRAYED, new CMenuXPText(MP_VIEWFILECOMMENTS, GetResString(IDS_CMT_SHOWALL), theApp.LoadIcon(_T("comments"), 16, 16)));
		pFileMenu->AppendODMenu(MF_STRING | (GetCompleteDownloads(curTab,total) > 0 ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_CLEARCOMPLETED,GetResString(IDS_DL_CLEAR), theApp.LoadIcon(_T("clean"), 16, 16)));
		pFileMenu->AppendODPopup(MF_STRING | MF_GRAYED, pA4AFMenu, new CMenuXPText(0, GetResString(IDS_A4AF), theApp.LoadIcon(_T("permission"), 16, 16)));
		if (thePrefs.IsExtControlsEnabled())
			pFileMenu->AppendODMenu(MF_STRING | MF_GRAYED, new CMenuXPText(MP_ADDSOURCE, GetResString(IDS_ADDSRCMANUALLY)));		
		pFileMenu->AppendODMenu(MF_STRING | MF_GRAYED, new CMenuXPText(MP_SHOWED2KLINK, GetResString(IDS_DL_SHOWED2KLINK)));		
		pFileMenu->AppendODMenu(MF_STRING | (theApp.IsEd2kFileLinkInClipboard() ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_PASTE, GetResString(IDS_SW_DIRECTDOWNLOAD)));		
		pFileMenu->SetDefaultItem((UINT)-1);
		//[TPT] - SR13: Import Parts
		pFileMenu->EnableMenuItem(MP_SR13_ImportParts,MF_GRAYED);
		//m_FileMenu.EnableMenuItem(MP_SR13_InitiateRehash,MF_GRAYED);
		//[TPT] - SR13: Import Parts
		
		int iWebMenuEntries = theWebServices.GetFileMenuEntries(pWebMenu);
		UINT flag2 = (iWebMenuEntries == 0) ? MF_GRAYED : MF_STRING;
		pFileMenu->AppendODPopup(MF_STRING | MF_POPUP | flag2, pWebMenu, new CMenuXPText(0, GetResString(IDS_WEBSERVICES)));		

		pFileMenu->TrackPopupMenu(TPM_LEFTBUTTON, point.x, point.y, this);		
	}

	delete pCatMenu;
	delete pA4AFMenu;
	delete pWebMenu;
	delete pPermMenu;
	delete pPrioMenu;	
	delete pFileMenu;
}

// [TPT] - New Menu Styles END


BOOL CDownloadListCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
		case MP_PASTE:
			if (theApp.IsEd2kFileLinkInClipboard())
			theApp.PasteClipboard(curTab);
			return TRUE;
	}

	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1)
	{
		const CtrlItem_Struct* content = (CtrlItem_Struct*)GetItemData(iSel);
		if (content->type == FILE_TYPE)
		{
			//for multiple selections 
			UINT selectedCount = 0;
			CTypedPtrList<CPtrList, CPartFile*> selectedList; 
			POSITION pos = GetFirstSelectedItemPosition();
			while(pos != NULL) 
			{ 
				int index = GetNextSelectedItem(pos);
				if(index > -1) 
				{
					if (((const CtrlItem_Struct*)GetItemData(index))->type == FILE_TYPE)
					{
						selectedCount++;
						selectedList.AddTail((CPartFile*)((const CtrlItem_Struct*)GetItemData(index))->value);
					}
				} 
			} 

			CPartFile* file = (CPartFile*)content->value;
			switch (wParam)
			{
				case MPG_DELETE:
				case MP_CANCEL:
				{
					if(selectedCount > 0)
					{
						SetRedraw(false);
						CString fileList;
						bool validdelete = false;
						bool removecompl =false;
						for (pos = selectedList.GetHeadPosition(); pos != 0; )
						{
							CPartFile* cur_file = selectedList.GetNext(pos);
							if (cur_file->GetStatus() != PS_COMPLETING && cur_file->GetStatus() != PS_COMPLETE){
								validdelete = true;
								if (selectedCount<50)
									fileList.Append(_T("\n") + CString(cur_file->GetFileName()));
							}
							else if (cur_file->GetStatus() == PS_COMPLETE)
									removecompl=true;

						}
						CString quest;
						if (selectedCount==1)
							quest=GetResString(IDS_Q_CANCELDL2);
						else
							quest=GetResString(IDS_Q_CANCELDL);
						if ( (removecompl && !validdelete ) || (validdelete && AfxMessageBox(quest + fileList,MB_DEFBUTTON2 | MB_ICONQUESTION|MB_YESNO) == IDYES) )
						{
							bool bRemovedItems = false;
							while(!selectedList.IsEmpty())
							{
								HideSources(selectedList.GetHead());
								switch (selectedList.GetHead()->GetStatus())
								{
									case PS_WAITINGFORHASH:
									case PS_HASHING:
									case PS_COMPLETING:
										selectedList.RemoveHead();
										bRemovedItems = true;
										break;
									case PS_COMPLETE:
										RemoveFile(selectedList.GetHead());
										selectedList.RemoveHead();
										bRemovedItems = true;
										break;
									case PS_PAUSED:
										selectedList.GetHead()->DeleteFile();
										selectedList.RemoveHead();
										bRemovedItems = true;
										break;
									default:
										if (selectedList.GetHead()->GetCategory())
										theApp.downloadqueue->StartNextFileIfPrefs(selectedList.GetHead()->GetCategory());
										selectedList.GetHead()->DeleteFile();
										selectedList.RemoveHead();
										bRemovedItems = true;
								}
								}
							if (bRemovedItems)
							{
								AutoSelectItem();
								theApp.emuledlg->transferwnd->UpdateCatTabTitles();
							}
						}
						SetRedraw(true);
					}
					break;
				}
				case MP_PRIOHIGH:
					SetRedraw(false);
					while (!selectedList.IsEmpty()){
						CPartFile* partfile = selectedList.GetHead();
						partfile->SetAutoDownPriority(false);
						partfile->SetDownPriority(PR_HIGH);
						selectedList.RemoveHead();
					}
					SetRedraw(true);
					break;
				case MP_PRIOLOW:
					SetRedraw(false);
					while (!selectedList.IsEmpty()){
						CPartFile* partfile = selectedList.GetHead();
						partfile->SetAutoDownPriority(false);
						partfile->SetDownPriority(PR_LOW);
						selectedList.RemoveHead();
					}
					SetRedraw(true);
					break;
				case MP_PRIONORMAL:
					SetRedraw(false);
					while (!selectedList.IsEmpty()){
						CPartFile* partfile = selectedList.GetHead();
						partfile->SetAutoDownPriority(false);
						partfile->SetDownPriority(PR_NORMAL);
						selectedList.RemoveHead();
					}
					SetRedraw(true);
					break;
				case MP_PRIOAUTO:
					SetRedraw(false);
					while (!selectedList.IsEmpty()){
						CPartFile* partfile = selectedList.GetHead();
						partfile->SetAutoDownPriority(true);
						partfile->SetDownPriority(PR_HIGH);
						selectedList.RemoveHead();
					}
					SetRedraw(true);
					break;
				
				// [TPT] - Sivka AutoHL Begin
				case MP_HARD_LIMIT:
					if(selectedCount == 1)
					{
						theApp.downloadqueue->InitTempVariables(file);
						CHardLimitDlg dialog;
						dialog.DoModal();
						if(thePrefs.GetTakeOverFileSettings())
					{
							theApp.downloadqueue->UpdateFileSettings(file);
							m_SettingsSaver.SaveSettings(file);
							UpdateItem(file);
						}
					}
					else if(selectedCount > 1)
					{
						theApp.downloadqueue->InitTempVariables(selectedList.GetHead());
						CHardLimitDlg dialog;
						dialog.DoModal();

						while(!selectedList.IsEmpty()) {
							if(thePrefs.GetTakeOverFileSettings())
					{
								theApp.downloadqueue->UpdateFileSettings(selectedList.GetHead());
								m_SettingsSaver.SaveSettings(selectedList.GetHead());
								UpdateItem(selectedList.GetHead());
						}
							selectedList.RemoveHead();
						}
					}
					break;
				// [TPT] - Sivka AutoHL End

				case MP_PAUSE:
					SetRedraw(false);
					while (!selectedList.IsEmpty()){
						CPartFile* partfile = selectedList.GetHead();
						if (partfile->CanPauseFile())
							partfile->PauseFile();
						selectedList.RemoveHead();
					}
					SetRedraw(true);
					break;
				case MP_RESUME:
					SetRedraw(false);
					while (!selectedList.IsEmpty()){
						CPartFile* partfile = selectedList.GetHead();
						if (partfile->CanResumeFile()){
							if (partfile->GetStatus() == PS_INSUFFICIENT)
								partfile->ResumeFileInsufficient();
							else
								partfile->ResumeFile();
						}
						selectedList.RemoveHead();
					}
					SetRedraw(true);
					break;
				case MP_STOP:
					SetRedraw(false);
					while (!selectedList.IsEmpty()){
						CPartFile *partfile = selectedList.GetHead();
						if (partfile->CanStopFile()){
							HideSources(partfile);
							partfile->StopFile();
						}
						selectedList.RemoveHead();
					}
					SetRedraw(true);
					theApp.emuledlg->transferwnd->UpdateCatTabTitles();
					break;
				case MP_CLEARCOMPLETED:
					SetRedraw(false);
					ClearCompleted();
					SetRedraw(true);
					break;
				case MP_ALL_A4AF_AUTO:
					file->SetA4AFAuto(!file->IsA4AFAuto());
					break;
				case MPG_F2:
					if (file->GetStatus() != PS_COMPLETE && file->GetStatus() != PS_COMPLETING)
					{
						InputBox inputbox;
						CString title = GetResString(IDS_RENAME);
						title.Remove(_T('&'));
						inputbox.SetLabels(title, GetResString(IDS_DL_FILENAME), file->GetFileName());
						inputbox.SetEditFilenameMode();
						if (inputbox.DoModal()==IDOK && !inputbox.GetInput().IsEmpty() && IsValidEd2kString(inputbox.GetInput()))
						{
							file->SetFileName(inputbox.GetInput(), true);
							file->UpdateDisplayedInfo();
							file->SavePartFile();
						}
					}
					else
						MessageBeep((UINT)-1);
					break;
				case MPG_ALTENTER:
				case MP_METINFO:
					ShowFileDialog(NULL);
					break;
				case MP_COPYSELECTED:
				case MP_GETED2KLINK:{
					CString str;
					while (!selectedList.IsEmpty()){
						if (!str.IsEmpty())
							str += _T("\r\n");
						str += CreateED2kLink(selectedList.GetHead());
						selectedList.RemoveHead();
					}
					theApp.CopyTextToClipboard(str);
					break;
				}				
				// [TPT] - Save ed2klinks
				case MP_SAVEED2KLINK:					
					if(selectedCount > 1)
					{
						CString str;						
						while(!selectedList.IsEmpty()) { 
							str = CreateED2kLink(selectedList.GetHead()); 
							SaveED2KLINK(thePrefs.GetCategory(selectedList.GetHead()->GetCategory())->title, selectedList.GetHead()->GetFileName(), str);
							selectedList.RemoveHead(); 								
						}												
						break; 
					} 					
					SaveED2KLINK(thePrefs.GetCategory(file->GetCategory())->title, file->GetFileName(), CreateED2kLink(file));
					break;
				// [TPT] - Save ed2klinks
				case MP_OPEN:
					if(selectedCount > 1)
						break;
					file->OpenFile();
					break;
                case MP_TRY_TO_GET_PREVIEW_PARTS:{
					if(selectedCount > 1)
						break;
                    file->SetPreviewPrio(!file->GetPreviewPrio());
					break;
				}
				//[TPT] - SR13: impor parts
				case MP_SR13_ImportParts:
					file->SR13_ImportParts();
					break;
				//[TPT] - SR13: impor parts

				// [TPT] - SLUGFILLER: showComments
				case MP_CMT:
				{
					ShowFileDialog(NULL, IDD_COMMENT);
					break;
				}
				// [TPT] - SLUGFILLER: showComments				
				// [TPT] - xMule_MOD: showSharePermissions
				case MP_PERMNONE:
				case MP_PERMFRIENDS:
				case MP_PERMALL: {
					while(!selectedList.IsEmpty()) { 
						CPartFile *file = selectedList.GetHead();
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
						selectedList.RemoveHead();
					}
					Invalidate();
					break;
				}
				// [TPT] - xMule_MOD: showSharePermissions
				// [TPT]
				case MP_FAKECHECK1:{
					if(selectedCount == 1)
						FakeCheck1(file);
					break;
				}
				// khaos::categorymod+
				// This is only called when there is a single selection, so we'll handle it thusly.
				case MP_CAT_SETRESUMEORDER: {
					InputBox	inputOrder;
					CString		currOrder;

					currOrder.Format(_T("%u"), file->GetCatResumeOrder());
					inputOrder.SetLabels(GetResString(IDS_CAT_SETORDER), GetResString(IDS_CAT_ORDER), currOrder);
					inputOrder.SetNumber(true);
					if (inputOrder.DoModal() == IDOK)
					{
						int newOrder = inputOrder.GetInputInt();
						if  (newOrder < 0 || newOrder == file->GetCatResumeOrder()) break;

						file->SetCatResumeOrder(newOrder);
						Invalidate(); // Display the new category.
					}
					break;
				}
				case MP_PREVIEW:{
					if(selectedCount > 1)
						break;
					file->PreviewFile();
					break;
				}
				case MP_VIEWFILECOMMENTS:
					ShowFileDialog(NULL, IDD_COMMENTLST);
					break;
				case MP_SHOWED2KLINK:
					ShowFileDialog(NULL, IDD_ED2KLINK);
					break;
				case MP_ADDSOURCE:
				{
					if(selectedCount > 1)
						break;
					CAddSourceDlg as;
					as.SetFile(file);
					as.DoModal();
					break;
				}
				// These next three are only called when there are multiple selections.
				case MP_CAT_ORDERAUTOINC: {
					// This option asks the user for a starting point, and then increments each selected item
					// automatically.  It uses whatever order they appear in the list, from top to bottom.
					InputBox	inputOrder;
					if (selectedCount <= 1) break;
						
					inputOrder.SetLabels(GetResString(IDS_CAT_SETORDER), GetResString(IDS_CAT_EXPAUTOINC), _T("0"));
					inputOrder.SetNumber(true);
                    if (inputOrder.DoModal() == IDOK)
					{
						int newOrder = inputOrder.GetInputInt();
						if  (newOrder < 0) break;

						while (!selectedList.IsEmpty()) {
							selectedList.GetHead()->SetCatResumeOrder(newOrder);
							newOrder++;
							selectedList.RemoveHead();
						}
						Invalidate();
					}
					break;
				}
				case MP_CAT_ORDERSTEPTHRU: {
					// This option asks the user for a different resume modifier for each file.  It
					// displays the filename in the inputbox so that they don't get confused about
					// which one they're setting at any given moment.
					InputBox	inputOrder;
					CString		currOrder;
					CString		currFile;
					CString		currInstructions;
					int			newOrder = 0;

					if (selectedCount <= 1) break;
					inputOrder.SetNumber(true);

					while (!selectedList.IsEmpty()) {
						currOrder.Format(_T("%u"), selectedList.GetHead()->GetCatResumeOrder());
						currFile = selectedList.GetHead()->GetFileName();
                        if (currFile.GetLength() > 50) currFile = currFile.Mid(0,47) + _T("...");
						currInstructions.Format(_T("%s %s"), GetResString(IDS_CAT_EXPSTEPTHRU), currFile);
						inputOrder.SetLabels(GetResString(IDS_CAT_SETORDER), currInstructions, currOrder);
												
						if (inputOrder.DoModal() == IDCANCEL) {
							if (MessageBox(GetResString(IDS_CAT_ABORTSTEPTHRU), GetResString(IDS_ABORT), MB_YESNO) == IDYES) {
								break;
							}
							else {
								selectedList.RemoveHead();
								continue;
							}
						}

						newOrder = inputOrder.GetInputInt();
						selectedList.GetHead()->SetCatResumeOrder(newOrder);
						selectedList.RemoveHead();
					}
					RedrawItems(0, GetItemCount() - 1);
					break;
				}
				case MP_CAT_ORDERALLSAME: {
					// This option asks the user for a single resume modifier and applies it to
					// all the selected files.
					InputBox	inputOrder;
					CString		currOrder;

					if (selectedCount <= 1) break;

					inputOrder.SetLabels(GetResString(IDS_CAT_SETORDER), GetResString(IDS_CAT_EXPALLSAME), _T("0"));
					inputOrder.SetNumber(true);
					if (inputOrder.DoModal() == IDCANCEL)
						break;

					int newOrder = inputOrder.GetInputInt();
					if  (newOrder < 0) break;

					while (!selectedList.IsEmpty()) {
						selectedList.GetHead()->SetCatResumeOrder(newOrder);
						selectedList.RemoveHead();
					}
					RedrawItems(0, GetItemCount() - 1);
					break;
				}


				// khaos::categorymod-
					

				default:
					if (wParam>=MP_WEBURL && wParam<=MP_WEBURL+99){
						theWebServices.RunURL(file, wParam);
					}
					else if (wParam>=MP_ASSIGNCAT && wParam<=MP_ASSIGNCAT+99){
						SetRedraw(FALSE);
						while (!selectedList.IsEmpty()){
							CPartFile *partfile = selectedList.GetHead();
							partfile->SetCategory(wParam - MP_ASSIGNCAT);
							partfile->UpdateDisplayedInfo(true);
							selectedList.RemoveHead();
						}
						SetRedraw(TRUE);
						UpdateCurrentCategoryView();
						if (thePrefs.ShowCatTabInfos())
							theApp.emuledlg->transferwnd->UpdateCatTabTitles();
					}
					else if (wParam>=MP_PREVIEW_APP_MIN && wParam<=MP_PREVIEW_APP_MAX){
						thePreviewApps.RunApp(file, wParam);
					}
					break;
			}
		}
		else{
			CUpDownClient* client = (CUpDownClient*)content->value;
			CPartFile* file = (CPartFile*)content->owner; // added by sivka

			switch (wParam){
				case MP_SHOWLIST:
					ShowClientDialog(client, IDD_BROWSEFILES);	// itsonlyme: viewSharedFiles
					break;
				case MP_MESSAGE:
					theApp.emuledlg->chatwnd->StartSession(client);
					break;
				case MP_ADDFRIEND:
					if (theApp.friendlist->AddFriend(client))
						UpdateItem(client);
					break;
				case MPG_ALTENTER:
				case MP_DETAIL:
					ShowClientDialog(client);
					break;
				case MP_BOOT:
					if (client->GetKadPort())
						Kademlia::CKademlia::bootstrap(ntohl(client->GetIP()), client->GetKadPort());
					break;
// ZZ:DownloadManager -->
				case MP_A4AF_CHECK_THIS_NOW:
					if (file->GetStatus(false) == PS_READY || file->GetStatus(false) == PS_EMPTY)
					{
						if (client->GetDownloadState() != DS_DOWNLOADING)
						{
							client->SwapToAnotherFile(_T("Manual init of source check. Test to be like ProcessA4AFClients(). CDownloadListCtrl::OnCommand() MP_SWAP_A4AF_DEBUG_THIS"), false, false, false, NULL, true, true, true); // ZZ:DownloadManager
							UpdateItem(file);
						}
					}
					break;
// <-- ZZ:DownloadManager
				// [TPT] - itsonlyme:reqFiles START
				case MP_LIST_REQUESTED_FILES: {
					ShowClientDialog(client, IDD_REQFILES);
					break;
				}
				// [TPT] - itsonlyme:reqFiles END
				// [TPT] - eWombat SNAFU v2
				case MP_REMOVEDL:
					if (client!=NULL)
					{
						if (thePrefs.GetVerbose())
							AddDebugLogLine(_T("S.N.A.F.U. %s kicked from downloadqueue"), client->GetUserName());
						if (client->GetDownloadState()!=DS_DOWNLOADING)
						{
							theApp.downloadqueue->RemoveSource(client);
							if (!client->socket)
								client->Disconnected(_T("Remove from downloadqueue due to S.N.A.F.U"), false, CUpDownClient::USR_SNAFU); // [TPT] - Maella -Upload Stop Reason-
						}
					}
					break;

			}
		}
	}
	else /*nothing selected*/
	{
		switch (wParam){
			case MP_CLEARCOMPLETED:
				ClearCompleted();
				break;
		}
	}

	return TRUE;
}

// [TPT] - Save ed2klinks
void CDownloadListCtrl::SaveED2KLINK(CString category, CString fullname, CString ed2klink)
{		
	CString name = thePrefs.GetLinkDir() + category;
	::CreateDirectory(thePrefs.GetLinkDir() + category, 0);
	name += _T("\\") + fullname + _T(".url");
	CStdioFile file;// no buffering needed here since we swap out the entire array
	CFileException fexp;	
	if (!file.Open(name, CFile::modeWrite|CFile::modeCreate|CFile::typeBinary, &fexp)){
		CString strError(_T("Error creating a file with ed2klink: "));		
		strError += ed2klink;
		AddLogLine(true, strError);
		return;
	}
	file.WriteString(_T("[InternetShortcut]"));
	file.WriteString(_T("\n"));
	file.WriteString(_T("URL="));
	file.WriteString(ed2klink);
	file.Close();
}
// [TPT] - Save ed2klinks

void CDownloadListCtrl::OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult){
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// Barry - Store sort order in preferences
	// Determine ascending based on whether already sorted on this column
	int sortItem = thePrefs.GetColumnSortItem(CPreferences::tableDownload);
	bool m_oldSortAscending = thePrefs.GetColumnSortAscending(CPreferences::tableDownload);
	// [TPT] - SLUGFILLER: DLsortFix
	int userSort = (GetAsyncKeyState(VK_CONTROL) < 0) ? 0x4000:0;	// Ctrl sorts sources only

	if (pNMListView->iSubItem + userSort==9) {
		m_bRemainSort=(sortItem != 9) ? false : (!m_oldSortAscending?m_bRemainSort:!m_bRemainSort);
	}

	bool sortAscending = (sortItem != pNMListView->iSubItem + userSort) ? (pNMListView->iSubItem == 0) : !m_oldSortAscending;	// descending by default for all but filename/username
	
	// Item is column clicked
	sortItem = pNMListView->iSubItem + userSort;
	// [TPT] - SLUGFILLER: DLsortFix
	
	// Save new preferences
	thePrefs.SetColumnSortItem(CPreferences::tableDownload, sortItem);
	thePrefs.SetColumnSortAscending(CPreferences::tableDownload, sortAscending);
	thePrefs.TransferlistRemainSortStyle(m_bRemainSort);
	
	// Sort table
	uint8 adder=0;
	if (sortItem!=9 || !m_bRemainSort)
		SetSortArrow(sortItem & 0xCFFF, sortAscending);	// [TPT] - SLUGFILLER: DLsortFix
	else {
        SetSortArrow(sortItem, sortAscending?arrowDoubleUp : arrowDoubleDown);
		adder=81;
	}
	

	SortItems(SortProc, sortItem + (sortAscending ? 0:100) + adder );

	*pResult = 0;
}

int CDownloadListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort){
	CtrlItem_Struct* item1 = (CtrlItem_Struct*)lParam1;
	CtrlItem_Struct* item2 = (CtrlItem_Struct*)lParam2;

	// [TPT] - SLUGFILLER: multiSort remove - already handled

	int sortMod = 1;
	if((lParamSort & 0x3FFF) >= 100) {	// [TPT] - SLUGFILLER: DLsortFix
		sortMod = -1;
		lParamSort -= 100;
	}

	int comp;

	if(item1->type == FILE_TYPE && item2->type != FILE_TYPE) {
		if(item1->value == item2->parent->value)
			return -1;

		comp = Compare((CPartFile*)item1->value, (CPartFile*)(item2->parent->value), lParamSort);

	} else if(item2->type == FILE_TYPE && item1->type != FILE_TYPE) {
		if(item1->parent->value == item2->value)
			return 1;

		comp = Compare((CPartFile*)(item1->parent->value), (CPartFile*)item2->value, lParamSort);

	} else if (item1->type == FILE_TYPE) {
		CPartFile* file1 = (CPartFile*)item1->value;
		CPartFile* file2 = (CPartFile*)item2->value;

		comp = Compare(file1, file2, lParamSort);

	} else {
		// [TPT] - SLUGFILLER: DLsortFix - never compare sources of different files
		if (item1->parent->value != item2->parent->value)
			return sortMod * Compare((CPartFile*)(item1->parent->value), (CPartFile*)(item2->parent->value), lParamSort);
		// [TPT] - SLUGFILLER: DLsortFix
		if (item1->type != item2->type)
			return item1->type - item2->type;

		CUpDownClient* client1 = (CUpDownClient*)item1->value;
		CUpDownClient* client2 = (CUpDownClient*)item2->value;
		comp = Compare(client1, client2, lParamSort,sortMod);
	}

	// [TPT] - SLUGFILLER: multiSort remove - already handled

	return sortMod * comp;
}

void CDownloadListCtrl::ClearCompleted(bool ignorecats){
	// Search for completed file(s)
	for(ListItems::iterator it = m_ListItems.begin(); it != m_ListItems.end(); ){
		CtrlItem_Struct* cur_item = it->second;
		it++; // Already point to the next iterator. 
		if(cur_item->type == FILE_TYPE){
			CPartFile* file = reinterpret_cast<CPartFile*>(cur_item->value);
			// [TPT] - Patch
			if(file && file->IsPartFile() == false && (file->CheckShowItemInGivenCat(curTab) || ignorecats) ){
				if (RemoveFile(file))
					it = m_ListItems.begin();
			}
		}
	}
	if (thePrefs.ShowCatTabInfos())
		theApp.emuledlg->transferwnd->UpdateCatTabTitles();
}

void CDownloadListCtrl::ClearCompleted(const CPartFile* pFile)
{
	if (!pFile->IsPartFile())
	{
		for (ListItems::iterator it = m_ListItems.begin(); it != m_ListItems.end(); )
		{
			CtrlItem_Struct* cur_item = it->second;
			it++;
			if (cur_item->type == FILE_TYPE)
			{
				const CPartFile* pCurFile = reinterpret_cast<CPartFile*>(cur_item->value);
				if (pCurFile == pFile)
				{
					RemoveFile(pCurFile);
					return;
				}
			}
		}
	}
}

void CDownloadListCtrl::SetStyle() {
	if (thePrefs.IsDoubleClickEnabled())
		SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
	else
		SetExtendedStyle(LVS_EX_ONECLICKACTIVATE | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
	//ika: don´t move to init() because we can forget the style go into preferences
	SetDoubleBufferStyle();//[TPT] - Double buffer stlye in lists
}

void CDownloadListCtrl::OnListModified(NMHDR *pNMHDR, LRESULT *pResult) {
	NM_LISTVIEW *pNMListView = (NM_LISTVIEW*)pNMHDR;

	//this works because true is equal to 1 and false equal to 0
	BOOL notLast = pNMListView->iItem + 1 != GetItemCount();
	BOOL notFirst = pNMListView->iItem != 0;
	RedrawItems(pNMListView->iItem - notFirst, pNMListView->iItem + notLast);
}

int CDownloadListCtrl::Compare(const CPartFile* file1, const CPartFile* file2, LPARAM lParamSort)
{
	// [TPT] - SLUGFILLER: DLsortFix
	if (lParamSort & 0x4000)
		return 0;
	// [TPT] - SLUGFILLER: DLsortFix
	int comp=0;
	switch(lParamSort)
	{
	case 0: //filename asc
			comp=CompareLocaleStringNoCase(file1->GetFileName(),file2->GetFileName());
			break;
	case 1: //size asc
		comp=CompareUnsigned(file1->GetFileSize(), file2->GetFileSize());
		break;
	case 2: //transferred asc
		comp=CompareUnsigned(file1->GetTransferred(), file2->GetTransferred());
		break;
	case 3: //completed asc
		comp=CompareUnsigned(file1->GetCompletedSize(), file2->GetCompletedSize());
		break;
	case 4: //speed asc
		comp=CompareUnsigned(file1->GetDownloadFileDatarate(), file2->GetDownloadFileDatarate());
		break;
	case 5: //progress asc
		comp = CompareFloat(file1->GetPercentCompleted(), file2->GetPercentCompleted());
		break;
	case 6: //sources asc
		comp=CompareUnsigned(file1->GetSourceCount(), file2->GetSourceCount());
		break;
	case 7: //priority asc
		comp=CompareUnsigned(file1->GetDownPriority(), file2->GetDownPriority());
		break;
	case 8: //Status asc 
		comp=CompareUnsigned(file2->getPartfileStatusRang(),file1->getPartfileStatusRang());
	break;
	case 9: //Remaining Time asc
	{
		//Make ascending sort so we can have the smaller remaining time on the top 
		//instead of unknowns so we can see which files are about to finish better..
			time_t f1 = file1->getTimeRemaining();
			time_t f2 = file2->getTimeRemaining();
		//Same, do nothing.
		if( f1 == f2 )
			{
				comp=0;
				break;
			}
		//If descending, put first on top as it is unknown
		//If ascending, put first on bottom as it is unknown
		if( f1 == -1 )
			{
				comp=1;
				break;
			}
			//If descending, put second on top as it is unknown
		//If ascending, put second on bottom as it is unknown
		if( f2 == -1 )
			{
				comp=-1;
				break;
			}
			//If descending, put first on top as it is bigger.
		//If ascending, put first on bottom as it is bigger.
			comp = CompareUnsigned(f1, f2);
			break;
	}
	case 90: //Remaining SIZE asc 
		comp = CompareUnsigned(file1->GetFileSize()-file1->GetCompletedSize(), file2->GetFileSize()-file2->GetCompletedSize());
		break;
	case 10: //last seen complete asc 
		if (file1->lastseencomplete > file2->lastseencomplete)
			comp=1;
		else if(file1->lastseencomplete < file2->lastseencomplete)
			comp=-1;
		else
			comp=0;
		break;
	case 11: //last received Time asc 
		if (file1->GetFileDate() > file2->GetFileDate())
			comp=1;
		else if(file1->GetFileDate() < file2->GetFileDate())
			comp=-1;
		else
			comp=0;
		break;
		// khaos::categorymod+
	case 12: // Cat
		if (const_cast<CPartFile*>(file1)->GetCategory() > const_cast<CPartFile*>(file2)->GetCategory())
			comp=1;
		else if (const_cast<CPartFile*>(file1)->GetCategory() < const_cast<CPartFile*>(file2)->GetCategory())
			comp=-1;
		else
			comp=0;
		break;
	case 13: // Mod
			if (const_cast<CPartFile*>(file1)->GetCatResumeOrder() > const_cast<CPartFile*>(file2)->GetCatResumeOrder())
			comp=1;
		else if (const_cast<CPartFile*>(file1)->GetCatResumeOrder() < const_cast<CPartFile*>(file2)->GetCatResumeOrder())
			comp=-1;
		else
			comp=0;
		break;
	// [TPT] - khaos::categorymod-
// [TPT] - WebCache ////////////////////////////////////////////////////////////////////////////////////
	//JP Webcache	
	case 14:
		comp = file1->GetWebcacheSourceCount() - file2->GetWebcacheSourceCount();
		break;

	// [TPT] - SLUGFILLER: DLsortFix
	case 0x8000:	// met file name asc, only available as last-resort sort to make sure no two files are equal
		comp=CompareLocaleStringNoCase(file1->GetPartMetFileName(), file2->GetPartMetFileName());
		break;	
	// [TPT] - SLUGFILLER: DLsortFix
	default:
			comp=0;
	}
	return comp;
}

int CDownloadListCtrl::Compare(const CUpDownClient *client1, const CUpDownClient *client2, LPARAM lParamSort, int sortMod)
{
	lParamSort &= 0xBFFF;	// [TPT] - SLUGFILLER: DLsortFix
	switch(lParamSort){
	case 0: //name asc
		if(client1->GetUserName() == client2->GetUserName())
			return 0;
		else if(!client1->GetUserName())
			return 1;
		else if(!client2->GetUserName())
			return -1;
		return CompareLocaleStringNoCase(client1->GetUserName(),client2->GetUserName());

	case 1: //size but we use status asc
		return Compare(client1, client2, 8, sortMod); // [TPT] - Sort by QR
	case 2://transferred asc
		if (!client1->Credits())
			return 1;
		else if (!client2->Credits())
			return -1;
		return client2->Credits()->GetDownloadedTotal() - client1->Credits()->GetDownloadedTotal();
	case 3://completed asc
		return CompareUnsigned(client1->GetTransferredDown(), client2->GetTransferredDown());

	case 4: //speed asc
		return CompareUnsigned(client1->GetDownloadDatarate(), client2->GetDownloadDatarate());

	case 5: //progress asc
		return CompareUnsigned(client1->GetAvailablePartCount(), client2->GetAvailablePartCount());

	// [TPT] - Sort. MORPH - Added by Yun.SF3, Maella -Support for tag ET_MOD_VERSION 0x55 II-
	// Maella -Support for tag ET_MOD_VERSION 0x55-
	case 6:
		if( client1->GetClientSoft() == client2->GetClientSoft() )
			if(client2->GetVersion() == client1->GetVersion() && client1->GetClientSoft() == SO_EMULE){
				return client2->DbgGetFullClientSoftVer().CompareNoCase( client1->DbgGetFullClientSoftVer());
			}
			else {
				return client2->GetVersion() - client1->GetVersion();
			}
		else
			return client1->GetClientSoft() - client2->GetClientSoft();
	// Maella end
	//MORPH - Added by Yun.SF3, Maella -Support for tag ET_MOD_VERSION 0x55 II-

	case 7: {//priority asc
		uint32 lastAskedTime1 = client1->GetLastAskedTime();
		uint32 lastAskedTime2 = client2->GetLastAskedTime();
		if ( lastAskedTime1 != 0){
			if ( lastAskedTime2 != 0){
				if (lastAskedTime1 > lastAskedTime2){
					return 1;
				} else if (lastAskedTime1 < lastAskedTime2) {
					return -1;
				} else {
					return 0;
				}
			}
			return 1;
		}
		else
			return (lastAskedTime2 == 0) ? -1 : 0;
	}
		case 8: {//Status asc
			EDownloadState clientState1 = client1->GetDownloadState();
			EDownloadState clientState2 = client2->GetDownloadState();

			if ( clientState1 == DS_DOWNLOADING ){
				if ( clientState2 == DS_DOWNLOADING) {
					return CompareUnsigned(client1->GetDownloadDatarate(), client2->GetDownloadDatarate());
				}
				return 1;
			} else if ( clientState2 == DS_DOWNLOADING) {
				return -1;
			}

			if ( clientState1 == DS_ONQUEUE )
			{
				if ( clientState2 == DS_ONQUEUE ) {
					if ( client1->IsRemoteQueueFull() ){
						return (client2->IsRemoteQueueFull()) ? 0 : -1;
					}
					else if ( client2->IsRemoteQueueFull() ){
						return 1;
					}

					if ( client1->GetRemoteQueueRank() ){
						return (client2->GetRemoteQueueRank()) ? client2->GetRemoteQueueRank() - client1->GetRemoteQueueRank() : 1;
					}
					return (client2->GetRemoteQueueRank()) ? -1 : 0;
				}
				return 1;
			} else if ( clientState2 == DS_ONQUEUE ){
				return -1;
			}

			if ( clientState1 == DS_NONEEDEDPARTS && clientState2 != DS_NONEEDEDPARTS)
				return 1;

			if ( clientState1 == DS_TOOMANYCONNS && clientState2 != DS_TOOMANYCONNS)
				return -1;

			return 0;
		}

	case 13:
		if(client1->GetCountryIP() && client2->GetCountryIP())
			return theApp.ip2country->Compare(client1->GetCountryIP(), client2->GetCountryIP());
		else if(client1->GetCountryIP())
			return 1;
		else
			return -1;
	// [TPT] - IP Country
// [TPT] - WebCache ////////////////////////////////////////////////////////////////////////////////////
	//JP Webcache START 	
	case 14:
		if (client1->SupportsWebCache() && client2->SupportsWebCache() )
			return CompareLocaleStringNoCase(client1->GetWebCacheName(),client2->GetWebCacheName());
		else
			return client1->SupportsWebCache() - client2->SupportsWebCache();
	//JP Webcache END	
	default:
		return 0;
	}
}

void CDownloadListCtrl::OnNMDblclkDownloadlist(NMHDR *pNMHDR, LRESULT *pResult)
{
	int iSel = GetSelectionMark();
	if (iSel != -1)
	{
		const CtrlItem_Struct* content = (CtrlItem_Struct*)GetItemData(iSel);
		if (content && content->value)
		{
			if (content->type == FILE_TYPE)
			{
				if (!thePrefs.IsDoubleClickEnabled())
				{
					CPoint pt;
					::GetCursorPos(&pt);
					ScreenToClient(&pt);
					LVHITTESTINFO hit;
					hit.pt = pt;
					if (HitTest(&hit) >= 0 && (hit.flags & LVHT_ONITEM))
					{
						LVHITTESTINFO subhit;
						subhit.pt = pt;
						if (SubItemHitTest(&subhit) >= 0 && subhit.iSubItem == 0)
						{
							CPartFile* file = (CPartFile*)content->value;
							// [TPT] - itsonlyme: displayOptions START
							if (!thePrefs.ShowFileSystemIcon())
								pt.x += theApp.GetSmallSytemIconSize().cx;
							// [TPT] - itsonlyme: displayOptions END
							// [TPT] - SLUGFILLER: showComments
							if (thePrefs.ShowLocalRating() 
								&& (!file->GetFileComment().IsEmpty() || file->GetFileRating())) {
								if (pt.x >= FILE_ITEM_MARGIN_X+theApp.GetSmallSytemIconSize().cx
									&& pt.x <= FILE_ITEM_MARGIN_X+theApp.GetSmallSytemIconSize().cx+16)
									ShowFileDialog(file, IDD_COMMENT);
								else
									pt.x -= 16 + 2;	// measure rating, if available, as second icon
							}
							else if (thePrefs.ShowRatingIndicator() 
							// [TPT] - SLUGFILLER: showComments
								&& (file->HasComment() || file->HasRating()) 
								&& pt.x >= FILE_ITEM_MARGIN_X+theApp.GetSmallSytemIconSize().cx 
								&& pt.x <= FILE_ITEM_MARGIN_X+theApp.GetSmallSytemIconSize().cx+RATING_ICON_WIDTH)
								ShowFileDialog(file, IDD_COMMENTLST);
							else if (thePrefs.GetPreviewOnIconDblClk()
									 && pt.x >= FILE_ITEM_MARGIN_X 
									 && pt.x < FILE_ITEM_MARGIN_X+theApp.GetSmallSytemIconSize().cx) {
								if (file->IsReadyForPreview())
									file->PreviewFile();
							else
									MessageBeep((UINT)-1);
							}
							else
								ShowFileDialog(file);
						}
					}
				}
			}
			else
			{
				ShowClientDialog((CUpDownClient*)content->value);
			}
		}
	}
	
	*pResult = 0;
}

/*
void CDownloadListCtrl::CreateMenues() {
	if (m_PrioMenu) VERIFY( m_PrioMenu.DestroyMenu() );
	if (m_A4AFMenu) VERIFY( m_A4AFMenu.DestroyMenu() );
	if (m_FileMenu) VERIFY( m_FileMenu.DestroyMenu() );

	m_PrioMenu.CreateMenu();
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOLOW,GetResString(IDS_PRIOLOW));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIONORMAL,GetResString(IDS_PRIONORMAL));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOHIGH, GetResString(IDS_PRIOHIGH));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOAUTO, GetResString(IDS_PRIOAUTO));

	m_A4AFMenu.CreateMenu();
// ZZ:DownloadManager -->
	//m_A4AFMenu.AppendMenu(MF_STRING, MP_ALL_A4AF_TO_THIS, GetResString(IDS_ALL_A4AF_TO_THIS)); // sivka [Tarod]
	//m_A4AFMenu.AppendMenu(MF_STRING, MP_ALL_A4AF_TO_OTHER, GetResString(IDS_ALL_A4AF_TO_OTHER)); // sivka
// <-- ZZ:DownloadManager
	m_A4AFMenu.AppendMenu(MF_STRING, MP_ALL_A4AF_AUTO, GetResString(IDS_ALL_A4AF_AUTO)); // sivka [Tarod]


	m_FileMenu.CreatePopupMenu();
	m_FileMenu.AddMenuTitle(GetResString(IDS_DOWNLOADMENUTITLE));
	m_FileMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_PrioMenu.m_hMenu, GetResString(IDS_PRIORITY) + _T(" (") + GetResString(IDS_DOWNLOAD) + _T(")"));

	m_FileMenu.AppendMenu(MF_STRING,MP_PAUSE, GetResString(IDS_DL_PAUSE));
	m_FileMenu.AppendMenu(MF_STRING,MP_STOP, GetResString(IDS_DL_STOP));
	m_FileMenu.AppendMenu(MF_STRING,MP_RESUME, GetResString(IDS_DL_RESUME));
	m_FileMenu.AppendMenu(MF_STRING,MP_CANCEL,GetResString(IDS_MAIN_BTN_CANCEL) );
	m_FileMenu.AppendMenu(MF_SEPARATOR);
	m_FileMenu.AppendMenu(MF_STRING,MP_OPEN, GetResString(IDS_DL_OPEN) );//<--9/21/02
	m_FileMenu.AppendMenu(MF_STRING,MP_PREVIEW, GetResString(IDS_DL_PREVIEW) );
	m_FileMenu.AppendMenu(MF_STRING,MP_METINFO, GetResString(IDS_DL_INFO) );//<--9/21/02
	m_FileMenu.AppendMenu(MF_STRING,MP_VIEWFILECOMMENTS, GetResString(IDS_CMT_SHOWALL) );

	m_FileMenu.AppendMenu(MF_SEPARATOR);
	m_FileMenu.AppendMenu(MF_STRING,MP_CLEARCOMPLETED, GetResString(IDS_DL_CLEAR));
	if (thePrefs.IsExtControlsEnabled())
		m_FileMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_A4AFMenu.m_hMenu, GetResString(IDS_A4AF));

	m_FileMenu.AppendMenu(MF_SEPARATOR);
	m_FileMenu.AppendMenu(MF_STRING,MP_GETED2KLINK, GetResString(IDS_DL_LINK1) );
	m_FileMenu.AppendMenu(MF_STRING,MP_GETHTMLED2KLINK, GetResString(IDS_DL_LINK2));
	m_FileMenu.AppendMenu(MF_STRING,MP_PASTE, GetResString(IDS_SW_DIRECTDOWNLOAD));
	m_FileMenu.AppendMenu(MF_SEPARATOR);
}
*/
CString CDownloadListCtrl::getTextList()
{
	CString out;

	for (ListItems::iterator it = m_ListItems.begin(); it != m_ListItems.end(); it++)
	{
		const CtrlItem_Struct* cur_item = it->second;
		if (cur_item->type == FILE_TYPE)
		{
			const CPartFile* file = reinterpret_cast<CPartFile*>(cur_item->value);

			CString temp;
			temp.Format(_T("\n%s\t [%.1f%%] %i/%i - %s"),
						file->GetFileName(),
						file->GetPercentCompleted(),
						file->GetTransferringSrcCount(),
						file->GetSourceCount(), 
						file->getPartfileStatus());
			
			out += temp;
		}
	}

	return out;
}

// [TPT] - khaos::categorymod+
void CDownloadListCtrl::ShowFilesCount() 
{
	// [TPT] - TransferWindow Fix
	if (!theApp.emuledlg->transferwnd->isDownloadListVisible())
		return;
	// [TPT] - TransferWindow Fix

	uint16	count=0;
	uint16	totcnt=0;
	CString counter;
	CtrlItem_Struct* cur_item;

	for(ListItems::const_iterator it = m_ListItems.begin(); it != m_ListItems.end(); it++)
	{
		cur_item = it->second;
		if (cur_item->type == FILE_TYPE)
		{
			CPartFile* file=(CPartFile*)cur_item->value;
			if (file->CheckShowItemInGivenCat(curTab))
				++count;
			++totcnt;
		}
	}

	if (thePrefs.GetCategory(curTab))
		counter.Format(_T("%s: %u (%u Total | %s)"), GetResString(IDS_TW_DOWNLOADS),count,totcnt,thePrefs.GetCategory(curTab)->viewfilters.bSuspendFilters ? GetResString(IDS_CAT_FILTERSSUSP) : GetResString(IDS_CAT_FILTERSACTIVE));
	theApp.emuledlg->transferwnd->GetDlgItem(IDC_DOWNLOAD_TEXT)->SetWindowText(counter);
}
// [TPT] - khaos::categorymod-

void CDownloadListCtrl::ShowSelectedFileDetails()
{
	POINT point;
	::GetCursorPos(&point);
	CPoint pt = point; 
    ScreenToClient(&pt); 
    int it = HitTest(pt);
    if (it == -1)
		return;

	SetItemState(-1, 0, LVIS_SELECTED);
	SetItemState(it, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	SetSelectionMark(it);   // display selection mark correctly! 

	CtrlItem_Struct* content = (CtrlItem_Struct*)GetItemData(GetSelectionMark());
	if (content->type == FILE_TYPE)
	{
		CPartFile* file = (CPartFile*)content->value;

		// [TPT] - itsonlyme: displayOptions START
		if (!thePrefs.ShowFileSystemIcon())
			pt.x += theApp.GetSmallSytemIconSize().cx;
		// [TPT] - itsonlyme: displayOptions END
		// [TPT] - SLUGFILLER: showComments
		if (thePrefs.ShowLocalRating() 
			&& (!file->GetFileComment().IsEmpty() || file->GetFileRating())) {
			if (pt.x >= FILE_ITEM_MARGIN_X+theApp.GetSmallSytemIconSize().cx
				&& pt.x <= FILE_ITEM_MARGIN_X+theApp.GetSmallSytemIconSize().cx+16)
				ShowFileDialog(file, IDD_COMMENT);
			else
				pt.x -= 16 + 2;	// measure rating, if available, as second icon
		}
		else if (thePrefs.ShowRatingIndicator() 
		// [TPT] - SLUGFILLER: showComments
			&& (file->HasComment() || file->HasRating()) 
			&& pt.x >= FILE_ITEM_MARGIN_X+theApp.GetSmallSytemIconSize().cx 
			&& pt.x <= FILE_ITEM_MARGIN_X+theApp.GetSmallSytemIconSize().cx+RATING_ICON_WIDTH)
			ShowFileDialog(file, IDD_COMMENTLST);
		else
			ShowFileDialog(NULL);
	}
	else
	{
		ShowClientDialog((CUpDownClient*)content->value);
	}
}

int CDownloadListCtrl::GetCompleteDownloads(int cat, int &total){
	int count=0;
	total=0;

	for(ListItems::const_iterator it = m_ListItems.begin(); it != m_ListItems.end(); it++){
		const CtrlItem_Struct* cur_item = it->second;
		if (cur_item->type == FILE_TYPE){
			CPartFile* file = reinterpret_cast<CPartFile*>(cur_item->value);
			
			if ( file->CheckShowItemInGivenCat(cat)) {
				++total;

				if (file->GetStatus()==PS_COMPLETE  )
					++count;
			}
		}
	}

	return count;
}

void CDownloadListCtrl::UpdateCurrentCategoryView(){
	ChangeCategory(curTab);
}

void CDownloadListCtrl::UpdateCurrentCategoryView(CPartFile* thisfile) {

	ListItems::const_iterator it = m_ListItems.find(thisfile);
	if (it != m_ListItems.end()) {
		const CtrlItem_Struct* cur_item = it->second;
		if (cur_item->type == FILE_TYPE){
			CPartFile* file = reinterpret_cast<CPartFile*>(cur_item->value);
			
			if (!file->CheckShowItemInGivenCat(curTab))
				HideFile(file);
			else
				ShowFile(file);
		}
	}

}

void CDownloadListCtrl::ChangeCategory(int newsel){

	SetRedraw(FALSE);

	// remove all displayed files with a different cat and show the correct ones
	for(ListItems::const_iterator it = m_ListItems.begin(); it != m_ListItems.end(); it++){
		const CtrlItem_Struct* cur_item = it->second;
		if (cur_item->type == FILE_TYPE){
			CPartFile* file = reinterpret_cast<CPartFile*>(cur_item->value);
			
			if (!file->CheckShowItemInGivenCat(newsel))
				HideFile(file);
			else
				ShowFile(file);
		}
	}

	SetRedraw(TRUE);
	curTab=newsel;
	ShowFilesCount();
}

void CDownloadListCtrl::HideFile(CPartFile* tohide)
{
	HideSources(tohide);

	// Retrieve all entries matching the source
	std::pair<ListItems::const_iterator, ListItems::const_iterator> rangeIt = m_ListItems.equal_range(tohide);
	for(ListItems::const_iterator it = rangeIt.first; it != rangeIt.second; it++){
		CtrlItem_Struct* updateItem  = it->second;

		// Find entry in CListCtrl and update object
 		LVFINDINFO find;
		find.flags = LVFI_PARAM;
		find.lParam = (LPARAM)updateItem;
		sint16 result = FindItem(&find);
		if(result != (-1)) {
			DeleteItem(result);
			return;
		}
	}
}

void CDownloadListCtrl::ShowFile(CPartFile* toshow){
	// Retrieve all entries matching the source
	std::pair<ListItems::const_iterator, ListItems::const_iterator> rangeIt = m_ListItems.equal_range(toshow);
	ListItems::const_iterator it = rangeIt.first;
	if(it != rangeIt.second){
		CtrlItem_Struct* updateItem  = it->second;

		// Check if entry is already in the List
 		LVFINDINFO find;
		find.flags = LVFI_PARAM;
		find.lParam = (LPARAM)updateItem;
		sint16 result = FindItem(&find);
		if(result == (-1))
			InsertItem(LVIF_PARAM|LVIF_TEXT,GetItemCount(),LPSTR_TEXTCALLBACK,0,0,0,(LPARAM)updateItem);
	}
}

void CDownloadListCtrl::GetDisplayedFiles(CArray<CPartFile*,CPartFile*> *list){
	for(ListItems::iterator it = m_ListItems.begin(); it != m_ListItems.end(); ){
		CtrlItem_Struct* cur_item = it->second;
		it++; // Already point to the next iterator. 
		if(cur_item->type == FILE_TYPE){
			CPartFile* file = reinterpret_cast<CPartFile*>(cur_item->value);
			list->Add(file);
		}
	}	
}

void CDownloadListCtrl::MoveCompletedfilesCat(uint8 from, uint8 to){
	uint8 mycat;

	for(ListItems::iterator it = m_ListItems.begin(); it != m_ListItems.end(); ){
		CtrlItem_Struct* cur_item = it->second;
		it++; // Already point to the next iterator.
		if(cur_item->type == FILE_TYPE){
			CPartFile* file = reinterpret_cast<CPartFile*>(cur_item->value);
			if (!file->IsPartFile()){
				mycat=file->GetCategory();
				if ( mycat>=min(from,to) && mycat<=max(from,to)) {
					if (mycat==from) 
						file->SetCategory(to); 
					else
						if (from<to) file->SetCategory(mycat-1);
							else file->SetCategory(mycat+1);
				}
			}
		}
	}
}

void CDownloadListCtrl::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
    NMLVDISPINFO* pDispInfo = (NMLVDISPINFO*)pNMHDR;
	/*TRACE("CDownloadListCtrl::OnGetDispInfo iItem=%d iSubItem=%d", pDispInfo->item.iItem, pDispInfo->item.iSubItem);
	if (pDispInfo->item.mask & LVIF_TEXT)
		TRACE(" LVIF_TEXT");
	if (pDispInfo->item.mask & LVIF_IMAGE)
		TRACE(" LVIF_IMAGE");
	if (pDispInfo->item.mask & LVIF_STATE)
		TRACE(" LVIF_STATE");
	TRACE("\n");*/

	// Although we have an owner drawn listview control we store the text for the primary item in the listview, to be
	// capable of quick searching those items via the keyboard. Because our listview items may change their contents,
	// we do this via a text callback function. The listview control will send us the LVN_DISPINFO notification if
	// it needs to know the contents of the primary item.
	//
	// But, the listview control sends this notification all the time, even if we do not search for an item. At least
	// this notification is only sent for the visible items and not for all items in the list. Though, because this
	// function is invoked *very* often, no *NOT* put any time consuming code here in.

    if (pDispInfo->item.mask & LVIF_TEXT){
        const CtrlItem_Struct* pItem = reinterpret_cast<CtrlItem_Struct*>(pDispInfo->item.lParam);
        if (pItem != NULL && pItem->value != NULL){
			if (pItem->type == FILE_TYPE){
				switch (pDispInfo->item.iSubItem){
					case 0:
						// [TPT] - MFCK [addon] - New Tooltips [Rayita]
						/*if (pDispInfo->item.cchTextMax > 0){
							_tcsncpy(pDispInfo->item.pszText, ((CPartFile*)pItem->value)->GetFileName(), pDispInfo->item.cchTextMax);
							pDispInfo->item.pszText[pDispInfo->item.cchTextMax-1] = _T('\0');
						}*/
						// [TPT] - MFCK [addon] - New Tooltips [Rayita]
						break;
					default:
						// shouldn't happen
						pDispInfo->item.pszText[0] = _T('\0');
						break;
				}
			}
			else if (pItem->type == UNAVAILABLE_SOURCE || pItem->type == AVAILABLE_SOURCE){
				switch (pDispInfo->item.iSubItem){
					case 0:
						if (((CUpDownClient*)pItem->value)->GetUserName() != NULL && pDispInfo->item.cchTextMax > 0){
							_tcsncpy(pDispInfo->item.pszText, ((CUpDownClient*)pItem->value)->GetUserName(), pDispInfo->item.cchTextMax);
							pDispInfo->item.pszText[pDispInfo->item.cchTextMax-1] = _T('\0');
						}
						break;
					default:
						// shouldn't happen
						pDispInfo->item.pszText[0] = _T('\0');
						break;
				}
			}
			else
				ASSERT(0);
        }
    }
    *pResult = 0;
}

// [TPT] - MFCK [addon] - New Tooltips [Rayita]
/*void CDownloadListCtrl::OnLvnGetInfoTip(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVGETINFOTIP pGetInfoTip = reinterpret_cast<LPNMLVGETINFOTIP>(pNMHDR);
	if (pGetInfoTip->iSubItem == 0)
	{
		LVHITTESTINFO hti = {0};
		::GetCursorPos(&hti.pt);
		ScreenToClient(&hti.pt);
		if (SubItemHitTest(&hti) == -1 || hti.iItem != pGetInfoTip->iItem || hti.iSubItem != 0){
			// don' show the default label tip for the main item, if the mouse is not over the main item
			if ((pGetInfoTip->dwFlags & LVGIT_UNFOLDED) == 0 && pGetInfoTip->cchTextMax > 0 && pGetInfoTip->pszText[0] != '\0')
				pGetInfoTip->pszText[0] = '\0';
			return;
		}

		CtrlItem_Struct* content = (CtrlItem_Struct*)GetItemData(pGetInfoTip->iItem);
		if (content && pGetInfoTip->pszText && pGetInfoTip->cchTextMax > 0)
		{
			CString info;

			// build info text and display it
			if (content->type == 1) // for downloading files
			{
				CPartFile* partfile = (CPartFile*)content->value;
				info=partfile->GetInfoSummary(partfile);
			}
			else if (content->type == 3 || content->type == 2) // for sources
			{
				CUpDownClient* client = (CUpDownClient*)content->value;
				if (client->IsEd2kClient())
				{
					in_addr server;
					server.S_un.S_addr = client->GetServerIP();
					
					info.Format(GetResString(IDS_NICKNAME) + _T(" %s\n")
								+ GetResString(IDS_SERVER) + _T(": %s:%d\n")
                                + GetResString(IDS_NEXT_REASK) + _T(": %s (%s)\n") // ZZ:DownloadManager
								+ GetResString(IDS_SOURCEINFO),
								client->GetUserName(),
								ipstr(server), client->GetServerPort(),
                                CastSecondsToHM(client->GetTimeUntilReask(client->GetRequestFile())/1000), CastSecondsToHM(client->GetTimeUntilReask(content->owner)/1000), // ZZ:DownloadManager
								client->GetAskedCountDown(), client->GetAvailablePartCount());

					if (content->type == 2)
					{
						info += GetResString(IDS_CLIENTSOURCENAME) + client->GetClientFilename();

						// SLUGFILLER: showComments remove - no per-client comments
					}
					else
					{	// client asked twice
						info += GetResString(IDS_ASKEDFAF);
                        if (client->GetRequestFile() && client->GetRequestFile()->GetFileName()){
                            info.AppendFormat(_T(": %s"),client->GetRequestFile()->GetFileName());
                        }
					}

// ZZ:DownloadManager -->
                    try {
                        if (thePrefs.IsExtControlsEnabled() && !client->m_OtherRequests_list.IsEmpty()){
                            CString a4afStr;
                            a4afStr.AppendFormat(_T("\n\n") + GetResString(IDS_A4AF_FILES) + _T(":\n"));
                            bool first = TRUE;
                            for (POSITION pos3 = client->m_OtherRequests_list.GetHeadPosition(); pos3!=NULL; client->m_OtherRequests_list.GetNext(pos3)){
                                if(!first) {
                                    a4afStr.Append(_T("\n"));
                                }
                                first = FALSE;
                                a4afStr.AppendFormat(_T("%s"),client->m_OtherRequests_list.GetAt(pos3)->GetFileName());
                            };
                            info += a4afStr;
                        }
                    }catch (...){
                        ASSERT(false);
                    };
// ZZ:DownloadManager <--
				}
				else
				{
					info.Format(_T("URL: %s\nAvailable parts: %u"), client->GetUserName(), client->GetAvailablePartCount());
				}
			}

			_tcsncpy(pGetInfoTip->pszText, info, pGetInfoTip->cchTextMax);
			pGetInfoTip->pszText[pGetInfoTip->cchTextMax-1] = _T('\0');
		}
	}
	*pResult = 0;
}*/

void CDownloadListCtrl::ShowFileDialog(CPartFile* pFile, UINT uInvokePage)
{
	CSimpleArray<CPartFile*> aFiles;
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos != NULL)
	{
		int iItem = GetNextSelectedItem(pos);
		if (iItem != -1)
		{
			const CtrlItem_Struct* pCtrlItem = (CtrlItem_Struct*)GetItemData(iItem);
			if (pCtrlItem->type == FILE_TYPE)
				aFiles.Add((CPartFile*)pCtrlItem->value);
		}
	}

	if (aFiles.GetSize() > 0)
	{
		aFiles[0]->GetDetailDialogInterface()->OpenDetailDialog(&aFiles, uInvokePage, this);	// SLUGFILLER: modelessDialogs	
	}
}

CDownloadListListCtrlItemWalk::CDownloadListListCtrlItemWalk(CDownloadListCtrl* pListCtrl)
	: CListCtrlItemWalk(pListCtrl)
{
	m_pDownloadListCtrl = pListCtrl;
	m_eItemType = (ItemType)-1;
}

CObject* CDownloadListListCtrlItemWalk::GetPrevSelectableItem(CObject* pCurrentObj)	// SLUGFILLER: modelessDialogs
{
	ASSERT( m_pDownloadListCtrl != NULL );
	if (m_pDownloadListCtrl == NULL)
		return NULL;
	// [TPT] - SLUGFILLER: modelessDialogs
	std::pair<CDownloadListCtrl::ListItems::iterator, CDownloadListCtrl::ListItems::iterator> rangeIt = m_pDownloadListCtrl->m_ListItems.equal_range(pCurrentObj);
	const CtrlItem_Struct* current_ctrl_item = NULL;
	for(CDownloadListCtrl::ListItems::iterator it = rangeIt.first; it != rangeIt.second; it++) {
		current_ctrl_item = rangeIt.first->second;
		if (rangeIt.first->second->type != UNAVAILABLE_SOURCE)	// we skip A4AF, so find the non-A4AF one if one exists
			break;
	}
	if (!current_ctrl_item)
		return NULL;
	if (current_ctrl_item->type == FILE_TYPE)
		SetItemType(FILE_TYPE);
	else
		SetItemType(AVAILABLE_SOURCE);	// must be this, unavailable sources are skipped due to duplicates
	// [TPT] - SLUGFILLER: modelessDialogs
	ASSERT( m_eItemType != (ItemType)-1 );

	int iItemCount = m_pDownloadListCtrl->GetItemCount();
	if (iItemCount >= 2)
	{
		// [TPT] - SLUGFILLER: modelessDialogs
		LVFINDINFO find;
		find.flags = LVFI_PARAM;
		find.lParam = (LPARAM)current_ctrl_item;
		int iItem = m_pDownloadListCtrl->FindItem(&find);
		if (iItem != (-1))
		{
		// [TPT] - SLUGFILLER: modelessDialogs
			int iCurSelItem = iItem;
			while (iItem-1 >= 0)
			{
				iItem--;

				const CtrlItem_Struct* ctrl_item = (CtrlItem_Struct*)m_pDownloadListCtrl->GetItemData(iItem);
				if (ctrl_item->type == m_eItemType)	// SLUGFILLER: modelessDialogs - skip A4AF because they won't be recognized on next call
				{
					m_pDownloadListCtrl->SetItemState(iCurSelItem, 0, LVIS_SELECTED | LVIS_FOCUSED);
					m_pDownloadListCtrl->SetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
					m_pDownloadListCtrl->SetSelectionMark(iItem);
					m_pDownloadListCtrl->EnsureVisible(iItem, FALSE);
					return STATIC_DOWNCAST(CObject, (CObject*)ctrl_item->value);
				}
			}
		}
	}
	return NULL;
}

CObject* CDownloadListListCtrlItemWalk::GetNextSelectableItem(CObject* pCurrentObj)	// SLUGFILLER: modelessDialogs
{
	ASSERT( m_pDownloadListCtrl != NULL );
	if (m_pDownloadListCtrl == NULL)
		return NULL;
	// SLUGFILLER: modelessDialogs
	std::pair<CDownloadListCtrl::ListItems::iterator, CDownloadListCtrl::ListItems::iterator> rangeIt = m_pDownloadListCtrl->m_ListItems.equal_range(pCurrentObj);
	const CtrlItem_Struct* current_ctrl_item = NULL;
	for(CDownloadListCtrl::ListItems::iterator it = rangeIt.first; it != rangeIt.second; it++) {
		current_ctrl_item = rangeIt.first->second;
		if (rangeIt.first->second->type != UNAVAILABLE_SOURCE)	// we skip A4AF, so find the non-A4AF one if one exists
			break;
	}
	if (!current_ctrl_item)
		return NULL;
	if (current_ctrl_item->type == FILE_TYPE)
		CDownloadListListCtrlItemWalk::SetItemType(FILE_TYPE);
	else
		CDownloadListListCtrlItemWalk::SetItemType(AVAILABLE_SOURCE);	// must be this, unavailable sources are skipped due to duplicates
	// [TPT] - SLUGFILLER: modelessDialogs
	ASSERT( m_eItemType != (ItemType)-1 );

	int iItemCount = m_pDownloadListCtrl->GetItemCount();
	if (iItemCount >= 2)
	{
		// [TPT] - SLUGFILLER: modelessDialogs
		LVFINDINFO find;
		find.flags = LVFI_PARAM;
		find.lParam = (LPARAM)current_ctrl_item;
		int iItem = m_pDownloadListCtrl->FindItem(&find);
		if (iItem != (-1))
		{
		// [TPT] - SLUGFILLER: modelessDialogs
			int iCurSelItem = iItem;
			while (iItem+1 < iItemCount)
			{
				iItem++;

				const CtrlItem_Struct* ctrl_item = (CtrlItem_Struct*)m_pDownloadListCtrl->GetItemData(iItem);
				if (ctrl_item->type == m_eItemType)	// SLUGFILLER: modelessDialogs - skip A4AF because they won't be recognized on next call
				{
					m_pDownloadListCtrl->SetItemState(iCurSelItem, 0, LVIS_SELECTED | LVIS_FOCUSED);
					m_pDownloadListCtrl->SetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
					m_pDownloadListCtrl->SetSelectionMark(iItem);
					m_pDownloadListCtrl->EnsureVisible(iItem, FALSE);
					return STATIC_DOWNCAST(CObject, (CObject*)ctrl_item->value);
				}
			}
		}
	}
	return NULL;
}

void CDownloadListCtrl::ShowClientDialog(CUpDownClient* pClient, UINT uInvokePage)
{
	pClient->GetDetailDialogInterface()->OpenDetailDialog(this, uInvokePage);	// SLUGFILLER: modelessDialogs
}

// [TPT] - MFCK [addon] - New Tooltips [Rayita]
// START adding by rayita
void CDownloadListCtrl::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	int iSel = GetSelectionMark();
	if (iSel != -1)
	{
		// Get the bounding rectangle of an item. If the mouse
		// location is within the bounding rectangle of the item,
		// you know you have found the item that was being clicked.
		CRect r;
		GetItemRect(iSel, &r, LVIR_BOUNDS);

		CtrlItem_Struct *pListItem = (CtrlItem_Struct*)GetItemData(iSel);
		if(!pListItem || pListItem->type != FILE_TYPE || !pListItem->value)			// if it isn't a file we're done
			return;

		CPartFile *pPartFile = (CPartFile*)pListItem->value;

		DWORD pos = GetMessagePos();
		CPoint pt(LOWORD(pos), HIWORD(pos));
		ScreenToClient(&pt);

// [+]/[-]
		CRect rPlusMinus(r);
		rPlusMinus.left += 4; // + 4 - 4;						// eeek, hardcoded values
		rPlusMinus.top += 1 + 2;								//  --||--
		rPlusMinus.right = rPlusMinus.left + 16 - 3;	// - 4;	//  --||--
		rPlusMinus.bottom = rPlusMinus.top + 16 - 4;			//  --||--
		if(rPlusMinus.PtInRect(pt))
		{
			if(pPartFile->IsPaused()) // modified by rayita
				pPartFile->ResumeFile();
			else
				ExpandCollapseItem(iSel, 2);
			return;
		}

// rating
		CRect rRating;
		rRating.left = rPlusMinus.right + 2; //EC 30.07.03 +2 for filetype icon gap +16 for size
		rRating.top = rPlusMinus.top;
		rRating.bottom = rRating.top + 16 - 2;
		rRating.right = rRating.left + 16;

		return;
	}

	*pResult = 0;
}

void CDownloadListCtrl::OnLvnKeydown(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
	if (GetSelectionMark() != -1)
	{
		uint8 action = EXPAND_COLLAPSE;
		if (pLVKeyDow->wVKey==VK_ADD || pLVKeyDow->wVKey==VK_RIGHT)
			action = EXPAND_ONLY;
		else if ( pLVKeyDow->wVKey==VK_SUBTRACT || pLVKeyDow->wVKey==VK_LEFT )
			action = COLLAPSE_ONLY;
		if (action < EXPAND_COLLAPSE)
			ExpandCollapseItem(GetSelectionMark(), action, true);
	}
	*pResult = 0;
}
// [TPT] - MFCK [addon] - New Tooltips [Rayita]


//[TPT] - Double buffer style in lists
//TODO: I have done in this way because in future could be an option
void CDownloadListCtrl::SetDoubleBufferStyle()
{
	if((_AfxGetComCtlVersion() >= MAKELONG(0, 6)) && thePrefs.GetDoubleBufferStyle())	
		SetExtendedStyle(GetExtendedStyle() | 0x00010000 /*LVS_EX_DOUBLEBUFFER*/);
	else
		if((GetExtendedStyle() & 0x00010000 /*LVS_EX_DOUBLEBUFFER*/) != 0)
			SetExtendedStyle(GetExtendedStyle() ^ 0x00010000);//XOR: delete the style if present
}
