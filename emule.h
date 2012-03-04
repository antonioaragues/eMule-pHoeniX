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
#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif
#include ".\Optimizer\cpu_info.h" // [TPT] - Optimizer
#include "resource.h"
#include "wombatlists.h"// [TPT] - eWombat SNAFU v2
#include "TBHMM.h" // [TPT] - TBH: minimule
#include "UPnP_IGDControlPoint.h"	// [TPT] - MoNKi: -UPnPNAT Support-

#define	DEFAULT_NICK		thePrefs.GetHomepageBaseURL()
#define	DEFAULT_TCP_PORT	4662
#define	DEFAULT_UDP_PORT	(DEFAULT_TCP_PORT+10)

class CSearchList;
class CUploadQueue;
class CListenSocket;
class CDownloadQueue;
class CScheduler;
class UploadBandwidthThrottler;
class LastCommonRouteFinder;
class CemuleDlg;
class CClientList;
class CKnownFileList;
class CServerConnect;
class CServerList;
class CSharedFileList;
class CClientCreditsList;
class CFriendList;
class CClientUDPSocket;
class CIPFilter;
class CWebServer;
class CMMServer;
class CAbstractFile;
class CUpDownClient;
class CPeerCacheFinder;
class CFirewallOpener;
class CBandWidthControl; // [TPT]
class CIP2Country; // [TPT] - IP Country
class CFakecheck; // [TPT] - Fakecheck

struct SLogItem;

enum AppState{
	APP_STATE_RUNNING=0,
   	APP_STATE_SHUTINGDOWN,
	APP_STATE_DONE
};

// [TPT] - Moonlight: Global shutdown event to wake up threads.
class CShutdownEvent : private CEvent {
public:
    CShutdownEvent(void) : CEvent(FALSE, TRUE), m_Shutdown(false)   {}
    virtual ~CShutdownEvent     (void)  {}
    void Signal                 (void)  {CEvent::SetEvent(); m_Shutdown = true;}
    bool isShuttingDown         (void)  {return m_Shutdown;}
    CSyncObject *   operator&   (void)  {return this;}
private:
    bool    m_Shutdown;
};
extern CShutdownEvent ShutdownEvent;    /// Declares a global shutdown object.
// [TPT] - Moonlight: Global shutdown event to wake up threads.

class CemuleApp : public CWinApp
{
public:
	CemuleApp(LPCTSTR lpszAppName = NULL);

	// ZZ:UploadSpeedSense -->
    UploadBandwidthThrottler* uploadBandwidthThrottler;
    LastCommonRouteFinder* lastCommonRouteFinder;
	// ZZ:UploadSpeedSense <--
	CPUInfo 			cpu; 		// [TPT] - Optimizer	
	CemuleDlg*			emuledlg;
	CClientList*		clientlist;
	CWombatLists*		wombatlist;// [TPT] - eWombat SNAFU v2
	CKnownFileList*		knownfiles;
	CServerConnect*		serverconnect;
	CServerList*		serverlist;	
	CSharedFileList*	sharedfiles;
	CSearchList*		searchlist;
	CListenSocket*		listensocket;
	CUploadQueue*		uploadqueue;
	CDownloadQueue*		downloadqueue;
	CClientCreditsList*	clientcredits;
	CFriendList*		friendlist;
	CClientUDPSocket*	clientudp;
	CIPFilter*			ipfilter;
	CWebServer*			webserver;
	CScheduler*			scheduler;
	CMMServer*			mmserver;
	CPeerCacheFinder*	m_pPeerCache;
	CFirewallOpener*	m_pFirewallOpener;
	CTBHMM*				minimule; // [TPT] - TBH: minimule
	CIP2Country*		ip2country; // [TPT] - IP Country
	CFakecheck*			FakeCheck; // [TPT] - Fakecheck

	// [TPT] - Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	CBandWidthControl*	pBandWidthControl;
	// Maella end

	HANDLE				m_hMutexOneInstance;	
	int					m_iDfltImageListColorFlags;
	CFont				m_fontHyperText;
	CFont				m_fontDefaultBold;
	CFont				m_fontSymbol;
	CFont				m_fontLog;
	CBrush				m_brushBackwardDiagonal;
	DWORD				m_dwProductVersionMS;
	DWORD				m_dwProductVersionLS;
	CString				m_strCurVersionLong;
	UINT				m_uCurVersionShort;
	UINT				m_uCurVersionCheck;
	ULONGLONG			m_ullComCtrlVer;
	AppState			m_app_state; // defines application state for shutdown 
	CMutex				hashing_mut;
	CString*			pendinglink;
	COPYDATASTRUCT		sendstruct;

// Implementierung
	virtual BOOL InitInstance();

	// ed2k link functions
	// [TPT] - MoNKi: -Check already downloaded files-
	//void		AddEd2kLinksToDownload(CString strLinks, uint8 cat);
	// [TPT] - khaos::categorymod+ Changed Param: uint8 cat
	void	AddEd2kLinksToDownload(CString strlink,int theCat = -1);
	// [TPT] - khaos::categorymod-
	// [TPT] - MoNKi: -Check already downloaded files-
	void		SearchClipboard();
	void		IgnoreClipboardLinks(CString strLinks) {m_strLastClipboardContents = strLinks;}
	void		PasteClipboard(uint8 uCategory = 0);
	bool		IsEd2kFileLinkInClipboard();
	bool		IsEd2kServerLinkInClipboard();
	bool		IsEd2kLinkInClipboard(LPCSTR pszLinkType, int iLinkTypeLen);

	CString		CreateED2kSourceLink(const CAbstractFile* f);
//	CString		CreateED2kHostnameSourceLink(const CAbstractFile* f);
	CString		CreateKadSourceLink(const CAbstractFile* f);

	// clipboard (text)
	bool		CopyTextToClipboard(CString strText);
	CString		CopyTextFromClipboard();

	void		OnlineSig();
	// [TPT] - not used
	//void		UpdateReceivedBytes(uint32 bytesToAdd);
	//void		UpdateSentBytes(uint32 bytesToAdd, bool sentToFriend = false);
	int			GetFileTypeSystemImageIdx(LPCTSTR pszFilePath, int iLength = -1);
	HIMAGELIST	GetSystemImageList() { return m_hSystemImageList; }
	CSize		GetSmallSytemIconSize() { return m_sizSmallSystemIcon; }
	void		CreateBackwardDiagonalBrush();
	void		CreateAllFonts();
	bool		IsPortchangeAllowed();
	bool		IsConnected();
	bool		IsFirewalled();
	bool		DoCallback( CUpDownClient *client );
	uint32		GetID();
	uint32		GetPublicIP() const;	// return current (valid) public IP or 0 if unknown
	void		SetPublicIP(const uint32 dwIP);

	// because nearly all icons we are loading are 16x16, the default size is specified as 16 and not as 32 nor LR_DEFAULTSIZE
	HICON		LoadIcon(LPCTSTR lpszResourceName, int cx = 16, int cy = 16, UINT uFlags = LR_DEFAULTCOLOR) const;
	HICON		LoadIcon(UINT nIDResource) const;
	HBITMAP		LoadImage(LPCTSTR lpszResourceName, LPCTSTR pszResourceType) const;
	HBITMAP		LoadImage(UINT nIDResource, LPCTSTR pszResourceType) const;
	bool		LoadSkinColor(LPCTSTR pszKey, COLORREF& crColor) const;
	bool		LoadSkinColorAlt(LPCTSTR pszKey, LPCTSTR pszAlternateKey, COLORREF& crColor) const;
	CString		GetSkinFileItem(LPCTSTR lpszResourceName, LPCTSTR pszResourceType) const;
	void		ApplySkin(LPCTSTR pszSkinProfile);

	bool		GetLangHelpFilePath(CString& strResult);
	void		SetHelpFilePath(LPCTSTR pszHelpFilePath);
	void		ShowHelp(UINT uTopic, UINT uCmd = HELP_CONTEXT);
	bool		ShowWebHelp();

    // Elandal:ThreadSafeLogging -->
    // thread safe log calls
    void			QueueDebugLogLine(bool bAddToStatusBar, LPCTSTR line,...);
    void			QueueDebugLogLineEx(UINT uFlags, LPCTSTR line,...);
    void			HandleDebugLogQueue();
    void			ClearDebugLogQueue(bool bDebugPendingMsgs = false);

	void			QueueLogLine(bool bAddToStatusBar, LPCTSTR line,...);
    void			QueueLogLineEx(UINT uFlags, LPCTSTR line,...);
    void			HandleLogQueue();
    void			ClearLogQueue(bool bDebugPendingMsgs = false);
    // Elandal:ThreadSafeLogging <--

	bool			DidWeAutoStart() { return m_bAutoStart; }

protected:
	bool ProcessCommandline();
	//void SetTimeOnTransfer(); // [TPT]
	static BOOL CALLBACK SearchEmuleWindow(HWND hWnd, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnHelp();

	HIMAGELIST m_hSystemImageList;
	CMapStringToPtr m_aExtToSysImgIdx;
	CSize m_sizSmallSystemIcon;

	bool		m_bGuardClipboardPrompt;
	CString		m_strLastClipboardContents;

    // Elandal:ThreadSafeLogging -->
    // thread safe log calls
    CCriticalSection m_queueLock;
    CTypedPtrList<CPtrList, SLogItem*> m_QueueDebugLog;
    CTypedPtrList<CPtrList, SLogItem*> m_QueueLog;
    // Elandal:ThreadSafeLogging <--

	uint32 m_dwPublicIP;
	bool m_bAutoStart;

public:
	void OptimizerInfo(void); // [TPT] - Optimizer
	WSADATA				m_wsaData; // [TPT] - eWombat [WINSOCK2]
	// [TPT] - Announ: -Copy BBCode ed2k links-
public:
	CString		CreateBBCodeED2kLink(CAbstractFile* f);
	CString		CreateBBCodeED2kSourceLink(CAbstractFile* f);
	CString		StripBrackets(CString strText) const;
	// [TPT] - Announ: -Copy BBCode ed2k links-
	// [TPT] - MoNKi: -UPnPNAT Support-
public:
	CUPnP_IGDControlPoint *m_UPnPNat;
	BOOL		AddUPnPNatPort(CUPnP_IGDControlPoint::UPNPNAT_MAPPING *mapping);
	BOOL		RemoveUPnPNatPort(CUPnP_IGDControlPoint::UPNPNAT_MAPPING *mapping);
	// End -UPnPNAT Support-
	// [TPT] - Announ: -Friend eLinks-
	public:
		bool	IsEd2kFriendLinkInClipboard();
	// End -Friend eLinks-
};

extern CemuleApp theApp;


//////////////////////////////////////////////////////////////////////////////
// CTempIconLoader

class CTempIconLoader
{
public:
	// because nearly all icons we are loading are 16x16, the default size is specified as 16 and not as 32 nor LR_DEFAULTSIZE
	CTempIconLoader(LPCTSTR pszResourceID, int cx = 16, int cy = 16, UINT uFlags = LR_DEFAULTCOLOR);
	CTempIconLoader(UINT uResourceID, int cx = 16, int cy = 16, UINT uFlags = LR_DEFAULTCOLOR);
	~CTempIconLoader();

	operator HICON() const{
		return this == NULL ? NULL : m_hIcon;
	}

protected:
	HICON m_hIcon;
};
