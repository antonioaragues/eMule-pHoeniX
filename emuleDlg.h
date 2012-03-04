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
#pragma once
#include "TrayDialog.h"
#include "MeterIcon.h"
#include "ToolTips\PPToolTip.h" // [TPT] - MFCK [addon] - New Tooltips [Rayita]
#include "MuleSystrayDlg.h" // [TPT] - TBH: muleSysTray
#include "myinfownd.h" // [TPT] - eWombat [MYINFOWND]
#include "FadeWnd.h"// [TPT] - Fade Window on Exit
#include "TaskBarNotifier.h" // [TPT] - enkeyDEV(th1) -notifier-

namespace Kademlia {
	class CSearch;
	class CContact;
	class CEntry;
	class CUInt128;
};

class CChatWnd;
class CIrcWnd;
class CKademliaWnd;
class CKnownFileList; 
class CMainFrameDropTarget;
class CMuleStatusBarCtrl;
class CMuleToolbarCtrl;
class CPreferencesDlg;
class CSearchDlg;
class CServerWnd;
class CSharedFilesWnd;
class CStatisticsDlg;
class CTaskbarNotifier;
class CTransferWnd;
struct Status;
class CSplashScreen;
class CMuleSystrayDlg;

// emuleapp <-> emuleapp
#define OP_ED2KLINK				12000
#define OP_CLCOMMAND			12001

#define	EMULE_HOTMENU_ACCEL		'x'
#define	EMULSKIN_BASEEXT		_T("eMuleSkin")

class CemuleDlg : public CTrayDialog
{
	friend class CTBHMM; // [TPT] - TBH: minimule
	friend class CMuleToolbarCtrl;
public:
	CemuleDlg(CWnd* pParent = NULL);
	~CemuleDlg();

	enum { IDD = IDD_EMULE_DIALOG };

	bool IsRunning();
	void			AddServerMessageLine(LPCTSTR line);
	void			ShowConnectionState();
	void			ShowNotifier(CString Text, int MsgType=TBN_NULL, LPCTSTR pszLink = NULL, bool ForceSoundOFF=false); //<<-- [TPT] - enkeyDEV(th1) -notifier-
	void			ShowUserCount();
	void			ShowMessageState(uint8 iconnr);
	void			SetActiveDialog(CWnd* dlg);
	void			ShowTransferRate(bool forceAll=false);
	void			LoadNotifier(); //<<-- [TPT] - enkeyDEV(th1) -notifier-
    void            ShowPing();
	void			Localize();

	// Logging
	void			AddLogText(TbnMsg msg, UINT uFlags, LPCTSTR pszText); //<<-- [TPT] - enkeyDEV(th1) -notifier-
	void			AddPhoenixText(TbnMsg msg, UINT uFlags, LPCTSTR pszText); //<<--[TPT] - Debug log
	void			ResetLog();
	void			ResetDebugLog();
	void			ResetPhoenixLog(); // [TPT] - Debug log
	CString			GetLastLogEntry();
	CString			GetLastDebugLogEntry();
	CString			GetAllLogEntries();
	CString			GetAllDebugLogEntries();
	CString	GetConnectionStateString();

	void			StopTimer();
	void			DoVersioncheck(bool manual);
	void			ApplyHyperTextFont(LPLOGFONT pFont);
	void			ApplyLogFont(LPLOGFONT pFont);
	void			ProcessED2KLink(LPCTSTR pszData);
	void			SetStatusBarPartsSize();
	int ShowPreferences(UINT uStartPageID = (UINT)-1);
	bool IsPreferencesDlgOpen() const;

	virtual void RestoreWindow();
	virtual void HtmlHelp(DWORD_PTR dwData, UINT nCmd = 0x000F);

	CTransferWnd*	transferwnd;
	CServerWnd*		serverwnd;
	CPreferencesDlg* preferenceswnd;
	CSharedFilesWnd* sharedfileswnd;
	CSearchDlg*		searchwnd;
	CChatWnd*		chatwnd;
	CMuleStatusBarCtrl* statusbar;
	CStatisticsDlg*  statisticswnd;
	CIrcWnd*		ircwnd;
	CTaskbarNotifier* m_wndTaskbarNotifier;
	CReBarCtrl		m_ctlMainTopReBar;
	CMuleToolbarCtrl* toolbar;
	CKademliaWnd*	kademliawnd;
	CWnd*			activewnd;
	uint8			status;

protected:
	HICON m_hIcon;
	bool			ready;
	bool			m_bStartMinimizedChecked;
	bool			m_bStartMinimized;
	WINDOWPLACEMENT m_wpFirstRestore;
	HICON			connicons[9];
	HICON			transicons[4];
	HICON			imicons[3];
	HICON			m_icoSysTrayCurrent;
	HICON			usericon;
	CMeterIcon		m_TrayIcon;
	HICON			m_icoSysTrayConnected;		// do not use those icons for anything else than the traybar!!!
	HICON			m_icoSysTrayDisconnected;	// do not use those icons for anything else than the traybar!!!
	HICON			m_icoSysTrayLowID;	// do not use those icons for anything else than the traybar!!!
	HICON			sourceTrayIconMail;	// [TPT] - Notify message
	int				m_iMsgIcon;
	UINT			m_uLastSysTrayIconCookie;
	//uint32			m_uUpDatarate; // [TPT]
	//uint32			m_uDownDatarate; // [TPT]
	CImageList		imagelist;
	CMuleSystrayDlg* m_pSystrayDlg;
	CMainFrameDropTarget* m_pDropTarget;
	CMenu			m_SysMenuOptions;
	CMenu			m_menuUploadCtrl;
	CMenu			m_menuDownloadCtrl;
	char			m_acVCDNSBuffer[MAXGETHOSTSTRUCT];

	// Splash screen
	CSplashScreen *m_pSplashWnd;
	DWORD m_dwSplashTime;
	void ShowSplash();
	void DestroySplash();

    // Maella -New Timer Management- (quick-n-dirty)
	enum {MAIN_TIMER};
	void StartMainTimer(); 
	void StopMainTimer();
	void MainTimerProc();
	UINT_PTR m_nMainTimer;

	// Startup Timer
	UINT_PTR m_hTimer;
	static void CALLBACK StartupTimer(HWND hwnd, UINT uiMsg, UINT idEvent, DWORD dwTime);

	DWORD m_dwMSNtime;//[TPT] - Show in MSN7

	void StartConnection();
	void CloseConnection();
	void PostStartupMinimized();
	void UpdateTrayIcon(int iPercent);
	void ShowConnectionStateIcon();
	void ShowTransferStateIcon();
	void ShowUserStateIcon();
	void AddSpeedSelectorSys(CMenu* addToMenu);
	float  GetRecMaxUpload() const; // [TPT] - Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-	
	//void LoadNotifier(CString configuration); //<<--enkeyDEV(kei-kun) -TaskbarNotifier-
	bool notifierenabled;					  //<<-- enkeyDEV(kei-kun) -Quick disable/enable notifier-
	void SwitchNotifierStatus();   //<<-- [TPT] - enkeyDEV(kei-kun) -Quick disable/enable notifier-
	void ForceNotifierPopup();     //<<-- [TPT] - enkeyDEV(kei-kun) -Manual notifier popup-
	void ShowToolPopup(bool toolsonly=false);
	void SetAllIcons();
	bool CanClose();

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();
	virtual void OnTrayRButtonUp(CPoint pt);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType,int cx,int cy);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButton2();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnBnClickedHotmenu();
	afx_msg LRESULT OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu);
	afx_msg void OnSysColorChange();
	afx_msg BOOL OnQueryEndSession();
	afx_msg void OnEndSession(BOOL bEnding);
	afx_msg LRESULT OnKickIdle(UINT nWhy, long lIdleCount);
	afx_msg void OnShowWindow( BOOL bShow, UINT nStatus );
	afx_msg BOOL OnChevronPushed(UINT id, NMHDR *pnm, LRESULT *pResult);

	// quick-speed changer -- based on xrmb
	afx_msg void QuickSpeedUpload(UINT nID);
	afx_msg void QuickSpeedDownload(UINT nID);
	afx_msg void QuickSpeedOther(UINT nID);
	// end of quick-speed changer
	
	afx_msg LRESULT OnTaskbarNotifierClicked(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnWMData(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnFileHashed(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnHashFailed(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnFileAllocExc(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnFileCompleted(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnFileOpProgress(WPARAM wParam,LPARAM lParam);

	//Framegrabbing
	afx_msg LRESULT OnFrameGrabFinished(WPARAM wParam,LPARAM lParam);

	afx_msg LRESULT OnAreYouEmule(WPARAM, LPARAM);

	//Webserver [kuchin]
	afx_msg LRESULT OnWebServerConnect(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWebServerDisonnect(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWebServerRemove(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWebSharedFilesReload(WPARAM wParam, LPARAM lParam);

	// VersionCheck DNS
	afx_msg LRESULT OnVersionCheckResponse(WPARAM wParam, LPARAM lParam);

	// Peercache DNS
	afx_msg LRESULT OnPeerCacheResponse(WPARAM wParam, LPARAM lParam);
	
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);// [TPT] - New Menu Style
public:
	bool			message; // [TPT] - Notify message
// emulEspaña: added by [TPT]-MoNKi [MoNKi: -invisible mode-]
	BOOL	RegisterInvisibleHotKey();
	BOOL	UnRegisterInvisibleHotKey();
protected:
	LRESULT	OnHotKey(WPARAM wParam, LPARAM lParam);

	// Allows "invisible mode" on multiple instances of eMule
	afx_msg LRESULT OnRestoreWindowInvisibleMode(WPARAM, LPARAM);
	static BOOL CALLBACK AskEmulesForInvisibleMode(HWND hWnd, LPARAM lParam);
// End emulEspaña

// [TPT] - MFCK [addon] - New Tooltips [Rayita]
public:
	void SetTTDelay();

private:
	CPreferences* app_prefs;
	CPPToolTip	m_ttip; // [improved tooltips]

protected:	
	afx_msg BOOL OnToolTipNotify(UINT id, NMHDR *pNMH, LRESULT *pResult); // [improved tooltips]
// [TPT] - MFCK [addon] - New Tooltips [Rayita]
};
enum EEMuleAppMsgs
{
	//thread messages
	TM_FINISHEDHASHING = WM_APP + 10,
	TM_HASHFAILED,
	TM_FRAMEGRABFINISHED,
	TM_FILEALLOCEXC,
	TM_FILECOMPLETED,
	TM_FILEOPPROGRESS
};
// emulEspaña: added by [TPT]-MoNKi [MoNKi: -invisible mode-]
enum EEmuleHotKeysIDs
{
	HOTKEY_INVISIBLEMODE_ID
};

enum EEMuleInvisibleModeEnumOptions
{
	INVMODE_RESTOREWINDOW,
	INVMODE_REGISTERHOTKEY
};
// End emulEspaña