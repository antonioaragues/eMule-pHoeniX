// PPgPhoenix1.cpp : implementation file
//

#include "stdafx.h"
#include "emule.h"
#include "PPgPhoenix1.h"
#include "preferences.h"
#include "PreferencesDlg.h"
#include "HttpDownloadDlg.h"
#include "BandwidthControl.h"
#include "EmuleDlg.h"
#include "SharedFilesWnd.h"
#include "TransferWnd.h"
#include "HelpIDs.h"
#include "opcodes.h"
#include "XMessageBox.h" // [TPT]- TBH-AutoBackup
#include "scheduler.h"
#include "Fakecheck.h" // [TPT] - Fakecheck
#include "UploadBandwidthThrottler.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CPPgPhoenix1 dialog

IMPLEMENT_DYNAMIC(CPPgPhoenix1, CPropertyPage) // [TPT] - SLUGFILLER: modelessDialogs
CPPgPhoenix1::CPPgPhoenix1()
	: CPropertyPage(CPPgPhoenix1::IDD) // [TPT] - SLUGFILLER: modelessDialogs
{	

}

CPPgPhoenix1::~CPPgPhoenix1()
{
}

void CPPgPhoenix1::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PPG_PHOENIX1_TAB, m_tabCtr);
}

BEGIN_MESSAGE_MAP(CPPgPhoenix1, CPropertyPage) // [TPT] - SLUGFILLER: modelessDialogs
	// TAB control
	ON_NOTIFY(TCN_SELCHANGE, IDC_PPG_PHOENIX1_TAB, OnTabSelectionChange)

	//--------------------------------------------------------------BEGIN-

	//CONEXION -----------------------------------------------------------
	ON_EN_CHANGE(IDC_PPG_MAELLA_MTU_EDIT, OnSettingsChange) 
	// Maella -Send Socket Size-
	ON_EN_CHANGE(IDC_PPG_MAELLA_SND_SOCKET_SIZE_EDIT, OnSettingsChange) 
	// [TPT] - quick start
	ON_BN_CLICKED(IDC_PPG_PHOENIX_QUICKSTART, OnQuickStartChange)   	
	
	// [TPT] - Manage Connection
	ON_BN_CLICKED(IDC_PPG_PHOENIX_MANAGECONNECTION, OnSettingsChange) 

	// [TPT] - MoNKi: -UPnPNAT Support-
	ON_BN_CLICKED(IDC_PPG_PHOENIX_UPNP_NAT, OnSettingsChange)
	ON_BN_CLICKED(IDC_PPG_PHOENIX_UPNP_NAT_WEB, OnSettingsChange)
	// [TPT] - MoNKi: -UPnPNAT Support-


	//SUBIDAS ------------------------------------------------------------
	// Maella -NAFC-	
	ON_EN_CHANGE(IDC_PPG_MAELLA_NAFC_EDIT, OnSettingsChange) 
	ON_BN_CLICKED(IDC_PPG_MAELLA_NAFC_CHECK01, OnNAFCChange) 
	ON_BN_CLICKED(IDC_PPG_MAELLA_NAFC_CHECK02, OnNAFCFullControlChange) 
	// Maella -Minimum Upload Slot-
	ON_EN_CHANGE(IDC_PPG_MAELLA_MIN_SLOT_EDIT, OnSettingsChange) 
	// Maella - New Upload Slot Sharping-
	ON_BN_CLICKED(IDC_PPG_MAELLA_NUSS_CHECK01, OnNewUploadSlotSharpingChange)
	// [TPT] - Cumulate Bandwidth
	ON_BN_CLICKED(IDC_PPG_PHOENIX_CUMULATEBW, OnSettingsChange) 
	// [TPT] - Pawcio: MUS
	ON_BN_CLICKED(IDC_PPG_PHOENIX_MINIMIZESLOTS, OnSettingsChange) 	

 	ON_BN_CLICKED(IDC_PPG_PHOENIX_UNLIMITEDSPEED, OnUnlimitedSpeed)

	//ZZ UploadSpeedSense
	ON_BN_CLICKED(IDC_PPG_PHOENIX_USSENABLE, OnUSSChange)
	ON_BN_CLICKED(IDC_PPG_PHOENIX_USSTOLERANCEPERCENTCHECK, OnUSSTolerancePercent)
	ON_BN_CLICKED(IDC_PPG_PHOENIX_USSTOLERANCEMSCHECK, OnUSSToleranceCheck)

	ON_EN_CHANGE(IDC_PPG_PHOENIX_USSLOWSPEED, OnSettingsChange) 
	ON_EN_CHANGE(IDC_PPG_PHOENIX_USSTOLERANCEPERCENTEDIT, OnSettingsChange) 
	ON_EN_CHANGE(IDC_PPG_PHOENIX_USSTOLERANCEMSEDIT, OnSettingsChange) 
	ON_EN_CHANGE(IDC_PPG_PHOENIX_USSGOINGUP, OnSettingsChange) 
	ON_EN_CHANGE(IDC_PPG_PHOENIX_USSGOINGDOWN, OnSettingsChange) 
	ON_EN_CHANGE(IDC_PPG_PHOENIX_USSNUMBEROFPINGS, OnSettingsChange) 

	
	ON_BN_CLICKED(IDC_PPG_PHOENIX_CHANGEUPVIEWS, OnBnClickedChangeUpViews)

	//FUENTES -----------------------------------------------------------
	// Maella -Enable/Disable source exchange in preference- (Tarod)
	ON_BN_CLICKED(IDC_PPG_MAELLA_XS_CHECK, OnSettingsChange) 
	ON_BN_CLICKED(IDC_PPG_MAELLA_REASK_SOURCE_AFTER_IP_CHANGE_CHECK, OnSettingsChange) 
	// [TPT] - SUQWT
	ON_BN_CLICKED(IDC_PPG_PHOENIX_SUQWT, OnSettingsChange)

	//SEGURIDAD ----------------------------------------------------------
	ON_BN_CLICKED(IDC_PPG_PHOENIX_SNAFU_CHECK, OnSettingsChange) 
	ON_BN_CLICKED(IDC_PPG_PHOENIX_ACT_CHECK, OnSettingsChange) 
	ON_BN_CLICKED(IDC_PPG_PHOENIX_FRIENDSHARE_CHECK, OnSettingsChange) 
	// [TPT] - eWombat SNAFU v2end
	// [TPT] - Fakecheck
	ON_BN_CLICKED(IDC_PPG_FAKESTARTUP, OnSettingsChange)
	ON_BN_CLICKED(IDC_PPG_FAKEUP, OnBnClickedUpdatefakes)
	ON_EN_CHANGE(IDC_PPG_FAKEURL, OnSettingsChange)
	// [TPT] - Fakecheck end

	//LOG FILTERS --------------------------------------------------------
	// Maella -Filter verbose messages-
	ON_BN_CLICKED(IDC_PPG_MAELLA_FILTER_VERBOSE_CHECK01, OnSettingsChange)
	ON_BN_CLICKED(IDC_PPG_MAELLA_FILTER_VERBOSE_CHECK02, OnSettingsChange)  // [TPT] - Filter dead sources	
	ON_BN_CLICKED(IDC_PPG_MAELLA_FILTER_VERBOSE_CHECK04, OnSettingsChange) 
	ON_BN_CLICKED(IDC_PPG_MAELLA_FILTER_VERBOSE_CHECK05, OnSettingsChange) 
	ON_BN_CLICKED(IDC_PPG_MAELLA_FILTER_VERBOSE_CHECK06, OnSettingsChange) 
	ON_BN_CLICKED(IDC_PPG_MAELLA_FILTER_VERBOSE_CHECK07, OnSettingsChange) 
	ON_BN_CLICKED(IDC_PPG_MAELLA_FILTER_VERBOSE_CHECK08, OnSettingsChange) 
	ON_BN_CLICKED(IDC_PPG_MAELLA_FILTER_VERBOSE_CHECK09, OnSettingsChange)  // [TPT] - Filter own messages
	ON_BN_CLICKED(IDC_PPG_MAELLA_FILTER_VERBOSE_CHECK10, OnSettingsChange)  // [TPT] - Filter Handshake messages
	
	
	//BACKUP FILES --------------------------------------------------------
	// [TPT]- TBH-AutoBackup
	ON_BN_CLICKED(IDC_BACKUPNOW, OnBnClickedBackupnow)
	ON_BN_CLICKED(IDC_DAT, OnBnClickedDat)
	ON_BN_CLICKED(IDC_MET, OnBnClickedMet)
	ON_BN_CLICKED(IDC_INI, OnBnClickedIni)
	ON_BN_CLICKED(IDC_PART, OnBnClickedPart)
	ON_BN_CLICKED(IDC_PARTMET, OnBnClickedPartMet)
	ON_BN_CLICKED(IDC_SELECTALL, OnBnClickedSelectall)
	ON_BN_CLICKED(IDC_AUTOBACKUP, OnBnAutobackup)
	ON_BN_CLICKED(IDC_AUTOBACKUP2, OnSettingsChange)

	//----------------------------------------------------------------END-

	ON_WM_HSCROLL()

	ON_WM_HELPINFO()

END_MESSAGE_MAP()

// CPPgPhoenix1 message handlers

BOOL CPPgPhoenix1::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// Init the Tab control
	InitTab();

	// Create and Init all controls
	InitControl();

	m_Tip.Create(this);// [TPT] - Tooltips in preferences
	m_Tip.SetEffectBk(CPPToolTip::PPTOOLTIP_EFFECT_HGRADIENT);
	m_Tip.SetGradientColors(RGB(255,255,225),RGB(0,0,0), RGB(255,198,167));

	// Set default tab
	m_currentTab = NONE;
	m_tabCtr.SetCurSel(0);
	SetTab(CONNECTION);//Tab x defecto Conexion
	
	// Load setting
	LoadSettings();
	isWndNormalUpActive = true;
	Localize();
	UpdatepHoeniXOneTooltips();// [TPT] - Tooltips in preferences


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// [TPT] - Tooltips in preferences
BOOL CPPgPhoenix1::PreTranslateMessage(MSG* pMsg) 
{
	m_Tip.RelayEvent(pMsg);

	return CDialog::PreTranslateMessage(pMsg);
}

void CPPgPhoenix1::UpdatepHoeniXOneTooltips()
{
	//Conection
	m_Tip.AddTool(GetDlgItem(IDC_PPG_MAELLA_MTU_EDIT), GetResString(IDS_TOOLTIP_MTU));
	m_Tip.AddTool(GetDlgItem(IDC_PPG_MAELLA_SND_SOCKET_SIZE_EDIT), GetResString(IDS_TOOLTIP_SENDSOCKETBUFFER));
	m_Tip.AddTool(GetDlgItem(IDC_PPG_PHOENIX_QUICKSTART), GetResString(IDS_TOOLTIP_QUICKSART));
	m_Tip.AddTool(GetDlgItem(IDC_PPG_PHOENIX_QUICKSTART_CONN_PER_FIVE), GetResString(IDS_TOOLTIP_MAXCONPERFIVE));
	m_Tip.AddTool(GetDlgItem(IDC_PPG_PHOENIX_QUICKSTART_CONN), GetResString(IDS_TOOLTIP_MAXCON));
	m_Tip.AddTool(GetDlgItem(IDC_PPG_PHOENIX_MANAGECONNECTION), GetResString(IDS_TOOLTIP_MANAGECON));
	m_Tip.AddTool(GetDlgItem(IDC_PPG_PHOENIX_UPNP_NAT), GetResString(IDS_TOOLTIP_UPNPNAT));
	m_Tip.AddTool(GetDlgItem(IDC_PPG_PHOENIX_UPNP_NAT_WEB), GetResString(IDS_TOOLTIP_WEBINT));

	m_Tip.AddTool(GetDlgItem(IDC_PPG_MAELLA_MTU_STATIC), GetResString(IDS_TOOLTIP_MTU));
	m_Tip.AddTool(GetDlgItem(IDC_PPG_MAELLA_SND_SOCKET_SIZE_STATIC), GetResString(IDS_TOOLTIP_SENDSOCKETBUFFER));
	m_Tip.AddTool(GetDlgItem(IDC_PPG_PHOENIX_QUICKSTART_CONN_PER_FIVE_STATIC), GetResString(IDS_TOOLTIP_MAXCONPERFIVE));
	m_Tip.AddTool(GetDlgItem(IDC_PPG_PHOENIX_QUICKSTART_CONN_STATIC), GetResString(IDS_TOOLTIP_MAXCON));

	//Uploads
	//Standard Upload
	m_Tip.AddTool(GetDlgItem(IDC_PPG_MAELLA_NAFC_CHECK01), GetResString(IDS_TOOLTIP_CONTRUPLNAFC));
	//m_Tip.AddTool(GetDlgItem(IDC_PPG_MAELLA_NAFC_EDIT), GetResString(IDS_TOOLTIP_UPLTHRESHOLD));
	m_Tip.AddTool(GetDlgItem(IDC_PPG_MAELLA_NAFC_CHECK02), GetResString(IDS_TOOLTIP_AUTOUDLIMITS));
	m_Tip.AddTool(GetDlgItem(IDC_PPG_MAELLA_MIN_SLOT_EDIT), GetResString(IDS_TOOLTIP_MINUPLSLOTS));
	m_Tip.AddTool(GetDlgItem(IDC_PPG_MAELLA_NUSS_CHECK01), GetResString(IDS_TOOLTIP_NEWUPLSLOTSH));
	m_Tip.AddTool(GetDlgItem(IDC_PPG_PHOENIX_CUMULATEBW), GetResString(IDS_TOOLTIP_CUMULATE));
	m_Tip.AddTool(GetDlgItem(IDC_PPG_PHOENIX_MINIMIZESLOTS), GetResString(IDS_TOOLTIP_MINIMIZE));
	m_Tip.AddTool(GetDlgItem(IDC_PPG_MAELLA_MIN_SLOT_STATIC), GetResString(IDS_TOOLTIP_MINUPLSLOTS));

	
	//Sources
	m_Tip.AddTool(GetDlgItem(IDC_PPG_MAELLA_XS_CHECK), GetResString(IDS_TOOLTIP_SOURCEEXCH));
	m_Tip.AddTool(GetDlgItem(IDC_PPG_MAELLA_REASK_SOURCE_AFTER_IP_CHANGE_CHECK), GetResString(IDS_TOOLTIP_DONTREASK));

	//Security
	m_Tip.AddTool(GetDlgItem(IDC_PPG_PHOENIX_SNAFU_CHECK), GetResString(IDS_TOOLTIP_SNAFU));
	m_Tip.AddTool(GetDlgItem(IDC_PPG_PHOENIX_ACT_CHECK), GetResString(IDS_TOOLTIP_ANTICREDIT));
	m_Tip.AddTool(GetDlgItem(IDC_PPG_PHOENIX_FRIENDSHARE_CHECK), GetResString(IDS_TOOLTIP_ANTIFRIENDSHARE));

	//Backup
	m_Tip.AddTool(GetDlgItem(IDC_BACKUPNOW), GetResString(IDS_TOOLTIP_BACKUPNOW));
	m_Tip.AddTool(GetDlgItem(IDC_DAT), GetResString(IDS_TOOLTIP_DAT));
	m_Tip.AddTool(GetDlgItem(IDC_MET), GetResString(IDS_TOOLTIP_DAT));
	m_Tip.AddTool(GetDlgItem(IDC_INI), GetResString(IDS_TOOLTIP_DAT));
	m_Tip.AddTool(GetDlgItem(IDC_PART), GetResString(IDS_TOOLTIP_DAT));
	m_Tip.AddTool(GetDlgItem(IDC_PARTMET), GetResString(IDS_TOOLTIP_DAT));
	m_Tip.AddTool(GetDlgItem(IDC_SELECTALL), GetResString(IDS_TOOLTIP_SELECTALL));
	m_Tip.AddTool(GetDlgItem(IDC_AUTOBACKUP), GetResString(IDS_TOOLTIP_AUTOBACKUP));
	m_Tip.AddTool(GetDlgItem(IDC_AUTOBACKUP2), GetResString(IDS_TOOLTIP_DOUBLEBACKUP));
}

// [TPT] - Tooltips in preferences END

void CPPgPhoenix1::LoadSettings()
{

		//We compare at the beginnig to set button to correct state
		if((thePrefs.IsDynUpEnabled() && thePrefs.GetNAFCFullControl() == true) || (thePrefs.IsDynUpEnabled() && thePrefs.GetEnableNewUSS() == true))
		{
			TCHAR buffer[200];
			_stprintf(buffer,GetResString(IDS_SELECTUP));
			if(MessageBox(buffer,GetResString(IDS_UPUNCOMPATIBLE),MB_ICONQUESTION|MB_YESNO)== IDYES)
			{
				//Desactivo
				thePrefs.SetNAFCFullControl(false);
				thePrefs.SetEnableNewUSS(false);
			}
			else
				thePrefs.SetDynUpEnabled(false);
		}

	//--------------------------------------------------------------BEGIN-

	//CONEXION -----------------------------------------------------------

	// Maella -MTU Configuration-
	{
		CString buffer;
		buffer.Format(_T("%u"), thePrefs.GetMTU());
		m_MTUEdit.SetWindowText(buffer);
	}
	// Maella -New bandwidth control-
	{
		CString buffer;
		buffer.Format(_T("%u"), thePrefs.GetSendSocketBufferSize());
		m_sndSocketSizeEdit.SetWindowText(buffer);
	}
	// [TPT] - quick start
	m_iQuickStart.SetCheck(thePrefs.GetQuickStart());
	{
		CString buffer;
		buffer.Format(_T("%u"), thePrefs.GetQuickStartMaxConPerFive());
		m_iMaxConnPerFive.EnableWindow(m_iQuickStart.GetCheck() == BST_CHECKED);		
		m_iMaxConnPerFive.SetWindowText(buffer);
		m_iMaxConnPerFiveStatic.EnableWindow(m_iQuickStart.GetCheck() == BST_CHECKED);
		m_iMaxConn.EnableWindow(m_iQuickStart.GetCheck() == BST_CHECKED);
		buffer.Format(_T("%u"), thePrefs.GetQuickStartMaxCon());
		m_iMaxConn.SetWindowText(buffer);
		m_iMaxConnStatic.EnableWindow(m_iQuickStart.GetCheck() == BST_CHECKED);
	}
	// [TPT] - quick start

	m_iManageConnection.SetCheck(thePrefs.IsManageConnection()); // [TPT] - Manage Connection

	// [TPT] - MoNKi: -UPnPNAT Support-
	m_iUPnPNat.SetCheck(thePrefs.GetUPnPNat());
	m_iUPnPNatWeb.SetCheck(thePrefs.GetUPnPNatWeb());
	// [TPT] - MoNKi: -UPnPNAT Support-


	//SUBIDAS ------------------------------------------------------------
	// Maella -Network Adapter Feedback Control-
	m_NAFCCheck01.EnableWindow(theApp.pBandWidthControl->IsNAFCAvailable() == true);
	m_NAFCCheck01.SetCheck((thePrefs.GetNAFCEnable() == true) ? BST_CHECKED : BST_UNCHECKED);
	m_NAFCCheck02.EnableWindow(theApp.pBandWidthControl->IsNAFCAvailable() == true && m_NAFCCheck01.GetCheck() != BST_UNCHECKED);
	m_NAFCCheck02.SetCheck((thePrefs.GetNAFCFullControl() == true) ? BST_CHECKED : BST_UNCHECKED);
	{
		CString buffer;
		m_NAFCEdit.EnableWindow(false);
		//m_NAFCEdit.EnableWindow(m_NAFCCheck01.GetCheck() == BST_CHECKED);
		buffer.Format(_T("%.1f"), thePrefs.maxupload);//Pillamos la subida de conexion
		m_NAFCEdit.SetWindowText(buffer);
	}
	// Maella -Minimum Upload Slot-
	{
		CString buffer;
		buffer.Format(_T("%u"), thePrefs.GetMinUploadSlot());
		m_minSlotEdit.SetWindowText(buffer);
	}
	//New Upload slot sharping

	m_newUploadSlotSharpingCheck.SetCheck((thePrefs.GetEnableNewUSS() == true) ? BST_CHECKED : BST_UNCHECKED);		
	m_iCumulateBW.SetCheck(thePrefs.IsCumulateBandwidthEnabled()); // [TPT] - Cumulate Bandwidth
	m_iMinimizeSlots.SetCheck(thePrefs.MinimizeNumberOfSlots()); // [TPT] - Pawcio: MUS

	m_unlimitedUP.SetCheck(thePrefs.GetUnlimitedUp());

	//ZZ UploadSpeedSense %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

	CString ussBuffer;
	
	//Enable zz uss
	m_USSEnable.SetCheck((thePrefs.IsDynUpEnabled() == true) ? BST_CHECKED : BST_UNCHECKED);
	//zz: Min Up Speed
	ussBuffer.Format(_T("%.1f"), thePrefs.GetMinUpload());
	m_USSLowUpSpeed.SetWindowText(ussBuffer);
	//zz: Ping Tolerance
	m_USSPingTolerancePercentCheck.SetCheck((!thePrefs.IsDynUpUseMillisecondPingTolerance() == true) ? BST_CHECKED : BST_UNCHECKED);
	ussBuffer.Format(_T("%u"), thePrefs.GetDynUpPingTolerance());
	m_USSPingTolerancePercentEdit.SetWindowText(ussBuffer);

	m_USSPingToleranceMSCheck.SetCheck((thePrefs.IsDynUpUseMillisecondPingTolerance() == true) ? BST_CHECKED : BST_UNCHECKED);
	ussBuffer.Format(_T("%u"), thePrefs.GetDynUpPingToleranceMilliseconds());
	m_USSPingToleranceMSEdit.SetWindowText(ussBuffer);	
	
	//zz: going down
	ussBuffer.Format(_T("%u"), thePrefs.GetDynUpGoingDownDivider());
	m_USSGoingDown.SetWindowText(ussBuffer);	
	//zz: going up
	ussBuffer.Format(_T("%u"), thePrefs.GetDynUpGoingUpDivider());
	m_USSGoingUp.SetWindowText(ussBuffer);	
	//zz: number of pings
	ussBuffer.Format(_T("%u"), thePrefs.GetDynUpNumberOfPings());
	m_USSNumberOfPings.SetWindowText(ussBuffer);


	//SERVIDOR -----------------------------------------------------------
	// Maella -Enable/Disable source exchange in preference- (Tarod)
	m_XSCheck.SetCheck((thePrefs.GetDisabledXS() == true) ? BST_CHECKED : BST_UNCHECKED);
	// Maella -Reask sources after IP change- (idea Xman)
	m_reaskSourceAfterIPChangeCheck.SetCheck((thePrefs.GetReaskSourceAfterIPChange() != true) ? BST_CHECKED : BST_UNCHECKED); // [TPT]
	m_iSUQWT.SetCheck(thePrefs.SaveUploadQueueWaitTime()); // [TPT] - SUQWT

	//SEGURIDAD ----------------------------------------------------------
	// [TPT] - eWombat SNAFU v2
	m_iSnafu.SetCheck(thePrefs.GetAntiSnafu());
	m_iAntiCreditTheft.SetCheck(thePrefs.GetAntiCreditTheft());
	m_iAntiFriendshare.SetCheck(thePrefs.GetAntiFriendshare());
	// [TPT] - eWombat SNAFU v2

	// [TPT] - Fakecheck
	CString buffer;
	buffer.Format(_T("%s"), thePrefs.GetUpdateURLFakeList());
	m_fakeURL.SetWindowText(buffer);

	m_fakeStartup.SetCheck(thePrefs.IsUpdateFakeStartupEnabled());
	buffer.Format(_T("v.%u"), thePrefs.GetFakesDatVersion());
	m_fakeVersion.SetWindowText(buffer);

	// [TPT] - Fakecheck end

	//LOG FILTERS --------------------------------------------------------
	// Maella -Filter verbose messages-
	m_filterVerboseCheck01.SetCheck((thePrefs.GetBlockUploadEndMsg() == true) ? BST_CHECKED : BST_UNCHECKED);
	m_filterVerboseCheck01.EnableWindow((thePrefs.GetVerbose() == false) ? FALSE : TRUE);	
	m_filterVerboseCheck02.SetCheck((thePrefs.GetBlockMaellaSpecificMsg() == true) ? BST_CHECKED : BST_UNCHECKED);
	m_filterVerboseCheck02.EnableWindow((thePrefs.GetVerbose() == false) ? FALSE : TRUE);
	m_filterVerboseCheck03.SetCheck((thePrefs.GetBlockMaellaSpecificDebugMsg() == true) ? BST_CHECKED : BST_UNCHECKED);
	m_filterVerboseCheck03.EnableWindow((thePrefs.GetVerbose() == false) ? FALSE : TRUE);
	// [TPT] - Filter own messages
	m_filterVerboseCheck04.SetCheck((thePrefs.GetBlockPhoenixMsg() == true) ? BST_CHECKED : BST_UNCHECKED);
	m_filterVerboseCheck04.EnableWindow((thePrefs.GetVerbose() == false) ? FALSE : TRUE);
	// [TPT] - Filter Handshake messages
	m_filterVerboseCheck05.SetCheck((thePrefs.GetBlockHandshakeMsg() == true) ? BST_CHECKED : BST_UNCHECKED);
	m_filterVerboseCheck05.EnableWindow((thePrefs.GetVerbose() == false) ? FALSE : TRUE);
	// [TPT] - Filter dead sources
	m_filterVerboseCheck06.SetCheck((thePrefs.GetBlockDeadSourcesMsg() == true) ? BST_CHECKED : BST_UNCHECKED);
	m_filterVerboseCheck06.EnableWindow((thePrefs.GetVerbose() == false) ? FALSE : TRUE);
	
	//BACKUP ---------------------------------------------------------------
	// [TPT] - TBH-Autobackup
	m_AutoBackup.SetCheck(thePrefs.GetAutoBackup());
	m_AutoBackup2.SetCheck(thePrefs.GetAutoBackup2());

	//----------------------------------------------------------------END-

}

BOOL CPPgPhoenix1::OnApply()
{	
	//--------------------------------------------------------------BEGIN-

	//CONEXION -----------------------------------------------------------

	// Maella -MTU Configuration-
	{
		CString buffer;
		m_MTUEdit.GetWindowText(buffer);
		int MTU = _tstoi(buffer);
		if(MTU > 1500) MTU = 1500;
		if(MTU < 250) MTU = 250;
		thePrefs.SetMTU(MTU);
		theApp.uploadBandwidthThrottler->SetMTUValue(MTU);
	}
	// Maella -New bandwidth control-
	{
		CString buffer;
		m_sndSocketSizeEdit.GetWindowText(buffer);
		int sendSocketBufferSize = _tstoi(buffer);
		if(sendSocketBufferSize > 8192) sendSocketBufferSize = 8192;
		if(sendSocketBufferSize < MINIMUM_SEND_BUFFER_SIZE) sendSocketBufferSize = MINIMUM_SEND_BUFFER_SIZE;
		thePrefs.SetSendSocketBufferSize(sendSocketBufferSize);
	}
	// [TPT] - quick start
	thePrefs.m_QuickStart = m_iQuickStart.GetCheck();
	if (m_iQuickStart.GetCheck() == BST_CHECKED)
	{
		CString buffer;
		m_iMaxConn.GetWindowText(buffer);
		int maxConn = _tstoi(buffer);
		thePrefs.SetQuickStartMaxCon(maxConn);
		m_iMaxConnPerFive.GetWindowText(buffer);
		maxConn = _tstoi(buffer);
		thePrefs.SetQuickStartMaxConPerFive(maxConn);
	}
	// [TPT] - quick start
	
	thePrefs.SetManageConnection(m_iManageConnection.GetCheck() != BST_UNCHECKED); // [TPT] - Manage Connection

	// [TPT] - MoNKi: -UPnPNAT Support-
	thePrefs.SetUPnPNat(m_iUPnPNat.GetCheck() != BST_UNCHECKED);
	thePrefs.SetUPnPNatWeb(m_iUPnPNatWeb.GetCheck() != BST_UNCHECKED);
	// [TPT] - MoNKi: -UPnPNAT Support-


	//SUBIDAS ------------------------------------------------------------
	// Maella -Network Adapter Feedback Control-
	thePrefs.SetNAFCEnable(m_NAFCCheck01.GetCheck() == BST_CHECKED);
	thePrefs.SetNAFCFullControl(m_NAFCCheck02.GetCheck() == BST_CHECKED);	
	//thePrefs.SetNAFCNetworkOut(thePrefs.maxupload);
	theApp.uploadBandwidthThrottler->SetNAFCEnable((m_NAFCCheck01.GetCheck() == BST_CHECKED));

	// Maella -Minimum Upload Slot-
	{
		CString buffer;
		m_minSlotEdit.GetWindowText(buffer);
		int minUploadSlot = _tstoi(buffer);
		if(minUploadSlot > 4) minUploadSlot = 4;
		if(minUploadSlot < 2) minUploadSlot = 2;
		thePrefs.SetMinUploadSlot(minUploadSlot);
	}
	//new upload slot sharping
    thePrefs.SetEnableNewUSS(m_newUploadSlotSharpingCheck.GetCheck() != BST_UNCHECKED);
	// [TPT] - Cumulate Bandwidth
	thePrefs.SetCumulateBandwidth(m_iCumulateBW.GetCheck() != BST_UNCHECKED); 
	theApp.uploadBandwidthThrottler->SetCumulateBandwidth(m_iCumulateBW.GetCheck() != BST_UNCHECKED);
	// [TPT] - Cumulate Bandwidth
	thePrefs.SetMinimizeNumberOfSlots(m_iMinimizeSlots.GetCheck() != BST_UNCHECKED); // [TPT] - Pawcio: MUS
	thePrefs.SetUnlimitedUp(m_unlimitedUP.GetCheck() != BST_UNCHECKED); 

	//ZZ UploadSpeedSense %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	thePrefs.SetDynUpEnabled(m_USSEnable.GetCheck() == BST_CHECKED);
	CString buffer;
	m_USSLowUpSpeed.GetWindowText(buffer);
	float minUpSpeed = _tstof(buffer);	
    thePrefs.minupload = minUpSpeed;


	m_USSPingTolerancePercentEdit.GetWindowText(buffer);
	int pingTolerance = _tstoi(buffer);
    thePrefs.m_iDynUpPingTolerance = pingTolerance;

	m_USSPingToleranceMSEdit.GetWindowText(buffer);
	int pingToleranceMS = _tstoi(buffer);
    thePrefs.m_iDynUpPingToleranceMilliseconds = pingToleranceMS;

    thePrefs.m_bDynUpUseMillisecondPingTolerance = ((m_USSPingToleranceMSCheck.GetCheck() == BST_CHECKED) || (m_USSPingTolerancePercentCheck.GetCheck() != BST_CHECKED));
   
	m_USSGoingUp.GetWindowText(buffer);
	int goingUp = _tstoi(buffer);
    thePrefs.m_iDynUpGoingUpDivider = goingUp;
	
	m_USSGoingDown.GetWindowText(buffer);
	int goingDown = _tstoi(buffer);
    thePrefs.m_iDynUpGoingDownDivider = goingDown;

	m_USSNumberOfPings.GetWindowText(buffer);
	int numberPings = _tstoi(buffer);
    thePrefs.m_iDynUpNumberOfPings = numberPings;	

	//SERVIDOR -----------------------------------------------------------
	// Maella -Enable/Disable source exchange in preference- (Tarod)
	thePrefs.SetDisabledXS(m_XSCheck.GetCheck() != BST_UNCHECKED);

	// Maella -Reask sources after IP change- (idea Xman)
	thePrefs.SetReaskSourceAfterIPChange(m_reaskSourceAfterIPChangeCheck.GetCheck() == BST_UNCHECKED); // [TPT]
	thePrefs.SetSaveUploadQueueWaitTime(m_iSUQWT.GetCheck() !=	BST_UNCHECKED); // [TPT] - SUQWT

	//SEGURIDAD ----------------------------------------------------------
	// [TPT] - eWombat SNAFU v2
	thePrefs.SetEnableSnafu(m_iSnafu.GetCheck() != BST_UNCHECKED);
	thePrefs.SetEnableACT(m_iAntiCreditTheft.GetCheck() != BST_UNCHECKED);
	thePrefs.SetEnableAntiFriendshare(m_iAntiFriendshare.GetCheck() != BST_UNCHECKED);
	// [TPT] - eWombat SNAFU v2

	// [TPT] - Fakecheck
	{
		CString buffer;
		m_fakeURL.GetWindowText(buffer);		
		_tcscpy(thePrefs.UpdateURLFakeList, buffer);
	}	
	thePrefs.UpdateFakeStartup = m_fakeStartup.GetCheck() != BST_UNCHECKED;
	// [TPT] - Fakecheck end


	//LOG FILTERS --------------------------------------------------------
	// Maella -Filter verbose messages-
	thePrefs.SetBlockUploadEndMsg(m_filterVerboseCheck01.GetCheck() != BST_UNCHECKED);			
	thePrefs.SetBlockMaellaSpecificMsg(m_filterVerboseCheck02.GetCheck() != BST_UNCHECKED);
	thePrefs.SetBlockMaellaSpecificDebugMsg(m_filterVerboseCheck03.GetCheck() != BST_UNCHECKED);
	thePrefs.SetBlockPhoenixMsg(m_filterVerboseCheck04.GetCheck() != BST_UNCHECKED); // [TPT] - Filter own messages
	thePrefs.SetBlockHandshakeMsg(m_filterVerboseCheck05.GetCheck() != BST_UNCHECKED); // [TPT] - Filter Handshake messages
	thePrefs.SetBlockDeadSourcesMsg(m_filterVerboseCheck06.GetCheck() != BST_UNCHECKED); // [TPT] - Filter dead sources
			
	//BACKUP --------------------------------------------------------
	// [TPT]- TBH-AutoBackup
	thePrefs.SetAutoBackup(m_AutoBackup.GetCheck() != BST_UNCHECKED);
	thePrefs.SetAutoBackup2(m_AutoBackup2.GetCheck() != BST_UNCHECKED); 


	//----------------------------------------------------------------END-

	theApp.scheduler->SaveOriginals(); // use the new settings as original

	// Refresh Setting
	LoadSettings();
	SetModified(FALSE);	

	return CPropertyPage::OnApply();
}

void CPPgPhoenix1::Localize(void)
{	
	if(m_hWnd)
	{
		// Create an icon list for the tab control
		m_imageList.DeleteImageList();
		m_imageList.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
		m_imageList.SetBkColor(CLR_NONE);
		m_imageList.Add(CTempIconLoader(_T("connection")));//Imagen Conexion
		m_imageList.Add(CTempIconLoader(_T("upload")));//Imagen Subidas
		m_imageList.Add(CTempIconLoader(_T("VIEWQUEUEONLY")));//Imagen Servidor
		m_imageList.Add(CTempIconLoader(_T("security")));//Imagen Seguridad
		m_imageList.Add(CTempIconLoader(_T("myinfo")));//Imagen Filtros		
		m_imageList.Add(CTempIconLoader(_T("HARDDISK")));//Imagen del backup // [TPT]- TBH-AutoBackup


		CString Buffer;
		SetWindowText(_T("pHoeniX One"));		

		int row = m_tabCtr.GetRowCount();
		InitTab(); // To update string, could be improved
		if(row != 0 && row != m_tabCtr.GetRowCount()){
			// Shift all windows object
			// .. to do
		}

	//CONEXION -----------------------------------------------------------
		m_MTUStatic.SetWindowText(GetResString(IDS_PPG_MAELLA_MTU_STATIC));
		Buffer.Format(GetResString(IDS_PPG_MAELLA_SND_SOCKET_SIZE_STATIC), MINIMUM_SEND_BUFFER_SIZE);
		m_sndSocketSizeStatic.SetWindowText(Buffer);
		// [TPT] - quick start		
		m_iQuickStart.SetWindowText(GetResString(IDS_ENABLEQUICKSTART)); 
		m_iMaxConnStatic.SetWindowText(GetResString(IDS_QUICK_START_MAX_CONN));
		m_iMaxConnPerFiveStatic.SetWindowText(GetResString(IDS_QUICK_START_MAX_CONN_PER_FIVE));
		m_QuickGroupBox.SetWindowText(GetResString(IDS_QUICK_START));
		// [TPT] - quick start
		
		m_iManageConnection.SetWindowText(GetResString(IDS_PPG_PHOENIX_MANAGECONNECTION)); // [TPT] - Manage Connection
	
		// [TPT] - MoNKi: -UPnPNAT Support-
		m_iUPnPNatGroupBox.SetWindowText(GetResString(IDS_PPG_PHOENIX_UPNP_NAT_GROUP_BOX));
		m_iUPnPNat.SetWindowText(GetResString(IDS_PPG_PHOENIX_UPNP_NAT));
		m_iUPnPNatWeb.SetWindowText(GetResString(IDS_PPG_PHOENIX_UPNP_NAT_WEB));
		// [TPT] - MoNKi: -UPnPNAT Support-
		
	//SUBIDAS ------------------------------------------------------------
		m_minSlotStatic.SetWindowText(GetResString(IDS_PPG_MAELLA_MIN_SLOT_STATIC));
		m_NAFCGroupBox.SetWindowText(GetResString(IDS_PPG_MAELLA_NAFC_STATIC01));
		m_NAFCStatic.SetWindowText(GetResString(IDS_PPG_MAELLA_NAFC_STATIC02));
		m_NAFCCheck01.SetWindowText(GetResString(IDS_PPG_MAELLA_NAFC_CHECK01));
		m_NAFCCheck02.SetWindowText(GetResString(IDS_PPG_MAELLA_NAFC_CHECK02));		
		m_iCumulateBW.SetWindowText(GetResString(IDS_PPG_PHOENIX_CUMULATEBW)); // [TPT] - Cumulate Bandwidth		
		m_newUploadSlotSharpingCheck.SetWindowText(GetResString(IDS_PPG_MAELLA_NUSS_CHECK01));
		m_uploadTweaks.SetWindowText(GetResString(IDS_UPLOADTWEAKSBOX));

		m_iMinimizeSlots.SetWindowText(GetResString(IDS_MINIMIZESLOTS));// [TPT] - MUS
		m_unlimitedUP.SetWindowText(GetResString(IDS_UNLIMITEDUP));

		//ZZ UploadSpeedSense %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		m_USSEnable.SetWindowText(GetResString(IDS_DYNUPENABLED));
		m_USSLowUpSpeedStatic.SetWindowText(GetResString(IDS_DYNUP_MINUPLOAD));
		m_USSPingTolerancePercentCheck.SetWindowText(GetResString(IDS_USSPERCENTAGE));
		m_USSPingTolerancePercentStatic.SetWindowText(GetResString(IDS_USSPERCENTAGE));
		m_USSPingToleranceMSCheck.SetWindowText(GetResString(IDS_USSTOLERANCEMS));
		m_USSPingToleranceMSStatic.SetWindowText(GetResString(IDS_USSTOLERANCEMS2));
		m_USSGoingDownStatic.SetWindowText(GetResString(IDS_DYNUP_GOINGDOWNDIVIDER));
		m_USSGoingUpStatic.SetWindowText(GetResString(IDS_DYNUP_GOINGUPDIVIDER));
		m_USSNumberOfPingsStatic.SetWindowText(GetResString(IDS_DYNUP_NUMBEROFPINGS));
		m_USSpingToleranceBox.SetWindowText(GetResString(IDS_USSTOLERANCE));
		
		m_changeUpViews.SetWindowText(GetResString(IDS_VIEWUSS));
		
	

	//FUENTES -----------------------------------------------------------
		m_XSCheck.SetWindowText(GetResString(IDS_PPG_MAELLA_XS_CHECK));
		m_reaskSourceAfterIPChangeCheck.SetWindowText(GetResString(IDS_PPG_MAELLA_REASK_SOURCE_AFTER_IP_CHANGE_CHECK));
		m_iSUQWT.SetWindowText(GetResString(IDS_SUQWT)); // [TPT] - SUQWT
		infoSUQWT.SetWindowText(GetResString(IDS_SUQWTINFO));

	//SEGURIDAD ----------------------------------------------------------		
		m_userHashStatic.SetWindowText(GetResString(IDS_CD_UHASH));
		m_userHashEdit.SetWindowText(EncodeBase16((const unsigned char*)thePrefs.GetUserHash(), 16));

		// [TPT] - eWombat SNAFU v2
		m_SnafuGroupBox.SetWindowText(GetResString(IDS_PPG_SNAFUGROUP));
		m_iSnafu.SetWindowText(GetResString(IDS_PPG_PHOENIX_SNAFU));
		m_iAntiCreditTheft.SetWindowText(GetResString(IDS_PPG_PHOENIX_ACT));
		m_iAntiFriendshare.SetWindowText(GetResString(IDS_PPG_PHOENIX_FRIENDSHARE));
		// [TPT] - eWombat SNAFU v2

		// [TPT] - Fakecheck
		m_fakeBox.SetWindowText(_T("Fakes"));
		m_fakeStartup.SetWindowText(GetResString(IDS_UPDATEFAKECHECKONSTART));
		m_fakeUpdate.SetWindowText(GetResString(IDS_UPDATEFAKES));

	//LOG FILTERS --------------------------------------------------------
		m_filterVerboseGroupbox.SetWindowText(GetResString(IDS_PPG_MAELLA_FILTER_STATIC) + GetResString(IDS_VERBOSE));
		m_filterVerboseCheck01.SetWindowText(GetResString(IDS_PPG_MAELLA_FILTER_CHECK01));		
		m_filterVerboseCheck02.SetWindowText(GetResString(IDS_PPG_MAELLA_FILTER_CHECK02));
		m_filterVerboseCheck03.SetWindowText(GetResString(IDS_PPG_MAELLA_FILTER_CHECK03));
		m_filterVerboseCheck04.SetWindowText(GetResString(IDS_PPG_MAELLA_FILTER_CHECK04)); // [TPT] - Filter own messages
		m_filterVerboseCheck05.SetWindowText(GetResString(IDS_PPG_MAELLA_FILTER_CHECK05)); // [TPT] - Filter Handshake messages
		m_filterVerboseCheck06.SetWindowText(GetResString(IDS_PPG_MAELLA_FILTER_CHECK06)); // [TPT] - Filter dead sources
						
	//BACKUP
		// [TPT] - TBH-AutoBackup

		m_backupDAT.SetWindowText(GetResString(IDS_BACKUPDAT));
		m_backupMET.SetWindowText(GetResString(IDS_BACKUPMET));
		m_backupINI.SetWindowText(GetResString(IDS_BACKUPINI));
		m_backupPARTMET.SetWindowText(GetResString(IDS_BACKUPPARTMET));
		m_backupPART.SetWindowText(GetResString(IDS_BACKUPPART));
		m_backupNow.SetWindowText(GetResString(IDS_BACKUPNOW));
		m_selectall.SetWindowText(GetResString(IDS_BACKUPSELECTALL));
		m_AutoBackup.SetWindowText(GetResString(IDS_BACKUPAUTO));
		m_AutoBackup2.SetWindowText(GetResString(IDS_BACKUPAUTO2));
		m_AutoBackupBox.SetWindowText(GetResString(IDS_BACKUPAUTOBOX));
		m_backupFilesBox.SetWindowText(GetResString(IDS_BACKUPFILESBOX));
		m_backupStatic.SetWindowText(GetResString(IDS_BACKUPSTATIC));
		


	}
	

}

void CPPgPhoenix1::InitTab(){
	// Clear all to be sure
	m_tabCtr.DeleteAllItems();
	
	// Change style
	// Remark: It seems that the multi-row can not be activated with the properties
	m_tabCtr.ModifyStyle(0, TCS_MULTILINE);

	// Add all items with icon (connection, tweak, etc...)
	m_tabCtr.SetImageList(&m_imageList);
	m_tabCtr.InsertItem(TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM, CONNECTION, GetResString(IDS_PW_CONNECTION), 0, (LPARAM)CONNECTION); 	
	m_tabCtr.InsertItem(TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM, UPLOAD, GetResString(IDS_TW_UPLOADS), 1, (LPARAM)UPLOAD); 	
	m_tabCtr.InsertItem(TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM, SOURCES, GetResString(IDS_DL_SOURCES), 2, (LPARAM)SOURCES); 		
	m_tabCtr.InsertItem(TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM, SECURITY, GetResString(IDS_SECURITY), 3, (LPARAM)SECURITY); 	
	m_tabCtr.InsertItem(TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM, LOGFILTER, GetResString(IDS_LOGFILTER), 4, (LPARAM)LOGFILTER); 				
	m_tabCtr.InsertItem(TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM, BACKUP, GetResString(IDS_BACKUPNAME), 5, (LPARAM)BACKUP); // [TPT]- TBH-AutoBackup
	
}


void CPPgPhoenix1::InitControl(){
	// Remark: don't use the dialog editor => avoid to merge rc

	// Retrieve the bottom of the tab's header
	RECT rect1;
	RECT rect2;
	m_tabCtr.GetWindowRect(&rect1);
	ScreenToClient(&rect1);
	m_tabCtr.GetItemRect(m_tabCtr.GetItemCount() - 1 , &rect2);
	const int top = rect1.top + (rect2.bottom - rect2.top + 1) * m_tabCtr.GetRowCount() + 10;
	const int right = rect1.left + 10;
	const int bottom = rect1.bottom;
	

	//CONEXION ----------------------------------------------------------
		// Maella -MTU Configuration-
		m_MTUEdit.CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), 
						WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
						ES_RIGHT | ES_AUTOHSCROLL | ES_NUMBER, 
						CRect(right, top+20, right+40, top+20+20), this, IDC_PPG_MAELLA_MTU_EDIT);
		m_MTUEdit.SetFont(GetFont());
		m_MTUEdit.SetLimitText(4);

		m_MTUStatic.CreateEx(0, _T("STATIC"), _T(""), 
							WS_CHILD /*| WS_VISIBLE*/, 
							CRect(right+45, top+20+3, right+45+260, top+20+3+20), this, IDC_PPG_MAELLA_MTU_STATIC);
		m_MTUStatic.SetFont(GetFont());

		// Maella -Send Socket Size-
		m_sndSocketSizeEdit.CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), 
							WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
							ES_RIGHT | ES_AUTOHSCROLL | ES_NUMBER, 
							CRect(right, top+50, right+40, top+50+20), this, IDC_PPG_MAELLA_SND_SOCKET_SIZE_EDIT);
		m_sndSocketSizeEdit.SetFont(GetFont());
		m_sndSocketSizeEdit.SetLimitText(4);

		m_sndSocketSizeStatic.CreateEx(0, _T("STATIC"), _T(""), 
								WS_CHILD /*| WS_VISIBLE*/, 
								CRect(right+45, top+50-3, right+45+260, top+50+3+20), this, IDC_PPG_MAELLA_SND_SOCKET_SIZE_STATIC);
		m_sndSocketSizeStatic.SetFont(GetFont());

		// [TPT] - quick start
		m_QuickGroupBox.CreateEx(0, _T("BUTTON"), _T(""), 
							   WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
							   BS_GROUPBOX,
							   CRect(right, top+90, right+300, top+200), this, IDC_QUICKBOX);
		m_QuickGroupBox.SetFont(GetFont());

		m_iQuickStart.CreateEx(0, _T("BUTTON"), _T(""), 
										WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
										BS_AUTOCHECKBOX, 
										CRect(right+10, top+110, right+295, top+110+20), this, IDC_PPG_PHOENIX_QUICKSTART);
		m_iQuickStart.SetFont(GetFont());

		m_iMaxConnPerFive.CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), 
							WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
							ES_RIGHT | ES_AUTOHSCROLL | ES_NUMBER, 
							CRect(right+10, top+140, right+50, top+140+20), this,  IDC_PPG_PHOENIX_QUICKSTART_CONN_PER_FIVE);
		m_iMaxConnPerFive.SetFont(GetFont());
		m_iMaxConnPerFive.SetLimitText(4);
		m_iMaxConnPerFiveStatic.CreateEx(0, _T("STATIC"), _T(""), 
								WS_CHILD /*| WS_VISIBLE*/, 
								CRect(right+55, top+142, right+55+230, top+142+20), this, IDC_PPG_PHOENIX_QUICKSTART_CONN_PER_FIVE_STATIC);
		m_iMaxConnPerFiveStatic.SetFont(GetFont());

		m_iMaxConn.CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), 
							WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
							ES_RIGHT | ES_AUTOHSCROLL | ES_NUMBER, 
							CRect(right+10, top+170, right+50, top+170+20), this,  IDC_PPG_PHOENIX_QUICKSTART_CONN);
		m_iMaxConn.SetFont(GetFont());
		m_iMaxConn.SetLimitText(4);
		m_iMaxConnStatic.CreateEx(0, _T("STATIC"), _T(""), 
								WS_CHILD /*| WS_VISIBLE*/, 
								CRect(right+55, top+172, right+55+230, top+172+20), this, IDC_PPG_PHOENIX_QUICKSTART_CONN_STATIC);
		m_iMaxConnStatic.SetFont(GetFont());
		// [TPT] - quick start

		// [TPT] - Manage Connection
		m_iManageConnection.CreateEx(0, _T("BUTTON"), _T(""), 
						                      WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
						                      BS_AUTOCHECKBOX, 
						                      CRect(right, top+205, right+300, top+205+20), this, IDC_PPG_PHOENIX_MANAGECONNECTION);
		m_iManageConnection.SetFont(GetFont());
		// [TPT] - Manage Connection

		// [TPT] - MoNKi: -UPnPNAT Support-
		m_iUPnPNatGroupBox.CreateEx(0, _T("BUTTON"), _T(""), 
							   WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
							   BS_GROUPBOX,
							   CRect(right, top+235, right+300, top+280), this, IDC_PPG_PHOENIX_UPNP_NAT_BOX);
		m_iUPnPNatGroupBox.SetFont(GetFont());

		m_iUPnPNat.CreateEx(0, _T("BUTTON"), _T(""), 
						                      WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
						                      BS_AUTOCHECKBOX, 
						                      CRect(right+10, top+250, right+130, top+250+20), this, IDC_PPG_PHOENIX_UPNP_NAT);
		m_iUPnPNat.SetFont(GetFont());
		m_iUPnPNatWeb.CreateEx(0, _T("BUTTON"), _T(""), 
						                      WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
						                      BS_AUTOCHECKBOX, 
						                      CRect(right+150, top+250, right+290, top+250+20), this, IDC_PPG_PHOENIX_UPNP_NAT_WEB);
		m_iUPnPNatWeb.SetFont(GetFont());
		// [TPT] - MoNKi: -UPnPNAT Support-
		
	//SUBIDAS ------------------------------------------------------------

		//NAFC
		m_NAFCGroupBox.CreateEx(0, _T("BUTTON"), _T(""), 
							   WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
							   BS_GROUPBOX,
							   CRect(right, top+20, right+300, top+20+75), this, IDC_PPG_MAELLA_NAFC_STATIC01);
		m_NAFCGroupBox.SetFont(GetFont());

		m_NAFCCheck01.CreateEx(0, _T("BUTTON"), _T(""), 
							  WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
							  BS_AUTOCHECKBOX, 
							  CRect(right+10, top+20+20, right+295, top+20+20+20), this, IDC_PPG_MAELLA_NAFC_CHECK01);
		m_NAFCCheck01.SetFont(GetFont());

		m_NAFCEdit.CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), 
							WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
							ES_RIGHT | ES_AUTOHSCROLL, 
  							CRect(right+10, top+20+3+40, right+10+45, top+20+3+40+20), this, IDC_PPG_MAELLA_NAFC_EDIT);
		m_NAFCEdit.SetFont(GetFont());
		m_NAFCEdit.SetLimitText(5);

		m_NAFCStatic.CreateEx(0, _T("STATIC"), _T(""), 
							  WS_CHILD /*| WS_VISIBLE*/,  
							  CRect(right+10+40+10, top+20+6+40, right+175, top+20+6+40+20), this, IDC_PPG_MAELLA_NAFC_STATIC02);
		m_NAFCStatic.SetFont(GetFont());

		m_NAFCCheck02.CreateEx(0, _T("BUTTON"), _T(""), 
							  WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
							  BS_AUTOCHECKBOX, 
							  CRect(right+10+175, top+20+3+40, right+295, top+20+3+40+20), this, IDC_PPG_MAELLA_NAFC_CHECK02);
		m_NAFCCheck02.SetFont(GetFont());
		// Maella -Minimum Upload Slot-
		m_minSlotEdit.CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), 
							WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
							ES_RIGHT | ES_AUTOHSCROLL | ES_NUMBER, 
							CRect(right, top+115, right+40, top+115+20), this, IDC_PPG_MAELLA_MIN_SLOT_EDIT);
		m_minSlotEdit.SetFont(GetFont());
		m_minSlotEdit.SetLimitText(1);

		m_minSlotStatic.CreateEx(0, _T("STATIC"), _T(""), 
								WS_CHILD /*| WS_VISIBLE*/, 
								CRect(right+45, top+115+3, right+45+260, top+115+3+20), this, IDC_PPG_MAELLA_MIN_SLOT_STATIC);
		m_minSlotStatic.SetFont(GetFont());

		
		m_uploadTweaks.CreateEx(0, _T("BUTTON"), _T(""), 
							   WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
							   BS_GROUPBOX,
							   CRect(right, top+145, right+300, top+235), this, IDC_UPLOADTWEAKS);
		m_uploadTweaks.SetFont(GetFont());

		//Maella New Upload Slot Sharping
		m_newUploadSlotSharpingCheck.CreateEx(0, _T("BUTTON"), _T(""), 
						                      WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
						                      BS_AUTOCHECKBOX, 
						                      CRect(right+10, top+145+20, right+295, top+145+40), this, IDC_PPG_MAELLA_NUSS_CHECK01);
		m_newUploadSlotSharpingCheck.SetFont(GetFont());
		
		// [TPT] - Cumulate Bandwidth
		m_iCumulateBW.CreateEx(0, _T("BUTTON"), _T(""), 
						                      WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
						                      BS_AUTOCHECKBOX, 
						                      CRect(right+10, top+165+20, right+295, top+165+40), this, IDC_PPG_PHOENIX_CUMULATEBW);
		m_iCumulateBW.SetFont(GetFont());

		// [TPT] - Pawcio: MUS
		m_iMinimizeSlots.CreateEx(0, _T("BUTTON"), _T(""), 
						                      WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
						                      BS_AUTOCHECKBOX, 
						                      CRect(right+10, top+185+20, right+295, top+185+40), this, IDC_PPG_PHOENIX_MINIMIZESLOTS);
		m_iMinimizeSlots.SetFont(GetFont());
		
		m_unlimitedUP.CreateEx(0, _T("BUTTON"), _T(""), 
						                      WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
						                      BS_AUTOCHECKBOX, 
						                      CRect(right, top+245, right+300, top+265), this, IDC_PPG_PHOENIX_UNLIMITEDSPEED);
		m_unlimitedUP.SetFont(GetFont());

		//ZZ: Upload Speed Sense
		m_USSEnable.CreateEx(0, _T("BUTTON"), _T(""), 
						                      WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
						                      BS_AUTOCHECKBOX, 
						                      CRect(right, top+20, right+300, top+40), this, IDC_PPG_PHOENIX_USSENABLE);
		m_USSEnable.SetFont(GetFont());
		m_USSLowUpSpeed.CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), 
							WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
							ES_RIGHT | ES_AUTOHSCROLL | ES_NUMBER, 
							CRect(right, top+50, right+40, top+70), this, IDC_PPG_PHOENIX_USSLOWSPEED);
		m_USSLowUpSpeed.SetFont(GetFont());

		m_USSLowUpSpeedStatic.CreateEx(0, _T("STATIC"), _T(""), 
								WS_CHILD /*| WS_VISIBLE*/, 
								CRect(right+45, top+53, right+45+265, top+73), this, IDC_PPG_PHOENIX_USSLOWSPEEDSTATIC);
		m_USSLowUpSpeedStatic.SetFont(GetFont());

		m_USSpingToleranceBox.CreateEx(0, _T("BUTTON"), _T(""), 
							   WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
							   BS_GROUPBOX,
							   CRect(right, top+80, right+300, top+220), this, IDC_PPG_PHOENIX_TOLERANCEBOX);
		m_USSpingToleranceBox.SetFont(GetFont());

		m_USSPingTolerancePercentCheck.CreateEx(0, _T("BUTTON"), _T(""), 
						                      WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
						                      BS_AUTOCHECKBOX, 
						                      CRect(right+10, top+100, right+295, top+120), this, IDC_PPG_PHOENIX_USSTOLERANCEPERCENTCHECK);
		m_USSPingTolerancePercentCheck.SetFont(GetFont());

		m_USSPingTolerancePercentEdit.CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), 
							WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
							ES_RIGHT | ES_AUTOHSCROLL | ES_NUMBER, 
							CRect(right+20, top+130, right+60, top+150), this, IDC_PPG_PHOENIX_USSTOLERANCEPERCENTEDIT);
		m_USSPingTolerancePercentEdit.SetFont(GetFont());

		m_USSPingTolerancePercentStatic.CreateEx(0, _T("STATIC"), _T(""), 
								WS_CHILD /*| WS_VISIBLE*/, 
								CRect(right+65, top+133, right+295, top+153), this, IDC_PPG_PHOENIX_USSTOLERANCEPERCENTEDITSTATIC);
		m_USSPingTolerancePercentStatic.SetFont(GetFont());


		m_USSPingToleranceMSCheck.CreateEx(0, _T("BUTTON"), _T(""), 
						                      WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
						                      BS_AUTOCHECKBOX, 
						                      CRect(right+10, top+160, right+295, top+180), this, IDC_PPG_PHOENIX_USSTOLERANCEMSCHECK);
		m_USSPingToleranceMSCheck.SetFont(GetFont());

		m_USSPingToleranceMSEdit.CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), 
							WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
							ES_RIGHT | ES_AUTOHSCROLL | ES_NUMBER, 
							CRect(right+20, top+190, right+60, top+210), this, IDC_PPG_PHOENIX_USSTOLERANCEMSEDIT);
		m_USSPingToleranceMSEdit.SetFont(GetFont());

		m_USSPingToleranceMSStatic.CreateEx(0, _T("STATIC"), _T(""), 
								WS_CHILD /*| WS_VISIBLE*/, 
								CRect(right+65, top+193, right+295, top+213), this, IDC_PPG_PHOENIX_USSTOLERANCEMSEDITSTATIC);
		m_USSPingToleranceMSStatic.SetFont(GetFont());


		m_USSGoingDown.CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), 
							WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
							ES_RIGHT | ES_AUTOHSCROLL | ES_NUMBER, 
							CRect(right, top+230, right+40, top+250), this, IDC_PPG_PHOENIX_USSGOINGDOWN);
		m_USSGoingDown.SetFont(GetFont());

		m_USSGoingDownStatic.CreateEx(0, _T("STATIC"), _T(""), 
								WS_CHILD /*| WS_VISIBLE*/, 
								CRect(right+45, top+233, right+45+260, top+253), this, IDC_PPG_PHOENIX_USSGOINGDOWNSTATIC);
		m_USSGoingDownStatic.SetFont(GetFont());


		m_USSGoingUp.CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), 
							WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
							ES_RIGHT | ES_AUTOHSCROLL | ES_NUMBER, 
							CRect(right, top+260, right+40, top+280), this, IDC_PPG_PHOENIX_USSGOINGUP);
		m_USSGoingUp.SetFont(GetFont());

		m_USSGoingUpStatic.CreateEx(0, _T("STATIC"), _T(""), 
								WS_CHILD /*| WS_VISIBLE*/, 
								CRect(right+45, top+263, right+45+260, top+283), this, IDC_PPG_PHOENIX_USSGOINGUPSTATIC);
		m_USSGoingUpStatic.SetFont(GetFont());

		m_USSNumberOfPings.CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), 
							WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
							ES_RIGHT | ES_AUTOHSCROLL | ES_NUMBER, 
							CRect(right, top+290, right+40, top+310), this, IDC_PPG_PHOENIX_USSNUMBEROFPINGS);
		m_USSNumberOfPings.SetFont(GetFont());

		m_USSNumberOfPingsStatic.CreateEx(0, _T("STATIC"), _T(""), 
								WS_CHILD /*| WS_VISIBLE*/, 
								CRect(right+45, top+293, right+45+260, top+313), this, IDC_PPG_PHOENIX_USSNUMBEROFPINGSSTATIC);
		m_USSNumberOfPingsStatic.SetFont(GetFont());


		m_changeUpViews.CreateEx(0, _T("BUTTON"), _T(""), 
										WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
										BS_FLAT,
										CRect(right+5, bottom-40, right+295, bottom-20), this, IDC_PPG_PHOENIX_CHANGEUPVIEWS);
		m_changeUpViews.SetFont(GetFont());

	//SERVIDOR -----------------------------------------------------------
		// Maella -Enable/Disable source exchange in preference- (Tarod)
		m_XSCheck.CreateEx(0, _T("BUTTON"), _T(""), 
						WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
						BS_AUTOCHECKBOX, 
						CRect(right, top+40, right+300, top+40+20), this, IDC_PPG_MAELLA_XS_CHECK);
		m_XSCheck.SetFont(GetFont());

		m_reaskSourceAfterIPChangeCheck.CreateEx(0, _T("BUTTON"), _T(""), 
												 WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
												 BS_AUTOCHECKBOX, 
												 CRect(right, top+40+20, right+300, top+40+20+20), this, IDC_PPG_MAELLA_REASK_SOURCE_AFTER_IP_CHANGE_CHECK);
		m_reaskSourceAfterIPChangeCheck.SetFont(GetFont());

		// [TPT] - SUQWT
		m_iSUQWT.CreateEx(0, _T("BUTTON"), _T(""), 
					       WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
					       BS_AUTOCHECKBOX, 
						   CRect(right, top+80, right+300, top+80+20), this, IDC_PPG_PHOENIX_SUQWT);
		m_iSUQWT.SetFont(GetFont());

		infoSUQWT.CreateEx(0, _T("STATIC"), _T(""), 
										 WS_CHILD /*| WS_VISIBLE*/,  
										 CRect(right, top+105, right+300, top+155), this, IDC_PPG_PHOENIX_SUQWTINFO);
		infoSUQWT.SetFont(GetFont());


	//SEGURIDAD ---------------------------------------------------------
		// [TPT] - eWombat SNAFU v2
		m_SnafuGroupBox.CreateEx(0, _T("BUTTON"), _T(""), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_GROUPBOX,
									CRect(right, top+20, right+300, top+20+90), this, IDC_PPG_SNAFU);
		m_SnafuGroupBox.SetFont(GetFont());

		m_iSnafu.CreateEx(0, _T("BUTTON"), _T(""), 
									 WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									 BS_AUTOCHECKBOX, 
									 CRect(right+10, top+20+20, right+295, top+20+20+20), this, IDC_PPG_PHOENIX_SNAFU_CHECK);
		m_iSnafu.SetFont(GetFont());
		m_iAntiCreditTheft.CreateEx(0, _T("BUTTON"), _T(""), 
									 WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									 BS_AUTOCHECKBOX, 
									 CRect(right+10, top+20+40, right+295, top+20+40+20), this, IDC_PPG_PHOENIX_ACT_CHECK);
		m_iAntiCreditTheft.SetFont(GetFont());
		m_iAntiFriendshare.CreateEx(0, _T("BUTTON"), _T(""), 
									 WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									 BS_AUTOCHECKBOX, 
									 CRect(right+10, top+20+60, right+295, top+20+60+20), this, IDC_PPG_PHOENIX_FRIENDSHARE_CHECK);
		m_iAntiFriendshare.SetFont(GetFont());
		// [TPT] - eWombat SNAFU v2 END

		// [TPT] - Fakecheck
		m_fakeBox.CreateEx(0, _T("BUTTON"), _T(""), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_GROUPBOX,
									CRect(right, top+130, right+300, top+225), this, IDC_PPG_FAKEBOX);
		m_fakeBox.SetFont(GetFont());
		m_fakeStartup.CreateEx(0, _T("BUTTON"), _T(""), 
									 WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									 BS_AUTOCHECKBOX, 
									 CRect(right+10, top+150, right+295, top+170), this, IDC_PPG_FAKESTARTUP);
		m_fakeStartup.SetFont(GetFont());

		m_fakeURL.CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), 
							WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
							ES_LEFT | ES_AUTOHSCROLL, 
							CRect(right+10, top+170, right+295, top+190), this, IDC_PPG_FAKEURL);
		m_fakeURL.SetFont(GetFont());
		m_fakeUpdate.CreateEx(0, _T("BUTTON"), _T(""), 
										WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
										BS_FLAT,
										CRect(right+105, top+195, right+295, top+215), this, IDC_PPG_FAKEUP);
		m_fakeUpdate.SetFont(GetFont());

		m_fakeVersion.CreateEx(0, _T("STATIC"), _T(""), 
								WS_CHILD /*| WS_VISIBLE*/, 
								CRect(right+10, top+195, right+100, top+215), this, IDC_PPG_FAKEVERSION);
		m_fakeVersion.SetFont(GetFont());


		m_userHashStatic.CreateEx(0, _T("STATIC"), _T(""), 
								  WS_CHILD /*| WS_VISIBLE*/,
								  CRect(right, bottom-40, right+80, bottom-5), this, IDC_PPG_MAELLA_USERHASH_STATIC);
		m_userHashStatic.SetFont(GetFont());

		m_userHashEdit.CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), 
								WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
								ES_LEFT | ES_AUTOHSCROLL | ES_READONLY , 
								CRect(right+85, bottom-40, right+295, bottom-20), this, IDC_PPG_MAELLA_USERHASH_EDIT);
		m_userHashEdit.SetFont(GetFont());
	//LOG FILTERS --------------------------------------------------------
		// Maella -Filter verbose messages-
		m_filterVerboseGroupbox.CreateEx(0, _T("BUTTON"), _T(""), 
										WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
										BS_GROUPBOX,
										CRect(right, top+20, right+300, top+20+160), this, IDC_PPG_MAELLA_FILTER_STATIC);
		m_filterVerboseGroupbox.SetFont(GetFont());

		m_filterVerboseCheck01.CreateEx(0, _T("BUTTON"), _T(""), 
										WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
										BS_AUTOCHECKBOX, 
										CRect(right+10, top+20+20, right+295, top+20+40), this, IDC_PPG_MAELLA_FILTER_VERBOSE_CHECK01);
		m_filterVerboseCheck01.SetFont(GetFont());
		
		m_filterVerboseCheck02.CreateEx(0, _T("BUTTON"), _T(""), 
										WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
										BS_AUTOCHECKBOX, 
										CRect(right+10, top+20+40, right+295, top+20+40+20), this, IDC_PPG_MAELLA_FILTER_VERBOSE_CHECK07);
		m_filterVerboseCheck02.SetFont(GetFont());

		m_filterVerboseCheck03.CreateEx(0, _T("BUTTON"), _T(""), 
										WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
										BS_AUTOCHECKBOX, 
										CRect(right+10, top+20+60, right+295, top+20+60+20), this, IDC_PPG_MAELLA_FILTER_VERBOSE_CHECK08);
		m_filterVerboseCheck03.SetFont(GetFont());

		// [TPT] - Filter own messages
		m_filterVerboseCheck04.CreateEx(0, _T("BUTTON"), _T(""), 
										WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
										BS_AUTOCHECKBOX, 
										CRect(right+10, top+20+80, right+295, top+20+80+20), this, IDC_PPG_MAELLA_FILTER_VERBOSE_CHECK09);
		m_filterVerboseCheck04.SetFont(GetFont());

		// [TPT] - Filter Handshake messages
		m_filterVerboseCheck05.CreateEx(0, _T("BUTTON"), _T(""), 
										WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
										BS_AUTOCHECKBOX, 
										CRect(right+10, top+20+100, right+295, top+20+100+20), this, IDC_PPG_MAELLA_FILTER_VERBOSE_CHECK10);
		m_filterVerboseCheck05.SetFont(GetFont());
		
		// [TPT] - Filter dead sources
		m_filterVerboseCheck06.CreateEx(0, _T("BUTTON"), _T(""), 
										WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
										BS_AUTOCHECKBOX, 
										CRect(right+10, top+20+120, right+295, top+20+120+20), this, IDC_PPG_MAELLA_FILTER_VERBOSE_CHECK02);
		m_filterVerboseCheck06.SetFont(GetFont());
		
		
		//BACKUP ------------------------------------------------------------------
		// [TPT]- TBH-AutoBackup

		m_backupFilesBox.CreateEx(0, _T("BUTTON"), _T(""), 
							   WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
							   BS_GROUPBOX,
							   CRect(right, top+20, right+300, top+160), this, IDC_BACKUPBOX);
		m_backupFilesBox.SetFont(GetFont());

		m_backupDAT.CreateEx(0, _T("BUTTON"), _T(""), 
										WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
										BS_AUTOCHECKBOX, 
										CRect(right+10, top+40, right+140, top+60), this, IDC_DAT);
		m_backupDAT.SetFont(GetFont());		
		m_backupMET.CreateEx(0, _T("BUTTON"), _T(""), 
										WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
										BS_AUTOCHECKBOX, 
										CRect(right+10, top+60, right+140, top+80), this, IDC_MET);
		m_backupMET.SetFont(GetFont());	
		m_backupINI.CreateEx(0, _T("BUTTON"), _T(""), 
										WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
										BS_AUTOCHECKBOX, 
										CRect(right+10, top+80, right+140, top+100), this, IDC_INI);
		m_backupINI.SetFont(GetFont());	
		m_backupPARTMET.CreateEx(0, _T("BUTTON"), _T(""), 
										WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
										BS_AUTOCHECKBOX, 
										CRect(right+10, top+100, right+140, top+120), this, IDC_PARTMET);
		m_backupPARTMET.SetFont(GetFont());					
		m_backupPART.CreateEx(0, _T("BUTTON"), _T(""), 
										WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
										BS_AUTOCHECKBOX, 
										CRect(right+10, top+120, right+140, top+140), this, IDC_PART);
		m_backupPART.SetFont(GetFont());


		m_backupNow.CreateEx(0, _T("BUTTON"), _T(""), 
										WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
										BS_FLAT,
										CRect(right+160, top+100, right+290, top+120), this, IDC_BACKUPNOW);
		m_backupNow.SetFont(GetFont());
		m_selectall.CreateEx(0, _T("BUTTON"), _T(""), 
										WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
										BS_FLAT,
										CRect(right+160, top+20+80+20, right+290, top+140), this, IDC_SELECTALL);
		m_selectall.SetFont(GetFont());


		m_AutoBackupBox.CreateEx(0, _T("BUTTON"), _T(""), 
										WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
										BS_GROUPBOX,
										CRect(right, top+180, right+200, top+250), this, IDC_AUTOBACKUPBOX);
		m_AutoBackupBox.SetFont(GetFont());
		m_AutoBackup.CreateEx(0, _T("BUTTON"), _T(""), 
										WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
										BS_AUTOCHECKBOX, 
										CRect(right+10, top+200, right+190, top+220), this, IDC_AUTOBACKUP);
		m_AutoBackup.SetFont(GetFont());
		m_AutoBackup2.CreateEx(0, _T("BUTTON"), _T(""), 
										WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
										BS_AUTOCHECKBOX, 
										CRect(right+30, top+220, right+190, top+240), this, IDC_AUTOBACKUP2);
		m_AutoBackup2.SetFont(GetFont());
		m_backupStaticBox.CreateEx(0, _T("BUTTON"), _T(""), 
										WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
										BS_GROUPBOX,
										CRect(right, top+270, right+300, top+325), this, IDC_BACKUPSTATICBOX);
		m_backupStaticBox.SetFont(GetFont());
		m_backupStatic.CreateEx(0, _T("STATIC"), _T(""), 
										 WS_CHILD /*| WS_VISIBLE*/,  
										 CRect(right+20, top+280, right+270, top+320), this, IDC_BACKUPSTATIC);
		m_backupStatic.SetFont(GetFont());
		// [TPT]- TBH-AutoBackup
}

void CPPgPhoenix1::OnTabSelectionChange(NMHDR *pNMHDR, LRESULT *pResult)
{
	// Retrieve tab to display
	TCITEM tabCtrlItem; 
	tabCtrlItem.mask = TCIF_PARAM;
	if(m_tabCtr.GetItem(m_tabCtr.GetCurSel(), &tabCtrlItem) == TRUE){
		SetTab(static_cast<eTab>(tabCtrlItem.lParam));
	}

	*pResult = 0;
}


void CPPgPhoenix1::SetTab(eTab tab){
	if(m_currentTab != tab){
		// Hide all control
		switch(m_currentTab){
			case CONNECTION:
				m_MTUEdit.ShowWindow(SW_HIDE);
				m_MTUEdit.EnableWindow(FALSE);
				m_MTUStatic.ShowWindow(SW_HIDE);
				m_MTUStatic.EnableWindow(FALSE);
				m_sndSocketSizeEdit.ShowWindow(SW_HIDE);
				m_sndSocketSizeEdit.EnableWindow(FALSE);
				m_sndSocketSizeStatic.ShowWindow(SW_HIDE);
				m_sndSocketSizeStatic.EnableWindow(FALSE);
				// [TPT] - quick start
				m_iQuickStart.ShowWindow(SW_HIDE);
				m_iQuickStart.EnableWindow(FALSE);
				m_iMaxConnPerFive.ShowWindow(SW_HIDE);
				m_iMaxConnPerFive.EnableWindow(FALSE);
				m_iMaxConnPerFiveStatic.ShowWindow(SW_HIDE);
				m_iMaxConnPerFiveStatic.EnableWindow(FALSE);
				m_iMaxConn.ShowWindow(SW_HIDE);
				m_iMaxConn.EnableWindow(FALSE);
				m_iMaxConnStatic.ShowWindow(SW_HIDE);
				m_iMaxConnStatic.EnableWindow(FALSE);
				m_QuickGroupBox.ShowWindow(SW_HIDE);
				m_QuickGroupBox.EnableWindow(FALSE);				
				// [TPT] - Manage Connection
				m_iManageConnection.ShowWindow(SW_HIDE);
				m_iManageConnection.EnableWindow(FALSE);
				// [TPT] - MoNKi: -UPnPNAT Support-
				m_iUPnPNatGroupBox.ShowWindow(SW_HIDE);
				m_iUPnPNatGroupBox.EnableWindow(FALSE);
				m_iUPnPNat.ShowWindow(SW_HIDE);
				m_iUPnPNat.EnableWindow(FALSE);
				m_iUPnPNatWeb.ShowWindow(SW_HIDE);
				m_iUPnPNatWeb.EnableWindow(FALSE);
				break;		
			case UPLOAD:
				ShowNafcWindow(false);
				EnableNafcWindow(false);
				EnableUSSWindow(false);
				ShowUSSWindow(false);
				m_changeUpViews.ShowWindow(SW_HIDE);
				m_changeUpViews.EnableWindow(FALSE);
				break;

			case SOURCES:
				m_XSCheck.ShowWindow(SW_HIDE);
				m_XSCheck.EnableWindow(FALSE);

				m_reaskSourceAfterIPChangeCheck.ShowWindow(SW_HIDE);
				m_reaskSourceAfterIPChangeCheck.EnableWindow(FALSE);
				m_iSUQWT.ShowWindow(SW_HIDE);
				m_iSUQWT.EnableWindow(FALSE);
				infoSUQWT.ShowWindow(SW_HIDE);
				infoSUQWT.EnableWindow(FALSE);
				break;
			case SECURITY:
				// [TPT] - eWombat SNAFU v2
				m_SnafuGroupBox.ShowWindow(SW_HIDE);
				m_SnafuGroupBox.EnableWindow(FALSE);
				m_iSnafu.ShowWindow(SW_HIDE);
				m_iSnafu.EnableWindow(FALSE);
				m_iAntiCreditTheft.ShowWindow(SW_HIDE);
				m_iAntiCreditTheft.EnableWindow(FALSE);	
				m_iAntiFriendshare.ShowWindow(SW_HIDE);
				m_iAntiFriendshare.EnableWindow(FALSE);
				// [TPT] - eWombat SNAFU v2
				m_userHashStatic.ShowWindow(SW_HIDE);
				m_userHashStatic.EnableWindow(FALSE);
				m_userHashEdit.ShowWindow(SW_HIDE);
				m_userHashEdit.EnableWindow(FALSE);
				// [TPT] - Fakecheck
				m_fakeBox.ShowWindow(SW_HIDE);
				m_fakeBox.EnableWindow(FALSE);
				m_fakeStartup.ShowWindow(SW_HIDE);
				m_fakeStartup.EnableWindow(FALSE);
				m_fakeUpdate.ShowWindow(SW_HIDE);
				m_fakeUpdate.EnableWindow(FALSE);
				m_fakeURL.ShowWindow(SW_HIDE);
				m_fakeURL.EnableWindow(FALSE);
				m_fakeVersion.ShowWindow(SW_HIDE);
				m_fakeVersion.EnableWindow(FALSE);
				break;
			case LOGFILTER:
				m_filterVerboseGroupbox.ShowWindow(SW_HIDE);
				m_filterVerboseGroupbox.EnableWindow(FALSE);
				m_filterVerboseCheck01.ShowWindow(SW_HIDE);
				m_filterVerboseCheck01.EnableWindow(FALSE);												
				m_filterVerboseCheck02.ShowWindow(SW_HIDE);
				m_filterVerboseCheck02.EnableWindow(FALSE);
				m_filterVerboseCheck03.ShowWindow(SW_HIDE);
				m_filterVerboseCheck03.EnableWindow(FALSE);
				// [TPT] - Filter own messages
				m_filterVerboseCheck04.ShowWindow(SW_HIDE);
				m_filterVerboseCheck04.EnableWindow(FALSE);
				// [TPT] - Filter Handshake messages
				m_filterVerboseCheck05.ShowWindow(SW_HIDE);
				m_filterVerboseCheck05.EnableWindow(FALSE);				
				// [TPT] - Filter dead sources
				m_filterVerboseCheck06.ShowWindow(SW_HIDE);
				m_filterVerboseCheck06.EnableWindow(FALSE);								
				break;
			case BACKUP:
				// [TPT]- TBH-AutoBackup
				m_backupDAT.ShowWindow(SW_HIDE);
				m_backupDAT.EnableWindow(FALSE);
				m_backupMET.ShowWindow(SW_HIDE);
				m_backupMET.EnableWindow(FALSE);
				m_backupINI.ShowWindow(SW_HIDE);
				m_backupINI.EnableWindow(FALSE);
				m_backupPARTMET.ShowWindow(SW_HIDE);
				m_backupPARTMET.EnableWindow(FALSE);
				m_backupPART.ShowWindow(SW_HIDE);
				m_backupPART.EnableWindow(FALSE);
				m_backupNow.ShowWindow(SW_HIDE);
				m_backupNow.EnableWindow(FALSE);
				m_selectall.ShowWindow(SW_HIDE);
				m_selectall.EnableWindow(FALSE);
				m_AutoBackup.ShowWindow(SW_HIDE);
				m_AutoBackup.EnableWindow(FALSE);
				m_AutoBackup2.ShowWindow(SW_HIDE);
				m_AutoBackup2.EnableWindow(FALSE);				
				m_AutoBackupBox.ShowWindow(SW_HIDE);
				m_AutoBackupBox.EnableWindow(FALSE);				
				m_backupFilesBox.ShowWindow(SW_HIDE);
				m_backupFilesBox.EnableWindow(FALSE);
				m_backupStatic.ShowWindow(SW_HIDE);
				m_backupStatic.EnableWindow(FALSE);
				m_backupStaticBox.ShowWindow(SW_HIDE);
				m_backupStaticBox.EnableWindow(FALSE);
				// [TPT]- TBH-AutoBackup
				break;
		}

		// Show new controls
		m_currentTab = tab;
		switch(m_currentTab){
			case CONNECTION:

				m_MTUEdit.ShowWindow(SW_SHOW);
				m_MTUEdit.EnableWindow(TRUE);
				m_MTUStatic.ShowWindow(SW_SHOW);
				m_MTUStatic.EnableWindow(TRUE);
				m_sndSocketSizeEdit.ShowWindow(SW_SHOW);
				m_sndSocketSizeEdit.EnableWindow(TRUE);
				m_sndSocketSizeStatic.ShowWindow(SW_SHOW);
				m_sndSocketSizeStatic.EnableWindow(TRUE);
				// [TPT] - quick start
				m_iQuickStart.ShowWindow(SW_SHOW);
				m_iQuickStart.EnableWindow(TRUE);
				m_iMaxConnPerFive.ShowWindow(SW_SHOW);
				m_iMaxConnPerFive.EnableWindow(TRUE);
				m_iMaxConnPerFiveStatic.ShowWindow(SW_SHOW);
				m_iMaxConnPerFiveStatic.EnableWindow(TRUE);
				m_iMaxConn.ShowWindow(SW_SHOW);
				m_iMaxConn.EnableWindow(TRUE);
				m_iMaxConnStatic.ShowWindow(SW_SHOW);
				m_iMaxConnStatic.EnableWindow(TRUE);
				m_QuickGroupBox.ShowWindow(SW_SHOW);
				m_QuickGroupBox.EnableWindow(TRUE);				
				// [TPT] - Manage Connection
				m_iManageConnection.ShowWindow(SW_SHOW);
				m_iManageConnection.EnableWindow(TRUE);
				// [TPT] - MoNKi: -UPnPNAT Support-
				m_iUPnPNatGroupBox.ShowWindow(SW_SHOW);
				m_iUPnPNatGroupBox.EnableWindow(TRUE);
				m_iUPnPNat.ShowWindow(SW_SHOW);
				m_iUPnPNat.EnableWindow(TRUE);
				m_iUPnPNatWeb.ShowWindow(SW_SHOW);
				m_iUPnPNatWeb.EnableWindow(TRUE);
				break;			
			case UPLOAD:
				if(isWndNormalUpActive == true)
				{
				ShowNafcWindow(true);
				EnableNafcWindow(true);
				}
				else
				{
				EnableUSSWindow(true);
				ShowUSSWindow(true);
				}
				m_changeUpViews.ShowWindow(SW_SHOW);
				m_changeUpViews.EnableWindow(TRUE);
				break;
			case SOURCES:
				m_XSCheck.ShowWindow(SW_SHOW);
				m_XSCheck.EnableWindow(TRUE);

				m_reaskSourceAfterIPChangeCheck.ShowWindow(SW_SHOW);
				m_reaskSourceAfterIPChangeCheck.EnableWindow(TRUE);

				m_iSUQWT.ShowWindow(SW_SHOW);
				m_iSUQWT.EnableWindow(TRUE);
				infoSUQWT.ShowWindow(SW_SHOW);
				infoSUQWT.EnableWindow(TRUE);
				break;
			case SECURITY:
				// [TPT] - eWombat SNAFU v2
				m_SnafuGroupBox.ShowWindow(SW_SHOW);
				m_SnafuGroupBox.EnableWindow(TRUE);
				m_iSnafu.ShowWindow(SW_SHOW);
				m_iSnafu.EnableWindow(TRUE);
				m_iAntiCreditTheft.ShowWindow(SW_SHOW);
				m_iAntiCreditTheft.EnableWindow(TRUE);	
				m_iAntiFriendshare.ShowWindow(SW_SHOW);
				m_iAntiFriendshare.EnableWindow(TRUE);
				// [TPT] - eWombat SNAFU v2

				// [TPT] - Fakecheck
				m_fakeBox.ShowWindow(SW_SHOW);
				m_fakeBox.EnableWindow(TRUE);
				m_fakeStartup.ShowWindow(SW_SHOW);
				m_fakeStartup.EnableWindow(TRUE);
				m_fakeUpdate.ShowWindow(SW_SHOW);
				m_fakeUpdate.EnableWindow(TRUE);
				m_fakeURL.ShowWindow(SW_SHOW);
				m_fakeURL.EnableWindow(TRUE);
				m_fakeVersion.ShowWindow(SW_SHOW);
				m_fakeVersion.EnableWindow(TRUE);
				m_userHashStatic.ShowWindow(SW_SHOW);
				m_userHashStatic.EnableWindow(TRUE);
				m_userHashEdit.ShowWindow(SW_SHOW);
				m_userHashEdit.EnableWindow(TRUE);
				break;
			case LOGFILTER:
				m_filterVerboseGroupbox.ShowWindow(SW_SHOW);
				m_filterVerboseGroupbox.EnableWindow(TRUE);
				m_filterVerboseCheck01.ShowWindow(SW_SHOW);
				m_filterVerboseCheck01.EnableWindow((thePrefs.GetVerbose() == false) ? FALSE : TRUE);												
				m_filterVerboseCheck02.ShowWindow(SW_SHOW);
				m_filterVerboseCheck02.EnableWindow((thePrefs.GetVerbose() == false) ? FALSE : TRUE);
				m_filterVerboseCheck03.ShowWindow(SW_SHOW);
				m_filterVerboseCheck03.EnableWindow((thePrefs.GetVerbose() == false) ? FALSE : TRUE);
				// [TPT] - Filter own messages
				m_filterVerboseCheck04.ShowWindow(SW_SHOW);
				m_filterVerboseCheck04.EnableWindow((thePrefs.GetVerbose() == false) ? FALSE : TRUE);
				// [TPT] - Filter Handshake messages
				m_filterVerboseCheck05.ShowWindow(SW_SHOW);
				m_filterVerboseCheck05.EnableWindow((thePrefs.GetVerbose() == false) ? FALSE : TRUE);				
				// [TPT] - Filter dead sources
				m_filterVerboseCheck06.ShowWindow(SW_SHOW);
				m_filterVerboseCheck06.EnableWindow((thePrefs.GetVerbose() == false) ? FALSE : TRUE);					
				break;
			case BACKUP:
				// [TPT]- TBH-AutoBackup
				m_backupDAT.ShowWindow(SW_SHOW);
				m_backupDAT.EnableWindow(TRUE);
				m_backupMET.ShowWindow(SW_SHOW);
				m_backupMET.EnableWindow(TRUE);
				m_backupINI.ShowWindow(SW_SHOW);
				m_backupINI.EnableWindow(TRUE);
				m_backupPARTMET.ShowWindow(SW_SHOW);
				m_backupPARTMET.EnableWindow(TRUE);
				m_backupPART.ShowWindow(SW_SHOW);
				m_backupPART.EnableWindow(TRUE);
				m_backupNow.ShowWindow(SW_SHOW);
				m_backupNow.EnableWindow(CheckAnyActive());
				m_selectall.ShowWindow(SW_SHOW);
				m_selectall.EnableWindow(TRUE);
				m_AutoBackup.ShowWindow(SW_SHOW);
				m_AutoBackup.EnableWindow(TRUE);
				m_AutoBackup2.ShowWindow(SW_SHOW);
				m_AutoBackup2.EnableWindow(m_AutoBackup.GetCheck() == BST_CHECKED);				
				m_AutoBackupBox.ShowWindow(SW_SHOW);
				m_AutoBackupBox.EnableWindow(TRUE);				
				m_backupFilesBox.ShowWindow(SW_SHOW);
				m_backupFilesBox.EnableWindow(TRUE);
				m_backupStatic.ShowWindow(SW_SHOW);
				m_backupStatic.EnableWindow(TRUE);
				m_backupStaticBox.ShowWindow(SW_SHOW);
				m_backupStaticBox.EnableWindow(TRUE);
				// [TPT]- TBH-AutoBackup
				break;
		}
	}
}
// [TPT] - quick start
	void CPPgPhoenix1::OnQuickStartChange(void)
{
	bool check = (m_iQuickStart.GetCheck() == BST_CHECKED);

	m_iMaxConn.EnableWindow(check);
	m_iMaxConnStatic.EnableWindow(check);				
	m_iMaxConnPerFive.EnableWindow(check);				
	m_iMaxConnPerFiveStatic.EnableWindow(check);

	SetModified(TRUE);
}
// [TPT] - quick start


void CPPgPhoenix1::OnBnClickedUpdatefakes()
{
	OnApply();
	theApp.FakeCheck->DownloadFakeList();
	CString strBuffer;
	strBuffer.Format(_T("v.%u"), thePrefs.GetFakesDatVersion());
	m_fakeVersion.SetWindowText(strBuffer);

}

void CPPgPhoenix1::OnBnClickedChangeUpViews()
{
	if(isWndNormalUpActive == true)
	{
				EnableNafcWindow(false);
				ShowNafcWindow(false);
				EnableUSSWindow(true);
				ShowUSSWindow(true);
				m_changeUpViews.SetWindowText(GetResString(IDS_VIEWNORMALUP));
				isWndNormalUpActive = false;
	}
	else
	{
				EnableNafcWindow(true);
				ShowNafcWindow(true);
				EnableUSSWindow(false);
				ShowUSSWindow(false);
				m_changeUpViews.SetWindowText(GetResString(IDS_VIEWUSS));
				isWndNormalUpActive = true;
	}

}


void CPPgPhoenix1::OnUSSChange()
{
	if(m_USSEnable.GetCheck() == BST_CHECKED)
	{
		if((m_newUploadSlotSharpingCheck.GetCheck() == BST_CHECKED) || (m_NAFCCheck02.GetCheck() == BST_CHECKED)
			|| (thePrefs.GetNAFCFullControl() == true) || (thePrefs.GetEnableNewUSS() == true))
		{
		TCHAR buffer[200];
		_stprintf(buffer,GetResString(IDS_USSPREF));
		if(MessageBox(buffer,GetResString(IDS_UPUNCOMPATIBLE),MB_ICONQUESTION|MB_YESNO)== IDYES)
		{
			//Desactivo
			thePrefs.SetNAFCFullControl(false);
			thePrefs.SetEnableNewUSS(false);
			m_newUploadSlotSharpingCheck.SetCheck(false);
			m_NAFCCheck02.SetCheck(false);
		}
		else
		{
			//No puedo usar el USS, asi que lo desactivo
			thePrefs.SetDynUpEnabled(false);
			m_USSEnable.SetCheck(false);
			EnableUSSWindow(false);
		}
		
		}
				EnableUSSWindow(true);
	}
				else
				{
				EnableUSSWindow(false);
				}

	SetModified(TRUE);
}
void CPPgPhoenix1::OnUSSTolerancePercent()
{

	if(m_USSPingTolerancePercentCheck.GetCheck() == BST_CHECKED)
	{
		m_USSPingToleranceMSCheck.SetCheck(false);
		m_USSPingToleranceMSEdit.EnableWindow(FALSE);
		m_USSPingToleranceMSStatic.EnableWindow(FALSE);
		m_USSPingTolerancePercentEdit.EnableWindow(TRUE);
		m_USSPingTolerancePercentStatic.EnableWindow(TRUE);
	}
	if(m_USSPingTolerancePercentCheck.GetCheck() == BST_UNCHECKED)
	{
		m_USSPingToleranceMSCheck.SetCheck(true);
		m_USSPingToleranceMSEdit.EnableWindow(TRUE);
		m_USSPingToleranceMSStatic.EnableWindow(TRUE);
		m_USSPingTolerancePercentEdit.EnableWindow(FALSE);
		m_USSPingTolerancePercentStatic.EnableWindow(FALSE);
	}

	SetModified(TRUE);
}

void CPPgPhoenix1::OnUSSToleranceCheck()
{
	if(m_USSPingToleranceMSCheck.GetCheck() == BST_CHECKED)
	{
		m_USSPingTolerancePercentCheck.SetCheck(false);
		m_USSPingToleranceMSEdit.EnableWindow(TRUE);
		m_USSPingToleranceMSStatic.EnableWindow(TRUE);
		m_USSPingTolerancePercentEdit.EnableWindow(FALSE);
		m_USSPingTolerancePercentStatic.EnableWindow(FALSE);
	}
	if(m_USSPingToleranceMSCheck.GetCheck() == BST_UNCHECKED)
	{
		m_USSPingTolerancePercentCheck.SetCheck(true);
		m_USSPingToleranceMSEdit.EnableWindow(FALSE);
		m_USSPingToleranceMSStatic.EnableWindow(FALSE);
		m_USSPingTolerancePercentEdit.EnableWindow(TRUE);
		m_USSPingTolerancePercentStatic.EnableWindow(TRUE);
	}

	SetModified(TRUE);
}



void CPPgPhoenix1::OnNAFCChange(){
	m_NAFCCheck02.EnableWindow(m_NAFCCheck01.GetCheck() == BST_CHECKED);	
	//ikabot: siempre desabilitado
	//m_NAFCEdit.EnableWindow(theApp.pBandWidthControl->IsNAFCAvailable() == true && m_NAFCCheck01.GetCheck() != BST_UNCHECKED);	
	SetModified(TRUE);
}

void CPPgPhoenix1::OnNAFCFullControlChange()
{
	if(m_NAFCCheck02.GetCheck() == BST_CHECKED)
	{
		if((thePrefs.IsDynUpEnabled() == true) || (m_USSEnable.GetCheck() == BST_CHECKED))
		{
		TCHAR buffer[200];
		_stprintf(buffer,GetResString(IDS_FULLNAFCPREF));
		if(MessageBox(buffer,GetResString(IDS_UPUNCOMPATIBLE),MB_ICONQUESTION|MB_YESNO)== IDYES)
		{
			thePrefs.SetDynUpEnabled(false);
			m_USSEnable.SetCheck(false);

		}
		else
		{
			thePrefs.SetNAFCFullControl(false);
			m_NAFCCheck02.SetCheck(false);
		}
		}
	}
	SetModified(TRUE);
}
void CPPgPhoenix1::OnNewUploadSlotSharpingChange()
{


	if(m_newUploadSlotSharpingCheck.GetCheck() == BST_CHECKED)
	{
		if((thePrefs.IsDynUpEnabled() == true) || (m_USSEnable.GetCheck() == BST_CHECKED))
		{
		TCHAR buffer[200];
		_stprintf(buffer,GetResString(IDS_NUSSPREF));
		if(MessageBox(buffer,GetResString(IDS_UPUNCOMPATIBLE),MB_ICONQUESTION|MB_YESNO)== IDYES)
		{
			thePrefs.SetDynUpEnabled(false);
			m_USSEnable.SetCheck(false);

		}
		else
		{
			thePrefs.SetEnableNewUSS(false);
			m_newUploadSlotSharpingCheck.SetCheck(false);
		}
		}
	}
	SetModified(TRUE);
}

// [TPT]- TBH-AutoBackup -->
bool CPPgPhoenix1::CheckAnyActive()
{
	if(m_backupDAT.GetCheck() == BST_CHECKED ||
		m_backupMET.GetCheck() == BST_CHECKED || m_backupINI.GetCheck() == BST_CHECKED ||
			m_backupPART.GetCheck() == BST_CHECKED || m_backupPARTMET.GetCheck() == BST_CHECKED)
			return true;
	return false;
}

// [TPT] - TBH - AutoBackup
void CPPgPhoenix1::OnBnClickedDat()
{
	m_backupNow.EnableWindow(m_backupDAT.GetCheck() == BST_CHECKED || CheckAnyActive());
}

void CPPgPhoenix1::OnBnClickedMet()
{
	m_backupNow.EnableWindow(m_backupMET.GetCheck() == BST_CHECKED || CheckAnyActive());
}

void CPPgPhoenix1::OnBnClickedIni()
{
	m_backupNow.EnableWindow(m_backupINI.GetCheck() == BST_CHECKED || CheckAnyActive());
}

void CPPgPhoenix1::OnBnClickedPart()
{
	m_backupNow.EnableWindow(m_backupPART.GetCheck() == BST_CHECKED || CheckAnyActive());
}

void CPPgPhoenix1::OnBnClickedPartMet()
{
	m_backupNow.EnableWindow(m_backupPARTMET.GetCheck() == BST_CHECKED || CheckAnyActive());
}

void CPPgPhoenix1::OnBnAutobackup()
{
	m_AutoBackup2.EnableWindow(m_AutoBackup.GetCheck() == BST_CHECKED);
}

void CPPgPhoenix1::OnBnClickedBackupnow()
{

	TCHAR buffer[200];
	y2All = FALSE;

	if(m_backupDAT.GetCheck() == BST_CHECKED)
	{
		Backup(_T("*.dat"), true);
		m_backupDAT.SetCheck(false);
	}
	if(m_backupMET.GetCheck() == BST_CHECKED)
	{
		Backup(_T("*.met"), true);
		m_backupMET.SetCheck(false);
	}
	if(m_backupINI.GetCheck() == BST_CHECKED)
	{
		Backup(_T("*.ini"), true);
		m_backupINI.SetCheck(false);
	}
	if(m_backupPARTMET.GetCheck() == BST_CHECKED)
	{	
		Backup2(_T("*.part.met"));
		m_backupPARTMET.SetCheck(false);
	}
	if(m_backupPART.GetCheck() == BST_CHECKED)
	{
		_stprintf(buffer,GetResString(IDS_BACKUPCAUTIONPART));
		if(MessageBox(buffer,GetResString(IDS_BACKUPARESURE),MB_ICONQUESTION|MB_YESNO)== IDYES)
			Backup2(_T("*.part"));
		m_backupPART.SetCheck(false);
	}

	MessageBox(GetResString(IDS_BACKUPSUCCESS), GetResString(IDS_BACKUPCOMPLETE), MB_OK);
	y2All = FALSE;
}


void CPPgPhoenix1::Backup2(LPCTSTR extensionToBack)  
{

	WIN32_FIND_DATA FileData;   
	HANDLE hSearch;   
	TCHAR buffer[200];  


	//CString szDirPath = CString(thePrefs.GetAppDir());  
	CString szDirPath = CString(thePrefs.GetConfigDir());
	CString szTempPath = CString(thePrefs.GetTempDir());  
	TCHAR szNewPath[MAX_PATH]; 

	BOOL fFinished = FALSE;     
	BOOL error = FALSE;  
	BOOL OverWrite = TRUE;  
	szDirPath += _T("Backup\\");

	if(!PathFileExists(szDirPath))  
		CreateDirectory(szDirPath, NULL);  

	szDirPath+= _T("Temp\\");  

	if(!PathFileExists(szDirPath))  
		CreateDirectory(szDirPath, NULL);  


	// Start searching for files in the current directory.   
	SetCurrentDirectory(szTempPath);  

	hSearch = FindFirstFile(extensionToBack, &FileData);   

	if (hSearch == INVALID_HANDLE_VALUE)   
	{   
		error = TRUE;
	}   

	// Copy each file to the new directory   
	while (!fFinished && !error)   
	{   
		lstrcpy(szNewPath, szDirPath);   
		lstrcat(szNewPath, FileData.cFileName);   

		//MessageBox(szNewPath,"New Path",MB_OK);  
		if(PathFileExists(szNewPath))  
		{  
				if (y2All == FALSE)
				{
					_stprintf(buffer, GetResString(IDS_BACKUPALREADYEXISTS), FileData.cFileName);
					int rc = ::XMessageBox(m_hWnd,buffer,GetResString(IDS_BACKUPOVERWRITE),MB_YESNO|MB_YESTOALL|MB_ICONQUESTION);
					if (rc == IDYES)
						OverWrite = TRUE;
					else if (rc == IDYESTOALL)
					{
						OverWrite = TRUE;
						y2All = TRUE;
					}
					else 
						OverWrite = FALSE;
				} else
					OverWrite = TRUE;  
		}  

		if(OverWrite)  
			CopyFile(FileData.cFileName, szNewPath, FALSE);  

		if (!FindNextFile(hSearch, &FileData))   
		{  
			if (GetLastError() == ERROR_NO_MORE_FILES)   
			{   

				fFinished = TRUE;   
			}   
			else   
			{   
				error = TRUE;  
			}   
		}  

	}   

	// Close the search handle.   
	if (!FindClose(hSearch))   
	{   
		error = TRUE;  
	}   
	SetCurrentDirectory(CString(thePrefs.GetConfigDir()));  

	if (error)  
		MessageBox(GetResString(IDS_BACKUPERROR), _T("Error"), MB_OK);  

} 

void CPPgPhoenix1::OnBnClickedSelectall()
{
		m_backupDAT.SetCheck(true);
		m_backupMET.SetCheck(true);
		m_backupINI.SetCheck(true);
		m_backupPARTMET.SetCheck(true);
		m_backupPART.SetCheck(true);
		m_backupNow.EnableWindow(true);

}

void CPPgPhoenix1::Backup3()
{
	WIN32_FIND_DATA FileData; 
	HANDLE hSearch; 
	CString szDirPath = CString(thePrefs.GetConfigDir())+ _T("Backup\\");
	if(!PathFileExists(szDirPath)) return;
	TCHAR szNewPath[MAX_PATH]; 

	SetCurrentDirectory(szDirPath);
	BOOL error = FALSE;
	szDirPath = CString(thePrefs.GetConfigDir())+ _T("Backup2\\");

	BOOL fFinished = FALSE; 

	// Create a new directory if one does not exist
	if(!PathFileExists(szDirPath))
		CreateDirectory(szDirPath, NULL);

	// Start searching for files in the current directory. 

	hSearch = FindFirstFile(_T("*.*"), &FileData); 
	if (hSearch == INVALID_HANDLE_VALUE) 
	{ 
		error = TRUE;
	} 

	// Copy each file to the new directory 
	while (!fFinished && !error) 
	{ 
		lstrcpy(szNewPath, szDirPath); 
		lstrcat(szNewPath, FileData.cFileName); 

		CopyFile(FileData.cFileName, szNewPath, FALSE);

		if (!FindNextFile(hSearch, &FileData)) 
		{
			if (GetLastError() == ERROR_NO_MORE_FILES) 
			{ 
				//MessageBox("File Copied Successfully.", "BackUp complete", MB_OK); 
				fFinished = TRUE; 

			} 
			else 
			{ 
				error = TRUE;
			} 
		}

	} 

	// Close the search handle. 
	if (!FindClose(hSearch)) 
	{ 
		error = TRUE;
	} 
	if (error)
		MessageBox(GetResString(IDS_BACKUPERROR), _T("Error"), MB_OK);
}


void CPPgPhoenix1::Backup(LPCTSTR extensionToBack, BOOL conFirm)  
{
	WIN32_FIND_DATA FileData; 
	HANDLE hSearch; 
	TCHAR buffer[200];
	//CString szDirPath = CString(thePrefs.GetAppDir());
	CString szDirPath = CString(thePrefs.GetConfigDir());
	TCHAR szNewPath[MAX_PATH]; 

	SetCurrentDirectory(szDirPath);
	BOOL error = FALSE;
	BOOL OverWrite = TRUE;
	szDirPath += _T("Backup\\");

	BOOL fFinished = FALSE; 

	// Create a new directory if one does not exist
	if(!PathFileExists(szDirPath))
		CreateDirectory(szDirPath, NULL);

	// Start searching for files in the current directory. 

	hSearch = FindFirstFile(extensionToBack, &FileData); 
	if (hSearch == INVALID_HANDLE_VALUE) 
	{ 
		error = TRUE;
	} 

	// Copy each file to the new directory 
	CString str;
	while (!fFinished && !error) 
	{ 
		lstrcpy(szNewPath, szDirPath); 
		lstrcat(szNewPath, FileData.cFileName); 

		if(PathFileExists(szNewPath))
		{
			if (conFirm)
			{
				if (y2All == FALSE)
				{
					_stprintf(buffer, GetResString(IDS_BACKUPALREADYEXISTS), FileData.cFileName);
					int rc = ::XMessageBox(m_hWnd,buffer,GetResString(IDS_BACKUPOVERWRITE),MB_YESNO|MB_YESTOALL|MB_ICONQUESTION);
					if (rc == IDYES)
						OverWrite = TRUE;
					else if (rc == IDYESTOALL)
					{
						OverWrite = TRUE;
						y2All = TRUE;
					}
					else 
						OverWrite = FALSE;
				} else
					OverWrite = TRUE;
			} 
			else
				OverWrite = TRUE;
		}	
		if(OverWrite)
			CopyFile(FileData.cFileName, szNewPath, FALSE);

		if (!FindNextFile(hSearch, &FileData)) 
		{
			if (GetLastError() == ERROR_NO_MORE_FILES) 
			{ 
				//MessageBox("File Copied Successfully.", "BackUp complete", MB_OK); 
				fFinished = TRUE; 

			} 
			else 
			{ 
				error = TRUE;
			} 
		}

	} 


	// Close the search handle. 
	if (!FindClose(hSearch)) 
	{ 
		error = TRUE;
	} 
	if (error)
		MessageBox(GetResString(IDS_BACKUPERROR), _T("Error"), MB_OK);
}
// [TPT]- TBH-AutoBackup <--



void CPPgPhoenix1::OnHelp()
{
	//theApp.ShowHelp(eMule_FAQ_Preferences_Extended_Settings);
}

BOOL CPPgPhoenix1::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPPgPhoenix1::OnHelpInfo(HELPINFO* pHelpInfo)
{
	OnHelp();
	return TRUE;
}

void CPPgPhoenix1::EnableNafcWindow(bool state)
{
	m_NAFCGroupBox.EnableWindow(state && theApp.pBandWidthControl->IsNAFCAvailable());
	m_NAFCStatic.EnableWindow(state &&theApp.pBandWidthControl->IsNAFCAvailable());
	m_NAFCCheck01.EnableWindow(state && theApp.pBandWidthControl->IsNAFCAvailable());
	m_NAFCCheck02.EnableWindow(state && m_NAFCCheck01.GetCheck());
	m_NAFCEdit.EnableWindow(FALSE);
	m_minSlotEdit.EnableWindow(state);
	m_minSlotStatic.EnableWindow(state);			
	m_iCumulateBW.EnableWindow(state);
	m_iMinimizeSlots.EnableWindow(state);
	m_newUploadSlotSharpingCheck.EnableWindow(state);
	m_uploadTweaks.EnableWindow(state);
	m_unlimitedUP.EnableWindow(state);
}

void CPPgPhoenix1::ShowNafcWindow(bool doShow)
{
	if(doShow == true)
	{
		m_NAFCGroupBox.ShowWindow(SW_SHOW);
		m_NAFCStatic.ShowWindow(SW_SHOW);
		m_NAFCCheck01.ShowWindow(SW_SHOW);
		m_NAFCCheck02.ShowWindow(SW_SHOW);
		m_NAFCEdit.ShowWindow(SW_SHOW);
		m_minSlotEdit.ShowWindow(SW_SHOW);
		m_minSlotStatic.ShowWindow(SW_SHOW);
		m_iCumulateBW.ShowWindow(SW_SHOW);
		m_iMinimizeSlots.ShowWindow(SW_SHOW);
		m_newUploadSlotSharpingCheck.ShowWindow(SW_SHOW);
		m_uploadTweaks.ShowWindow(SW_SHOW);
		m_unlimitedUP.ShowWindow(SW_SHOW);

	}
	else
	{
		m_NAFCGroupBox.ShowWindow(SW_HIDE);
		m_NAFCStatic.ShowWindow(SW_HIDE);
		m_NAFCCheck01.ShowWindow(SW_HIDE);
		m_NAFCCheck02.ShowWindow(SW_HIDE);
		m_NAFCEdit.ShowWindow(SW_HIDE);
		m_minSlotEdit.ShowWindow(SW_HIDE);
		m_minSlotStatic.ShowWindow(SW_HIDE);
		m_iCumulateBW.ShowWindow(SW_HIDE);
		m_iMinimizeSlots.ShowWindow(SW_HIDE);
		m_newUploadSlotSharpingCheck.ShowWindow(SW_HIDE);
		m_uploadTweaks.ShowWindow(SW_HIDE);
		m_unlimitedUP.ShowWindow(SW_HIDE);
	}
}

void CPPgPhoenix1::EnableUSSWindow(bool state)
{
	if(state == true)
	{
		if(m_USSEnable.GetCheck() == BST_CHECKED)
		{
			m_USSLowUpSpeed.EnableWindow(TRUE);
			m_USSLowUpSpeedStatic.EnableWindow(TRUE);
			m_USSPingTolerancePercentCheck.EnableWindow(TRUE);
			m_USSPingToleranceMSCheck.EnableWindow(TRUE);
			m_USSPingToleranceMSEdit.EnableWindow(thePrefs.IsDynUpUseMillisecondPingTolerance());	
			m_USSPingToleranceMSStatic.EnableWindow(thePrefs.IsDynUpUseMillisecondPingTolerance());			
			m_USSPingTolerancePercentEdit.EnableWindow(!thePrefs.IsDynUpUseMillisecondPingTolerance());				
			m_USSPingTolerancePercentStatic.EnableWindow(!thePrefs.IsDynUpUseMillisecondPingTolerance());
			m_USSGoingDown.EnableWindow(TRUE);				
			m_USSGoingDownStatic.EnableWindow(TRUE);				
			m_USSGoingUp.EnableWindow(TRUE);				
			m_USSGoingUpStatic.EnableWindow(TRUE);				
			m_USSNumberOfPings.EnableWindow(TRUE);				
			m_USSNumberOfPingsStatic.EnableWindow(TRUE);
			m_USSpingToleranceBox.EnableWindow(TRUE);
		}
		else
		{
			m_USSLowUpSpeed.EnableWindow(FALSE);
			m_USSLowUpSpeedStatic.EnableWindow(FALSE);
			m_USSPingTolerancePercentCheck.EnableWindow(FALSE);
			m_USSPingToleranceMSCheck.EnableWindow(FALSE);
			m_USSPingToleranceMSEdit.EnableWindow(FALSE);
			m_USSPingToleranceMSStatic.EnableWindow(FALSE);
			m_USSPingTolerancePercentEdit.EnableWindow(FALSE);
			m_USSPingTolerancePercentStatic.EnableWindow(FALSE);
			m_USSGoingDown.EnableWindow(FALSE);
			m_USSGoingDownStatic.EnableWindow(FALSE);
			m_USSGoingUp.EnableWindow(FALSE);
			m_USSGoingUpStatic.EnableWindow(FALSE);
			m_USSNumberOfPings.EnableWindow(FALSE);
			m_USSNumberOfPingsStatic.EnableWindow(FALSE);
			m_USSpingToleranceBox.EnableWindow(FALSE);
		}
	}
	else
	{
		m_USSLowUpSpeed.EnableWindow(FALSE);
		m_USSLowUpSpeedStatic.EnableWindow(FALSE);
		m_USSPingTolerancePercentCheck.EnableWindow(FALSE);
		m_USSPingToleranceMSCheck.EnableWindow(FALSE);
		m_USSPingToleranceMSEdit.EnableWindow(FALSE);
		m_USSPingToleranceMSStatic.EnableWindow(FALSE);
		m_USSPingTolerancePercentEdit.EnableWindow(FALSE);
		m_USSPingTolerancePercentStatic.EnableWindow(FALSE);
		m_USSGoingDown.EnableWindow(FALSE);
		m_USSGoingDownStatic.EnableWindow(FALSE);
		m_USSGoingUp.EnableWindow(FALSE);
		m_USSGoingUpStatic.EnableWindow(FALSE);
		m_USSNumberOfPings.EnableWindow(FALSE);
		m_USSNumberOfPingsStatic.EnableWindow(FALSE);
		m_USSpingToleranceBox.EnableWindow(FALSE);
	}
}

void CPPgPhoenix1::ShowUSSWindow(bool doShow)
{
	if(doShow == true)
	{
		m_USSEnable.ShowWindow(SW_SHOW);
		m_USSLowUpSpeed.ShowWindow(SW_SHOW);
		m_USSLowUpSpeedStatic.ShowWindow(SW_SHOW);
		m_USSPingTolerancePercentCheck.ShowWindow(SW_SHOW);
		m_USSPingTolerancePercentEdit.ShowWindow(SW_SHOW);
		m_USSPingTolerancePercentStatic.ShowWindow(SW_SHOW);
		m_USSPingToleranceMSCheck.ShowWindow(SW_SHOW);
		m_USSPingToleranceMSEdit.ShowWindow(SW_SHOW);
		m_USSPingToleranceMSStatic.ShowWindow(SW_SHOW);
		m_USSGoingDown.ShowWindow(SW_SHOW);
		m_USSGoingDownStatic.ShowWindow(SW_SHOW);
		m_USSGoingUpStatic.ShowWindow(SW_SHOW);
		m_USSGoingUp.ShowWindow(SW_SHOW);
		m_USSNumberOfPings.ShowWindow(SW_SHOW);
		m_USSNumberOfPingsStatic.ShowWindow(SW_SHOW);
		m_USSpingToleranceBox.ShowWindow(SW_SHOW);
	}
	else
	{
		m_USSEnable.ShowWindow(SW_HIDE);
		m_USSLowUpSpeed.ShowWindow(SW_HIDE);
		m_USSLowUpSpeedStatic.ShowWindow(SW_HIDE);
		m_USSPingTolerancePercentCheck.ShowWindow(SW_HIDE);
		m_USSPingTolerancePercentEdit.ShowWindow(SW_HIDE);
		m_USSPingTolerancePercentStatic.ShowWindow(SW_HIDE);
		m_USSPingToleranceMSCheck.ShowWindow(SW_HIDE);
		m_USSPingToleranceMSEdit.ShowWindow(SW_HIDE);
		m_USSPingToleranceMSStatic.ShowWindow(SW_HIDE);
		m_USSGoingDown.ShowWindow(SW_HIDE);
		m_USSGoingDownStatic.ShowWindow(SW_HIDE);
		m_USSGoingUpStatic.ShowWindow(SW_HIDE);
		m_USSGoingUp.ShowWindow(SW_HIDE);
		m_USSNumberOfPings.ShowWindow(SW_HIDE);
		m_USSNumberOfPingsStatic.ShowWindow(SW_HIDE);
		m_USSpingToleranceBox.ShowWindow(SW_HIDE);
	}
}

//TODO: we can use another speed instead the unlimited speed :)
void CPPgPhoenix1::OnUnlimitedSpeed()
{
	SetModified(TRUE);
}

//ikabot: experimental, i´m trying to increase the lower IP layer
//efficiency. Only with ATM encapsulated over AAL5.
//An ATM cell is formed by: 48 bytes of data + 5 bytes of headers
//We have to fragment the PDU from IP layer into cells.
//the formula to get the cells number is: (PDUlenght + 8)/48
//With this, we´ll get an efficiency, but we can improve it
//MTU is de max datagram over a network, at IP level.
//We can get this value from user preferences and play with it
//but always, a lower value than MTU in preferences
//#define EnteroSuperior( a ) ((double)((int)a) == (double)a ? (int)a : (int)a + 1)
/*void CPPgPhoenix1::OnClickRecalculate()
{
	uint16 oldMTU = thePrefs.GetMTU();
	//AAL5 formula. If there is another encapsulation, we need other
	//but usually, ADSL companies use AAL5
	float cellsNumber = (oldMTU + 8)/48;
	int infCell = (int)cellsNumber; //I have the lower integer
	//TODOika rework. I don´t like the previous code, I need a better one :D

}*/