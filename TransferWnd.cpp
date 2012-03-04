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
#include "SearchDlg.h"
#include "TransferWnd.h"
#include "OtherFunctions.h"
#include "ClientList.h"
#include "UploadQueue.h"
#include "DownloadQueue.h"
#include "emuledlg.h"
#include "MenuCmds.h"
#include "PartFile.h"
#include "CatDialog.h"
#include "InputBox.h"
#include "UserMsgs.h"
#include "math.h" // [TPT]

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif


// CTransferWnd dialog

IMPLEMENT_DYNAMIC(CTransferWnd, CDialog)
CTransferWnd::CTransferWnd(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CTransferWnd::IDD, pParent)
{
	icon_download = NULL;
	m_uWnd2 = DFLT_TRANSFER_WND2;
	m_pLastMousePoint.x = -1;
	m_pLastMousePoint.y = -1;
	m_nLastCatTT = -1;
	
	// [TPT] - MFCK [addon] - New Tooltips [Rayita]
	lists_list[UPLOAD_WND] = &uploadlistctrl;
	lists_list[QUEUE_WND] = &queuelistctrl;
	lists_list[KNOWN_WND] = &clientlistctrl;
	lists_list[TRANSF_WND] = &downloadclientsctrl;
	lists_list[UPDOWN_WND] = &downloadlistctrl;
	lists_list[DOWNLOAD_WND] = &downloadlistctrl;
	// [TPT] - MFCK [addon] - New Tooltips [Rayita]
}

CTransferWnd::~CTransferWnd()
{
	if (icon_download)
		VERIFY( DestroyIcon(icon_download) );
}

BEGIN_MESSAGE_MAP(CTransferWnd, CResizableDialog)
	ON_NOTIFY(LVN_HOTTRACK, IDC_UPLOADLIST, OnHoverUploadList)
	ON_NOTIFY(LVN_HOTTRACK, IDC_QUEUELIST, OnHoverUploadList)
	ON_NOTIFY(LVN_HOTTRACK, IDC_DOWNLOADLIST, OnHoverDownloadList)
	ON_NOTIFY(LVN_HOTTRACK, IDC_CLIENTLIST , OnHoverUploadList)
	ON_NOTIFY(TCN_SELCHANGE, IDC_DLTAB, OnTcnSelchangeDltab)
	ON_NOTIFY(NM_RCLICK, IDC_DLTAB, OnNMRclickDltab)
	ON_NOTIFY(LVN_BEGINDRAG, IDC_DOWNLOADLIST, OnLvnBegindrag)
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_NOTIFY(LVN_KEYDOWN, IDC_DOWNLOADLIST, OnLvnKeydownDownloadlist)
	ON_NOTIFY(UM_TABMOVED, IDC_DLTAB, OnTabMovement)
	ON_WM_SYSCOLORCHANGE()
	// [TPT] - MFCK [addon] - New Tooltips [Rayita]
	ON_NOTIFY_EX_RANGE(UDM_TOOLTIP_DISPLAY, 0, 0xFFFF, OnToolTipNotify) // added by rayita [Improved Tooltips]
	// [TPT] - MFCK [addon] - New Tooltips [Rayita]
END_MESSAGE_MAP()


BOOL CTransferWnd::OnInitDialog()
{
	CResizableDialog::OnInitDialog();
	InitWindowStyles(this);

	uploadlistctrl.Init();
	downloadlistctrl.Init();
	queuelistctrl.Init();
	clientlistctrl.Init();
	downloadclientsctrl.Init();// [TPT] - TBH Transfers Window Style	

	if (thePrefs.GetRestoreLastMainWndDlg())
		m_uWnd2 = thePrefs.GetTransferWnd2();
	bKl = !thePrefs.m_bDisableKnownClientList;
	bQl = !thePrefs.m_bDisableQueueList;
	ShowWnd2(m_uWnd2);

	SetAllIcons();

	//button tooltips
	m_btttp.Create(this);

    Localize(); // i_a 

	m_uplBtn.SetAlign(CButtonST::ST_ALIGN_HORIZ);
	m_uplBtn.SetFlat();
	m_uplBtn.DrawFlatFocus(TRUE);
	m_uplBtn.SetLeftAlign(true);
	
	// [TPT] - TBH Transfers Window Style
	m_btnChangeView.SetIcon(_T("ViewCycle"));	
	m_btnChangeView.SetAlign(CButtonST::ST_ALIGN_HORIZ);
	m_btnChangeView.SetLeftAlign(true); 
	
	m_btnDownUploads.SetIcon(_T("Up1Down1"));
	m_btnDownUploads.SetAlign(CButtonST::ST_ALIGN_HORIZ);
	m_btnDownUploads.SetLeftAlign(true);

	m_btnDownloads.SetIcon(_T("DirectDownload"));
	m_btnDownloads.SetAlign(CButtonST::ST_ALIGN_HORIZ);
	m_btnDownloads.SetLeftAlign(true); 

	m_btnUploads.SetIcon(_T("Upload"));
	m_btnUploads.SetAlign(CButtonST::ST_ALIGN_HORIZ);
	m_btnUploads.SetLeftAlign(true); 

	m_btnClient.SetIcon(_T("ViewKnownOnly"));
	m_btnClient.SetAlign(CButtonST::ST_ALIGN_HORIZ);
	m_btnClient.SetLeftAlign(true); 

	m_btnQueue.SetIcon(_T("ViewQueueOnly"));
	m_btnQueue.SetAlign(CButtonST::ST_ALIGN_HORIZ);
	m_btnQueue.SetLeftAlign(true); 

	m_btnTransfers.SetIcon(_T("TRANSFERENCIAS"));
	m_btnTransfers.SetAlign(CButtonST::ST_ALIGN_HORIZ);
	m_btnTransfers.SetLeftAlign(true); 

	//[TPT] - Switch List Icons
	m_btnULChangeView.SetIcon(_T("ViewCycle"));
	m_btnULChangeView.SetAlign(CButtonST::ST_ALIGN_HORIZ);
	m_btnULChangeView.SetLeftAlign(true);

	m_btnULClients.SetIcon(_T("ViewKnownOnly"));
	m_btnULClients.SetAlign(CButtonST::ST_ALIGN_HORIZ);
	m_btnULClients.SetLeftAlign(true);

	m_btnULQueue.SetIcon(_T("ViewQueueOnly"));
	m_btnULQueue.SetAlign(CButtonST::ST_ALIGN_HORIZ);
	m_btnULQueue.SetLeftAlign(true);

	m_btnULTransfers.SetIcon(_T("TRANSFERENCIAS"));
	m_btnULTransfers.SetAlign(CButtonST::ST_ALIGN_HORIZ);
	m_btnULTransfers.SetLeftAlign(true);

	m_btnULUploads.SetIcon(_T("Upload"));
	m_btnULUploads.SetAlign(CButtonST::ST_ALIGN_HORIZ);
	m_btnULUploads.SetLeftAlign(true);
	//[TPT] - Switch List Icons

	AddAnchor(IDC_DOWNLOADLIST,TOP_LEFT,CSize(100, thePrefs.GetSplitterbarPosition() ));
	AddAnchor(IDC_UPLOADLIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
	AddAnchor(IDC_QUEUELIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
	AddAnchor(IDC_CLIENTLIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
	AddAnchor(IDC_DOWNLOADCLIENTS,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);// [TPT] - TBH Transfers Window Style
	AddAnchor(IDC_UPLOAD_ICO,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
	AddAnchor(IDC_QUEUECOUNT,BOTTOM_LEFT);
	AddAnchor(IDC_TSTATIC1,BOTTOM_LEFT);
	AddAnchor(IDC_QUEUE_REFRESH_BUTTON, BOTTOM_RIGHT);
	AddAnchor(IDC_DLTAB,CSize(50,0) ,TOP_RIGHT);

	// splitting functionality
	CRect rc,rcSpl,rcDown;

	GetWindowRect(rc);
	ScreenToClient(rc);

	rcSpl=rc; rcSpl.top=rc.bottom-100 ; rcSpl.bottom=rcSpl.top+5;rcSpl.left=330; 	//[TPT] - Switch List Icons
	m_wndSplitter.Create(WS_CHILD | WS_VISIBLE, rcSpl, this, IDC_SPLITTER);
	//SetInitLayout();// [TPT] - TBH Transfers Window Style	
	m_uWnd2 = 1;

	//[TPT] - Switch List Icons
	m_btnULChangeView.ShowWindow(SW_SHOW);
	m_btnULClients.ShowWindow(SW_SHOW);
	m_btnULQueue.ShowWindow(SW_SHOW);
	m_btnULTransfers.ShowWindow(SW_SHOW);
	m_btnULUploads.ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC7)->ShowWindow(SW_SHOW);
	//[TPT] - Switch List Icons

	OnBnClickedDownUploads();
	// [TPT] - TBH Transfers Window Style
	
	//cats
	rightclickindex=-1;

	downloadlistactive=true;
	m_bIsDragging=false;

	// show & cat-tabs
	// [TPT] - MFCK [addon] - New Tooltips [Rayita]
	// [TPT] - khaos::categorymod+
  	for (int ix=0; ix < thePrefs.GetCatCount(); ix++)
	{
		Category_Struct* curCat = thePrefs.GetCategory(ix);
		// MORPH - TO WATCH FOR CATEGORY STATUS
		/*if (ix == 0 && curCat->viewfilters.nFromCats == 2)
			curCat->viewfilters.nFromCats = 0;
		else*/ if (curCat->viewfilters.nFromCats != 2 && ix != 0 && theApp.downloadqueue->GetCategoryFileCount(ix) != 0)
			curCat->viewfilters.nFromCats = 2;

		m_dlTab.InsertItem(ix,thePrefs.GetCategory(ix)->title );
	}
	UpdateCatTabTitles();
	// [TPT] - khaos::categorymod-

	// download tabs tooltips
	m_tabtip.Create(this, TTS_NOPREFIX);
	m_tabtip.SetDirection(CPPToolTip::PPTOOLTIP_RIGHT_BOTTOM);

	m_tabtip.AddTool(&m_dlTab, _T(""));

	// listctrls tooltips
	m_ttip.Create(this);


	m_ttip.AddTool(&downloadlistctrl, _T(""));
	m_ttip.AddTool(&uploadlistctrl, _T(""));
	m_ttip.AddTool(&queuelistctrl, _T(""));
	m_ttip.AddTool(&clientlistctrl, _T(""));
	m_ttip.AddTool(&downloadclientsctrl, _T(""));


	for(int i = 0; i < ARRSIZE(m_iOldToolTipItem); i++)
		m_iOldToolTipItem[i] = -1;

	m_othertips.Create(this);

	SetTTDelay();
	// [TPT] - MFCK [addon] - New Tooltips [Rayita]

	UpdateListCount(m_uWnd2);
	VerifyCatTabSize();

	return true;
}

void CTransferWnd::ShowQueueCount(uint32 number){
	TCHAR buffer[100];
	// [TPT] - eWombat SNAFU v2
	int banned=theApp.clientlist->GetSnafuCount();	
	
	if(thePrefs.IsInfiniteQueueEnabled())
		_stprintf(buffer,_T("%u / %s (%u ") + GetResString(IDS_BANNED).MakeLower() + _T(")") + _T("  %s"), 
			number-banned, GetResString(IDS_INFINITEQUEUE), banned, thePrefs.GetEnableMultiQueue() == true ? GetResString(IDS_MULTIQUEUE_ENABLED) : GetResString(IDS_MULTIQUEUE_DISABLED) );
	else
		_stprintf(buffer,_T("%u / %u (%u ") + GetResString(IDS_BANNED).MakeLower() + _T(")") + _T("  %s"), 
			number-banned, (thePrefs.GetQueueSize() + max(thePrefs.GetQueueSize()/4, 200)),banned, thePrefs.GetEnableMultiQueue() == true ? GetResString(IDS_MULTIQUEUE_ENABLED) : GetResString(IDS_MULTIQUEUE_DISABLED) );
	// [TPT] - eWombat SNAFU v2
	GetDlgItem(IDC_QUEUECOUNT)->SetWindowText(buffer);
}

void CTransferWnd::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_UPLOADLIST, uploadlistctrl);
	DDX_Control(pDX, IDC_DOWNLOADLIST, downloadlistctrl);
	DDX_Control(pDX, IDC_DOWNLOADCLIENTS, downloadclientsctrl);// [TPT] - TBH Transfers Window Style
	DDX_Control(pDX, IDC_QUEUELIST, queuelistctrl);
	DDX_Control(pDX, IDC_CLIENTLIST, clientlistctrl);
	DDX_Control(pDX, IDC_UPLOAD_ICO, m_uplBtn);
	DDX_Control(pDX, IDC_DLTAB, m_dlTab);
	// [TPT] - TBH Transfers Window Style
	DDX_Control(pDX, IDC_DL_CHANGEVIEW, m_btnChangeView);
	DDX_Control(pDX, IDC_DL_DOWN_UPLOADS, m_btnDownUploads);
	DDX_Control(pDX, IDC_DL_DOWNLOADS, m_btnDownloads);
	DDX_Control(pDX, IDC_DL_UPLOADS, m_btnUploads);
	DDX_Control(pDX, IDC_DL_QUEUE, m_btnQueue);
	DDX_Control(pDX, IDC_DL_TRANSFERS, m_btnTransfers);
	DDX_Control(pDX, IDC_DL_CLIENT, m_btnClient);
	// [TPT] - TBH Transfers Window Style

	//[TPT] - Switch List Icons
	DDX_Control(pDX, IDC_UL_CHANGEVIEW, m_btnULChangeView);
	DDX_Control(pDX, IDC_UL_UPLOADS, m_btnULUploads);
	DDX_Control(pDX, IDC_UL_QUEUE, m_btnULQueue);
	DDX_Control(pDX, IDC_UL_CLIENTS, m_btnULClients);
	DDX_Control(pDX, IDC_UL_TRANSFERS, m_btnULTransfers);
	//[TPT] - Switch List Icons
}
// Deleted due to [TPT] - TBH Transfers Window Style
/*
void CTransferWnd::SetInitLayout() {
	CRect rcDown,rcSpl,rcW;

	GetWindowRect(rcW);
	ScreenToClient(rcW);

	LONG splitpos=(thePrefs.GetSplitterbarPosition()*rcW.Height())/100;

	GetDlgItem(IDC_DOWNLOADLIST)->GetWindowRect(rcDown);
	ScreenToClient(rcDown);
	rcDown.right=rcW.right-7;
	rcDown.bottom=splitpos-5;
	downloadlistctrl.MoveWindow(rcDown);
	
	GetDlgItem(IDC_UPLOADLIST)->GetWindowRect(rcDown);
	ScreenToClient(rcDown);
	rcDown.right=rcW.right-7;
	rcDown.bottom=rcW.bottom-20;
	rcDown.top=splitpos+20;
	uploadlistctrl.MoveWindow(rcDown);

	GetDlgItem(IDC_QUEUELIST)->GetWindowRect(rcDown);
	ScreenToClient(rcDown);
	rcDown.right=rcW.right-7;
	rcDown.bottom=rcW.bottom-20;
	rcDown.top=splitpos+20;
	queuelistctrl.MoveWindow(rcDown);

	GetDlgItem(IDC_CLIENTLIST)->GetWindowRect(rcDown);
	ScreenToClient(rcDown);
	rcDown.right=rcW.right-7;
	rcDown.bottom=rcW.bottom-20;
	rcDown.top=splitpos+20;
	clientlistctrl.MoveWindow(rcDown);

	rcSpl=rcDown;
	rcSpl.top=rcDown.bottom+4;rcSpl.bottom=rcSpl.top+7;rcSpl.left=(rcDown.right/2)-50;rcSpl.right=rcSpl.left+100;
	m_wndSplitter.MoveWindow(rcSpl,true);

	DoResize(0);
}
*/
void CTransferWnd::DoResize(int delta)
{
	CSplitterControl::ChangeHeight(&downloadlistctrl, delta);
	CSplitterControl::ChangeHeight(&uploadlistctrl, -delta, CW_BOTTOMALIGN);
	CSplitterControl::ChangeHeight(&queuelistctrl, -delta, CW_BOTTOMALIGN);
	CSplitterControl::ChangeHeight(&clientlistctrl, -delta, CW_BOTTOMALIGN);
	CSplitterControl::ChangeHeight(&downloadclientsctrl, -delta, CW_BOTTOMALIGN);// [TPT] - TBH Transfers Window Style

	UpdateSplitterRange();

	Invalidate();
	UpdateWindow();
}

// setting splitter range limits
void CTransferWnd::UpdateSplitterRange()
{
	CRect rcDown,rcUp,rcW,rcSpl;

	GetWindowRect(rcW);
	ScreenToClient(rcW);

	GetDlgItem(IDC_DOWNLOADLIST)->GetWindowRect(rcDown);
	ScreenToClient(rcDown);

	GetDlgItem(IDC_UPLOADLIST)->GetWindowRect(rcUp);
	ScreenToClient(rcUp);

	GetDlgItem(IDC_QUEUELIST)->GetWindowRect(rcUp);
	ScreenToClient(rcUp);

	GetDlgItem(IDC_CLIENTLIST)->GetWindowRect(rcUp);
	ScreenToClient(rcUp);

	// [TPT] - TBH Transfers Window Style
	GetDlgItem(IDC_DOWNLOADCLIENTS)->GetWindowRect(rcUp);
	ScreenToClient(rcUp);

	thePrefs.SetSplitterbarPosition((int)floor((float)(rcDown.bottom*100.0f)/(float)rcW.Height()));

	RemoveAnchor(IDC_DOWNLOADLIST);
	RemoveAnchor(IDC_UPLOADLIST);
	RemoveAnchor(IDC_QUEUELIST);
	RemoveAnchor(IDC_CLIENTLIST);
	RemoveAnchor(IDC_DOWNLOADCLIENTS);// [TPT] - TBH Transfers Window Style
	AddAnchor(IDC_DOWNLOADLIST,TOP_LEFT,CSize(100,thePrefs.GetSplitterbarPosition() ));
	AddAnchor(IDC_UPLOADLIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
	AddAnchor(IDC_QUEUELIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
	AddAnchor(IDC_CLIENTLIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
	AddAnchor(IDC_DOWNLOADCLIENTS,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);// [TPT] - TBH Transfers Window Style

	m_wndSplitter.SetRange(rcDown.top+50 , rcUp.bottom-40);
}

LRESULT CTransferWnd::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message) {
		// arrange transferwindow layout
		case WM_PAINT:
			if (m_wndSplitter) {
				CRect rcDown,rcSpl,rcW;

				GetWindowRect(rcW);
				ScreenToClient(rcW);

				GetDlgItem(IDC_DOWNLOADLIST)->GetWindowRect(rcDown);
				ScreenToClient(rcDown);

				if (rcW.Height()>0) {
					// splitter paint update
					rcSpl=rcDown;
					rcSpl.top=rcDown.bottom+8;rcSpl.bottom=rcSpl.top+5;rcSpl.left=330; //[TPT] - Switch List Icons
					GetDlgItem(IDC_UPLOAD_ICO)->MoveWindow(10,rcSpl.top-4,170,18);
					//[TPT] - Switch List Icons
					m_btnULChangeView.MoveWindow(211, rcSpl.top-4, 18, 18);
					GetDlgItem(IDC_STATIC7)->MoveWindow(233, rcSpl.top-2, 2, 15);
					m_btnULUploads.MoveWindow(238, rcSpl.top-4, 18, 18);
					m_btnULQueue.MoveWindow(259, rcSpl.top-4, 18, 18);
					m_btnULClients.MoveWindow(280, rcSpl.top-4, 18, 18);
					m_btnULTransfers.MoveWindow(301, rcSpl.top-4, 18, 18);
					//[TPT] - Switch List Icons
					m_wndSplitter.MoveWindow(rcSpl,true);
					UpdateSplitterRange();
				}
			}
			break;
		case WM_NOTIFY:
			if (wParam == IDC_SPLITTER)
			{	
				SPC_NMHDR* pHdr = (SPC_NMHDR*) lParam;
				DoResize(pHdr->delta);
			}
			break;
		case WM_WINDOWPOSCHANGED : 
			{
				CRect rcW;
				GetWindowRect(rcW);
				ScreenToClient(rcW);

				if (m_wndSplitter && rcW.Height()>0) Invalidate();
				break;
			}
		case WM_SIZE:
			if (m_wndSplitter) {
				CRect rcDown,rcSpl,rcW;
				GetWindowRect(rcW);
				ScreenToClient(rcW);
				if (rcW.Height()>0){
					GetDlgItem(IDC_DOWNLOADLIST)->GetWindowRect(rcDown);
					ScreenToClient(rcDown);

					long splitpos=(thePrefs.GetSplitterbarPosition()*rcW.Height())/100;

					rcSpl.right=rcDown.right;rcSpl.top=splitpos+10;rcSpl.bottom=rcSpl.top+7;rcSpl.left=(rcDown.right/2)-50;rcSpl.right=rcSpl.left+100;
					m_wndSplitter.MoveWindow(rcSpl,true);
				}
			}
			break;
	}

	return CResizableDialog::DefWindowProc(message, wParam, lParam);
}

// CTransferWnd message handlers
BOOL CTransferWnd::PreTranslateMessage(MSG* pMsg)
{
	// [TPT] - MFCK [addon] - New Tooltips [Rayita]
	switch(pMsg->message)
	{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MOUSEMOVE:
		case WM_MOUSEWHEEL:
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			m_ttip.RelayEvent(pMsg);
			m_tabtip.RelayEvent(pMsg);
			m_othertips.RelayEvent(pMsg);
			m_btttp.RelayEvent(pMsg);
	}
	// [TPT] - MFCK [addon] - New Tooltips [Rayita]

	if (pMsg->message== WM_LBUTTONDBLCLK && pMsg->hwnd== GetDlgItem(IDC_DLTAB)->m_hWnd) {
		OnDblclickDltab();
		return TRUE;
	}

	if (pMsg->message==WM_MOUSEMOVE) {
		POINT point;
		::GetCursorPos(&point);
		if (point.x!=m_pLastMousePoint.x || point.y!=m_pLastMousePoint.y) {
			m_pLastMousePoint=point;

				UpdateToolTips(); // [TPT] - MFCK [addon] - New Tooltips [Rayita]
				UpdateTabToolTips(); // [TPT] - MFCK [addon] - New Tooltips [Rayita]
		}
	}

	if (pMsg->message == WM_MBUTTONUP){
		if (downloadlistactive)
			downloadlistctrl.ShowSelectedFileDetails();
		else{
			switch (m_uWnd2){
				// [TPT] - TransferWindow Fix
				// [TPT] - UserDetails
				case 0:
					downloadclientsctrl.ShowSelectedUserDetails();
					break;
				// [TPT] - UserDetails				
				case 1:
					uploadlistctrl.ShowSelectedUserDetails();
					break;
				case 2:
					queuelistctrl.ShowSelectedUserDetails();
					break;
				case 3:
					clientlistctrl.ShowSelectedUserDetails();
					break;
				// [TPT] - TransferWindow Fix
			}
		}
		return TRUE;
	}

	return CResizableDialog::PreTranslateMessage(pMsg);
}

// [TPT] - MFCK [addon] - New Tooltips [Rayita]
/*int CTransferWnd::GetItemUnderMouse(CListCtrl* ctrl)
{
	CPoint pt;
	::GetCursorPos(&pt);
	ctrl->ScreenToClient(&pt);
	LVHITTESTINFO hit, subhit;
	hit.pt = pt;
	subhit.pt = pt;
	ctrl->SubItemHitTest(&subhit);
	int sel = ctrl->HitTest(&hit);
	if (sel != LB_ERR && (hit.flags & LVHT_ONITEM))
	{
		if (subhit.iSubItem == 0)
			return sel;
	}
	return LB_ERR;
}*/
// [TPT] - MFCK [addon] - New Tooltips [Rayita]
void CTransferWnd::UpdateDownloadClientsCount(int count)
{
	if (showlist == IDC_DOWNLOADCLIENTS)
	{
		CString buffer;
		buffer.Format(_T(" (%i)"), count);
		GetDlgItem(IDC_DOWNLOAD_TEXT)->SetWindowText(GetResString(IDS_TW_TRANSFERS)+buffer);
	}
}

void CTransferWnd::UpdateListCount(uint8 listindex, int iCount /*=-1*/)
{
	if (m_uWnd2 != listindex)
		return;

	CString buffer;
	switch (m_uWnd2){
        case 1: {
            uint32 itemCount = iCount == -1 ? uploadlistctrl.GetItemCount() : iCount;
			// [TPT] - Remove trickling info
            //uint32 activeCount = theApp.uploadqueue->GetActiveUploadsCount();
            //if(activeCount >= itemCount) {
                buffer.Format(_T(" (%i)"), itemCount);
            //} else {
            //   buffer.Format(_T(" (%i/%i)"), activeCount, itemCount);
            //}
			// [TPT] - Remove trickling info
			GetDlgItem(IDC_UPLOAD_ICO)->SetWindowText(GetResString(IDS_TW_UPLOADS)+buffer);
			break;
        }
		case 2:
			buffer.Format(_T(" (%i)"), iCount == -1 ? queuelistctrl.GetItemCount() : iCount);
			GetDlgItem(IDC_UPLOAD_ICO)->SetWindowText(GetResString(IDS_ONQUEUE)+buffer);
			break;
		case 3: // [TPT] - TBH Transfers Window Style
			buffer.Format(_T(" (%i)"), iCount == -1 ? clientlistctrl.GetItemCount() : iCount);
			GetDlgItem(IDC_UPLOAD_ICO)->SetWindowText(GetResString(IDS_CLIENTLIST)+buffer);
			break;
		default:{
					buffer.Format(_T(" (%i)"), iCount == -1 ? downloadclientsctrl.GetItemCount() : iCount);					
					GetDlgItem(IDC_UPLOAD_ICO)->SetWindowText(GetResString(IDS_TW_TRANSFERS)+buffer);
			   }

	}
}

void CTransferWnd::SwitchUploadList()
{
	//[TPT] - TBH Transfer Window
	bKl = !thePrefs.m_bDisableKnownClientList;
	bQl = !thePrefs.m_bDisableQueueList;
	// [TPT] - TBH Transfer Window
	if( m_uWnd2 == 1){
		SetWnd2(2);
		if( thePrefs.IsQueueListDisabled()){
			SwitchUploadList();
			return;
		}
		uploadlistctrl.Hide();
		clientlistctrl.Hide();
		downloadclientsctrl.Hide();// [TPT] - TBH Transfers Window Style
		GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_SHOW);
		queuelistctrl.Visable();
		GetDlgItem(IDC_UPLOAD_ICO)->SetWindowText(GetResString(IDS_ONQUEUE));
		//[TPT] - Switch List Icons
		GetDlgItem(IDC_UL_CHANGEVIEW)->EnableWindow(true);
		GetDlgItem(IDC_UL_CLIENTS)->EnableWindow(bKl);
		GetDlgItem(IDC_UL_QUEUE)->EnableWindow(false);
		GetDlgItem(IDC_UL_TRANSFERS)->EnableWindow(true);
		GetDlgItem(IDC_UL_UPLOADS)->EnableWindow(true);
		//[TPT] - Switch List Icons
	}
	else if( m_uWnd2 == 2){
		SetWnd2(3); // [TPT] - TBH Transfers Window Style
		if( thePrefs.IsKnownClientListDisabled()){
			SwitchUploadList();
			return;
		}
		uploadlistctrl.Hide();
		queuelistctrl.Hide();
		downloadclientsctrl.Hide();// [TPT] - TBH Transfers Window Style
		clientlistctrl.Visable();
		GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_UPLOAD_ICO)->SetWindowText(GetResString(IDS_CLIENTLIST));
		//[TPT] - Switch List Icons
		GetDlgItem(IDC_UL_CHANGEVIEW)->EnableWindow(true);
		GetDlgItem(IDC_UL_CLIENTS)->EnableWindow(false);
		GetDlgItem(IDC_UL_QUEUE)->EnableWindow(bQl);
		GetDlgItem(IDC_UL_TRANSFERS)->EnableWindow(true);
		GetDlgItem(IDC_UL_UPLOADS)->EnableWindow(true);
		//[TPT] - Switch List Icons
	}
	else if( m_uWnd2 == 3){// [TPT] - TBH Transfers Window Style
		SetWnd2(0);
		queuelistctrl.Hide();
		clientlistctrl.Hide();
		uploadlistctrl.Hide();
		downloadclientsctrl.Show();
		GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_UPLOAD_ICO)->SetWindowText(GetResString(IDS_TW_TRANSFERS));
		//[TPT] - Switch List Icons
		GetDlgItem(IDC_UL_CHANGEVIEW)->EnableWindow(true);
		GetDlgItem(IDC_UL_CLIENTS)->EnableWindow(bKl);
		GetDlgItem(IDC_UL_QUEUE)->EnableWindow(bQl);
		GetDlgItem(IDC_UL_TRANSFERS)->EnableWindow(false);
		GetDlgItem(IDC_UL_UPLOADS)->EnableWindow(true);
		//[TPT] - Switch List Icons
	}
	else{
		queuelistctrl.Hide();
		clientlistctrl.Hide();
		downloadclientsctrl.Hide();// [TPT] - TBH Transfers Window Style
		uploadlistctrl.Visable();
		GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_HIDE);
		SetWnd2(1);
		GetDlgItem(IDC_UPLOAD_ICO)->SetWindowText(GetResString(IDS_TW_UPLOADS));
		//[TPT] - Switch List Icons
		GetDlgItem(IDC_UL_CHANGEVIEW)->EnableWindow(true);
		GetDlgItem(IDC_UL_CLIENTS)->EnableWindow(bKl);
		GetDlgItem(IDC_UL_QUEUE)->EnableWindow(bQl);
		GetDlgItem(IDC_UL_TRANSFERS)->EnableWindow(true);
		GetDlgItem(IDC_UL_UPLOADS)->EnableWindow(false);
		//[TPT] - Switch List Icons
	}
	UpdateListCount(m_uWnd2);
	SetWnd2Icon();
}

void CTransferWnd::ShowWnd2(uint8 uWnd2)
{
	//TPT] - TBH Transfer Window
	bKl = !thePrefs.m_bDisableKnownClientList;
	bQl = !thePrefs.m_bDisableQueueList;
	//[TPT] - TBH Transfer Window

	if (uWnd2 == 2 && !thePrefs.IsQueueListDisabled())
	{
		uploadlistctrl.Hide();
		clientlistctrl.Hide();
		downloadclientsctrl.Hide(); // [TPT] - TBH Transfers Window Style
		queuelistctrl.Visable();
		GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_UPLOAD_ICO)->SetWindowText(GetResString(IDS_ONQUEUE));
		//[TPT] - Switch List Icons
		GetDlgItem(IDC_UL_CHANGEVIEW)->EnableWindow(true);
		GetDlgItem(IDC_UL_CLIENTS)->EnableWindow(bKl);
		GetDlgItem(IDC_UL_QUEUE)->EnableWindow(false);
		GetDlgItem(IDC_UL_TRANSFERS)->EnableWindow(true);
		GetDlgItem(IDC_UL_UPLOADS)->EnableWindow(true);
		//[TPT] - Switch List Icons
		SetWnd2(uWnd2);
	}
	else if (uWnd2 == 3 && !thePrefs.IsKnownClientListDisabled()) // [TPT] - TBH Transfers Window Style
	{
		uploadlistctrl.Hide();
		queuelistctrl.Hide();
		downloadclientsctrl.Hide(); // [TPT] - TBH Transfers Window Style
		clientlistctrl.Visable();
		GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_UPLOAD_ICO)->SetWindowText(GetResString(IDS_CLIENTLIST));
		//[TPT] - Switch List Icons
		GetDlgItem(IDC_UL_CHANGEVIEW)->EnableWindow(true);
		GetDlgItem(IDC_UL_CLIENTS)->EnableWindow(false);
		GetDlgItem(IDC_UL_QUEUE)->EnableWindow(bQl);
		GetDlgItem(IDC_UL_TRANSFERS)->EnableWindow(true);
		GetDlgItem(IDC_UL_UPLOADS)->EnableWindow(true);
		//[TPT] - Switch List Icons
		SetWnd2(uWnd2);
	}
	// [TPT] - TBH Transfers Window Style
	else if (uWnd2 == 0) 
	{
		uploadlistctrl.Hide();
		queuelistctrl.Hide();		
		clientlistctrl.Hide();
		downloadclientsctrl.Show();
		GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_UPLOAD_ICO)->SetWindowText(GetResString(IDS_TW_TRANSFERS));
		//[TPT] - Switch List Icons
		GetDlgItem(IDC_UL_CHANGEVIEW)->EnableWindow(true);
		GetDlgItem(IDC_UL_CLIENTS)->EnableWindow(bKl);
		GetDlgItem(IDC_UL_QUEUE)->EnableWindow(bQl);
		GetDlgItem(IDC_UL_TRANSFERS)->EnableWindow(false);
		GetDlgItem(IDC_UL_UPLOADS)->EnableWindow(true);
		//[TPT] - Switch List Icons
		SetWnd2(uWnd2);
	}
	// [TPT] - TBH Transfers Window Style
	else
	{
		queuelistctrl.Hide();
		clientlistctrl.Hide();
		downloadclientsctrl.Hide(); // [TPT] - TBH Transfers Window Style
		uploadlistctrl.Visable();
		GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_UPLOAD_ICO)->SetWindowText(GetResString(IDS_TW_UPLOADS));
		//[TPT] - Switch List Icons
		GetDlgItem(IDC_UL_CHANGEVIEW)->EnableWindow(true);
		GetDlgItem(IDC_UL_CLIENTS)->EnableWindow(bKl);
		GetDlgItem(IDC_UL_QUEUE)->EnableWindow(bQl);
		GetDlgItem(IDC_UL_TRANSFERS)->EnableWindow(true);
		GetDlgItem(IDC_UL_UPLOADS)->EnableWindow(false);
		//[TPT] - Switch List Icons
		SetWnd2(1);
	}

	UpdateListCount(m_uWnd2); //[TPT] - Switch List Icons
	SetWnd2Icon();
}

void CTransferWnd::SetWnd2(uint8 uWnd2)
{
	m_uWnd2 = uWnd2;
	thePrefs.SetTransferWnd2(m_uWnd2);
}

//START [TPT] - enkeyDEV(th1) -notifier-
void CTransferWnd::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CResizableDialog::OnShowWindow(bShow, nStatus);
	theApp.emuledlg->m_wndTaskbarNotifier->SetAutoClose(true);
}
//END - [TPT] - enkeyDEV(th1) -notifier-

void CTransferWnd::OnSysColorChange()
{
	CResizableDialog::OnSysColorChange();
	SetAllIcons();
}

void CTransferWnd::SetAllIcons()
{
	// [TPT] - MFCK [addon] - New Tooltips [Rayita]
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
	m_ImageList.Add(CTempIconLoader(_T("RATINGRECEIVED")));//9
	m_ImageList.Add(CTempIconLoader(_T("BADRATINGRECEIVED")));//10
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkeyHybrid")));//11
	m_ImageList.Add(CTempIconLoader(_T("ClientShareaza")));//12
	m_ImageList.Add(CTempIconLoader(_T("ClientRightEdonkey"))); //13	
	// [TPT] - MFCK [addon] - New Tooltips [Rayita]

	if (icon_download)
		VERIFY( DestroyIcon(icon_download) );
	icon_download = theApp.LoadIcon(_T("Download"), 16, 16);
	((CStatic*)GetDlgItem(IDC_DOWNLOAD_ICO))->SetIcon(icon_download);
	SetWnd2Icon();
}

void CTransferWnd::SetWnd2Icon()
{
	if (m_uWnd2 == 2)
		m_uplBtn.SetIcon(_T("ClientsOnQueue"));
	else if (m_uWnd2 == 1)
		m_uplBtn.SetIcon(_T("Upload"));
	else if (m_uWnd2 == 3)
		m_uplBtn.SetIcon(_T("ViewKnownOnly"));
	else
		m_uplBtn.SetIcon(_T("TRANSFERENCIAS"));
}

void CTransferWnd::Localize()
{
	GetDlgItem(IDC_DOWNLOAD_TEXT)->SetWindowText(GetResString(IDS_TW_DOWNLOADS));
	GetDlgItem(IDC_UPLOAD_ICO)->SetWindowText(GetResString(IDS_TW_UPLOADS));
	GetDlgItem(IDC_TSTATIC1)->SetWindowText(GetResString(IDS_TW_QUEUE));
	GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->SetWindowText(GetResString(IDS_SV_UPDATE));

	uploadlistctrl.Localize();
	queuelistctrl.Localize();
	downloadlistctrl.Localize();
	clientlistctrl.Localize();
	downloadclientsctrl.Localize();// [TPT] - TBH Transfers Window Style

	//Adding tooltips to switch buttons
	m_btttp.AddTool(GetDlgItem(IDC_DL_CHANGEVIEW), GetResString(IDS_TTBT_VIEWS));
	m_btttp.AddTool(GetDlgItem(IDC_DL_DOWN_UPLOADS), GetResString(IDS_TTBT_DWUP));
	m_btttp.AddTool(GetDlgItem(IDC_DL_DOWNLOADS), GetResString(IDS_TTBT_DW));
	m_btttp.AddTool(GetDlgItem(IDC_DL_UPLOADS), GetResString(IDS_TTBT_UP));
	m_btttp.AddTool(GetDlgItem(IDC_DL_QUEUE), GetResString(IDS_TTBT_QUEUE));
	m_btttp.AddTool(GetDlgItem(IDC_DL_TRANSFERS), GetResString(IDS_TTBT_TRANSFER));
	m_btttp.AddTool(GetDlgItem(IDC_DL_CLIENT), GetResString(IDS_TTBT_CLIENTS));


	UpdateListCount(m_uWnd2);
}

void CTransferWnd::OnBnClickedQueueRefreshButton()
{
	CUpDownClient* update = theApp.uploadqueue->GetNextClient(NULL);

	while( update ){
		theApp.emuledlg->transferwnd->queuelistctrl.RefreshClient( update);
		update = theApp.uploadqueue->GetNextClient(update);
	}
}
// [TPT] - TBH Transfers Window Style
void CTransferWnd::OnBnClickedChangeView() {

	int wndToChange = 0;
	bKl = thePrefs.m_bDisableKnownClientList;
	bQl = thePrefs.m_bDisableQueueList;
	
	if(showlist == IDC_DOWNLOADLIST){
		ShowList(IDC_UPLOADLIST);
		wndToChange = 1;}
	else if(showlist == IDC_UPLOADLIST){
		if(bQl && bKl){
			  ShowList(IDC_DOWNLOADCLIENTS);
			  wndToChange = 4;}
		else if(bQl && !bKl){
			ShowList(IDC_CLIENTLIST);
			wndToChange = 3;}
		else{
			ShowList(IDC_QUEUELIST);
			wndToChange = 2;}
	}
	else if(showlist == IDC_QUEUELIST){
		if(!bKl){
		ShowList(IDC_CLIENTLIST);
		wndToChange = 3;}
		else{
			ShowList(IDC_DOWNLOADCLIENTS);
			wndToChange = 4;}
	}
	else if(showlist == IDC_CLIENTLIST){
		ShowList(IDC_DOWNLOADCLIENTS);
		wndToChange = 4;}
	else if(showlist == IDC_DOWNLOADCLIENTS){
		OnBnClickedDownUploads();
		wndToChange = 0;}
	else if(showlist == IDC_UPLOADLIST+IDC_DOWNLOADLIST){
		ShowList(IDC_DOWNLOADLIST);
		wndToChange = 0;}

	ChangeDlIcon(wndToChange);

}

//[TPT] - Fixing the NO change in the icon of up list :S
//One parameter, the wnd we are going to shiwcht to
//0: download
//1: upload
//2: queue
//3: known
//4: transfers
void CTransferWnd::ChangeDlIcon(int wndToChange)
{
	switch(wndToChange){
		case 0:
			((CStatic*)GetDlgItem(IDC_DOWNLOAD_ICO))->SetIcon(theApp.LoadIcon(_T("DirectDownload"), 16, 16));
			break;
		case 1:
			((CStatic*)GetDlgItem(IDC_DOWNLOAD_ICO))->SetIcon(theApp.LoadIcon(_T("Upload"), 16, 16));
			break;
		case 2:
			((CStatic*)GetDlgItem(IDC_DOWNLOAD_ICO))->SetIcon(theApp.LoadIcon(_T("ViewQueueOnly"), 16, 16));
			break;
		case 3:
			((CStatic*)GetDlgItem(IDC_DOWNLOAD_ICO))->SetIcon(theApp.LoadIcon(_T("ViewKnownOnly"), 16, 16));
			break;
		case 4:
			((CStatic*)GetDlgItem(IDC_DOWNLOAD_ICO))->SetIcon(theApp.LoadIcon(_T("TRANSFERENCIAS"), 16, 16));
			break;
		default:
			((CStatic*)GetDlgItem(IDC_DOWNLOAD_ICO))->SetIcon(theApp.LoadIcon(_T("DirectDownload"), 16, 16));
			break;

	}
}

// [TPT] - TBH Transfers Window Style
void CTransferWnd::ShowList(uint16 list) {
	CRect rcDown,rcW;
	CWnd* pWnd;
	CString buffer; // [TPT] - Show count number

	GetWindowRect(rcW);
	ScreenToClient(rcW);
	pWnd = GetDlgItem(list);
	pWnd->GetWindowRect(rcDown);
	ScreenToClient(rcDown);
	rcDown.bottom=rcW.bottom-20;
	rcDown.top=28;
	m_wndSplitter.DestroyWindow();
	//[TPT] - Switch List Icons
	m_btnULChangeView.ShowWindow(SW_HIDE);
	m_btnULClients.ShowWindow(SW_HIDE);
	m_btnULQueue.ShowWindow(SW_HIDE);
	m_btnULTransfers.ShowWindow(SW_HIDE);
	m_btnULUploads.ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC7)->ShowWindow(SW_HIDE);
	//[TPT] - Switch List Icons
	RemoveAnchor(list);
	GetDlgItem(IDC_UPLOAD_ICO)->ShowWindow(SW_HIDE); // [TPT] - TransferWindow Fix

	showlist = list; // [TPT] - TransferWindow Fix
	bKl = !thePrefs.m_bDisableKnownClientList;
	bQl = !thePrefs.m_bDisableQueueList;

	switch(list){
		case IDC_DOWNLOADLIST:
			m_dlTab.ShowWindow(SW_SHOW); //MORPH - Added by SiRoB, Show/Hide dlTab
			downloadlistctrl.MoveWindow(rcDown);
			// [TPT] - TransferWindow Fix
			uploadlistctrl.ShowWindow(SW_HIDE);
			queuelistctrl.ShowWindow(SW_HIDE);
			downloadclientsctrl.ShowWindow(SW_HIDE);
			clientlistctrl.ShowWindow(SW_HIDE);
			downloadlistctrl.ShowWindow(SW_SHOW);
			// [TPT] - TransferWindow Fix
			// GetDlgItem(IDC_DOWNLOAD_TEXT)->SetWindowText(GetResString(IDS_TW_DOWNLOADS)); // [TPT] - Show count number
			downloadlistctrl.ShowFilesCount();
			GetDlgItem(IDC_DL_DOWN_UPLOADS)->EnableWindow(true);
			GetDlgItem(IDC_DL_UPLOADS)->EnableWindow(true);
			GetDlgItem(IDC_DL_DOWNLOADS)->EnableWindow(false);
			GetDlgItem(IDC_DL_QUEUE)->EnableWindow(bQl);
			GetDlgItem(IDC_DL_TRANSFERS)->EnableWindow(true);
			GetDlgItem(IDC_DL_CLIENT)->EnableWindow(bKl);
			GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_HIDE);
			break;
		case IDC_UPLOADLIST:
			m_dlTab.ShowWindow(SW_HIDE); //MORPH - Added by SiRoB, Show/Hide dlTab
			uploadlistctrl.MoveWindow(rcDown);
			// [TPT] - TransferWindow Fix
			downloadlistctrl.ShowWindow(SW_HIDE);
			queuelistctrl.ShowWindow(SW_HIDE);
			clientlistctrl.ShowWindow(SW_HIDE);
			downloadclientsctrl.ShowWindow(SW_HIDE);
			uploadlistctrl.ShowWindow(SW_SHOW);
			// [TPT] - TransferWindow Fix
			// [TPT] - Show count number
			buffer.Format(_T(" (%i)"),uploadlistctrl.GetItemCount());
			GetDlgItem(IDC_DOWNLOAD_TEXT)->SetWindowText(GetResString(IDS_TW_UPLOADS)+buffer);
			// [TPT] - Show count number
			GetDlgItem(IDC_DL_DOWN_UPLOADS)->EnableWindow(true);
			GetDlgItem(IDC_DL_UPLOADS)->EnableWindow(false);
			GetDlgItem(IDC_DL_DOWNLOADS)->EnableWindow(true);
			GetDlgItem(IDC_DL_QUEUE)->EnableWindow(bQl);			
			GetDlgItem(IDC_DL_TRANSFERS)->EnableWindow(true);
			GetDlgItem(IDC_DL_CLIENT)->EnableWindow(bKl);
			GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_HIDE);
			break;
		case IDC_QUEUELIST:
			m_dlTab.ShowWindow(SW_HIDE); //MORPH - Added by SiRoB, Show/Hide dlTab
			queuelistctrl.MoveWindow(rcDown);
			// [TPT] - TransferWindow Fix
			uploadlistctrl.ShowWindow(SW_HIDE);
			downloadlistctrl.ShowWindow(SW_HIDE);
			clientlistctrl.ShowWindow(SW_HIDE);
			downloadclientsctrl.ShowWindow(SW_HIDE);
			queuelistctrl.ShowWindow(SW_SHOW);
			// [TPT] - TransferWindow Fix
			// [TPT] - Show count number
			buffer.Format(_T(" (%i)"),queuelistctrl.GetItemCount());
			GetDlgItem(IDC_DOWNLOAD_TEXT)->SetWindowText(GetResString(IDS_ONQUEUE)+buffer);
			// [TPT] - Show count number
			GetDlgItem(IDC_DL_DOWN_UPLOADS)->EnableWindow(true);
			GetDlgItem(IDC_DL_UPLOADS)->EnableWindow(true);
			GetDlgItem(IDC_DL_DOWNLOADS)->EnableWindow(true);
			GetDlgItem(IDC_DL_QUEUE)->EnableWindow(false);
			GetDlgItem(IDC_DL_TRANSFERS)->EnableWindow(true);
			GetDlgItem(IDC_DL_CLIENT)->EnableWindow(bKl);
			GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_SHOW);
			break;
		case IDC_DOWNLOADCLIENTS:
			m_dlTab.ShowWindow(SW_HIDE); //MORPH - Added by SiRoB, Show/Hide dlTab
			downloadclientsctrl.MoveWindow(rcDown);
			// [TPT] - TransferWindow Fix
			uploadlistctrl.ShowWindow(SW_HIDE);
			downloadlistctrl.ShowWindow(SW_HIDE);
			queuelistctrl.ShowWindow(SW_HIDE);
			clientlistctrl.ShowWindow(SW_HIDE);
			downloadclientsctrl.ShowWindow(SW_SHOW);
			// [TPT] - TransferWindow Fix
			// [TPT] - Show count number
			buffer.Format(_T(" (%i)"),downloadclientsctrl.GetItemCount());
			GetDlgItem(IDC_DOWNLOAD_TEXT)->SetWindowText(GetResString(IDS_TW_TRANSFERS)+buffer);
			// [TPT] - Show count number
			GetDlgItem(IDC_DL_DOWN_UPLOADS)->EnableWindow(true);
			GetDlgItem(IDC_DL_UPLOADS)->EnableWindow(true);
			GetDlgItem(IDC_DL_DOWNLOADS)->EnableWindow(true);
			GetDlgItem(IDC_DL_QUEUE)->EnableWindow(bQl);
			GetDlgItem(IDC_DL_TRANSFERS)->EnableWindow(false);
			GetDlgItem(IDC_DL_CLIENT)->EnableWindow(bKl);
			GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_HIDE);
			break;
		case IDC_CLIENTLIST:
			m_dlTab.ShowWindow(SW_HIDE); //MORPH - Added by SiRoB, Show/Hide dlTab
			clientlistctrl.MoveWindow(rcDown);
			// [TPT] - TransferWindow Fix
			uploadlistctrl.ShowWindow(SW_HIDE);
			downloadlistctrl.ShowWindow(SW_HIDE);
			queuelistctrl.ShowWindow(SW_HIDE);
			downloadclientsctrl.ShowWindow(SW_HIDE);
			clientlistctrl.ShowWindow(SW_SHOW);
			// [TPT] - TransferWindow Fix
			// [TPT] - Show count number
			buffer.Format(_T(" (%i)"),clientlistctrl.GetItemCount());
			GetDlgItem(IDC_DOWNLOAD_TEXT)->SetWindowText(GetResString(IDS_CLIENTLIST)+buffer);
			// [TPT] - Show count number
			GetDlgItem(IDC_DL_DOWN_UPLOADS)->EnableWindow(true);
			GetDlgItem(IDC_DL_UPLOADS)->EnableWindow(true);
			GetDlgItem(IDC_DL_DOWNLOADS)->EnableWindow(true);
			GetDlgItem(IDC_DL_QUEUE)->EnableWindow(bQl);
			GetDlgItem(IDC_DL_TRANSFERS)->EnableWindow(true);
			GetDlgItem(IDC_DL_CLIENT)->EnableWindow(false);
			GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_HIDE);
			break;

	}
	AddAnchor(list,TOP_LEFT,BOTTOM_RIGHT);
	//showlist = list; // [TPT] - TransferWindow Fix
}
// [TPT] - TBH Transfers Window Style
void CTransferWnd::OnBnClickedDownUploads() 
{
	m_dlTab.ShowWindow(SW_SHOW); //MORPH - Added by SiRoB, Show/Hide dlTab
	if (showlist == IDC_DOWNLOADLIST+IDC_UPLOADLIST)
		return;

	if(!(showlist == IDC_DOWNLOADLIST))//Change the icon only if we didn´t have the down before
		ChangeDlIcon(0);

	CRect rcDown,rcUp,rcSpl,rcW;
	CWnd* pWnd;

	GetWindowRect(rcW);
	ScreenToClient(rcW);

	LONG splitpos=(thePrefs.GetSplitterbarPosition()*rcW.Height())/100;

	pWnd = GetDlgItem(IDC_DOWNLOADLIST);
	pWnd->GetWindowRect(rcDown);
	ScreenToClient(rcDown);
	rcDown.bottom=splitpos+5;
	downloadlistctrl.MoveWindow(rcDown);

	pWnd = GetDlgItem(IDC_UPLOADLIST);
	pWnd->GetWindowRect(rcDown);
	ScreenToClient(rcDown);
	rcDown.right=rcW.right-7;
	rcDown.bottom=rcW.bottom-20;
	rcDown.top=splitpos+30;
	uploadlistctrl.MoveWindow(rcDown);

	pWnd = GetDlgItem(IDC_QUEUELIST);
	pWnd->GetWindowRect(rcDown);
	ScreenToClient(rcDown);
	rcDown.right=rcW.right-7;
	rcDown.bottom=rcW.bottom-20;
	rcDown.top=splitpos+30;
	queuelistctrl.MoveWindow(rcDown);

	pWnd = GetDlgItem(IDC_CLIENTLIST);
	pWnd->GetWindowRect(rcDown);
	ScreenToClient(rcDown);
	rcDown.right=rcW.right-7;
	rcDown.bottom=rcW.bottom-20;
	rcDown.top=splitpos+30;
	clientlistctrl.MoveWindow(rcDown);

	pWnd = GetDlgItem(IDC_DOWNLOADCLIENTS);
	pWnd->GetWindowRect(rcDown);
	ScreenToClient(rcDown);
	rcDown.right=rcW.right-7;
	rcDown.bottom=rcW.bottom-20;
	rcDown.top=splitpos+30;
	downloadclientsctrl.MoveWindow(rcDown);

	rcSpl=rcDown;
	rcSpl.top=rcDown.bottom+4;rcSpl.bottom=rcSpl.top+5;rcSpl.left=150;
	if (!m_wndSplitter)
		m_wndSplitter.Create(WS_CHILD | WS_VISIBLE, rcSpl, this, IDC_SPLITTER);
	else
		m_wndSplitter.MoveWindow(rcSpl,true);

	DoResize(0);

	showlist=IDC_DOWNLOADLIST+IDC_UPLOADLIST; // [TPT] - TransferWindow Fix

	downloadlistctrl.ShowFilesCount();
	GetDlgItem(IDC_UPLOAD_ICO)->ShowWindow(SW_SHOW); // [TPT] - TransferWindow Fix
	//[TPT] - Switch List Icons
	GetDlgItem(IDC_STATIC7)->ShowWindow(SW_SHOW);
	m_btnULChangeView.ShowWindow(SW_SHOW);
	m_btnULClients.ShowWindow(SW_SHOW);
	m_btnULQueue.ShowWindow(SW_SHOW);
	m_btnULTransfers.ShowWindow(SW_SHOW);
	m_btnULUploads.ShowWindow(SW_SHOW);
	//[TPT] - Switch List Icons

	RemoveAnchor(IDC_DOWNLOADLIST);
	RemoveAnchor(IDC_UPLOADLIST);
	RemoveAnchor(IDC_QUEUELIST);
	RemoveAnchor(IDC_DOWNLOADCLIENTS);
	RemoveAnchor(IDC_CLIENTLIST);
	RemoveAnchor(IDC_UPLOAD_ICO);

	AddAnchor(IDC_DOWNLOADLIST,TOP_LEFT,CSize(100, thePrefs.GetSplitterbarPosition() ));
	AddAnchor(IDC_UPLOADLIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
	AddAnchor(IDC_QUEUELIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
	AddAnchor(IDC_CLIENTLIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
	AddAnchor(IDC_DOWNLOADCLIENTS,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
	AddAnchor(IDC_UPLOAD_ICO,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);

	// [TPT] - TransferWindow Fix
	downloadlistctrl.ShowWindow(SW_SHOW);
	
	switch (m_uWnd2)
	{
	case 0:
		{
			uploadlistctrl.ShowWindow(SW_HIDE);
			queuelistctrl.ShowWindow(SW_HIDE);
			clientlistctrl.ShowWindow(SW_HIDE);
			GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_HIDE);
			downloadclientsctrl.ShowWindow(SW_SHOW);
			break;
		}
	case 1:
		{
			queuelistctrl.ShowWindow(SW_HIDE);
			clientlistctrl.ShowWindow(SW_HIDE);
			downloadclientsctrl.ShowWindow(SW_HIDE);
			GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_HIDE);
			uploadlistctrl.ShowWindow(SW_SHOW);
			break;
		}
	case 2:
		{
			uploadlistctrl.ShowWindow(SW_HIDE);
			clientlistctrl.ShowWindow(SW_HIDE);
			downloadclientsctrl.ShowWindow(SW_HIDE);
			GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_SHOW);
			queuelistctrl.ShowWindow(SW_SHOW);
			break;
		}	
	case 3:
		{
			uploadlistctrl.ShowWindow(SW_HIDE);
			queuelistctrl.ShowWindow(SW_HIDE);
			downloadclientsctrl.ShowWindow(SW_HIDE);
			GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_HIDE);
			clientlistctrl.ShowWindow(SW_SHOW);
			break;
		}	
	default:
		{

			queuelistctrl.ShowWindow(SW_HIDE);
			clientlistctrl.ShowWindow(SW_HIDE);
			downloadclientsctrl.ShowWindow(SW_HIDE);
			GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_HIDE);
			uploadlistctrl.ShowWindow(SW_SHOW);
			m_uWnd2 = 1;

		}
	}

	bKl = !thePrefs.m_bDisableKnownClientList;
	bQl = !thePrefs.m_bDisableQueueList;

	GetDlgItem(IDC_DL_DOWN_UPLOADS)->EnableWindow(false);
	GetDlgItem(IDC_DL_UPLOADS)->EnableWindow(true);
	GetDlgItem(IDC_DL_DOWNLOADS)->EnableWindow(true);
	GetDlgItem(IDC_DL_QUEUE)->EnableWindow(bQl);
	GetDlgItem(IDC_DL_TRANSFERS)->EnableWindow(true);
	GetDlgItem(IDC_DL_CLIENT)->EnableWindow(bKl);

	UpdateListCount(m_uWnd2);
	// [TPT] - TransferWindow Fix

}




void CTransferWnd::OnHoverUploadList(NMHDR *pNMHDR, LRESULT *pResult)
{
	downloadlistactive=false;
	*pResult = 0;
}

void CTransferWnd::OnHoverDownloadList(NMHDR *pNMHDR, LRESULT *pResult)
{
	downloadlistactive=true;
	*pResult = 0;
}

void CTransferWnd::OnTcnSelchangeDltab(NMHDR *pNMHDR, LRESULT *pResult)
{
	downloadlistctrl.ChangeCategory(m_dlTab.GetCurSel());
	*pResult = 0;
}

// Ornis' download categories
// [TPT] - khaos::categorymod+
void CTransferWnd::OnNMRclickDltab(NMHDR *pNMHDR, LRESULT *pResult)
{
	// Menu for category
	// [TPT] - New Menu Styles BEGIN
	//CTitleMenu menu;
	CMenuXP	*pMenu = new CMenuXP;
	POINT point;
	::GetCursorPos(&point);

	CPoint pt(point);
	rightclickindex=GetTabUnderMouse(&pt);
	if (rightclickindex==-1)
		return;

	UINT flag;
	flag=(rightclickindex==0) ? MF_GRAYED:MF_STRING;



	// Create the main menu...	
	pMenu->CreatePopupMenu();
	pMenu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
	pMenu->AddSideBar(new CMenuXPSideBar(17, GetResString(IDS_CAT)));
	pMenu->SetSideBarStartColor(RGB(255,0,0));
	pMenu->SetSideBarEndColor(RGB(255,128,0));
	pMenu->SetSelectedBarColor(RGB(242,120,114));


	// Create sub-menus first...

	// [TPT] - ZZ:DownloadManager -->
	Category_Struct* category_Struct = thePrefs.GetCategory(rightclickindex);

	// Priority Menu
	CMenuXP	*pPrioMenu = new CMenuXP;
	pPrioMenu->CreatePopupMenu();
	pPrioMenu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
	pPrioMenu->SetSelectedBarColor(RGB(242,120,114));
	pPrioMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_PRIOLOW,GetResString(IDS_PRIOLOW)));
	pPrioMenu->CheckMenuItem(MP_PRIOLOW, category_Struct && category_Struct->prio == PR_LOW ? MF_CHECKED : MF_UNCHECKED);	
	pPrioMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_PRIONORMAL,GetResString(IDS_PRIONORMAL)));
	pPrioMenu->CheckMenuItem(MP_PRIONORMAL, category_Struct && category_Struct->prio != PR_LOW && category_Struct->prio != PR_HIGH ? MF_CHECKED : MF_UNCHECKED);
	pPrioMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_PRIOHIGH, GetResString(IDS_PRIOHIGH)));
	pPrioMenu->CheckMenuItem(MP_PRIOHIGH, category_Struct && category_Struct->prio == PR_HIGH ? MF_CHECKED : MF_UNCHECKED);
	//pPrioMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_PRIOAUTO, GetResString(IDS_PRIOAUTO)));
	// <-- [TPT] - ZZ:DownloadManager	
	
	// View Filter Menu
	CMenuXP	*pCatViewFilter = new CMenuXP;
	pCatViewFilter->CreatePopupMenu();
	pCatViewFilter->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
	pCatViewFilter->SetSelectedBarColor(RGB(242,120,114));
	if (rightclickindex==0 ){
		pCatViewFilter->AppendODMenu(MF_STRING,new CMenuXPText(MP_CAT_SET0, GetResString(IDS_ALL)));	
		pCatViewFilter->AppendODMenu(MF_STRING,new CMenuXPText(MP_CAT_SET0+2, GetResString(IDS_CAT_THISCAT)));

		pCatViewFilter->AppendSeparator();		
		pCatViewFilter->AppendODMenu(MF_STRING,new CMenuXPText(MP_CAT_SET0+3, GetResString(IDS_COMPLETE)));
		pCatViewFilter->AppendODMenu(MF_STRING,new CMenuXPText(MP_CAT_SET0+4, GetResString(IDS_COMPLETING)));
		pCatViewFilter->AppendODMenu(MF_STRING,new CMenuXPText(MP_CAT_SET0+5, GetResString(IDS_DOWNLOADING)));
		pCatViewFilter->AppendODMenu(MF_STRING,new CMenuXPText(MP_CAT_SET0+6, GetResString(IDS_WAITING)));
		pCatViewFilter->AppendODMenu(MF_STRING,new CMenuXPText(MP_CAT_SET0+7, GetResString(IDS_PAUSED)));
		pCatViewFilter->AppendODMenu(MF_STRING,new CMenuXPText(MP_CAT_SET0+8, GetResString(IDS_STOPPED)));
		pCatViewFilter->AppendODMenu(MF_STRING,new CMenuXPText(MP_CAT_SET0+9, GetResString(IDS_HASHING)));
		pCatViewFilter->AppendODMenu(MF_STRING,new CMenuXPText(MP_CAT_SET0+10, GetResString(IDS_ERRORLIKE)));
			
		pCatViewFilter->AppendSeparator();			
		pCatViewFilter->AppendODMenu(MF_STRING,new CMenuXPText(MP_CAT_SET0+11, GetResString(IDS_VIDEO)));
		pCatViewFilter->AppendODMenu(MF_STRING,new CMenuXPText(MP_CAT_SET0+12, GetResString(IDS_AUDIO)));
		pCatViewFilter->AppendODMenu(MF_STRING,new CMenuXPText(MP_CAT_SET0+13, GetResString(IDS_SEARCH_ARC)));
		pCatViewFilter->AppendODMenu(MF_STRING,new CMenuXPText(MP_CAT_SET0+14, GetResString(IDS_SEARCH_CDIMG)));
		
		pCatViewFilter->AppendSeparator();			
		pCatViewFilter->AppendODMenu(MF_STRING,new CMenuXPText(MP_CAT_SET0+15, GetResString(IDS_CAT_SUSPENDFILTERS)));
		pCatViewFilter->AppendODMenu(MF_STRING,new CMenuXPText(MP_CAT_SET0+16, GetResString(IDS_CAT_EDIT)));

		Category_Struct* curCat = thePrefs.GetCategory(rightclickindex);

		// Check and enable the appropriate menu items in Select View Filter
		pCatViewFilter->CheckMenuItem(MP_CAT_SET0, (curCat->viewfilters.nFromCats == 0) ? MF_CHECKED : MF_UNCHECKED);
		//m_mnuCatViewFilter.CheckMenuItem(MP_CAT_SET0+1, (curCat->viewfilters.nFromCats == 1) ? MF_CHECKED : MF_UNCHECKED);
		pCatViewFilter->CheckMenuItem(MP_CAT_SET0+2, (curCat->viewfilters.nFromCats == 2) ? MF_CHECKED : MF_UNCHECKED);
		
		pCatViewFilter->CheckMenuItem(MP_CAT_SET0+3, (curCat->viewfilters.bComplete) ? MF_CHECKED : MF_UNCHECKED);
		pCatViewFilter->CheckMenuItem(MP_CAT_SET0+4, (curCat->viewfilters.bCompleting) ? MF_CHECKED : MF_UNCHECKED);
		pCatViewFilter->CheckMenuItem(MP_CAT_SET0+5, (curCat->viewfilters.bTransferring) ? MF_CHECKED : MF_UNCHECKED);
		pCatViewFilter->CheckMenuItem(MP_CAT_SET0+6, (curCat->viewfilters.bWaiting) ? MF_CHECKED : MF_UNCHECKED);
		pCatViewFilter->CheckMenuItem(MP_CAT_SET0+7, (curCat->viewfilters.bPaused) ? MF_CHECKED : MF_UNCHECKED);
		pCatViewFilter->CheckMenuItem(MP_CAT_SET0+8, (curCat->viewfilters.bStopped) ? MF_CHECKED : MF_UNCHECKED);
		pCatViewFilter->CheckMenuItem(MP_CAT_SET0+9, (curCat->viewfilters.bHashing) ? MF_CHECKED : MF_UNCHECKED);
		pCatViewFilter->CheckMenuItem(MP_CAT_SET0+10, (curCat->viewfilters.bErrorUnknown) ? MF_CHECKED : MF_UNCHECKED);

		pCatViewFilter->CheckMenuItem(MP_CAT_SET0+11, (curCat->viewfilters.bVideo) ? MF_CHECKED : MF_UNCHECKED);
		pCatViewFilter->CheckMenuItem(MP_CAT_SET0+12, (curCat->viewfilters.bAudio) ? MF_CHECKED : MF_UNCHECKED);
		pCatViewFilter->CheckMenuItem(MP_CAT_SET0+13, (curCat->viewfilters.bArchives) ? MF_CHECKED : MF_UNCHECKED);
		pCatViewFilter->CheckMenuItem(MP_CAT_SET0+14, (curCat->viewfilters.bImages) ? MF_CHECKED : MF_UNCHECKED);
		pCatViewFilter->CheckMenuItem(MP_CAT_SET0+15, (curCat->viewfilters.bSuspendFilters) ? MF_CHECKED : MF_UNCHECKED);

		pMenu->AppendODPopup(MF_STRING|MF_POPUP, pCatViewFilter, new CMenuXPText(0, GetResString(IDS_CHANGECATVIEW) ));
		pMenu->AppendSeparator();
	}
	
	pMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_CAT_ADD,GetResString(IDS_CAT_ADD) ));
	pMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_CAT_EDIT,GetResString(IDS_CAT_EDIT) ));
	pMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_CAT_MERGE,GetResString(IDS_CAT_MERGE) ));
	pMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_CAT_REMOVE,GetResString(IDS_CAT_REMOVE) ));
	pMenu->AppendSeparator();	
	pMenu->AppendODPopup(MF_STRING | MF_POPUP, pPrioMenu, new CMenuXPText(0, GetResString(IDS_PRIORITY), theApp.LoadIcon(_T("priority"), 16, 16)));
	pMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_CANCEL,GetResString(IDS_MAIN_BTN_CANCEL), theApp.LoadIcon(_T("delete"), 16, 16)) );
	pMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_STOP, GetResString(IDS_DL_STOP), theApp.LoadIcon(_T("stop"), 16, 16)));
	pMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_PAUSE, GetResString(IDS_DL_PAUSE), theApp.LoadIcon(_T("pause"), 16, 16)));
	pMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_RESUME, GetResString(IDS_DL_RESUME), theApp.LoadIcon(_T("start"), 16, 16)));
	pMenu->AppendSeparator();		
	pMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_CAT_STOPLAST,GetResString(IDS_CAT_STOPLAST) ));			
	pMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_CAT_PAUSELAST,GetResString(IDS_CAT_PAUSELAST) ));			
	pMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_CAT_RESUMENEXT,GetResString(IDS_CAT_RESUMENEXT) ));	

	pMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_HM_OPENINC, GetResString(IDS_OPENINC) + _T("...") )); // [TPT] - Incoming folder

	// [TPT] - ZZ:DownloadManager -->
    if(rightclickindex != 0 && thePrefs.IsExtControlsEnabled())
	{
		pMenu->AppendODMenu(MF_STRING,new CMenuXPText(MP_DOWNLOAD_LINEAL_PRIORITY,GetResString(IDS_DOWNLOAD_LINEAL_PRIORITY)));	        
        pMenu->CheckMenuItem(MP_DOWNLOAD_LINEAL_PRIORITY, category_Struct && category_Struct->downloadInLinealPriorityOrder? MF_CHECKED : MF_UNCHECKED);
    }
	// <-- [TPT] - ZZ:DownloadManager

	// If the current category is '0'...  Well, we can't very well delete the default category, now can we...
	// Nor can we merge it.
	pMenu->EnableMenuItem(MP_CAT_REMOVE, rightclickindex == 0 ? MF_GRAYED : MF_ENABLED);
	pMenu->EnableMenuItem(MP_CAT_MERGE, rightclickindex == 0 ? MF_GRAYED : MF_ENABLED);	

	
	pMenu->TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
	delete pPrioMenu;
	delete pCatViewFilter;
	delete pMenu;
	// [TPT] - New Menu Styles END

	*pResult = 0;
}

void CTransferWnd::OnLvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult)
{
    int iSel = downloadlistctrl.GetSelectionMark();
	if (iSel==-1) return;
	if (((CtrlItem_Struct*)downloadlistctrl.GetItemData(iSel))->type != FILE_TYPE) return;
	
	m_bIsDragging = true;

	POINT pt;
	::GetCursorPos(&pt);

	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	m_nDragIndex = pNMLV->iItem;
	m_pDragImage = downloadlistctrl.CreateDragImage( downloadlistctrl.GetSelectionMark() ,&pt);
    m_pDragImage->BeginDrag( 0, CPoint(0,0) );
    m_pDragImage->DragEnter( GetDesktopWindow(), pNMLV->ptAction );
    SetCapture();
	m_nDropIndex = -1;

	*pResult = 0;
}

void CTransferWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	if( !(nFlags & MK_LBUTTON) ) m_bIsDragging = false;

	if (m_bIsDragging){
		CPoint pt(point);           //get our current mouse coordinates
		ClientToScreen(&pt);        //convert to screen coordinates

		m_nDropIndex=GetTabUnderMouse(&pt);
		m_dlTab.SetCurSel(m_nDropIndex);
		m_dlTab.Invalidate();
		
		::GetCursorPos(&pt);
		pt.y-=10;
		m_pDragImage->DragMove(pt); //move the drag image to those coordinates
	}
}

// [TPT] - khaos::categorymod+
void CTransferWnd::OnLButtonUp(UINT nFlags, CPoint point)
{

	if (m_bIsDragging)
	{
		ReleaseCapture ();
		m_bIsDragging = false;
		m_pDragImage->DragLeave (GetDesktopWindow ());
		m_pDragImage->EndDrag ();
		delete m_pDragImage;
		
		if (m_nDropIndex>-1 && (downloadlistctrl.curTab==0 ||
				(downloadlistctrl.curTab>0 && m_nDropIndex!=downloadlistctrl.curTab) )) {

			CPartFile* file;

			// for multiselections
			CTypedPtrList <CPtrList,CPartFile*> selectedList; 
			POSITION pos = downloadlistctrl.GetFirstSelectedItemPosition();
			while(pos != NULL) 
			{ 
				int index = downloadlistctrl.GetNextSelectedItem(pos);
				if(index > -1 && (((CtrlItem_Struct*)downloadlistctrl.GetItemData(index))->type == FILE_TYPE))
					selectedList.AddTail( (CPartFile*)((CtrlItem_Struct*)downloadlistctrl.GetItemData(index))->value );
			}

			while (!selectedList.IsEmpty())
			{
				file = selectedList.GetHead();
				selectedList.RemoveHead();
				file->SetCategory(m_nDropIndex);
			}


			m_dlTab.SetCurSel(downloadlistctrl.curTab);
			//if (m_dlTab.GetCurSel()>0 || (thePrefs.GetAllcatType()==1 && m_dlTab.GetCurSel()==0) )
			downloadlistctrl.UpdateCurrentCategoryView();

			UpdateCatTabTitles();

		} else m_dlTab.SetCurSel(downloadlistctrl.curTab);
		downloadlistctrl.Invalidate();
	}
}
// [TPT] - khaos::categorymod-

BOOL CTransferWnd::OnCommand(WPARAM wParam,LPARAM lParam ){ 

	// [TPT] - khaos::categorymod+
	Category_Struct* curCat;
	curCat = thePrefs.GetCategory(rightclickindex);
	// [TPT] - khaos::categorymod-

	switch (wParam){ 
		case MP_CAT_ADD: {
			m_nLastCatTT=-1;
			int newindex=AddCategory(_T("?"),thePrefs.GetIncomingDir(),_T(""),_T(""),false);
			CCatDialog dialog(newindex);
			dialog.DoModal();
			if (dialog.WasCancelled())
				thePrefs.RemoveCat(newindex);
			else {
			// [TPT] - khaos::categorymod+ obsolete //theApp.emuledlg->searchwnd.UpdateCatTabs();
				m_dlTab.InsertItem(newindex,thePrefs.GetCategory(newindex)->title);
				EditCatTabLabel(newindex,thePrefs.GetCategory(newindex)->title);
				thePrefs.SaveCats();
				VerifyCatTabSize();
			}
			break;
		}
		// [TPT] - khaos::categorymod+						 
		case MP_CAT_SET0+16:
		case MP_CAT_EDIT: {
		// [TPT] - khaos::categorymod-
			m_nLastCatTT=-1;
			CCatDialog dialog(rightclickindex);
			dialog.DoModal();

			CString csName;
			csName.Format(_T("%s"), thePrefs.GetCategory(rightclickindex)->title );
			EditCatTabLabel(rightclickindex,csName);
		
			// [TPT] - khaos::categorymod+ obsolete //theApp.emuledlg->searchwnd.UpdateCatTabs();
			theApp.emuledlg->transferwnd->downloadlistctrl.UpdateCurrentCategoryView();
			thePrefs.SaveCats();
			break;
		}
		// [TPT] - khaos::categorymod+ Merge Category
		case MP_CAT_MERGE: {
			uint8 useCat;

			CSelCategoryDlg* getCatDlg = new CSelCategoryDlg((CWnd*)theApp.emuledlg);
			int nResult = getCatDlg->DoModal();

			if (nResult != IDOK)
				break;

			useCat = getCatDlg->GetInput();
			delete getCatDlg;

			if (useCat == rightclickindex) break;
			m_nLastCatTT=-1;

			if (useCat > rightclickindex) useCat--;

			theApp.downloadqueue->ResetCatParts(rightclickindex, useCat);
			thePrefs.RemoveCat(rightclickindex);
			m_dlTab.DeleteItem(rightclickindex);
			m_dlTab.SetCurSel(useCat);
			theApp.emuledlg->transferwnd->downloadlistctrl.UpdateCurrentCategoryView();
			thePrefs.SaveCats();
			break;
		}
		// [TPT] - khaos::categorymod-
		case MP_CAT_REMOVE: {
			m_nLastCatTT=-1;
			theApp.downloadqueue->ResetCatParts(rightclickindex);
			thePrefs.RemoveCat(rightclickindex);
			m_dlTab.DeleteItem(rightclickindex);
			m_dlTab.SetCurSel(0);
			downloadlistctrl.ChangeCategory(0);
			thePrefs.SaveCats();
			// [TPT] - khaos::categorymod+
			//if (thePrefs.GetCatCount()==1) thePrefs.SetAllcatType(0);
			//theApp.emuledlg->searchwnd->UpdateCatTabs();
			// [TPT] - khaos::categorymod-
			VerifyCatTabSize();
			break;
		}
// ZZ:DownloadManager -->
		case MP_PRIOLOW: {
            thePrefs.GetCategory(rightclickindex)->prio = PR_LOW;
			
            //CString csName;
            //csName.Format(_T("%s"), thePrefs.GetCategory(rightclickindex)->title );
            //EditCatTabLabel(rightclickindex,csName);

            //theApp.emuledlg->searchwnd->UpdateCatTabs();
			thePrefs.SaveCats();
			UpdateCatTabTitles();
			break;
		}
		case MP_PRIONORMAL: {
            thePrefs.GetCategory(rightclickindex)->prio = PR_NORMAL;
			
            //CString csName;
            //csName.Format(_T("%s"), thePrefs.GetCategory(rightclickindex)->title );
            //EditCatTabLabel(rightclickindex,csName);

            //theApp.emuledlg->searchwnd->UpdateCatTabs();
			thePrefs.SaveCats();
			UpdateCatTabTitles();
			break;
		}
		case MP_PRIOHIGH: {
            thePrefs.GetCategory(rightclickindex)->prio = PR_HIGH;
			
            //CString csName;
            //csName.Format(_T("%s"), thePrefs.GetCategory(rightclickindex)->title );
            //EditCatTabLabel(rightclickindex,csName);

            //theApp.emuledlg->searchwnd->UpdateCatTabs();
			thePrefs.SaveCats();
			UpdateCatTabTitles();
			break;
		}
// <-- ZZ:DownloadManager

		case MP_PAUSE: {
			theApp.downloadqueue->SetCatStatus(rightclickindex,MP_PAUSE);
			break;
		}
		case MP_STOP : {
				theApp.downloadqueue->SetCatStatus(rightclickindex,MP_STOP);
			break;
		}
		case MP_CANCEL: {
			if (AfxMessageBox(GetResString(IDS_Q_CANCELDL),MB_ICONQUESTION|MB_YESNO) == IDYES)
				theApp.downloadqueue->SetCatStatus(rightclickindex,MP_CANCEL);
			break;
		}
		case MP_RESUME: {
			theApp.downloadqueue->SetCatStatus(rightclickindex,MP_RESUME);
			break;
		}
		// [TPT] - khaos::categorymod+
		case MP_CAT_RESUMENEXT: {
			theApp.downloadqueue->StartNextFile(rightclickindex,false);
			break;
		}
		case MP_CAT_STOPLAST: {
			theApp.downloadqueue->StopPauseLastFile(MP_STOP, rightclickindex);
			break;
		}
		case MP_CAT_PAUSELAST: {
			theApp.downloadqueue->StopPauseLastFile(MP_PAUSE, rightclickindex);
			break;
		}
		// [TPT] - khaos::categorymod-
		// [TPT] - ZZ:DownloadManager -->
		case MP_DOWNLOAD_LINEAL_PRIORITY: {
            BOOL newSetting = !thePrefs.GetCategory(rightclickindex)->downloadInLinealPriorityOrder;
            thePrefs.GetCategory(rightclickindex)->downloadInLinealPriorityOrder = newSetting;
			thePrefs.SaveCats();
            if(newSetting) {
                // any auto prio files will be set to normal now.
                theApp.downloadqueue->RemoveAutoPrioInCat(rightclickindex, PR_NORMAL);
            }

            break;
		}
		// <-- [TPT] - ZZ:DownloadManager
		// [TPT] - TBH Transfers Window Style
		case IDC_UPLOAD_ICO: {
			SwitchUploadList();
			break;
		}
		case IDC_QUEUE_REFRESH_BUTTON: {
			OnBnClickedQueueRefreshButton();
			break;
		}

		case MP_HM_OPENINC:
			ShellExecute(NULL, _T("open"), thePrefs.GetCategory(rightclickindex)->incomingpath,NULL, NULL, SW_SHOW);
			break;

		case IDC_DL_CHANGEVIEW: {
			OnBnClickedChangeView();
			break;
								}
		case IDC_DL_DOWN_UPLOADS: {
			OnBnClickedDownUploads();
			break;
								  }
		case IDC_DL_DOWNLOADS: {
			ShowList(IDC_DOWNLOADLIST);
			if(!(showlist == IDC_DOWNLOADLIST + IDC_UPLOADLIST))
				ChangeDlIcon(0);
			break;
							   }
		case IDC_DL_UPLOADS: {
			ShowList(IDC_UPLOADLIST);
			ChangeDlIcon(1);
			break;
							 }
		case IDC_DL_QUEUE: {
			ShowList(IDC_QUEUELIST);
			ChangeDlIcon(2);
			break;
						   }
		case IDC_DL_TRANSFERS: {
			ShowList(IDC_DOWNLOADCLIENTS);
			ChangeDlIcon(4);
			break;
							   }
		case IDC_DL_CLIENT: {
			ShowList(IDC_CLIENTLIST);
			ChangeDlIcon(3);
			break;
							}
		//[TPT] - Switch List Icons
		case IDC_UL_CHANGEVIEW: 
			{
				SwitchUploadList();
				break;
			}
		case IDC_UL_UPLOADS: 
			{
				ShowWnd2(m_uWnd2 = 1);
			break;
			}
		case IDC_UL_QUEUE: 
			{
				ShowWnd2(m_uWnd2 = 2);
				break;
			}
		case IDC_UL_CLIENTS: 
			{
				ShowWnd2(m_uWnd2 = 3);
				break;
			}
		case IDC_UL_TRANSFERS: 
			{
				ShowWnd2(m_uWnd2 = 0);
				break;
			}
		//[TPT] - Switch List Icons
						
		// [TPT] - TBH Transfers Window Style
		// [TPT] - khaos::categorymod+ Handle the new view filter menu.
		case MP_CAT_SET0+1:
			if (rightclickindex != 0 && theApp.downloadqueue->GetCategoryFileCount(rightclickindex))
			{
				MessageBox(GetResString(IDS_CAT_FROMCATSINFO), GetResString(IDS_CAT_FROMCATSCAP));
				curCat->viewfilters.nFromCats = 2;
				EditCatTabLabel(rightclickindex, CString(curCat->title));
				break;
			}
		case MP_CAT_SET0:
		case MP_CAT_SET0+2: {
			curCat->viewfilters.nFromCats = wParam - MP_CAT_SET0;
			break;
		}
		case MP_CAT_SET0+3: {
			curCat->viewfilters.bComplete = curCat->viewfilters.bComplete ? false : true;
			break;
	}
		case MP_CAT_SET0+4: {
			curCat->viewfilters.bCompleting = curCat->viewfilters.bCompleting ? false : true;
			break;
		}
		case MP_CAT_SET0+5: {
			curCat->viewfilters.bTransferring = curCat->viewfilters.bTransferring ? false : true;
			break;
		}
		case MP_CAT_SET0+6: {
			curCat->viewfilters.bWaiting = curCat->viewfilters.bWaiting ? false : true;
			break;
		}
		case MP_CAT_SET0+7: {
			curCat->viewfilters.bPaused = curCat->viewfilters.bPaused ? false : true;
			break;
		}
		case MP_CAT_SET0+8: {
			curCat->viewfilters.bStopped = curCat->viewfilters.bStopped ? false : true;
			break;
		}
		case MP_CAT_SET0+9: {
			curCat->viewfilters.bHashing = curCat->viewfilters.bHashing ? false : true;
			break;
		}
		case MP_CAT_SET0+10: {
			curCat->viewfilters.bErrorUnknown = curCat->viewfilters.bErrorUnknown ? false : true;
			break;
		}
		case MP_CAT_SET0+11: {
			curCat->viewfilters.bVideo = curCat->viewfilters.bVideo ? false : true;
			break;
		}
		case MP_CAT_SET0+12: {
			curCat->viewfilters.bAudio = curCat->viewfilters.bAudio ? false : true;
			break;
		}
		case MP_CAT_SET0+13: {
			curCat->viewfilters.bArchives = curCat->viewfilters.bArchives ? false : true;
			break;
		}
		case MP_CAT_SET0+14: {
			curCat->viewfilters.bImages = curCat->viewfilters.bImages ? false : true;
			break;
		}
		case MP_CAT_SET0+15: {
			curCat->viewfilters.bSuspendFilters = curCat->viewfilters.bSuspendFilters ? false : true;
			break;
		}
	}

	if (wParam >= MP_CAT_SET0 && wParam <= MP_CAT_SET0 + 15)
		downloadlistctrl.ChangeCategory(m_dlTab.GetCurSel());
	
	// [TPT] - khaos::categorymod-
	
	return TRUE;
}

// [TPT] - khaos::categorymod+
void CTransferWnd::UpdateCatTabTitles(bool force) {


	CPoint pt;
	::GetCursorPos(&pt);
	if (!force && GetTabUnderMouse(&pt)!=-1)		// avoid cat tooltip jumping
		return;

	for (uint8 i=0;i<m_dlTab.GetItemCount();i++)
		EditCatTabLabel(i, thePrefs.GetCategory(i)->title);
}
// [TPT] - khaos::categorymod-

void CTransferWnd::EditCatTabLabel(int index,CString newlabel) {

	TCITEM tabitem;
	tabitem.mask = TCIF_PARAM;
	m_dlTab.GetItem(index,&tabitem);
	tabitem.mask = TCIF_TEXT;

	newlabel.Replace(_T("&"),_T("&&"));

	// [TPT] - khaos::categorymod
	// add filter label
	/*if (index && thePrefs.GetCatFilter(index)>0) {
		newlabel.Append(_T(" (")) ;
		if (thePrefs.GetCatFilterNeg(index))
			newlabel.Append(_T("!"));			
			newlabel.Append( GetCatTitle(thePrefs.GetCatFilter(index)) + _T(")") );
	} else
		if (!index && thePrefs.GetCatFilterNeg(index)  )
			newlabel=_T("!") + newlabel;*/
	// [TPT] - khaos::categorymod


	int count,dwl;

// ZZ:DownloadManager -->
    //CString prioStr;
    //switch(thePrefs.GetCategory(index)->prio) {
    //    case PR_LOW:
    //        prioStr = _T(" ") + GetResString(IDS_PR_SHORT_LOW);
    //        break;

    //    case PR_HIGH:
    //        prioStr = _T(" ") + GetResString(IDS_PR_SHORT_HIGH);
    //        break;

    //    default:
    //        prioStr = _T("");
    //        break;
    //}
// <-- ZZ:DownloadManager

	if (thePrefs.ShowCatTabInfos()) {
		CPartFile* cur_file;
		count=dwl=0;
		for (int i=0;i<theApp.downloadqueue->GetFileCount();i++) {
			cur_file=theApp.downloadqueue->GetFileByIndex(i);
			if (cur_file==0) continue;
			if (cur_file->CheckShowItemInGivenCat(index)) {
				if (cur_file->GetTransferringSrcCount()>0) ++dwl;
			}
		}
		CString title=newlabel;
		theApp.emuledlg->transferwnd->downloadlistctrl.GetCompleteDownloads(index,count);
		newlabel.Format(_T("%s %i/%i"),title,dwl,count); // ZZ:DownloadManager
	}

	tabitem.pszText = newlabel.LockBuffer();
	m_dlTab.SetItem(index,&tabitem);
	newlabel.UnlockBuffer();

	VerifyCatTabSize();
}

int CTransferWnd::AddCategory(CString newtitle,CString newincoming,CString newcomment, CString newautocat, bool addTab){
	Category_Struct* newcat=new Category_Struct;

	_stprintf(newcat->title,newtitle);
	newcat->prio=PR_NORMAL; // [TPT] - ZZ:DownloadManager
	_stprintf(newcat->incomingpath,newincoming);
	_stprintf(newcat->comment,newcomment);
	// newcat->autocat=newautocat; // [TPT] - khaos::categorymod
	newcat->downloadInLinealPriorityOrder = false; // [TPT] - ZZ:DownloadManager
	// [TPT] - khaos::categorymod+ Initialize View Filter Variables
	newcat->viewfilters.bArchives = true;
	newcat->viewfilters.bAudio = true;
	newcat->viewfilters.bComplete = true;
	newcat->viewfilters.bCompleting = true;
	newcat->viewfilters.bErrorUnknown = true;
	newcat->viewfilters.bHashing = true;
	newcat->viewfilters.bImages = true;
	newcat->viewfilters.bPaused = true;
	newcat->viewfilters.bStopped = true;
	newcat->viewfilters.bSuspendFilters = false;
	newcat->viewfilters.bTransferring = true;
	newcat->viewfilters.bVideo = true;
	newcat->viewfilters.bWaiting = true;
	newcat->viewfilters.nAvailSourceCountMax = 0;
	newcat->viewfilters.nAvailSourceCountMin = 0;
	newcat->viewfilters.nFromCats = 2;
	newcat->viewfilters.nFSizeMax = 0;
	newcat->viewfilters.nFSizeMin = 0;
	newcat->viewfilters.nRSizeMax = 0;
	newcat->viewfilters.nRSizeMin = 0;
	newcat->viewfilters.nSourceCountMax = 0;
	newcat->viewfilters.nSourceCountMin = 0;
	newcat->viewfilters.nTimeRemainingMax = 0;
	newcat->viewfilters.nTimeRemainingMin = 0;
	newcat->viewfilters.sAdvancedFilterMask = "";
	newcat->selectioncriteria.bAdvancedFilterMask = true;
	newcat->selectioncriteria.bFileSize = true;
	// [TPT] - khaos::categorymod-

	int index=thePrefs.AddCat(newcat);
	if (addTab) m_dlTab.InsertItem(index,newtitle);
	VerifyCatTabSize();
	
	return index;
}

int CTransferWnd::GetTabUnderMouse(CPoint* point) {

		TCHITTESTINFO hitinfo;
		CRect rect;
		m_dlTab.GetWindowRect(&rect);
		point->Offset(0-rect.left,0-rect.top);
		hitinfo.pt = *point;

		if( m_dlTab.GetItemRect( 0, &rect ) )
			if (hitinfo.pt.y< rect.top+30 && hitinfo.pt.y >rect.top-30)
				hitinfo.pt.y = rect.top;

		// Find the destination tab...
		unsigned int nTab = m_dlTab.HitTest( &hitinfo );

		if( hitinfo.flags != TCHT_NOWHERE )
			return nTab;
		else return -1;
}

void CTransferWnd::OnLvnKeydownDownloadlist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
	int iItem = downloadlistctrl.GetSelectionMark();
	if (iItem != -1)
	{
		bool bAltKey = GetAsyncKeyState(VK_MENU) < 0;
		int iAction = EXPAND_COLLAPSE;
		if (pLVKeyDow->wVKey==VK_ADD || (bAltKey && pLVKeyDow->wVKey==VK_RIGHT))
			iAction = EXPAND_ONLY;
		else if (pLVKeyDow->wVKey==VK_SUBTRACT || (bAltKey && pLVKeyDow->wVKey==VK_LEFT))
			iAction = COLLAPSE_ONLY;
		if (iAction < EXPAND_COLLAPSE)
			downloadlistctrl.ExpandCollapseItem(iItem, iAction, true);
	}
	*pResult = 0;
}

// [TPT] - MFCK [addon] - New Tooltips [Rayita]
void CTransferWnd::UpdateTabToolTips() // modified by rayita
{
	CPoint point;
	::GetCursorPos(&point);
	int Index = GetTabUnderMouse(&point);
	if (Index != m_nLastCatTT)
		{
		m_nLastCatTT = Index;
		if (m_nDropIndex != -1)
			m_tabtip.Pop();
	}
}


/*CString CTransferWnd::GetTabStatistic(uint8 tab) {
	uint16 count,dwl;
	count=dwl=0;
	float speed=0;
	uint64 size=0;
	uint64 trsize=0;
	uint64 disksize=0;

	CPartFile* cur_file;

	for (int i=0;i<theApp.downloadqueue->GetFileCount();++i) {
		cur_file=theApp.downloadqueue->GetFileByIndex(i);
		if (cur_file==0) continue;
		if (cur_file->CheckShowItemInGivenCat(tab)) {
			count++;
			if (cur_file->GetTransferringSrcCount()>0) ++dwl;
			// [TPT]
			speed+=cur_file->GetDownloadDatarate()/1024.0f; // Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
			size+=cur_file->GetFileSize();
			trsize+=cur_file->GetCompletedSize();
			disksize+=cur_file->GetRealFileSize();
		}
	}

	int total;
	compl=theApp.emuledlg->transferwnd->downloadlistctrl.GetCompleteDownloads(tab,total);

// ZZ:DownloadManager -->
    CString prio;
    switch(thePrefs.GetCategory(tab)->prio) {
        case PR_LOW:
            prio = GetResString(IDS_PRIOLOW);
            break;

        case PR_HIGH:
            prio = GetResString(IDS_PRIOHIGH);
            break;

        default:
            prio = GetResString(IDS_PRIONORMAL);
            break;
    }
// ZZ:DownloadManager <--

	CString title;
	title.Format(_T("%s: %i\n\n%s: %i\n%s: %i\n%s: %i\n%s: %i\n\n%s: %s\n\n%s: %.1f %s\n%s: %s/%s\n%s%s"), // ZZ:DownloadManager
		
		GetResString(IDS_FILES), count+compl,
		GetResString(IDS_DOWNLOADING), dwl,
		GetResString(IDS_PAUSED) ,paus,
		GetResString(IDS_ERRORLIKE) ,err,
		GetResString(IDS_DL_TRANSFCOMPL) ,compl,

        GetResString(IDS_PRIORITY), prio, // ZZ:DownloadManager

		GetResString(IDS_DL_SPEED) ,speed,GetResString(IDS_KBYTESEC),
		

		GetResString(IDS_DL_SIZE),CastItoXBytes(trsize, false, false),CastItoXBytes(size, false, false),
		GetResString(IDS_ONDISK),CastItoXBytes(disksize, false, false));
	return title;
}*/
// [TPT] - MFCK [addon] - New Tooltips [Rayita]

void CTransferWnd::OnDblclickDltab(){
	POINT point;
	::GetCursorPos(&point);
	CPoint pt(point);
	int tab=GetTabUnderMouse(&pt);
	if (tab<1) return;
	rightclickindex=tab;
	OnCommand(MP_CAT_EDIT,0);
}

void CTransferWnd::OnTabMovement(NMHDR *pNMHDR, LRESULT *pResult) {
	UINT from=m_dlTab.GetLastMovementSource();
	UINT to=m_dlTab.GetLastMovementDestionation();

	if (from==0 || to==0 || from==to-1) return;

	// do the reorder
	
	// rearrange the cat-map
	if (!thePrefs.MoveCat(from,to)) return;

	// update partfile-stored assignment
	theApp.downloadqueue->MoveCat((uint8)from,(uint8)to);

	// move category of completed files
	downloadlistctrl.MoveCompletedfilesCat((uint8)from,(uint8)to);

	// of the tabcontrol itself
	m_dlTab.ReorderTab(from,to);

	UpdateCatTabTitles();
	// [TPT] - khaos::categorymod+ obsolete theApp.emuledlg->searchwnd->UpdateCatTabs();

	if (to>from) --to;
	m_dlTab.SetCurSel(to);
	downloadlistctrl.ChangeCategory(to);
}

void CTransferWnd::VerifyCatTabSize() {
	CRect rect;
	int size=0;
	int right;

	for (int i=0;i<m_dlTab.GetItemCount();i++) {
		m_dlTab.GetItemRect(i,&rect);
		size+= rect.Width();
	}
	size+=20;

	WINDOWPLACEMENT wpTabWinPos;

	downloadlistctrl.GetWindowPlacement(&wpTabWinPos);
	right=wpTabWinPos.rcNormalPosition.right;

	m_dlTab.GetWindowPlacement(&wpTabWinPos);
	if (wpTabWinPos.rcNormalPosition.right<0) return;

	wpTabWinPos.rcNormalPosition.right=right;
	int left=wpTabWinPos.rcNormalPosition.right-size;
	if (left<450) left=450; // [TPT] - TBH Transfers Window Style
	wpTabWinPos.rcNormalPosition.left=left;

	RemoveAnchor(m_dlTab);
	m_dlTab.SetWindowPlacement(&wpTabWinPos);
	AddAnchor(m_dlTab,TOP_RIGHT);
}

CString CTransferWnd::GetCatTitle(int catid)
{
	switch (catid) {
		case 0 : return GetResString(IDS_ALL);
		case 1 : return GetResString(IDS_ALLOTHERS);
		case 2 : return GetResString(IDS_STATUS_NOTCOMPLETED);
		case 3 : return GetResString(IDS_DL_TRANSFCOMPL);
		case 4 : return GetResString(IDS_WAITING);
		case 5 : return GetResString(IDS_DOWNLOADING);
		case 6 : return GetResString(IDS_ERRORLIKE);
		case 7 : return GetResString(IDS_PAUSED);
		case 8 : return GetResString(IDS_SEENCOMPL);
		case 10 : return GetResString(IDS_VIDEO);
		case 11 : return GetResString(IDS_AUDIO);
		case 12 : return GetResString(IDS_SEARCH_ARC);
		case 13 : return GetResString(IDS_SEARCH_CDIMG);

		case 14 : return GetResString(IDS_SEARCH_DOC);
		case 15 : return GetResString(IDS_SEARCH_PICS);
		case 16 : return GetResString(IDS_SEARCH_PRG);
//		case 18 : return GetResString(IDS_REGEXPRESSION);
	}
	return _T("?");
}

// [TPT] - MFCK [addon] - New Tooltips [Rayita]
int CTransferWnd::GetClientImage(CUpDownClient* client)
{
	if (client->IsFriend())
		return 6;
	int iImage = 0;
	switch(client->GetClientSoft())
	{
		case SO_MLDONKEY:
			iImage = 8;
			break;
		case SO_EDONKEYHYBRID:
			iImage = 11;
			break;
		case SO_SHAREAZA:
			iImage = 12;
			break;
		case SO_EDONKEY:
			iImage = 13;
			break;
		default:
			if (client->ExtProtocolAvailable())
				iImage = 5;
			else
				iImage = 4;
	}

	return iImage;
	}

void CTransferWnd::UpdateToolTips()
{
	bool pop = true;
	for(int i = UPDOWN_WND; i >= 0; i--)
	{
		int sel = lists_list[i]->GetItemUnderMouse();
		if (sel != -1)
		{
			pop = false;
			if (sel != m_iOldToolTipItem[i])
			{
				m_iOldToolTipItem[i] = sel;
				return;
			}
		}
	}

	if (pop)
		m_ttip.Pop();
}

BOOL CTransferWnd::OnToolTipNotify(UINT id, NMHDR *pNMH, LRESULT *pResult)
{
	NM_PPTOOLTIP_DISPLAY * pNotify = (NM_PPTOOLTIP_DISPLAY*)pNMH;
	int control_id = CWnd::FromHandle(pNotify->ti->hWnd)->GetDlgCtrlID();

	if (!control_id) return FALSE;

	switch(control_id)
	{
		case IDC_DLTAB:
		{
			int index = GetTabUnderMouse(&CPoint(*pNotify->pt));
			if(index < 0) return FALSE;
			theApp.downloadqueue->GetTipInfoByCat(index, *((CString *)&pNotify->ti->sTooltip));
			return TRUE;
		}
		case IDC_DOWNLOAD_ICO:
		case IDC_UPLOAD_ICO:
		case IDC_DOWNLOADLIST:
		case IDC_DOWNLOADCLIENTS:
		case IDC_UPLOADLIST:
		case IDC_QUEUELIST:
		case IDC_CLIENTLIST:
		{
			DWORD_PTR pItem;
			try
			{
				CMuleListCtrl* list = (CMuleListCtrl*)GetDlgItem(control_id);

				if (list->GetItemCount() < 1)
					return FALSE;

				int sel = list->GetItemUnderMouse();
				if (sel < 0)
					return FALSE;

				pItem = list->GetItemData(sel);
				if (!pItem) return FALSE;
			}
			catch(...)
			{
				return FALSE;
			}

			CString info;
			switch(control_id)
			{
				case IDC_DOWNLOADLIST:
				{
					// build info text and display it
					CtrlItem_Struct* pContent = (CtrlItem_Struct*)pItem;
					if (!pContent->value) return FALSE;
					if(pContent->type == FILE_TYPE)	// for downloading files
					{
						CPartFile* partfile = (CPartFile*)pContent->value;

						partfile->GetTooltipFileInfo(info);

						SHFILEINFO shfi;
						MEMZERO(&shfi, sizeof(shfi));
						SHGetFileInfo(partfile->GetFileName(), FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(shfi), SHGFI_ICON|SHGFI_USEFILEATTRIBUTES);
						pNotify->ti->hIcon = shfi.hIcon;

						downloadlistctrl.SetFocus();
					}
					else if (pContent->type == AVAILABLE_SOURCE || pContent->type == UNAVAILABLE_SOURCE) // for sources
					{
						CUpDownClient* client = (CUpDownClient*)pContent->value;

						pNotify->ti->hIcon = m_ImageList.ExtractIcon(GetClientImage(client));

						client->GetTooltipDownloadInfo(info, pContent->type == UNAVAILABLE_SOURCE, pContent->owner);
					}

					break;
				}
				case IDC_DOWNLOADCLIENTS:
					{
						((CUpDownClient*)pItem)->GetTooltipDownloadInfo(info, false, NULL);
					}
					break;
				case IDC_UPLOADLIST:
					((CUpDownClient*)pItem)->GetTooltipUploadInfo(info);
					break;
				case IDC_QUEUELIST:
					((CUpDownClient*)pItem)->GetTooltipQueueInfo(info);
					break;
				case IDC_CLIENTLIST:
					((CUpDownClient*)pItem)->GetTooltipClientInfo(info);
					break;
			}

			if (control_id != IDC_DOWNLOADLIST)
				pNotify->ti->hIcon = m_ImageList.ExtractIcon(GetClientImage(((CUpDownClient*)pItem)));

			SetDlgItemFocus(control_id);
			pNotify->ti->sTooltip = info;

			return TRUE;
		}
		default:
			if(pNotify->ti->hIcon)
				pNotify->ti->hIcon = DuplicateIcon(AfxGetInstanceHandle(), pNotify->ti->hIcon);
			return TRUE;
	}
}

void CTransferWnd::SetDlgItemFocus(int nID)
{
	GetDlgItem(nID)->SetFocus();
}

void CTransferWnd::SetTTDelay()
{
	m_ttip.SetDelayTime(TTDT_AUTOPOP, 20000);
	m_ttip.SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay());
	m_tabtip.SetDelayTime(TTDT_AUTOPOP, 40000);
	m_tabtip.SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay()/4);
	m_othertips.SetDelayTime(TTDT_AUTOPOP, 20000);
	m_othertips.SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay()*1.5);
}
// [TPT] - MFCK [addon] - New Tooltips [Rayita]

