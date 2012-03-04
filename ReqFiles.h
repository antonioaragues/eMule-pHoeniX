// itsonlyme:reqFiles START
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

#pragma once

#include "ResizableLib/ResizablePage.h"


///////////////////////////////////////////////////////////////////////////////
// CReqFilesPage

class CReqFilesPage : public CResizablePage
{ 
	DECLARE_DYNAMIC(CReqFilesPage) 

public: 
	CReqFilesPage(); 
	virtual ~CReqFilesPage(); 

	void SetClients(const CSimpleArray<CObject*>* paClients) { m_paClients = paClients; m_bDataChanged = true; }

// Dialog Data 
	enum { IDD = IDD_REQFILES }; 

protected: 
	CString m_strCaption;
	CListCtrl m_lstFiles;
	const CSimpleArray<CObject*>* m_paClients;
	bool m_bDataChanged;

	void Localize(); 
	void RefreshData();

	virtual BOOL OnInitDialog(); 
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support 
	virtual BOOL OnSetActive();

	DECLARE_MESSAGE_MAP() 
	afx_msg void OnBnClickedApply(); 
	afx_msg void OnBnClickedRefresh(); 
	afx_msg LRESULT OnDataChanged(WPARAM, LPARAM);
};

// itsonlyme:reqFiles END