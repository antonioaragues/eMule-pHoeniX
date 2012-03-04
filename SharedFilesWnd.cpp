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
#include "emuleDlg.h"
#include "SharedFilesWnd.h"
#include "OtherFunctions.h"
#include "SharedFileList.h"
#include "KnownFileList.h"
#include "KnownFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif


// CSharedFilesWnd dialog

IMPLEMENT_DYNAMIC(CSharedFilesWnd, CDialog)

BEGIN_MESSAGE_MAP(CSharedFilesWnd, CResizableDialog)
	ON_BN_CLICKED(IDC_RELOADSHAREDFILES, OnBnClickedReloadsharedfiles)
	ON_BN_CLICKED(IDC_BROWSELOCALFILES, OnBnClickedBrowseLocalFiles)	// [TPT] - itsonlyme: viewSharedFiles
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_SFLIST, OnLvnItemActivateSflist)
	ON_NOTIFY(NM_CLICK, IDC_SFLIST, OnNMClickSflist)
	// [TPT] - MoNKi: -Downloaded History-
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_DOWNHISTORYLIST, OnLvnItemActivateHistorylist)
	ON_NOTIFY(NM_CLICK, IDC_DOWNHISTORYLIST, OnNMClickHistorylist)
	ON_BN_CLICKED(IDC_BTN_CHANGEVIEW, OnBnClickedBtnChangeview)
	// [TPT] - MoNKi: -Downloaded History-
	ON_WM_SYSCOLORCHANGE()
	//ON_STN_DBLCLK(IDC_FILES_ICO, OnStnDblclickFilesIco)
END_MESSAGE_MAP()

CSharedFilesWnd::CSharedFilesWnd(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CSharedFilesWnd::IDD, pParent)
{
	icon_files = NULL;
}

CSharedFilesWnd::~CSharedFilesWnd()
{
	if (icon_files)
		VERIFY( DestroyIcon(icon_files) );
}

void CSharedFilesWnd::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SFLIST, sharedfilesctrl);
	DDX_Control(pDX, IDC_POPBAR, pop_bar);
	DDX_Control(pDX, IDC_POPBAR2, pop_baraccept);
	DDX_Control(pDX, IDC_POPBAR3, pop_bartrans);
	DDX_Control(pDX, IDC_STATISTICS, m_ctrlStatisticsFrm);
	// [TPT] - MoNKi: -Downloaded History-
	DDX_Control(pDX, IDC_DOWNHISTORYLIST, historylistctrl);
	DDX_Control(pDX, IDC_BTN_CHANGEVIEW, m_btnChangeView);
	// [TPT] - MoNKi: -Downloaded History-
}

BOOL CSharedFilesWnd::OnInitDialog()
{
	CResizableDialog::OnInitDialog();
	InitWindowStyles(this);
	SetAllIcons();
	sharedfilesctrl.Init();
	
	// [TPT] - MoNKi: -Support for High Contrast Mode-
	COLORREF clrStart, clrEnd, clrBarText, midColor;

	clrStart = ::GetSysColor(COLOR_HIGHLIGHT);
	clrEnd = ::GetHighContrastColor(::GetSysColor(COLOR_3DFACE), ::GetSysColor(COLOR_HIGHLIGHTTEXT));

	midColor = RGB((GetRValue(clrStart) + GetRValue(clrEnd))/2,
		(GetGValue(clrStart) + GetGValue(clrEnd))/2,
		(GetBValue(clrStart) + GetBValue(clrEnd))/2);

	clrBarText = ::GetHighContrastColor(midColor, ::GetSysColor(COLOR_HIGHLIGHTTEXT));

	pop_bar.SetGradientColors(clrStart,clrEnd);
	pop_bar.SetTextColor(clrBarText, ::GetSysColor(COLOR_BTNTEXT));
	pop_baraccept.SetGradientColors(clrStart,clrEnd);
	pop_baraccept.SetTextColor(clrBarText, ::GetSysColor(COLOR_BTNTEXT));
	pop_bartrans.SetGradientColors(clrStart,clrEnd);
	pop_bartrans.SetTextColor(clrBarText, ::GetSysColor(COLOR_BTNTEXT));
	// [TPT] - MoNKi: -Support for High Contrast Mode-

	LOGFONT lf;
	GetFont()->GetLogFont(&lf);
	lf.lfWeight = FW_BOLD;
	bold.CreateFontIndirect(&lf);
	m_ctrlStatisticsFrm.SetIcon(_T("StatsDetail"));
    //m_ctrlStatisticsFrm.SetFont(&bold); // should run 'SetIcon' *first* before setting bold font
    m_ctrlStatisticsFrm.SetWindowText(GetResString(IDS_SF_STATISTICS));
    
	Localize();

	GetDlgItem(IDC_CURSESSION_LBL)->SetFont(&bold);
	GetDlgItem(IDC_TOTAL_LBL)->SetFont(&bold);
	
	// [TPT] - MoNKi: -Downloaded History-
	CRect tmpRect,tmpRect2;
	
	GetDlgItem(IDC_BTN_CHANGEVIEW)->GetWindowRect(tmpRect);
	GetDlgItem(IDC_FILES_ICO)->GetWindowRect(tmpRect2);
	ScreenToClient(tmpRect);
	ScreenToClient(tmpRect2);
	tmpRect.MoveToX(tmpRect2.left);
	m_btnChangeView.MoveWindow(tmpRect,true);

	GetDlgItem(IDC_FILES_ICO)->ShowWindow(false);
	GetDlgItem(IDC_TRAFFIC_TEXT)->ShowWindow(false);

	m_btnChangeView.SetIcon(_T("Sharedfiles"));
	m_btnChangeView.SetAlign(CButtonST::ST_ALIGN_HORIZ);
	m_btnChangeView.SetFlat();
	m_btnChangeView.SetLeftAlign(true); 
	
	sharedfilesctrl.GetWindowRect(tmpRect);
	ScreenToClient(tmpRect);
	historylistctrl.MoveWindow(tmpRect,true);
	historylistctrl.ShowWindow(false);
	historylistctrl.Init();
	
	AddAnchor(IDC_BTN_CHANGEVIEW,TOP_LEFT);
	AddAnchor(IDC_DOWNHISTORYLIST,TOP_LEFT,BOTTOM_RIGHT);
	// [TPT] - MoNKi: -Downloaded History-


	AddAnchor(IDC_TRAFFIC_TEXT, TOP_LEFT);
	AddAnchor(IDC_SFLIST,TOP_LEFT,BOTTOM_RIGHT);
	AddAnchor(IDC_RELOADSHAREDFILES, BOTTOM_RIGHT);
	AddAnchor(IDC_BROWSELOCALFILES, BOTTOM_RIGHT);	// [TPT] - itsonlyme: viewSharedFiles
	AddAnchor(IDC_STATISTICS,BOTTOM_LEFT);
	AddAnchor(IDC_CURSESSION_LBL, BOTTOM_LEFT);
	AddAnchor(IDC_TOTAL_LBL, BOTTOM_LEFT);
	AddAnchor(IDC_FSTATIC4, BOTTOM_LEFT);
	AddAnchor(IDC_SREQUESTED,BOTTOM_LEFT);
	AddAnchor(IDC_POPBAR,BOTTOM_LEFT);
	AddAnchor(IDC_FSTATIC7,BOTTOM_LEFT);
	AddAnchor(IDC_SREQUESTED2,BOTTOM_LEFT);
	AddAnchor(IDC_FSTATIC5,BOTTOM_LEFT);
	AddAnchor(IDC_SACCEPTED,BOTTOM_LEFT);
	AddAnchor(IDC_POPBAR2,BOTTOM_LEFT);
	AddAnchor(IDC_FSTATIC8,BOTTOM_LEFT);
	AddAnchor(IDC_SACCEPTED2,BOTTOM_LEFT);
	AddAnchor(IDC_FSTATIC6,BOTTOM_LEFT);
	AddAnchor(IDC_STRANSFERRED,BOTTOM_LEFT);
	AddAnchor(IDC_POPBAR3,BOTTOM_LEFT);
	AddAnchor(IDC_FSTATIC9,BOTTOM_LEFT);
	AddAnchor(IDC_STRANSFERRED2,BOTTOM_LEFT);
	
	return TRUE;
}

void CSharedFilesWnd::Reload()
{
	theApp.sharedfiles->Reload();
	ShowSelectedFilesSummary();
}

//[TPT] - We use the icon as button
/*void CSharedFilesWnd::OnStnDblclickFilesIco()
{
	theApp.emuledlg->ShowPreferences(IDD_PPG_DIRECTORIES);
}*/

void CSharedFilesWnd::OnBnClickedReloadsharedfiles()
{
	Reload();
}

// [TPT] - itsonlyme: viewSharedFiles
void CSharedFilesWnd::OnBnClickedBrowseLocalFiles()
{
	theApp.sharedfiles->ShowLocalFilesDialog();
}
// [TPT] - itsonlyme: viewSharedFiles

void CSharedFilesWnd::OnLvnItemActivateSflist(NMHDR *pNMHDR, LRESULT *pResult)
{
	ShowSelectedFilesSummary();
}

//[TPT] - MoNKi: -Downloaded History-
void CSharedFilesWnd::ShowSelectedFilesSummary(bool bHistory /*=false*/)
{
	const CKnownFile* pTheFile = NULL;
	int iFiles = 0;
	uint64 uTransferred = 0;
	UINT uRequests = 0;
	UINT uAccepted = 0;
	uint64 uAllTimeTransferred = 0;
	UINT uAllTimeRequests = 0;
	UINT uAllTimeAccepted = 0;
	//[TPT] - [MoNKi: -Downloaded History-]
	/*
	POSITION pos = sharedfilesctrl.GetFirstSelectedItemPosition();
	*/
	POSITION pos;
	if(bHistory){
		pos = historylistctrl.GetFirstSelectedItemPosition();
	}
	else{
		pos = sharedfilesctrl.GetFirstSelectedItemPosition();
	}
	// [TPT] - end
	while (pos)
	{
		//[TPT] - [MoNKi: -Downloaded History-]
		/*
		int iItem = sharedfilesctrl.GetNextSelectedItem(pos);
		const CKnownFile* pFile = (CKnownFile*)sharedfilesctrl.GetItemData(iItem);
		*/
		int iItem;
		const CKnownFile* pFile;
		if(bHistory){
			iItem = historylistctrl.GetNextSelectedItem(pos);
			pFile = (CKnownFile*)historylistctrl.GetItemData(iItem);
		}
		else{
			iItem = sharedfilesctrl.GetNextSelectedItem(pos);
			pFile = (CKnownFile*)sharedfilesctrl.GetItemData(iItem);
		}
		// [TPT] end
		iFiles++;
		if (iFiles == 1)
			pTheFile = pFile;

		uTransferred += pFile->statistic.GetTransferred();
		uRequests += pFile->statistic.GetRequests();
		uAccepted += pFile->statistic.GetAccepts();

		uAllTimeTransferred += pFile->statistic.GetAllTimeTransferred();
		uAllTimeRequests += pFile->statistic.GetAllTimeRequests();
		uAllTimeAccepted += pFile->statistic.GetAllTimeAccepts();
	}

	if (iFiles != 0)
	{
		pop_bartrans.SetRange32(0, theApp.knownfiles->transferred/1024);
		pop_bartrans.SetPos(uTransferred/1024);
		pop_bartrans.SetShowPercent();			
		SetDlgItemText(IDC_STRANSFERRED, CastItoXBytes(uTransferred, false, false));

		pop_bar.SetRange32(0, theApp.knownfiles->requested);
		pop_bar.SetPos(uRequests);
		pop_bar.SetShowPercent();			
		SetDlgItemInt(IDC_SREQUESTED, uRequests, FALSE);

		pop_baraccept.SetRange32(0, theApp.knownfiles->accepted);
		pop_baraccept.SetPos(uAccepted);
		pop_baraccept.SetShowPercent();
		SetDlgItemInt(IDC_SACCEPTED, uAccepted, FALSE);

		SetDlgItemText(IDC_STRANSFERRED2, CastItoXBytes(uAllTimeTransferred, false, false));
		SetDlgItemInt(IDC_SREQUESTED2, uAllTimeRequests, FALSE);
		SetDlgItemInt(IDC_SACCEPTED2, uAllTimeAccepted, FALSE);

		CString str(GetResString(IDS_SF_STATISTICS));
		if (iFiles == 1 && pTheFile != NULL)
			str += _T(" (") + MakeStringEscaped(pTheFile->GetFileName()) +_T(")");
		m_ctrlStatisticsFrm.SetWindowText(str);
	}
	else
	{
		pop_bartrans.SetRange32(0, 100);
		pop_bartrans.SetPos(0);
		pop_bartrans.SetTextFormat(_T(""));
		SetDlgItemText(IDC_STRANSFERRED, _T("-"));

		pop_bar.SetRange32(0, 100);
		pop_bar.SetPos(0);
		pop_bar.SetTextFormat(_T(""));
		SetDlgItemText(IDC_SREQUESTED, _T("-"));

		pop_baraccept.SetRange32(0, 100);
		pop_baraccept.SetPos(0);
		pop_baraccept.SetTextFormat(_T(""));
		SetDlgItemText(IDC_SACCEPTED, _T("-"));

		SetDlgItemText(IDC_STRANSFERRED2, _T("-"));
		SetDlgItemText(IDC_SREQUESTED2, _T("-"));
		SetDlgItemText(IDC_SACCEPTED2, _T("-"));

		m_ctrlStatisticsFrm.SetWindowText(GetResString(IDS_SF_STATISTICS));
	}
}

void CSharedFilesWnd::OnNMClickSflist(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnLvnItemActivateSflist(pNMHDR,pResult);
	*pResult = 0;
}

BOOL CSharedFilesWnd::PreTranslateMessage(MSG* pMsg) 
{
   if(pMsg->message == WM_KEYUP){
	   if (pMsg->hwnd == GetDlgItem(IDC_SFLIST)->m_hWnd)
			OnLvnItemActivateSflist(0,0);
   }
   if (pMsg->message == WM_MBUTTONUP){
		POINT point;
		::GetCursorPos(&point);
		CPoint p = point; 
		sharedfilesctrl.ScreenToClient(&p); 
		int it = sharedfilesctrl.HitTest(p); 
		if (it == -1)
			return FALSE;

		sharedfilesctrl.SetItemState(-1, 0, LVIS_SELECTED);
		sharedfilesctrl.SetItemState(it, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		sharedfilesctrl.SetSelectionMark(it);   // display selection mark correctly! 
		sharedfilesctrl.ShowComments((CKnownFile*)sharedfilesctrl.GetItemData(it));
		return TRUE;
   }

   return CResizableDialog::PreTranslateMessage(pMsg);
}

void CSharedFilesWnd::OnSysColorChange()
{
	CResizableDialog::OnSysColorChange();
	SetAllIcons();
}

void CSharedFilesWnd::SetAllIcons()
{
	if (icon_files)
		VERIFY( DestroyIcon(icon_files) );
	icon_files = theApp.LoadIcon(_T("SharedFilesList"), 16, 16);
	((CStatic*)GetDlgItem(IDC_FILES_ICO))->SetIcon(icon_files);
}

void CSharedFilesWnd::Localize()
{
	sharedfilesctrl.Localize();
	historylistctrl.Localize(); // [TPT] - MoNKi: -Downloaded History-	
	GetDlgItem(IDC_TRAFFIC_TEXT)->SetWindowText(GetResString(IDS_SF_FILES));
	GetDlgItem(IDC_RELOADSHAREDFILES)->SetWindowText(GetResString(IDS_SF_RELOAD));
	GetDlgItem(IDC_BROWSELOCALFILES)->SetWindowText(GetResString(IDS_SF_BROWSELOCAL));	// [TPT] - itsonlyme: viewSharedFiles

	m_ctrlStatisticsFrm.SetWindowText(GetResString(IDS_SF_STATISTICS));
	GetDlgItem(IDC_CURSESSION_LBL)->SetWindowText(GetResString(IDS_SF_CURRENT));
	GetDlgItem(IDC_TOTAL_LBL)->SetWindowText(GetResString(IDS_SF_TOTAL));
	GetDlgItem(IDC_FSTATIC6)->SetWindowText(GetResString(IDS_SF_TRANS));
	GetDlgItem(IDC_FSTATIC5)->SetWindowText(GetResString(IDS_SF_ACCEPTED));
	GetDlgItem(IDC_FSTATIC4)->SetWindowText(GetResString(IDS_SF_REQUESTS)+_T(":"));
	GetDlgItem(IDC_FSTATIC9)->SetWindowText(GetResString(IDS_SF_TRANS));
	GetDlgItem(IDC_FSTATIC8)->SetWindowText(GetResString(IDS_SF_ACCEPTED));
	GetDlgItem(IDC_FSTATIC7)->SetWindowText(GetResString(IDS_SF_REQUESTS)+_T(":"));
	// [TPT] - MoNKi: -Downloaded History-
	if(!historylistctrl.IsWindowVisible())
		sharedfilesctrl.ShowFilesCount();
	else
		m_btnChangeView.SetWindowText(GetResString(IDS_DOWNHISTORY));
	// [TPT] - MoNKi: -Downloaded History-
}

// [TPT] - MoNKi: -Downloaded History-
void CSharedFilesWnd::OnBnClickedBtnChangeview()
{
	if(historylistctrl.IsWindowVisible()){
		sharedfilesctrl.ShowWindow(true);
		historylistctrl.ShowWindow(false);
		m_btnChangeView.SetIcon(_T("SHAREDFILES"));
		sharedfilesctrl.ShowFilesCount();
		GetDlgItem(IDC_RELOADSHAREDFILES)->ShowWindow(true);
	}
	else
	{
		historylistctrl.ShowWindow(true);
		sharedfilesctrl.ShowWindow(false);
		m_btnChangeView.SetIcon(_T("DOWNLOAD"));
		m_btnChangeView.SetWindowText(GetResString(IDS_DOWNHISTORY));
		GetDlgItem(IDC_RELOADSHAREDFILES)->ShowWindow(false);
	}
}

void CSharedFilesWnd::OnNMClickHistorylist(NMHDR *pNMHDR, LRESULT *pResult){
	OnLvnItemActivateHistorylist(pNMHDR,pResult);
	*pResult = 0;
}

void CSharedFilesWnd::OnLvnItemActivateHistorylist(NMHDR *pNMHDR, LRESULT *pResult)
{
	ShowSelectedFilesSummary(true);
}
// [TPT] - MoNKi: -Downloaded History-
