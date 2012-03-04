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
#include "ClientListCtrl.h"
#include "otherfunctions.h"
#include "MenuCmds.h"
#include "ClientDetailDialog.h"
#include "KademliaWnd.h"
#include "ClientList.h"
#include "emuledlg.h"
#include "FriendList.h"
#include "TransferWnd.h"
#include "MemDC.h"
#include "UpDownClient.h"
#include "ClientCredits.h"
#include "ListenSocket.h"
#include "ChatWnd.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "Kademlia/net/KademliaUDPListener.h"
// [TPT]
#include "updownclient.h"
#include "uploadqueue.h"
#include "downloadqueue.h"
#include "log.h"
// [TPT] - IP Country
#include "CxImage/xImage.h"
#include "ip2country.h"
// [TPT] - IP Country
#include "MenuXP.h"// [TPT] - New Menu Styles
#include "mod_version.h" // [TPT]

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif


// CClientListCtrl

IMPLEMENT_DYNAMIC(CClientListCtrl, CMuleListCtrl)

CClientListCtrl::CClientListCtrl()
	: CListCtrlItemWalk(this)
{
}

void CClientListCtrl::Init()
{
	CImageList ilDummyImageList; //dummy list for getting the proper height of listview entries
	ilDummyImageList.Create(1, theApp.GetSmallSytemIconSize().cy,theApp.m_iDfltImageListColorFlags|ILC_MASK, 1, 1); 
	SetImageList(&ilDummyImageList, LVSIL_SMALL);
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) == 0 );
	ilDummyImageList.Detach();

	SetExtendedStyle(LVS_EX_FULLROWSELECT);
	InsertColumn(0,GetResString(IDS_QL_USERNAME),LVCFMT_LEFT,150,0);
	InsertColumn(1,GetResString(IDS_CL_UPLOADSTATUS),LVCFMT_LEFT,150,1);
	InsertColumn(2,GetResString(IDS_CL_TRANSFUP),LVCFMT_LEFT,150,2);
	InsertColumn(3,GetResString(IDS_CL_DOWNLSTATUS),LVCFMT_LEFT,150,3);
	InsertColumn(4,GetResString(IDS_CL_TRANSFDOWN),LVCFMT_LEFT,150,4);
	CString coltemp;coltemp=GetResString(IDS_CD_CSOFT);coltemp.Remove(':');
	InsertColumn(5,coltemp,LVCFMT_LEFT,150,5);
	InsertColumn(6,GetResString(IDS_CONNECTED),LVCFMT_LEFT,150,6);
	coltemp=GetResString(IDS_CD_UHASH);coltemp.Remove(':');
	InsertColumn(7,coltemp,LVCFMT_LEFT,150,7);
	InsertColumn(8,GetResString(IDS_BANNED),LVCFMT_LEFT,60,8);	// [TPT] - eWombat SNAFU v2
	InsertColumn(9,GetResString(IDS_COUNTRY),LVCFMT_LEFT,100,9);	// [TPT] - IP Country
	
	SetAllIcons();
	Localize();
	LoadSettings(CPreferences::tableClientList);
	int sortItem = thePrefs.GetColumnSortItem(CPreferences::tableClientList);
	bool sortAscending = thePrefs.GetColumnSortAscending(CPreferences::tableClientList);
	SetSortArrow(sortItem, sortAscending);
	// [TPT] - SLUGFILLER: multiSort - load multiple params
	for (int i = thePrefs.GetColumnSortCount(CPreferences::tableClientList); i > 0; ) {
		i--;
		sortItem = thePrefs.GetColumnSortItem(CPreferences::tableClientList, i);
		sortAscending = thePrefs.GetColumnSortAscending(CPreferences::tableClientList, i);
	SortItems(SortProc, sortItem + (sortAscending ? 0:100));
	}
	// [TPT] - SLUGFILLER: multiSort
}

CClientListCtrl::~CClientListCtrl()
{
}

void CClientListCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
}

void CClientListCtrl::SetAllIcons()
{
	imagelist.DeleteImageList();
	imagelist.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	imagelist.SetBkColor(CLR_NONE);
	imagelist.Add(CTempIconLoader(_T("ClientEDonkey")));
	imagelist.Add(CTempIconLoader(_T("ClientCompatible")));
	imagelist.Add(CTempIconLoader(_T("Friend")));
	imagelist.Add(CTempIconLoader(_T("ClientMLDonkey")));
	imagelist.Add(CTempIconLoader(_T("ClientEDonkeyHybrid")));
	imagelist.Add(CTempIconLoader(_T("ClientShareaza")));
	imagelist.Add(CTempIconLoader(_T("Server")));
	imagelist.Add(CTempIconLoader(_T("ClientAMule")));
	imagelist.Add(CTempIconLoader(_T("ClientLPhant")));
	imagelist.Add(CTempIconLoader(_T("IDI_SNAFU"))); // [TPT] - eWombat SNAFU v2
	imagelist.SetOverlayImage(imagelist.Add(CTempIconLoader(_T("ClientSecureOvl"))), 1);
	// [TPT] - Own icon
	imagelist.Add(CTempIconLoader(_T("PHOENIX")));//11
	imagelist.Add(CTempIconLoader(_T("PHOENIXPLUS")));//12
}

void CClientListCtrl::Localize()
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;

	if(pHeaderCtrl->GetItemCount() != 0) {
		CString strRes;

		strRes = GetResString(IDS_QL_USERNAME);
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(0, &hdi);
		strRes.ReleaseBuffer();

		strRes = GetResString(IDS_CL_UPLOADSTATUS);
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(1, &hdi);
		strRes.ReleaseBuffer();

		strRes = GetResString(IDS_CL_TRANSFUP);
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(2, &hdi);
		strRes.ReleaseBuffer();

		strRes = GetResString(IDS_CL_DOWNLSTATUS);
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(3, &hdi);
		strRes.ReleaseBuffer();

		strRes = GetResString(IDS_CL_TRANSFDOWN);
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(4, &hdi);
		strRes.ReleaseBuffer();

		strRes=GetResString(IDS_CD_CSOFT);strRes.Remove(':');
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(5, &hdi);
		strRes.ReleaseBuffer();

		strRes = GetResString(IDS_CONNECTED);
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(6, &hdi);
		strRes.ReleaseBuffer();

		strRes=GetResString(IDS_CD_UHASH);strRes.Remove(':');
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(7, &hdi);
		strRes.ReleaseBuffer();
		
		//<<< [TPT] - eWombat SNAFU v2
		strRes=GetResString(IDS_BANNED);
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(8, &hdi);
		strRes.ReleaseBuffer();
		//>>> [TPT] - eWombat SNAFU v2

		// [TPT] - IP Country
		strRes = GetResString(IDS_COUNTRY);;
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(9, &hdi);
		strRes.ReleaseBuffer();
		// [TPT] - IP Country
	}
}

void CClientListCtrl::ShowKnownClients()
{
	DeleteAllItems();
	int iItemCount = 0;
	for(POSITION pos = theApp.clientlist->list.GetHeadPosition(); pos != NULL;){
		const CUpDownClient* cur_client = theApp.clientlist->list.GetNext(pos);
		int iItem = InsertItem(LVIF_TEXT|LVIF_PARAM,iItemCount,LPSTR_TEXTCALLBACK,0,0,0,(LPARAM)cur_client);
		Update(iItem);
		iItemCount++;
	}
	theApp.emuledlg->transferwnd->UpdateListCount(3, iItemCount); // [TPT] - TBH Transfers Window Style
}

void CClientListCtrl::AddClient(const CUpDownClient* client)
{
	if (!theApp.emuledlg->IsRunning())
		return;
	if (thePrefs.IsKnownClientListDisabled())
		return;

	int iItemCount = GetItemCount();
	int iItem = InsertItem(LVIF_TEXT|LVIF_PARAM,iItemCount,LPSTR_TEXTCALLBACK,0,0,0,(LPARAM)client);
	Update(iItem);
	theApp.emuledlg->transferwnd->UpdateListCount(3, iItemCount+1); // [TPT] - TBH Transfers Window Style
}

void CClientListCtrl::RemoveClient(const CUpDownClient* client)
{
	if (!theApp.emuledlg->IsRunning())
		return;

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)client;
	sint32 result = FindItem(&find);
	if (result != -1){
		DeleteItem(result);
		theApp.emuledlg->transferwnd->UpdateListCount(3); // [TPT] - TBH Transfers Window Style
	}
}

void CClientListCtrl::RefreshClient(const CUpDownClient* client)
{
	// There is some type of timing issue here.. If you click on item in the queue or upload and leave
	// the focus on it when you exit the cient, it breaks on line 854 of emuleDlg.cpp.. 
	// I added this IsRunning() check to this function and the DrawItem method and
	// this seems to keep it from crashing. This is not the fix but a patch until
	// someone points out what is going wrong.. Also, it will still assert in debug mode..
	if (!theApp.emuledlg->IsRunning())
		return;

	// [TPT] - MORPH START - SiRoB, Don't Refresh item if not needed
	if( theApp.emuledlg->activewnd != theApp.emuledlg->transferwnd || theApp.emuledlg->transferwnd->clientlistctrl.IsWindowVisible() == false )
		return;
	// [TPT] - MORPH START - SiRoB, Don't Refresh item if not needed
	
	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)client;
	sint16 result = FindItem(&find);
	if (result != -1)
		Update(result);
}

#define DLC_DT_TEXT (DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS)

void CClientListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if( !theApp.emuledlg->IsRunning() )
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
	BOOL bCtrlFocused = ((GetFocus() == this ) || (GetStyle() & LVS_SHOWSELALWAYS));
	if( (lpDrawItemStruct->itemAction | ODA_SELECT) && (lpDrawItemStruct->itemState & ODS_SELECTED )){
		if(bCtrlFocused)
			odc->SetBkColor(m_crHighlight);
		else
			odc->SetBkColor(m_crNoHighlight);
	}
	else
		odc->SetBkColor(GetBkColor());
	const CUpDownClient* client = (CUpDownClient*)lpDrawItemStruct->itemData;
	CMemDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem);
	CFont* pOldFont = dc.SelectObject(GetFont());
	// RECT cur_rec = lpDrawItemStruct->rcItem; // [TPT] - Morph. Don't draw hidden Rect
	COLORREF crOldTextColor = dc.SetTextColor(m_crWindowText);

	int iOldBkMode;
	if (m_crWindowTextBk == CLR_NONE){
		DefWindowProc(WM_ERASEBKGND, (WPARAM)(HDC)dc, 0);
		iOldBkMode = dc.SetBkMode(TRANSPARENT);
	}
	else
		iOldBkMode = OPAQUE;

	CString Sbuffer;

	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();
	cur_rec.right = cur_rec.left - 8;
	cur_rec.left += 4;

	for(int iCurrent = 0; iCurrent < iCount; iCurrent++){
		int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
		if( !IsColumnHidden(iColumn) ){
			cur_rec.right += GetColumnWidth(iColumn);
			// [TPT] - MORPH START - Added by SiRoB, Don't draw hidden columns
			if (cur_rec.left < clientRect.right && cur_rec.right > clientRect.left)
			{			
				switch(iColumn){
					case 0:{
						uint8 image;
						// [TPT] - eWombat SNAFU v2					
						if (client->IsSnafu())
							image=9;
						// [TPT] - eWombat SNAFU v2
						else if (client->IsFriend())
							image = 2;
						else if (client->GetClientSoft() == SO_EDONKEYHYBRID)
							image = 4;
						else if (client->GetClientSoft() == SO_MLDONKEY)
							image = 3;
						else if (client->GetClientSoft() == SO_SHAREAZA)
							image = 5;
						else if (client->GetClientSoft() == SO_URL)
							image = 6;
						else if (client->GetClientSoft() == SO_AMULE)
							image = 7;
						else if (client->GetClientSoft() == SO_LPHANT)
							image = 8;
						// [TPT] - Own icon
						else if (client->ExtProtocolAvailable())
							image = (client->GetpHoeniXClient()) ? 12 : 1;
						else
							image = (client->GetpHoeniXClient()) ? 11 : 0;
						// [TPT] - Own icon
	
						POINT point = {cur_rec.left, cur_rec.top+1};
						imagelist.Draw(dc,image, point, ILD_NORMAL | ((client->Credits() && client->Credits()->GetCurrentIdentState(client->GetIP()) == IS_IDENTIFIED) ? INDEXTOOVERLAYMASK(1) : 0));
						if (client->GetUserName()==NULL)
							Sbuffer.Format(_T("(%s)"), GetResString(IDS_UNKNOWN));
						else
							Sbuffer = client->GetUserName();
						cur_rec.left +=20;
						dc->DrawText(Sbuffer,Sbuffer.GetLength(),&cur_rec,DLC_DT_TEXT);
						cur_rec.left -=20;
						break;
					}
					case 1:{
						Sbuffer = client->GetUploadStateDisplayString();
						break;
					}
					case 2:{
						if(client->credits)
							Sbuffer = CastItoXBytes(client->credits->GetUploadedTotal(), false, false);
						else
							Sbuffer.Empty();
						break;
					}
					case 3:{
						Sbuffer = client->GetDownloadStateDisplayString();
						break;
					}
					case 4:{
						if(client->credits)
							Sbuffer = CastItoXBytes(client->credits->GetDownloadedTotal(), false, false);
						else
							Sbuffer.Empty();
						break;
					}
					case 5:{
						Sbuffer = client->DbgGetFullClientSoftVer(); // [TPT]
						if (Sbuffer.IsEmpty())
							Sbuffer = GetResString(IDS_UNKNOWN);
						break;
					}
					case 6:{
						if(client->socket){
							if(client->socket->IsConnected()){
								Sbuffer = GetResString(IDS_YES);
								break;
							}
						}
						Sbuffer = GetResString(IDS_NO);
						break;
					}
					case 7:
						Sbuffer = md4str(client->GetUserHash());
						break;
					//<<< [TPT] - eWombat SNAFU v2
					case 8:
						//if(client->IsBanned())
						//	Sbuffer = GetResString(IDS_YES);
						//else
						//	Sbuffer = GetResString(IDS_NO);
						Sbuffer = client->GetBanStr();
						break;
					//<<< [TPT] - eWombat SNAFU v2
					case 9:
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
				if( iColumn != 0 && iColumn != 9) // [TPT] - IP Country
					dc->DrawText(Sbuffer,Sbuffer.GetLength(),&cur_rec,DLC_DT_TEXT);
			}// [TPT] - MORPH - Added by SiRoB, Don't draw hidden colums
			cur_rec.left += GetColumnWidth(iColumn);
		}
	}
//draw rectangle around selected item(s)
	if ((lpDrawItemStruct->itemAction | ODA_SELECT) && (lpDrawItemStruct->itemState & ODS_SELECTED))
	{
		RECT outline_rec = lpDrawItemStruct->rcItem;

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

	if (m_crWindowTextBk == CLR_NONE)
		dc.SetBkMode(iOldBkMode);
	dc.SelectObject(pOldFont);
	dc.SetTextColor(crOldTextColor);
}

BEGIN_MESSAGE_MAP(CClientListCtrl, CMuleListCtrl)
	ON_WM_CONTEXTMENU()
	ON_WM_MEASUREITEM()// [TPT] - New Menu Styles
	ON_WM_SYSCOLORCHANGE()
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
END_MESSAGE_MAP()

// CClientListCtrl message handlers
	
// [TPT] - New Menu Styles BEGIN
void CClientListCtrl::OnContextMenu(CWnd* pWnd, CPoint point)
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	
	CUpDownClient* client = (iSel != -1) ? (CUpDownClient*)GetItemData(iSel) : NULL; // [TPT] remove const due to snafu, see how to implemented it

	//Menu Configuration
	CMenuXP	*pMenu = new CMenuXP;
	pMenu->CreatePopupMenu();
	pMenu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
	pMenu->AddSideBar(new CMenuXPSideBar(17, MOD_VERSION));
	pMenu->SetSideBarStartColor(RGB(255,0,0));
	pMenu->SetSideBarEndColor(RGB(255,128,0));
	pMenu->SetSelectedBarColor(RGB(242,120,114));
	//Add items
	pMenu->AppendODMenu(MF_STRING | (client ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_DETAIL, GetResString(IDS_SHOWDETAILS), theApp.LoadIcon(_T("details"), 16, 16)));
	// [TPT] - itsonlyme:clientDetails
	pMenu->AppendODMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetViewSharedFilesSupport()) ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_SHOWLIST, GetResString(IDS_VIEWFILES), theApp.LoadIcon(_T("seeshared2"), 16, 16)));
	pMenu->AppendODMenu(MF_STRING | ((!client) ? MF_GRAYED : MF_ENABLED), new CMenuXPText(MP_LIST_REQUESTED_FILES, GetResString(IDS_LIST_REQ_FILES), theApp.LoadIcon(_T("requestedFiles2"), 16, 16)));
	pMenu->AppendSeparator();
	// [TPT] - itsonlyme:clientDetails
	pMenu->AppendODMenu(MF_STRING | ((client && client->IsEd2kClient() && !client->IsFriend()) ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_ADDFRIEND, GetResString(IDS_ADDFRIEND), theApp.LoadIcon(_T("friend2"), 16, 16)));
	pMenu->AppendODMenu(MF_STRING | ((client && client->IsEd2kClient()) ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_MESSAGE, GetResString(IDS_SEND_MSG), theApp.LoadIcon(_T("sendMessage"), 16, 16)));
	
	//<<< [TPT] - eWombat SNAFU v2	
	if (client && (client->IsSnafu() || client->IsSUIFailed() || client->IsBanned()))
	{
		pMenu->AppendSeparator();
		if (client->IsSnafu() || client->IsSUIFailed())
		{
			if (theApp.uploadqueue->IsOnUploadQueue(client))
				pMenu->AppendODMenu(MF_STRING | (client ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_REMOVEUL, GetResString(IDS_REMOVEUL)));
			if (client->GetDownloadState()!=DS_NONE)
				pMenu->AppendODMenu(MF_STRING | (client ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_REMOVEDL, GetResString(IDS_REMOVEDL)));
		}
		if (!client->IsSUIFailed() && (client->IsBanned() || client->IsSnafu()))
				pMenu->AppendODMenu(MF_STRING | (client ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_UNBAN, GetResString(IDS_UNBAN)));
	}
	//>>> [TPT] - eWombat SNAFU v2

	// [TPT] - itsonlyme:clientDetails remove - moved up
	if (Kademlia::CKademlia::isRunning() && !Kademlia::CKademlia::isConnected())
		pMenu->AppendODMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetKadPort()!=0) ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_BOOT, GetResString(IDS_BOOTSTRAP), theApp.LoadIcon(_T("boostrap2"), 16, 16)));
	
	pMenu->TrackPopupMenu(TPM_LEFTBUTTON, point.x, point.y, this);

	delete pMenu;

}
// [TPT] - New Menu Styles END


// [TPT] - New Menu Styles BEGIN
void CClientListCtrl::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	HMENU hMenu = AfxGetThreadState()->m_hTrackingMenu;
	CMenu	*pMenu = CMenu::FromHandle(hMenu);
	pMenu->MeasureItem(lpMeasureItemStruct);
	
	CWnd::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}
// [TPT] - New Menu Styles END

BOOL CClientListCtrl::OnCommand(WPARAM wParam,LPARAM lParam )
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1){
		CUpDownClient* client = (CUpDownClient*)GetItemData(iSel);
		switch (wParam){
			case MP_SHOWLIST:
				client->GetDetailDialogInterface()->OpenDetailDialog(this, IDD_BROWSEFILES);	// [TPT] - itsonlyme: viewSharedFiles
				break;
			case MP_MESSAGE:
				theApp.emuledlg->chatwnd->StartSession(client);
				break;
			case MP_ADDFRIEND:
				if (theApp.friendlist->AddFriend(client))
					Update(iSel);
				RefreshClient(client);	// [TPT] - itsonlyme: displayOptions
				break;
		    //<<< [TPT] - eWombat SNAFU v2
			case MP_UNBAN:
				if (!client)
					break;

				if (client->IsSnafu())
					{
					client->UnSnafu();
					client->SnafuExclude();
					}
				if (client->IsBanned()){
					client->UnBan();
					Update(iSel);
				}
				break;
			case MP_REMOVEUL:
				if (client!=NULL)
					{
					if (thePrefs.GetVerbose())
						AddDebugLogLine(_T("S.N.A.F.U. %s kicked from uploadqueue"), client->GetUserName());
					theApp.uploadqueue->RemoveFromWaitingQueue(client,false);
					if (!client->socket)
						 client->Disconnected(_T("Remove from uploadqueue due to S.N.A.F.U"), false, CUpDownClient::USR_SNAFU); // [TPT] - Maella -Upload Stop Reason-						
					}
				break;

			case MP_REMOVEDL:
				if (client!=NULL)
					{
					if (thePrefs.GetVerbose())
						AddDebugLogLine(_T("S.N.A.F.U. %s kicked from downloadqueue"), client->GetUserName());
					if (client->GetDownloadState()!=DS_DOWNLOADING)
						{
						theApp.downloadqueue->RemoveSource(client);
						if (!client->socket)
							client->Disconnected(_T("Remove from download due to S.N.A.F.U"), false, CUpDownClient::USR_SNAFU); // [TPT] - Maella -Upload Stop Reason-
						}
					}
				break;
		    //>>> [TPT] - eWombat SNAFU v2
			case MPG_ALTENTER:
			case MP_DETAIL:{
				client->GetDetailDialogInterface()->OpenDetailDialog(this);	// [TPT] - SLUGFILLER: modelessDialogs
				break;
			}
			case MP_BOOT:
				if (client->GetKadPort())
					Kademlia::CKademlia::bootstrap(ntohl(client->GetIP()), client->GetKadPort());
				break;
			// [TPT] - itsonlyme:reqFiles START
			case MP_LIST_REQUESTED_FILES: {
				client->GetDetailDialogInterface()->OpenDetailDialog(this, IDD_REQFILES);	// [TPT] - SLUGFILLER: modelessDialogs
				break;
			}
			// [TPT] - itsonlyme:reqFiles END
		}
	}
	return true;
} 

void CClientListCtrl::OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult){

	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// Barry - Store sort order in preferences
	// Determine ascending based on whether already sorted on this column
	int sortItem = thePrefs.GetColumnSortItem(CPreferences::tableClientList);
	bool m_oldSortAscending = thePrefs.GetColumnSortAscending(CPreferences::tableClientList);
	bool sortAscending = (sortItem != pNMListView->iSubItem) ? true : !m_oldSortAscending;
	// Item is column clicked
	sortItem = pNMListView->iSubItem;
	// Save new preferences
	thePrefs.SetColumnSortItem(CPreferences::tableClientList, sortItem);
	thePrefs.SetColumnSortAscending(CPreferences::tableClientList, sortAscending);
	// Sort table
	SetSortArrow(sortItem, sortAscending);
	SortItems(SortProc, sortItem + (sortAscending ? 0:100));

	*pResult = 0;
}

int CClientListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	const CUpDownClient* item1 = (CUpDownClient*)lParam1;
	const CUpDownClient* item2 = (CUpDownClient*)lParam2;
	switch(lParamSort){
		case 0: 
			if(item1->GetUserName() && item2->GetUserName())
				return CompareLocaleStringNoCase(item1->GetUserName(), item2->GetUserName());
			else if(item1->GetUserName())
				return 1;
			else
				return -1;
		case 100:
			if(item1->GetUserName() && item2->GetUserName())
				return CompareLocaleStringNoCase(item2->GetUserName(), item1->GetUserName());
			else if(item2->GetUserName())
				return 1;
			else
				return -1;
		case 1:
			return item1->GetUploadState()-item2->GetUploadState();
		case 101:
			return item2->GetUploadState()-item1->GetUploadState();
		case 2:
			if( item1->credits && item2->credits )
				return CompareUnsigned64(item1->credits->GetUploadedTotal(), item2->credits->GetUploadedTotal());
			else if( !item1->credits )
				return 1;
			else
				return -1;
		case 102:
			if( item1->credits && item2->credits )
				return CompareUnsigned64(item2->credits->GetUploadedTotal(), item1->credits->GetUploadedTotal());
			else if( !item1->credits )
				return 1;
			else
				return -1;
		case 3:
		    if( item1->GetDownloadState() == item2->GetDownloadState() ){
			    if( item1->IsRemoteQueueFull() && item2->IsRemoteQueueFull() )
				    return 0;
			    else if( item1->IsRemoteQueueFull() )
				    return 1;
			    else if( item2->IsRemoteQueueFull() )
				    return -1;
			    else
				    return 0;
		    }
			return item1->GetDownloadState()-item2->GetDownloadState();
		case 103:
		    if( item2->GetDownloadState() == item1->GetDownloadState() ){
			    if( item2->IsRemoteQueueFull() && item1->IsRemoteQueueFull() )
				    return 0;
			    else if( item2->IsRemoteQueueFull() )
				    return 1;
			    else if( item1->IsRemoteQueueFull() )
				    return -1;
			    else
				    return 0;
		    }
			return item2->GetDownloadState()-item1->GetDownloadState();
		case 4:
			if( item1->credits && item2->credits )
				return CompareUnsigned64(item1->credits->GetDownloadedTotal(), item2->credits->GetDownloadedTotal());
			else if( !item1->credits )
				return 1;
			else
				return -1;
		case 104:
			if( item1->credits && item2->credits )
				return CompareUnsigned64(item2->credits->GetDownloadedTotal(), item1->credits->GetDownloadedTotal());
			else if( !item2->credits ) // [TPT] - Patch
				return 1;
			else
				return -1;
		// [TPT] - Morph sort
		case 5:
			if(item1->GetClientSoft() == item2->GetClientSoft())
				if(item2->GetVersion() == item1->GetVersion() && item1->GetClientSoft() == SO_EMULE){
					return item2->DbgGetFullClientSoftVer().CompareNoCase( item1->DbgGetFullClientSoftVer());
				}
				else {
					return item2->GetVersion() - item1->GetVersion();
				}
			else
				return item1->GetClientSoft() - item2->GetClientSoft();
		case 105:
			if(item1->GetClientSoft() == item2->GetClientSoft())
				if(item2->GetVersion() == item1->GetVersion() && item1->GetClientSoft() == SO_EMULE){
					return item2->DbgGetFullClientSoftVer().CompareNoCase( item1->DbgGetFullClientSoftVer());
				}
				else {
					return item1->GetVersion() - item2->GetVersion();
				}
			else
				return item2->GetClientSoft() - item1->GetClientSoft();
		// [TPT] - Morph sort
		case 6:
			if( item1->socket && item2->socket )
				return item1->socket->IsConnected()-item2->socket->IsConnected();
			else if( !item1->socket )
				return -1;
			else
				return 1;
		case 106:
			if( item1->socket && item2->socket )
				return item2->socket->IsConnected()-item1->socket->IsConnected();
			else if( !item2->socket )
				return -1;
			else
				return 1;
		case 7:
			return memcmp(item1->GetUserHash(), item2->GetUserHash(), 16);
		case 107:
			return memcmp(item2->GetUserHash(), item1->GetUserHash(), 16);
		//<<< [TPT] - eWombat SNAFU v2
		case 8: 
			return item1->GetBanSort() - item2->GetBanSort();
		case 108: 
			return item2->GetBanSort() - item1->GetBanSort();
		//<<< [TPT] - eWombat SNAFU v2
		// [TPT] - IP Country
		case 9:
			if(item1->GetCountryIP() && item2->GetCountryIP())
				return theApp.ip2country->Compare(item1->GetCountryIP(), item2->GetCountryIP());
			else if(item1->GetCountryIP())
				return 1;
			else
				return -1;
		case 109:
			if(item1->GetCountryIP() && item2->GetCountryIP())
				return theApp.ip2country->Compare(item2->GetCountryIP(), item1->GetCountryIP());
			else if(item2->GetCountryIP())
				return 1;
			else
				return -1;
		// [TPT] - IP Country

		default:
			return 0;
	}
}

void CClientListCtrl::ShowSelectedUserDetails()
{
	POINT point;
	::GetCursorPos(&point);
	CPoint p = point; 
    ScreenToClient(&p); 
    int it = HitTest(p); 
    if (it == -1)
		return;

	SetItemState(-1, 0, LVIS_SELECTED);
	SetItemState(it, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	SetSelectionMark(it);   // display selection mark correctly!

	CUpDownClient* client = (CUpDownClient*)GetItemData(GetSelectionMark());
	if (client){
		client->GetDetailDialogInterface()->OpenDetailDialog(this);	// [TPT] - SLUGFILLER: modelessDialogs
	}
}

void CClientListCtrl::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult) {
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1) {
		CUpDownClient* client = (CUpDownClient*)GetItemData(iSel);
		if (client){
			client->GetDetailDialogInterface()->OpenDetailDialog(this);	// [TPT] - SLUGFILLER: modelessDialogs
		}
	}
	*pResult = 0;
}

void CClientListCtrl::OnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
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
			const CUpDownClient* pClient = reinterpret_cast<CUpDownClient*>(pDispInfo->item.lParam);
			if (pClient != NULL){
				switch (pDispInfo->item.iSubItem){
					case 0:
						if (pClient->GetUserName() != NULL && pDispInfo->item.cchTextMax > 0){
							_tcsncpy(pDispInfo->item.pszText, pClient->GetUserName(), pDispInfo->item.cchTextMax);
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
