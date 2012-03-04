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
#include "QueueListCtrl.h"
#include "OtherFunctions.h"
#include "MenuCmds.h"
#include "ClientDetailDialog.h"
#include "Exceptions.h"
#include "KademliaWnd.h"
#include "emuledlg.h"
#include "FriendList.h"
#include "UploadQueue.h"
#include "UpDownClient.h"
#include "TransferWnd.h"
#include "MemDC.h"
#include "SharedFileList.h"
#include "ClientCredits.h"
#include "PartFile.h"
#include "ChatWnd.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "Kademlia/Kademlia/Prefs.h"
#include "kademlia/net/KademliaUDPListener.h"
#include "Log.h"
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


// CQueueListCtrl

IMPLEMENT_DYNAMIC(CQueueListCtrl, CMuleListCtrl)

CQueueListCtrl::CQueueListCtrl()
	: CListCtrlItemWalk(this)
{
	// Barry - Refresh the queue every 10 secs
	VERIFY( (m_hTimer = ::SetTimer(NULL, NULL, 10000, QueueUpdateTimer)) != NULL );
	if (thePrefs.GetVerbose() && !m_hTimer)
		AddDebugLogLine(true,_T("Failed to create 'queue list control' timer - %s"),GetErrorMessage(GetLastError()));
}

void CQueueListCtrl::Init()
{
	CImageList ilDummyImageList; //dummy list for getting the proper height of listview entries
	ilDummyImageList.Create(1, theApp.GetSmallSytemIconSize().cy,theApp.m_iDfltImageListColorFlags|ILC_MASK, 1, 1); 
	SetImageList(&ilDummyImageList, LVSIL_SMALL);
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) == 0 );
	ilDummyImageList.Detach();

	SetExtendedStyle(LVS_EX_FULLROWSELECT);
	InsertColumn(0,GetResString(IDS_QL_USERNAME),LVCFMT_LEFT,150,0);
	InsertColumn(1,GetResString(IDS_FILE),LVCFMT_LEFT,275,1);
	InsertColumn(2,GetResString(IDS_FILEPRIO),LVCFMT_LEFT,110,2);
	InsertColumn(3,GetResString(IDS_QL_RATING),LVCFMT_LEFT,60,3);
	InsertColumn(4,GetResString(IDS_SCORE),LVCFMT_LEFT,60,4);
	InsertColumn(5,GetResString(IDS_ASKED),LVCFMT_LEFT,60,5);
	InsertColumn(6,GetResString(IDS_LASTSEEN),LVCFMT_LEFT,110,6);
	InsertColumn(7,GetResString(IDS_ENTERQUEUE),LVCFMT_LEFT,110,7);
	InsertColumn(8,GetResString(IDS_BANNED),LVCFMT_LEFT,60,8);
	InsertColumn(9,GetResString(IDS_UPSTATUS),LVCFMT_LEFT,100,9);
	InsertColumn(10,GetResString(IDS_CLIENTSOFTWARE),LVCFMT_LEFT,150,10);	// [TPT] - itsonlyme: clientSoft
	InsertColumn(11,GetResString(IDS_TOTAL_UP_DL),LVCFMT_LEFT,100,11); // [TPT] - Total UL/DL	
	InsertColumn(12,GetResString(IDS_COUNTRY),LVCFMT_LEFT,100,12);	// [TPT] - IP Country
	// [TPT] - WebCache 
	InsertColumn(13,GetResString(IDS_WEBCACHE_COLUMN),LVCFMT_LEFT, 100); //JP Webcache column
	// [TPT] - WebCache 

	SetAllIcons();
	Localize();
	LoadSettings(CPreferences::tableQueue);
	// Barry - Use preferred sort order from preferences
	int sortItem = thePrefs.GetColumnSortItem(CPreferences::tableQueue);
	bool sortAscending = thePrefs.GetColumnSortAscending(CPreferences::tableQueue);
	SetSortArrow(sortItem, sortAscending);
	// [TPT] - SLUGFILLER: multiSort - load multiple params
	for (int i = thePrefs.GetColumnSortCount(CPreferences::tableQueue); i > 0; ) {
		i--;
		sortItem = thePrefs.GetColumnSortItem(CPreferences::tableQueue, i);
		sortAscending = thePrefs.GetColumnSortAscending(CPreferences::tableQueue, i);
	SortItems(SortProc, sortItem + (sortAscending ? 0:100));
	}
	// [TPT] - SLUGFILLER: multiSort

	if(!thePrefs.GetEnableShowCountryFlags() && !thePrefs.GetEnableShowCountryNames())
		HideIPCountryColumn(true); // [TPT] - IP Country
}

CQueueListCtrl::~CQueueListCtrl()
{
	// Barry - Kill the timer that was created
	try
	{
		if (m_hTimer)
			::KillTimer(NULL, m_hTimer);
	}
	catch(...){
		ASSERT(0);
	}
}

void CQueueListCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
}

void CQueueListCtrl::SetAllIcons()
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
	imagelist.Add(CTempIconLoader(_T("ClientAMule")));//11
	imagelist.Add(CTempIconLoader(_T("ClientAMulePlus")));//12
	imagelist.Add(CTempIconLoader(_T("ClientLPhant")));//13
	imagelist.Add(CTempIconLoader(_T("ClientLPhantPlus")));//14
	imagelist.Add(CTempIconLoader(_T("IDI_SNAFU"))); //15 [TPT] - eWombat SNAFU v2
	imagelist.SetOverlayImage(imagelist.Add(CTempIconLoader(_T("ClientSecureOvl"))), 1);//16
	// [TPT] - Own icon
	imagelist.Add(CTempIconLoader(_T("PHOENIX")));//17
	imagelist.Add(CTempIconLoader(_T("PHOENIXPLUS")));//18

}

void CQueueListCtrl::Localize()
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

		strRes = GetResString(IDS_FILE);
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(1, &hdi);
		strRes.ReleaseBuffer();

		strRes = GetResString(IDS_FILEPRIO);
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(2, &hdi);
		strRes.ReleaseBuffer();

		strRes = GetResString(IDS_QL_RATING);
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(3, &hdi);
		strRes.ReleaseBuffer();

		strRes = GetResString(IDS_SCORE);
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(4, &hdi);
		strRes.ReleaseBuffer();

		strRes = GetResString(IDS_ASKED);
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(5, &hdi);
		strRes.ReleaseBuffer();

		strRes = GetResString(IDS_LASTSEEN);
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(6, &hdi);
		strRes.ReleaseBuffer();

		strRes = GetResString(IDS_ENTERQUEUE);
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(7, &hdi);
		strRes.ReleaseBuffer();

		strRes = GetResString(IDS_BANNED);
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(8, &hdi);
		strRes.ReleaseBuffer();
		
		strRes = GetResString(IDS_UPSTATUS);
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(9, &hdi);
		strRes.ReleaseBuffer();

		// [TPT] - itsonlyme: clientSoft
		strRes = GetResString(IDS_CLIENTSOFTWARE);
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(10, &hdi);
		strRes.ReleaseBuffer();
		// [TPT] - itsonlyme: clientSoft END

		// [TPT] - Total UL/DL
		strRes = GetResString(IDS_TOTAL_UP_DL);
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(11, &hdi);
		strRes.ReleaseBuffer();

		// [TPT] - IP Country
		strRes = GetResString(IDS_COUNTRY);;
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(12, &hdi);
		strRes.ReleaseBuffer();
		// [TPT] - IP Country
				
		// [TPT] - WebCache 
		strRes = GetResString(IDS_WEBCACHE_COLUMN);
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(13, &hdi);
		strRes.ReleaseBuffer();
		// [TPT] - WebCache 				
	}
}

void CQueueListCtrl::AddClient(/*const*/ CUpDownClient* client, bool resetclient)
{
	if (resetclient && client){
		// [TPT] - SUQWT
		//client->SetWaitStartTime();
		// [TPT] - SUQWT
		client->SetAskedCount(1);
	}

	if (!theApp.emuledlg->IsRunning())
		return;
	if (thePrefs.IsQueueListDisabled())
		return;

	int iItemCount = GetItemCount();
	int iItem = InsertItem(LVIF_TEXT|LVIF_PARAM,iItemCount,LPSTR_TEXTCALLBACK,0,0,0,(LPARAM)client);
	Update(iItem);
	theApp.emuledlg->transferwnd->UpdateListCount(2, iItemCount+1);
}

void CQueueListCtrl::RemoveClient(const CUpDownClient* client)
{
	if (!theApp.emuledlg->IsRunning())
		return;

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)client;
	sint32 result = FindItem(&find);
	if (result != -1){
		DeleteItem(result);
		theApp.emuledlg->transferwnd->UpdateListCount(2);
	}
}

void CQueueListCtrl::RefreshClient(const CUpDownClient* client)
{
	// There is some type of timing issue here.. If you click on item in the queue or upload and leave
	// the focus on it when you exit the cient, it breaks on line 854 of emuleDlg.cpp.. 
	// I added this IsRunning() check to this function and the DrawItem method and
	// this seems to keep it from crashing. This is not the fix but a patch until
	// someone points out what is going wrong.. Also, it will still assert in debug mode..
	if (!theApp.emuledlg->IsRunning())
		return;
	// [TPT] - MORPH START - SiRoB, Don't Refresh item if not needed
	if( theApp.emuledlg->activewnd != theApp.emuledlg->transferwnd || theApp.emuledlg->transferwnd->queuelistctrl.IsWindowVisible() == false )
		return;
	// [TPT] - MORPH END   - SiRoB, Don't Refresh item if not needed
	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)client;
	sint16 result = FindItem(&find);
	if (result != -1)
		Update(result);
}

#define DLC_DT_TEXT (DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS)

void CQueueListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
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
	//RECT cur_rec = lpDrawItemStruct->rcItem; // [TPT] - Morph. Don't draw hidden Rect
	COLORREF crOldTextColor = dc.SetTextColor(m_crWindowText);

	int iOldBkMode;
	if (m_crWindowTextBk == CLR_NONE){
		DefWindowProc(WM_ERASEBKGND, (WPARAM)(HDC)dc, 0);
		iOldBkMode = dc.SetBkMode(TRANSPARENT);
	}
	else
		iOldBkMode = OPAQUE;

	CString Sbuffer;
	CKnownFile* file = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
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
						// [TPT] - IP Country
						CxImage imagen = theApp.ip2country->GetCountryFlag(client->GetCountryIP());
						if(thePrefs.GetEnableShowCountryFlags())
						{
							if (imagen.IsEnabled())
							{
								imagen.Draw(dc, cur_rec.left, cur_rec.top+1, 16, 16);
								cur_rec.left +=20;
							}
						}
						// [TPT] - IP Country
						uint8 image;
						// [TPT] - eWombat SNAFU v2
						if (client->IsSnafu())
							image = 15;
						// [TPT] - eWombat SNAFU v2
						else if (client->IsFriend())
							image = 4;
						else if (client->GetClientSoft() == SO_EDONKEYHYBRID){
							if (client->credits && client->credits->GetScoreRatio(client->GetIP()) > 1) // [TPT] - Patch
								image = 8;
							else
								image = 7;
						}
						else if (client->GetClientSoft() == SO_MLDONKEY){
							if (client->credits && client->credits->GetScoreRatio(client->GetIP()) > 1) // [TPT] - Patch
								image = 6;
							else
								image = 5;
						}
						else if (client->GetClientSoft() == SO_SHAREAZA){
							if(client->credits && client->credits->GetScoreRatio(client->GetIP()) > 1) // [TPT] - Patch
								image = 10;
							else
								image = 9;
						}
						else if (client->GetClientSoft() == SO_AMULE){
							if(client->credits && client->credits->GetScoreRatio(client->GetIP()) > 1) // [TPT] - Patch
								image = 12;
							else
								image = 11;
						}
						else if (client->GetClientSoft() == SO_LPHANT){
							if(client->credits && client->credits->GetScoreRatio(client->GetIP()) > 1) // [TPT] - Patch
								image = 14;
							else
								image = 13;
						}
						// [TPT] - Own icon
						else if (client->ExtProtocolAvailable()){
							if(client->credits && client->credits->GetScoreRatio(client->GetIP()) > 1) // [TPT] - Patch
								image = (client->GetpHoeniXClient()) ? 18 : 3;
							else
								image = (client->GetpHoeniXClient()) ? 17 : 1;
						}
						// [TPT] - Own icon
						else{
							if (client->credits && client->credits->GetScoreRatio(client->GetIP()) > 1) // [TPT] - Patch
								image = 2;
							else
								image = 0;
						}
	
						POINT point = {cur_rec.left, cur_rec.top+1};
						imagelist.Draw(dc,image, point, ILD_NORMAL | ((client->Credits() && client->Credits()->GetCurrentIdentState(client->GetIP()) == IS_IDENTIFIED) ? INDEXTOOVERLAYMASK(1) : 0));
						Sbuffer = client->GetUserName();
						cur_rec.left +=20;
						dc->DrawText(Sbuffer,Sbuffer.GetLength(),&cur_rec,DLC_DT_TEXT);
						if(thePrefs.GetEnableShowCountryFlags() && imagen.IsEnabled())
							cur_rec.left -=40;
						else
							cur_rec.left -=20;
						break;
					}
					case 1:
						if(file)
							Sbuffer = file->GetFileName();
						else
							Sbuffer = _T("?");
						// START - [TPT] - enkeyDEV(ColdShine) -FileIcons-
						{
							int toRight = 0;
							int iIconDrawWidth = theApp.GetSmallSytemIconSize().cx + 3;
							if (file) {
								int iImage = theApp.GetFileTypeSystemImageIdx(file->GetFileName());
								if (theApp.GetSystemImageList() != NULL) {
									::ImageList_Draw(theApp.GetSystemImageList(), iImage, dc->GetSafeHdc(), cur_rec.left, cur_rec.top+((cur_rec.bottom - cur_rec.top - ::GetSystemMetrics(SM_CYSMICON)) >> 1), ILD_TRANSPARENT);
									toRight += iIconDrawWidth;
									cur_rec.left += iIconDrawWidth;
								}
							}
							dc->DrawText(Sbuffer, Sbuffer.GetLength(), &cur_rec, DLC_DT_TEXT);
							cur_rec.left -= toRight;
						}
						// END - [TPT] - enkeyDEV(ColdShine) -FileIcons-
						break;
					case 2:
						if(file){
							switch (file->GetUpPriority()) {
								case PR_VERYLOW : {
									Sbuffer = GetResString(IDS_PRIOVERYLOW);
									break; }
								case PR_LOW : {
									if( file->IsAutoUpPriority() )
										Sbuffer = GetResString(IDS_PRIOAUTOLOW);
									else
										Sbuffer = GetResString(IDS_PRIOLOW);
									break; }
								case PR_NORMAL : {
									if( file->IsAutoUpPriority() )
										Sbuffer = GetResString(IDS_PRIOAUTONORMAL);
									else
										Sbuffer = GetResString(IDS_PRIONORMAL);
									break; }
								case PR_HIGH : {
									if( file->IsAutoUpPriority() )
										Sbuffer = GetResString(IDS_PRIOAUTOHIGH);
									else
										Sbuffer = GetResString(IDS_PRIOHIGH);
									break; }
								case PR_VERYHIGH : {
									Sbuffer = GetResString(IDS_PRIORELEASE);
									break; }
								default:
									Sbuffer.Empty();
							}
							// [TPT] - Powershare
							if(file->GetPowerShared()) {
								CString tempString = GetResString(IDS_POWERSHARE_PREFIX);
								tempString.Append(_T(" "));
								tempString.Append(Sbuffer);
								Sbuffer.Empty(); //MORPH - HotFix by SiRoB, ZZ Upload System
								Sbuffer = tempString;
							}
							// [TPT] - Powershare END
						}
						else
							Sbuffer = _T("?");
						break;
					case 3:
						Sbuffer.Format(_T("%i"),client->GetScore(false,false,true));
						break;
					case 4:
						if (client->HasLowID()){
							if (client->m_bAddNextConnect)
								Sbuffer.Format(_T("%i ****"),client->GetScore(false));
							else
	                            Sbuffer.Format(_T("%i LowID"),client->GetScore(false));
						}
						else
							Sbuffer.Format(_T("%i"),client->GetScore(false));
						// [TPT] - Powershare
						if (client->GetPowerShared())
							Sbuffer.Append(_T(" PS"));
						break;
					case 5:
						Sbuffer.Format(_T("%i"),client->GetAskedCount());
						break;
					case 6:
						Sbuffer = CastSecondsToHM((::GetTickCount() - client->GetLastUpRequest())/1000);
						break;
					case 7:
						Sbuffer = CastSecondsToHM((::GetTickCount() - client->GetWaitStartTime())/1000);
						break;
					//<<< [TPT] - eWombat SNAFU v2
					case 8:
						//if(client->IsBanned())
						//	Sbuffer = GetResString(IDS_YES);
						//else
						//	Sbuffer = GetResString(IDS_NO);
						Sbuffer = client->GetBanStr();
						break;
					//<<< eWombat [SNAFU]
					case 9:
						if( client->GetUpPartCount()){
							cur_rec.bottom--;
							cur_rec.top++;
							client->DrawUpStatusBar(dc,&cur_rec,false,thePrefs.UseFlatBar());
							cur_rec.bottom++;
							cur_rec.top--;
						}
						break;
					case 10:
						Sbuffer = client->DbgGetFullClientSoftVer();
						if (Sbuffer.IsEmpty())
							Sbuffer = GetResString(IDS_UNKNOWN);
						break;				
					// [TPT] - Total UL/DL
					case 11:
						if (client->Credits()){
							Sbuffer.Format(_T("%s/%s"),
								CastItoXBytes((float)client->Credits()->GetUploadedTotal(), false, false),
								CastItoXBytes((float)client->Credits()->GetDownloadedTotal(), false, false));
						   }
						else{
								Sbuffer.Format(_T("%s/%s"),_T("?"),_T("?"));
						}
						break;
					// [TPT] - Total UL/DL
					case 12: {
						if (thePrefs.GetEnableShowCountryNames())
						{
							Sbuffer = theApp.ip2country->GetCountryFromIP(client->GetCountryIP());
						}
						else
							Sbuffer = _T("N/A");
						// [TPT] - IP Country					
						break;
						   }
					// [TPT] - WebCache ////////////////////////////////////////////////////////////////////////////////////
			//JP Webcache START
					case 13: 
						{
						if (client->SupportsWebCache())
						{
							Sbuffer = client->GetWebCacheName();
							if (client->IsBehindOurWebCache())
								dc->SetTextColor(RGB(0, 180, 0)); //if is behind our webcache display green
							else if (Sbuffer != _T(""))
								dc->SetTextColor(RGB(255, 0, 0)); // if webcache info is there but not our own set red
							else
								Sbuffer = _T("no proxy set");	// if no webcache info colour is black
						   }
						else
							Sbuffer = _T("");
						dc->DrawText(Sbuffer,Sbuffer.GetLength(),&cur_rec,DLC_DT_TEXT);
	 					dc->SetTextColor(RGB(0, 0, 0));
						break;
					}
					//JP Webcache END
				}
				if( iColumn != 0 && iColumn != 1 && iColumn != 9 && iColumn != 13 ) // [TPT] - enkeyDEV(ColdShine) -FileIcons- // [TPT] - IP Country
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

BEGIN_MESSAGE_MAP(CQueueListCtrl, CMuleListCtrl)
	ON_WM_CONTEXTMENU()
	ON_WM_MEASUREITEM()// [TPT] - New Menu Styles
	ON_WM_SYSCOLORCHANGE()
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
	//ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo) // [TPT] - MFCK [addon] - New Tooltips [Rayita]
END_MESSAGE_MAP()

// CQueueListCtrl message handlers
// [TPT] - New Menu Styles BEGIN
void CQueueListCtrl::OnContextMenu(CWnd* pWnd, CPoint point)
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	CUpDownClient* client = (iSel != -1) ? (CUpDownClient*)GetItemData(iSel) : NULL;

	//Menu Configuration
	CMenuXP	*pMenu = new CMenuXP;
	pMenu->CreatePopupMenu();
	pMenu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
	pMenu->AddSideBar(new CMenuXPSideBar(17, MOD_VERSION));
	pMenu->SetSideBarStartColor(RGB(255,0,0));
	pMenu->SetSideBarEndColor(RGB(255,128,0));
	pMenu->SetSelectedBarColor(RGB(242,120,114));

	pMenu->AppendODMenu(MF_STRING | (client ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_DETAIL, GetResString(IDS_SHOWDETAILS), theApp.LoadIcon(_T("details"), 16, 16)));
	// [TPT] - itsonlyme:clientDetails
	pMenu->AppendODMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetViewSharedFilesSupport()) ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_SHOWLIST, GetResString(IDS_VIEWFILES), theApp.LoadIcon(_T("seeshared2"), 16, 16)));
	pMenu->AppendODMenu(MF_STRING | (client ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_LIST_REQUESTED_FILES, GetResString(IDS_LIST_REQ_FILES), theApp.LoadIcon(_T("requestedFiles2"), 16, 16)));
	pMenu->AppendSeparator();
	// [TPT] - itsonlyme:clientDetails
	pMenu->AppendODMenu(MF_STRING | ((client && client->IsEd2kClient() && !client->IsFriend()) ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_ADDFRIEND, GetResString(IDS_ADDFRIEND), theApp.LoadIcon(_T("friend2"), 16, 16)));
	if (client && client->IsFriend() && !client->HasLowID())
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
	
	pMenu->AppendODMenu(MF_STRING | ((client && client->IsEd2kClient()) ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_MESSAGE, GetResString(IDS_SEND_MSG), theApp.LoadIcon(_T("sendMessage"), 16, 16)));
	
	//<<< [TPT] - eWombat SNAFU v2	
	if(client && (client->IsSnafu() || client->IsSUIFailed() || client->IsBanned()))
			{
				pMenu->AppendSeparator();
				if (client->IsSnafu() || client->IsSUIFailed())
				{
				if (theApp.uploadqueue->IsOnUploadQueue(client))
					pMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_REMOVEUL, GetResString(IDS_REMOVEUL)));
				}
				
		if(!client->IsSUIFailed() && (client->IsBanned() || client->IsSnafu()))
		{
				pMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_UNBAN, GetResString(IDS_UNBAN)));
		}
	}	

	// [TPT] - itsonlyme:clientDetails remove - moved up	
	if (Kademlia::CKademlia::isRunning() && !Kademlia::CKademlia::isConnected())
	{
		pMenu->AppendODMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetKadPort()!=0) ? MF_ENABLED : MF_GRAYED), new CMenuXPText(MP_BOOT, GetResString(IDS_BOOTSTRAP), theApp.LoadIcon(_T("boostrap2"), 16, 16)));
	}
	pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);

	delete pMenu;
}
// [TPT] - New Menu Styles END


// [TPT] - New Menu Styles BEGIN
void CQueueListCtrl::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	HMENU hMenu = AfxGetThreadState()->m_hTrackingMenu;
	CMenu	*pMenu = CMenu::FromHandle(hMenu);
	pMenu->MeasureItem(lpMeasureItemStruct);
	
	CWnd::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}
// [TPT] - New Menu Styles END

BOOL CQueueListCtrl::OnCommand(WPARAM wParam,LPARAM lParam )
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
			{
				if (theApp.friendlist->AddFriend(client))
					Update(iSel);
				RefreshClient(client);	// [TPT] - itsonlyme: displayOptions
				break;
			}
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
		    //<<< [TPT] - eWombat SNAFU v2
			case MP_UNBAN:
				if (client!=NULL)
					{
					if (client->IsSnafu())
						{
						client->UnSnafu();
						client->SnafuExclude();
						}
				if( client->IsBanned() )
					client->UnBan();
				}
				break;
			case MP_REMOVEUL:
				if (client!=NULL)
				{
					if (thePrefs.GetVerbose()) AddPhoenixLogLine(false, _T("S.N.A.F.U. %s kicked from uploadqueue"),client->GetUserName());
					if (theApp.uploadqueue->RemoveFromWaitingQueue(client,true))
						{
					if (!client->socket)
							client->Disconnected(_T("Remove client due to S.N.A.F.U"), false, CUpDownClient::USR_SNAFU); // [TPT] - Maella -Upload Stop Reason-
						client=NULL;
						}
			}
				break;
		    //>>> eWombat [SNAFU]
			case MPG_ALTENTER:
			case MP_DETAIL: {
				client->GetDetailDialogInterface()->OpenDetailDialog(this);	// [TPT] - SLUGFILLER: modelessDialogs
				break;
			}
			// [TPT] - itsonlyme:reqFiles START
			case MP_LIST_REQUESTED_FILES: { 
				client->GetDetailDialogInterface()->OpenDetailDialog(this, IDD_REQFILES);	// [TPT] - SLUGFILLER: modelessDialogs
				break;
			}
			// [TPT] - itsonlyme:reqFiles END
			case MP_BOOT:
				if (client->GetKadPort())
					Kademlia::CKademlia::bootstrap(ntohl(client->GetIP()), client->GetKadPort());
				break;
		}
	}
	return true;
} 

void CQueueListCtrl::OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult){

	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// Barry - Store sort order in preferences
	// Determine ascending based on whether already sorted on this column
	int sortItem = thePrefs.GetColumnSortItem(CPreferences::tableQueue);
	bool m_oldSortAscending = thePrefs.GetColumnSortAscending(CPreferences::tableQueue);
	bool sortAscending = (sortItem != pNMListView->iSubItem) ? true : !m_oldSortAscending;
	// Item is column clicked
	sortItem = pNMListView->iSubItem;
	// Save new preferences
	thePrefs.SetColumnSortItem(CPreferences::tableQueue, sortItem);
	thePrefs.SetColumnSortAscending(CPreferences::tableQueue, sortAscending);
	// Sort table
	SetSortArrow(sortItem, sortAscending);
	SortItems(SortProc, sortItem + (sortAscending ? 0:100));

	*pResult = 0;
}

int CQueueListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
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
			if(item2->GetUserName() && item1->GetUserName())
				return CompareLocaleStringNoCase(item2->GetUserName(), item1->GetUserName());
			else if(item2->GetUserName())
				return 1;
			else
				return -1;
		
		case 1: {
			CKnownFile* file1 = theApp.sharedfiles->GetFileByID(item1->GetUploadFileID());
			CKnownFile* file2 = theApp.sharedfiles->GetFileByID(item2->GetUploadFileID());
			if( (file1 != NULL) && (file2 != NULL))
				return CompareLocaleStringNoCase(file1->GetFileName(), file2->GetFileName());
			else if( file1 == NULL )
				return 1;
			else
				return -1;
		}
		case 101: {
			CKnownFile* file1 = theApp.sharedfiles->GetFileByID(item1->GetUploadFileID());
			CKnownFile* file2 = theApp.sharedfiles->GetFileByID(item2->GetUploadFileID());
			if( (file1 != NULL) && (file2 != NULL))
				return CompareLocaleStringNoCase(file2->GetFileName(), file1->GetFileName());
			else if( file1 == NULL )
				return 1;
			else
				return -1;
		}
		
		case 2: {
			// [TPT] - Powershare
			CKnownFile* file1 = theApp.sharedfiles->GetFileByID(item1->GetUploadFileID());
			CKnownFile* file2 = theApp.sharedfiles->GetFileByID(item2->GetUploadFileID());
			if (!file1 && !file2)
				return 0;
			else if (!file1)
				return -1;
			else if (!file2)
				return 1;
			if (file1->GetPowerShared() == false && file2->GetPowerShared() == true)
				return -1;			
			else if (file1->GetPowerShared() == true && file2->GetPowerShared() == false)
				return 1;
			else			
				if(file1->GetUpPriority() == PR_VERYLOW && file2->GetUpPriority() != PR_VERYLOW)
					return -1;
				else if (file1->GetUpPriority() != PR_VERYLOW && file2->GetUpPriority() == PR_VERYLOW)
					return 1;
				else
					return file1->GetUpPriority()-file2->GetUpPriority();
				}
		case 102: {
			CKnownFile* file1 = theApp.sharedfiles->GetFileByID(item1->GetUploadFileID());
			CKnownFile* file2 = theApp.sharedfiles->GetFileByID(item2->GetUploadFileID());
			if (!file1 && !file2)
				return 0;
			else if (!file1)
				return 1;
			else if (!file2)
				return -1;
			if (file2->GetPowerShared() == false && file1->GetPowerShared() == true)
				return -1;			
			else if (file2->GetPowerShared() == true && file1->GetPowerShared() == false)
				return 1;
			else		
				if(file2->GetUpPriority() == PR_VERYLOW && file1->GetUpPriority() != PR_VERYLOW )
					return -1;
				else if (file2->GetUpPriority() != PR_VERYLOW && file1->GetUpPriority() == PR_VERYLOW)
					return 1;
				else
					return file2->GetUpPriority()-file1->GetUpPriority();
				// [TPT] - Powershare end
		}
		case 3: 
			return CompareUnsigned(item1->GetScore(false,false,true), item2->GetScore(false,false,true));
		case 103: 
			return CompareUnsigned(item2->GetScore(false,false,true), item1->GetScore(false,false,true));

		// [TPT] - Powershare
		case 4: 
			//return CompareUnsigned(item1->GetScore(false), item2->GetScore(false));
		case 104: { 
			//return CompareUnsigned(item2->GetScore(false), item1->GetScore(false));
			int result = 0;

			CKnownFile* file1 = theApp.sharedfiles->GetFileByID(item1->GetUploadFileID());
			CKnownFile* file2 = theApp.sharedfiles->GetFileByID(item2->GetUploadFileID());
			if( (file1 != NULL) && (file2 != NULL))
				if(item1->GetPowerShared() == true && item2->GetPowerShared() == false)
					result = 1;
				else if(item1->GetPowerShared() == false && item2->GetPowerShared() == true)
					result = -1;
				//else if(item1->GetPowerShared() == true && item2->GetPowerShared() == true)
				//	result = ((file1->GetUpPriority()==PR_VERYLOW) ? -1 : file1->GetUpPriority()) - ((file2->GetUpPriority()==PR_VERYLOW) ? -1 : file2->GetUpPriority());
				else
					result = 0;
			else if( file1 == NULL )
				result = 1;
			else
				result = -1;

			if(result == 0)
				result = CompareUnsigned(item1->GetScore(false), item2->GetScore(false));

			if(lParamSort == 4)
				return result;
			else
				return -result;
		 }
		// [TPT] - Powershare
		case 5: 
			return item1->GetAskedCount() - item2->GetAskedCount();
		case 105: 
			return item2->GetAskedCount() - item1->GetAskedCount();
		
		case 6: 
			return item1->GetLastUpRequest() - item2->GetLastUpRequest();
		case 106: 
			return item2->GetLastUpRequest() - item1->GetLastUpRequest();
		
		case 7: 
			// [TPT] - SUQWT			
			{
				sint64 time1 = item1->GetWaitStartTime();
				sint64 time2 = item2->GetWaitStartTime();
				if ( time1 == time2 ) {
					return 0;
				} else if ( time1 > time2 ) {
					return 1;
				} else {
					return -1;
				}
			}			
		case 107: 			
			//return item2->GetWaitStartTime() - item1->GetWaitStartTime();
			{
				sint64 time1 = item1->GetWaitStartTime();
				sint64 time2 = item2->GetWaitStartTime();
				if ( time1 == time2 ) {
					return 0;
				} else if ( time1 < time2 ) {
					return 1;
				} else {
					return -1;
				}
			}
			// [TPT] - SUQWT
		case 8: 
			return item1->GetBanSort() - item2->GetBanSort();// [TPT] - eWombat SNAFU v2
		case 108: 
			return item2->GetBanSort() - item1->GetBanSort();// [TPT] - eWombat SNAFU v2
		
		case 9: 
			return item1->GetUpPartCount() - item2->GetUpPartCount();
		case 109: 
			return item2->GetUpPartCount() - item1->GetUpPartCount();
		// [TPT] - itsonlyme: clientSoft
		case 10: {
			if( item1->GetClientSoft() != item2->GetClientSoft() )
				return CompareUnsigned(item2->GetClientSoft(), item1->GetClientSoft());
			if (item1->GetVersion() != item2->GetVersion())
				return CompareUnsigned(item1->GetVersion(), item2->GetVersion());
			return item1->GetClientModVer().CompareNoCase(item2->GetClientModVer());
			}
		case 110: {
			if( item1->GetClientSoft() != item2->GetClientSoft() )
				return CompareUnsigned(item1->GetClientSoft(), item2->GetClientSoft());
			if (item1->GetVersion() != item2->GetVersion())
				return CompareUnsigned(item2->GetVersion(), item1->GetVersion());
			return item2->GetClientModVer().CompareNoCase(item1->GetClientModVer());
			}
		// [TPT] - itsonlyme: clientSoft END
		// [TPT] - Total UL/DL
		case 11:
			return item2->Credits()->GetUploadedTotal() - item1->Credits()->GetUploadedTotal();
		case 111: 
			return item2->Credits()->GetDownloadedTotal() - item1->Credits()->GetDownloadedTotal();
		// [TPT] - Total UL/DL
		// [TPT] - IP Country
		case 12:
			if(item1->GetCountryIP() && item2->GetCountryIP())
				return theApp.ip2country->Compare(item1->GetCountryIP(), item2->GetCountryIP());
			else if(item1->GetCountryIP())
				return 1;
			else
				return -1;
		case 112:
			if(item1->GetCountryIP() && item2->GetCountryIP())
				return theApp.ip2country->Compare(item2->GetCountryIP(), item1->GetCountryIP());
			else if(item2->GetCountryIP())
				return 1;
			else
				return -1;
		// [TPT] - IP Country
// [TPT] - WebCache ////////////////////////////////////////////////////////////////////////////////////
		//JP Webcache START 
		case 13:
			if (item1->SupportsWebCache() && item2->SupportsWebCache() )
				return CompareLocaleStringNoCase(item1->GetWebCacheName(),item2->GetWebCacheName());
			else
				return item1->SupportsWebCache() - item2->SupportsWebCache();
		case 113:
			if (item2->SupportsWebCache() && item1->SupportsWebCache() )
				return CompareLocaleStringNoCase(item2->GetWebCacheName(),item1->GetWebCacheName());
			else
				return item2->SupportsWebCache() - item1->SupportsWebCache();
		//JP Webcache END		
		default:
			return 0;
	}
}

// Barry - Refresh the queue every 10 secs
void CALLBACK CQueueListCtrl::QueueUpdateTimer(HWND hwnd, UINT uiMsg, UINT idEvent, DWORD dwTime)
{
	// NOTE: Always handle all type of MFC exceptions in TimerProcs - otherwise we'll get mem leaks
	try
	{
		if (   !theApp.emuledlg->IsRunning() // Don't do anything if the app is shutting down - can cause unhandled exceptions
			|| !thePrefs.GetUpdateQueueList()
			|| theApp.emuledlg->activewnd != theApp.emuledlg->transferwnd
			|| !theApp.emuledlg->transferwnd->queuelistctrl.IsWindowVisible() )
			return;

		const CUpDownClient* update = theApp.uploadqueue->GetNextClient(NULL);
		while( update )
		{
			theApp.emuledlg->transferwnd->queuelistctrl.RefreshClient(update);
			update = theApp.uploadqueue->GetNextClient(update);
		}
	}
	CATCH_DFLT_EXCEPTIONS(_T("CQueueListCtrl::QueueUpdateTimer"))
}

void CQueueListCtrl::ShowQueueClients()
{
	DeleteAllItems(); 
	CUpDownClient* update = theApp.uploadqueue->GetNextClient(NULL);
	while( update )
	{
		AddClient(update, false);
		update = theApp.uploadqueue->GetNextClient(update);
	}
}

void CQueueListCtrl::ShowSelectedUserDetails()
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

void CQueueListCtrl::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1) {
		CUpDownClient* client = (CUpDownClient*)GetItemData(iSel);
		if (client){
			client->GetDetailDialogInterface()->OpenDetailDialog(this);	// [TPT] - SLUGFILLER: modelessDialogs
		}
	}
	*pResult = 0;
}

// [TPT] - IP Country
void CQueueListCtrl::HideIPCountryColumn(bool hide)
{
	if (hide)
		HideColumn(12);
	else
		ShowColumn(12);
}
// [TPT] - IP Country

// [TPT] - MFCK [addon] - New Tooltips [Rayita]
/*void CQueueListCtrl::OnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
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
}*/
