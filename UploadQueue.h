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
#include "updownclient.h" // [TPT]

class CUpDownClient;
typedef CTypedPtrList<CPtrList, CUpDownClient*> CUpDownClientPtrList;

class CUploadQueue
{
	friend class CemuleDlg; // [TPT]
public:
	CUploadQueue();
	~CUploadQueue();

	void	Process();
	void	AddClientToQueue(CUpDownClient* client,bool bIgnoreTimelimit = false);
	// [TPT] - Maella -Upload Stop Reason-
	bool	RemoveFromUploadQueue(CUpDownClient* client, LPCTSTR pszReason = NULL, CUpDownClient::UpStopReason reason = CUpDownClient::USR_NONE, bool updatewindow = true, bool earlyabort = false);
	bool	RemoveFromWaitingQueue(CUpDownClient* client,bool updatewindow = true);
	bool	IsOnUploadQueue(CUpDownClient* client)	const {return (waitinglist.Find(client) != 0);}
	bool	IsDownloading(CUpDownClient* client)	const {return (uploadinglist.Find(client) != 0);}
	
	//not used
	//void    UpdateDatarates();
	//uint32	GetDatarate();
    //uint32  GetToNetworkDatarate();

	bool	CheckForTimeOver(CUpDownClient* client);
	int		GetWaitingUserCount()					{return waitinglist.GetCount();}
	int		GetUploadQueueLength()					{return uploadinglist.GetCount();}
	uint32	GetActiveUploadsCount()					{return m_MaxActiveClientsShortTime;}
	
	uint16	SlotsInRange(bool lowID = false); // [TPT] - Pawcio: MUS
	POSITION GetFirstFromUploadList()				{return uploadinglist.GetHeadPosition();}
	CUpDownClient* GetNextFromUploadList(POSITION &curpos)	{return uploadinglist.GetNext(curpos);}
	CUpDownClient* GetQueueClientAt(POSITION &curpos)	{return uploadinglist.GetAt(curpos);}

	POSITION GetFirstFromWaitingList()				{return waitinglist.GetHeadPosition();}
	CUpDownClient* GetNextFromWaitingList(POSITION &curpos)	{return waitinglist.GetNext(curpos);}
	CUpDownClient* GetWaitClientAt(POSITION &curpos)	{return waitinglist.GetAt(curpos);}

	CUpDownClient*	GetWaitingClientByIP_UDP(uint32 dwIP, uint16 nUDPPort);
	CUpDownClient*	GetWaitingClientByIP(uint32 dwIP);
	CUpDownClient*	GetNextClient(const CUpDownClient* update);
	
	CUpDownClient*	FindClientByWebCacheUploadId(const uint32 id); // [TPT] - WebCache // Superlexx - webcache

	// [TPT] - MFCK [addon] - New Tooltips [Rayita]
	void GetTransferTipInfo(CString &info);
	// [TPT] - MFCK [addon] - New Tooltips [Rayita]
	
	void	DeleteAll();
	uint16	GetWaitingPosition(CUpDownClient* client);
	
	uint32	GetSuccessfullUpCount()					{return successfullupcount;}
	uint32	GetFailedUpCount()						{return failedupcount;}
	uint32	GetAverageUpTime();

	CUpDownClient* FindBestClientInQueue(bool allowLowIdAddNextConnectToBeSet = false);
    void ReSortUploadSlots(bool force = false);

	// [TPT] - Resort
	void		ResortUploadClients(); 
	void		Sort();	
	void		HeapSort(uint16 first, uint16 last);
	void		SwapParts(POSITION pos1, POSITION pos2);
	bool		CompareParts(POSITION pos1, POSITION pos2);		
	// [TPT] - Resort

	// [TPT] - Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	void	CompUploadRate();
protected:
	void	RemoveFromWaitingQueue(POSITION pos, bool updatewindow);
	bool		AcceptNewClient();
	//bool		AcceptNewClient(uint32 curUploadSlots);
	bool		ForceNewClient(bool allowEmptyWaitingQueue = false);

	bool		AddUpNextClient(LPCTSTR pszReason, CUpDownClient* directadd = 0);
	
	static VOID CALLBACK UploadTimer(HWND hWnd, UINT nMsg, UINT nId, DWORD dwTime);

	void	UpdateMaxClientScore(); // [TPT] - Maella Moved here
private:	
	uint32	GetMaxClientScore()						{return m_imaxscore;}
	void    UpdateActiveClientsInfo(DWORD curTick);

    void InsertInUploadingList(CUpDownClient* newclient);
    float GetAverageCombinedFilePrioAndCredit();

	// [TPT] - Friend Slot
	CUpDownClient* GetFriendSlotInUploadQueue()		  { return friendSlotClient;}
	void SetFriendSlotInUploadQueue(CUpDownClient* newClient) { friendSlotClient = newClient; }
	CUpDownClient* friendSlotClient;
	// [TPT] - Friend Slot

	CUpDownClientPtrList waitinglist;
	CUpDownClientPtrList uploadinglist;
	// [TPT] - Maella
	/*// By BadWolf - Accurate Speed Measurement
	typedef struct TransferredData {
		uint32	datalen;
		DWORD	timestamp;
	};
	CList<uint64> avarage_dr_list;
    CList<uint64> avarage_friend_dr_list;
	CList<DWORD,DWORD> avarage_tick_list;	
	CList<int,int> activeClients_list;
    CList<DWORD,DWORD> activeClients_tick_list;
	uint32	datarate;   //datarate sent to network (including friends)
    uint32  friendDatarate; // datarate of sent to friends (included in above total)
	// By BadWolf - Accurate Speed Measurement*/
	CList<int,int> activeClients_list;
	CList<DWORD,DWORD> activeClients_tick_list;

	UINT_PTR h_timer;
	uint32	successfullupcount;
	uint32	failedupcount;
	uint32	totaluploadtime;
	//uint32	m_nLastStartUpload; // [TPT]
	uint32	m_dwRemovedClientByScore;
	// [TPT] - Maella -Pseudo overhead datarate control-
	uint32 m_nextAncTime;          // Used by AcceptNewClient()	
	// Maella end
	
	uint32	m_imaxscore;
	
	DWORD   m_dwLastCalculatedAverageCombinedFilePrioAndCredit;
    float   m_fAverageCombinedFilePrioAndCredit;
    uint32  m_iHighestNumberOfFullyActivatedSlotsSinceLastCall;
    uint32  m_MaxActiveClients;
    uint32  m_MaxActiveClientsShortTime;
    
    //uint64  m_avarage_dr_sum; // [TPT]

    DWORD   m_dwLastResortedUploadSlots;
// [TPT] - Maella -One-queue-per-file- (idea bloodymad)
public:
    bool IsReleaseSlotUsed() const {return m_bIsReleaseSlotUsed;}

private:
    bool m_bIsReleaseSlotUsed;
	uint16  endingTransfer; // [TPT] Pawcio: MUS
	uint16 clientsrdy; // [TPT] Pawcio: MUS
// [TPT] - end
};
