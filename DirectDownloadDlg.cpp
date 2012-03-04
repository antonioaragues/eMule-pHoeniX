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
#include "DirectDownloadDlg.h"
#include "OtherFunctions.h"
#include "emuleDlg.h"
#include "DownloadQueue.h"
#include "ED2KLink.h"
#include "Preferences.h"
#include "KnownFileList.h" // [TPT] - MoNKi: -Check already downloaded files-
#include "transferwnd.h" // [TPT] - Categorymod

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif


#define	PREF_INI_SECTION	_T("DirectDownloadDlg")

IMPLEMENT_DYNAMIC(CDirectDownloadDlg, CDialog)

BEGIN_MESSAGE_MAP(CDirectDownloadDlg, CResizableDialog)
	ON_EN_KILLFOCUS(IDC_ELINK, OnEnKillfocusElink)
	ON_EN_UPDATE(IDC_ELINK, OnEnUpdateElink)
END_MESSAGE_MAP()

CDirectDownloadDlg::CDirectDownloadDlg(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CDirectDownloadDlg::IDD, pParent)
{
	m_icnWnd = NULL;
}

CDirectDownloadDlg::~CDirectDownloadDlg()
{
	if (m_icnWnd)
		VERIFY( DestroyIcon(m_icnWnd) );
}

void CDirectDownloadDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DDOWN_FRM, m_ctrlDirectDlFrm);
	// [TPT] - Categorymod
	DDX_Control(pDX, IDC_PAUSECHECK, m_buttonPause);
	DDX_Control(pDX, IDC_CATS, m_box);
	// [TPT] - Categorymod
}

void CDirectDownloadDlg::UpdateControls()
{
	GetDlgItem(IDOK)->EnableWindow(GetDlgItem(IDC_ELINK)->GetWindowTextLength() > 0);
}

void CDirectDownloadDlg::OnEnUpdateElink()
{
	UpdateControls();
}

void CDirectDownloadDlg::OnEnKillfocusElink()
{
	CString strLinks;
	GetDlgItem(IDC_ELINK)->GetWindowText(strLinks);
	if (strLinks.IsEmpty() || strLinks.Find(_T('\n')) == -1)
		return;
	strLinks.Replace(_T("\n"), _T("\r\n"));
	strLinks.Replace(_T("\r\r"), _T("\r"));
	GetDlgItem(IDC_ELINK)->SetWindowText(strLinks);
}

void CDirectDownloadDlg::OnOK()
{
	// [TPT] - Categorymod
	if (m_buttonPause.GetCheck() == BST_CHECKED)
	{
		m_addPaused = true;	
	}
	// [TPT] - Categorymod

	CString strLinks;
	GetDlgItem(IDC_ELINK)->GetWindowText(strLinks);

	int curPos = 0;
	CString strTok = strLinks.Tokenize(_T("\t\n\r"), curPos);
	while (!strTok.IsEmpty())
	{
		if (strTok.Right(1) != _T("/"))
			strTok += _T("/");
		try
		{
			CED2KLink* pLink = CED2KLink::CreateLinkFromUrl(strTok.Trim());
			if (pLink)
			{
				if (pLink->GetKind() == CED2KLink::kFile)
				{
					// [TPT] - MoNKi: -Check already downloaded files-
					// [TPT] - khaos::categorymod+ Modified to support sel cat
					// pFileLink IS NOT A LEAK, DO NOT DELETE.
					CED2KFileLink* pFileLink = (CED2KFileLink*)CED2KLink::CreateLinkFromUrl(strTok.Trim());
					uint8 theCat = (thePrefs.GetCatCount() == 0) ? 0 : m_box.GetCurSel();
					pFileLink->SetCat(theCat);					
					if(theApp.knownfiles->CheckAlreadyDownloadedFileQuestion(pLink->GetFileLink()->GetHashKey(),pLink->GetFileLink()->GetName()))
					{
						theApp.downloadqueue->AddFileLinkToDownload(pFileLink, true, theCat>=0?true:false, m_addPaused);
					}					
					// [TPT] - Add sources
					else
					{
						theApp.downloadqueue->AddSources(pFileLink);
					}
					// [TPT] - Add sources
					// [TPT] - khaos::categorymod-
					// [TPT] - MoNKi: -Check already downloaded files-
				}
				else
				{
					delete pLink;
					throw CString(_T("bad link"));
				}
				delete pLink;
			}
		}
		catch(CString error)
		{
			TCHAR szBuffer[200];
			_sntprintf(szBuffer, ARRSIZE(szBuffer), GetResString(IDS_ERR_INVALIDLINK), error);
			CString strError;
			strError.Format(GetResString(IDS_ERR_LINKERROR), szBuffer);
			AfxMessageBox(strError);
			return;
		}
		strTok = strLinks.Tokenize(_T("\t\n\r"), curPos);
	}

	CResizableDialog::OnOK();
}

BOOL CDirectDownloadDlg::OnInitDialog()
{
	CResizableDialog::OnInitDialog();
	InitWindowStyles(this);
	SetIcon(m_icnWnd = theApp.LoadIcon(_T("PasteLink")), FALSE);

	AddAnchor(IDC_DDOWN_FRM, TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_ELINK, TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDCANCEL, BOTTOM_RIGHT);
	AddAnchor(IDOK, BOTTOM_RIGHT);
	AddAnchor(IDC_CATLABEL, BOTTOM_LEFT);
	AddAnchor(IDC_CATS, BOTTOM_LEFT,BOTTOM_RIGHT);
	AddAnchor(IDC_PAUSECHECK, BOTTOM_LEFT); // [TPT] - Categorymod
	EnableSaveRestore(PREF_INI_SECTION);

	SetWindowText(GetResString(IDS_SW_DIRECTDOWNLOAD));
	m_ctrlDirectDlFrm.SetWindowText(GetResString(IDS_SW_DIRECTDOWNLOAD));
	m_ctrlDirectDlFrm.SetIcon(_T("Download"));
    GetDlgItem(IDOK)->SetWindowText(GetResString(IDS_DOWNLOAD));
	GetDlgItem(IDC_FSTATIC2)->SetWindowText(GetResString(IDS_SW_LINK));
	GetDlgItem(IDC_CATLABEL)->SetWindowText(GetResString(IDS_CAT)+_T(":"));
	GetDlgItem(IDC_PAUSECHECK)->SetWindowText(GetResString(IDS_ADDPAUSED)); // [TPT] - Categorymod
	GetDlgItem(IDOK)->SetWindowText(GetResString(IDS_DOWNLOAD));
	GetDlgItem(IDCANCEL)->SetWindowText(GetResString(IDS_CANCEL));

	// [TPT] - Categorymod	
	m_addPaused = false;
	m_buttonPause.SetCheck(false);

	if (thePrefs.GetCatCount()==0) {
		GetDlgItem(IDC_CATLABEL)->EnableWindow(false);
		GetDlgItem(IDC_CATS)->EnableWindow(false);
	}
	else {
		UpdateCatTabs();
	}
	// [TPT] - Categorymod

	UpdateControls();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

// [TPT] - Categorymod
void CDirectDownloadDlg::UpdateCatTabs() 
{			
		m_box.AddString(GetResString(IDS_ALL) + _T("/") + GetResString(IDS_CAT_UNASSIGN));
		if (thePrefs.GetCatCount() > 1)
			for (int i=1; i < thePrefs.GetCatCount(); i++)
				m_box.AddString(thePrefs.GetCategory(i)->title);
	// Select the category that is currently visible in the transfer dialog as default, or 0 if they are
	// not using "New Downloads Default To Active Category"
	m_box.SetCurSel(thePrefs.UseActiveCatForLinks() ? theApp.emuledlg->transferwnd->GetActiveCategory() : 0);
}
// [TPT] - Categorymod