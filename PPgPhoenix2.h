#pragma once

#include "preferences.h"
#include "HypertextCtrl.h"


// CPPgPhoenix dialog
class CPPgPhoenix2 : public CPropertyPage // [TPT] - SLUGFILLER: modelessDialogs
{
	DECLARE_DYNAMIC(CPPgPhoenix2)

public:
	CPPgPhoenix2();
	virtual ~CPPgPhoenix2();

	void Localize(void);
	void LoadSettings();
	void EnableWebcacheControls(bool active);
	// Dialog Data
	enum { IDD = IDD_PPG_PHOENIX2 }; 

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnApply();
	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSettingsChange() { SetModified(TRUE); }
	afx_msg void OnTabSelectionChange(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);

protected:
	CPreferences *app_prefs;

private:
	enum eTab {NONE, WEBCACHE};
	void SetTab(eTab tab);

	void InitTab();
	void InitControl();	


	// Tab	
	CTabCtrl   m_tabCtr;
	eTab       m_currentTab;
	CImageList m_ImageList;
	
	//[TPT] - Webcache
	bool guardian;
	bool showadvanced;
	bool bCreated2;
	afx_msg void OnEnChangeActivatewebcachedownloads();
	afx_msg void OnBnClickedDetectWebCache();
	afx_msg void OnBnClickedAdvancedcontrols();
	afx_msg void OnBnClickedTestProxy(); //JP proxy configuration test
	CButton m_enableWebcache;
	CButton m_webcacheBox;
	CStatic m_webcacheAddressStatic;
	CEdit m_webcacheAddressEdit;
	CStatic m_webcachePortStatic;
	CEdit m_webcachePortEdit;
	CStatic m_webcacheBlocksStatic;
	CStatic m_webcacheBlocksSize;
	CEdit m_webcacheBlocksEdit;
	CButton m_webcacheTimeout;
	CButton m_webcacheNotSameISP;
	CButton m_webcachePersistent;
	CButton m_webcacheAutodetect;
	CButton m_hideControls;
	CButton m_testProxy;
	CButton m_webcacheUpdateAuto;

	CHyperTextCtrl	m_wndSubmitWebcacheLink2;	
};