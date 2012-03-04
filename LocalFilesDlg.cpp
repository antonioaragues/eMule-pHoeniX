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

// itsonlyme: viewSharedFiles

// LocalFilesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "emule.h"
#include "otherfunctions.h"
#include "LocalFilesDlg.h"
#include "SharedFileList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//////////////////////////////////////////////////////////////////////////////
// CLocalFilesDialog

IMPLEMENT_DYNAMIC(CLocalFilesDialog, CModelessPropertySheet)	// SLUGFILLER: modelessDialogs

BEGIN_MESSAGE_MAP(CLocalFilesDialog, CModelessPropertySheet)	// SLUGFILLER: modelessDialogs
	ON_WM_DESTROY()
END_MESSAGE_MAP()

CLocalFilesDialog::CLocalFilesDialog()
	: CModelessPropertySheet(NULL)	// [TPT] - SLUGFILLER: modelessDialogs
{
	m_psh.dwFlags &= ~PSH_HASHELP;
	m_psh.dwFlags |= PSH_NOAPPLYNOW;

	m_wndSharedFiles.m_psp.dwFlags &= ~PSP_HASHELP;
	m_wndSharedFiles.m_psp.dwFlags |= PSP_USEICONID;
	m_wndSharedFiles.m_psp.pszIcon = _T("SharedFilesList");
	m_wndSharedFiles.SetLocalFiles();
	AddPage(&m_wndSharedFiles);
}

CLocalFilesDialog::~CLocalFilesDialog()
{
	theApp.sharedfiles->LocalFilesDialogClosed();	// [TPT] - SLUGFILLER: modelessDialogs
}

void CLocalFilesDialog::OnDestroy()
{
	CListViewWalkerPropertySheet::OnDestroy();
}

BOOL CLocalFilesDialog::OnInitDialog()
{
	EnableStackedTabs(FALSE);
	BOOL bResult = CModelessPropertySheet::OnInitDialog();	// [TPT] - SLUGFILLER: modelessDialogs
	InitWindowStyles(this);
	EnableSaveRestore(_T("LocalFilesDialog")); // call this after(!) OnInitDialog
	SetWindowText(GetResString(IDS_LOCAL_SHARED_FILES));

	return bResult;
}

// itsonlyme: virtualDirs
void CLocalFilesDialog::UpdateAll()
{
	m_wndSharedFiles.UpdateTree(_T(""));
}
// itsonlyme: virtualDirs

// itsonlyme: viewSharedFiles
