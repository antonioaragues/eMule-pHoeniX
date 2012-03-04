//--- xrmb:downloadclientslist ---

//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
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
#include "DownloadClientsCtrl.h"
#include "FileDetailDialog.h"
#include "otherfunctions.h"
#include "opcodes.h"
#include "ClientDetailDialog.h"
#include "emuledlg.h"
#include "memdc.h"
#include "MenuCmds.h"
#include "FriendList.h"
#include "TransferWnd.h"
#include "ChatWnd.h"
#include "updownclient.h"
// [TPT] - IP Country
#include "CxImage/xImage.h"
#include "ip2country.h"
// [TPT] - IP Country
#include "MenuXP.h"// [TPT] - New Menu Styles
#include "mod_version.h" // [TPT]
#include "PartFile.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DCL_COLUMN_USERNAME		0
#define DCL_COLUMN_VERSION		1
#define DCL_COLUMN_FILE			2
#define DCL_COLUMN_SPEED		3
#define DCL_COLUMN_TRANSDN		4
#define DCL_COLUMN_TRANSUP		5
#define DCL_COLUMN_ULDL			6
#define DCL_COLUMN_COUNTRY		7 // [TPT] - IP Country


IMPLEMENT_DYNAMIC(CDownloadClientsCtrl, CMuleListCtrl)
CDownloadClientsCtrl::CDownloadClientsCtrl()
	: CListCtrlItemWalk(this)
{
}

BEGIN_MESSAGE_MAP(CDownloadClientsCtrl, CMuleListCtrl)
	ON_WM_CONTEXTMENU()
	ON_WM_SYSCOLORCHANGE()
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclkDownloadClientlist)
	ON_WM_MEASUREITEM() // [TPT] - New Menu Styles
END_MESSAGE_MAP()

void CDownloadClientsCtrl::Init()
{
	CImageList ilDummyImageList; //dummy list for getting the proper height of listview entries
	ilDummyImageList.Create(1, theApp.GetSmallSytemIconSize().cy,theApp.m_iDfltImageListColorFlags|ILC_MASK, 1, 1); 
	SetImageList(&ilDummyImageList, LVSIL_SMALL);
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) == 0 );
	ilDummyImageList.Detach();

	SetExtendedStyle(LVS_EX_FULLROWSELECT);

	InsertColumn(DCL_COLUMN_USERNAME,	GetResString(IDS_QL_USERNAME),		LVCFMT_LEFT, 150);
	InsertColumn(DCL_COLUMN_VERSION,	GetResString(IDS_CLIENTSOFTWARE),	LVCFMT_LEFT, 110); 
	InsertColumn(DCL_COLUMN_FILE,		GetResString(IDS_FILE),				LVCFMT_LEFT, 275);
	InsertColumn(DCL_COLUMN_SPEED,		GetResString(IDS_DL_SPEED),			LVCFMT_LEFT, 60);		
	InsertColumn(DCL_COLUMN_TRANSDN,	GetResString(IDS_TRANSDN),			LVCFMT_LEFT, 130);	
	InsertColumn(DCL_COLUMN_TRANSUP,	GetResString(IDS_TRANSUP),			LVCFMT_LEFT, 130);	
	InsertColumn(DCL_COLUMN_ULDL,		GetResString(IDS_DL_ULDL),			LVCFMT_LEFT, 60);
	InsertColumn(DCL_COLUMN_COUNTRY,	GetResString(IDS_COUNTRY),			LVCFMT_LEFT, 100);	// [TPT] - IP Country

	SetAllIcons();
	Localize();
	LoadSettings(CPreferences::tableDownloadClients);
	// Barry - Use preferred sort order from preferences
	int sortItem = thePrefs.GetColumnSortItem(CPreferences::tableDownloadClients);
	bool sortAscending = thePrefs.GetColumnSortAscending(CPreferences::tableDownloadClients);
	SetSortArrow(sortItem, sortAscending);
	// [TPT] - SLUGFILLER: multiSort - load multiple params
	for (int i = thePrefs.GetColumnSortCount(CPreferences::tableDownloadClients); i > 0; ) {
		i--;
		sortItem = thePrefs.GetColumnSortItem(CPreferences::tableDownloadClients, i);
		sortAscending = thePrefs.GetColumnSortAscending(CPreferences::tableDownloadClients, i);
		SortItems(SortProc, sortItem + (sortAscending ? 0:100));
	}
	// [TPT] - SLUGFILLER: multiSort
}

CDownloadClientsCtrl::~CDownloadClientsCtrl()
{
}



void CDownloadClientsCtrl::SetAllIcons() 
{
	imagelist.DeleteImageList();
	imagelist.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	imagelist.SetBkColor(CLR_NONE);
	imagelist.Add(CTempIconLoader(_T("ClientEDonkey")));//0
	imagelist.Add(CTempIconLoader(_T("ClientCompatible")));//1
	imagelist.Add(CTempIconLoader(_T("ClientEDonkeyPlus")));//2
	imagelist.Add(CTempIconLoader(_T("ClientCompatiblePlus")));//3
	imagelist.Add(CTempIconLoader(_T("Friend")));//4
	imagelist.Add(CTempIconLoader(_T("ClientMLDonkey")));//5
	imagelist.Add(CTempIconLoader(_T("ClientMLDonkeyPlus")));//6
	imagelist.Add(CTempIconLoader(_T("ClientEDonkeyHybrid")));//7
	imagelist.Add(CTempIconLoader(_T("ClientEDonkeyHybridPlus")));//8
	imagelist.Add(CTempIconLoader(_T("ClientShareaza")));//9
	imagelist.Add(CTempIconLoader(_T("ClientShareazaPlus")));//10
	imagelist.Add(CTempIconLoader(_T("IDI_SNAFU"))); //11 [TPT] - eWombat SNAFU v2
	imagelist.SetOverlayImage(imagelist.Add(CTempIconLoader(_T("ClientSecureOvl"))), 1);//12
	imagelist.Add(CTempIconLoader(_T("ClientAMule")));//13
	imagelist.Add(CTempIconLoader(_T("ClientAMulePlus")));//14
	imagelist.Add(CTempIconLoader(_T("ClientLPhant")));//15
	imagelist.Add(CTempIconLoader(_T("ClientLPhantPlus")));//16
	// [TPT] - Own icon
	imagelist.Add(CTempIconLoader(_T("PHOENIX")));//17
	imagelist.Add(CTempIconLoader(_T("PHOENIXPLUS")));//18
	imagelist.Add(CTempIconLoader(_T("WEBCACHE"))); // [TPT] - WebCache	// jp webcacheclient icon //19
}
void CDownloadClientsCtrl::Localize() 
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
	CString strRes;

	strRes = GetResString(IDS_QL_USERNAME);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(DCL_COLUMN_USERNAME, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_CLIENTSOFTWARE);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(DCL_COLUMN_VERSION, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_FILE);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(DCL_COLUMN_FILE, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_DL_SPEED);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(DCL_COLUMN_SPEED, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_TRANSDN);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(DCL_COLUMN_TRANSDN, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_TRANSUP);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(DCL_COLUMN_TRANSUP, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_DL_ULDL);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(DCL_COLUMN_ULDL, &hdi);
	strRes.ReleaseBuffer();

	// [TPT] - IP Country
	strRes = GetResString(IDS_COUNTRY);;
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(DCL_COLUMN_COUNTRY, &hdi);
	strRes.ReleaseBuffer();
	// [TPT] - IP Country
	
	//CreateMenues(); // [TPT] - New Menu Style
}

void CDownloadClientsCtrl::AddClient(CUpDownClient* client)
{
	if(!theApp.emuledlg->IsRunning())
		return;
       
	InsertItem(LVIF_TEXT|LVIF_PARAM, GetItemCount(), client->GetUserName(), 0, 0, 1, (LPARAM)client);
	RefreshClient(client);
	// [TPT] - TBH Transfers Window Style
	theApp.emuledlg->transferwnd->UpdateListCount(0, GetItemCount()); 
	theApp.emuledlg->transferwnd->UpdateDownloadClientsCount(GetItemCount());
	// [TPT] - TBH Transfers Window Style
}

void CDownloadClientsCtrl::RemoveClient(CUpDownClient* client)
{
	if(!theApp.emuledlg->IsRunning())
		return;

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)client;
	sint32 result = FindItem(&find);
	if (result != (-1) )
		DeleteItem(result);
	// [TPT] - TBH Transfers Window Style
	theApp.emuledlg->transferwnd->UpdateListCount(0, GetItemCount()); 
	theApp.emuledlg->transferwnd->UpdateDownloadClientsCount(GetItemCount());
	// [TPT] - TBH Transfers Window Style
}

// [TPT] - TransferWindow Fix
void CDownloadClientsCtrl::RefreshClient(CUpDownClient* client)
{
	if( !theApp.emuledlg->IsRunning() )
		return;
	
	//MORPH START - SiRoB, Don't Refresh item if not needed 
	if( theApp.emuledlg->activewnd != theApp.emuledlg->transferwnd  || theApp.emuledlg->transferwnd->downloadclientsctrl.IsWindowVisible() == false ) 
		return; 
	//MORPH END   - SiRoB, Don't Refresh item if not needed 
	
	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)client;
	sint16 result = FindItem(&find);
	if(result != -1)
		Update(result);
	return;
}

#define DLC_DT_TEXT (DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS)

void CDownloadClientsCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if( !theApp.emuledlg->IsRunning() )
		return;
	if (!lpDrawItemStruct->itemData)
		return;
	//MORPH START - Added by SiRoB, Don't draw hidden Rect
	RECT clientRect;
	GetClientRect(&clientRect);
	RECT cur_rec = lpDrawItemStruct->rcItem;
	if (cur_rec.top >= clientRect.bottom || cur_rec.bottom <= clientRect.top)
		return;
	//MORPH END   - Added by SiRoB, Don't draw hidden Rect
	
	CDC* odc = CDC::FromHandle(lpDrawItemStruct->hDC);
	BOOL bCtrlFocused = ((GetFocus() == this ) || (GetStyle() & LVS_SHOWSELALWAYS));
	if( odc && (lpDrawItemStruct->itemAction | ODA_SELECT) && (lpDrawItemStruct->itemState & ODS_SELECTED )){
		if(bCtrlFocused)
			odc->SetBkColor(m_crHighlight);
		else
			odc->SetBkColor(m_crNoHighlight);
	}
	else
		odc->SetBkColor(GetBkColor());

	CUpDownClient* client = (CUpDownClient*)lpDrawItemStruct->itemData;
	CMemDC dc(CDC::FromHandle(lpDrawItemStruct->hDC),&CRect(lpDrawItemStruct->rcItem));
	CFont *pOldFont = dc.SelectObject(GetFont());
	//MORPH - Moved by SiRoB, Don't draw hidden Rect
	/*
	RECT cur_rec;
	MEMCOPY(&cur_rec,&lpDrawItemStruct->rcItem,sizeof(RECT));
	*/
	COLORREF crOldTextColor = dc.SetTextColor(m_crWindowText);

	CString Sbuffer;	
	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();
	cur_rec.right = cur_rec.left - 8;
	cur_rec.left += 4;

	for(int iCurrent = 0; iCurrent < iCount; iCurrent++){
		int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
		if( !IsColumnHidden(iColumn) ){
			cur_rec.right += GetColumnWidth(iColumn);
			//MORPH START - Added by SiRoB, Don't draw hidden columns
			if (cur_rec.left < clientRect.right && cur_rec.right > clientRect.left)
			{
			//MORPH END   - Added by SiRoB, Don't draw hidden columns
			switch(iColumn)
			{
				case DCL_COLUMN_USERNAME:
					{
						uint8 image;
						// [TPT] - eWombat SNAFU v2
						if (client->IsSnafu())
							image = 11;
						// [TPT] - eWombat SNAFU v2
						else if (client->IsFriend())
							image = 4;
						else if (client->GetClientSoft() == SO_EDONKEYHYBRID){
							if (client->Credits() && client->Credits()->GetScoreRatio(client->GetIP()) > 1) 
								image = 8;
							else
								image = 7;
						}
						else if (client->GetClientSoft() == SO_MLDONKEY){
							if (client->Credits() && client->Credits()->GetScoreRatio(client->GetIP()) > 1) 
								image = 6;
							else
								image = 5;
						}
						else if (client->GetClientSoft() == SO_SHAREAZA){
							if(client->Credits() && client->Credits()->GetScoreRatio(client->GetIP()) > 1)
								image = 10;
							else
								image = 9;
						}
						else if (client->GetClientSoft() == SO_AMULE){
							if(client->Credits() && client->Credits()->GetScoreRatio(client->GetIP()) > 1)
								image = 14;
							else
								image = 13;
						}
						else if (client->GetClientSoft() == SO_LPHANT){
							if(client->Credits() && client->Credits()->GetScoreRatio(client->GetIP()) > 1)
								image = 15;
							else
								image = 14;
						}
						// [TPT] - Own icon
						else if (client->ExtProtocolAvailable()){
							if(client->Credits() && client->Credits()->GetScoreRatio(client->GetIP()) > 1)
								image = (client->GetpHoeniXClient())? 18 : 3;
							else
								image = (client->GetpHoeniXClient())? 17 : 1;
						}
						// [TPT] - Own icon
						else{
							if (client->Credits() && client->Credits()->GetScoreRatio(client->GetIP()) > 1)
								image = 2;
							else
								image = 0;
						}
						// [TPT] - WebCache
						bool isWebCache = (client->SupportsWebCache() || client->IsProxy())?true:false;
						if (isWebCache)
						{
							POINT point= {cur_rec.left-4,cur_rec.top+2};
							imagelist.Draw(dc, 19, point, ILD_NORMAL);
							cur_rec.left+=11;
						}
						// [TPT] - WebCache

						POINT point = {cur_rec.left, cur_rec.top+1};
						imagelist.Draw(dc,image, point, ILD_NORMAL | ((client->Credits() && client->Credits()->GetCurrentIdentState(client->GetIP()) == IS_IDENTIFIED) ? INDEXTOOVERLAYMASK(1) : 0));
						Sbuffer = client->GetUserName();
						cur_rec.left +=20;			
						
						dc->DrawText(Sbuffer,Sbuffer.GetLength(),&cur_rec,DLC_DT_TEXT);
						if(isWebCache)
							cur_rec.left -= 31;
						else
							cur_rec.left -= 20;

						break;
					}
				case DCL_COLUMN_VERSION:
					Sbuffer.Format(_T("%s"), client->DbgGetFullClientSoftVer());
					break;
				case DCL_COLUMN_FILE:
					if (client->GetRequestFile())
						Sbuffer.Format(_T("%s"), client->GetRequestFile()->GetFileName());
					else
						Sbuffer = _T("?");
					break;
				case DCL_COLUMN_SPEED:
					Sbuffer.Format(_T("%s"), CastItoXBytes(client->GetDownloadDatarate(), false, true));
					break;
				case DCL_COLUMN_TRANSDN:
					if (client->Credits())
						Sbuffer.Format(_T("%s (%s)"), CastItoXBytes(client->GetTransferredDown(), false, false), CastItoXBytes(client->credits->GetDownloadedTotal(), false, false));
					else
						Sbuffer.Format(_T("%s (0)"), CastItoXBytes(client->GetTransferredDown(), false, false));
					break;
				case DCL_COLUMN_TRANSUP:
					if (client->Credits())
						Sbuffer.Format(_T("%s (%s)"), CastItoXBytes(client->GetTransferredUp(), false, false), CastItoXBytes(client->credits->GetUploadedTotal(), false, false));
					else
						Sbuffer.Format(_T("%s (0)"), CastItoXBytes(client->GetTransferredUp(), false, false));
					break;
				case DCL_COLUMN_ULDL:
					if (client->Credits())
						Sbuffer.Format(_T("%.1f/%.1f"), client->credits->GetScoreRatio(client->GetIP()), client->credits->GetMyScoreRatio(client->GetIP()));
					else
						Sbuffer = _T("");
					break;
				case DCL_COLUMN_COUNTRY:
				{
					// [TPT] - IP Country													
					CxImage imagen = theApp.ip2country->GetCountryFlag(client->GetCountryIP());
					if (imagen.IsEnabled())
					{
						imagen.Draw(dc, cur_rec.left, cur_rec.top+1, 16, 16);																	
					}
					cur_rec.left += 20;
					if (thePrefs.GetEnableShowCountryNames())
					{
						Sbuffer = theApp.ip2country->GetCountryFromIP(client->GetCountryIP());
						dc->DrawText(Sbuffer,Sbuffer.GetLength(),&cur_rec,DLC_DT_TEXT);
						cur_rec.left -=20;					
					}
					// [TPT] - IP Country			
					break;
				}
			}
			if( iColumn != DCL_COLUMN_USERNAME && iColumn != DCL_COLUMN_COUNTRY) // [TPT] - IP Country
				dc->DrawText(Sbuffer,Sbuffer.GetLength(),&cur_rec,DLC_DT_TEXT);
			}//MORPH - Added by SiRoB, Don't draw hidden columns
			cur_rec.left += GetColumnWidth(iColumn);
		}
	}
	
	//draw rectangle around selected item(s)
	if ((lpDrawItemStruct->itemAction | ODA_SELECT) && (lpDrawItemStruct->itemState & ODS_SELECTED))
	{
		RECT outline_rec;
		MEMCOPY(&outline_rec,&lpDrawItemStruct->rcItem,sizeof(RECT));

		outline_rec.top--;
		outline_rec.bottom++;
		dc->FrameRect(&outline_rec, &CBrush(GetBkColor()));
		outline_rec.top++;
		outline_rec.bottom--;
		outline_rec.left++;
		outline_rec.right--;

		if(bCtrlFocused)
			dc->FrameRect(&outline_rec, &CBrush(m_crFocusLine));
		else
			dc->FrameRect(&outline_rec, &CBrush(m_crNoFocusLine));
	}

	dc.SelectObject(pOldFont);
	dc.SetTextColor(crOldTextColor);
}
// [TPT] - TransferWindow Fix

void CDownloadClientsCtrl::OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult){
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// Barry - Store sort order in preferences
	// Determine ascending based on whether already sorted on this column
	int oldSortItem = thePrefs.GetColumnSortItem(CPreferences::tableDownloadClients); 

	bool m_oldSortAscending = thePrefs.GetColumnSortAscending(CPreferences::tableDownloadClients);
	bool sortAscending = (oldSortItem != pNMListView->iSubItem) ? (pNMListView->iSubItem == 0) : !m_oldSortAscending;	

	// Item is column clicked
	int sortItem = pNMListView->iSubItem; 

	// Save new preferences
	thePrefs.SetColumnSortItem(CPreferences::tableDownloadClients, sortItem);
	thePrefs.SetColumnSortAscending(CPreferences::tableDownloadClients, sortAscending);
	
	// Sort table
	SetSortArrow(sortItem, sortAscending);
	SortItems(SortProc, sortItem + (sortAscending ? 0:100));

	*pResult = 0;
}

BOOL CDownloadClientsCtrl::OnCommand(WPARAM wParam,LPARAM lParam ){
	if (GetSelectionMark() != (-1))
	{
		CUpDownClient* client = (CUpDownClient*)GetItemData(GetSelectionMark());
		switch (wParam)
		{
		case MP_SHOWLIST:
			ShowClientDialog(client, IDD_BROWSEFILES);	// [TPT] - itsonlyme: viewSharedFiles
			break;

		case MP_MESSAGE:
			theApp.emuledlg->chatwnd->StartSession(client);
			break;

		case MP_ADDFRIEND:
			theApp.friendlist->AddFriend(client);
			break;
			// [TPT] - FriendSlot
		case MP_FRIENDSLOT:
			{
				if (client && client->IsFriend())
				{
					if( !client->GetFriendSlot() )
					{
						theApp.friendlist->RemoveAllFriendSlots();
						client->SetFriendSlot(true);
					}
				}
				break;
			}
		case MP_RM_FRIENDSLOT:
			theApp.friendlist->RemoveAllFriendSlots();
			break;
			// [TPT] - FriendSlot
		case MP_DETAIL:
			ShowClientDialog(client);	// [TPT] - SLUGFILLER: modelessDialogs			
			break;
		}
	}
	switch(wParam){
		case MP_SWITCHCTRL:{
			((CTransferWnd*)GetParent())->SwitchUploadList();
			break;
						   }
	}
	return true;
}



int CDownloadClientsCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort){
	CUpDownClient* client1 = (CUpDownClient*)lParam1;
	CUpDownClient* client2 = (CUpDownClient*)lParam2;
	if (lParamSort != 0xFFFF)
		lParamSort &= 0x7FFF;
	switch(lParamSort){
	case (DCL_COLUMN_USERNAME + 100): 
		if(client1->GetUserName() == client2->GetUserName())
			return 0;
		else if(!client1->GetUserName())
			return 1;
		else if(!client2->GetUserName())
			return -1;
		return _tcsicmp(client1->GetUserName(),client2->GetUserName());
	case DCL_COLUMN_USERNAME: 
		if(client2->GetUserName() == client1->GetUserName())
			return 0;
		else if(!client2->GetUserName())
			return 1;
		else if(!client1->GetUserName())
			return -1;
		return _tcsicmp(client2->GetUserName(),client1->GetUserName());

	case (DCL_COLUMN_VERSION + 100):
		if( client1->GetClientSoft() != client2->GetClientSoft() )
			return CompareUnsigned(client2->GetClientSoft(), client1->GetClientSoft());
		if (client1->GetVersion() != client2->GetVersion())
			return CompareUnsigned(client1->GetVersion(), client2->GetVersion());
		return client1->GetClientSoftVer().CompareNoCase(client2->GetClientSoftVer());
	case DCL_COLUMN_VERSION:
		if( client2->GetClientSoft() != client1->GetClientSoft() )
			return CompareUnsigned(client1->GetClientSoft(), client2->GetClientSoft());
		if (client2->GetVersion() != client1->GetVersion())
			return CompareUnsigned(client2->GetVersion(), client1->GetVersion());
		return client2->GetClientSoftVer().CompareNoCase(client1->GetClientSoftVer());
	case (DCL_COLUMN_FILE + 100): 
		if (!client2->GetRequestFile())
			return -1;
		if (!client1->GetRequestFile())
			return 1;
		return client1->GetRequestFile()->GetFileName().CompareNoCase(client2->GetRequestFile()->GetFileName());
	case DCL_COLUMN_FILE:
		if (!client1->GetRequestFile())
			return -1;
		if (!client2->GetRequestFile())
			return 1;
		return client2->GetRequestFile()->GetFileName().CompareNoCase(client1->GetRequestFile()->GetFileName());
	case (DCL_COLUMN_SPEED + 100):
		return CompareUnsigned(client1->GetDownloadDatarate(), client2->GetDownloadDatarate());
	case DCL_COLUMN_SPEED:
		return CompareUnsigned(client2->GetDownloadDatarate(), client1->GetDownloadDatarate());
	case (DCL_COLUMN_TRANSDN + 100):
		return CompareUnsigned(client1->GetTransferredDown(), client2->GetTransferredDown());
	case DCL_COLUMN_TRANSDN:
		return CompareUnsigned(client2->GetTransferredDown(), client1->GetTransferredDown());
	case (DCL_COLUMN_TRANSUP + 100):
		return CompareUnsigned(client1->GetUploadDatarate(), client2->GetUploadDatarate());
	case DCL_COLUMN_TRANSUP:
		return CompareUnsigned(client2->GetUploadDatarate(), client1->GetUploadDatarate());
	case (DCL_COLUMN_ULDL + 100):
	{
		if (client1->credits && client2->credits)
		{
			float r1=client1->credits->GetScoreRatio(client1->GetIP());
			float r2=client2->credits->GetScoreRatio(client2->GetIP());
			return r1==r2? 0 : r1<r2? -1 : 1;
		}
		else if (!client1->credits)
			return 1;
		else
			return -1;
	}
	case DCL_COLUMN_ULDL:
	{
		if (client1->credits && client2->credits)
		{
			float r1=client2->credits->GetScoreRatio(client2->GetIP());
			float r2=client1->credits->GetScoreRatio(client1->GetIP());
			return r1==r2? 0 : r1<r2? -1 : 1;
		}
		else if (!client2->credits)
			return 1;
		else
			return -1;
	}
	// [TPT] - IP Country
	case DCL_COLUMN_COUNTRY:
		if(client1->GetCountryIP() && client2->GetCountryIP())
				return theApp.ip2country->Compare(client1->GetCountryIP(), client2->GetCountryIP());
			else if(client1->GetCountryIP())
				return 1;
			else
				return -1;
	case (DCL_COLUMN_COUNTRY+100):
		if(client1->GetCountryIP() && client2->GetCountryIP())
				return theApp.ip2country->Compare(client2->GetCountryIP(), client1->GetCountryIP());
			else if(client2->GetCountryIP())
				return 1;
			else
				return -1;
	// [TPT] - IP Country

	default:
		return 0;
	}
}

// [TPT] - New Menu Styles BEGIN
void CDownloadClientsCtrl::OnContextMenu(CWnd* pWnd, CPoint point)
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	UINT uFlags = (iSel != -1) ? MF_ENABLED : MF_GRAYED;

	CUpDownClient* client = (iSel != -1) ? (CUpDownClient*)GetItemData(iSel) : NULL;

	//Menu Configuration
	CMenuXP	*pMenu = new CMenuXP;
	pMenu->CreatePopupMenu();
	pMenu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
	pMenu->AddSideBar(new CMenuXPSideBar(17, MOD_VERSION));
	pMenu->SetSideBarStartColor(RGB(255,0,0));
	pMenu->SetSideBarEndColor(RGB(255,128,0));
	pMenu->SetSelectedBarColor(RGB(242,120,114));

	//Add items
	pMenu->AppendODMenu(MF_STRING | uFlags, new CMenuXPText(MP_DETAIL, GetResString(IDS_SHOWDETAILS), theApp.LoadIcon(_T("details"), 16, 16)));
	pMenu->AppendODMenu(MF_STRING | ((!client || !client->GetViewSharedFilesSupport()) ? MF_GRAYED : MF_ENABLED) | uFlags, new CMenuXPText(MP_SHOWLIST, GetResString(IDS_VIEWFILES), theApp.LoadIcon(_T("seeshared2"), 16, 16)));
	pMenu->AppendODMenu(MF_STRING | uFlags, new CMenuXPText(MP_LIST_REQUESTED_FILES, GetResString(IDS_LIST_REQ_FILES), theApp.LoadIcon(_T("requestedFiles2"), 16, 16)));
	pMenu->AppendSeparator();
	pMenu->AppendODMenu(MF_STRING | uFlags, new CMenuXPText(MP_ADDFRIEND, GetResString(IDS_ADDFRIEND), theApp.LoadIcon(_T("friend2"), 16, 16)));
	pMenu->AppendODMenu(MF_STRING | uFlags, new CMenuXPText(MP_MESSAGE, GetResString(IDS_SEND_MSG), theApp.LoadIcon(_T("sendMessage"), 16, 16)));
	if (client && client->IsFriend() && !client->HasLowID())
	{
		if (client->GetFriendSlot() == false)
		{
			pMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_FRIENDSLOT, GetResString(IDS_FRIENDSLOT)));
			pMenu->EnableMenuItem(MP_FRIENDSLOT,MF_ENABLED);			
		}
		else
		{
			pMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_RM_FRIENDSLOT, GetResString(IDS_RM_FRIENDSLOT)));
			pMenu->EnableMenuItem(MP_RM_FRIENDSLOT,MF_ENABLED);			
		}
	}
	pMenu->AppendSeparator();
	pMenu->AppendODMenu(MF_STRING | uFlags, new CMenuXPText(MP_SWITCHCTRL, GetResString(IDS_VIEWUPLOADS)));

	pMenu->TrackPopupMenu(TPM_LEFTBUTTON, point.x, point.y, this);

	delete pMenu;

}
// [TPT] - New Menu Styles END

// [TPT] - New Menu Styles BEGIN
void CDownloadClientsCtrl::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	HMENU hMenu = AfxGetThreadState()->m_hTrackingMenu;
	CMenu	*pMenu = CMenu::FromHandle(hMenu);
	pMenu->MeasureItem(lpMeasureItemStruct);
	
	CWnd::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}
// [TPT] - New Menu Styles END


void CDownloadClientsCtrl::OnNMDblclkDownloadClientlist(NMHDR *pNMHDR, LRESULT *pResult) {
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1){
		CUpDownClient* client = (CUpDownClient*)GetItemData(iSel);
		if (client){
			ShowClientDialog(client);	// [TPT] - SLUGFILLER: modelessDialogs
		}
	}
	*pResult = 0;
}

// [TPT] - UserDetails
void CDownloadClientsCtrl::ShowSelectedUserDetails() {
	POINT point;
	::GetCursorPos(&point);
	CPoint p = point; 
    ScreenToClient(&p); 
    int it = HitTest(p); 
    if (it == -1) return;
	SetSelectionMark(it);   // display selection mark correctly! 

	CUpDownClient* client = (CUpDownClient*)GetItemData(GetSelectionMark());

	if (client){
		ShowClientDialog(client);	// [TPT] - SLUGFILLER: modelessDialogs
	}
}
// [TPT] - UserDetails
void CDownloadClientsCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
}

void CDownloadClientsCtrl::ShowClientDialog(CUpDownClient* pClient, UINT uInvokePage)
{
	pClient->GetDetailDialogInterface()->OpenDetailDialog(this, uInvokePage);	// SLUGFILLER: modelessDialogs
}