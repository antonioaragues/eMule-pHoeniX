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
#include "PreferencesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif


IMPLEMENT_DYNAMIC(CPreferencesDlg, CModelessTreePropSheet)	// SLUGFILLER: modelessDialogs

BEGIN_MESSAGE_MAP(CPreferencesDlg, CModelessTreePropSheet)	// SLUGFILLER: modelessDialogs
	ON_WM_DESTROY()
	ON_WM_HELPINFO()
END_MESSAGE_MAP()

CPreferencesDlg::CPreferencesDlg()
{
	m_psh.dwFlags &= ~PSH_HASHELP;
	m_wndGeneral.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndDisplay.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndConnection.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndServer.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndDirectories.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndFiles.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndStats.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndIRC.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndWebServer.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndTweaks.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndSecurity.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndScheduler.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndNotify.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndProxy.m_psp.dwFlags &= ~PSH_HASHELP; // deadlake PROXYSUPPORT		
	// [TPT]
	m_wndVirtual.m_psp.dwFlags &= ~PSH_HASHELP;	// [TPT] - itsonlyme: virtualDirs
	m_wndPhoenix1.m_psp.dwFlags &= ~PSH_HASHELP; // [TPT] - Three NEW Preferences Panels
	m_wndPhoenix.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndPhoenix2.m_psp.dwFlags &= ~PSH_HASHELP;
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	m_wndDebug.m_psp.dwFlags &= ~PSH_HASHELP;
#endif

	CTreePropSheet::SetPageIcon(&m_wndGeneral, _T("Preferences"));
	CTreePropSheet::SetPageIcon(&m_wndDisplay, _T("DISPLAY"));
	CTreePropSheet::SetPageIcon(&m_wndConnection, _T("CONNECTION"));
	CTreePropSheet::SetPageIcon(&m_wndProxy, _T("PROXY"));
	CTreePropSheet::SetPageIcon(&m_wndServer, _T("SEARCHMETHOD_SERVER"));
	CTreePropSheet::SetPageIcon(&m_wndDirectories, _T("FOLDERS"));
	CTreePropSheet::SetPageIcon(&m_wndFiles, _T("HARDDISK"));
	CTreePropSheet::SetPageIcon(&m_wndNotify, _T("NOTIFICATIONS"));
	CTreePropSheet::SetPageIcon(&m_wndStats, _T("PRIORITY"));
	CTreePropSheet::SetPageIcon(&m_wndIRC, _T("IRCCLIPBOARD"));
	CTreePropSheet::SetPageIcon(&m_wndSecurity, _T("SECURITY"));
	CTreePropSheet::SetPageIcon(&m_wndScheduler, _T("SCHEDULER"));
	CTreePropSheet::SetPageIcon(&m_wndWebServer, _T("WEB"));
	CTreePropSheet::SetPageIcon(&m_wndTweaks, _T("TWEAK"));	
	CTreePropSheet::SetPageIcon(&m_wndVirtual, _T("VIRTUAL"));
	CTreePropSheet::SetPageIcon(&m_wndPhoenix1, _T("PHOENIX"));
	CTreePropSheet::SetPageIcon(&m_wndPhoenix, _T("PHOENIX"));	
	CTreePropSheet::SetPageIcon(&m_wndPhoenix2, _T("PHOENIX"));
	
	
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	CTreePropSheet::SetPageIcon(&m_wndDebug, _T("Preferences"));
#endif

	AddPage(&m_wndGeneral);
	AddPage(&m_wndDisplay);
	AddPage(&m_wndConnection);
	AddPage(&m_wndProxy);
	AddPage(&m_wndServer);
	AddPage(&m_wndDirectories);
	AddPage(&m_wndFiles);
	AddPage(&m_wndNotify);
	AddPage(&m_wndStats);
	AddPage(&m_wndIRC);
	AddPage(&m_wndSecurity);
	AddPage(&m_wndScheduler);		
	AddPage(&m_wndWebServer);
	AddPage(&m_wndTweaks);
	// [TPT]	
	AddPage(&m_wndVirtual);	// [TPT] - itsonlyme: virtualDirs
	AddPage(&m_wndPhoenix1); // [TPT]
	AddPage(&m_wndPhoenix); // [TPT]	
	AddPage(&m_wndPhoenix2);// [TPT]
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	AddPage(&m_wndDebug);
#endif

	SetTreeViewMode(TRUE, TRUE, TRUE);
	SetTreeWidth(170);

	m_pPshStartPage = NULL;
}

CPreferencesDlg::~CPreferencesDlg()
{
}

void CPreferencesDlg::OnDestroy()
{
	CTreePropSheet::OnDestroy();
	thePrefs.Save();
	m_pPshStartPage = GetPage(GetActiveIndex())->m_psp.pszTemplate;
}

BOOL CPreferencesDlg::OnInitDialog()
{
	BOOL bResult = CModelessTreePropSheet::OnInitDialog();	// SLUGFILLER: modelessDialogs
	InitWindowStyles(this);

	for (int i = 0; i < m_pages.GetSize(); i++)
	{
		if (GetPage(i)->m_psp.pszTemplate == m_pPshStartPage)
		{
			SetActivePage(i);
			break;
		}
	}	

	// [TPT] - New Preferences Banner		
	CBitmap bmp;
	bmp.LoadBitmap(IDB_PHOENIX);
	m_banner.SetTexture((HBITMAP)bmp.Detach());	
	m_banner.SetFillFlag(KCSB_FILL_TEXTURE);
	m_banner.SetSize(80);
	m_banner.SetTitle(_T(""));
	m_banner.SetCaption(_T(""));
	m_banner.Attach(this, KCSB_ATTACH_RIGHT);	
	// [TPT] - New Preferences Banner end

	Localize();	
	return bResult;
}

void CPreferencesDlg::Localize()
{
	SetTitle(RemoveAmbersand(GetResString(IDS_EM_PREFS))); 

	m_wndGeneral.Localize();
	m_wndDisplay.Localize();
	m_wndConnection.Localize();
	m_wndServer.Localize();
	m_wndDirectories.Localize();
	m_wndFiles.Localize();
	m_wndStats.Localize();
	m_wndNotify.Localize();
	m_wndIRC.Localize();
	m_wndSecurity.Localize();
	m_wndTweaks.Localize();
	m_wndWebServer.Localize();
	m_wndScheduler.Localize();
	m_wndProxy.Localize();
	m_wndVirtual.Localize();	// [TPT] - itsonlyme: virtualDirs	
	m_wndPhoenix1.Localize(); 
	m_wndPhoenix.Localize();	// [TPT] - Three New Preferences Panel		
	m_wndPhoenix2.Localize();
	
	CTreeCtrl* pTree = GetPageTreeControl();
	if (pTree)
	{
		pTree->SetItemText(GetPageTreeItem(0), RemoveAmbersand(GetResString(IDS_PW_GENERAL)));
		pTree->SetItemText(GetPageTreeItem(1), RemoveAmbersand(GetResString(IDS_PW_DISPLAY))); 
		pTree->SetItemText(GetPageTreeItem(2), RemoveAmbersand(GetResString(IDS_PW_CONNECTION))); 
		pTree->SetItemText(GetPageTreeItem(3), RemoveAmbersand(GetResString(IDS_PW_PROXY))); 
		pTree->SetItemText(GetPageTreeItem(4), RemoveAmbersand(GetResString(IDS_PW_SERVER))); 
		pTree->SetItemText(GetPageTreeItem(5), RemoveAmbersand(GetResString(IDS_PW_DIR))); 
		pTree->SetItemText(GetPageTreeItem(6), RemoveAmbersand(GetResString(IDS_PW_FILES))); 
		pTree->SetItemText(GetPageTreeItem(7), RemoveAmbersand(GetResString(IDS_PW_EKDEV_OPTIONS))); 
		pTree->SetItemText(GetPageTreeItem(8), RemoveAmbersand(GetResString(IDS_STATSSETUPINFO))); 
		pTree->SetItemText(GetPageTreeItem(9), RemoveAmbersand(GetResString(IDS_IRC)));
		pTree->SetItemText(GetPageTreeItem(10), RemoveAmbersand(GetResString(IDS_SECURITY))); 
		pTree->SetItemText(GetPageTreeItem(11), RemoveAmbersand(GetResString(IDS_SCHEDULER)));
		pTree->SetItemText(GetPageTreeItem(12), RemoveAmbersand(GetResString(IDS_PW_WS)));
		pTree->SetItemText(GetPageTreeItem(13), RemoveAmbersand(GetResString(IDS_PW_TWEAK)));		
		// [TPT]
		pTree->SetItemText(GetPageTreeItem(14), RemoveAmbersand(GetResString(IDS_VDS_MM))); // [TPT] - itsonlyme: virtualDirs
		pTree->SetItemText(GetPageTreeItem(15), RemoveAmbersand(_T("pHoeniX One")));
		pTree->SetItemText(GetPageTreeItem(16), RemoveAmbersand(_T("pHoeniX Reload")));
		pTree->SetItemText(GetPageTreeItem(17), RemoveAmbersand(_T("pHoeniX Extreme")));
	#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
		pTree->SetItemText(GetPageTreeItem(18), _T("Debug"));
	#endif
	}	
	
	m_banner.UpdateSize(); // [TPT] - New Preferences Banner
	

	UpdateCaption();
}

void CPreferencesDlg::OnHelp()
{
	int iCurSel = GetActiveIndex();
	if (iCurSel >= 0)
	{
		CPropertyPage* pPage = GetPage(iCurSel);
		if (pPage)
		{
			HELPINFO hi = {0};
			hi.cbSize = sizeof hi;
			hi.iContextType = HELPINFO_WINDOW;
			hi.iCtrlId = 0;
			hi.hItemHandle = pPage->m_hWnd;
			hi.dwContextId = 0;
			pPage->SendMessage(WM_HELP, 0, (LPARAM)&hi);
			return;
		}
	}

	theApp.ShowHelp(0, HELP_CONTENTS);
}

BOOL CPreferencesDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPreferencesDlg::OnHelpInfo(HELPINFO* pHelpInfo)
{
	OnHelp();
	return TRUE;
}

void CPreferencesDlg::SetStartPage(UINT uStartPageID)
{
	m_pPshStartPage = MAKEINTRESOURCE(uStartPageID);
}
