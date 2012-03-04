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
#pragma once

#include "ResizableLib/ResizablePage.h"
#include "Modeless.h"	// SLUGFILLER: modelessDialogs
#include "ReqFiles.h"
#include "SharedFilesPage.h"

class CUpDownClient;

///////////////////////////////////////////////////////////////////////////////
// CClientDetailPage

class CClientDetailPage : public CResizablePage
{
	DECLARE_DYNAMIC(CClientDetailPage)

public:
	CClientDetailPage();   // standard constructor
	virtual ~CClientDetailPage ();

	void SetClients(const CSimpleArray<CObject*>* paClients) { m_paClients = paClients; m_bDataChanged = true; }

	enum { IDD = IDD_SOURCEDETAILWND };

protected:
	const CSimpleArray<CObject*>* m_paClients;
	bool m_bDataChanged;

	void Localize();
	void RefreshData();

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnSetActive();

	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnDataChanged(WPARAM, LPARAM);
};


///////////////////////////////////////////////////////////////////////////////
// CClientDetailDialog

class CClientDetailDialog : public CModelessPropertySheet	// SLUGFILLER: modelessDialogs
{
	DECLARE_DYNAMIC(CClientDetailDialog)

public:
	// [TPT] - itsonlyme:clientDetails START
	CClientDetailDialog(CUpDownClient* pClient, CListCtrlItemWalk* pListCtrl = NULL, UINT uInvokePage = 0);
	CClientDetailDialog(const CSimpleArray<CUpDownClient*>* paClients, CListCtrlItemWalk* pListCtrl = NULL, UINT uInvokePage = 0);
	// [TPT] - itsonlyme:clientDetails END
	virtual ~CClientDetailDialog();

	void	UpdateTree(CString newDir)	{ m_wndSharedFiles.UpdateTree(newDir); }	// [TPT] - itsonlyme: viewSharedFiles

protected:
	CClientDetailPage m_wndClient;
	CReqFilesPage m_wndReqFiles;	// [TPT] - itsonlyme:reqFiles
	CSharedFilesPage m_wndSharedFiles;	// [TPT] - itsonlyme: viewSharedFiles
	UINT m_uInvokePage;	// [TPT] - itsonlyme:clientDetails

	void Construct();

	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
};


// [TPT] - SLUGFILLER: modelessDialogs
///////////////////////////////////////////////////////////////////////////////
// CClientDetailDialogInterface

class CClientDetailDialogInterface : public CModelessPropertySheetInterface
{
public:
	CClientDetailDialogInterface(CUpDownClient* owner);
	// itsonlyme:clientDetails START
	void	OpenDetailDialog(CListCtrlItemWalk* pListCtrl = NULL, UINT uInvokePage = 0);
	void	OpenDetailDialog(const CSimpleArray<CUpDownClient*>* paClients, CListCtrlItemWalk* pListCtrl = NULL, UINT uInvokePage = 0);
	// itsonlyme:clientDetails END
	void	UpdateTree(CString newDir);	// [TPT] - itsonlyme: viewSharedFiles

protected:
	virtual CModelessPropertySheet* CreatePropertySheet(va_list);
};
// [TPT] - SLUGFILLER: modelessDialogs
