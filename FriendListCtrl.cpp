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
#include "FriendListCtrl.h"
#include "friend.h"
#include "ClientDetailDialog.h"
#include "Addfriend.h"
#include "FriendList.h"
#include "emuledlg.h"
#include "ClientList.h"
#include "OtherFunctions.h"
#include "UpDownClient.h"
#include "ListenSocket.h"
#include "MenuCmds.h"
#include "ChatWnd.h"
#include "MenuXP.h"// [TPT] - New Menu Styles
#include "log.h" // [TPT]
// [TPT] - Announ: -Friend eLinks-
#include "ED2KLink.h"
// [TPT] - Announ: -Friend eLinks-
#include "mod_version.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif


// CFriendListCtrl

IMPLEMENT_DYNAMIC(CFriendListCtrl, CMuleListCtrl)
CFriendListCtrl::CFriendListCtrl()
{
}

CFriendListCtrl::~CFriendListCtrl()
{
}


BEGIN_MESSAGE_MAP(CFriendListCtrl, CMuleListCtrl)
	ON_WM_CONTEXTMENU()
	ON_WM_MEASUREITEM()// [TPT] - New Menu Styles
	ON_WM_SYSCOLORCHANGE()
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnLvnColumnclick)
END_MESSAGE_MAP()



// CFriendListCtrl message handlers

void CFriendListCtrl::Init()
{
	SetExtendedStyle(LVS_EX_FULLROWSELECT);

	SetDoubleBufferStyle();//[TPT] - Double Buffer style in lists

	// [TPT] - Announ: -Friend eLinks-
	ModifyStyle(LVS_SINGLESEL,0);
	// [TPT] - Announ: -Friend eLinks-

	RECT rcWindow;
	GetWindowRect(&rcWindow);
	InsertColumn(0, GetResString(IDS_QL_USERNAME), LVCFMT_LEFT, rcWindow.right - rcWindow.left - 4, 0);
	InsertColumn(1, GetResString(IDS_FRIEND_STATUS), LVCFMT_LEFT, 110, 1);// [TPT] - Friend State Column
	SetAllIcons();
	theApp.friendlist->SetWindow(this);
	// [TPT] - Friend State Column
	LoadSettings(CPreferences::tableFriendList);
	// Barry - Use preferred sort order from preferences
	int sortItem = thePrefs.GetColumnSortItem(CPreferences::tableFriendList);
	bool sortAscending = thePrefs.GetColumnSortAscending(CPreferences::tableFriendList);
	SetSortArrow(sortItem, sortAscending);
	// [TPT] - SLUGFILLER: multiSort - load multiple params
	for (int i = thePrefs.GetColumnSortCount(CPreferences::tableFriendList); i > 0; ) {
		i--;
		sortItem = thePrefs.GetColumnSortItem(CPreferences::tableFriendList, i);
		sortAscending = thePrefs.GetColumnSortAscending(CPreferences::tableFriendList, i);
		SortItems(SortProc, sortItem + (sortAscending ? 0:100));
	}
	// [TPT] - SLUGFILLER: multiSort
	// [TPT] - Friend State Column
}

void CFriendListCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
}

void CFriendListCtrl::SetAllIcons()
{
	CImageList iml;
	iml.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	iml.SetBkColor(CLR_NONE);
	iml.Add(CTempIconLoader(_T("FriendNoClient")));
	iml.Add(CTempIconLoader(_T("FriendWithClient")));
	iml.Add(CTempIconLoader(_T("FriendConnected")));
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) == 0 );
	HIMAGELIST himlOld = ApplyImageList(iml.Detach());
	if (himlOld)
		ImageList_Destroy(himlOld);
}

void CFriendListCtrl::Localize()
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
	CString strRes;

	strRes = GetResString(IDS_QL_USERNAME);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(0, &hdi);
	strRes.ReleaseBuffer();
	
	// [TPT] - Friend State Column 
	strRes = GetResString(IDS_FRIEND_STATUS);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(1, &hdi);
	strRes.ReleaseBuffer();
	// [TPT] - Friend State Column end

	int iItems = GetItemCount();
	for (int i = 0; i < iItems; i++)
		UpdateFriend(i, (CFriend*)GetItemData(i));
}

// [TPT] - Rework
void CFriendListCtrl::UpdateFriend(int iItem, const CFriend* pFriend)
{
	if (pFriend == NULL) return;
	SetItemText(iItem, 0, pFriend->m_strName);

	int iImage;	
	CUpDownClient* client = pFriend->GetLinkedClient(); 
    if (client == NULL)
	{
		iImage = 0;
		SetItemText(iItem, 1, GetResString(IDS_FRIEND_STATE_NOCLIENT)); // [TPT] - Friend State Column 
	}
	else if (client && client->socket && client->socket->IsConnected())
	{	
		iImage = 2;
		SetItemText(iItem, 1, GetResString(IDS_FRIEND_STATE_CONECTED)); // [TPT] - Friend State Column 
	}
	else
	{
		iImage = 1;
		SetItemText(iItem, 1, GetResString(IDS_FRIEND_STATE_CLIENT)); // [TPT] - Friend State Column 
	}

	SetItem(iItem, 0, LVIF_IMAGE, 0, iImage, 0, 0, 0, 0);

	if (pFriend && pFriend->GetFriendSlot())
		SetItemState(iItem, LVIS_GLOW, LVIS_GLOW); //[TPT] - Remark friend slot
	else
		SetItemState(iItem, 0, LVIS_GLOW);
}
// [TPT] - Rework

void CFriendListCtrl::AddFriend(const CFriend* pFriend)
{
	if (!theApp.emuledlg->IsRunning())	
 	     return;
	int iItem = InsertItem(LVIF_TEXT|LVIF_PARAM,GetItemCount(),pFriend->m_strName,0,0,0,(LPARAM)pFriend);
	if (iItem >= 0)
		UpdateFriend(iItem, pFriend);
	theApp.emuledlg->chatwnd->UpdateFriendlistCount(theApp.friendlist->GetCount());
}

void CFriendListCtrl::RemoveFriend(const CFriend* pFriend)
{
	if (!theApp.emuledlg->IsRunning())	
 	     return;
	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)pFriend;
	int iItem = FindItem(&find);
	if (iItem != -1)
		DeleteItem(iItem);
	theApp.emuledlg->chatwnd->UpdateFriendlistCount(theApp.friendlist->GetCount());
}

void CFriendListCtrl::RefreshFriend(const CFriend* pFriend)
{
	if (!theApp.emuledlg->IsRunning())	
 	     return;
	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)pFriend;
	int iItem = FindItem(&find);
	if (iItem != -1)
		UpdateFriend(iItem, pFriend);
}

// [TPT] - Announ: -Friend eLinks-
void CFriendListCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	// [TPT] - New Menu Styles BEGIN

	//Menu Configuration
	CMenuXP	*pMenu = new CMenuXP;
	pMenu->CreatePopupMenu();
	pMenu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
	pMenu->AddSideBar(new CMenuXPSideBar(17, MOD_VERSION));
	pMenu->SetSideBarStartColor(RGB(255,0,0));
	pMenu->SetSideBarEndColor(RGB(255,128,0));
	pMenu->SetSelectedBarColor(RGB(242,120,114));

	UINT nSel = GetSelectedCount();
	CFriend* cur_friend = NULL;
	if ( nSel == 1U )
	{
		POSITION posSel = GetFirstSelectedItemPosition();
		cur_friend = (CFriend*)GetItemData( GetNextSelectedItem(posSel) );
		pMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_DETAIL, GetResString(IDS_SHOWDETAILS), theApp.LoadIcon(_T("details"), 16, 16)));
		// [TPT] - itsonlyme:clientDetails
		pMenu->AppendODMenu(MF_STRING | ((cur_friend==NULL || (cur_friend && cur_friend->GetLinkedClient() && !cur_friend->GetLinkedClient()->GetViewSharedFilesSupport())) ? MF_GRAYED : MF_ENABLED), new CMenuXPText(MP_SHOWLIST, GetResString(IDS_VIEWFILES), theApp.LoadIcon(_T("seeshared2"), 16, 16)));
		pMenu->AppendODMenu(MF_STRING | ((cur_friend==NULL || (cur_friend && cur_friend->GetLinkedClient() && !cur_friend->GetLinkedClient()->GetViewSharedFilesSupport())) ? MF_GRAYED : MF_ENABLED), new CMenuXPText(MP_LIST_REQUESTED_FILES, GetResString(IDS_LIST_REQ_FILES), theApp.LoadIcon(_T("requestedFiles2"), 16, 16)));
		pMenu->AppendSeparator();
		// [TPT] - itsonlyme:clientDetails
		pMenu->SetDefaultItem(MP_DETAIL);
	}
	pMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_ADDFRIEND, GetResString(IDS_ADDFRIEND), theApp.LoadIcon(_T("friend2"), 16, 16)));	
	pMenu->AppendODMenu(MF_STRING | (nSel ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_REMOVEFRIEND, GetResString(IDS_REMOVEFRIEND), theApp.LoadIcon(_T("delete"),16,16)));
	pMenu->AppendODMenu(MF_STRING | (cur_friend ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_MESSAGE, GetResString(IDS_SEND_MSG), theApp.LoadIcon(_T("sendMessage"), 16, 16)));
	// [TPT] - itsonlyme:clientDetails remove - moved up

	pMenu->AppendSeparator();

	pMenu->AppendODMenu(MF_STRING | (theApp.IsEd2kFriendLinkInClipboard() ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_PASTE, GetResString(IDS_PASTE)));
	pMenu->AppendODMenu(MF_STRING | (nSel ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_GETFRIENDED2KLINK, GetResString(IDS_GETFRIENDED2KLINK)));
	pMenu->AppendODMenu(MF_STRING | (nSel ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_GETHTMLFRIENDED2KLINK, GetResString(IDS_GETHTMLFRIENDED2KLINK)));
	pMenu->AppendODMenu(MF_STRING | (nSel ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_GETBBCODEFRIENDED2KLINK, GetResString(IDS_GETBBCODEFRIENDED2KLINK)));
	pMenu->AppendSeparator();
	pMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_FRIENDSLOT, GetResString(IDS_FRIENDSLOT)));
	


	if (cur_friend && cur_friend->GetLinkedClient() && !cur_friend->GetLinkedClient()->HasLowID()){
		pMenu->EnableMenuItem(MP_FRIENDSLOT, MF_ENABLED);
		pMenu->CheckMenuItem(MP_FRIENDSLOT, (cur_friend->GetFriendSlot()) ? MF_CHECKED : MF_UNCHECKED);
	}
	else
		pMenu->EnableMenuItem(MP_FRIENDSLOT, MF_GRAYED);
	
	pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);

	delete pMenu;

	// [TPT] - New Menu Styles END

}
// End -Friend eLinks-

// [TPT] - New Menu Styles BEGIN
void CFriendListCtrl::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	HMENU hMenu = AfxGetThreadState()->m_hTrackingMenu;
	CMenu	*pMenu = CMenu::FromHandle(hMenu);
	pMenu->MeasureItem(lpMeasureItemStruct);
	
	CWnd::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}
// [TPT] - New Menu Styles END

BOOL CFriendListCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	UINT nSel = GetSelectedCount();
	CFriend** ppSelFriends = new CFriend*[nSel];
	POSITION iPos = GetFirstSelectedItemPosition();
	for (int iFrnd = 0; iPos != NULL; ++iFrnd)
		ppSelFriends[iFrnd] = (CFriend*)GetItemData(GetNextSelectedItem(iPos));
	
	switch ( wParam )
	{
	case MP_MESSAGE:
		if ( nSel == 1U && ppSelFriends[0] )
		{
			if ( ppSelFriends[0]->GetLinkedClient() )
				theApp.emuledlg->chatwnd->StartSession(ppSelFriends[0]->GetLinkedClient());
			else
			{
				CUpDownClient* chatclient = new CUpDownClient(0, ppSelFriends[0]->m_nLastUsedPort, ppSelFriends[0]->m_dwLastUsedIP, 0U, 0U, true);
				chatclient->SetUserName(ppSelFriends[0]->m_strName.GetBuffer());
				theApp.clientlist->AddClient(chatclient);
				theApp.emuledlg->chatwnd->StartSession(chatclient);
			}
		}
		break;
	case MP_REMOVEFRIEND:
		for (UINT iFrnd = 0; iFrnd < nSel; ++iFrnd)
		{
			if ( ppSelFriends[iFrnd] )	// Probably redundant check
			{
				theApp.friendlist->RemoveFriend(ppSelFriends[iFrnd]);
				theApp.emuledlg->chatwnd->CheckFriendMessageDetails(); // [TPT] - Remove friend details
			}
		}
		break;
	case MP_ADDFRIEND:
		{
			CAddFriend dialog2; 
			dialog2.DoModal();
		}
		break;
	case MPG_ALTENTER:
	case MP_DETAIL:
		if ( nSel == 1U && ppSelFriends[0] )
			ShowFriendDetails(ppSelFriends[0]);
		break;
	case MP_SHOWLIST:
		if ( nSel == 1U && ppSelFriends[0] )
		{
			if ( ppSelFriends[0]->GetLinkedClient() )
				ppSelFriends[0]->GetLinkedClient()->GetDetailDialogInterface()->OpenDetailDialog(NULL, IDD_BROWSEFILES);	// [TPT] - itsonlyme: viewSharedFiles
			else
			{
				CUpDownClient* newclient = new CUpDownClient(0, ppSelFriends[0]->m_nLastUsedPort, ppSelFriends[0]->m_dwLastUsedIP, 0U, 0U, true);
				newclient->SetUserName(ppSelFriends[0]->m_strName.GetBuffer());
				theApp.clientlist->AddClient(newclient);
				newclient->GetDetailDialogInterface()->OpenDetailDialog(NULL, IDD_BROWSEFILES);	// [TPT] - itsonlyme: viewSharedFiles
			}
		}
		break;
	// [TPT] - itsonlyme:reqFiles START
	case MP_LIST_REQUESTED_FILES: { 
		if ( nSel == 1U && ppSelFriends[0] )
			if (ppSelFriends[0]->GetLinkedClient())
				ppSelFriends[0]->GetLinkedClient()->GetDetailDialogInterface()->OpenDetailDialog(NULL, IDD_REQFILES);	// [TPT] - SLUGFILLER: modelessDialogs
		break;
	}
	// [TPT] - itsonlyme:reqFiles END
	case MP_FRIENDSLOT:
		if ( nSel == 1U && ppSelFriends[0] && ppSelFriends[0]->GetLinkedClient() )
		{
			bool IsAlready = ppSelFriends[0]->GetLinkedClient()->GetFriendSlot();
			theApp.friendlist->RemoveAllFriendSlots();
			if ( !IsAlready )
				ppSelFriends[0]->GetLinkedClient()->SetFriendSlot(true);
		}
		break;
	case MP_PASTE:
		{
			CString link = theApp.CopyTextFromClipboard();
			link.Trim();
			if ( link.IsEmpty() )
				break;

				try{
			CED2KLink* pLink = CED2KLink::CreateLinkFromUrl(link);
				
			if (pLink && pLink->GetKind() == CED2KLink::kFriend )
			{
				// Better with dynamic_cast, but no RTTI enabled in the project
				CED2KFriendLink* pFriendLink = static_cast<CED2KFriendLink*>(pLink);
				uchar userHash[16];
				pFriendLink->GetUserHash(userHash);

				if ( ! theApp.friendlist->IsAlreadyFriend(userHash) )
					theApp.friendlist->AddFriend(userHash, 0U, 0U, 0U, 0U, pFriendLink->GetUserName(), 1U);
				else
				{
					CString msg;
					msg.Format(GetResString(IDS_USER_ALREADY_FRIEND), pFriendLink->GetUserName());
					AddLogLine(true, msg);
				}
			}
		}
				catch(CString strError){
					TCHAR szBuffer[200];
					_sntprintf(szBuffer, ARRSIZE(szBuffer), GetResString(IDS_ERR_INVALIDLINK), strError);
					LogError(LOG_STATUSBAR, GetResString(IDS_ERR_LINKERROR), szBuffer);				}
			}
		break;

	case MP_GETFRIENDED2KLINK:
		{
			CString sCompleteLink;
			for (UINT iFrnd = 0; iFrnd < nSel; ++iFrnd)
			{
				if ( ppSelFriends[iFrnd] && ppSelFriends[iFrnd]->m_dwHasHash )
				{
					CString sLink;
					CED2KFriendLink friendLink(ppSelFriends[iFrnd]->m_strName, ppSelFriends[iFrnd]->m_abyUserhash);
					friendLink.GetLink(sLink);
					if ( !sCompleteLink.IsEmpty() )
						sCompleteLink.Append(_T("\r\n"));
					sCompleteLink.Append(sLink);
				}
			}
			if ( !sCompleteLink.IsEmpty() )
				theApp.CopyTextToClipboard(sCompleteLink);
		}
		break;
	case MP_GETHTMLFRIENDED2KLINK:
		{
			CString sCompleteLink;
			for (UINT iFrnd = 0; iFrnd < nSel; ++iFrnd)
			{
				if ( ppSelFriends[iFrnd] && ppSelFriends[iFrnd]->m_dwHasHash )
				{
					CString sLink;
					CED2KFriendLink friendLink(ppSelFriends[iFrnd]->m_strName, ppSelFriends[iFrnd]->m_abyUserhash);
					friendLink.GetLink(sLink);
					sLink = _T("<a href=\"") + sLink + _T("\">") + StripInvalidFilenameChars(ppSelFriends[iFrnd]->m_strName, true) + _T("</a>");
					if ( !sCompleteLink.IsEmpty() )
						sCompleteLink.Append(_T("\r\n"));
					sCompleteLink.Append(sLink);
				}
			}
			if ( !sCompleteLink.IsEmpty() )
				theApp.CopyTextToClipboard(sCompleteLink);
		}
		break;
	case MP_GETBBCODEFRIENDED2KLINK:
		{
			CString sCompleteLink;
			for (UINT iFrnd = 0; iFrnd < nSel; ++iFrnd)
			{
				if ( ppSelFriends[iFrnd] && ppSelFriends[iFrnd]->m_dwHasHash )
				{
					CString sLink;
					CED2KFriendLink friendLink(ppSelFriends[iFrnd]->m_strName, ppSelFriends[iFrnd]->m_abyUserhash);
					friendLink.GetLink(sLink);

					CString sLinkStrip = theApp.StripBrackets(sLink);
					sLinkStrip.Replace(_T("["), _T("%5B"));					
					sLinkStrip.Replace(_T("]"), _T("%5D"));

					sLink = _T("[URL=") + sLinkStrip + _T("]") + theApp.StripBrackets(StripInvalidFilenameChars(ppSelFriends[iFrnd]->m_strName, true)) + _T("[/URL]");
					if ( !sCompleteLink.IsEmpty() )
						sCompleteLink.Append(_T("\r\n"));
					sCompleteLink.Append(sLink);
				}
			}
			if ( !sCompleteLink.IsEmpty() )
				theApp.CopyTextToClipboard(sCompleteLink);
		}
		break;
	}
	delete[] ppSelFriends;

	return true;
}
// End -Friend eLinks-

// [TPT] - Announ: -Friend eLinks-
/*
BOOL CFriendListCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	CFriend* cur_friend = NULL;
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1) 
		cur_friend = (CFriend*)GetItemData(iSel);
	
	switch (wParam){
		case MP_MESSAGE:{
			if (cur_friend){
				if (cur_friend->GetLinkedClient())
					theApp.emuledlg->chatwnd->StartSession(cur_friend->GetLinkedClient());
				else{
					CUpDownClient* chatclient = new CUpDownClient(0,cur_friend->m_nLastUsedPort,cur_friend->m_dwLastUsedIP,0,0,true);
					chatclient->SetUserName(cur_friend->m_strName);
					theApp.clientlist->AddClient(chatclient);
					theApp.emuledlg->chatwnd->StartSession(chatclient);
				}
			}
			break;
		}
		case MP_REMOVEFRIEND:{
			if (cur_friend){
				theApp.friendlist->RemoveFriend(cur_friend);
				// auto select next item after deleted one.
				if (iSel < GetItemCount()){
					SetSelectionMark(iSel);
					SetItemState(iSel, LVIS_SELECTED, LVIS_SELECTED);
				}
			}
			break;
		}
		case MP_ADDFRIEND:{
			CAddFriend dialog2; 
			dialog2.DoModal();
			break;
		}
		case MPG_ALTENTER:
		case MP_DETAIL:
			if (cur_friend)
				ShowFriendDetails(cur_friend);
			break;
		case MP_SHOWLIST:
		{
			if (cur_friend){
				if (cur_friend->GetLinkedClient())
					cur_friend->GetLinkedClient()->GetDetailDialogInterface()->OpenDetailDialog(NULL, IDD_BROWSEFILES);	// itsonlyme: viewSharedFiles
				else{
					CUpDownClient* newclient = new CUpDownClient(0,cur_friend->m_nLastUsedPort,cur_friend->m_dwLastUsedIP,0,0,true);
					newclient->SetUserName(cur_friend->m_strName);
					theApp.clientlist->AddClient(newclient);
					newclient->GetDetailDialogInterface()->OpenDetailDialog(NULL, IDD_BROWSEFILES);	// itsonlyme: viewSharedFiles
				}
			}
			break;
		}
		// [TPT] - itsonlyme:reqFiles START
		case MP_LIST_REQUESTED_FILES: { 
			if (cur_friend && cur_friend->GetLinkedClient()) {
				cur_friend->GetLinkedClient()->GetDetailDialogInterface()->OpenDetailDialog(NULL, IDD_REQFILES);	// SLUGFILLER: modelessDialogs
			}
			break;
		}
		// [TPT] - itsonlyme:reqFiles END
		case MP_FRIENDSLOT:
		{
			if (cur_friend){
				bool IsAlready;
                IsAlready = cur_friend->GetFriendSlot();
				theApp.friendlist->RemoveAllFriendSlots();
				if( !IsAlready )
                    cur_friend->SetFriendSlot(true);
			}
			break;
		}
	}
	return true;
}*/
// [TPT] - Announ: -Friend eLinks-

void CFriendListCtrl::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1) 
		ShowFriendDetails((CFriend*)GetItemData(iSel));
	*pResult = 0;
}

void CFriendListCtrl::ShowFriendDetails(const CFriend* pFriend)
{
	if (pFriend){
		if (pFriend->GetLinkedClient()){
			pFriend->GetLinkedClient()->GetDetailDialogInterface()->OpenDetailDialog();	// [TPT] - SLUGFILLER: modelessDialogs
		}
		else{
			CAddFriend dlg;
			dlg.m_pShowFriend = const_cast<CFriend*>(pFriend);
			dlg.DoModal();
		}
	}
}

BOOL CFriendListCtrl::PreTranslateMessage(MSG* pMsg) 
{
   	if ( pMsg->message == 260 && pMsg->wParam == 13 && GetAsyncKeyState(VK_MENU)<0 ) {
		PostMessage(WM_COMMAND, MPG_ALTENTER, 0);
		return TRUE;
	}
	else if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_DELETE)
		PostMessage(WM_COMMAND, MP_REMOVEFRIEND, 0);
	else if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_INSERT)
		PostMessage(WM_COMMAND, MP_ADDFRIEND, 0);

	return CMuleListCtrl::PreTranslateMessage(pMsg);
}

void CFriendListCtrl::OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// [TPT] - Friend State Column
	// Barry - Store sort order in preferences
	// Determine ascending based on whether already sorted on this column
	int oldSortItem = thePrefs.GetColumnSortItem(CPreferences::tableFriendList); 

	bool m_oldSortAscending = thePrefs.GetColumnSortAscending(CPreferences::tableFriendList);
	bool sortAscending = (oldSortItem != pNMListView->iSubItem) ? (pNMListView->iSubItem == 0) : !m_oldSortAscending;	

	// Item is column clicked
	int sortItem = pNMListView->iSubItem; 

	// Save new preferences
	thePrefs.SetColumnSortItem(CPreferences::tableFriendList, sortItem);
	thePrefs.SetColumnSortAscending(CPreferences::tableFriendList, sortAscending);

	// Sort table
	SetSortArrow(sortItem, sortAscending);
	SortItems(SortProc, sortItem + (sortAscending ? 0:100));
	// [TPT] - Friend State Column

	*pResult = 0;
}

int CFriendListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CFriend* item1 = (CFriend*)lParam1;
	CFriend* item2 = (CFriend*)lParam2; 

	CString item1Text;// [TPT] - Friend State Column
	CString item2Text;// [TPT] - Friend State Column
	if (item1 == NULL || item2 == NULL)
		return 0;
	
	// [TPT] - Fix Rework
	switch (LOWORD(lParamSort))
	{
		case 0:
			return CompareLocaleStringNoCase(item1->m_strName, item2->m_strName);			
		case 100:
			return CompareLocaleStringNoCase(item2->m_strName, item1->m_strName);			
		// [TPT] - Friend State Column BEGIN
		case 1:
			if(!item1->GetLinkedClient())
				item1Text = GetResString(IDS_FRIEND_STATE_NOCLIENT);
			else if (item1->GetLinkedClient()->socket && item1->GetLinkedClient()->socket->IsConnected())	
				item1Text = GetResString(IDS_FRIEND_STATE_CONECTED);
			else
				item1Text = GetResString(IDS_FRIEND_STATE_CLIENT);			

			if(!item2->GetLinkedClient())
				item2Text = GetResString(IDS_FRIEND_STATE_NOCLIENT);
			else if (item2->GetLinkedClient()->socket && item2->GetLinkedClient()->socket->IsConnected())	
				item2Text = GetResString(IDS_FRIEND_STATE_CONECTED);
			else
				item2Text = GetResString(IDS_FRIEND_STATE_CLIENT);
			
			return CompareLocaleStringNoCase(item2Text, item1Text);
		case 101:
			if(!item1->GetLinkedClient())
				item1Text = GetResString(IDS_FRIEND_STATE_NOCLIENT);
			else if (item1->GetLinkedClient()->socket && item1->GetLinkedClient()->socket->IsConnected())	
				item1Text = GetResString(IDS_FRIEND_STATE_CONECTED);
			else
				item1Text = GetResString(IDS_FRIEND_STATE_CLIENT);			

			if(!item2->GetLinkedClient())
				item2Text = GetResString(IDS_FRIEND_STATE_NOCLIENT);
			else if (item2->GetLinkedClient()->socket && item2->GetLinkedClient()->socket->IsConnected())	
				item2Text = GetResString(IDS_FRIEND_STATE_CONECTED);
			else
				item2Text = GetResString(IDS_FRIEND_STATE_CLIENT);
			
			return CompareLocaleStringNoCase(item1Text, item2Text);
		// [TPT] - Friend State Column End
		default:
			return 0;
	}	
	// [TPT] - Fix Rework	
}

void CFriendListCtrl::UpdateList()
{
	theApp.emuledlg->chatwnd->UpdateFriendlistCount(theApp.friendlist->GetCount());
	SortItems(SortProc, MAKELONG(GetSortItem(), (GetSortAscending() ? 0 : 0x0001)));
}

// [TPT] - Announ: -Friend eLinks-
bool CFriendListCtrl::AddEmfriendsMetToList(const CString& strFile)
{
	ShowWindow(SW_HIDE);
	bool ret = theApp.friendlist->AddEmfriendsMetToList(strFile);
	theApp.friendlist->ShowFriends();
	UpdateList();
	ShowWindow(SW_SHOW);
	return ret;
}
// End -Friend eLinks-

//[TPT] - Double buffer style in lists
//TODO: I have done in this way because in future could be an option
void CFriendListCtrl::SetDoubleBufferStyle()
{
	if((_AfxGetComCtlVersion() >= MAKELONG(0, 6)) && thePrefs.GetDoubleBufferStyle())	
		SetExtendedStyle(GetExtendedStyle() | 0x00010000 /*LVS_EX_DOUBLEBUFFER*/);
	else
		if((GetExtendedStyle() & 0x00010000 /*LVS_EX_DOUBLEBUFFER*/) != 0)
			SetExtendedStyle(GetExtendedStyle() ^ 0x00010000);//XOR: delete the style if present
}