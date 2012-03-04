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
#include "resource.h"
#include "InputBox.h"
#include "OtherFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif


IMPLEMENT_DYNAMIC(InputBox, CDialog)

BEGIN_MESSAGE_MAP(InputBox, CDialog)
	ON_BN_CLICKED(IDC_CLEANFILENAME, OnCleanFilename)
END_MESSAGE_MAP()

InputBox::InputBox(CWnd* pParent /*=NULL*/)
	: CDialog(InputBox::IDD, pParent)
{
	m_cancel = true;
	m_bFilenameMode = false;
	// [TPT] - khaos::categorymod+
	isNumber = false;
	// [TPT] - khaos::categorymod-
}

InputBox::~InputBox()
{
}

void InputBox::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

void InputBox::OnOK()
{	
	m_cancel = false;
	// [TPT] - khaos::categorymod+
	if (!isNumber) 
		GetDlgItemText(IDC_TEXT, m_return);
	else 
		GetDlgItemText(IDC_TEXTNUM, m_return);
	// [TPT] - khaos::categorymod-
	CDialog::OnOK();
}

// [TPT] - khaos::categorymod+
void InputBox::OnCancel()
{
	if (isNumber) m_return = _T("-1");
	else m_return = _T("0");

	CDialog::OnCancel();
}
// [TPT] - khaos::categorymod-

void InputBox::SetLabels(CString title, CString label, CString defaultStr)
{
	m_label = label;
	m_title = title;
	m_default = defaultStr;
}

// [TPT] - khaos::categorymod+
int InputBox::GetInputInt(){
	return _tstoi(m_return);
}
// [TPT] - khaos::categorymod-

BOOL InputBox::OnInitDialog()
{
	CDialog::OnInitDialog();
	InitWindowStyles(this);

	GetDlgItem(IDC_IBLABEL)->SetWindowText(m_label);
	// [TPT] - khaos::categorymod+
	if (!isNumber)
		GetDlgItem(IDC_TEXT)->SetWindowText(m_default);
	else {
		GetDlgItem(IDC_TEXTNUM)->SetWindowText(m_default);
		GetDlgItem(IDC_TEXT)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_TEXTNUM)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_TEXTNUM)->SetFocus();
	}
	// [TPT] - khaos::categorymod-
	SetWindowText(m_title);

	GetDlgItem(IDCANCEL)->SetWindowText(GetResString(IDS_CANCEL));
	SetDlgItemText(IDC_CLEANFILENAME,GetResString(IDS_CLEANUP));
	GetDlgItem(IDC_CLEANFILENAME)->ShowWindow(m_bFilenameMode ? SW_NORMAL : SW_HIDE);
	return TRUE;
}

void InputBox::OnCleanFilename()
{
	CString filename;
	GetDlgItem(IDC_TEXT)->GetWindowText(filename);
	GetDlgItem(IDC_TEXT)->SetWindowText(CleanupFilename(filename));
}
