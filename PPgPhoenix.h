#pragma once

#include "preferences.h"
#include "wizard.h"
#include "ToolTips\PPToolTip.h"// [TPT] - Tooltips in preferences
#include "SharedFileList.h" // [TPT] - Powershare


// CPPgPhoenix dialog
class CPPgPhoenix : public CPropertyPage // [TPT] - SLUGFILLER: modelessDialogs
{
	DECLARE_DYNAMIC(CPPgPhoenix)

public:
	CPPgPhoenix();
	virtual ~CPPgPhoenix();

	void Localize(void);
	void LoadSettings();

	// emulEspaña: added by MoNKi [MoNKi: -invisible mode-]
	afx_msg void OnBnClickedInvisiblemode();
	afx_msg void OnCbnSelchangeKeymodcombo();
	// End MoNKi

	// Dialog Data
	enum { IDD = IDD_PPG_PHOENIX }; 

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// emulEspaña: added by MoNKi [MoNKi: -invisible mode-]	
	UINT			m_iActualKeyModifier;
	// End MoNKi

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnApply();
	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSettingsChange() { SetModified(TRUE); }
	afx_msg void OnTabSelectionChange(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnQuickStartChange(void); // [TPT] - quick start
	afx_msg void OnMultiQueueChange();
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	virtual BOOL PreTranslateMessage(MSG* pMsg);// [TPT] - Tooltips in preferences
	void UpdatepHoeniXReloadTooltips();

protected:
	CPreferences *app_prefs;

private:
	enum eTab {NONE, FILES, MISCELLANEOUS, CATEGORIES, VISUALIZATION, MINIMULE, STATSSETUPINFO};
	void SetTab(eTab tab);

	void InitTab();
	void InitControl();	

	// [TPT] - Tooltips in preferences
	CPPToolTip m_Tip2;

	// Tab	
	CTabCtrl   m_tabCtr;
	eTab       m_currentTab;
	CImageList m_ImageList;
	
	// [TPT] - SLUGFILLER: hideOS
	CEdit	m_iHideOS;
	CStatic m_iHideOSStatic;
	CButton m_iSelectiveShare;	
	// [TPT] - SLUGFILLER: hideOS
	// [TPT] - itsonlyme: displayOptions START	
	CButton m_iShowFileSystemIcon;
	CButton m_iShowLocalRating; // [TPT] -  SLUGFILLER:showComments
	// [TPT] - itsonlyme: displayOptions END
	
	CButton m_iShowBackgroundInMenus;

	CButton m_iHighContrast;   // [TPT] - MoNKi: -Support for High Contrast Mode-

	CButton m_iSpreadBars; // [TPT] - SLUGFILLER: Spreadbars
	
	CButton m_iBold;
	// STATSSETUPINFO
	CButton m_zoomGroupBox;
	CStatic m_zoomStatic;

	CSliderCtrl m_zoomSlider;

	CButton     m_sampleRateGroupBox;
	CStatic     m_sampleRateStatic;
	CSliderCtrl m_sampleRateSlider;

	CButton		m_memoryConsuming;
	CButton		m_NAFCGraph;
	CButton		m_OverheadGraph;
	CButton		m_showVerticalLine;
	// FILES
	CButton m_multiQueueGroupBox;
	CButton m_multiQueueCheck01;
	CButton m_multiQueueCheck02;
	// [TPT] - Powershare
	CButton m_powersharebox;
	CButton m_powersharedisable;
	CButton m_powershareenable;
	CButton m_powershareauto;
	CButton m_powersharelimit;
	CEdit	m_powersharelimitedit;
	CStatic	m_powersharelimiteditstatic;
	CSliderCtrl m_iPowershare;

	// [TPT] - Select process priority 
	bool ExtraPrios;
	CButton  m_PriorityGroupBox;
	CSliderCtrl m_selectPriorityProcess; 	
	CStatic m_RealTimePriority, m_HighPriority, m_AboveNormalPriority,
		m_NormalPriority, m_BelowNormalPriority, m_IdlePriority;
	// [TPT] - Select process priority 
	
	// [TPT] - TBH: minimule
	CButton  m_MiniMuleGroupBox;
	CButton m_iMiniMule;		
	CButton m_iMiniMuleLives;
	CEdit m_iMiniMuleUpdate;
	CStatic m_iMiniMuleUpdateStatic;
	CSliderCtrl m_MiniMuleTransparency;
	CStatic m_MiniMuleTransparencyStatic;
	CStatic m_MiniMuleTransparencyMinimum, m_MiniMuleTransparencyMaximum;
	// [TPT] - TBH: minimule
	
	CSliderCtrl m_iCredits; // [TPT] - Credit System
	
	// [TPT] - khaos::categorymod+
	CButton m_iShowCatNames;
	CButton m_iSelectCat;
	CButton m_iUseActiveCat;
	CButton m_iAutoSetResOrder;	
	CEdit m_iResumeFileInNewCat;	
	CStatic m_iResumeFileInNewCatStatic;
	CButton m_iUseAutoCat;
	CButton m_iShowPriorityInTab;
	// [TPT] - khaos::categorymod-
	CButton m_iCreditsGroupBox;
	CStatic m_iCreditsPawcioStatic, m_iCreditsLovelaceStatic, 
		m_iCreditsOfficialStatic, m_iCreditsNoneStatic;
	// [TPT] - Credit System

	// emulEspaña: added by MoNKi [TPT] - [MoNKi: -invisible mode-]
	CButton m_InvisibleModeGroupBox;
	CButton m_iInvisibleMode;
	CStatic m_iInvisibleModeSelectStatic;
	CStatic m_iInvisibleModeModifierStatic;
	CStatic m_iInvisibleModeKeyStatic;	
	CStatic m_iInvisibleModePlus;
	CComboBox		m_invmode_keymodifiers;
	CComboBox		m_invmode_keys;	

	// [TPT] - IP Country
	CButton m_iEnableShowCountryNames;	
	CButton m_iEnableShowCountryFlags;	
	// [TPT] - IP Country

	CButton m_iShowInMSN7;
	CButton	m_iShowUpPrio;
};