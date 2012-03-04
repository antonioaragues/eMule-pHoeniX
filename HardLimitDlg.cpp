//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
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
#include "HardLimitDlg.h"
#include "preferences.h"
#include "otherfunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CHardLimitDlg, CDialog)
CHardLimitDlg::CHardLimitDlg()
	: CDialog(CHardLimitDlg::IDD, 0)
{
}

CHardLimitDlg::~CHardLimitDlg()
{
}

void CHardLimitDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CHardLimitDlg, CDialog)
	ON_BN_CLICKED(IDC_TAKEOVER, OnBnClickedTakeOver)
	ON_BN_CLICKED(IDC_DEFAULT_BUTTON, OnBnClickedSwitch)
	ON_BN_CLICKED(IDC_HARDLIMIT_TAKEOVER, OnBnClickedMaxSourcesPerFileTakeOver)
END_MESSAGE_MAP()

BOOL CHardLimitDlg::OnInitDialog()
{
	thePrefs.m_MaxSourcesPerFileTakeOver = false;
	m_RestoreDefault = false;
	thePrefs.m_TakeOverFileSettings = false;

	CDialog::OnInitDialog();
	LoadSettings();
	Localize();

	return TRUE;
}

void CHardLimitDlg::LoadSettings(void)
{
	if(m_hWnd)
	{
		CString strBuffer;

		strBuffer.Format(_T("%d"), thePrefs.m_MaxSourcesPerFileTemp);
		GetDlgItem(IDC_HARDLIMIT)->SetWindowText(strBuffer);
		CheckDlgButton(IDC_HARDLIMIT_TAKEOVER, thePrefs.m_MaxSourcesPerFileTakeOver);
		OnBnClickedMaxSourcesPerFileTakeOver();
	}
}

void CHardLimitDlg::OnBnClickedTakeOver()
{
	TCHAR buffer[510];

	thePrefs.m_MaxSourcesPerFileTakeOver = true;
	//Want take over always enabled
	/*IsDlgButtonChecked(IDC_HARDLIMIT_TAKEOVER);*/
	if(GetDlgItem(IDC_HARDLIMIT)->GetWindowTextLength() && thePrefs.m_MaxSourcesPerFileTakeOver)
	{
		GetDlgItem(IDC_HARDLIMIT)->GetWindowText(buffer,20);
		thePrefs.m_MaxSourcesPerFileTemp = (_tstoi(buffer)) ? _tstoi(buffer) : 1;
	}

	LoadSettings();
	thePrefs.m_TakeOverFileSettings = true;
//	CDialog::OnOK();
}

void CHardLimitDlg::OnBnClickedSwitch()
{
	if(m_RestoreDefault)
	{
		LoadSettings();
		GetDlgItem(IDC_DEFAULT_BUTTON)->SetWindowText(GetResString(IDS_DEFAULT));
		m_RestoreDefault = false;
	}
	else
	{
		SetWithDefaultValues();
		GetDlgItem(IDC_DEFAULT_BUTTON)->SetWindowText(GetResString(IDS_MAIN_POPUP_RESTORE));
		m_RestoreDefault = true;
	}
}

void CHardLimitDlg::Localize(void)
{
	if(m_hWnd)
	{
		SetWindowText(GetResString(IDS_HARDLIMIT));
		GetDlgItem(IDC_HARDLIMIT_LABEL)->SetWindowText(GetResString(IDS_PW_MAXSOURCES));
		GetDlgItem(IDC_TAKEOVER)->SetWindowText(GetResString(IDS_PW_APPLY));
		GetDlgItem(IDC_DEFAULT_BUTTON)->SetWindowText(GetResString(IDS_DEFAULT));
		GetDlgItem(IDOK)->SetWindowText(GetResString(IDS_FD_CLOSE));
	}
}

void CHardLimitDlg::SetWithDefaultValues()
{
	if(m_hWnd)
	{
		CString strBuffer;

		GetDlgItem(IDC_HARDLIMIT_LABEL)->EnableWindow(true);
		GetDlgItem(IDC_HARDLIMIT)->EnableWindow(true);
		strBuffer.Format(_T("%d"), thePrefs.GetMaxSourcePerFile());
		GetDlgItem(IDC_HARDLIMIT)->SetWindowText(strBuffer);
		CheckDlgButton(IDC_HARDLIMIT_TAKEOVER, true);

	}
}

void CHardLimitDlg::OnBnClickedMaxSourcesPerFileTakeOver()
{
	GetDlgItem(IDC_HARDLIMIT)->EnableWindow(IsDlgButtonChecked(IDC_HARDLIMIT_TAKEOVER));
	GetDlgItem(IDC_HARDLIMIT_LABEL)->EnableWindow(IsDlgButtonChecked(IDC_HARDLIMIT_TAKEOVER));
}