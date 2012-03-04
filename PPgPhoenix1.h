#pragma once

#include "preferences.h"
#include "wizard.h"
#include "ToolTips\PPToolTip.h"// [TPT] - Tooltips in preferences

// CPPgPhoenix1 dialog
class CPPgPhoenix1 : public CPropertyPage // [TPT] - SLUGFILLER: modelessDialogs
{
	DECLARE_DYNAMIC(CPPgPhoenix1)

public:
	CPPgPhoenix1();
	virtual ~CPPgPhoenix1();

	void Localize(void);
	void LoadSettings();


	// Dialog Data
	enum { IDD = IDD_PPG_PHOENIX1 }; 

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
// [TPT]- TBH-AutoBackup
private:
	void Backup2(LPCTSTR extensionToBack);
	BOOL y2All;
// [TPT]- TBH-AutoBackup
	void ShowUSSWindow(bool doShow);
	void EnableUSSWindow(bool state);
	void ShowNafcWindow(bool doShow);
	void EnableNafcWindow(bool state);
public:
	virtual BOOL OnApply();
	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	// [TPT]- TBH-AutoBackup
	afx_msg void OnBnClickedSelectall();
	afx_msg void OnBnClickedBackupnow();
	afx_msg void OnBnAutobackup();
	afx_msg void OnBnClickedPartMet();
	afx_msg void OnBnClickedPart();
	afx_msg void OnBnClickedIni();
	afx_msg void OnBnClickedMet();
	afx_msg void OnBnClickedDat();
	// [TPT]- TBH-AutoBackup
	afx_msg void OnNAFCChange();
	afx_msg void OnSettingsChange() { SetModified(TRUE); }
	afx_msg void OnTabSelectionChange(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnQuickStartChange(void); // [TPT] - quick start
	// [TPT]- TBH-AutoBackup
	void Backup3();
	void Backup(LPCTSTR extensionToBack, BOOL conFirm);
	bool CheckAnyActive();
	// [TPT]- TBH-AutoBackup
	afx_msg void OnUSSToleranceCheck();
	afx_msg void OnUSSTolerancePercent();
	afx_msg void OnUSSChange();
	afx_msg void OnBnClickedChangeUpViews();
	afx_msg void OnNAFCFullControlChange();
	afx_msg void OnNewUploadSlotSharpingChange();
	afx_msg void OnUnlimitedSpeed();
	virtual BOOL PreTranslateMessage(MSG* pMsg);// [TPT] - Tooltips in preferences
	void UpdatepHoeniXOneTooltips();
	afx_msg void OnBnClickedUpdatefakes();// [TPT] - Fakecheck

protected:
	CPreferences *app_prefs;

private:
	enum eTab {NONE, UPLOAD, CONNECTION, SOURCES, LOGFILTER, SECURITY, BACKUP}; // [TPT]- TBH-AutoBackup
	void SetTab(eTab tab);

	void InitTab();
	void InitControl();	
	// [TPT] - Tooltips in preferences
	CPPToolTip m_Tip;


	// Tab	
	CTabCtrl   m_tabCtr;
	eTab       m_currentTab;
	CImageList m_imageList;

	//CONEXION -----------------------------------------------------------
	// [TPT] - quick start
	CButton m_iQuickStart; 
	CEdit m_iMaxConnPerFive;
	CStatic m_iMaxConnPerFiveStatic;
	CEdit m_iMaxConn;
	CStatic m_iMaxConnStatic;
	// [TPT] - quick start

	CButton  m_QuickGroupBox;

	CEdit   m_MTUEdit;
	CStatic m_MTUStatic;
	CEdit   m_sndSocketSizeEdit;
	CStatic m_sndSocketSizeStatic;
	
	CButton  m_iManageConnection; // [TPT] - Manage Connection

	// [TPT] - MoNKi: -UPnPNAT Support-
	CButton m_iUPnPNatGroupBox;
	CButton m_iUPnPNat;
	CButton m_iUPnPNatWeb;
	// [TPT] - MoNKi: -UPnPNAT Support-

	//SUBIDAS ------------------------------------------------------------
	//Standard Upload System
	CButton  m_NAFCGroupBox;
	CStatic  m_NAFCStatic;
	CButton  m_NAFCCheck01;
	CButton  m_NAFCCheck02;
	CNumEdit m_NAFCEdit;
	CEdit   m_minSlotEdit;
	CStatic m_minSlotStatic;	
	CButton	 m_iCumulateBW; // [TPT] - Cumulate Bandwidth
	CButton  m_iMinimizeSlots; // [TPT] - Pawcio: MUS
    CButton m_newUploadSlotSharpingCheck;
	CButton m_uploadTweaks;
	CButton m_unlimitedUP;
	CButton m_iSUQWT; // [TPT] - SUQWT
	CStatic infoSUQWT;
	//ZZ: Upload Speed Sense
	CButton m_USSEnable;

	CEdit m_USSLowUpSpeed;
	CStatic m_USSLowUpSpeedStatic;

	CButton m_USSPingTolerancePercentCheck;
	CEdit m_USSPingTolerancePercentEdit;
	CStatic m_USSPingTolerancePercentStatic;
	CButton m_USSPingToleranceMSCheck;
	CEdit m_USSPingToleranceMSEdit;
	CStatic m_USSPingToleranceMSStatic;

	CEdit m_USSGoingDown;
	CStatic m_USSGoingDownStatic;

	CEdit m_USSGoingUp;
	CStatic m_USSGoingUpStatic;

	CEdit m_USSNumberOfPings;
	CStatic m_USSNumberOfPingsStatic;
	
	CButton m_USSpingToleranceBox;
	CButton m_changeUpViews;



	//SERVIDOR -----------------------------------------------------------
	CButton m_XSCheck;

	CButton m_reaskSourceAfterIPChangeCheck;
	//SEGURIDAD ----------------------------------------------------------
	// [TPT] - eWombat SNAFU v2
	CButton  m_SnafuGroupBox;
	CButton  m_iSnafu;
	CButton	 m_iAntiFriendshare;
	CButton  m_iAntiCreditTheft;

	// [TPT] - Fakecheck
	CButton m_fakeBox;
	CButton m_fakeStartup;
	CButton m_fakeUpdate;
	CEdit	m_fakeURL;
	CStatic m_fakeVersion;

	CStatic m_userHashStatic;
	CEdit   m_userHashEdit;
	//LOG FILTERS --------------------------------------------------------
	CButton m_filterVerboseGroupbox;
	CButton m_filterVerboseCheck01;	
	CButton m_filterVerboseCheck02;
	CButton m_filterVerboseCheck03;
	CButton m_filterVerboseCheck04;
	CButton m_filterVerboseCheck05;	
	CButton m_filterVerboseCheck06;	
	CButton m_filterVerboseCheck07;	
	
	// [TPT]- TBH-AutoBackup
	CButton m_backupDAT;
	CButton m_backupMET;
	CButton m_backupINI;
	CButton m_backupPARTMET;
	CButton m_backupPART;
	CButton m_backupNow;
	CButton m_selectall;
	CButton m_AutoBackup;
	CButton m_AutoBackup2;
	CButton m_AutoBackupBox;
	CButton m_backupFilesBox;
	CStatic m_backupStatic;
	CButton m_backupStaticBox;

	bool isWndNormalUpActive;
};