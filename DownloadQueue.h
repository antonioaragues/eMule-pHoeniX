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

// [TPT] - khaos::categorymod+
#include "SelCategoryDlg.h"
#include "MenuCmds.h"
#include "Preferences.h"
// [TPT] - khaos::categorymod-

class CSafeMemFile;
class CSearchFile;
class CUpDownClient;
class CServer;
class CPartFile;
class CSharedFileList;
class CKnownFile;
struct SUnresolvedHostname;
class CPartFileIndex;	// [TPT] - SLUGFILLER: indexPartFiles
namespace Kademlia 
{
	class CUInt128;
};

class CSourceHostnameResolveWnd : public CWnd
{
// Construction
public:
	CSourceHostnameResolveWnd();
	virtual ~CSourceHostnameResolveWnd();

	void AddToResolve(const uchar* fileid, LPCSTR pszHostname, uint16 port, LPCTSTR pszURL = NULL);

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnHostnameResolved(WPARAM wParam, LPARAM lParam);

private:
	struct Hostname_Entry {
		uchar fileid[16];
		CStringA strHostname;
		uint16 port;
		CString strURL;
	};
	CTypedPtrList<CPtrList, Hostname_Entry*> m_toresolve;
	char m_aucHostnameBuffer[MAXGETHOSTSTRUCT];
};


class CDownloadQueue
{
	friend class CAddFileThread;
	friend class CServerSocket;
	friend class CPartFile; // [TPT] - quick start
public:
	CDownloadQueue();
	~CDownloadQueue();

	void	Process();
	void	ProcessDL(sint32 nDownloadSlopeControl, bool isLimited); // [TPT] - Maella -New bandwidth control-
	void	Init();
	
	// [TPT] - quick start
	void	ResetQuickStart(); 
	void	QuickStart();
	// [TPT] - quick start
	
	// add/remove entries
	void	AddPartFilesToShare();
	void	AddDownload(CPartFile* newfile, bool paused);
	// [TPT] - khaos::categorymod+ Modified these three functions by adding and in some cases removing params.
	void	AddSearchToDownload(CSearchFile* toadd, uint8 paused=2, uint8 cat = 0, uint16 useOrder = 0);
	void	AddSearchToDownload(CString link, uint8 paused=2, uint8 cat = 0, uint16 useOrder = 0);
	void	AddFileLinkToDownload(class CED2KFileLink* pLink, bool AllocatedLink = false, bool SkipQueue = false, bool AddedPaused = thePrefs.AddNewFilesPaused());
	// [TPT] - khaos::categorymod-
	void	AddSources(class CED2KFileLink* pLink); // [TPT] - Add sources
	void	RemoveFile(CPartFile* toremove);
	void	DeleteAll();

	int		GetFileCount() {return filelist.GetCount();}
	UINT	GetDownloadingFileCount() const;
	uint16	GetPausedFileCount();

	bool	IsFileExisting(const uchar* fileid, bool bLogWarnings = true);
	bool	IsPartFile(const CKnownFile* file) const;
	bool	IsTempFile(const CString& rstrDirectory, const CString& rstrName) const;	// SLUGFILLER: SafeHash

	CPartFile* GetFileByID(const uchar* filehash) const;
	CPartFile* GetFileByIndex(int index) const;
	CPartFile* GetFileByKadFileSearchID(uint32 ID) const;
	CPartFile*	GetFileByMetFileName(const CString& rstrName) const;	// [TPT] - SLUGFILLER: indexPartFiles
	void    StartNextFileIfPrefs(int cat);
	// [TPT] - khaos::categorymod+	
	bool	StartNextFile(int Category = -1, bool force = false);
	// [TPT] - khaos::categorymod-
	void	DisableAllA4AFAuto(void);

	// sources
	CUpDownClient* GetDownloadClientByIP(uint32 dwIP);
	CUpDownClient* GetDownloadClientByIP_UDP(uint32 dwIP, uint16 nUDPPort);
	bool	IsInList(const CUpDownClient* client) const;

	bool    CheckAndAddSource(CPartFile* sender,CUpDownClient* source);
	bool    CheckAndAddKnownSource(CPartFile* sender,CUpDownClient* source, bool bIgnoreGlobDeadList = false);
	bool	RemoveSource(CUpDownClient* toremove, bool bDoStatsUpdate = true);

	// statistics
	typedef struct{
		int	a[23];
	} SDownloadStats;
	void	GetDownloadStats(SDownloadStats& results);
	void	GetDownloadStats(int results[],uint64& pui64TotFileSize,uint64& pui64TotBytesLeftToTransfer,uint64& pui64TotNeededSpace);
	//uint32	GetDatarate() {return datarate;} // [TPT] - Maella
	// [TPT]
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	void	CompDownloadRate();

	void	AddUDPFileReasks()								{m_nUDPFileReasks++;}
	uint32	GetUDPFileReasks() const						{return m_nUDPFileReasks;}
	void	AddFailedUDPFileReasks()						{m_nFailedUDPFileReasks++;}
	uint32	GetFailedUDPFileReasks() const					{return m_nFailedUDPFileReasks;}

	// categories
	
	// [TPT] - khaos::categorymod+	
	void	StopPauseLastFile(int Mode = MP_PAUSE, int Category = -1);
	uint16	GetMaxCatResumeOrder(uint8 iCategory = 0);
	void	GetCategoryFileCounts(uint8 iCategory, int cntFiles[]);
	uint16	GetCategoryFileCount(uint8 iCategory);
	uint16	GetHighestAvailableSourceCount(int nCat = -1);
	uint16	GetCatActiveFileCount(uint8 iCategory);
	uint8	GetAutoCat(CString sFullName, ULONG nFileSize);
	bool	ApplyFilterMask(CString sFullName, uint8 nCat);
	// [TPT] - khaos::categorymod-
	
	// [TPT]
	
	// [TPT] - khaos::categorymod+ Added parameter: useCat
	void	ResetCatParts(int cat, uint8 useCat = 0);
	// [TPT] - khaos::categorymod-	
	void	SetCatPrio(int cat, uint8 newprio);
    void    RemoveAutoPrioInCat(int cat, uint8 newprio); // ZZ:DownloadManager
	void	SetCatStatus(int cat, int newstatus);
	void	MoveCat(uint8 from, uint8 to);
	//void	SetAutoCat(CPartFile* newfile); // [TPT] - khaos::categorymod
	

	// searching on local server
	void	SendLocalSrcRequest(CPartFile* sender);
	void	RemoveLocalServerRequest(CPartFile* pFile);
	void	ResetLocalServerRequests();

	// searching in Kad
	void	SetLastKademliaFileRequest()				{lastkademliafilerequest = ::GetTickCount();}
	bool	DoKademliaFileRequest();
	void	KademliaSearchFile(uint32 searchID, const Kademlia::CUInt128* pcontactID, const Kademlia::CUInt128* pkadID, uint8 type, uint32 ip, uint16 tcp, uint16 udp, uint32 serverip, uint16 serverport, uint32 clientid);

	// searching on global servers
	void	StopUDPRequests();

	// check diskspace
	void	SortByPriority();
	void	CheckDiskspace(bool bNotEnoughSpaceLeft = false); // SLUGFILLER: checkDiskspace
	void	CheckDiskspaceTimed();
	// [TPT] - SLUGFILLER: indexPartFiles remove - obsolete
	void	OnConnectionState(bool bConnected);
	
	void	AddToResolved( CPartFile* pFile, SUnresolvedHostname* pUH );


	// [TPT] - Sivka AutoHL Begin
	void InitTempVariables(CPartFile* file);
	void UpdateFileSettings(CPartFile* file);
	// [TPT] - Sivka AutoHL end

	CServer* cur_udpserver;

	// [TPT] - WebCache
	bool	ContainsUnstoppedFiles(); //jp webcache release

protected:
	bool	SendNextUDPPacket();
	void	ProcessLocalRequests();
	int		GetMaxFilesPerUDPServerPacket() const;
	bool	SendGlobGetSourcesUDPPacket(CSafeMemFile* data);

private:
	// SLUGFILLER: checkDiskspace
	bool	CompareParts(POSITION pos1, POSITION pos2);
	void	SwapParts(POSITION pos1, POSITION pos2);
	void	HeapSort(uint16 first, uint16 last);
	// SLUGFILLER: checkDiskspace
	CTypedPtrList<CPtrList, CPartFile*> filelist;
	CTypedPtrList<CPtrList, CPartFile*> m_localServerReqQueue;
	uint16	filesrdy;
	
	CPartFile*	lastfile;
	uint32		lastcheckdiskspacetime;	// SLUGFILLER: checkDiskspace
	uint32		lastudpsearchtime;
	uint32		lastudpstattime;
	uint8		udcounter;
	uint8		m_cRequestsSentToServer;
	uint32		m_dwNextTCPSrcReq;
	int			m_iSearchedServers;
	uint32		lastkademliafilerequest;

	//uint64		m_datarateMS; // [TPT] - Maella
	uint32		m_nUDPFileReasks;
	uint32		m_nFailedUDPFileReasks;
	// [TPT]
	// Maella -Pseudo overhead datarate control-
	uint32 m_lastProcessTime;
	uint64 m_lastOverallReceivedBytes;	
	uint64 m_lastReceivedBytes;	
	sint32 m_nDownloadSlopeControl;
	// Maella end
	
	bool m_fileListChange; // [TPT] - Xman possible crashfix

	/*// By BadWolf - Accurate Speed Measurement
	typedef struct TransferredData {
		uint32	datalen;
		DWORD	timestamp;
	};
	CList<TransferredData> avarage_dr_list;
	// END By BadWolf - Accurate Speed Measurement*/

	
	//[TPT] - Unlimited upload with no downloads
	void CanSwitchUnlimited();
	bool IsThereAnyDownload();
	uint32 timeAllowUnlimitedUP;
	//[TPT] - Unlimited upload with no downloads
	// [TPT] - quick start
	int quickflag;
	int quickflags;
	uint32 MaxconnPerFiveBack;
	uint32 MaxconBack;
	DWORD quicktime;
	int countfiles;
	bool manageConnection;
	// [TPT] - quick start
	
	// [TPT] - khaos::categorymod+ For queuing ED2K link additions.
	bool		m_bBusyPurgingLinks;
	bool		PurgeED2KLinkQueue();
	uint32		m_iLastLinkQueuedTick;

	CTypedPtrList<CPtrList, CED2KFileLink*> m_ED2KLinkQueue;
	// [TPT] - khaos::categorymod-
	CPartFileIndex* partfileindex;	// [TPT] - SLUGFILLER: indexPartFiles

	CSourceHostnameResolveWnd m_srcwnd;

    DWORD       m_dwLastA4AFtime; // ZZ:DownloadManager
public:
	float upTemp;//[TPT] - Unlimited upload with no downloads
	bool unlimitedFlag;

// [TPT] - MFCK [addon] - New Tooltips [Rayita]
	void	GetTransferTipInfo(CString &info);
	void	GetTipInfoByCat(uint8 cat, CString &info);
// [TPT] - MFCK [addon] - New Tooltips [Rayita]
};
