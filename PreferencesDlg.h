#pragma once
#include "Modeless.h"	// [TPT] - SLUGFILLER: modelessDialogs
#include "PPgGeneral.h"
#include "PPgConnection.h"
#include "PPgServer.h"
#include "PPgDirectories.h"
#include "PPgFiles.h"
#include "PPgStats.h"
#include "PPgNotify.h"
#include "PPgIRC.h"
#include "PPgTweaks.h"
#include "PPgDisplay.h"
#include "PPgSecurity.h"
#include "PPgWebServer.h"
#include "PPgScheduler.h"
#include "PPgProxy.h"
#include "PPgVirtual.h"	// [TPT] - itsonlyme: virtualDirs
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
#include "PPgDebug.h"
#endif
// [TPT] - Three NEW Preference Panels
#include "PPgPhoenix1.h" 
#include "PPgPhoenix.h"
#include "PPgPhoenix2.h"
// [TPT] - end
#include "otherfunctions.h"
#include "KCSideBannerWnd.h" 	// [TPT] - New Preferences Banner	

class CPreferencesDlg : public CModelessTreePropSheet	// SLUGFILLER: modelessDialogs
{
	DECLARE_DYNAMIC(CPreferencesDlg)

public:
	CPreferencesDlg();
	virtual ~CPreferencesDlg();
	
	CPPgGeneral		m_wndGeneral;
	CPPgConnection	m_wndConnection;
	CPPgServer		m_wndServer;
	CPPgDirectories	m_wndDirectories;
	CPPgFiles		m_wndFiles;
	CPPgStats		m_wndStats;
	CPPgNotify		m_wndNotify;
	CPPgIRC			m_wndIRC;
	CPPgTweaks		m_wndTweaks;
	CPPgDisplay		m_wndDisplay;
	CPPgSecurity	m_wndSecurity;
	CPPgWebServer	m_wndWebServer;
	CPPgScheduler	m_wndScheduler;
	CPPgProxy		m_wndProxy;
	CPPgVirtual		m_wndVirtual;	// [TPT] - itsonlyme: virtualDirs
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	CPPgDebug		m_wndDebug;
#endif
	// [TPT]
	CPPgPhoenix1	m_wndPhoenix1; // [TPT] - Three New Preference Panels
	CPPgPhoenix		m_wndPhoenix;
	CPPgPhoenix2	m_wndPhoenix2;
	// [TPT]
	
	void Localize();
	void SetStartPage(UINT uStartPageID);

protected:
	LPCTSTR m_pPshStartPage;

	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
protected:
	CKCSideBannerWnd m_banner;	// [TPT] - New Preferences Banner	


};
