//this file is part of eMule
// added by itsonlyme
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

// itsonlyme: viewSharedFiles

#include "StdAfx.h"
#include "emule.h"
#include "emuledlg.h"
#include "SearchList.h"
#include "KnownFile.h"
#include "PartFile.h"
#include "SharedFilesPage.h"
#include "FileDetailDialog.h"
#include "SharedFilesCtrl.h"
#include "SearchListCtrl.h"
#include "PreviewDlg.h"
#include "CommentDialog.h"
#include "IrcWnd.h"
// itsonlyme: virtualDirs
#include "InputBox.h"
#include "PreferencesDlg.h"
// itsonlyme: virtualDirs
#include "ClientList.h"
#include "SharedFileList.h"
#include "DownloadQueue.h"
#include "UpDownClient.h"
#include "MenuCmds.h"
#include "WebServices.h"
#include "UserMsgs.h"
#include "MenuXP.h"// [TPT] - New Menu Styles
#include "log.h"
#include "mod_version.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define MLC_BLEND(A, B, X) ((A + B * (X-1) + ((X+1)/2)) / X)

#define MLC_RGBBLEND(A, B, X) (                   \
	RGB(MLC_BLEND(GetRValue(A), GetRValue(B), X), \
	MLC_BLEND(GetGValue(A), GetGValue(B), X),     \
	MLC_BLEND(GetBValue(A), GetBValue(B), X))     \
)

#define		MP_SFP_REFRESH_ROOT		1
#define		MP_SFP_REFRESH_DIR		2

enum EBLCType {
	BLCT_UNKNOWN = 0,
	BLCT_ROOT = 1,
	BLCT_FILE = 2,
	BLCT_DIR = 3,
	BLCT_LOCALFILE = 4
};

struct BLCItem_struct {
	BLCItem_struct ();
	~BLCItem_struct ();
	EBLCType		m_eItemType;
	CString			m_fullPath;
	CString			m_origPath;
	CString			m_name;
	CSearchFile*	m_file;
	CKnownFile*		m_knownFile;
};

BLCItem_struct::BLCItem_struct ()
{
	m_eItemType = BLCT_UNKNOWN;
	m_file = NULL;
	m_knownFile = NULL;
}

BLCItem_struct::~BLCItem_struct ()
{
	if (m_file)
		delete m_file;
}

//////////////////////////////////////////////////////////////////////////////
// CSharedFilesPage dialog

IMPLEMENT_DYNAMIC(CSharedFilesPage, CResizablePage)

BEGIN_MESSAGE_MAP(CSharedFilesPage, CResizablePage)
	ON_WM_DESTROY()
	ON_NOTIFY(NM_DBLCLK, IDC_EXT_OPTS, OnExtOptsDblClick)
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_EXT_OPTS, OnItemExpanding)
	// [TPT] - MFCK [addon] - New Tooltips [Rayita]
	//ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT,0,0xFFFF,OnToolTipNotify)	
	ON_NOTIFY_EX_RANGE(UDM_TOOLTIP_DISPLAY, 0, 0xFFFF, OnToolTipNotify)
	// [TPT] - MFCK [addon] - New Tooltips [Rayita]	
	ON_NOTIFY(NM_RCLICK, IDC_EXT_OPTS, OnExtOptsRightClick)
	ON_MESSAGE(UM_DATA_CHANGED, OnDataChanged)
END_MESSAGE_MAP()

CSharedFilesPage::CSharedFilesPage()
	: CResizablePage(CSharedFilesPage::IDD, 0)
	, m_ctrlTree(theApp.m_iDfltImageListColorFlags)
{
	m_paClients = NULL;
	m_bDataChanged = false;
	m_strCaption = GetResString(IDS_SHAREDFILES);
	m_psp.pszTitle = m_strCaption;
	m_psp.dwFlags |= PSP_USETITLE;
	m_htiRoot = NULL;
	//m_toolTip = NULL; // [TPT] - MFCK [addon] - New Tooltips [Rayita]
	m_bLocalFiles = false;
}

CSharedFilesPage::~CSharedFilesPage()
{
}

void CSharedFilesPage::DoDataExchange(CDataExchange* pDX)
{
	CResizablePage::DoDataExchange(pDX); 
	DDX_Control(pDX, IDC_EXT_OPTS, m_ctrlTree);
}

BOOL CSharedFilesPage::OnInitDialog()
{
	CResizablePage::OnInitDialog();
	InitWindowStyles(this);

	AddAnchor(IDC_EXT_OPTS,TOP_LEFT,BOTTOM_RIGHT);
	AddAnchor(IDC_WARNING,TOP_LEFT,TOP_RIGHT);

	Localize();
	
	// [TPT] - MFCK [addon] - New Tooltips [Rayita]
	m_ctrlTree.ModifyStyle(0, TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT, 0);
	m_ttip.Create(this);
	m_ttip.AddTool(&m_ctrlTree, _T(""));	
	m_otherstips.Create(this);
	SetTTDelay();	
	/*
	m_toolTip = new CToolTipCtrl;
	m_toolTip->Create(this, TTS_ALWAYSTIP);
	m_toolTip->SetDelayTime(TTDT_AUTOPOP, 10000);
	m_toolTip->SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay()*1000); 
	m_toolTip->SendMessage(TTM_SETMAXTIPWIDTH, 0, SHRT_MAX); // recognize \n chars!
	m_toolTip->AddTool(&m_ctrlTree);
	m_ctrlTree.SetToolTips(m_toolTip);*/
	// [TPT] - MFCK [addon] - New Tooltips [Rayita]

	m_iImgRoot = 8;
	m_iImgShDir = 8;
	m_iImgDir = 8;

	CImageList* imageList = m_ctrlTree.GetImageList(TVSIL_NORMAL);
	if (imageList) {					
		m_iImgRoot = imageList->Add(CTempIconLoader(_T("SharedFiles")));			
		m_iImgShDir = imageList->Add(CTempIconLoader(_T("SharedDir")));			
		m_iImgDir = imageList->Add(CTempIconLoader(_T("FOLDERS")));
	}

	return TRUE;
}

BOOL CSharedFilesPage::OnSetActive()
{
	if (!CResizablePage::OnSetActive())
		return FALSE;

	if (m_bDataChanged)
	{
		if (m_htiRoot)
			UpdateTree(_T(""));
		else
	FillTree();
	m_ctrlTree.Expand(m_htiRoot, TVE_EXPAND);
	m_ctrlTree.SendMessage(WM_VSCROLL, SB_TOP);
		m_bDataChanged = false;
	}
	return TRUE;
}

LRESULT CSharedFilesPage::OnDataChanged(WPARAM, LPARAM)
{
	m_bDataChanged = true;
	return 1;
}

void CSharedFilesPage::Localize()
{
	CUpDownClient* client = NULL;
	if (m_paClients && m_paClients->GetSize() > 0) {
		client = STATIC_DOWNCAST(CUpDownClient, (*m_paClients)[0]);
		if (!theApp.clientlist->IsValidClient(client))
			client = NULL;
	}

	CString buffer;
	if (m_bLocalFiles)
		buffer.Format(GetResString(IDS_LOCAL_SHARED_FILES));
	else if (client) {
		if (client->GetDeniesShare())
			buffer.Format(GetResString(IDS_DENIESSHARE), client->GetUserName());
		else
			buffer.Format(GetResString(IDS_SHAREDFILES2), client->GetUserName());
	}
	else
		buffer.Format(GetResString(IDS_SHAREDFILES));
	GetDlgItem(IDC_WARNING)->SetWindowText(buffer);

	m_strCaption = GetResString(IDS_SHAREDFILES);
	m_psp.pszTitle = m_strCaption;

	if (m_htiRoot) {
		CString buffer;
		if (!m_bLocalFiles)
			buffer.Format(GetResString(IDS_USERSSHAREDFILES), client->GetUserName());
		else
			buffer.Format(GetResString(IDS_USERSSHAREDFILES), thePrefs.GetUserNick());
		m_ctrlTree.SetItemText(m_htiRoot, buffer);
	}
}

void CSharedFilesPage::OnDestroy()
{
	m_ctrlTree.DeleteAllItems();
	m_ctrlTree.DestroyWindow();
	for (POSITION pos = m_BLCItem_list.GetHeadPosition(); pos != NULL; ){
		BLCItem_struct *BLCItem = m_BLCItem_list.GetNext(pos);
		delete BLCItem;
	}
	m_BLCItem_list.RemoveAll();
	m_HTIs_map.RemoveAll();
	// [TPT] - MFCK [addon] - New Tooltips [Rayita]
	/*if (m_toolTip) {
		m_toolTip->DestroyToolTipCtrl();
		m_toolTip = NULL;
	}*/
	// [TPT] - MFCK [addon] - New Tooltips [Rayita]
	CResizablePage::OnDestroy();
}

void CSharedFilesPage::OnExtOptsDblClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (m_ctrlTree.GetSelectedItem() == NULL)
		return;
	HTREEITEM hti = m_ctrlTree.GetSelectedItem();

	BLCItem_struct *BLCItem = (BLCItem_struct *)m_ctrlTree.GetUserItemData(hti);
	ASSERT(BLCItem);

	CUpDownClient* client = NULL;
	if (m_paClients && m_paClients->GetSize() > 0) {
		client = STATIC_DOWNCAST(CUpDownClient, (*m_paClients)[0]);
		if (!theApp.clientlist->IsValidClient(client))
			client = NULL;
	}

	if (!client)
		return;	// no client

	switch (BLCItem->m_eItemType) {
	case BLCT_DIR:
		if (client->SendDirRequest(BLCItem->m_fullPath))
			m_ctrlTree.SetItemText(hti, m_ctrlTree.GetItemText(hti) + GetResString(IDS_GETTING_FILE_LIST));
		break;
	case BLCT_FILE:
		if (GetKeyState(VK_MENU) & 0x8000){
			if (BLCItem->m_file)
				BLCItem->m_file->GetDetailSheetInterface()->OpenDetailSheet();	// SLUGFILLER: modelessDialogs
	}
		else {
		if (BLCItem->m_file)
			theApp.downloadqueue->AddSearchToDownload(BLCItem->m_file);
		RECT itemRect;
		m_ctrlTree.GetItemRect(hti, &itemRect, false);
		m_ctrlTree.InvalidateRect(&itemRect, false);
	}
		break;
	}
}

void CSharedFilesPage::OnExtOptsRightClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	CPoint pt;
	::GetCursorPos(&pt);
	int menuX = pt.x, menuY = pt.y;
	HTREEITEM hItem = GetItemUnderMouse();
	if (!hItem) return;
	m_ctrlTree.Select(hItem, TVGN_CARET);
	if (m_ctrlTree.GetItemData(hItem)) {
		BLCItem_struct *BLCItem = (BLCItem_struct *) m_ctrlTree.GetUserItemData(hItem);
		if (BLCItem) {
			// [TPT] - New Menu Styles BEGIN
			CMenuXP	*pVirtualMenu = new CMenuXP;
			pVirtualMenu->CreatePopupMenu();
			pVirtualMenu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
			pVirtualMenu->SetSelectedBarColor(RGB(242,120,114));

			CMenuXP	*pPrioMenu = new CMenuXP;
			pPrioMenu->CreatePopupMenu();
			pPrioMenu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
			pPrioMenu->SetSelectedBarColor(RGB(242,120,114));
		
			CMenuXP	*pPermMenu = new CMenuXP;
			pPermMenu->CreatePopupMenu();
			pPermMenu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
			pPermMenu->SetSelectedBarColor(RGB(242,120,114));

			CMenuXP	*pVirtualDirMenu = new CMenuXP;
			pVirtualDirMenu->CreatePopupMenu();
			pVirtualDirMenu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);	
			pVirtualDirMenu->SetSelectedBarColor(RGB(242,120,114));

			CMenuXP *pWebMenu= new CMenuXP;
			pWebMenu->CreatePopupMenu();
			pWebMenu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
			pWebMenu->SetSelectedBarColor(RGB(242,120,114));
			
			CString buffer = GetResString(IDS_CMT_REFRESH);
			buffer.Remove('&');
			switch (BLCItem->m_eItemType) {
			case BLCT_ROOT:
				// itsonlyme: virtualDirs
				if (m_bLocalFiles) {					
					pVirtualMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_IOM_VIRTPREFS, GetResString(IDS_VDS_ADVANCED)));					
				}
				// itsonlyme: virtualDirs
				else {
					pVirtualMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_SFP_REFRESH_ROOT, GetResString(IDS_REFRESH_ROOT)));
				}				
				break;
			case BLCT_DIR:
				// itsonlyme: virtualDirs
				if (m_bLocalFiles) {
					pVirtualMenu->AppendODMenu(MF_STRING | !BLCItem->m_origPath.IsEmpty() ? MF_ENABLED : MF_GRAYED, new CMenuXPText(MP_IOM_VIRTDIR, GetResString(IDS_VDS_MDIR)));
					pVirtualMenu->AppendODMenu(MF_STRING | !BLCItem->m_origPath.IsEmpty() ? MF_ENABLED : MF_GRAYED, new CMenuXPText(MP_IOM_VIRTSUBDIR, GetResString(IDS_VDS_MSDIR)));
					pVirtualMenu->AppendSeparator();								
					bool bVirtRemove = false;
					CString virt;
					CString path = BLCItem->m_origPath;
					path.MakeLower();
					path.TrimRight(_T('\\'));
					bVirtRemove = thePrefs.GetDirToVDirMap()->Lookup(path, virt);
					if (!bVirtRemove)
						bVirtRemove = thePrefs.GetSubDirToVDirMap()->Lookup(path, virt);
					pVirtualMenu->AppendODMenu(MF_STRING | bVirtRemove ? MF_ENABLED : MF_GRAYED, new CMenuXPText(MP_IOM_VIRTREMOVE, GetResString(IDS_VDS_REMOVE)));
					pVirtualMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_IOM_VIRTPREFS, GetResString(IDS_VDS_ADVANCED)));
				}
				// itsonlyme: virtualDirs
				else {
					pVirtualMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_SFP_REFRESH_DIR, GetResString(IDS_REFRESH_DIR)));				
				}
			break;
		case BLCT_FILE:
			{
				pVirtualMenu->AddSideBar(new CMenuXPSideBar(17, _T("pHoeniX")));
				pVirtualMenu->SetSideBarStartColor(RGB(255,0,0));
				pVirtualMenu->SetSideBarEndColor(RGB(255,128,0));
				pVirtualMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_RESUME, GetResString(IDS_DOWNLOAD)));					
				if (thePrefs.IsExtControlsEnabled())
					pVirtualMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_RESUMEPAUSED, GetResString(IDS_DOWNLOAD) + _T(" (") + GetResString(IDS_PAUSED) + _T(")")));					
				pVirtualMenu->AppendODMenu(MF_STRING | BLCItem->m_file->IsPreviewPossible() ? MF_ENABLED : MF_GRAYED, new CMenuXPText(MP_PREVIEW, GetResString(IDS_DL_PREVIEW)));
				if (thePrefs.IsExtControlsEnabled())
					pVirtualMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_DETAIL, GetResString(IDS_SHOWDETAILS)));				
				pVirtualMenu->AppendSeparator();								
				pVirtualMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_GETED2KLINK, GetResString(IDS_DL_LINK1)));				
				pVirtualMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_GETHTMLED2KLINK, GetResString(IDS_DL_LINK2)));				
				break;
			}
			case BLCT_LOCALFILE:
				{
				CKnownFile* file = BLCItem->m_knownFile;
				// add priority switcher
				pVirtualMenu->AddSideBar(new CMenuXPSideBar(17, MOD_VERSION));
				pVirtualMenu->SetSideBarStartColor(RGB(255,0,0));
				pVirtualMenu->SetSideBarEndColor(RGB(255,128,0));					
				pPrioMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_PRIOVERYLOW, GetResString(IDS_PRIOVERYLOW)));				
				pPrioMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_PRIOLOW, GetResString(IDS_PRIOLOW)));				
				pPrioMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_PRIONORMAL, GetResString(IDS_PRIONORMAL)));				
				pPrioMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_PRIOHIGH, GetResString(IDS_PRIOHIGH)));				
				pPrioMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_PRIOVERYHIGH, GetResString(IDS_PRIORELEASE)));				
				pPrioMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_PRIOAUTO, GetResString(IDS_PRIOAUTO)));				
				
				UINT uCurPrioMenuItem = 0;
				if (file->IsAutoUpPriority())
					uCurPrioMenuItem = MP_PRIOAUTO;
				else if (file->GetUpPriority() == PR_VERYLOW)
					uCurPrioMenuItem = MP_PRIOVERYLOW;
				else if (file->GetUpPriority() == PR_LOW)
					uCurPrioMenuItem = MP_PRIOLOW;
				else if (file->GetUpPriority() == PR_NORMAL)
					uCurPrioMenuItem = MP_PRIONORMAL;
				else if (file->GetUpPriority() == PR_HIGH)
					uCurPrioMenuItem = MP_PRIOHIGH;
				else if (file->GetUpPriority() == PR_VERYHIGH)
					uCurPrioMenuItem = MP_PRIOVERYHIGH;
				else
					ASSERT(0);
				
				pPrioMenu->CheckMenuRadioItem(MP_PRIOVERYLOW, MP_PRIOAUTO, uCurPrioMenuItem, 0);
				
				// [TPT] - xMule_MOD: showSharePermissions	
				pPermMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_PERMNONE, GetResString(IDS_HIDDEN))); // xMule_MOD: showSharePermissions				
				pPermMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_PERMFRIENDS, GetResString(IDS_FSTATUS_FRIENDSONLY)));				
				pPermMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_PERMALL, GetResString(IDS_FSTATUS_PUBLIC)));				
				UINT uCurPermMenuItem = 0;
				if (file->GetPermissions() == PERM_ALL)
					uCurPermMenuItem = MP_PERMALL;
				else if (file->GetPermissions() == PERM_FRIENDS)
					uCurPermMenuItem = MP_PERMFRIENDS;
				else if (file->GetPermissions() == PERM_NOONE)
					uCurPermMenuItem = MP_PERMNONE;
				else
					ASSERT(0);
				pPermMenu->CheckMenuRadioItem(MP_PERMALL, MP_PERMNONE, uCurPermMenuItem, 0);
				// [TPT] - xMule_MOD: showSharePermissions	

				// [TPT] - itsonlyme: virtualDirs
				pVirtualDirMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_IOM_VIRTFILE, GetResString(IDS_VDS_MFILE)));				
				pVirtualDirMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_IOM_VIRTDIR, GetResString(IDS_VDS_MDIR)));				
				pVirtualDirMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_IOM_VIRTSUBDIR, GetResString(IDS_VDS_MSDIR)));				
				pVirtualDirMenu->AppendSeparator();						
				pVirtualDirMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_IOM_VIRTREMOVE, GetResString(IDS_VDS_REMOVE)));
				pVirtualDirMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_IOM_VIRTPREFS, GetResString(IDS_VDS_ADVANCED)));
				
				bool bVirtRemove = false;
				CString virt, fileID;
				fileID.Format(_T("%i:%s"), file->GetFileSize(), EncodeBase16(file->GetFileHash(),16));
				bVirtRemove = thePrefs.GetFileToVDirMap()->Lookup(fileID, virt);
				if (!bVirtRemove) {
					CString path = file->GetPath();
					path.MakeLower();
					path.TrimRight(_T('\\'));
					bVirtRemove = thePrefs.GetDirToVDirMap()->Lookup(path, virt);
					if (!bVirtRemove)
						bVirtRemove = thePrefs.GetSubDirToVDirMap()->Lookup(path, virt);
				}
				pVirtualDirMenu->EnableMenuItem(MP_IOM_VIRTREMOVE, bVirtRemove ? MF_ENABLED : MF_GRAYED);
				// [TPT] - itsonlyme: virtualDirs

				//menu.AddMenuTitle(GetResString(IDS_FILE));				
				pVirtualMenu->AppendODPopup(MF_STRING|MF_POPUP, pPermMenu, new CMenuXPText(0,GetResString(IDS_PERMISSION), theApp.LoadIcon(_T("permission"), 16, 16))); // xMule_MOD: showSharePermissions - done				
				pVirtualMenu->AppendODPopup(MF_STRING|MF_POPUP, pPrioMenu, new CMenuXPText(0,GetResString(IDS_PRIORITY), theApp.LoadIcon(_T("PRIORITY"), 16, 16)));				
				pVirtualMenu->AppendODPopup(MF_STRING|MF_POPUP, pVirtualDirMenu, new CMenuXPText(0,GetResString(IDS_VDS_VIRTDIRTITLE), theApp.LoadIcon(_T("virtualDir"), 16, 16))); // itsonlyme: virtualDirs					
				pVirtualMenu->AppendSeparator();				
				
				pVirtualMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_DETAIL, GetResString(IDS_SHOWDETAILS), theApp.LoadIcon(_T("information"), 16, 16)));				
				pVirtualMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_CMT, GetResString(IDS_CMT_ADD)));				
				pVirtualMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_VIEWFILECOMMENTS, GetResString(IDS_CMT_SHOWALL), theApp.LoadIcon(_T("comments"), 16, 16))); // SLUGFILLER: showComments								
				pVirtualMenu->AppendSeparator();				

				pVirtualMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_SHOWED2KLINK, GetResString(IDS_DL_SHOWED2KLINK), theApp.LoadIcon(_T("COPY"), 16, 16)));				
				pVirtualMenu->AppendSeparator();	

				//This menu option is is for testing..
				if(theApp.emuledlg->ircwnd->IsConnected()){
					pVirtualMenu->AppendODMenu(MF_STRING, new CMenuXPText(Irc_SetSendLink, GetResString(IDS_IRC_ADDLINKTOIRC)));						
					pVirtualMenu->AppendSeparator();	
				}
								
				pWebMenu->SetSelectedBarColor(RGB(242,120,114));
				int iWebMenuEntries = theWebServices.GetAllMenuEntries(pWebMenu);
				UINT flag2 = (iWebMenuEntries == 0 ) ? MF_GRAYED : MF_STRING;				
				pVirtualMenu->AppendODPopup(MF_STRING | flag2 | MF_POPUP, pWebMenu, new CMenuXPText(0,GetResString(IDS_WEBSERVICES)));
				
				// itsonlyme: displayOptions
				break;
			}
			}
			pVirtualMenu->TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, menuX, menuY, this);

			delete pWebMenu;
			delete pPrioMenu;
			delete pPermMenu;
			delete pVirtualDirMenu;
			delete pVirtualMenu;
			// [TPT] - New Menu Styles
		}
	}
}

// [TPT] - New Menu Styles BEGIN
void CSharedFilesPage::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	HMENU hMenu = AfxGetThreadState()->m_hTrackingMenu;
	CMenu	*pMenu = CMenu::FromHandle(hMenu);
	pMenu->MeasureItem(lpMeasureItemStruct);
	
	CWnd::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}
// [TPT] - New Menu Styles END

void CSharedFilesPage::OnItemExpanding (NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	HTREEITEM hti = pNMTreeView->itemNew.hItem;

	*pResult = 0;

	CUpDownClient* client = NULL;
	if (m_paClients && m_paClients->GetSize() > 0) {
		client = STATIC_DOWNCAST(CUpDownClient, (*m_paClients)[0]);
		if (!theApp.clientlist->IsValidClient(client))
			client = NULL;
	}

	if (!client)
		return;	// no client

	BLCItem_struct *BLCItem = (BLCItem_struct *) m_ctrlTree.GetUserItemData(hti);
	switch (BLCItem->m_eItemType) {
	case BLCT_ROOT:
		client->RequestSharedFileList();
		break;
	case BLCT_DIR:
	// send dir list request
		if (client->SendDirRequest(BLCItem->m_fullPath))
		m_ctrlTree.SetItemText(hti, m_ctrlTree.GetItemText(hti) + GetResString(IDS_GETTING_FILE_LIST));
		break;
	}
}

BOOL CSharedFilesPage::OnCommand(WPARAM wParam, LPARAM lParam)
{
	CUpDownClient* client = NULL;
	if (m_paClients && m_paClients->GetSize() > 0) {
		client = STATIC_DOWNCAST(CUpDownClient, (*m_paClients)[0]);
		if (!theApp.clientlist->IsValidClient(client))
			client = NULL;
	}

	HTREEITEM hItem = m_ctrlTree.GetSelectedItem();
	if (hItem == NULL) return false;
	if (!m_ctrlTree.GetItemData(hItem)) return false;
	BLCItem_struct *BLCItem = (BLCItem_struct *) m_ctrlTree.GetUserItemData(hItem);
	if (!BLCItem) return false;
	switch (wParam)
	{
		case MP_SFP_REFRESH_ROOT: {
			if (BLCItem->m_eItemType == BLCT_ROOT) {
				if (client) {
					client->SetDeniesShare(false);
					client->RequestSharedFileList(true);
					Localize();
					m_ctrlTree.SetItemText(hItem, m_ctrlTree.GetItemText(hItem) + GetResString(IDS_GETTING_FILE_LIST));
				}
			}
			break;
		}
		case MP_SFP_REFRESH_DIR: {
			if (BLCItem->m_eItemType == BLCT_DIR) {
				if (client) {
					m_ctrlTree.SetItemText(hItem, m_ctrlTree.GetItemText(hItem) + GetResString(IDS_GETTING_FILE_LIST));
					client->SendDirRequest(BLCItem->m_fullPath, true);
				}
			}
			break;
		}
		case MP_RESUMEPAUSED:
		case MP_RESUME:
			if (BLCItem->m_eItemType == BLCT_FILE) {
				if (BLCItem->m_file)
					theApp.downloadqueue->AddSearchToDownload(BLCItem->m_file, wParam==MP_RESUMEPAUSED);
				RECT itemRect;
				m_ctrlTree.GetItemRect(hItem, &itemRect, false);
				m_ctrlTree.InvalidateRect(&itemRect, false);
			}
			break;
		case MPG_ALTENTER:
		case MP_DETAIL:
			if (BLCItem->m_eItemType == BLCT_FILE) {
				if (BLCItem->m_file) {
					CKnownFile *file = theApp.downloadqueue->GetFileByID(BLCItem->m_file->GetFileHash());
					if (!file)
						file = theApp.sharedfiles->GetFileByID(BLCItem->m_file->GetFileHash());
					if (!file) { // search file 
						BLCItem->m_file->GetDetailSheetInterface()->OpenDetailSheet();	// SLUGFILLER: modelessDialogs
					}
					else if (file->IsPartFile()) { // download file
						CSimpleArray<CPartFile*> aFiles;
						aFiles.Add((CPartFile*)file);
						((CPartFile*)file)->GetDetailDialogInterface()->OpenDetailDialog(&aFiles);	// SLUGFILLER: modelessDialogs
					}
					else { // shared file
						CTypedPtrList<CPtrList, CKnownFile*> aFiles;
						aFiles.AddHead(file);
						file->GetDetailsSheetInterface()->OpenDetailsSheet(aFiles);	// SLUGFILLER: modelessDialogs
					}
					}
				}
			else if (BLCItem->m_eItemType == BLCT_LOCALFILE) {
				CTypedPtrList<CPtrList, CKnownFile*> aFiles;
				aFiles.AddHead(BLCItem->m_knownFile);
				BLCItem->m_knownFile->GetDetailsSheetInterface()->OpenDetailsSheet(aFiles);	// SLUGFILLER: modelessDialogs
			}
			break;
		case MP_SHOWED2KLINK:
			if (BLCItem->m_eItemType == BLCT_LOCALFILE) {
				CTypedPtrList<CPtrList, CKnownFile*> aFiles;
				aFiles.AddHead(BLCItem->m_knownFile);
				BLCItem->m_knownFile->GetDetailsSheetInterface()->OpenDetailsSheet(aFiles, IDD_ED2KLINK);	// SLUGFILLER: modelessDialogs
			}
			break;
		case Irc_SetSendLink:
			if (BLCItem->m_eItemType == BLCT_LOCALFILE)
				theApp.emuledlg->ircwnd->SetSendFileString(CreateED2kLink(BLCItem->m_knownFile));
			break;
		case MP_GETED2KLINK:
			if (BLCItem->m_eItemType == BLCT_FILE && BLCItem->m_file) {
				CString clpbrd = CreateED2kLink(BLCItem->m_file);
				theApp.CopyTextToClipboard(clpbrd);
			}
			else if (BLCItem->m_eItemType == BLCT_LOCALFILE && BLCItem->m_knownFile) {
				CString clpbrd = CreateED2kLink(BLCItem->m_knownFile);
				theApp.CopyTextToClipboard(clpbrd);
			}
			break;
		case MP_GETHTMLED2KLINK:
			if (BLCItem->m_eItemType == BLCT_FILE && BLCItem->m_file) {
				CString clpbrd = CreateHTMLED2kLink(BLCItem->m_file);
				theApp.CopyTextToClipboard(clpbrd);
			}			
			break;
		case MP_PREVIEW:
			if (BLCItem->m_eItemType == BLCT_FILE && BLCItem->m_file && client) {
				CSearchFile *file = BLCItem->m_file;
				CSearchFile *clientFile = NULL;
				for (POSITION pos = client->GetListFiles()->GetHeadPosition(); pos != NULL; ) {
					CSearchFile *item = client->GetListFiles()->GetNext(pos);
					if (!md4cmp(file->GetFileHash(), item->GetFileHash())) {
						clientFile = item;
						break;
					}
				}
				if (clientFile) {
					if (clientFile->GetPreviews().GetSize() > 0){
						// already have previews
						(new PreviewDlg())->SetFile(clientFile);
					}
					else {
						client->SendPreviewRequest(clientFile);
						AddLogLine(true, GetResString(IDS_PREVIEWREQUESTED));
					}
				}
				else
					ASSERT(false); // there should be a corresponding cached CSearchFile* in CUpDownClient*
			}
			break;
		case MP_CMT: 
			if (BLCItem->m_eItemType == BLCT_LOCALFILE) {
				CTypedPtrList<CPtrList, CKnownFile*> aFiles;
				aFiles.AddHead(BLCItem->m_knownFile);
				BLCItem->m_knownFile->GetDetailsSheetInterface()->OpenDetailsSheet(aFiles, IDD_COMMENT);	// SLUGFILLER: modelessDialogs
			}
			break; 
		// SLUGFILLER: showComments
		case MP_VIEWFILECOMMENTS:
			if (BLCItem->m_eItemType == BLCT_LOCALFILE) {
				CTypedPtrList<CPtrList, CKnownFile*> aFiles;
				aFiles.AddHead(BLCItem->m_knownFile);
				BLCItem->m_knownFile->GetDetailsSheetInterface()->OpenDetailsSheet(aFiles, IDD_COMMENTLST);	// SLUGFILLER: modelessDialogs
			}
			break;
		// SLUGFILLER: showComments
		case MP_PRIOVERYLOW:
			{
				if (BLCItem->m_eItemType == BLCT_LOCALFILE) {
					BLCItem->m_knownFile->SetAutoUpPriority(false);
					BLCItem->m_knownFile->SetUpPriority(PR_VERYLOW);
				}
				break;
			}
		case MP_PRIOLOW:
			{
				if (BLCItem->m_eItemType == BLCT_LOCALFILE) {
					BLCItem->m_knownFile->SetAutoUpPriority(false);
					BLCItem->m_knownFile->SetUpPriority(PR_LOW);
				}
				break;
			}
		case MP_PRIONORMAL:
			{
				if (BLCItem->m_eItemType == BLCT_LOCALFILE) {
					BLCItem->m_knownFile->SetAutoUpPriority(false);
					BLCItem->m_knownFile->SetUpPriority(PR_NORMAL);
				}
				break;
			}
		case MP_PRIOHIGH:
			{
				if (BLCItem->m_eItemType == BLCT_LOCALFILE) {
					BLCItem->m_knownFile->SetAutoUpPriority(false);
					BLCItem->m_knownFile->SetUpPriority(PR_HIGH);
				}
				break;
			}
		case MP_PRIOVERYHIGH:
			{
				if (BLCItem->m_eItemType == BLCT_LOCALFILE) {
					BLCItem->m_knownFile->SetAutoUpPriority(false);
					BLCItem->m_knownFile->SetUpPriority(PR_VERYHIGH);
				}
				break;
			}
		case MP_PRIOAUTO:
			{
				if (BLCItem->m_eItemType == BLCT_LOCALFILE) {
					BLCItem->m_knownFile->SetAutoUpPriority(true);
					BLCItem->m_knownFile->UpdateAutoUpPriority();
				}
				break;
			}		
		// xMule_MOD: showSharePermissions
		case MP_PERMNONE:
			{
				if (BLCItem->m_eItemType == BLCT_LOCALFILE) {
					BLCItem->m_knownFile->SetPermissions(PERM_NOONE);
				}
				break;
			}
		case MP_PERMFRIENDS:
			{
				if (BLCItem->m_eItemType == BLCT_LOCALFILE) {
					BLCItem->m_knownFile->SetPermissions(PERM_FRIENDS);
				}
				break;
			}
		case MP_PERMALL:
			{
				if (BLCItem->m_eItemType == BLCT_LOCALFILE) {
					BLCItem->m_knownFile->SetPermissions(PERM_ALL);
				}
				break;
			}
		// xMule_MOD: showSharePermissions
		// itsonlyme: virtualDirs
		case MP_IOM_VIRTFILE:
		case MP_IOM_VIRTDIR: 
		case MP_IOM_VIRTSUBDIR: {
			InputBox input;
			CString title;
			CString path, virtpath;
			CString fileID;
			if (BLCItem->m_eItemType == BLCT_LOCALFILE) {
			CKnownFile *file = BLCItem->m_knownFile;
				path = file->GetPath();
				virtpath = file->GetPath(true);
				fileID.Format(_T("%i:%s"), file->GetFileSize(), EncodeBase16(file->GetFileHash(),16));
				if (wParam == MP_IOM_VIRTFILE) {
					title.Format(GetResString(IDS_VDS_CHANGEMAP), file->GetFileName());
					input.SetLabels(title,GetResString(IDS_VDS_VIRTUALFILE),file->GetPath(true));
				}
			}
			else if (BLCItem->m_eItemType == BLCT_DIR && m_bLocalFiles && !BLCItem->m_origPath.IsEmpty()) {
				if (wParam == MP_IOM_VIRTFILE)
					break;
				path = BLCItem->m_origPath;
				virtpath = BLCItem->m_fullPath;
			}
			else
				break;
			switch (wParam) {
				case MP_IOM_VIRTDIR:
					title.Format(GetResString(IDS_VDS_CHANGEMAP), path);
					input.SetLabels(title,GetResString(IDS_VDS_VIRTUALDIR),virtpath);
					break;
				case MP_IOM_VIRTSUBDIR:
					title.Format(GetResString(IDS_VDS_CHANGEMAP), path);
					input.SetLabels(title,GetResString(IDS_VDS_VIRTUALSUBDIR),virtpath);
					break;
			}
			input.DoModal();
			CString output = input.GetInput();
			if (!input.WasCancelled() && output.GetLength()>0) {
				output.MakeLower();
				output.TrimRight(_T('\\'));
				path.MakeLower();
				path.TrimRight(_T('\\'));
				if (wParam == MP_IOM_VIRTFILE)
					thePrefs.GetFileToVDirMap()->SetAt(fileID, output);
				else if (wParam == MP_IOM_VIRTDIR)
					thePrefs.GetDirToVDirMap()->SetAt(path, output);
				else if (wParam == MP_IOM_VIRTSUBDIR)
					thePrefs.GetSubDirToVDirMap()->SetAt(path, output);
			}
			UpdateTree(_T(""));
			break;
		}
		case MP_IOM_VIRTREMOVE: {
			CString path, virt;
			if (BLCItem->m_eItemType == BLCT_LOCALFILE) {
			CKnownFile *file = BLCItem->m_knownFile;
				CString fileID;
			CString path = file->GetPath();
			fileID.Format(_T("%i:%s"), file->GetFileSize(), EncodeBase16(file->GetFileHash(),16));
			if (thePrefs.GetFileToVDirMap()->Lookup(fileID, virt))
				thePrefs.GetFileToVDirMap()->RemoveKey(fileID);
			}
			else if (BLCItem->m_eItemType == BLCT_DIR && m_bLocalFiles && !BLCItem->m_origPath.IsEmpty())
				path = BLCItem->m_origPath;
			else
				break;
			path.MakeLower();
			path.TrimRight(_T('\\'));
			if (thePrefs.GetDirToVDirMap()->Lookup(path, virt))
				thePrefs.GetDirToVDirMap()->RemoveKey(path);
			if (thePrefs.GetSubDirToVDirMap()->Lookup(path, virt))
				thePrefs.GetSubDirToVDirMap()->RemoveKey(path);
			UpdateTree(_T(""));
			break;
		}
		case MP_IOM_VIRTPREFS:
			theApp.emuledlg->ShowPreferences(IDD_PPG_VIRTUAL);
			break;
		// itsonlyme: virtualDirs

		default:
			if (wParam>=MP_WEBURL && wParam<=MP_WEBURL+256) {
				CAbstractFile* file;
				if (BLCItem->m_eItemType == BLCT_FILE)
					file = BLCItem->m_file;
				else if (BLCItem->m_eItemType == BLCT_LOCALFILE)
					file = BLCItem->m_knownFile;
				theWebServices.RunURL(file, wParam);
			}
			break;
	}
	return true;
}

BOOL CSharedFilesPage::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	LPNMHDR pNmhdr = (LPNMHDR)lParam;

	switch (pNmhdr->code)
	{
		case NM_CUSTOMDRAW:
		{
			LPNMTVCUSTOMDRAW pCustomDraw = (LPNMTVCUSTOMDRAW)lParam;
			switch (pCustomDraw->nmcd.dwDrawStage)
			{
				case CDDS_PREPAINT:
					// Need to process this case and set pResult to CDRF_NOTIFYITEMDRAW, 
					// otherwise parent will never receive CDDS_ITEMPREPAINT notification.
					*pResult = CDRF_NOTIFYITEMDRAW;
					return true;

				case CDDS_ITEMPREPAINT:
					HTREEITEM hItem = (HTREEITEM) pCustomDraw->nmcd.dwItemSpec;
					if (!m_ctrlTree.GetItemData(hItem)) return true;
					BLCItem_struct *BLCItem = (BLCItem_struct *) m_ctrlTree.GetUserItemData(hItem);
					if (BLCItem) {
						switch (BLCItem->m_eItemType) {
						case BLCT_FILE:
							{
							CKnownFile *file = theApp.downloadqueue->GetFileByID(BLCItem->m_file->GetFileHash());
							if (!file)
								file = theApp.sharedfiles->GetFileByID(BLCItem->m_file->GetFileHash());
							if (pCustomDraw->nmcd.uItemState == (CDIS_FOCUS | CDIS_SELECTED))
								pCustomDraw->clrTextBk = MLC_RGBBLEND(::GetSysColor(COLOR_HIGHLIGHT), ::GetSysColor(COLOR_WINDOW), 4);
							if (!file)
								pCustomDraw->clrText = ::GetSysColor(COLOR_WINDOWTEXT);
							else if (file->IsPartFile())
								pCustomDraw->clrText = RGB(224, 0, 0);
							else
								pCustomDraw->clrText = RGB(0, 128, 0);
							break;
						}
						case BLCT_LOCALFILE:
							if (pCustomDraw->nmcd.uItemState == (CDIS_FOCUS | CDIS_SELECTED))
								pCustomDraw->clrTextBk = MLC_RGBBLEND(::GetSysColor(COLOR_HIGHLIGHT), ::GetSysColor(COLOR_WINDOW), 4);
							if (BLCItem->m_knownFile->GetPermissions() == PERM_NOONE)
								pCustomDraw->clrText = RGB(240, 0, 0);
							else if (BLCItem->m_knownFile->GetPermissions() == PERM_FRIENDS)
								pCustomDraw->clrText = RGB(208, 128, 0);
							else if (BLCItem->m_knownFile->IsPartFile())
								pCustomDraw->clrText = RGB(0, 0, 192);
							break;
						}
					}
					*pResult = CDRF_SKIPDEFAULT;
					return false;
			}
		}
		break;
	}
	return CResizablePage::OnNotify(wParam, lParam, pResult);
}

// [TPT] - MFCK [addon] - New Tooltips [Rayita]
BOOL CSharedFilesPage::PreTranslateMessage(MSG* pMsg)
{
	m_ttip.RelayEvent(pMsg);
	m_otherstips.RelayEvent(pMsg);

	return CResizablePage::PreTranslateMessage(pMsg);
}

BOOL CSharedFilesPage::OnToolTipNotify(UINT id, NMHDR *pNMH, LRESULT *pResult)
{
	NM_PPTOOLTIP_DISPLAY * pNotify = (NM_PPTOOLTIP_DISPLAY*)pNMH;
	int control_id = CWnd::FromHandle(pNotify->ti->hWnd)->GetDlgCtrlID();
	if (!control_id)
		return FALSE;	
	CString info;

	if (control_id == IDC_EXT_OPTS)
	{
		HTREEITEM hItem = GetItemUnderMouse();
		if (hItem != NULL) 
		{
			CTreeOptionsItemData* pItemData = (CTreeOptionsItemData*) m_ctrlTree.GetItemData(hItem);
			if (pItemData) 
			{
				BLCItem_struct *BLCItem = (BLCItem_struct *) pItemData->m_dwItemData;
				if (BLCItem)
					if (BLCItem->m_eItemType == BLCT_FILE) {
						CPartFile *file = theApp.downloadqueue->GetFileByID(BLCItem->m_file->GetFileHash());
						if (file && file->IsPartFile())
							info = file->GetInfoSummary(file);
						else
							info.Format(
								GetResString(IDS_DL_FILENAME)+ _T(": %s\n") +
								GetResString(IDS_FD_HASH) + _T(" %s\n") +
								GetResString(IDS_FD_SIZE) + _T(" %s"), 
								BLCItem->m_file->GetFileName(),
								EncodeBase16(BLCItem->m_file->GetFileHash(),16), 
								CastItoXBytes(BLCItem->m_file->GetFileSize()));

						pNotify->ti->sTooltip = info;
						
						SetFocus();
						return TRUE;
					}
			}
		}
	}
	else		
		if(pNotify->ti->hIcon)
			pNotify->ti->hIcon = DuplicateIcon(AfxGetInstanceHandle(), pNotify->ti->hIcon);
	
	return TRUE;	
}

void CSharedFilesPage::SetTTDelay()
{
	m_ttip.SetDelayTime(TTDT_AUTOPOP, 20000);
	m_ttip.SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay());
	m_otherstips.SetDelayTime(TTDT_AUTOPOP, 20000);
	m_otherstips.SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay()*1.5);
}		


// [TPT] - MFCK [addon] - New Tooltips [Rayita]
/*BOOL CSharedFilesPage::PreTranslateMessage(MSG* pMsg)
{
   	if ( pMsg->message == 260 && pMsg->wParam == 13 && GetAsyncKeyState(VK_MENU)<0 ) {
		PostMessage(WM_COMMAND, MPG_ALTENTER, 0);
		return TRUE;
	}
	if (pMsg->message== WM_LBUTTONDOWN || pMsg->message== WM_LBUTTONUP || pMsg->message== WM_MOUSEMOVE) {
		m_toolTip->RelayEvent(pMsg);
	}
	if (pMsg->message == WM_MOUSEMOVE) {
		HTREEITEM hItem = GetItemUnderMouse();
		if (hItem != NULL)
		{
			if (hItem != m_htiOldToolTipItemDown)
			{
				if (m_toolTip->IsWindowVisible())
					m_toolTip->Update();
				m_htiOldToolTipItemDown = hItem;
			}
		}
	}
	return CResizablePage::PreTranslateMessage(pMsg);
}*/
// [TPT] - MFCK [addon] - New Tooltips [Rayita]

static int CALLBACK CompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CTreeOptionsItemData* pItemData1 = (CTreeOptionsItemData*) lParam1;
	BLCItem_struct *Item1 = (BLCItem_struct*)pItemData1->m_dwItemData;
	CTreeOptionsItemData* pItemData2 = (CTreeOptionsItemData*) lParam2;
	BLCItem_struct *Item2 = (BLCItem_struct*)pItemData2->m_dwItemData;

	if (Item1->m_eItemType == Item2->m_eItemType)
		return Item1->m_name.CompareNoCase(Item2->m_name);
	if (Item1->m_eItemType == BLCT_ROOT)
		return -1;
	if (Item2->m_eItemType == BLCT_ROOT)
		return 1;
	if (Item1->m_eItemType == BLCT_DIR)
		return -1;
	if (Item2->m_eItemType == BLCT_DIR)
		return 1;
	ASSERT(false);
	return 0;
}

void CSharedFilesPage::SortDirs(HTREEITEM hParent) {
	HTREEITEM hChild = m_ctrlTree.GetChildItem(hParent);
	while (hChild != NULL) {
		SortDirs(hChild);
		hChild = m_ctrlTree.GetNextSiblingItem(hChild);
	}

	TVSORTCB tvs;

	tvs.hParent = hParent;
	tvs.lpfnCompare = CompareProc;

	m_ctrlTree.SortChildrenCB(&tvs);
}

void CSharedFilesPage::FillTree()
{
	CUpDownClient* client = NULL;
	if (m_paClients && m_paClients->GetSize() > 0) {
		client = STATIC_DOWNCAST(CUpDownClient, (*m_paClients)[0]);
		if (!theApp.clientlist->IsValidClient(client))
			client = NULL;
	}

	CRBMap <CString, bool> *listDirs = NULL;
	CList <CSearchFile *> *listFiles = NULL;
	if (!m_bLocalFiles) {
		if (!client)
			return;	// no client
		listDirs = client->GetListDirs();
		listFiles = client->GetListFiles();
	}
	else {
		GetDirs();
		listDirs = &m_dirsList;
	}

	BLCItem_struct *BLCItem;

	if (!m_htiRoot) {
		CString buffer;
		BLCItem = new BLCItem_struct;
		BLCItem->m_eItemType = BLCT_ROOT;
		m_BLCItem_list.AddTail(BLCItem);
		if (!m_bLocalFiles)
			buffer.Format(GetResString(IDS_USERSSHAREDFILES), client->GetUserName());
		else
			buffer.Format(GetResString(IDS_USERSSHAREDFILES), thePrefs.GetUserNick());
		m_htiRoot = m_ctrlTree.InsertGroup(buffer, m_iImgRoot, TVI_ROOT, TVI_LAST, (DWORD)BLCItem);
		m_HTIs_map.SetAt(_T("\\"), m_htiRoot);
		TVITEM it;
		it.hItem = m_htiRoot;
		it.mask = TVIF_CHILDREN;
		it.cChildren = 1;
		m_ctrlTree.SetItem(&it);
	}

	for (POSITION pos = listDirs->GetHeadPosition(); pos != NULL; listDirs->GetNext(pos)) {
		int curPos = 0;
		HTREEITEM dirHti;
		HTREEITEM lastNode = m_htiRoot;
		CString newDir = listDirs->GetKeyAt(pos);

		CString resToken = newDir.Tokenize(_T("\\"), curPos);
		while (resToken != _T(""))
		{
			CString fullPath = newDir.Left(curPos);
			fullPath.TrimRight(_T('\\'));
			if (!m_HTIs_map.Lookup(CString(fullPath).MakeLower(), dirHti)) {
				BLCItem = new BLCItem_struct;
				BLCItem->m_eItemType = BLCT_DIR;
				BLCItem->m_fullPath = fullPath;
				BLCItem->m_name = resToken;
				m_BLCItem_list.AddTail(BLCItem);
				dirHti = m_ctrlTree.InsertGroup(resToken, m_iImgDir, lastNode, TVI_LAST, (DWORD)BLCItem);
				m_HTIs_map.SetAt(CString(fullPath).MakeLower(), dirHti);
				TVITEM it;
				it.hItem = dirHti;
				it.mask = TVIF_CHILDREN;
				it.cChildren = 1;
				m_ctrlTree.SetItem(&it);
			}
			lastNode = dirHti;
			resToken = newDir.Tokenize(_T("\\"), curPos);
		};
		if (m_HTIs_map.Lookup(CString(newDir).TrimRight(_T('\\')).MakeLower(), dirHti)) {
			m_ctrlTree.SetItemImage(dirHti, m_iImgShDir, m_iImgShDir);
			BLCItem = (BLCItem_struct *) m_ctrlTree.GetUserItemData(dirHti);
			ASSERT(BLCItem->m_eItemType == BLCT_DIR);
			BLCItem->m_fullPath = newDir;	// Must use unaltered string when requesting
		}
		else
			ASSERT(false);
	}

	if (m_bLocalFiles) {
		for (uint16 i = 0; i<theApp.sharedfiles->GetCount(); i++) {
			CKnownFile* cur_file = theApp.sharedfiles->GetFileByIndex(i);
			if (cur_file->GetPermissions() == PERM_NOONE)
				continue;
			CString path = cur_file->GetPath(true);	// itsonlyme: virtualDirs
			path.TrimRight(_T('\\'));
			path.MakeLower();
			HTREEITEM dirHti;
			if (m_HTIs_map.Lookup(path, dirHti)) {
				BLCItem = new BLCItem_struct;
				BLCItem->m_eItemType = BLCT_LOCALFILE;
				BLCItem->m_knownFile = cur_file;
				BLCItem->m_name = cur_file->GetFileName();
				m_BLCItem_list.AddTail(BLCItem);
				//[TPT] - Show the filesize beside the name
				if(BLCItem->m_eItemType == BLCT_LOCALFILE && BLCItem->m_knownFile)
				{
					CString buffer;
					buffer.Format(_T("%s (%s)"), BLCItem->m_name, CastItoXBytes(BLCItem->m_knownFile->GetFileSize(), false, false));
					HTREEITEM newHti = m_ctrlTree.InsertGroup(buffer, GetFileIcon(BLCItem->m_name), dirHti, TVI_LAST, (DWORD)BLCItem);
				}
				else
					HTREEITEM newHti = m_ctrlTree.InsertGroup(BLCItem->m_name, GetFileIcon(BLCItem->m_name), dirHti, TVI_LAST, (DWORD)BLCItem);
				// TODO: Read virtual dirs data and set, instead of using the data from the files
				BLCItem = (BLCItem_struct *) m_ctrlTree.GetUserItemData(dirHti);
				BLCItem->m_origPath = cur_file->GetPath();
			}
			else
				ASSERT (false);
		}
	}
	else {
		for (POSITION pos = listFiles->GetHeadPosition(); pos != NULL; listFiles->GetNext(pos)) {
			HTREEITEM dirHti;
			if (m_HTIs_map.Lookup(CString(listFiles->GetAt(pos)->GetDirectory()).TrimRight(_T('\\')).MakeLower(), dirHti)) {
				BLCItem = new BLCItem_struct;
				BLCItem->m_eItemType = BLCT_FILE;
				BLCItem->m_file = new CSearchFile(listFiles->GetAt(pos));
				BLCItem->m_name = BLCItem->m_file->GetFileName();
				m_BLCItem_list.AddTail(BLCItem);
				//[TPT] - Show the filesize beside the name
				if(BLCItem->m_eItemType == BLCT_FILE && BLCItem->m_file)
				{
					CString buffer;
					buffer.Format(_T("%s (%s)"), BLCItem->m_name, CastItoXBytes(BLCItem->m_file->GetFileSize(), false, false));
					HTREEITEM newHti = m_ctrlTree.InsertGroup(buffer, GetFileIcon(BLCItem->m_name), dirHti, TVI_LAST, (DWORD)BLCItem);
				}
				else
					HTREEITEM newHti = m_ctrlTree.InsertGroup(BLCItem->m_name, GetFileIcon(BLCItem->m_name), dirHti, TVI_LAST, (DWORD)BLCItem);
			}
			else
				ASSERT (false);
		}
	}
	SortDirs(m_htiRoot);
}

bool CSharedFilesPage::FilterDirs(CRBMap<CString, bool> *listDirs, HTREEITEM hParent) {
	BLCItem_struct *BLCItem = (BLCItem_struct *) m_ctrlTree.GetUserItemData(hParent);
	if (BLCItem->m_eItemType != BLCT_DIR) {
		POSITION pos = m_BLCItem_list.Find(BLCItem);
		ASSERT(pos != NULL);
		m_BLCItem_list.RemoveAt(pos);
		m_ctrlTree.DeleteItem(hParent);
		delete BLCItem;
		return false;
	}

	bool requested;
	bool hasChildren = false;
	HTREEITEM hChild = m_ctrlTree.GetChildItem(hParent);
	while (hChild != NULL) {
		HTREEITEM hNext = m_ctrlTree.GetNextSiblingItem(hChild);
		hasChildren |= FilterDirs(listDirs, hChild);
		hChild = hNext;
	}
	if (listDirs->Lookup(BLCItem->m_fullPath, requested))
		return true;
	if (hasChildren) {
		m_ctrlTree.SetItemImage(hParent, m_iImgDir, m_iImgDir);	// Not shared, but has shared subdir
		return true;
	}
	POSITION pos = m_BLCItem_list.Find(BLCItem);
	ASSERT(pos != NULL);
	m_BLCItem_list.RemoveAt(pos);
	m_ctrlTree.DeleteItem(hParent);
	m_HTIs_map.RemoveKey(CString(BLCItem->m_fullPath).MakeLower());
	delete BLCItem;
	return false;
}

void CSharedFilesPage::UpdateTree(CString newDir)
{
	CUpDownClient* client = NULL;
	if (m_paClients && m_paClients->GetSize() > 0) {
		client = STATIC_DOWNCAST(CUpDownClient, (*m_paClients)[0]);
		if (!theApp.clientlist->IsValidClient(client))
			client = NULL;
	}

	CRBMap <CString, bool> *listDirs = NULL;
	if (!m_bLocalFiles) {
		if (!client)
			return;	// no client
		listDirs = client->GetListDirs();
	}
	else {
		GetDirs();
		listDirs = &m_dirsList;
	}

	SetRedraw(false);
	HTREEITEM hChild = m_ctrlTree.GetChildItem(m_htiRoot);
	while (hChild != NULL) {
		HTREEITEM hNext = m_ctrlTree.GetNextSiblingItem(hChild);
		FilterDirs(listDirs, hChild);
		hChild = hNext;
	}
	if (!newDir.IsEmpty()) {
		HTREEITEM dirHti;
		if (m_HTIs_map.Lookup(CString(newDir).TrimRight(_T('\\')).MakeLower(), dirHti)) {
			BLCItem_struct *BLCItem = (BLCItem_struct*)m_ctrlTree.GetUserItemData(dirHti);
			m_ctrlTree.SetItemImage(dirHti, m_iImgShDir, m_iImgShDir);
			m_ctrlTree.SetItemText(dirHti, BLCItem->m_name);
			m_ctrlTree.SetItemState(dirHti, TVIS_EXPANDED, TVIS_EXPANDED);
		}
		else
			ASSERT(false);
	}
	else
		m_ctrlTree.SetItemState(m_htiRoot, TVIS_EXPANDED, TVIS_EXPANDED);
	Localize();
	FillTree();
	SetRedraw(true);
	Invalidate();
	UpdateWindow();
}

void CSharedFilesPage::GetDirs()
{
	ASSERT(m_bLocalFiles);
	CKnownFile* cur_file;
	m_dirsList.RemoveAll();
	for (uint16 i = 0; i<theApp.sharedfiles->GetCount(); i++) {
		cur_file = theApp.sharedfiles->GetFileByIndex(i);
		if (cur_file->GetPermissions() == PERM_NOONE)
			continue;
		CString path = cur_file->GetPath(true);	// itsonlyme: virtualDirs
		path.TrimRight(_T('\\'));
		m_dirsList.SetAt(path, true);
	}
}

HTREEITEM CSharedFilesPage::GetItemUnderMouse()
{
	CPoint pt;
	::GetCursorPos(&pt);
	m_ctrlTree.ScreenToClient(&pt);
	TVHITTESTINFO hit;
	hit.pt = pt;
	HTREEITEM hItem = m_ctrlTree.HitTest(&hit);
	return hItem;
}

int CSharedFilesPage::GetFileIcon(CString filename)
{
	int srcicon = theApp.GetFileTypeSystemImageIdx(filename.MakeLower());
	int dsticon;

	if (m_iconMap.Lookup(srcicon, dsticon))
		return dsticon;

	CImageList* imageList = m_ctrlTree.GetImageList(TVSIL_NORMAL);
	dsticon = imageList->Add(CImageList::FromHandle(theApp.GetSystemImageList())->ExtractIcon(srcicon));
	m_iconMap.SetAt(srcicon, dsticon);
	return dsticon;
}

// itsonlyme: viewSharedFiles
