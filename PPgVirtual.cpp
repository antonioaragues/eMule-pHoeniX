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

// [TPT] - itsonlyme: virtualDirs

#include "stdafx.h"
#include "emule.h"
#include "emuledlg.h"
#include "SharedFilesWnd.h"
#include "PPgVirtual.h"
#include "otherfunctions.h"
#include "MenuCmds.h"
#include "InputBox.h"
#include "SharedFileList.h"
#include "KnownFileList.h"
#include "KnownFile.h"
#include "mod_version.h" // [TPT]

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


struct VirtMapStruct;

IMPLEMENT_DYNAMIC(CPPgVirtual, CPropertyPage)
CPPgVirtual::CPPgVirtual()
	: CPropertyPage(CPPgVirtual::IDD)
{
}

CPPgVirtual::~CPPgVirtual()
{
	for (POSITION pos = structList.GetHeadPosition(); pos != NULL; )
		delete structList.GetNext(pos);
}

void CPPgVirtual::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
}


BEGIN_MESSAGE_MAP(CPPgVirtual, CPropertyPage)
	ON_NOTIFY(NM_CLICK, IDC_LIST, OnNMClkList)
	ON_BN_CLICKED(IDC_REMOVE, OnBnClickedRemove)
	ON_BN_CLICKED(IDC_NEW, OnBnClickedAdd)
	ON_BN_CLICKED(IDC_APPLY, OnBnClickedApply)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, OnNMRightClkList)
	ON_WM_MEASUREITEM()// [TPT] - New Menu Styles
	ON_WM_HELPINFO()
END_MESSAGE_MAP()

BOOL CPPgVirtual::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_list.ModifyStyle(LVS_SINGLESEL,0);
	m_list.InsertColumn(0,GetResString(IDS_VDS_MMTYPE),LVCFMT_LEFT,80,0);
	m_list.InsertColumn(1,GetResString(IDS_VDS_MMORIG),LVCFMT_LEFT,300,1);
	m_list.InsertColumn(2,GetResString(IDS_VDS_MMVIRT),LVCFMT_LEFT,300,2);

	Localize();
	FillList();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CPPgVirtual::FillList()
{
	int i = 0;

	m_list.DeleteAllItems();
	for (POSITION pos = thePrefs.GetFileToVDirMap()->GetHeadPosition(); pos != NULL; ) {
		VirtMapStruct *str = new VirtMapStruct;
		uchar hash[16];
		CString fileID;
		thePrefs.GetFileToVDirMap()->GetNextAssoc(pos, str->fileID, str->mapTo);
		fileID = str->fileID.Right(str->fileID.GetLength()-str->fileID.Find(_T(":"))-1);
		DecodeBase16(fileID.GetBuffer(), fileID.GetLength(), hash, ARRSIZE(hash));
		CKnownFile *file = theApp.knownfiles->FindKnownFileByID(hash);
		if (file == NULL) continue;
		str->mapFrom = file->GetFilePath();
		str->type = IOM_VST_FILE;
		structList.AddTail(str);
		m_list.InsertItem(i, GetResString(IDS_VDS_FILEMAP));
		m_list.SetItemText(i, 1, str->mapFrom);
		m_list.SetItemText(i, 2, str->mapTo);
		m_list.SetItemData(i++, (DWORD_PTR)str);
	}
	for (POSITION pos = thePrefs.GetDirToVDirMap()->GetHeadPosition(); pos != NULL; ) {
		VirtMapStruct *str = new VirtMapStruct;
		thePrefs.GetDirToVDirMap()->GetNextAssoc(pos, str->mapFrom, str->mapTo);
		str->type = IOM_VST_DIR;
		structList.AddTail(str);
		m_list.InsertItem(i, GetResString(IDS_VDS_DIRMAP));
		m_list.SetItemText(i, 1, str->mapFrom);
		m_list.SetItemText(i, 2, str->mapTo);
		m_list.SetItemData(i++, (DWORD_PTR)str);
	}
	for (POSITION pos = thePrefs.GetSubDirToVDirMap()->GetHeadPosition(); pos != NULL; ) {
		VirtMapStruct *str = new VirtMapStruct;
		thePrefs.GetSubDirToVDirMap()->GetNextAssoc(pos, str->mapFrom, str->mapTo);
		str->type = IOM_VST_SUBDIR;
		structList.AddTail(str);
		m_list.InsertItem(i, GetResString(IDS_VDS_SUBDIRMAP));
		m_list.SetItemText(i, 1, str->mapFrom);
		m_list.SetItemText(i, 2, str->mapTo);
		m_list.SetItemData(i++, (DWORD_PTR)str);
	}
	theApp.emuledlg->sharedfileswnd->Invalidate();
	theApp.emuledlg->sharedfileswnd->UpdateWindow();
	if (theApp.sharedfiles) 
		theApp.sharedfiles->ShowLocalFilesDialog(true);
}

void CPPgVirtual::Localize(void)
{
	if(m_hWnd)
	{
		SetWindowText(GetResString(IDS_VDS_MM));
		GetDlgItem(IDC_REMOVE)->SetWindowText(GetResString(IDS_REMOVE));
		GetDlgItem(IDC_NEW)->SetWindowText(GetResString(IDS_NEW));
		GetDlgItem(IDC_APPLY)->SetWindowText(GetResString(IDS_PW_APPLY));
		GetDlgItem(IDC_STATIC_DETAILS)->SetWindowText(GetResString(IDS_DETAILS));
		GetDlgItem(IDC_LAB_ORIG)->SetWindowText(GetResString(IDS_VDS_MMORIG));
		GetDlgItem(IDC_LAB_VIRTUAL)->SetWindowText(GetResString(IDS_VDS_MMVIRT));

		CHeaderCtrl* pHeaderCtrl = m_list.GetHeaderCtrl();
		HDITEM hdi;
		hdi.mask = HDI_TEXT;
		CString strRes;
		strRes = GetResString(IDS_VDS_MMTYPE);
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(0, &hdi);
		strRes.ReleaseBuffer();
		strRes = GetResString(IDS_VDS_MMORIG);
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(1, &hdi);
		strRes.ReleaseBuffer();
		strRes = GetResString(IDS_VDS_MMVIRT);
		hdi.pszText = strRes.GetBuffer();
		pHeaderCtrl->SetItem(2, &hdi);
		strRes.ReleaseBuffer();
	}
}

void CPPgVirtual::OnNMClkList(NMHDR *pNMHDR, LRESULT *pResult)
{
	int index = m_list.GetSelectionMark();
	if (index != -1 && m_list.GetSelectedCount() == 1) {
		VirtMapStruct *str = (VirtMapStruct *) m_list.GetItemData(index);
		GetDlgItem(IDC_EDIT_ORIG)->SendMessage(EM_SETREADONLY, str->type == IOM_VST_FILE, 0);
		GetDlgItem(IDC_EDIT_ORIG)->SetWindowText(str->mapFrom);
		GetDlgItem(IDC_EDIT_VIRTUAL)->SetWindowText(str->mapTo);
	}
	else {
		GetDlgItem(IDC_EDIT_ORIG)->SendMessage(EM_SETREADONLY, false, 0);
		GetDlgItem(IDC_EDIT_ORIG)->SetWindowText(_T(""));
		GetDlgItem(IDC_EDIT_VIRTUAL)->SetWindowText(_T(""));
	}
}

void CPPgVirtual::OnBnClickedAdd()
{
	int index;
	VirtMapStruct *str = new VirtMapStruct;
	str->type = IOM_VST_NEW;
	structList.AddTail(str);
	index = m_list.InsertItem(m_list.GetItemCount(), GetResString(IDS_NEW));
	m_list.SetItemText(index, 1, _T("?"));
	m_list.SetItemText(index, 2, _T("?"));
	m_list.SetItemData(index, (DWORD_PTR)str);
}

void CPPgVirtual::OnBnClickedApply()
{
	int index = m_list.GetSelectionMark();
	if (index != -1 && m_list.GetSelectedCount() == 1) {
		VirtMapStruct *str = (VirtMapStruct *)m_list.GetItemData(index);
		CString origTxt, virtTxt;
		GetDlgItem(IDC_EDIT_ORIG)->GetWindowText(origTxt);
		GetDlgItem(IDC_EDIT_VIRTUAL)->GetWindowText(virtTxt);
		origTxt.MakeLower();
		origTxt.TrimRight(_T("\\"));
		virtTxt.MakeLower();
		virtTxt.TrimRight(_T("\\"));
		if (str->type == IOM_VST_FILE && origTxt == str->mapFrom && !virtTxt.IsEmpty())
			thePrefs.GetFileToVDirMap()->SetAt(str->fileID, virtTxt);
		else if (str->type == IOM_VST_DIR && !origTxt.IsEmpty() && !virtTxt.IsEmpty()) {
			thePrefs.GetDirToVDirMap()->RemoveKey(str->mapFrom);
			thePrefs.GetDirToVDirMap()->SetAt(origTxt, virtTxt);
		}
		else if ((str->type == IOM_VST_SUBDIR || str->type == IOM_VST_NEW) && !origTxt.IsEmpty() && !virtTxt.IsEmpty()) {
			thePrefs.GetSubDirToVDirMap()->RemoveKey(str->mapFrom);
			thePrefs.GetSubDirToVDirMap()->SetAt(origTxt, virtTxt);
		}
		FillList();
	}
}

void CPPgVirtual::OnBnClickedRemove()
{
	int index = m_list.GetSelectionMark();
	if (index != -1 && m_list.GetSelectedCount() == 1) {
		VirtMapStruct *str = (VirtMapStruct *)m_list.GetItemData(index);
		if (str->type == IOM_VST_FILE)
			thePrefs.GetFileToVDirMap()->RemoveKey(str->fileID);
		else if (str->type == IOM_VST_DIR)
			thePrefs.GetDirToVDirMap()->RemoveKey(str->mapFrom);
		else if (str->type == IOM_VST_SUBDIR)
			thePrefs.GetSubDirToVDirMap()->RemoveKey(str->mapFrom);
		theApp.emuledlg->sharedfileswnd->Invalidate();
		theApp.emuledlg->sharedfileswnd->UpdateWindow();
		if (theApp.sharedfiles) 
			theApp.sharedfiles->ShowLocalFilesDialog(true);
		m_list.DeleteItem(index);
	}
}

BOOL CPPgVirtual::OnApply(){
	SetModified(FALSE);
	return CPropertyPage::OnApply();
}

void CPPgVirtual::OnNMRightClkList(NMHDR *pNMHDR, LRESULT *pResult)
{
	int index = m_list.GetSelectionMark();

	POINT point;
	::GetCursorPos(&point);

	// [TPT] - New Menu Styles BEGIN
	//Menu Configuration
	CMenuXP	*pMenu = new CMenuXP;
	pMenu->CreatePopupMenu();
	pMenu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
	pMenu->AddSideBar(new CMenuXPSideBar(17, MOD_VERSION));
	pMenu->SetSideBarStartColor(RGB(255,0,0));
	pMenu->SetSideBarEndColor(RGB(255,128,0));
	pMenu->SetSelectedBarColor(RGB(242,120,114));

	pMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_IOM_NEW_ENTRY, GetResString(IDS_VDS_ADDMAP)));			
	pMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_IOM_REMOVE, GetResString(IDS_VDS_REMOVEMAP)));			
	pMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_IOM_COPY, GetResString(IDS_VDS_COPYMAP)));			
	pMenu->AppendSeparator();
	pMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_IOM_SET_DIR, GetResString(IDS_VDS_CHANGEDIR)));			
	pMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_IOM_SET_SUBDIR, GetResString(IDS_VDS_CHANGESUBDIR)));		
	
	if (index == -1 || m_list.GetSelectedCount() == 0) {
		pMenu->EnableMenuItem(MP_IOM_REMOVE, MF_BYCOMMAND|MF_GRAYED);
		pMenu->EnableMenuItem(MP_IOM_COPY, MF_BYCOMMAND|MF_GRAYED);
		pMenu->EnableMenuItem(MP_IOM_SET_DIR, MF_BYCOMMAND|MF_GRAYED);
		pMenu->EnableMenuItem(MP_IOM_SET_SUBDIR, MF_BYCOMMAND|MF_GRAYED);
	} else {
		VirtMapStruct *str = (VirtMapStruct *)m_list.GetItemData(index);
		if (str->type == IOM_VST_FILE) {
			pMenu->EnableMenuItem(MP_IOM_SET_DIR, MF_BYCOMMAND|MF_GRAYED);
			pMenu->EnableMenuItem(MP_IOM_SET_SUBDIR, MF_BYCOMMAND|MF_GRAYED);
			pMenu->EnableMenuItem(MP_IOM_COPY, MF_BYCOMMAND|MF_GRAYED);
		} else if (str->type == IOM_VST_DIR) {
			pMenu->CheckMenuItem(MP_IOM_SET_DIR, MF_BYCOMMAND|MF_CHECKED);
		} else if (str->type == IOM_VST_SUBDIR) {
			pMenu->CheckMenuItem(MP_IOM_SET_SUBDIR, MF_BYCOMMAND|MF_CHECKED);
		}
	}

	pMenu->TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);

	delete pMenu;
	// [TPT] - New Menu Styles END

	*pResult = 0;
}

// [TPT] - New Menu Styles BEGIN
void CPPgVirtual::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	HMENU hMenu = AfxGetThreadState()->m_hTrackingMenu;
	CMenu	*pMenu = CMenu::FromHandle(hMenu);
	pMenu->MeasureItem(lpMeasureItemStruct);
	
	CWnd::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}
// [TPT] - New Menu Styles END


BOOL CPPgVirtual::OnCommand(WPARAM wParam,LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}

	int item = m_list.GetSelectionMark();
	bool selected = (item != -1 && m_list.GetSelectedCount() == 1);
	
	switch (wParam) {
		case MP_IOM_NEW_ENTRY: {
			OnBnClickedAdd();
			break;
		}
		case MP_IOM_REMOVE: {
			if (selected)
				OnBnClickedRemove();
			break;
		}
		case MP_IOM_COPY: {
			if (selected) {
				VirtMapStruct *str = (VirtMapStruct *)m_list.GetItemData(item);
				VirtMapStruct *strNew = new VirtMapStruct;
				strNew->mapFrom = str->mapFrom;
				strNew->mapTo = str->mapTo;
				strNew->type = IOM_VST_NEW;
				structList.AddTail(strNew);
				item = m_list.InsertItem(m_list.GetItemCount(), GetResString(IDS_NEW));
				m_list.SetItemText(item, 1, strNew->mapFrom);
				m_list.SetItemText(item, 2, strNew->mapTo);
				m_list.SetItemData(item, (DWORD_PTR)strNew);
			}
			break;
		}
		case MP_IOM_SET_DIR: {
			if (selected) {
				VirtMapStruct *str = (VirtMapStruct *)m_list.GetItemData(item);
				thePrefs.GetSubDirToVDirMap()->RemoveKey(str->mapFrom);
				thePrefs.GetDirToVDirMap()->SetAt(str->mapFrom, str->mapTo);
				str->type = IOM_VST_DIR;
				FillList();
			}
			break;
		}
		case MP_IOM_SET_SUBDIR: {
			if (selected) {
				VirtMapStruct *str = (VirtMapStruct *)m_list.GetItemData(item);
				thePrefs.GetDirToVDirMap()->RemoveKey(str->mapFrom);
				thePrefs.GetSubDirToVDirMap()->SetAt(str->mapFrom, str->mapTo);
				str->type = IOM_VST_SUBDIR;
				FillList();
			}
			break;
		}
	}

	return CPropertyPage::OnCommand(wParam, lParam);
}

void CPPgVirtual::OnHelp()
{
	//theApp.ShowHelp(0);
}

BOOL CPPgVirtual::OnHelpInfo(HELPINFO* pHelpInfo)
{
	OnHelp();
	return TRUE;
}

// itsonlyme: virtualDirs