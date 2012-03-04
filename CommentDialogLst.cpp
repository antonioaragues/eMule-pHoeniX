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
#include "CommentDialogLst.h"
#include "PartFile.h"
#include "OtherFunctions.h"
#include "UpDownClient.h"
#include "ClientList.h"	// [TPT] - SLUGFILLER: showComments
#include "emuledlg.h"
#include "ChatWnd.h"
#include "MenuCmds.h"
#include "MenuXP.h"// [TPT] - New Menu Styles
#include "UserMsgs.h"
#include "kademlia/kademlia/kademlia.h"
#include "kademlia/kademlia/SearchManager.h"
#include "kademlia/kademlia/Entry.h"
#include "kademlia/kademlia/Search.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif


IMPLEMENT_DYNAMIC(CCommentDialogLst, CResizablePage) 

BEGIN_MESSAGE_MAP(CCommentDialogLst, CResizablePage) 
   ON_BN_CLICKED(IDOK, OnBnClickedApply) 
   ON_BN_CLICKED(IDC_REFRESH, OnBnClickedRefresh) 
   ON_BN_CLICKED(IDC_SEARCHKAD, OnBnClickedSearchKad) 
   ON_NOTIFY(NM_DBLCLK, IDC_LST, OnNMDblclkLst)
   ON_WM_CONTEXTMENU()
	ON_MESSAGE(UM_DATA_CHANGED, OnDataChanged)
   ON_WM_MEASUREITEM()// [TPT] - New Menu Styles
END_MESSAGE_MAP() 

CCommentDialogLst::CCommentDialogLst() 
   : CResizablePage(CCommentDialogLst::IDD, IDS_CMT_READALL) 
{ 
	m_paFiles = NULL;
	m_bDataChanged = false;
	m_strCaption = GetResString(IDS_CMT_READALL);
	m_psp.pszTitle = m_strCaption;
	m_psp.dwFlags |= PSP_USETITLE;
	m_paFiles = NULL; 
} 

CCommentDialogLst::~CCommentDialogLst() 
{ 
} 

void CCommentDialogLst::DoDataExchange(CDataExchange* pDX) 
{ 
	CResizablePage::DoDataExchange(pDX); 
	DDX_Control(pDX, IDC_LST, m_lstComments);
} 

void CCommentDialogLst::OnBnClickedApply() 
{ 
	CResizablePage::OnOK(); 
} 

void CCommentDialogLst::OnBnClickedRefresh() 
{ 
	RefreshData();
} 

void CCommentDialogLst::OnBnClickedSearchKad()
{
	if(Kademlia::CKademlia::isConnected())
	{
		CKnownFile* file = STATIC_DOWNCAST(CKnownFile, (*m_paFiles)[0]);	// SLUGFILLER: showComments - not just part files
		if(file)
		{
			Kademlia::CSearch *notes = new Kademlia::CSearch;
			notes->setSearchTypes(Kademlia::CSearch::NOTES);
			Kademlia::CUInt128 ID(file->GetFileHash());
			notes->setTargetID(ID);
			if( !Kademlia::CSearchManager::startSearch(notes) )
				AfxMessageBox(GetResString(IDS_KADSEARCHALREADY),MB_OK | MB_ICONINFORMATION,0);
		}
	}
}

BOOL CCommentDialogLst::OnInitDialog()
{ 
	CResizablePage::OnInitDialog(); 
	InitWindowStyles(this);

	AddAnchor(IDC_LST,TOP_LEFT,BOTTOM_RIGHT);
	AddAnchor(IDC_REFRESH,BOTTOM_RIGHT);
	AddAnchor(IDC_SEARCHKAD,BOTTOM_RIGHT);
	AddAnchor(IDC_CMSTATUS,BOTTOM_LEFT);

	//[TPT] - Double Buffer style in lists
	if((_AfxGetComCtlVersion() >= MAKELONG(0, 6)) && thePrefs.GetDoubleBufferStyle())	
		m_lstComments.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | 0x00010000 /*LVS_EX_DOUBLEBUFFER*/);	
	else
		m_lstComments.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);	
	m_lstComments.InsertColumn(0, GetResString(IDS_QL_USERNAME), LVCFMT_LEFT, 130, -1); 
	m_lstComments.InsertColumn(1, GetResString(IDS_DL_FILENAME), LVCFMT_LEFT, 130, -1); 
	m_lstComments.InsertColumn(2, GetResString(IDS_QL_RATING), LVCFMT_LEFT, 80, 1); 
	m_lstComments.InsertColumn(3, GetResString(IDS_COMMENT), LVCFMT_LEFT, 340, 1); 

	Localize(); 

	return TRUE; 
} 

BOOL CCommentDialogLst::OnSetActive()
{
	if (!CResizablePage::OnSetActive())
		return FALSE;
	if (m_bDataChanged)
	{
	RefreshData(); 
		m_bDataChanged = false;
	}
	return TRUE;
}

LRESULT CCommentDialogLst::OnDataChanged(WPARAM, LPARAM)
{
	m_bDataChanged = true;
	return 1;
} 

void CCommentDialogLst::Localize(void)
{ 
	if(m_hWnd) 
	{
		GetDlgItem(IDC_REFRESH)->SetWindowText(GetResString(IDS_CMT_REFRESH));
		GetDlgItem(IDC_SEARCHKAD)->SetWindowText(GetResString(IDS_SEARCHKAD)); 
	}
} 

// [TPT] - SLUGFILLER: showComments - read new list
void CCommentDialogLst::RefreshData()
{ 
	m_lstComments.DeleteAllItems();

	//[TPT] - Double Buffer style in lists
	//ikaTOD: it is needed?recheck before release
	if((_AfxGetComCtlVersion() >= MAKELONG(0, 6)) && thePrefs.GetDoubleBufferStyle())	
		m_lstComments.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | 0x00010000 /*LVS_EX_DOUBLEBUFFER*/);	
	else
		m_lstComments.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);	



	int count=0; 
	CKnownFile* file = STATIC_DOWNCAST(CKnownFile, (*m_paFiles)[0]);
	if (!file->GetFileComment().IsEmpty() || file->GetFileRating()) {
		m_lstComments.InsertItem(LVIF_TEXT|LVIF_PARAM,count,thePrefs.GetUserNick(),0,0,1,NULL);
		m_lstComments.SetItemText(count, 1, file->GetFileName());
		m_lstComments.SetItemText(count, 2, GetRateString(file->GetFileRating()));
		m_lstComments.SetItemText(count, 3, file->GetFileComment());
		count++;
	} 

	const CList<Comment_Struct>& commlist = file->GetCommentList();
	for (POSITION pos = commlist.GetHeadPosition(); pos != NULL;){
		Comment_Struct cur_cs =	commlist.GetNext(pos);
		if (cur_cs.m_strComment.GetLength()>0 || cur_cs.m_uRating>0) {
			m_lstComments.InsertItem(LVIF_TEXT|LVIF_PARAM,count,cur_cs.m_strUserName,0,0,1,(LPARAM)theApp.clientlist->FindClientByIP(cur_cs.m_iUserIP,cur_cs.m_iUserPort));
			m_lstComments.SetItemText(count, 1, cur_cs.m_strFileName); 
			m_lstComments.SetItemText(count, 2, GetRateString(cur_cs.m_uRating)); 
			m_lstComments.SetItemText(count, 3, cur_cs.m_strComment); 
			count++;
		} 
	} 

	const CTypedPtrList<CPtrList, Kademlia::CEntry*>& list = file->getNotes();
	for(POSITION pos = list.GetHeadPosition(); pos != NULL; )
	{
		Kademlia::CEntry* entry = list.GetNext(pos);
		m_lstComments.InsertItem(LVIF_TEXT|LVIF_PARAM,count,(LPCTSTR)"",0,0,1,NULL);
		m_lstComments.SetItemText(count, 1, entry->fileName); 
		m_lstComments.SetItemText(count, 2, GetRateString(entry->GetIntTagValue(TAG_FILERATING))); 
		m_lstComments.SetItemText(count, 3, entry->GetStrTagValue(TAG_DESCRIPTION));
		count++;
	}

	CString info;
	if (count==0)
		info=_T("(") + GetResString(IDS_CMT_NONE) + _T(")");
	GetDlgItem(IDC_CMSTATUS)->SetWindowText(info);
}
// [TPT] - SLUGFILLER: showComments

void CCommentDialogLst::OnNMDblclkLst(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (m_lstComments.GetSelectedCount()==0)
		return;

	CUpDownClient* client = (CUpDownClient*)m_lstComments.GetItemData(m_lstComments.GetSelectionMark());
	if (client)
		theApp.emuledlg->chatwnd->StartSession(client);
	theApp.emuledlg->SetActiveDialog(theApp.emuledlg->chatwnd);
	*pResult = 0;
}

// [TPT] - New Menu Styles BEGIN
void CCommentDialogLst::OnContextMenu(CWnd* pWnd, CPoint point){
	UINT flag = MF_STRING;
	if (m_lstComments.GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED) == -1)
		flag = MF_GRAYED;
	
	CMenuXP	*pMenu = new CMenuXP;
	pMenu->CreatePopupMenu();
	pMenu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
	pMenu->SetSelectedBarColor(RGB(242,120,114));
	pMenu->AppendODMenu(MF_STRING | flag, new CMenuXPText(MP_MESSAGE, GetResString(IDS_CMT_COPYCLIPBOARD), theApp.LoadIcon(_T("copy"), 16, 16)));
	
	pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	delete pMenu;
}
// NoamSon: CopyComments-
// [TPT] - New Menu Styles END

// [TPT] - New Menu Styles BEGIN
void CCommentDialogLst::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	HMENU hMenu = AfxGetThreadState()->m_hTrackingMenu;
	CMenu	*pMenu = CMenu::FromHandle(hMenu);
	pMenu->MeasureItem(lpMeasureItemStruct);
	
	CWnd::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}
// [TPT] - New Menu Styles END

// NoamSon: CopyComments+
BOOL CCommentDialogLst::OnCommand(WPARAM wParam,LPARAM lParam ){
	int iSel = m_lstComments.GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1)
	{
		switch (wParam)
		{
			case MP_MESSAGE:
			theApp.CopyTextToClipboard(m_lstComments.GetItemText(iSel, 3));
			return TRUE;
		}
	}
	return CResizablePage::OnCommand(wParam, lParam);
}
