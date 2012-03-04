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
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#pragma once
#include "OtherFunctions.h" // [TPT]
#define	DFLT_TRANSFER_WND2	1

const CString strDefaultToolbar = _T("0099010203040506070899091011");

enum EViewSharedFilesAccess{
	vsfaEverybody = 0,
	vsfaFriends = 1,
	vsfaNobody = 2
};

enum EToolbarLabelType;


// [TPT] - Select process priority 
#define PROCESSPRIORITYNUMBER 6
static const PriorityClasses[] = { REALTIME_PRIORITY_CLASS, HIGH_PRIORITY_CLASS, ABOVE_NORMAL_PRIORITY_CLASS, NORMAL_PRIORITY_CLASS, BELOW_NORMAL_PRIORITY_CLASS, IDLE_PRIORITY_CLASS };
// [TPT] - Select process priority 


// DO NOT EDIT VALUES like making a uint16 to uint32, or insert any value. ONLY append new vars
#pragma pack(1)
struct Preferences_Ext_Struct{
	uint8	version;
	uchar	userhash[16];
	WINDOWPLACEMENT EmuleWindowPlacement;
};
#pragma pack()

// deadlake PROXYSUPPORT
struct ProxySettings{
	uint16 type;
	uint16 port;
	TCHAR name[50];
	CHAR user[50];
	CHAR password[50];
	bool EnablePassword;
	bool UseProxy;
};

// [TPT] - khaos::categorymod+ View Filter Struct
#pragma pack(1)
struct CategoryViewFilter_Struct{
	//		General View Filters
	uint8	nFromCats;  // 0 == All; 1 == Unassigned; 2 == This Cat Only
	bool	bSuspendFilters;
	//		File Type View Filters
	bool	bVideo;
	bool	bAudio;
	bool	bArchives;
	bool	bImages;
	//		File State View Filters
	bool	bWaiting;
	bool	bTransferring;
	bool	bPaused;
	bool	bStopped;
	bool	bComplete;
	bool	bHashing;
	bool	bErrorUnknown;
	bool	bCompleting;
	//		File Size View Filters
	uint32	nFSizeMin;
	uint32	nFSizeMax;
	uint32	nRSizeMin;
	uint32	nRSizeMax;
	//		Time Remaining Filters
	uint32	nTimeRemainingMin;
	uint32	nTimeRemainingMax;
	//		Source Count Filters
	uint16	nSourceCountMin;
	uint16	nSourceCountMax;
	uint16	nAvailSourceCountMin;
	uint16	nAvailSourceCountMax;
	//		Advanced Filter Mask
	CString	sAdvancedFilterMask;
};
#pragma pack()

// Criteria Selection Struct
#pragma pack(1)
struct CategorySelectionCriteria_Struct{
	bool	bFileSize;
	bool	bAdvancedFilterMask;
};
#pragma pack()
// [TPT] - khaos::categorymod-

#pragma pack(1)
struct Category_Struct{
	TCHAR	incomingpath[MAX_PATH];
	TCHAR	title[64];
	TCHAR	comment[255];
	DWORD	color;
	uint8	prio;
	//CString autocat; // [TPT] - khaos::categorymod
	BOOL    downloadInLinealPriorityOrder; // [TPT] - ZZ:DownloadManager	
	// [TPT] - khaos:kmod+ Category
	// View Filter Struct
	CategoryViewFilter_Struct viewfilters;
	CategorySelectionCriteria_Struct selectioncriteria;
	char	autoCat[255];
	// [TPT] - khaos:kmod-
};
#pragma pack()

class CPreferences
{
public:
	static	CString	strNick;
	// [TPT] - Sivka AutoHL Begin
	static	bool	m_MaxSourcesPerFileTakeOver;
	static	uint16	m_MaxSourcesPerFileTemp;
	static	bool	m_TakeOverFileSettings;
	// [TPT] - Sivka AutoHL end

	// Maella 
	static float	maxupload;                    // [FAF] -Allow Bandwidth Settings in <1KB Incremements-
	static float	maxdownload;
	// ZZ:UploadSpeedSense -->
	static float	minupload;
	// ZZ:UploadSpeedSense <--
	static float	maxGraphDownloadRate;
	static float	maxGraphUploadRate;
	static bool	disablexs;                    // -Enable/Disable source exchange in preference- (Tarod)
	static bool	reaskSourceAfterIPChange;     // -Reask sources after IP change- (idea Xman)
	static uint8	zoomFactor;                   // -Graph: display zoom-
	static bool	blockUploadEndMsg;            // -Filter verbose messages-
	static bool	blockPhoenixMsg;			  // [TPT] - Filter own messages
	static bool	blockHandshakeMsg;			  // [TPT] - Filter Handshake messages
	static bool blockDeadSources;			  // [TPT] - Filter dead sources	
		
	static bool    blockMaellaSpecificMsg;
	static bool    blockMaellaSpecificDebugMsg;
	static bool	compensateOverhead;           // -Overhead compensation (pseudo full rate control)-
	static uint16	MTU;                          // -MTU Configuration-
	static uint16	minUploadSlot;                // -Minimum Upload Slot-
	static uint16	sendSocketBufferSize;         // -New bandwidth control-
	static bool	NAFCEnable;					  // -Network Adapter Feedback Control-
	static bool	NAFCFullControl;	          // -Network Adapter Feedback Control-
	static float	NAFCNetworkOut;	              // -Network Adapter Feedback Control-
	static uint8    NAFCSlider;
	static uint8	datarateSamples;              // -Accurate measure of bandwidth: eDonkey data + control, network adapter-	
	static bool    enableMultiQueue;             // -One-queue-per-file- (idea bloodymad)
	static bool    enableReleaseMultiQueue;
	static bool    enableNewUSS;	
	static bool    enableA4AFTransfer;
	// Maella end
	
	static bool m_bShowInMSN7;//[TPT] - Show in MSN7
	static bool	m_bfadeOut;
	static bool m_bUseColorOnClients;
	static int m_iCreditSystem; // [TPT] - Credit System
	static bool	m_bShowBitmapInMenus;//[TPT]

	// [TPT] - eWombat SNAFU v2
	static bool m_bSnafu;
	static bool m_bAntiCreditTheft;
	static bool m_bAntiFriendshare;
	
	//[TPT] - Show Up Priority in downloadlist
	static bool	m_bShowUpPrioInDownloadList;

	//[TPT] - Check Nafc on ID change
	static bool	m_bDinamicNafc;

	// [TPT] - TBH: minimule
	static bool		m_bMiniMule;
	static uint32	m_iMiniMuleUpdate;
	static bool		m_bMiniMuleLives;
	static uint8	m_iMiniMuleTransparency;
	// [TPT] - TBH: minimule
	
	// [TPT] - MoNKi: -UPnPNAT Support-
	static bool	m_bUPnPNat;
	static bool	m_bUPnPNatWeb;	
	static uint16	m_iUPnPTCPExternal;
	static uint16	m_iUPnPUDPExternal;
	static uint16	m_iUPnPTCPInternal;
	static uint16	m_iUPnPUDPInternal;
	static bool		m_bUPnPVerboseLog;
	static uint16	m_iUPnPPort;

	// [TPT] - MoNKi: -UPnPNAT Support-
	
	// ZZ:UploadSpeedSense -->
	//static	uint16	minupload;
	// ZZ:UploadSpeedSense <--
	//static	uint16	maxupload;
	//static	uint16	maxdownload;
	static	uint16	port;
	static	uint16	udpport;
	static	uint16	nServerUDPPort;
	static	uint16	maxconnections;
	static	uint16	maxhalfconnections;
	static	uint8	reconnect;
	static	uint8	scorsystem;
	static	TCHAR	incomingdir[MAX_PATH];
	static	TCHAR	tempdir[MAX_PATH];
	static	uint8	ICH;
	static	uint8	autoserverlist;
	static	uint8	updatenotify;
	static	uint8	mintotray;
	static	uint8	autoconnect;
	static	uint8	autoconnectstaticonly; // Barry
	static	uint8	autotakeed2klinks;	   // Barry
	static	uint8	addnewfilespaused;	   // Barry
	static	uint8	depth3D;			   // Barry
	static	int		m_iStraightWindowStyles;
	static	CString	m_strSkinProfile;
	static	CString	m_strSkinProfileDir;
	static	uint8	addserversfromserver;
	static	uint8	addserversfromclient;
	static	uint16	maxsourceperfile;
	static	uint16	trafficOMeterInterval;
	static	uint16	statsInterval;
	static	uchar	userhash[16];
	static	WINDOWPLACEMENT EmuleWindowPlacement;
	//static int		maxGraphDownloadRate;
	//static int		maxGraphUploadRate;
	static	uint8	beepOnError;
	static	uint8	confirmExit;

	// [TPT] - khaos::categorymod+ +1
	// [TPT] - WebCache +1
	static uint16	downloadColumnWidths[13+1+1];
	static BOOL	downloadColumnHidden[13+1+1];
	static INT		downloadColumnOrder[13+1+1];
	// [TPT] - WebCache +1

	// [TPT] - khaos::categorymod+ +1
	
	// [TPT] - TBH Transfers Window Style
	// [TPT] - IP Country +1
	static uint16	downloadClientsColumnWidths[8+1];
	static BOOL	downloadClientsColumnHidden[8+1];
	static INT		downloadClientsColumnOrder[8+1];
	// [TPT] - IP Country +1
	// [TPT] - TBH Transfers Window Style
	
	// [TPT] - itsonlyme: clientSoft +1	
	// [TPT] - IP Country +1
	// [TPT] - Total UL/DL +1
	static uint16	uploadColumnWidths[8+1+1+1];
	static BOOL	uploadColumnHidden[8+1+1+1];
	static INT		uploadColumnOrder[8+1+1+1];
	// [TPT] - WebCache +1
	static uint16	queueColumnWidths[10+1+1+1+1];
	static BOOL	queueColumnHidden[10+1+1+1+1];
	static INT		queueColumnOrder[10+1+1+1+1];
	// [TPT] - WebCache end
	// [TPT] - Total UL/DL
	// [TPT] - IP Country
	// [TPT] - itsonlyme: clientSoft END

	static	uint16	searchColumnWidths[14+1];// [TPT] - Fakecheck
	static	BOOL	searchColumnHidden[14+1];//14 +1 due
	static	INT		searchColumnOrder[14+1]; //to fakecheck

	// [TPT] - itsonlyme: virtualDirs +1
	// [TPT] -  SLUGFILLER: Spreadbars +4
	// [TPT] - xMule_MOD: showSharePermissions +1
	// [TPT] - Powershare +2
	static uint16	sharedColumnWidths[12+1+1+4+2];
	static BOOL	sharedColumnHidden[12+1+1+4+2];
	static INT		sharedColumnOrder[12+1+1+4+2];
	//TPT] - Powershare END
	// [TPT] - xMule_MOD: showSharePermissions
	// [TPT] -  SLUGFILLER: Spreadbars
	// [TPT] - itsonlyme: virtualDirs

	// [TPT] - Aux Ports +1
	// [TPT] - serverOrder +1
	static uint16	serverColumnWidths[14+1+1];
	static BOOL	serverColumnHidden[14+1+1];
	static INT 	serverColumnOrder[14+1+1];
	// [TPT] - Aux Ports 
	// [TPT] - serverOrder
	
	// [TPT] - IP Country +1
	// [TPT] - eWombat SNAFU v2 +1
	static uint16	clientListColumnWidths[8+1+1];
	static BOOL	clientListColumnHidden[8+1+1];
	static INT 	clientListColumnOrder[8+1+1];
	// [TPT] - eWombat SNAFU v2
	// [TPT] - IP Country
	
	// [TPT] - MoNKi: -Downloaded History-
	static uint16	m_HistoryColumnWidths[7];
	static BOOL	m_HistoryColumnHidden[7];
	static INT 	m_HistoryColumnOrder[7];
	// [TPT] - MoNKi: -Downloaded History-

	// [TPT] - Friend State Column
	static uint16	friendListColumnWidths[2];
	static BOOL	friendListColumnHidden[2];
	static INT		friendListColumnOrder[2];
	// [TPT] - Friend State Column

	static	uint16	FilenamesListColumnWidths[2];
	static	BOOL	FilenamesListColumnHidden[2];
	static	INT		FilenamesListColumnOrder[2];
	static	DWORD	m_adwStatsColors[15];

	static	uint8	splashscreen;
	static	uint8	filterLANIPs;
	static	bool	m_bAllocLocalHostIP;
	static	uint8	onlineSig;

	// -khaos--+++> Struct Members for Storing Statistics

	// Saved stats for cumulative downline overhead...
	static	uint64	cumDownOverheadTotal;
	static	uint64	cumDownOverheadFileReq;
	static	uint64	cumDownOverheadSrcEx;
	static	uint64	cumDownOverheadServer;
	static	uint64	cumDownOverheadKad;
	static	uint64	cumDownOverheadTotalPackets;
	static	uint64	cumDownOverheadFileReqPackets;
	static	uint64	cumDownOverheadSrcExPackets;
	static	uint64	cumDownOverheadServerPackets;
	static	uint64	cumDownOverheadKadPackets;

	// Saved stats for cumulative upline overhead...
	static	uint64	cumUpOverheadTotal;
	static	uint64	cumUpOverheadFileReq;
	static	uint64	cumUpOverheadSrcEx;
	static	uint64	cumUpOverheadServer;
	static	uint64	cumUpOverheadKad;
	static	uint64	cumUpOverheadTotalPackets;
	static	uint64	cumUpOverheadFileReqPackets;
	static	uint64	cumUpOverheadSrcExPackets;
	static	uint64	cumUpOverheadServerPackets;
	static	uint64	cumUpOverheadKadPackets;

	// Saved stats for cumulative upline data...
	static	uint32	cumUpSuccessfulSessions;
	static	uint32	cumUpFailedSessions;
	static	uint32	cumUpAvgTime;
	// Cumulative client breakdown stats for sent bytes...
	static	uint64	cumUpData_EDONKEY;
	static	uint64	cumUpData_EDONKEYHYBRID;
	static	uint64	cumUpData_EMULE;
	static	uint64	cumUpData_MLDONKEY;
	static	uint64	cumUpData_AMULE;
	static	uint64	cumUpData_EMULECOMPAT;
	static	uint64	cumUpData_SHAREAZA;
	// Session client breakdown stats for sent bytes...
	static	uint64	sesUpData_EDONKEY;
	static	uint64	sesUpData_EDONKEYHYBRID;
	static	uint64	sesUpData_EMULE;
	static	uint64	sesUpData_MLDONKEY;
	static	uint64	sesUpData_AMULE;
	static	uint64	sesUpData_EMULECOMPAT;
	static	uint64	sesUpData_SHAREAZA;

	// Cumulative port breakdown stats for sent bytes...
	static	uint64	cumUpDataPort_4662;
	static	uint64	cumUpDataPort_OTHER;
	static	uint64	cumUpDataPort_PeerCache;
	// Session port breakdown stats for sent bytes...
	static	uint64	sesUpDataPort_4662;
	static	uint64	sesUpDataPort_OTHER;
	static	uint64	sesUpDataPort_PeerCache;

	// Cumulative source breakdown stats for sent bytes...
	static	uint64	cumUpData_File;
	static	uint64	cumUpData_Partfile;
	// Session source breakdown stats for sent bytes...
	static	uint64	sesUpData_File;
	static	uint64	sesUpData_Partfile;

	// Saved stats for cumulative downline data...
	static	uint32	cumDownCompletedFiles;
	static	uint32	cumDownSuccessfulSessions;
	static	uint32	cumDownFailedSessions;
	static	uint32	cumDownAvgTime;

	// Cumulative statistics for saved due to compression/lost due to corruption
	static	uint64	cumLostFromCorruption;
	static	uint64	cumSavedFromCompression;
	static	uint32	cumPartsSavedByICH;

	// Session statistics for download sessions
	static	uint32	sesDownSuccessfulSessions;
	static	uint32	sesDownFailedSessions;
	static	uint32	sesDownAvgTime;
	static	uint32	sesDownCompletedFiles;
	static	uint64	sesLostFromCorruption;
	static	uint64	sesSavedFromCompression;
	static	uint32	sesPartsSavedByICH;

	// Cumulative client breakdown stats for received bytes...
	static	uint64	cumDownData_EDONKEY;
	static	uint64	cumDownData_EDONKEYHYBRID;
	static	uint64	cumDownData_EMULE;
	static	uint64	cumDownData_MLDONKEY;
	static	uint64	cumDownData_AMULE;
	static	uint64	cumDownData_EMULECOMPAT;
	static	uint64	cumDownData_SHAREAZA;
	static	uint64	cumDownData_URL;
	static	uint64	cumDownData_WEBCACHE; // [TPT] - WebCache	//jp webcache statistics
	// Session client breakdown stats for received bytes...
	static	uint64	sesDownData_EDONKEY;
	static	uint64	sesDownData_EDONKEYHYBRID;
	static	uint64	sesDownData_EMULE;
	static	uint64	sesDownData_MLDONKEY;
	static	uint64	sesDownData_AMULE;
	static	uint64	sesDownData_EMULECOMPAT;
	static	uint64	sesDownData_SHAREAZA;
	static	uint64	sesDownData_URL;
	// [TPT] - WebCache	
	static	uint64	sesDownData_WEBCACHE; //jp webcache statistics
	static  uint32	ses_WEBCACHEREQUESTS; //jp webcache statistics
	static	uint32	ses_successfull_WCDOWNLOADS;  //jp webcache statistics
	static  uint32	ses_PROXYREQUESTS; //jp webcache statistics
	static  uint32	ses_successfullPROXYREQUESTS;//jp webcache statistics
	// [TPT] - WebCache	

	// Cumulative port breakdown stats for received bytes...
	static	uint64	cumDownDataPort_4662;
	static	uint64	cumDownDataPort_OTHER;
	static	uint64	cumDownDataPort_PeerCache;
	// Session port breakdown stats for received bytes...
	static	uint64	sesDownDataPort_4662;
	static	uint64	sesDownDataPort_OTHER;
	static	uint64	sesDownDataPort_PeerCache;

	// Saved stats for cumulative connection data...
	static	float	cumConnAvgDownRate;
	static	float	cumConnMaxAvgDownRate;
	static	float	cumConnMaxDownRate;
	static	float	cumConnAvgUpRate;
	static	float	cumConnMaxAvgUpRate;
	static	float	cumConnMaxUpRate;
	static	uint64	cumConnRunTime;
	static	uint32	cumConnNumReconnects;
	static	uint32	cumConnAvgConnections;
	static	uint32	cumConnMaxConnLimitReached;
	static	uint32	cumConnPeakConnections;
	static	uint32	cumConnTransferTime;
	static	uint32	cumConnDownloadTime;
	static	uint32	cumConnUploadTime;
	static	uint32	cumConnServerDuration;

	// Saved records for servers / network...
	static	uint32	cumSrvrsMostWorkingServers;
	static	uint32	cumSrvrsMostUsersOnline;
	static	uint32	cumSrvrsMostFilesAvail;

	// Saved records for shared files...
	static	uint32	cumSharedMostFilesShared;
	static	uint64	cumSharedLargestShareSize;
	static	uint64	cumSharedLargestAvgFileSize;
	static	uint64	cumSharedLargestFileSize;

	// Save the date when the statistics were last reset...
	static	__int64 stat_datetimeLastReset;

	// Save new preferences for PPgStats
	static	uint8	statsConnectionsGraphRatio; // This will store the divisor, i.e. for 1:3 it will be 3, for 1:20 it will be 20.
	// Save the expanded branches of the stats tree
	static	TCHAR	statsExpandedTreeItems[256];

	static	UINT	statsSaveInterval;
	static  bool	m_bShowVerticalHourMarkers;
	// <-----khaos- End Statistics Members


	// Original Stats Stuff
	static	uint64	totalDownloadedBytes;
	static	uint64	totalUploadedBytes;
	// End Original Stats Stuff
	static	WORD	m_wLanguageID;
	static	uint8	transferDoubleclick;
	static	EViewSharedFilesAccess m_iSeeShares;
	static	uint8	m_iToolDelayTime;	// tooltip delay time in seconds
	static	uint8	bringtoforeground;
	static	uint8	splitterbarPosition;
	static	uint8	m_uTransferWnd2;
	//MORPH START - Added by SiRoB, Splitting Bar [O²]
	static	uint8	splitterbarPositionStat;
	static	uint8	splitterbarPositionStat_HL;
	static	uint8	splitterbarPositionStat_HR;
	static	uint16	splitterbarPositionFriend;
	static	uint16	splitterbarPositionIRC;
	//MORPH END - Added by SiRoB, Splitting Bar [O²]
	static	uint16	deadserverretries;
	static	DWORD	m_dwServerKeepAliveTimeout;
	// -khaos--+++> Changed data type to avoid overflows
	static	uint16	statsMax;
	// <-----khaos-
	static	uint8	statsAverageMinutes;

	// [TPT] - enkeyDEV(th1) -notifier-
	static uint8    useNotifierUserTimings;
	static uint16  notifierUserTimeToShow;
	static uint16  notifierUserTimeToStay;
	static uint16  notifierUserTimeToHide;
	static uint8    notifierLessFramerate;
	static	uint8	useDownloadNotifier;
	static uint8	notifierAutoClose;  
	static uint8	useErrorNotifier;	// added by InterCeptor (notify on error) 11.11.02
	static	uint8	useNewDownloadNotifier;
	static	uint8	useChatNotifier;
	static	uint8	useLogNotifier;
	static	uint8	useSoundInNotifier;
	static	uint8	notifierPopsEveryChatMsg;
	static	uint8	notifierImportantError;
	static	uint8	notifierNewVersion;
	static	TCHAR	notifierSoundFilePath[510];
	static uint8	notifierSearchCompleted;    	
	static uint8	notifierNewPvtMsg;//Rocks
	//END [TPT] - enkeyDEV(th1) -notifier-
	
	static	TCHAR	m_sircserver[50];
	static	TCHAR	m_sircnick[30];
	static	TCHAR	m_sircchannamefilter[50];
	static	bool	m_bircaddtimestamp;
	static	bool	m_bircusechanfilter;
	static	uint16	m_iircchanneluserfilter;
	static	TCHAR	m_sircperformstring[255];
	static	bool	m_bircuseperform;
	static	bool	m_birclistonconnect;
	static	bool	m_bircacceptlinks;
	static	bool	m_bircacceptlinksfriends;
	static	bool	m_bircsoundevents;
	static	bool	m_bircignoremiscmessage;
	static	bool	m_bircignorejoinmessage;
	static	bool	m_bircignorepartmessage;
	static	bool	m_bircignorequitmessage;
	static	bool	m_bircignoreemuleprotoaddfriend;
	static	bool	m_bircallowemuleprotoaddfriend;
	static	bool	m_bircignoreemuleprotosendlink;
	static	bool	m_birchelpchannel;

	static	bool	m_bRemove2bin;
	static	bool	m_bShowCopyEd2kLinkCmd;
	static	bool	m_bpreviewprio;
	static	bool	smartidcheck;
	static	uint8	smartidstate;
	static	bool	safeServerConnect;
	static	bool	startMinimized;
	static	bool	m_bAutoStart;
	static	bool	m_bRestoreLastMainWndDlg;
	static	int		m_iLastMainWndDlgID;
	static	bool	m_bRestoreLastLogPane;
	static	int		m_iLastLogPaneID;
	static	uint16	MaxConperFive;
	static	int		checkDiskspace; // SLUGFILLER: checkDiskspace
	static	UINT	m_uMinFreeDiskSpace;
	static	bool	m_bSparsePartFiles;
	static	CString	m_strYourHostname;
	static bool	infiniteQueue;	// [TPT] - SLUGFILLER: infiniteQueue
	// [TPT] - SLUGFILLER: hideOS
	static uint8	hideOS;
	static bool	selectiveShare;
	// [TPT] - SLUGFILLER: hideOS
	// [TPT] - itsonlyme: displayOptions START
	static bool	showFileSystemIcon;
	static bool	showLocalRating; // [TPT] - SLUGFILLER: showComments
	// [TPT] - itsonlyme: displayOptions END
	static DWORD m_currentAdapterIndex; // [TPT] - NAFC Selection
	// [TPT] - quick start
	static bool	m_QuickStart;
	static uint16  m_QuickStartMaxCon;
	static uint16  m_QuickStartMaxConPerFive;
	// [TPT] - quick start
	static bool	m_bSpreadBars; // [TPT] - SLUGFILLER: Spreadbars
	static bool m_bNAFCGraph;

	static	bool	m_bEnableVerboseOptions;
	static	bool	m_bVerbose;
	static	bool	m_bFullVerbose;
	static	bool	m_bDebugSourceExchange; // Sony April 23. 2003, button to keep source exchange msg out of verbose log
	static	bool	m_bLogBannedClients;
	static	bool	m_bLogRatingDescReceived;
	static	bool	m_bLogSecureIdent;
	static	bool	m_bLogFilteredIPs;
	static	bool	m_bLogFileSaving;
    static  bool    m_bLogA4AF; // ZZ:DownloadManager
	static	bool	m_bLogUlDlEvents;
	// [TPT] - WebCache
	static	bool	m_bLogWebCacheEvents;//JP log webcache events
	static	bool	m_bLogICHEvents;//JP log ICH events
	static	bool	m_bUseDebugDevice;
	static	int		m_iDebugServerTCPLevel;
	static	int		m_iDebugServerUDPLevel;
	static	int		m_iDebugServerSourcesLevel;
	static	int		m_iDebugServerSearchesLevel;
	static	int		m_iDebugClientTCPLevel;
	static	int		m_iDebugClientUDPLevel;
	static	int		m_iDebugClientKadUDPLevel;
	static	bool	m_bupdatequeuelist;
	static	bool	m_bmanualhighprio;
	static	bool	m_btransferfullchunks;
	static	int		m_istartnextfile;
	static	bool	m_bshowoverhead;
	static	bool	m_bDAP;
	static	bool	m_bUAP;
	static	bool	m_bDisableKnownClientList;
	static	bool	m_bDisableQueueList;
	static	bool	m_bExtControls;
	static	bool	m_bTransflstRemain;
	static	bool	m_bMemoryConsumingGraph;// [TPT] - Memory Consuming
	static	bool	m_bOverheadGraph;
	static	uint8	versioncheckdays;
	// [TPT] - TBH-AutoBackup Begin
	static bool	autobackup;
	static bool	autobackup2;
	// [TPT] - TBH-AutoBackup End

	// Barry - Provide a mechanism for all tables to store/retrieve sort order

	// [TPT] - SLUGFILLER: multiSort - save multiple params
	// [TPT] - SLUGFILLER: DLsortFix - double, for client-only sorting
	
	// [TPT] - khaos::categorymod +1
	// [TPT] - WebCache +1
	static int		tableSortItemDownload[30];
	static bool	tableSortAscendingDownload[30];
	// [TPT] - WebCache +1
	// [TPT] - khaos::categorymod

	// [TPT] - IP Country +1
	static int		tableSortItemDownloadClient[18]; // [TPT] - TBH Transfers Window Style		
	static bool	tableSortAscendingDownloadClient[18]; // [TPT] - TBH Transfers Window Style	
	// [TPT] - IP Country
	// [TPT] - SLUGFILLER: DLsortFix

	// [TPT] - itsonlyme: clientSoft +1	
	// [TPT] - IP Country +1
	// [TPT] - Total UL/DL +1
	static int		tableSortItemUpload[11];
	static bool	tableSortAscendingUpload[11];	
	// [TPT] - WebCache +1
	static int		tableSortItemQueue[13+1];
	static bool	tableSortAscendingQueue[13+1];
	// [TPT] - WebCache
	// [TPT] - IP Country
	// [TPT] - itsonlyme: clientSoft

	static int		tableSortItemSearch[15];
	static bool	tableSortAscendingSearch[15];

	// [TPT] - itsonlyme: virtualDirs +1
	// [TPT] -  SLUGFILLER: Spreadbars +4
	// [TPT] - xMule_MOD: showSharePermissions +1
	static int		tableSortItemShared[12+1+1+4+2];
	static bool	tableSortAscendingShared[12+1+1+4+2];
	// [TPT] - xMule_MOD: showSharePermissions
	// [TPT] - itsonlyme: virtualDirs
	// [TPT] -  SLUGFILLER: Spreadbars

	// [TPT] - Aux Ports +1
	// [TPT] - serverOrder +1
	static int		tableSortItemServer[14+1+1];
	static bool	tableSortAscendingServer[14+1+1];
	// [TPT] - Aux Ports
	// [TPT] - serverOrder

	// [TPT] - IP Country +1
	// [TPT] - eWombat SNAFU v2 +1
	static int		tableSortItemClientList[8+1+1];
	static bool	tableSortAscendingClientList[8+1+1];
	// [TPT] - eWombat SNAFU v2
	// [TPT] - IP Country

	// [TPT] - MoNKi: -Downloaded History-
	static int		tableSortItemHistory[7];
	static bool	tableSortAscendingHistory[7];
	// [TPT] - MoNKi: -Downloaded History-

	// [TPT] - Friend State Column
	static int	tableSortItemFriendList[2];
	static bool	tableSortAscendingFriendList[2];
	// [TPT] - Friend State Column
	
	static  int		tableSortItemFilenames[2];
	static  bool	tableSortAscendingFilenames[2];
	// [TPT] - SLUGFILLER: multiSort	

	static	bool	showRatesInTitle;

	static	TCHAR	TxtEditor[256];
	static	TCHAR	VideoPlayer[256];
	static	bool	moviePreviewBackup;
	static	int		m_iPreviewSmallBlocks;
	static	int		m_iPreviewCopiedArchives;
	static	int		m_iInspectAllFileTypes;
	static	bool	m_bPreviewOnIconDblClk;
	static	bool	indicateratings;
	static	bool	watchclipboard;
	static	bool	filterserverbyip;
	static	bool	m_bFirstStart;
	static	bool	m_bCreditSystem;
	// [TPT] - TBH: minimule
	static int		speedmetermin;
	static int		speedmetermax;
	// [TPT] - TBH: minimule
	static	bool	log2disk;
	static	bool	debug2disk;
	static	int		iMaxLogBuff;
	static	UINT	uMaxLogFileSize;
	static	bool	scheduler;
	static	bool	dontcompressavi;
	static	bool	msgonlyfriends;
	static	bool	msgsecure;

	static	uint8	filterlevel;
	static	UINT	m_iFileBufferSize;
	static	UINT	m_iQueueSize;
	static	int		m_iCommitFiles;

	static  bool	m_bHistoryShowShared; // [TPT] - MoNKi: -Downloaded History-
	static	bool	m_bDBStyle;//[TPT] - Double buffer style in lists

	static	uint16	maxmsgsessions;
	static	uint32	versioncheckLastAutomatic;
	static	TCHAR	messageFilter[512];
	static	CString	commentFilter;
	static	TCHAR	filenameCleanups[512];
	static	TCHAR	notifierConfiguration[510];
	static	TCHAR	datetimeformat[64];
	static	TCHAR	datetimeformat4log[64];
	static	LOGFONT m_lfHyperText;
	static	LOGFONT m_lfLogText;
	static	COLORREF m_crLogError;
	static	COLORREF m_crLogWarning;
	static	COLORREF m_crLogSuccess;
	static	int		m_iExtractMetaData;
	static	bool	m_bAdjustNTFSDaylightFileTime;

	// Web Server [kuchin]
	static	TCHAR	m_sWebPassword[256];
	static	TCHAR	m_sWebLowPassword[256];
	static	uint16	m_nWebPort;
	static	bool	m_bWebEnabled;
	static	bool	m_bWebUseGzip;
	static	int		m_nWebPageRefresh;
	static	bool	m_bWebLowEnabled;
	static	TCHAR	m_sWebResDir[MAX_PATH];
	static	int		m_iWebTimeoutMins;

	static	TCHAR	m_sTemplateFile[MAX_PATH];
	static	ProxySettings proxy; // deadlake PROXYSUPPORT
	static	bool	m_bIsASCWOP;
	static	bool	m_bShowProxyErrors;

	static	bool	showCatTabInfos;
	static	bool	resumeSameCat;
	static	bool	dontRecreateGraphs;
	static	bool	autofilenamecleanup;
	static	bool	m_bUseAutocompl;
	static	bool	m_bShowDwlPercentage;
	static	bool	m_bRemoveFinishedDownloads;
	static	uint16	m_iMaxChatHistory;
	static	bool	m_bShowActiveDownloadsBold;

	static	int		m_iSearchMethod;
	static	bool	m_bAdvancedSpamfilter;
	static	bool	m_bUseSecureIdent;
	// mobilemule
	static	TCHAR	m_sMMPassword[256];
	static	bool	m_bMMEnabled;
	static	uint16	m_nMMPort;

	static	bool	networkkademlia;
	static	bool	networked2k;

	// toolbar
	static	EToolbarLabelType m_nToolbarLabels;
	static	CString	m_sToolbarBitmap;
	static	CString	m_sToolbarBitmapFolder;
	static	CString	m_sToolbarSettings;
	static	bool	m_bReBarToolbar;
	static	CSize	m_sizToolbarIconSize;

	//preview
	static	bool	m_bPreviewEnabled;

    // [TPT] - Powershare
	static uint8	m_iPowershareMode;
	static uint8	PowerShareLimit;
    // [TPT] - Powershare END

    static bool	m_bSaveUploadQueueWaitTime; // [TPT] - SUQWT
	static bool	m_bUnlimitedUP;//[TPT] - Unlimited upload with no downloads

	// ZZ:UploadSpeedSense -->
	static	bool	m_bDynUpEnabled;
	static	int		m_iDynUpPingTolerance;
	static	int		m_iDynUpGoingUpDivider;
	static	int		m_iDynUpGoingDownDivider;
	static	int		m_iDynUpNumberOfPings;
	static  int		m_iDynUpPingToleranceMilliseconds;
	static  bool	m_bDynUpUseMillisecondPingTolerance;
	// ZZ:UploadSpeedSense <--

    static bool     m_bA4AFSaveCpu; // ZZ:DownloadManager

	static	CStringList shareddir_list;
	static	CStringList adresses_list;

	static bool	cumulateBandwidth; // [TPT]-Cumulate Bandwidth
	static bool minimizeSlots; // [TPT] - Pawcio: MUS
	static bool manageConnection; // [TPT] - Manage Connection
	// added by [TPT]-MoNKi [MoNKi: -invisible mode-]
	static bool		m_bInvisibleMode;		
	static UINT		m_iInvisibleModeHotKeyModifier;
	static TCHAR	m_cInvisibleModeHotKey;
	// End [TPT]-MoNKi

	// [TPT] - IP Country
	static bool showFlags;
	static bool showCountryName;
	// [TPT] - IP Country

	// [TPT] - Fakecheck
	static TCHAR	UpdateURLFakeList[256];
	static uint32	m_FakesDatVersion;
	static bool		UpdateFakeStartup;
	// [TPT] - Fakecheck END

	// [TPT] - Manual eMfriend.met download
	static TCHAR	m_updateURLFriendList[256];
	// [TPT] - Manual eMfriend.met download

	static bool	m_bUseWindowsTextColorOnHighContrast; // [TPT] - MoNKi: -Support for High Contrast Mode-
	
	// [TPT] - khaos::categorymod+	
	static bool		m_bShowCatNames;
	static bool		m_bActiveCatDefault;
	static bool		m_bSelCatOnAdd;
	static bool		m_bAutoSetResumeOrder;	
	static uint8		m_iStartDLInEmptyCats;
	static bool		m_bRespectMaxSources;
	static bool		m_bUseAutoCat;
	static bool		m_bShowPriorityInTab;
	// [TPT] - khaos::categorymod-

	static	int		m_iDbgHeap;
	static	uint8	m_nWebMirrorAlertLevel;
	static	bool	m_bRunAsUser;

	static  bool	m_bUseOldTimeRemaining;

	// PeerCache
	static	uint32	m_uPeerCacheLastSearch;
	static	bool	m_bPeerCacheWasFound;
	static	bool	m_bPeerCacheEnabled;
	static	uint16	m_nPeerCachePort;
	static	bool	m_bPeerCacheShow;

	// Firewall settings
	static bool		m_bOpenPortsOnStartUp;
	
	//AICH Options
	static bool		m_bTrustEveryHash;

	static uint8	m_byLogLevel;

	// [TPT] - WebCache	
	// yonatan http start 
	// Superlexx - webcache
	static	bool	m_bHighIdPossible; // JP detect fake HighID (from netfinity)
	static	bool	WebCacheDisabledThisSession; //JP temp disabler
	static	uint32	WebCachePingSendTime;//jp check proxy config
	static	bool	expectingWebCachePing;//jp check proxy config
	static	bool	IsWebCacheTestPossible(); //jp check proxy config
	static	CString	webcacheName;		//jp move these to private?? and make member functions to set and change them??
	static	uint16	webcachePort;
	static	bool	webcacheReleaseAllowed; //jp webcache release
	static	bool	IsWebcacheReleaseAllowed() {return webcacheReleaseAllowed;}//jp webcache release
	static	bool	UpdateWebcacheReleaseAllowed();//jp webcache release
	static	bool	WebCacheIsTransparent() {return webcacheName.GetLength() > 15 && webcacheName.Left(12) == "transparent@";}
	static	uint16	webcacheBlockLimit;
	static	void	SetWebCacheBlockLimit(uint16 limit) {webcacheBlockLimit = limit;}
	static	uint16	GetWebCacheBlockLimit() {return webcacheBlockLimit;}
	static	bool	webcacheExtraTimeout;
	static	bool	PersistentConnectionsForProxyDownloads;
	static	bool	WCAutoupdate;
	static	void	SetWebCacheExtraTimeout(bool value) {webcacheExtraTimeout = value;}
	static	bool	GetWebCacheExtraTimeout() {return webcacheExtraTimeout;}
	static	bool	webcacheCachesLocalTraffic;
	static	void	SetWebCacheCachesLocalTraffic(bool value) {webcacheCachesLocalTraffic = value;}
	static	bool	GetWebCacheCachesLocalTraffic() {return webcacheCachesLocalTraffic;}
	static	bool	webcacheEnabled;
	static	bool	IsWebCacheDownloadEnabled() {return webcacheEnabled && !WebCacheDisabledThisSession;} //jp
	static	bool	UsesCachedTCPPort();	//jp
	static	bool	detectWebcacheOnStart; // jp detect webcache on startup
	static	uint32	webcacheLastSearch;
	static	void	SetWebCacheLastSearch(uint32 time) {webcacheLastSearch = time;}
	static	uint32	GetWebCacheLastSearch() {return webcacheLastSearch;}
	static	uint32	webcacheLastGlobalIP;
	static	void	SetWebCacheLastGlobalIP(uint32 IP) {webcacheLastGlobalIP = IP;}
	static	uint32	GetWebCacheLastGlobalIP() {return webcacheLastGlobalIP;}
	static	CString	webcacheLastResolvedName;
	static	void	SetLastResolvedName(CString name) {webcacheLastResolvedName = name;}
	static	CString	GetLastResolvedName()	{return webcacheLastResolvedName;}
	// Superlexx end
	static	uint8	webcacheTrustLevel;
	// Superlexx end 
	// yonatan http end 
	// [TPT] - WebCache	

	// [TPT] - MoNKi: -Downloaded History-
	// [TPT] - TBH Transfers Window Style

	enum Table
	{
		tableDownload, 
		tableDownloadClients, 
		tableUpload, 
		tableQueue, 
		tableSearch,
		tableShared, 
		tableServer, 
		tableClientList,
		tableFilenames,
		tableHistory,
		tableFriendList // [TPT] - Friend State Column
	};
	// [TPT] - MoNKi: -Downloaded History-

	friend class CPreferencesWnd;
	friend class CPPgGeneral;
	friend class CPPgConnection;
	friend class CPPgServer;
	friend class CPPgDirectories;
	friend class CPPgFiles;
	friend class CPPgNotify;
	friend class CPPgIRC;
	friend class Wizard;
	friend class CPPgTweaks;
	friend class CPPgDisplay;
	friend class CPPgSecurity;
	friend class CPPgScheduler;
	friend class CPPgDebug;
	friend class CPPgMaella; // [TPT]
	friend class CPPgPhoenix; // [TPT]
	friend class CHardLimit; // [TPT] - Sivka AutoHL


	// Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
	static float	GetMaxUpload() {return maxupload;}
	static void	SetMaxUpload(float in) {maxupload = in;}
	
	static float	GetMaxDownload(); // rate limited
	static void	SetMaxDownload(float in) {maxdownload = in;}

	static float	GetMaxGraphUploadRate() {return maxGraphUploadRate;}
	static void	SetMaxGraphUploadRate(float in) {maxGraphUploadRate=in;}

	static float	GetMaxGraphDownloadRate() {return maxGraphDownloadRate;}
	static void	SetMaxGraphDownloadRate(float in) {maxGraphDownloadRate=in;}
	// Maella end

	// Maella -Enable/Disable source exchange in preference- (Tarod)
	static bool	GetDisabledXS() { return disablexs; } 
	static void 	SetDisabledXS(bool flag) { disablexs = flag; } 
	// Maella end

	// Maella -Reask sources after IP change- (idea Xman)
	static bool	GetReaskSourceAfterIPChange() { return reaskSourceAfterIPChange; } 
	static void 	SetReaskSourceAfterIPChange(bool flag) { reaskSourceAfterIPChange = flag; } 
	// Maella end

	// Maella -Graph: display zoom-
	static uint8	GetZoomFactor() { return zoomFactor; }
	static void	SetZoomFactor(uint8 zoom) { zoomFactor = zoom; }
	// Maella end

	// Maella -Filter verbose messages-
	static bool	GetBlockUploadEndMsg() { return blockUploadEndMsg; }
	static void	SetBlockUploadEndMsg(bool flag) { blockUploadEndMsg = flag; }		

	// [TPT] - Filter own messages
	static bool	GetBlockPhoenixMsg() { return blockPhoenixMsg; }
	static void	SetBlockPhoenixMsg(bool flag) { blockPhoenixMsg = flag; }		
	
	// [TPT] - Filter Handshake messages
	static bool	GetBlockHandshakeMsg() { return blockHandshakeMsg; }
	static void	SetBlockHandshakeMsg(bool flag) { blockHandshakeMsg= flag; }		
		
	// [TPT] - Filter dead sources
	static bool	GetBlockDeadSourcesMsg() { return blockDeadSources; }
	static void	SetBlockDeadSourcesMsg(bool flag) { blockDeadSources= flag; }		
			
	static bool	GetBlockMaellaSpecificMsg() { return blockMaellaSpecificMsg; }
	static void	SetBlockMaellaSpecificMsg(bool flag) { blockMaellaSpecificMsg = flag; }
	static bool	GetBlockMaellaSpecificDebugMsg() { return blockMaellaSpecificDebugMsg; }
	static void	SetBlockMaellaSpecificDebugMsg(bool flag) { blockMaellaSpecificDebugMsg = flag; }
	// Maella end

	// [TPT] - Memory Consuming
	static bool GetMemoryConsumingGraph()	{ return m_bMemoryConsumingGraph;}
	static void SetMemoryConsumingGraph(bool in)	{ m_bMemoryConsumingGraph = in;}	

	static bool GetNAFCGraph()	{ return m_bNAFCGraph;}
	static void SetNAFCGraph(bool in)	{ m_bNAFCGraph = in;}

	static bool GetOverheadGraph()	{ return m_bOverheadGraph;}
	static void SetOverheadGraph(bool in)	{ m_bOverheadGraph = in;}

	// [TPT] - TBH-AutoBackup Begin
	static	bool    GetAutoBackup()	{ return autobackup;}
	static	bool    GetAutoBackup2()	{ return autobackup2;}
	static	void    SetAutoBackup(bool in) { autobackup = in;}
	static	void    SetAutoBackup2(bool in) { autobackup2 = in;}
	// [TPT] - TBH-AutoBackup ENd


	// Maella -MTU Configuration-
	static uint16	GetMTU() { return MTU; }
	static void	SetMTU(uint16 mtu) { MTU = mtu; }
	// Maella end

	// Maella -Minimum Upload Slot-
	static uint16	GetMinUploadSlot() { return minUploadSlot; }
	static void	SetMinUploadSlot(uint16 minUpSlot) { minUploadSlot = minUpSlot; }
	// Maella end

	// Maella -New bandwidth control-
	static uint16	GetSendSocketBufferSize() { return sendSocketBufferSize; }
	static void	SetSendSocketBufferSize(uint16 size) { sendSocketBufferSize = size; }
	// Maella end

	// Maella -Network Adapter Feedback Control-
	static bool	GetNAFCEnable() { return NAFCEnable; }
	static void	SetNAFCEnable(bool flag) { NAFCEnable = flag; }
	static bool	GetNAFCFullControl() { return NAFCFullControl; }
	static void	SetNAFCFullControl(bool flag) { NAFCFullControl = flag; }
	static float	GetNAFCNetworkOut() { return NAFCNetworkOut; }
	static void	SetNAFCNetworkOut(float rate) { NAFCNetworkOut = rate; }
	static uint8	GetNAFCSlider() { return NAFCSlider; }
	static void	SetNAFCSlider(uint8 rate) { NAFCSlider = rate; }
	// Maella end

	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	static uint8 GetDatarateSamples() { return datarateSamples; }
	static void  SetDatarateSamples(uint8 samples) { datarateSamples = samples; }
	// Maella end

	// Maella -One-queue-per-file- (idea bloodymad)
	static bool GetEnableMultiQueue() { return enableMultiQueue; }
	static void SetEnableMultiQueue(bool state) { enableMultiQueue = state; }
	static bool GetEnableReleaseMultiQueue() { return enableReleaseMultiQueue; }
	static void SetEnableReleaseMultiQueue(bool state) { enableReleaseMultiQueue = state; }
	static bool GetEnableNewUSS() { return enableNewUSS || (NAFCEnable && NAFCFullControl); }
	static void SetEnableNewUSS(bool state) { enableNewUSS = state; }
	// Maella end

	//[TPT] - Show in MSN7
	static bool	GetShowMSN7()	{return m_bShowInMSN7;}
	static void SetShowMSN7(bool state)	{ m_bShowInMSN7 = state;}
	//[TPT] - Show in MSN7

	//[TPT] - Fadeout on Exit
	static bool GetFadeOut()	{return m_bfadeOut;}
	static void SetFadeOut(bool state)	{ m_bfadeOut = state;}
	//[TPT] - Fadeout on Exit end
	
	// [TPT] - eWombat SNAFU v2
	static void SetEnableSnafu(bool state) { m_bSnafu = state; }
	static void SetEnableACT(bool state) { m_bAntiCreditTheft = state; }
	static void SetEnableAntiFriendshare(bool state) { m_bAntiFriendshare = state; }
	// [TPT] - eWombat SNAFU v2 end
	
	// [TPT] - IP Country
	static bool GetEnableShowCountryNames() { return showCountryName; }
	static bool GetEnableShowCountryFlags() { return showFlags; }	
	static void SetEnableShowCountrNames(bool value) { showCountryName = value; }
	static void SetEnableShowCountryFlags(bool value) { showFlags = value; }
	// [TPT] - IP Country

	//[TPT] - Check Nafc on ID change
	static bool GetDinamicNafc()	{return m_bDinamicNafc;}
	static void SetDinamicNafc(bool in)	{ m_bDinamicNafc = in;}

	// [TPT] - Fakecheck
	static	CString	GetUpdateURLFakeList()				{return CString(UpdateURLFakeList);}
	static	uint32	GetFakesDatVersion()				{return m_FakesDatVersion;}
	static	void	SetFakesDatVersion(uint32 version)	{m_FakesDatVersion = version;} 
	static	bool	IsUpdateFakeStartupEnabled()		{ return UpdateFakeStartup; }
	// [TPT] - Fakecheck end

	// [TPT] - Manual eMfriend.met download
	static	CString GetUpdateURLFriendList()				{return CString(m_updateURLFriendList);}
	static	void	SetUpdateURLFriendList(CString list)	{ _tcscpy(m_updateURLFriendList, list);}
	// [TPT] - Manual eMfriend.met download
	
	static bool	GetQuickStart()	        {return m_QuickStart;} // [TPT] - quick start	

	CPreferences();
	~CPreferences();

	// [TPT] - Sivka AutoHL Begin
	static	bool	GetMaxSourcesPerFileTakeOver(){return m_MaxSourcesPerFileTakeOver;}
	static	uint16	GetMaxSourcesPerFileTemp(){return m_MaxSourcesPerFileTemp;}
	static	void	SetMaxSourcesPerFileTemp(uint16 in){m_MaxSourcesPerFileTemp=in;}
	static	bool	GetTakeOverFileSettings() {return m_TakeOverFileSettings;}
	static	void	SetTakeOverFileSettings(bool in) {m_TakeOverFileSettings=in;}
	// [TPT] - Sivka AutoHL End

	static	void	Init();
	static	void	Uninit();

	static	const CString& GetAppDir()				{return appdir;}
	static	LPCTSTR GetIncomingDir()				{return incomingdir;}
	static	LPCTSTR GetTempDir()					{return tempdir;}
	static	const CString& GetConfigDir()			{return configdir;}
	static	const CString& GetWebServerDir()		{return m_strWebServerDir;}
	static	const CString& GetFileCommentsFilePath(){return m_strFileCommentsFilePath;}
	static	const CString& GetLogDir()				{return m_strLogDir;}
	static  const CString&	GetLinkDir() 			{return ed2klinkdir;}	 // [TPT] - Save ed2klinks
	static  const CString&	GetAutoHLDir() 			{return autoHLdir;}	 // [TPT] - Sivka autoHL
	static  const CString&	GetSaveSourcesDir() 	{return saveSourcesdir;}	 // [TPT] - Save sources

	static	bool	IsTempFile(const CString& rstrDirectory, const CString& rstrName);
	static	bool	IsConfigFile(const CString& rstrDirectory, const CString& rstrName);
	static	bool	IsShareableDirectory(const CString& rstrDirectory);
	static	bool	IsInstallationDirectory(const CString& rstrDir);

	static	bool	Save();
	static	void	SaveCats();
	// [TPT] - itsonlyme: virtualDirs
	static void	LoadVirtualDirs();
	static	void	SaveVirtualDirs();
	// [TPT] - itsonlyme: virtualDirs

	static	uint8	Score()							{return scorsystem;}
	static	bool	Reconnect()						{return reconnect;}
	static	const CString& GetUserNick()			{return strNick;}
	static	void	SetUserNick(LPCTSTR pszNick);
	static	int		GetMaxUserNickLength()			{return 50;}

	// [TPT] - MoNKi: -UPnPNAT Support-
	/*
	static	uint16	GetPort()		{return port;}
	static	uint16	GetUDPPort()	{return udpport;}
	*/
	static	uint16	GetPort(bool newPort = false);
	static	uint16	GetUDPPort(bool newPort = false);
	// [TPT] - MoNKi: -UPnPNAT Support-
	static	uint16	GetServerUDPPort(){return nServerUDPPort;}
	static	uchar*	GetUserHash()	{return userhash;}
	// ZZ:UploadSpeedSense -->
	static	float	GetMinUpload()	{return minupload;} // [TPT]
	// ZZ:UploadSpeedSense <--
	//static	uint16	GetMaxUpload()	{return maxupload;}
	static	bool	IsICHEnabled()	{return ICH;}
	static	bool	AutoServerlist(){return autoserverlist;}
	static	bool	UpdateNotify()	{return updatenotify;}
	static	bool	DoMinToTray()	{return mintotray;}
	static	bool	DoAutoConnect() {return autoconnect;}
	static	void	SetAutoConnect( bool inautoconnect) {autoconnect = inautoconnect;}
	static	bool	AddServersFromServer()		{return addserversfromserver;}
	static	bool	AddServersFromClient()		{return addserversfromclient;}
	static	uint8*	GetMinTrayPTR() {return &mintotray;}
	static	uint16	GetTrafficOMeterInterval() { return trafficOMeterInterval;}
	static	void	SetTrafficOMeterInterval(uint16 in) { trafficOMeterInterval=in;}
	static	uint16	GetStatsInterval() { return statsInterval;}
	static	void	SetStatsInterval(uint16 in) { statsInterval=in;}
	static	void	Add2TotalDownloaded(uint64 in) {totalDownloadedBytes+=in;}
	static	void	Add2TotalUploaded(uint64 in) {totalUploadedBytes+=in;}

	// -khaos--+++> Many, many, many, many methods.
	static	void	SaveStats(int bBackUp = 0);
	static	void	SetRecordStructMembers();
	static	void	SaveCompletedDownloadsStat();
	static	bool	LoadStats(int loadBackUp = 0);
	static	void	ResetCumulativeStatistics();

	//		Functions from base code that update original cumulative stats, now obsolete. (KHAOS)
	//void	Add2TotalDownloaded(uint64 in) {totalDownloadedBytes+=in;}
	//void	Add2TotalUploaded(uint64 in) {totalUploadedBytes+=in;}
	//		End functions from base code.

	//		Add to, increment and replace functions.  They're all named Add2 for the sake of some kind of naming
	//		convention.
	static	void	Add2DownCompletedFiles()			{ cumDownCompletedFiles++; }
	static	void	SetConnMaxAvgDownRate(float in)		{ cumConnMaxAvgDownRate = in; }
	static	void	SetConnMaxDownRate(float in)		{ cumConnMaxDownRate = in; }
	static	void	SetConnAvgUpRate(float in)			{ cumConnAvgUpRate = in; }
	static	void	SetConnMaxAvgUpRate(float in)		{ cumConnMaxAvgUpRate = in; }
	static	void	SetConnMaxUpRate(float in)			{ cumConnMaxUpRate = in; }
	static	void	SetConnPeakConnections(int in)		{ cumConnPeakConnections = in; }
	static	void	SetUpAvgTime(int in)				{ cumUpAvgTime = in; }
	static	void	Add2DownSAvgTime(int in)			{ sesDownAvgTime += in; }
	static	void	SetDownCAvgTime(int in)				{ cumDownAvgTime = in; }
	static	void	Add2ConnTransferTime(int in)		{ cumConnTransferTime += in; }
	static	void	Add2ConnDownloadTime(int in)		{ cumConnDownloadTime += in; }
	static	void	Add2ConnUploadTime(int in)			{ cumConnUploadTime += in; }
	static	void	Add2DownSessionCompletedFiles()		{ sesDownCompletedFiles++; }
	static	void	Add2SessionTransferData				(UINT uClientID, UINT uClientPort, BOOL bFromPF, BOOL bUpDown, uint32 bytes, bool sentToFriend = false);
	static	void	Add2DownSuccessfulSessions()		{ sesDownSuccessfulSessions++;
														  cumDownSuccessfulSessions++; }
	static	void	Add2DownFailedSessions()			{ sesDownFailedSessions++;
														  cumDownFailedSessions++; }
	static	void	Add2LostFromCorruption(uint64 in)	{ sesLostFromCorruption += in;}
	static	void	Add2SavedFromCompression(uint64 in) { sesSavedFromCompression += in;}
	static	void	Add2SessionPartsSavedByICH(int in)	{ sesPartsSavedByICH += in;}

	//		Functions that return stats stuff...
	//		Saved stats for cumulative downline overhead
	static	uint64	GetDownOverheadTotal()			{ return cumDownOverheadTotal;}
	static	uint64	GetDownOverheadFileReq()		{ return cumDownOverheadFileReq;}
	static	uint64	GetDownOverheadSrcEx()			{ return cumDownOverheadSrcEx;}
	static	uint64	GetDownOverheadServer()			{ return cumDownOverheadServer;}
	static	uint64	GetDownOverheadKad()			{ return cumDownOverheadKad;}
	static	uint64	GetDownOverheadTotalPackets()	{ return cumDownOverheadTotalPackets;}
	static	uint64	GetDownOverheadFileReqPackets() { return cumDownOverheadFileReqPackets;}
	static	uint64	GetDownOverheadSrcExPackets()	{ return cumDownOverheadSrcExPackets;}
	static	uint64	GetDownOverheadServerPackets()	{ return cumDownOverheadServerPackets;}
	static	uint64	GetDownOverheadKadPackets()		{ return cumDownOverheadKadPackets;}

	//		Saved stats for cumulative upline overhead
	static	uint64	GetUpOverheadTotal()			{ return cumUpOverheadTotal;}
	static	uint64	GetUpOverheadFileReq()			{ return cumUpOverheadFileReq;}
	static	uint64	GetUpOverheadSrcEx()			{ return cumUpOverheadSrcEx;}
	static	uint64	GetUpOverheadServer()			{ return cumUpOverheadServer;}
	static	uint64	GetUpOverheadKad()				{ return cumUpOverheadKad;}
	static	uint64	GetUpOverheadTotalPackets()		{ return cumUpOverheadTotalPackets;}
	static	uint64	GetUpOverheadFileReqPackets()	{ return cumUpOverheadFileReqPackets;}
	static	uint64	GetUpOverheadSrcExPackets()		{ return cumUpOverheadSrcExPackets;}
	static	uint64	GetUpOverheadServerPackets()	{ return cumUpOverheadServerPackets;}
	static	uint64	GetUpOverheadKadPackets()		{ return cumUpOverheadKadPackets;}

	//		Saved stats for cumulative upline data
	static	uint32	GetUpSuccessfulSessions()		{ return cumUpSuccessfulSessions;}
	static	uint32	GetUpFailedSessions()			{ return cumUpFailedSessions;}
	static	uint32	GetUpAvgTime()					{ return cumUpAvgTime;}

	// [TPT] - Credit System
	static	void	SetCreditSystem(uint8 m_iInCreditSystem)	{m_iCreditSystem = m_iInCreditSystem;}
	static	int		GetCreditSystem()					{return m_iCreditSystem;}
	static	bool	UseCreditSystem()					{if (m_iCreditSystem != 4) return true; else return false;}
	// [TPT] - Credit System	

	static	bool	IsSpreadBarsEnable()			{ return m_bSpreadBars;} // [TPT] - SLUGFILLER: Spreadbars

	//		Saved stats for cumulative downline data
	static	uint32	GetDownCompletedFiles()			{ return cumDownCompletedFiles;}
	static	uint32	GetDownC_SuccessfulSessions()	{ return cumDownSuccessfulSessions;}
	static	uint32	GetDownC_FailedSessions()		{ return cumDownFailedSessions;}
	static	uint32	GetDownC_AvgTime()				{ return cumDownAvgTime;}
	//		Session download stats
	static	uint32	GetDownSessionCompletedFiles()	{ return sesDownCompletedFiles;}
	static	uint32	GetDownS_SuccessfulSessions()	{ return sesDownSuccessfulSessions;}
	static	uint32	GetDownS_FailedSessions()		{ return sesDownFailedSessions;}
	static	uint32	GetDownS_AvgTime()				{ return GetDownS_SuccessfulSessions()?sesDownAvgTime/GetDownS_SuccessfulSessions():0;}

	//		Saved stats for corruption/compression
	static	uint64	GetCumLostFromCorruption()			{ return cumLostFromCorruption;}
	static	uint64	GetCumSavedFromCompression()		{ return cumSavedFromCompression;}
	static	uint64	GetSesLostFromCorruption()			{ return sesLostFromCorruption;}
	static	uint64	GetSesSavedFromCompression()		{ return sesSavedFromCompression;}
	static	uint32	GetCumPartsSavedByICH()				{ return cumPartsSavedByICH;}
	static	uint32	GetSesPartsSavedByICH()				{ return sesPartsSavedByICH;}

	// Cumulative client breakdown stats for sent bytes
	static	uint64	GetUpTotalClientData()			{ return   GetCumUpData_EDONKEY()
															  + GetCumUpData_EDONKEYHYBRID()
															  + GetCumUpData_EMULE()
															  + GetCumUpData_MLDONKEY()
															  + GetCumUpData_AMULE()
															 + GetCumUpData_EMULECOMPAT()
															 + GetCumUpData_SHAREAZA(); }
	static	uint64	GetCumUpData_EDONKEY()			{ return (cumUpData_EDONKEY +		sesUpData_EDONKEY );}
	static	uint64	GetCumUpData_EDONKEYHYBRID()	{ return (cumUpData_EDONKEYHYBRID +	sesUpData_EDONKEYHYBRID );}
	static	uint64	GetCumUpData_EMULE()			{ return (cumUpData_EMULE +			sesUpData_EMULE );}
	static	uint64	GetCumUpData_MLDONKEY()			{ return (cumUpData_MLDONKEY +		sesUpData_MLDONKEY );}
	static	uint64	GetCumUpData_AMULE()			{ return (cumUpData_AMULE +			sesUpData_AMULE );}
	static	uint64	GetCumUpData_EMULECOMPAT()		{ return (cumUpData_EMULECOMPAT +	sesUpData_EMULECOMPAT );}
	static	uint64	GetCumUpData_SHAREAZA()			{ return (cumUpData_SHAREAZA +		sesUpData_SHAREAZA );}
	// Session client breakdown stats for sent bytes
	static	uint64	GetUpSessionClientData()		{ return   sesUpData_EDONKEY 
															  +	sesUpData_EDONKEYHYBRID 
															  + sesUpData_EMULE 
															  +	sesUpData_MLDONKEY 
															  + sesUpData_AMULE
															 + sesUpData_EMULECOMPAT
															 + sesUpData_SHAREAZA; }
	static	uint64	GetUpData_EDONKEY()				{ return sesUpData_EDONKEY;}
	static	uint64	GetUpData_EDONKEYHYBRID()		{ return sesUpData_EDONKEYHYBRID;}
	static	uint64	GetUpData_EMULE()				{ return sesUpData_EMULE;}
	static	uint64	GetUpData_MLDONKEY()			{ return sesUpData_MLDONKEY;}
	static	uint64	GetUpData_AMULE()				{ return sesUpData_AMULE;}
	static	uint64	GetUpData_EMULECOMPAT()			{ return sesUpData_EMULECOMPAT;}
	static	uint64	GetUpData_SHAREAZA()			{ return sesUpData_SHAREAZA;}

	// Cumulative port breakdown stats for sent bytes...
	static	uint64	GetUpTotalPortData()			{ return   GetCumUpDataPort_4662() 
															 + GetCumUpDataPort_OTHER()
															 + GetCumUpDataPort_PeerCache(); }
	static	uint64	GetCumUpDataPort_4662()			{ return (cumUpDataPort_4662 +		sesUpDataPort_4662 );}
	static	uint64	GetCumUpDataPort_OTHER()		{ return (cumUpDataPort_OTHER +		sesUpDataPort_OTHER );}
	static	uint64	GetCumUpDataPort_PeerCache()	{ return (cumUpDataPort_PeerCache +	sesUpDataPort_PeerCache );}

	// Session port breakdown stats for sent bytes...
	static	uint64	GetUpSessionPortData()			{ return   sesUpDataPort_4662 
															 + sesUpDataPort_OTHER
															 + sesUpDataPort_PeerCache; }
	static	uint64	GetUpDataPort_4662()			{ return sesUpDataPort_4662;}
	static	uint64	GetUpDataPort_OTHER()			{ return sesUpDataPort_OTHER;}
	static	uint64	GetUpDataPort_PeerCache()		{ return sesUpDataPort_PeerCache; }

	// Cumulative DS breakdown stats for sent bytes...
	static	uint64	GetUpTotalDataFile()			{ return (GetCumUpData_File() +		GetCumUpData_Partfile() );}
	static	uint64	GetCumUpData_File()				{ return (cumUpData_File +			sesUpData_File );}
	static	uint64	GetCumUpData_Partfile()			{ return (sesUpData_Partfile +		sesUpData_Partfile );}
	// Session DS breakdown stats for sent bytes...
	static	uint64	GetUpSessionDataFile()			{ return (sesUpData_File +			sesUpData_Partfile );}
	static	uint64	GetUpData_File()				{ return sesUpData_File;}
	static	uint64	GetUpData_Partfile()			{ return sesUpData_Partfile;}

	// Cumulative client breakdown stats for received bytes
	static	uint64	GetDownTotalClientData()		{ return   GetCumDownData_EDONKEY() 
															  + GetCumDownData_EDONKEYHYBRID() 
															  + GetCumDownData_EMULE() 
															  +	GetCumDownData_MLDONKEY() 
															  + GetCumDownData_AMULE()
															  + GetCumDownData_EMULECOMPAT()
															  + GetCumDownData_SHAREAZA()
															  + GetCumDownData_URL()
															  + GetCumDownData_WEBCACHE(); } // [TPT] - WebCache	// jp webcache statistics
	static	uint64	GetCumDownData_EDONKEY()		{ return (cumDownData_EDONKEY +			sesDownData_EDONKEY);}
	static	uint64	GetCumDownData_EDONKEYHYBRID()	{ return (cumDownData_EDONKEYHYBRID +	sesDownData_EDONKEYHYBRID);}
	static	uint64	GetCumDownData_EMULE()			{ return (cumDownData_EMULE +			sesDownData_EMULE);}
	static	uint64	GetCumDownData_MLDONKEY()		{ return (cumDownData_MLDONKEY +		sesDownData_MLDONKEY);}
	static	uint64	GetCumDownData_AMULE()			{ return (cumDownData_AMULE +			sesDownData_AMULE);}
	static	uint64	GetCumDownData_EMULECOMPAT()	{ return (cumDownData_EMULECOMPAT +		sesDownData_EMULECOMPAT);}
	static	uint64	GetCumDownData_SHAREAZA()		{ return (cumDownData_SHAREAZA +		sesDownData_SHAREAZA);}
	static	uint64	GetCumDownData_URL()			{ return (cumDownData_URL +				sesDownData_URL);}
	static	uint64	GetCumDownData_WEBCACHE()		{ return (cumDownData_WEBCACHE +		sesDownData_WEBCACHE);} // [TPT] - WebCache	//jp webcache statistics
	// Session client breakdown stats for received bytes
	static	uint64	GetDownSessionClientData()		{ return   sesDownData_EDONKEY 
															  + sesDownData_EDONKEYHYBRID 
															  + sesDownData_EMULE 
															  +	sesDownData_MLDONKEY 
															  + sesDownData_AMULE
															  + sesDownData_EMULECOMPAT
															  + sesDownData_SHAREAZA
															  + sesDownData_URL															  
															  + sesDownData_WEBCACHE; } // [TPT] - WebCache	// jp webcache statistics
	static	uint64	GetDownData_EDONKEY()			{ return sesDownData_EDONKEY;}
	static	uint64	GetDownData_EDONKEYHYBRID()		{ return sesDownData_EDONKEYHYBRID;}
	static	uint64	GetDownData_EMULE()				{ return sesDownData_EMULE;}
	static	uint64	GetDownData_MLDONKEY()			{ return sesDownData_MLDONKEY;}
	static	uint64	GetDownData_AMULE()				{ return sesDownData_AMULE;}
	static	uint64	GetDownData_EMULECOMPAT()		{ return sesDownData_EMULECOMPAT;}
	static	uint64	GetDownData_SHAREAZA()			{ return sesDownData_SHAREAZA;}
	static	uint64	GetDownData_URL()				{ return sesDownData_URL;}
	static	uint64	GetDownData_WEBCACHE()			{ return sesDownData_WEBCACHE;} // [TPT] - WebCache	//jp webcache statistics

	// Cumulative port breakdown stats for received bytes...
	static	uint64	GetDownTotalPortData()			{ return   GetCumDownDataPort_4662() 
															 + GetCumDownDataPort_OTHER()
															 + GetCumDownDataPort_PeerCache(); }
	static	uint64	GetCumDownDataPort_4662()		{ return cumDownDataPort_4662		+ sesDownDataPort_4662; }
	static	uint64	GetCumDownDataPort_OTHER()		{ return cumDownDataPort_OTHER		+ sesDownDataPort_OTHER; }
	static	uint64	GetCumDownDataPort_PeerCache()	{ return cumDownDataPort_PeerCache	+ sesDownDataPort_PeerCache; }

	// Session port breakdown stats for received bytes...
	static	uint64	GetDownSessionDataPort()		{ return   sesDownDataPort_4662 
															 + sesDownDataPort_OTHER
															 + sesDownDataPort_PeerCache; }
	static	uint64	GetDownDataPort_4662()			{ return sesDownDataPort_4662;}
	static	uint64	GetDownDataPort_OTHER()			{ return sesDownDataPort_OTHER;}
	static	uint64	GetDownDataPort_PeerCache()		{ return sesDownDataPort_PeerCache; }

	//		Saved stats for cumulative connection data
	static	float	GetConnAvgDownRate()			{ return cumConnAvgDownRate;}
	static	float	GetConnMaxAvgDownRate()			{ return cumConnMaxAvgDownRate;}
	static	float	GetConnMaxDownRate()			{ return cumConnMaxDownRate;}
	static	float	GetConnAvgUpRate()				{ return cumConnAvgUpRate;}
	static	float	GetConnMaxAvgUpRate()			{ return cumConnMaxAvgUpRate;}
	static	float	GetConnMaxUpRate()				{ return cumConnMaxUpRate;}
	static	uint64	GetConnRunTime()				{ return cumConnRunTime;}
	static	uint32	GetConnNumReconnects()			{ return cumConnNumReconnects;}
	static	uint32	GetConnAvgConnections()			{ return cumConnAvgConnections;}
	static	uint32	GetConnMaxConnLimitReached()	{ return cumConnMaxConnLimitReached;}
	static	uint32	GetConnPeakConnections()		{ return cumConnPeakConnections;}
	static	uint32	GetConnTransferTime()			{ return cumConnTransferTime;}
	static	uint32	GetConnDownloadTime()			{ return cumConnDownloadTime;}
	static	uint32	GetConnUploadTime()				{ return cumConnUploadTime;}
	static	uint32	GetConnServerDuration()			{ return cumConnServerDuration;}

	//		Saved records for servers / network
	static	uint32	GetSrvrsMostWorkingServers()	{ return cumSrvrsMostWorkingServers;}
	static	uint32	GetSrvrsMostUsersOnline()		{ return cumSrvrsMostUsersOnline;}
	static	uint32	GetSrvrsMostFilesAvail()		{ return cumSrvrsMostFilesAvail;}

	//		Saved records for shared files
	static	uint32	GetSharedMostFilesShared()		{ return cumSharedMostFilesShared;}
	static	uint64	GetSharedLargestShareSize()		{ return cumSharedLargestShareSize;}
	static	uint64	GetSharedLargestAvgFileSize()	{ return cumSharedLargestAvgFileSize;}
	static	uint64	GetSharedLargestFileSize()		{ return cumSharedLargestFileSize;}

	//		Get the long date/time when the stats were last reset
	static	__int64 GetStatsLastResetLng()			{ return stat_datetimeLastReset;}
	static	CString GetStatsLastResetStr(bool formatLong = true);
	static	UINT	GetStatsSaveInterval()			{ return statsSaveInterval; }

	//		Get and Set our new preferences
	static	void	SetStatsMax(uint16 in)						{ statsMax = in; }
	static	void	SetStatsConnectionsGraphRatio(uint8 in)		{ statsConnectionsGraphRatio = in; }
	static	uint8	GetStatsConnectionsGraphRatio()				{ return statsConnectionsGraphRatio; }
	static	void	SetExpandedTreeItems(CString in)			{ _stprintf(statsExpandedTreeItems,_T("%s"),in); }
	static	CString GetExpandedTreeItems()						{ return statsExpandedTreeItems; }
	// <-----khaos- End Statistics Methods

	//		Original Statistics Functions
	static	uint64	GetTotalDownloaded()		{return totalDownloadedBytes;}
	static	uint64	GetTotalUploaded()			{return totalUploadedBytes;}
	//		End Original Statistics Functions
	static	bool	IsErrorBeepEnabled()		{return beepOnError;}
	static	bool	IsConfirmExitEnabled()		{return confirmExit;}
	static	bool	UseSplashScreen()			{return splashscreen;}
	static	bool	FilterLANIPs()				{return filterLANIPs;}
	static	bool	GetAllowLocalHostIP()		{return m_bAllocLocalHostIP;}
	static	bool	IsOnlineSignatureEnabled()	{return onlineSig;}
	// [TPT]
	//static	int		GetMaxGraphUploadRate()		{return maxGraphUploadRate;}
	//static	int		GetMaxGraphDownloadRate()		{return maxGraphDownloadRate;}
	//static	void	SetMaxGraphUploadRate(int in)	{maxGraphUploadRate	=(in)?in:16;}
	//static	void	SetMaxGraphDownloadRate(int in) {maxGraphDownloadRate=(in)?in:96;}

	//static uint16	GetMaxDownload();
	static	uint64	GetMaxDownloadInBytesPerSec(bool dynamic = false);
	static	uint16	GetMaxConnections()			{return maxconnections;}
	static	uint16	GetMaxHalfConnections()		{return maxhalfconnections;}
	static	uint16	GetMaxSourcePerFile()		{return maxsourceperfile;}
	static	uint16	GetMaxSourcePerFileSoft();
	static	uint16	GetMaxSourcePerFileUDP();
	static	uint16	GetDeadserverRetries()		{return deadserverretries;}
	static	DWORD	GetServerKeepAliveTimeout() {return m_dwServerKeepAliveTimeout;}

	static	int		GetColumnWidth (Table t, int index);
	static	BOOL	GetColumnHidden(Table t, int index);
	static	int		GetColumnOrder (Table t, int index);
	static	void	SetColumnWidth (Table t, int index, int width);
	static	void	SetColumnHidden(Table t, int index, BOOL bHidden);
	static	void	SetColumnOrder (Table t, INT *piOrder);

	// Barry - Provide a mechanism for all tables to store/retrieve sort order
	// [TPT] - SLUGFILLER: multiSort
	static	int		GetColumnSortItem (Table t, int column = 0);
	static	bool	GetColumnSortAscending (Table t, int column = 0);
	static	int		GetColumnSortCount(Table t);
	// [TPT] - SLUGFILLER: multiSort
	static	void	SetColumnSortItem (Table t, int sortItem);
	static	void	SetColumnSortAscending (Table t, bool sortAscending);

	static	WORD	GetLanguageID();
	static	void	SetLanguageID(WORD lid);
	static	void	GetLanguages(CWordArray& aLanguageIDs);
	static	void	SetLanguage();
	static	const CString& GetLangDir()					{return m_strLangDir;}
	static	bool	IsLanguageSupported(LANGID lidSelected, bool bUpdateBefore);
	static	CString GetLangDLLNameByID(LANGID lidSelected);
	static	void	InitThreadLocale();
	static	void	SetRtlLocale(LCID lcid);
	static	CString GetHtmlCharset();

	static	uint8	IsDoubleClickEnabled()				{return transferDoubleclick;}
	static	EViewSharedFilesAccess CanSeeShares(void) {return m_iSeeShares;}
	// [TPT] - MFCK [addon] - New Tooltips [Rayita]
	static	uint32	GetToolTipDelay(void) { return m_iToolDelayTime ? (uint32)m_iToolDelayTime * 1000 : 500; }
	// [TPT] - MFCK [addon] - New Tooltips [Rayita]
	static	uint8	IsBringToFront()					{return bringtoforeground;}

	static	uint8	GetSplitterbarPosition()			{return splitterbarPosition;}
	static	void	SetSplitterbarPosition(uint8 pos)	{splitterbarPosition=pos;}
	static	uint8	GetTransferWnd2()					{return m_uTransferWnd2;}
	static	void	SetTransferWnd2(uint8 uWnd2)		{m_uTransferWnd2 = uWnd2;}
	//MORPH START - Added by SiRoB, Splitting Bar [O²]
	static	uint8   GetSplitterbarPositionStat()	{return splitterbarPositionStat;}
	static	void	SetSplitterbarPositionStat(uint8 pos) {splitterbarPositionStat=pos;}
	static	uint8   GetSplitterbarPositionStat_HL()	{return splitterbarPositionStat_HL;}
	static	void	SetSplitterbarPositionStat_HL(uint8 pos) {splitterbarPositionStat_HL=pos;}
	static	uint8   GetSplitterbarPositionStat_HR()	{return splitterbarPositionStat_HR;}
	static	void	SetSplitterbarPositionStat_HR(uint8 pos) {splitterbarPositionStat_HR=pos;}
	static	uint16   GetSplitterbarPositionFriend()	{return splitterbarPositionFriend;}
	static	void	SetSplitterbarPositionFriend(uint16 pos) {splitterbarPositionFriend=pos;}
	static	uint16  GetSplitterbarPositionIRC()	{return splitterbarPositionIRC;}
	static	void	SetSplitterbarPositionIRC(uint16 pos) {splitterbarPositionIRC=pos;}
	//MORPH END   - Added by SiRoB, Splitting Bar [O²]
	// -khaos--+++> Changed datatype to avoid overflows
	static	uint16	GetStatsMax()						{return statsMax;}
	// <-----khaos-
	static	uint8	UseFlatBar()						{return (depth3D==0);}
	static	int		GetStraightWindowStyles()			{return m_iStraightWindowStyles;}

	static	const CString& GetSkinProfile()				{return m_strSkinProfile;}
	static	void	SetSkinProfile(LPCTSTR pszProfile)	{m_strSkinProfile = pszProfile; }

	static	const CString& GetSkinProfileDir()			{return m_strSkinProfileDir;}
	static	void	SetSkinProfileDir(LPCTSTR pszDir)	{m_strSkinProfileDir = pszDir; }

	static	uint8	GetStatsAverageMinutes()			{return statsAverageMinutes;}
	static	void	SetStatsAverageMinutes(uint8 in)	{statsAverageMinutes=in;}

	// [TPT] - enkeyDEV(th1) -notifier-
	static	bool    GetUseNotifierUserTimings() {return useNotifierUserTimings;}
	static	uint16  GetNotifierUserTimeToShow() {return notifierUserTimeToShow;}
	static	uint16  GetNotifierUserTimeToStay() {return notifierUserTimeToStay;}
	static	uint16  GetNotifierUserTimeToHide() {return notifierUserTimeToHide;}
	static	bool    GetNotifierLessFramerate() {return notifierLessFramerate;}
	static	bool	GetUseDownloadNotifier()			{return useDownloadNotifier;}
	static	bool	GetUseNewDownloadNotifier()			{return useNewDownloadNotifier;}
	static	bool	GetUseChatNotifier()				{return useChatNotifier;}
	static	bool	GetUseLogNotifier()					{return useLogNotifier;}
	static	bool	GetUseSoundInNotifier()				{return useSoundInNotifier;}
	static	bool	GetNotifierPopsEveryChatMsg()		{return notifierPopsEveryChatMsg;}
	static	bool	GetNotifierPopOnImportantError()	{return notifierImportantError;}
	static	bool	GetNotifierPopOnSearch()			{return notifierSearchCompleted;}
    static	bool    GetNotifierAutoClose()              {return notifierAutoClose;};        
    static	bool    GetUseErrorNotifier()		{return useErrorNotifier;}      // added by InterCeptor (notify on error) 11.11.02
	static	bool	GetNotifierPopOnNewVersion()		{return notifierNewVersion;}
	static	TCHAR*	GetNotifierWavSoundPath()			{return notifierSoundFilePath;}
	static	bool	GetNotifierNewPvtMsg() {return notifierNewPvtMsg;}//Rocks
	// [TPT] - enkeyDEV(th1) -notifier-

	static	CString GetIRCNick()						{return m_sircnick;}
	static	void	SetIRCNick( TCHAR in_nick[] )		{ _tcscpy(m_sircnick,in_nick);}
	static	CString GetIRCServer()						{return m_sircserver;}
	static	bool	GetIRCAddTimestamp()				{return m_bircaddtimestamp;}
	static	CString GetIRCChanNameFilter()				{return m_sircchannamefilter;}
	static	bool	GetIRCUseChanFilter()				{return m_bircusechanfilter;}
	static	uint16	GetIRCChannelUserFilter()			{return m_iircchanneluserfilter;}
	static	CString GetIrcPerformString()				{return m_sircperformstring;}
	static	bool	GetIrcUsePerform()					{return m_bircuseperform;}
	static	bool	GetIRCListOnConnect()				{return m_birclistonconnect;}
	static	bool	GetIrcAcceptLinks()					{return m_bircacceptlinks;}
	static	bool	GetIrcAcceptLinksFriends()			{return m_bircacceptlinksfriends;}
	static	bool	GetIrcSoundEvents()					{return m_bircsoundevents;}
	static	bool	GetIrcIgnoreMiscMessage()			{return m_bircignoremiscmessage;}
	static	bool	GetIrcIgnoreJoinMessage()			{return m_bircignorejoinmessage;}
	static	bool	GetIrcIgnorePartMessage()			{return m_bircignorepartmessage;}
	static	bool	GetIrcIgnoreQuitMessage()			{return m_bircignorequitmessage;}
	static	bool	GetIrcIgnoreEmuleProtoAddFriend()	{return m_bircignoreemuleprotoaddfriend;}
	static	bool	GetIrcAllowEmuleProtoAddFriend()	{return m_bircallowemuleprotoaddfriend;}
	static	bool	GetIrcIgnoreEmuleProtoSendLink()	{return m_bircignoreemuleprotosendlink;}
	static	bool	GetIrcHelpChannel()					{return m_birchelpchannel;}
	static	WORD	GetWindowsVersion();
	static	bool	GetStartMinimized()					{return startMinimized;}
	static	void	SetStartMinimized( bool instartMinimized) {startMinimized = instartMinimized;}
	static	bool	GetAutoStart()						{return m_bAutoStart;}
	static	void	SetAutoStart( bool val)				{m_bAutoStart = val;}

	static	bool	GetRestoreLastMainWndDlg()			{return m_bRestoreLastMainWndDlg;}
	static	int		GetLastMainWndDlgID()				{return m_iLastMainWndDlgID;}
	static	void	SetLastMainWndDlgID(int iID)		{m_iLastMainWndDlgID = iID;}

	static	bool	GetRestoreLastLogPane()				{return m_bRestoreLastLogPane;}
	static	int		GetLastLogPaneID()					{return m_iLastLogPaneID;}
	static	void	SetLastLogPaneID(int iID)			{m_iLastLogPaneID = iID;}

	static	bool	GetSmartIdCheck()					{return smartidcheck;}
	static	void	SetSmartIdCheck(bool in_smartidcheck) {smartidcheck = in_smartidcheck;}
	static	uint8	GetSmartIdState()					{return smartidstate;}
	static	void	SetSmartIdState(uint8 in_smartidstate) {smartidstate = in_smartidstate;}
	static	bool	GetPreviewPrio()					{return m_bpreviewprio;}
	static	void	SetPreviewPrio(bool in)				{m_bpreviewprio=in;}
	static	bool	GetUpdateQueueList()				{return m_bupdatequeuelist;}
	static	bool	GetManualHighPrio()					{return m_bmanualhighprio;}
	static	bool	TransferFullChunks()				{return m_btransferfullchunks;}
	static	void	SetTransferFullChunks( bool m_bintransferfullchunks )				{m_btransferfullchunks = m_bintransferfullchunks;}
	static	int		StartNextFile()						{return m_istartnextfile;}
	static	bool	ShowOverhead()						{return m_bshowoverhead;}
	static	void	SetNewAutoUp(bool m_bInUAP)			{m_bUAP = m_bInUAP;}
	static	bool	GetNewAutoUp()						{return m_bUAP;}
	static	void	SetNewAutoDown(bool m_bInDAP)		{m_bDAP = m_bInDAP;}
	static	bool	GetNewAutoDown()					{return m_bDAP;}
	static	bool	IsKnownClientListDisabled()			{return m_bDisableKnownClientList;}
	static	bool	IsQueueListDisabled()				{return m_bDisableQueueList;}
	static	bool	IsFirstStart()						{return m_bFirstStart;}
	// [TPT]
	//static	bool	UseCreditSystem()					{return m_bCreditSystem;}
	//static	void	SetCreditSystem(bool m_bInCreditSystem) {m_bCreditSystem = m_bInCreditSystem;}

	static	TCHAR*	GetTxtEditor()						{return TxtEditor;}
	static	CString	GetVideoPlayer()					{if (_tcslen(VideoPlayer)==0) return _T(""); else return CString(VideoPlayer);}

	static	UINT	GetFileBufferSize()					{return m_iFileBufferSize;}
	static	UINT	GetQueueSize()						{return m_iQueueSize;}
	static	int		GetCommitFiles()					{return m_iCommitFiles;}
	static	bool	GetShowCopyEd2kLinkCmd()			{return m_bShowCopyEd2kLinkCmd;}

	// Barry
	static	uint16	Get3DDepth() { return depth3D;}
	static	bool	AutoTakeED2KLinks() {return autotakeed2klinks;}
	static	bool	AddNewFilesPaused() {return addnewfilespaused;}

	static	bool	TransferlistRemainSortStyle()	{ return m_bTransflstRemain;}
	static	void	TransferlistRemainSortStyle(bool in)	{ m_bTransflstRemain=in;}

	static	DWORD	GetStatsColor(int index)			{return m_adwStatsColors[index];}
	static	void	SetStatsColor(int index, DWORD value){m_adwStatsColors[index] = value;}
	static	int		GetNumStatsColors()					{return ARRSIZE(m_adwStatsColors);}
	static	void	GetAllStatsColors(int iCount, LPDWORD pdwColors);
	static	bool	SetAllStatsColors(int iCount, const DWORD* pdwColors);
	static	void	ResetStatsColor(int index);

	static	void	SetMaxConsPerFive(int in)			{MaxConperFive=in;}
	static	LPLOGFONT GetHyperTextLogFont()				{return &m_lfHyperText;}
	static	void	SetHyperTextFont(LPLOGFONT plf)		{m_lfHyperText = *plf;}
	static	LPLOGFONT GetLogFont()						{return &m_lfLogText;}
	static	void	SetLogFont(LPLOGFONT plf)			{m_lfLogText = *plf;}
	static	COLORREF GetLogErrorColor()					{return m_crLogError;}
	static	COLORREF GetLogWarningColor()				{return m_crLogWarning;}
	static	COLORREF GetLogSuccessColor()				{return m_crLogSuccess;}

	// [TPT] - quick start
	static	void	SetMaxCon(int in)         {maxconnections=in;} 
	static	uint16	GetMaxCon()		          {return maxconnections;}
	static	void	SetQuickStartMaxCon(int in) { m_QuickStartMaxCon = in; }
	static	uint16  GetQuickStartMaxCon()		{ return m_QuickStartMaxCon; }
	static	void    SetQuickStartMaxConPerFive (int in) { m_QuickStartMaxConPerFive = in; }
	static	uint16  GetQuickStartMaxConPerFive(){ return m_QuickStartMaxConPerFive; }
	// [TPT] - quick start

	//[TPT]- Show Upload Priority in downloadlist
	static bool		GetShowUpPrioInDownloadList()	{return m_bShowUpPrioInDownloadList;}
	static void		SetShowUpPrioInDownloadList(bool in)	{m_bShowUpPrioInDownloadList = in;}

	//[TPT] - Double buffer style in lists
	static	void	SetDoubleBufferStyle(bool in)	{m_bDBStyle = in;}
	static	bool	GetDoubleBufferStyle()	{return m_bDBStyle;}

	static	uint16	GetMaxConperFive()					{return MaxConperFive;}
	static	uint16	GetDefaultMaxConperFive();

	static	bool	IsSafeServerConnectEnabled()		{return safeServerConnect;}
	static	void	SetSafeServerConnectEnabled(bool in){safeServerConnect=in;}
	static	bool	IsMoviePreviewBackup()				{return moviePreviewBackup;}
	static	int		GetPreviewSmallBlocks()				{return m_iPreviewSmallBlocks;}
	static	int		GetPreviewCopiedArchives()			{return m_iPreviewCopiedArchives;}
	static	int		GetInspectAllFileTypes()			{return m_iInspectAllFileTypes;}
	static	int		GetExtractMetaData()				{return m_iExtractMetaData;}
	static	bool	GetAdjustNTFSDaylightFileTime()		{return m_bAdjustNTFSDaylightFileTime;}

	static	const CString& GetYourHostname()			{return m_strYourHostname;}
	static	void	SetYourHostname(LPCTSTR pszHostname){m_strYourHostname = pszHostname;}
	static	bool	IsCheckDiskspaceEnabled()			{return checkDiskspace != 0;}	// SLUGFILLER: checkDiskspace
	static	UINT	GetMinFreeDiskSpace()				{return m_uMinFreeDiskSpace;}
	static	bool	GetSparsePartFiles()				{return m_bSparsePartFiles;}
	static	void	SetSparsePartFiles(bool bEnable)	{m_bSparsePartFiles = bEnable;}

	// [TPT]
	//static	void	SetMaxUpload(uint16 in);
	//static	void	SetMaxDownload(uint16 in);
	static	bool	IsInfiniteQueueEnabled()		{return infiniteQueue;}	// [TPT] - SLUGFILLER: infiniteQueue
	// [TPT] - SLUGFILLER: hideOS
	static	uint8	GetHideOvershares()		{return hideOS;}
	static	bool	IsSelectiveShareEnabled()	{return selectiveShare;}
	// [TPT] - SLUGFILLER: hideOS
	// [TPT] - itsonlyme: displayOptions START
	static	bool	ShowFileSystemIcon()	{return showFileSystemIcon;}
	static	bool	ShowLocalRating()		{return showLocalRating;}  // [TPT] - SLUGFILLER: showComments
	// [TPT] - itsonlyme: displayOptions END

	static	bool	GetShowBitmapInMenus()	{return m_bShowBitmapInMenus;}
	static  void	SetShowBitmapInMenus(bool in)	{m_bShowBitmapInMenus = in;}

	// [TPT] - NAFC Selection
	static DWORD	GetNAFCSelection()		{return m_currentAdapterIndex; }
	static void		SetNAFCSelection(DWORD value) { m_currentAdapterIndex = value;}
	// [TPT] - NAFC Selection

	static	WINDOWPLACEMENT GetEmuleWindowPlacement() {return EmuleWindowPlacement; }
	static	void	SetWindowLayout(WINDOWPLACEMENT in) {EmuleWindowPlacement=in; }

	static	uint8	AutoConnectStaticOnly() {return autoconnectstaticonly;}
	static	uint8	GetUpdateDays()			{return versioncheckdays;}
	static	uint32	GetLastVC()				{return versioncheckLastAutomatic;}
	static	void	UpdateLastVC();
	static	int		GetIPFilterLevel()		{ return filterlevel;}
	static	CString GetMessageFilter()		{ return CString(messageFilter);}
	static	const CString& GetCommentFilter(){ return commentFilter; }
	static	CString GetFilenameCleanups()	{ return CString(filenameCleanups);}

	static	bool	ShowRatesOnTitle()		{ return showRatesInTitle;}
	static	TCHAR*	GetNotifierConfiguration()	  {return notifierConfiguration;}; //<<-- enkeyDEV(kei-kun) -skinnable notifier-
	static	void	SetNotifierConfiguration(CString configFullPath) {_stprintf(notifierConfiguration,_T("%s"),configFullPath); } //<<-- enkeyDEV(kei-kun) -skinnable notifier-
	static	void	LoadCats();
	static	CString GetDateTimeFormat()		{ return CString(datetimeformat);}
	static	CString GetDateTimeFormat4Log() { return CString(datetimeformat4log);}

	// Download Categories (Ornis)
	static	int		AddCat(Category_Struct* cat) { catMap.Add(cat); return catMap.GetCount()-1;}
	static	bool	MoveCat(UINT from, UINT to);
	static	void	RemoveCat(int index);
	static	int		GetCatCount()			{ return catMap.GetCount();}
	// [TPT] - khaos::categorymod
	//static  bool	SetCatFilter(int index, int filter);
	//static  int		GetCatFilter(int index);
	//static	bool	GetCatFilterNeg(int index);
	//static	void	SetCatFilterNeg(int index, bool val);
	// [TPT] - khaos::categorymod
	static	Category_Struct* GetCategory(int index) { if (index>=0 && index<catMap.GetCount()) return catMap.GetAt(index); else return NULL;}
	static	TCHAR*	GetCatPath(uint8 index) { return catMap.GetAt(index)->incomingpath;}
	static	DWORD	GetCatColor(uint8 index)	{ if (index>=0 && index<catMap.GetCount()) return catMap.GetAt(index)->color; else return 0;}

	static	bool	GetPreviewOnIconDblClk() { return m_bPreviewOnIconDblClk; }
	static	bool	ShowRatingIndicator()	{ return indicateratings;}
	static	bool	WatchClipboard4ED2KLinks()	{ return watchclipboard;}
	static	bool	GetRemoveToBin()			{ return m_bRemove2bin;}
	static	bool	FilterServerByIP()		{ return filterserverbyip;}

	static	bool	GetLog2Disk()							{ return log2disk;}
	static	bool	GetDebug2Disk()							{ return m_bVerbose && debug2disk;}
	static	int		GetMaxLogBuff()							{ return iMaxLogBuff;}
	static	UINT	GetMaxLogFileSize()						{ return uMaxLogFileSize; }
	// [TPT] - itsonlyme: virtualDirs
	static	CRBMap<CString, CString> *GetFileToVDirMap() { return &m_fileToVDir_map; }
	static	CRBMap<CString, CString> *GetDirToVDirMap() { return &m_dirToVDir_map; }
	static	CRBMap<CString, CString> *GetSubDirToVDirMap() { return &m_dirToVDirWithSD_map; }
	// [TPT] - itsonlyme: virtualDirs

	// WebServer
	static	uint16	GetWSPort()								{ return m_nWebPort; }
	static	void	SetWSPort(uint16 uPort)					{ m_nWebPort=uPort; }
	static	CString GetWSPass()								{ return CString(m_sWebPassword); }
	static	void	SetWSPass(CString strNewPass);
	static	bool	GetWSIsEnabled()						{ return m_bWebEnabled; }
	static	void	SetWSIsEnabled(bool bEnable)			{ m_bWebEnabled=bEnable; }
	static	bool	GetWebUseGzip()							{ return m_bWebUseGzip; }
	static	void	SetWebUseGzip(bool bUse)				{ m_bWebUseGzip=bUse; }
	static	int		GetWebPageRefresh()						{ return m_nWebPageRefresh; }
	static	void	SetWebPageRefresh(int nRefresh)			{ m_nWebPageRefresh=nRefresh; }
	static	bool	GetWSIsLowUserEnabled()					{ return m_bWebLowEnabled; }
	static	void	SetWSIsLowUserEnabled(bool in)			{ m_bWebLowEnabled=in; }
	static	CString GetWSLowPass()							{ return CString(m_sWebLowPassword); }
	static	int		GetWebTimeoutMins()						{ return m_iWebTimeoutMins;}
	static	void	SetWSLowPass(CString strNewPass);

	static	void	SetMaxSourcesPerFile(uint16 in)			{ maxsourceperfile=in;}
	static	void	SetMaxConnections(uint16 in)			{ maxconnections =in;}
	static	void	SetMaxHalfConnections(uint16 in)		{ maxhalfconnections =in;}
	static	bool	IsSchedulerEnabled()					{ return scheduler;}
	static	void	SetSchedulerEnabled(bool in)			{ scheduler=in;}
	static	bool	GetDontCompressAvi()					{ return dontcompressavi;}

	static	bool	MsgOnlyFriends()						{ return msgonlyfriends;}
	static	bool	MsgOnlySecure()							{ return msgsecure;}
	static	uint16	GetMsgSessionsMax()						{ return maxmsgsessions;}
	static	bool	IsSecureIdentEnabled()					{ return m_bUseSecureIdent;} // use clientcredits->CryptoAvailable() to check if crypting is really available and not this function
	static	bool	IsAdvSpamfilterEnabled()				{ return m_bAdvancedSpamfilter;}
	static	CString GetTemplate()							{ return CString(m_sTemplateFile);}
	static	void	SetTemplate(CString in)					{ _stprintf(m_sTemplateFile,_T("%s"),in);}
	static	bool	GetNetworkKademlia()					{ return networkkademlia;}
	static	void	SetNetworkKademlia(bool val);
	static	bool	GetNetworkED2K()						{ return networked2k;}
	static	void	SetNetworkED2K(bool val)				{ networked2k = val;}

	// mobileMule
	static	CString GetMMPass()								{ return CString(m_sMMPassword); }
	static	void	SetMMPass(CString strNewPass);
	static	bool	IsMMServerEnabled()						{ return m_bMMEnabled; }
	static	void	SetMMIsEnabled(bool bEnable)			{ m_bMMEnabled=bEnable; }
	static	uint16	GetMMPort()								{ return m_nMMPort; }
	static	void	SetMMPort(uint16 uPort)					{ m_nMMPort=uPort; }

	// deadlake PROXYSUPPORT
	static	const ProxySettings& GetProxy()					{ return proxy; }
	static	void	SetProxySettings(const ProxySettings& proxysettings) { proxy = proxysettings; }


	static	uint16	GetListenPort()							{ if (m_UseProxyListenPort) return ListenPort; else return port; }



	static	void	SetListenPort(uint16 uPort)				{ ListenPort = uPort; m_UseProxyListenPort = true; }
	static	void	ResetListenPort()						{ ListenPort = 0; m_UseProxyListenPort = false; }
	static	void	SetUseProxy(bool in)					{ proxy.UseProxy=in;}
	static	bool	GetShowProxyErrors()					{ return m_bShowProxyErrors; }
	static	void	SetShowProxyErrors(bool bEnable)		{ m_bShowProxyErrors = bEnable; }

	static	bool	IsProxyASCWOP()							{ return m_bIsASCWOP;}
	static	void	SetProxyASCWOP(bool in)					{ m_bIsASCWOP=in;}

	static	bool	ShowCatTabInfos()						{ return showCatTabInfos;}
	static	void	ShowCatTabInfos(bool in)				{ showCatTabInfos=in;}

	static	bool	AutoFilenameCleanup()						{ return autofilenamecleanup;}
	static	void	AutoFilenameCleanup(bool in)				{ autofilenamecleanup=in;}
	static	void	SetFilenameCleanups(CString in)				{ _stprintf(filenameCleanups,_T("%s"),in);}

	static	bool	GetResumeSameCat()							{ return resumeSameCat;}
	static	bool	IsGraphRecreateDisabled()					{ return dontRecreateGraphs;}
	static	bool	IsExtControlsEnabled()						{ return m_bExtControls;}
	static	void	SetExtControls(bool in)						{ m_bExtControls=in;}
	static	bool	GetRemoveFinishedDownloads()				{ return m_bRemoveFinishedDownloads;}

	static	uint16	GetMaxChatHistoryLines()					{ return m_iMaxChatHistory;}
	static	bool	GetUseAutocompletion()						{ return m_bUseAutocompl;}
	static	bool	GetUseDwlPercentage()						{ return m_bShowDwlPercentage;}
	static	void	SetUseDwlPercentage(bool in)				{ m_bShowDwlPercentage=in;}
	static	bool	GetShowActiveDownloadsBold()				{ return m_bShowActiveDownloadsBold; }

	//Toolbar
	static	const CString& GetToolbarSettings()					{ return m_sToolbarSettings; }
	static	void	SetToolbarSettings(const CString& in)		{ m_sToolbarSettings = in; }
	static	const CString& GetToolbarBitmapSettings()			{ return m_sToolbarBitmap; }
	static	void	SetToolbarBitmapSettings(const CString& path){ m_sToolbarBitmap = path; }
	static	const CString& GetToolbarBitmapFolderSettings()		{ return m_sToolbarBitmapFolder; }
	static	void	SetToolbarBitmapFolderSettings(const CString& path){ m_sToolbarBitmapFolder = path; }
	static	EToolbarLabelType GetToolbarLabelSettings()			{ return m_nToolbarLabels; }
	static	void	SetToolbarLabelSettings(EToolbarLabelType eLabelType) { m_nToolbarLabels = eLabelType; }
	static	bool	GetReBarToolbar()							{ return m_bReBarToolbar; }
	static	bool	GetUseReBarToolbar();
	static	CSize	GetToolbarIconSize()						{ return m_sizToolbarIconSize; }
	static	void	SetToolbarIconSize(CSize siz)				{ m_sizToolbarIconSize = siz; }

	static	int		GetSearchMethod()							{ return m_iSearchMethod; }
	static	void	SetSearchMethod(int iMethod)				{ m_iSearchMethod = iMethod; }

	// ZZ:UploadSpeedSense -->
	static	bool	IsDynUpEnabled()							{ return m_bDynUpEnabled; }
	static	void	SetDynUpEnabled(bool newValue)				{ m_bDynUpEnabled = newValue; }
	static	int		GetDynUpPingTolerance()						{ return m_iDynUpPingTolerance; }
	static	int		GetDynUpGoingUpDivider()					{ return m_iDynUpGoingUpDivider; }
	static	int		GetDynUpGoingDownDivider()					{ return m_iDynUpGoingDownDivider; }
	static	int		GetDynUpNumberOfPings()						{ return m_iDynUpNumberOfPings; }
    static  bool	IsDynUpUseMillisecondPingTolerance()        { return m_bDynUpUseMillisecondPingTolerance;} // EastShare - Added by TAHO, USS limit
	static  int		GetDynUpPingToleranceMilliseconds()         { return m_iDynUpPingToleranceMilliseconds; } // EastShare - Added by TAHO, USS limit
	static  void	SetDynUpPingToleranceMilliseconds(int in)   { m_iDynUpPingToleranceMilliseconds = in; }
	// ZZ:UploadSpeedSense <--

    static bool     GetA4AFSaveCpu()                            { return m_bA4AFSaveCpu; } // ZZ:DownloadManager

	static	CString	GetHomepageBaseURL()						{ return GetHomepageBaseURLForLevel(GetWebMirrorAlertLevel()); }
	static	CString	GetVersionCheckBaseURL();					
	static	void	SetWebMirrorAlertLevel(uint8 newValue)		{ m_nWebMirrorAlertLevel = newValue; }
	static bool	IsDefaultNick(const CString strCheck);
	static	uint8	GetWebMirrorAlertLevel();
	static bool		UseSimpleTimeRemainingComputation()			{ return m_bUseOldTimeRemaining;}

	static	bool	IsRunAsUserEnabled();

	// PeerCache
	static	bool	IsPeerCacheDownloadEnabled()				{ return m_bPeerCacheEnabled; }
	static	uint32	GetPeerCacheLastSearch()					{ return m_uPeerCacheLastSearch; }
	static	bool	WasPeerCacheFound()							{ return m_bPeerCacheWasFound; }
	static	void	SetPeerCacheLastSearch(uint32 dwLastSearch) { m_uPeerCacheLastSearch = dwLastSearch; }
	static	void	SetPeerCacheWasFound(bool bFound)			{ m_bPeerCacheWasFound = bFound; }
	static	uint16	GetPeerCachePort()							{ return m_nPeerCachePort; }
	static	void	SetPeerCachePort(uint16 nPort)				{ m_nPeerCachePort = nPort; }
	static	bool	GetPeerCacheShow()							{ return m_bPeerCacheShow; }

	// Verbose log options
	static	bool	GetEnableVerboseOptions()			{return m_bEnableVerboseOptions;}
	static	bool	GetVerbose()						{return m_bVerbose;}
	static	bool	GetFullVerbose()					{return m_bVerbose && m_bFullVerbose;}
	static	bool	GetDebugSourceExchange()			{return m_bVerbose && m_bDebugSourceExchange;}
	static	bool	GetLogBannedClients()				{return m_bVerbose && m_bLogBannedClients;}
	static	bool	GetLogRatingDescReceived()			{return m_bVerbose && m_bLogRatingDescReceived;}
	static	bool	GetLogSecureIdent()					{return m_bVerbose && m_bLogSecureIdent;}
	static	bool	GetLogFilteredIPs()					{return m_bVerbose && m_bLogFilteredIPs;}
	static	bool	GetLogFileSaving()					{return m_bVerbose && m_bLogFileSaving;}
    static	bool	GetLogA4AF()    					{return m_bVerbose && m_bLogA4AF;} // ZZ:DownloadManager
	static	bool	GetLogUlDlEvents()					{return m_bVerbose && m_bLogUlDlEvents;}
	// [TPT] - WebCache 
	static	bool	GetLogWebCacheEvents()				{return m_bVerbose && m_bLogWebCacheEvents;}//JP log webcache events
	static	bool	GetLogICHEvents()					{return m_bVerbose && m_bLogICHEvents;}//JP log ICH events
	static	bool	GetUseDebugDevice()					{return m_bUseDebugDevice;}
	static	int		GetDebugServerTCPLevel()			{return m_iDebugServerTCPLevel;}
	static	int		GetDebugServerUDPLevel() 			{return m_iDebugServerUDPLevel;}
	static	int		GetDebugServerSourcesLevel()		{return m_iDebugServerSourcesLevel;}
	static	int		GetDebugServerSearchesLevel()		{return m_iDebugServerSearchesLevel;}
	static	int		GetDebugClientTCPLevel()			{return m_iDebugClientTCPLevel;}
	static	int		GetDebugClientUDPLevel()			{return m_iDebugClientUDPLevel;}
	static	int		GetDebugClientKadUDPLevel()			{return m_iDebugClientKadUDPLevel;}
	static	uint8	GetVerboseLogPriority()	{return	m_byLogLevel;} // hard coded now, will of course be selectable later

	// Firewall settings
	static  bool	IsOpenPortsOnStartupEnabled()		{return m_bOpenPortsOnStartUp; }
	
	//AICH Hash
	static	bool	IsTrustingEveryHash()				{return m_bTrustEveryHash;} // this is a debug option
	
    // [TPT] - Powershare
	static	uint8  GetPowerShareMode()	{return m_iPowershareMode;} //MORPH - Added by SiRoB, Avoid misusing of powersharing
	static	uint8	GetPowerShareLimit() {return PowerShareLimit;}
   // [TPT] - Powershare END

	// [TPT] - SUQWT
	static	bool	SaveUploadQueueWaitTime()			{return m_bSaveUploadQueueWaitTime;} 
	static  void    SetSaveUploadQueueWaitTime(bool value) { m_bSaveUploadQueueWaitTime = value; }
	// [TPT] - SUQWT

	//[TPT] - Unlimited upload with no downloads
	static	bool	GetUnlimitedUp()	{return m_bUnlimitedUP;}
	static	void	SetUnlimitedUp(bool value)	{m_bUnlimitedUP = value;}
	//[TPT] - Unlimited upload with no downloads

	// [TPT] - khaos::categorymod+	
	static	bool	ShowCatNameInDownList()	{ return m_bShowCatNames; }
	static	bool	SelectCatForNewDL()		{ return m_bSelCatOnAdd; }
	static	bool	UseActiveCatForLinks()	{ return m_bActiveCatDefault; }
	static	bool	AutoSetResumeOrder()	{ return m_bAutoSetResumeOrder; }	
	static	uint8	StartDLInEmptyCats()	{ return m_iStartDLInEmptyCats; } // 0 = disabled, otherwise num to resume
	static	bool	UseAutoCat()			{ return m_bUseAutoCat; }
	static	void    DownloadNextInThisCat()  { m_bActiveCatDefault = true; 
											   m_bSelCatOnAdd = false;}
	static  bool	ShowPriorityInTab()		{ return m_bShowPriorityInTab; }
	// [TPT] - khaos::categorymod-

	// [TPT] - eWombat SNAFU v2
	static	bool GetAntiSnafu()	{ return m_bSnafu; };
	static	bool GetAntiCreditTheft()	{ return m_bAntiCreditTheft; };
	static	bool GetAntiFriendshare() { return m_bAntiFriendshare; };

	// [TPT] - TBH: minimule
	static	bool	IsMiniMuleEnabled() { return m_bMiniMule;}
	static	void	SetMiniMuleEnabled(bool in) {m_bMiniMule = in; }
	static	uint32	GetMiniMuleUpdate()	{return m_iMiniMuleUpdate;}
	static	void	SetMiniMuleUpdate(uint32 in)  {m_iMiniMuleUpdate = in;}
	static	void	SetMiniMuleLives(bool in) {m_bMiniMuleLives = in; }
	static	bool	GetMiniMuleLives()	{ return m_bMiniMuleLives;}
	static	void	SetMiniMuleTransparency(uint8 in) { m_iMiniMuleTransparency = in; }
	static	uint8	GetMiniMuleTransparency() { return m_iMiniMuleTransparency; }
	// [TPT] - TBH: minimule

	// [TPT] - Select process priority 
	const DWORD GetMainProcessPriority() const { return m_MainProcessPriority; }	
	// [TPT] - Select process priority 	
		
	// [TPT]-Cumulate Bandwidth
	static	bool	IsCumulateBandwidthEnabled() { return cumulateBandwidth; }
	static	void	SetCumulateBandwidth(float in)	{ cumulateBandwidth = in; }
	// [TPT]-Cumulate Bandwidth
	// [TPT] - Pawcio: MUS
	static	bool	MinimizeNumberOfSlots() {return minimizeSlots;}
	static	void	SetMinimizeNumberOfSlots(float in)	{minimizeSlots = in;}
	// [TPT] - Pawcio: MUS
	// [TPT] - Manage Connection
	static	bool	IsManageConnection() {return manageConnection;}
	static	void	SetManageConnection(bool in)	{manageConnection = in;}
	// [TPT] - Manage Connection
	// added by [TPT]-MoNKi [MoNKi: -invisible mode-]
	static	bool GetInvisibleMode()					{ return m_bInvisibleMode; }
	static	UINT GetInvisibleModeHKKeyModifier()	{ return m_iInvisibleModeHotKeyModifier; }
	static	TCHAR GetInvisibleModeHKKey()			{ return m_cInvisibleModeHotKey; }
	static	void SetInvisibleMode(bool on, UINT keymodifier, TCHAR key);
	// End [TPT]-MoNKi 
	
	// [TPT] - MoNKi: -Downloaded History-
	static	bool	GetShowSharedInHistory()		{ return m_bHistoryShowShared; }
	static	void	SetShowSharedInHistory(bool on)	{ m_bHistoryShowShared = on; }
	// [TPT] - MoNKi: -Downloaded History-

	// [TPT] - MoNKi: -Support for High Contrast Mode-
	static	bool	WindowsTextColorOnHighContrast() { return m_bUseWindowsTextColorOnHighContrast; }
	static	void	SetWindowsTextColorOnHighContrast(bool on) { m_bUseWindowsTextColorOnHighContrast = on; }
	// [TPT] - MoNKi: -Support for High Contrast Mode-
	// [TPT] - TBH: minimule
	static	int	GetSpeedMeterMin()		{return speedmetermin;}
	static	int	GetSpeedMeterMax()		{return speedmetermax;}
	static	void	SetSpeedMeterMin(int in)	{speedmetermin = in;}
	static	void	SetSpeedMeterMax(int in) {speedmetermax = in;}
	// [TPT] - TBH: minimule

	// [TPT] - MoNKi: -UPnPNAT Support-
public:
	static	bool	GetUPnPNat()						{ return m_bUPnPNat; }
	static	void	SetUPnPNat(bool on)					{ m_bUPnPNat = on; }
	static	bool	GetUPnPNatWeb()						{ return m_bUPnPNatWeb; }
	static	void	SetUPnPNatWeb(bool on)				{ m_bUPnPNatWeb = on; }
	static	void	SetUPnPTCPExternal(uint16 port)		{ m_iUPnPTCPExternal = port; }
	static	uint16	GetUPnPTCPExternal()				{ return m_iUPnPTCPExternal; }
	static	void	SetUPnPUDPExternal(uint16 port)		{ m_iUPnPUDPExternal = port; }
	static	uint16	GetUPnPUDPExternal()				{ return m_iUPnPUDPExternal; }
	static	void	SetUPnPTCPInternal(uint16 port)		{ m_iUPnPTCPInternal = port; }
	static	uint16	GetUPnPTCPInternal()				{ return m_iUPnPTCPInternal; }
	static	void	SetUPnPUDPInternal(uint16 port)		{ m_iUPnPUDPInternal = port; }
	static	uint16	GetUPnPUDPInternal()				{ return m_iUPnPUDPInternal; }
	static	void	SetUPnPVerboseLog(bool on)			{ m_bUPnPVerboseLog = on; }
	static	bool	GetUPnPVerboseLog()					{ return m_bUPnPVerboseLog; }
	static	void	SetUPnPPort(uint16 port)			{ m_iUPnPUDPInternal = m_iUPnPPort; }
	static	uint16	GetUPnPPort()						{ return m_iUPnPPort; }
	// [TPT] - MoNKi: -UPnPNAT Support-



protected:
	static	CString appdir;
	static	CString configdir;
	static	CString ed2klinkdir; // [TPT] - Save ed2klinks
	static	CString autoHLdir;//[TPT] - Sivka AutoHL
	static	CString	saveSourcesdir;//[TPT] - Save sources
	static	CString m_strWebServerDir;
	static	CString m_strLangDir;
	static	CString m_strFileCommentsFilePath;
	static	CString m_strLogDir;
	static	Preferences_Ext_Struct* prefsExt;
	static	WORD m_wWinVer;
	static	bool m_UseProxyListenPort;
	static	uint16	ListenPort;
	static	CArray<Category_Struct*,Category_Struct*> catMap;
	// [TPT] - itsonlyme: virtualDirs
	static	CRBMap<CString, CString>	m_fileToVDir_map;
	static	CRBMap<CString, CString>	m_dirToVDir_map;
	static	CRBMap<CString, CString>	m_dirToVDirWithSD_map;
	// [TPT] - itsonlyme: virtualDirs

	static void	CreateUserHash();
	static void	SetStandartValues();
	static int	GetRecommendedMaxConnections();
	static void LoadPreferences();
	static void SavePreferences();
	static CString GetHomepageBaseURLForLevel(uint8 nLevel);
	
	static	uint32	m_MainProcessPriority;	// [TPT] - Select process priority 

};

extern CPreferences thePrefs;
