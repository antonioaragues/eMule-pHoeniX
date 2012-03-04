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
#include "PPgNotify.h"
#include "Preferences.h"
#include "OtherFunctions.h"
#include "Preferences.h"
#include "HelpIDs.h"
#include "ini2.h"
#include "emuledlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

// [TPT] - enkeyDEV(th1) -notifier-

IMPLEMENT_DYNAMIC(CPPgNotify, CPropertyPage)

CPPgNotify::CPPgNotify()
	: CPropertyPage(CPPgNotify::IDD)
{
	LoadSettings();
	Localize();
}

CPPgNotify::~CPPgNotify()
{
}

void CPPgNotify::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LBOX_TBN_CONFIG, tnbConfigList);
}

void CPPgNotify::LoadSettings(void)
{
	CEdit* editPtr;
	CButton* btnPTR;
	CString selectedskin;
	char* itemdata;
	
	USES_CONVERSION;
	if(m_hWnd)
	{
		// start added by InterCeptor (notify on error) 11.11.02
		if (thePrefs.useErrorNotifier)
				CheckDlgButton(IDC_CB_TBN_ONERROR, BST_CHECKED);
		// end added by InterCeptor (notify on error) 11.11.02
		if (thePrefs.useDownloadNotifier) 
			CheckDlgButton(IDC_CB_TBN_ONDOWNLOAD, BST_CHECKED);
		if (thePrefs.useNewDownloadNotifier) 
			CheckDlgButton(IDC_CB_TBN_ONNEWDOWNLOAD, BST_CHECKED);
		if (thePrefs.useChatNotifier)  
			CheckDlgButton(IDC_CB_TBN_ONCHAT, BST_CHECKED);
		if (thePrefs.useSoundInNotifier)
			CheckDlgButton(IDC_CB_TBN_USESOUND, BST_CHECKED);
		if (thePrefs.useLogNotifier)
			CheckDlgButton(IDC_CB_TBN_ONLOG, BST_CHECKED);
		if (thePrefs.notifierImportantError) 
			CheckDlgButton(IDC_CB_TBN_IMPORTATNT, BST_CHECKED);
		if (thePrefs.notifierPopsEveryChatMsg)
			CheckDlgButton(IDC_CB_TBN_POP_ALWAYS, BST_CHECKED);
		if (thePrefs.notifierAutoClose)
			CheckDlgButton(IDC_CB_TBN_AUTOCLOSE, BST_CHECKED);
		if (thePrefs.notifierNewVersion) 
			CheckDlgButton(IDC_CB_TBN_ONNEWVERSION, BST_CHECKED);
		if (thePrefs.notifierSearchCompleted) 
			CheckDlgButton(IDC_CB_TBN_SEARCH, BST_CHECKED);
		if (thePrefs.notifierNewPvtMsg) 
			CheckDlgButton(IDC_CB_TBN_NEWPRIVATEMSG, BST_CHECKED);//Rocks
		
	
		btnPTR = (CButton*) GetDlgItem(IDC_CB_TBN_POP_ALWAYS);
		btnPTR->EnableWindow(IsDlgButtonChecked(IDC_CB_TBN_ONCHAT));
		editPtr = (CEdit*) GetDlgItem(IDC_EDIT_TBN_WAVFILE);
		editPtr->SetWindowText(LPCTSTR(thePrefs.notifierSoundFilePath));
			
		LoadNotifierConfigurations();

		// load the correct config name in the list box
		selectedskin.Format(_T("%s"), thePrefs.notifierConfiguration);
		if (!selectedskin.IsEmpty()) {
			for (int i=0; i<tnbConfigList.GetCount(); i++) {
				itemdata = (char*) tnbConfigList.GetItemDataPtr(i);
				if (_tcscmp(A2T(itemdata), thePrefs.notifierConfiguration) == 0)
					tnbConfigList.SetCurSel(i);
			}
		}
	}
}

CString CPPgNotify::DialogBrowseFile(CString Filters, CString DefaultFileName) {
	CFileDialog myFileDialog(true,NULL,LPCTSTR(DefaultFileName),
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, LPCTSTR(Filters));
	myFileDialog.DoModal();
	return myFileDialog.GetPathName();
}

void CPPgNotify::LoadNotifierConfigurations() {

	WIN32_FIND_DATA FileData;
	HANDLE hSearch;
	bool fFinished = false;	
	char* itemdata;
	CString configDir = _T("");
	CString filenamenoext = _T("");

	USES_CONVERSION;

	configDir.Format(_T("%s\\tbnskin"), thePrefs.GetAppDir());

	// Start searching for .ini files in the current directory.

	hSearch = FindFirstFile(configDir + CString(_T("\\*.ini")), &FileData);
	if (hSearch == INVALID_HANDLE_VALUE) {
		 fFinished = true;
	}

	while (!fFinished)
	{
		// load list box items		
		itemdata = nstrdup(T2CA(configDir + CString(_T("\\"))+CString(FileData.cFileName)));
		CIni currentIni(A2T(itemdata), _T("CONFIG"));
		int itemNum = tnbConfigList.AddString(currentIni.GetString(_T("SkinName"),CString(FileData.cFileName).Left(CString(FileData.cFileName).GetLength()-4)));
		tnbConfigList.SetItemDataPtr(itemNum, (void*) itemdata);
		
		if (!FindNextFile(hSearch, &FileData))
		{
			if (GetLastError() == ERROR_NO_MORE_FILES)
				fFinished = true;
			else
				//TODO: better integrated error handling
				fFinished = true;
		}
	}

	// Close the search handle.
	if (!FindClose(hSearch)) { //TODO: better integrated error handling
	}
}

void CPPgNotify::Localize(void)
{
	if(m_hWnd){
		SetWindowText(GetResString(IDS_PW_EKDEV_OPTIONS));
		GetDlgItem(IDC_CB_TBN_USESOUND)->SetWindowText(GetResString(IDS_PW_TBN_USESOUND));
		GetDlgItem(IDC_BTN_BROWSE_WAV)->SetWindowText(GetResString(IDS_PW_BROWSE));
		GetDlgItem(IDC_CB_TBN_ONLOG)->SetWindowText(GetResString(IDS_PW_TBN_ONLOG));
		GetDlgItem(IDC_CB_TBN_ONCHAT)->SetWindowText(GetResString(IDS_PW_TBN_ONCHAT));
		GetDlgItem(IDC_CB_TBN_POP_ALWAYS)->SetWindowText(GetResString(IDS_PW_TBN_POP_ALWAYS));
		GetDlgItem(IDC_CB_TBN_ONDOWNLOAD)->SetWindowText(GetResString(IDS_PW_TBN_ONDOWNLOAD));
		GetDlgItem(IDC_CB_TBN_ONNEWDOWNLOAD)->SetWindowText(GetResString(IDS_TBN_ONNEWDOWNLOAD));
		GetDlgItem(IDC_TASKBARNOTIFIER)->SetWindowText(GetResString(IDS_PW_TASKBARNOTIFIER));
		GetDlgItem(IDC_CB_TBN_IMPORTATNT)->SetWindowText(GetResString(IDS_PS_TBN_IMPORTANT));
		GetDlgItem(IDC_CB_TBN_AUTOCLOSE)->SetWindowText(GetResString(IDS_PW_TBN_AUTOCLOSE));   
		GetDlgItem(IDC_CB_TBN_ONERROR)->SetWindowText(GetResString(IDS_PW_TBN_ONERROR));
		GetDlgItem(IDC_LBL_TBN_CONFIGSELECTION)->SetWindowText(GetResString(IDS_PW_TBN_CONFIG));
		GetDlgItem(IDC_TBN_OPTIONS)->SetWindowText(GetResString(IDS_PW_TBN_OPTIONS));
		GetDlgItem(IDC_CB_TBN_ONNEWVERSION)->SetWindowText(GetResString(IDS_CB_TBN_ONNEWVERSION));
		GetDlgItem(IDC_CB_TBN_SEARCH)->SetWindowText(GetResString(IDS_PW_TBN_ONSEARCH));
		GetDlgItem(IDC_CB_TBN_NEWPRIVATEMSG)->SetWindowText(GetResString(IDS_PW_TBN_NEWPRIVATEMSG));//Rocks
	}
}

BEGIN_MESSAGE_MAP(CPPgNotify, CPropertyPage)
	ON_BN_CLICKED(IDC_CB_TBN_USESOUND, OnSettingsChange)
	ON_BN_CLICKED(IDC_CB_TBN_ONLOG, OnSettingsChange)
	ON_BN_CLICKED(IDC_CB_TBN_ONCHAT, OnBnClickedCbTbnOnchat)
	ON_BN_CLICKED(IDC_CB_TBN_POP_ALWAYS, OnSettingsChange)
	ON_BN_CLICKED(IDC_CB_TBN_ONDOWNLOAD, OnSettingsChange)
	ON_BN_CLICKED(IDC_CB_TBN_ONNEWDOWNLOAD, OnSettingsChange)
    ON_BN_CLICKED(IDC_CB_TBN_ONERROR, OnSettingsChange)
	ON_BN_CLICKED(IDC_CB_TBN_IMPORTATNT, OnSettingsChange)
	ON_BN_CLICKED(IDC_CB_TBN_AUTOCLOSE, OnSettingsChange)
	ON_BN_CLICKED(IDC_BTN_BROWSE_WAV, OnBnClickedBtnBrowseWav)
	ON_BN_CLICKED(IDC_CB_TBN_ONNEWVERSION, OnSettingsChange)
	ON_BN_CLICKED(IDC_CB_TBN_SEARCH, OnSettingsChange)
	ON_WM_DESTROY()
	ON_LBN_SELCHANGE(IDC_LBOX_TBN_CONFIG, OnLbnSelchangeLboxTbnConfig)
	ON_BN_CLICKED(IDC_CB_TBN_NEWPRIVATEMSG, OnSettingsChange)//Rocks
	ON_WM_HELPINFO()
END_MESSAGE_MAP()


// gestori di messaggi CPPgNotify

BOOL CPPgNotify::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);
	LoadSettings();
	Localize();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPgNotify::OnApply() {
    CEdit* editPTR;
	CString buffer, cfgname, oldconfig;
	char* itemdata;
		
	USES_CONVERSION;
	thePrefs.useErrorNotifier = IsDlgButtonChecked(IDC_CB_TBN_ONERROR);	// added by InterCeptor (notify on error) 11.11.02
    thePrefs.useDownloadNotifier = IsDlgButtonChecked(IDC_CB_TBN_ONDOWNLOAD);
    thePrefs.useNewDownloadNotifier = IsDlgButtonChecked(IDC_CB_TBN_ONNEWDOWNLOAD);
    thePrefs.useChatNotifier = IsDlgButtonChecked(IDC_CB_TBN_ONCHAT);
    thePrefs.useLogNotifier = IsDlgButtonChecked(IDC_CB_TBN_ONLOG);        
    thePrefs.useSoundInNotifier = IsDlgButtonChecked(IDC_CB_TBN_USESOUND);
	thePrefs.notifierImportantError = IsDlgButtonChecked(IDC_CB_TBN_IMPORTATNT);
	thePrefs.notifierPopsEveryChatMsg = IsDlgButtonChecked(IDC_CB_TBN_POP_ALWAYS);
	thePrefs.notifierAutoClose = IsDlgButtonChecked(IDC_CB_TBN_AUTOCLOSE); 
	thePrefs.notifierNewVersion = IsDlgButtonChecked(IDC_CB_TBN_ONNEWVERSION);
	thePrefs.notifierSearchCompleted = IsDlgButtonChecked(IDC_CB_TBN_SEARCH);		
	thePrefs.notifierNewPvtMsg = IsDlgButtonChecked(IDC_CB_TBN_NEWPRIVATEMSG);//Rocks

	int sel = tnbConfigList.GetCurSel();
	if (sel != LB_ERR)
		tnbConfigList.GetText(sel, cfgname);
	else
		cfgname = _T("");

	if (!cfgname.IsEmpty()) {		
		itemdata = (char*) tnbConfigList.GetItemDataPtr(tnbConfigList.GetCurSel());
		oldconfig.Format(_T("%s"), thePrefs.notifierConfiguration);
		_stprintf(thePrefs.notifierConfiguration, _T("%s"), A2T(itemdata));

		if (_tcscmp(oldconfig, thePrefs.notifierConfiguration) != 0) { //load new configuration
			theApp.emuledlg->LoadNotifier();
			theApp.emuledlg->ShowNotifier(GetResString(IDS_TBN_READY));
		}
	}	

	buffer = _T("");
    editPTR = (CEdit*) GetDlgItem(IDC_EDIT_TBN_WAVFILE);
	editPTR->GetWindowText(buffer);
	_stprintf(thePrefs.notifierSoundFilePath, _T("%s"), buffer);
    
	SetModified(FALSE);
	return CPropertyPage::OnApply();
}

void CPPgNotify::OnBnClickedCbTbnOnchat()
{
    CButton* btnPTR;
    btnPTR = (CButton*) GetDlgItem(IDC_CB_TBN_POP_ALWAYS);
    btnPTR->EnableWindow(IsDlgButtonChecked(IDC_CB_TBN_ONCHAT));	
	SetModified();
}

void CPPgNotify::OnBnClickedBtnBrowseWav()
{
    CEdit* editPTR;
	CString buffer;
    buffer = DialogBrowseFile(_T("File wav (*.wav)|*.wav||"));
    editPTR = (CEdit*) GetDlgItem(IDC_EDIT_TBN_WAVFILE);
    editPTR->SetWindowText(LPCTSTR(buffer));
	_stprintf(thePrefs.notifierSoundFilePath, _T("%s"), buffer);
	SetModified();
}

void CPPgNotify::OnDestroy()
{
	char* itemdata;

	for (int i=0; i<tnbConfigList.GetCount(); i++) {
		itemdata = (char*) tnbConfigList.GetItemDataPtr(i);
		delete[] itemdata;
		itemdata = NULL;
	}
}

void CPPgNotify::OnHelp()
{
	theApp.ShowHelp(eMule_FAQ_Preferences_Notifications);
}

BOOL CPPgNotify::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPPgNotify::OnHelpInfo(HELPINFO* pHelpInfo)
{
	OnHelp();
	return TRUE;
}
