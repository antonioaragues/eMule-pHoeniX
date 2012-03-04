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

// itsonlyme:reqFiles START

#include "stdafx.h" 
#include "emule.h"
#include "ReqFiles.h"
#include "OtherFunctions.h"
#include "ClientList.h"
#include "DownloadQueue.h"
#include "UpDownClient.h"
#include "PartFile.h"
#include "UserMsgs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


///////////////////////////////////////////////////////////////////////////////
// CReqFilesPage

IMPLEMENT_DYNAMIC(CReqFilesPage, CResizablePage) 

BEGIN_MESSAGE_MAP(CReqFilesPage, CResizablePage) 
   ON_BN_CLICKED(IDOK, OnBnClickedApply) 
   ON_BN_CLICKED(IDC_REFRESH, OnBnClickedRefresh) 
	ON_MESSAGE(UM_DATA_CHANGED, OnDataChanged)
END_MESSAGE_MAP() 

CReqFilesPage::CReqFilesPage() 
   : CResizablePage(CReqFilesPage::IDD, 0)
{ 
	m_paClients = NULL;
	m_bDataChanged = false;
	m_strCaption = GetResString(IDS_REQUESTED_FILES);
	m_psp.pszTitle = m_strCaption;
	m_psp.dwFlags |= PSP_USETITLE;
} 

CReqFilesPage::~CReqFilesPage() 
{ 
} 

void CReqFilesPage::DoDataExchange(CDataExchange* pDX) 
{ 
	CResizablePage::DoDataExchange(pDX); 
	DDX_Control(pDX, IDC_LST, m_lstFiles);
} 

void CReqFilesPage::OnBnClickedApply() 
{ 
	CResizablePage::OnOK(); 
} 

void CReqFilesPage::OnBnClickedRefresh() 
{ 
	RefreshData();
} 

BOOL CReqFilesPage::OnInitDialog()
{ 
	CResizablePage::OnInitDialog(); 
	InitWindowStyles(this);

	AddAnchor(IDC_LST,TOP_LEFT,BOTTOM_RIGHT);
	AddAnchor(IDC_REFRESH,BOTTOM_RIGHT);
	AddAnchor(IDC_CMSTATUS,BOTTOM_LEFT);
	
	Localize();
	
	//[TPT] - Double Buffer style in lists
	if((_AfxGetComCtlVersion() >= MAKELONG(0, 6)) && thePrefs.GetDoubleBufferStyle())	
		m_lstFiles.SetExtendedStyle(LVS_EX_FULLROWSELECT | 0x00010000 /*LVS_EX_DOUBLEBUFFER*/);	
	else
		m_lstFiles.SetExtendedStyle(LVS_EX_FULLROWSELECT );	

	m_lstFiles.InsertColumn(0, GetResString(IDS_DL_FILENAME), LVCFMT_LEFT, 360, -1); 
	m_lstFiles.InsertColumn(1, GetResString(IDS_STATUS), LVCFMT_LEFT, 140, -1); 

	return TRUE; 
} 

BOOL CReqFilesPage::OnSetActive()
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

LRESULT CReqFilesPage::OnDataChanged(WPARAM, LPARAM)
{
	m_bDataChanged = true;
	return 1;
}

void CReqFilesPage::Localize(void)
{ 
	GetDlgItem(IDC_REFRESH)->SetWindowText(GetResString(IDS_CMT_REFRESH));
	CUpDownClient* client = STATIC_DOWNCAST(CUpDownClient, (*m_paClients)[0]);
	if (theApp.clientlist->IsValidClient(client))
		m_strCaption = GetResString(IDS_REQUESTED_FILES) + client->GetUserName();
	else
		m_strCaption = GetResString(IDS_REQUESTED_FILES);
	m_psp.pszTitle = m_strCaption;
} 

void CReqFilesPage::RefreshData()
{
	m_lstFiles.DeleteAllItems();

	int count=0; 
	CUpDownClient* client = STATIC_DOWNCAST(CUpDownClient, (*m_paClients)[0]);
	if (!theApp.clientlist->IsValidClient(client))
		return;

	if (theApp.downloadqueue->IsPartFile(client->GetRequestFile()))
	{
		m_lstFiles.InsertItem(count, client->GetRequestFile()->GetFileName());
		if (client->GetDownloadState() == DS_DOWNLOADING)
			m_lstFiles.SetItemText(count, 1, GetResString(IDS_DOWNLOADING));
		else
			m_lstFiles.SetItemText(count, 1, GetResString(IDS_WAITING));
		count++;
	}
	for(POSITION pos = client->m_OtherRequests_list.GetHeadPosition(); pos; client->m_OtherRequests_list.GetNext(pos))
	{
		m_lstFiles.InsertItem(count, client->m_OtherRequests_list.GetAt(pos)->GetFileName());
		m_lstFiles.SetItemText(count, 1, GetResString(IDS_ASKED4ANOTHERFILE)); 
		count++;
	}
	for(POSITION pos = client->m_OtherNoNeeded_list.GetHeadPosition(); pos; client->m_OtherNoNeeded_list.GetNext(pos))
	{
		m_lstFiles.InsertItem(count, client->m_OtherNoNeeded_list.GetAt(pos)->GetFileName());
		m_lstFiles.SetItemText(count, 1, GetResString(IDS_NONEEDEDPARTS)); 
		count++;
	}

	CString info;
	if (count==0) 
		info=GetResString(IDS_DOWNFILES_NONE);
	else
		info.Format(GetResString(IDS_DOWNFILES_COUNT), count);
	GetDlgItem(IDC_CMSTATUS)->SetWindowText(info);
}

// itsonlyme:reqFiles END
