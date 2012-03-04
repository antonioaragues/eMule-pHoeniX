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
#include "stdafx.h"
#include "emule.h"
#include "UploadQueue.h"
#include "Packets.h"
#include "KnownFile.h"
#include "ListenSocket.h"
#include "Exceptions.h"
#include "Scheduler.h"
#include "PerfLog.h"
#include "UploadBandwidthThrottler.h"
#include "ClientList.h"
#include "LastCommonRouteFinder.h"
#include "DownloadQueue.h"
#include "FriendList.h"
#include "Statistics.h"
#include "MMServer.h"
#include "OtherFunctions.h"
#include "UpDownClient.h"
#include "SharedFileList.h"
#include "KnownFileList.h"
#include "Sockets.h"
#include "ClientCredits.h"
#include "Server.h"
#include "ServerList.h"
#include "WebServer.h"
#include "emuledlg.h"
#include "ServerWnd.h"
#include "TransferWnd.h"
#include "SearchDlg.h"
#include "StatisticsDlg.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "Kademlia/Kademlia/Prefs.h"
#include "Log.h"
#include "BandWidthControl.h" // [TPT]
#include "PartFile.h" // [TPT]
#include "Safefile.h" // [TPT] - Powershare

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif


// [TPT]

//TODO rewrite the whole networkcode, use overlapped sockets.. sure....

CUploadQueue::CUploadQueue()
{
	// [TPT]
	//VERIFY( (h_timer = SetTimer(0,0,100,UploadTimer)) != NULL );
	//if (thePrefs.GetVerbose() && !h_timer)
	//	AddDebugLogLine(true,_T("Failed to create 'upload queue' timer - %s"),GetErrorMessage(GetLastError()));
	//datarate = 0;
	//counter=0;
	successfullupcount = 0;
	failedupcount = 0;
	totaluploadtime = 0;

	//m_nLastStartUpload = 0;
	//statsave=0;
	// -khaos--+++>
	//iupdateconnstats=0;
	// <-----khaos-
	m_dwRemovedClientByScore = ::GetTickCount();
	// Maella -Pseudo overhead datarate control-
	m_nextAncTime = ::GetTickCount() + 5000;	
	// Maella end	
	endingTransfer = 0; // [TPT] - Pawcio: MUS
	clientsrdy = 0; // [TPT] - Pawcio: MUS
 	m_iHighestNumberOfFullyActivatedSlotsSinceLastCall = 0;
    m_MaxActiveClients = 0;
    m_MaxActiveClientsShortTime = 0;
    
    // [TPT]
    /*m_lastCalculatedDataRateTick = 0;
    m_avarage_dr_sum = 0;
    friendDatarate = 0;*/

    m_dwLastResortedUploadSlots = 0;

	SetFriendSlotInUploadQueue(NULL); // [TPT] - Friend Slot
}

/**
 * Find the highest ranking client in the waiting queue, and return it.
 *
 * Low id client are ranked as lowest possible, unless they are currently connected.
 * A low id client that is not connected, but would have been ranked highest if it
 * had been connected, gets a flag set. This flag means that the client should be
 * allowed to get an upload slot immediately once it connects.
 *
 * @return address of the highest ranking client.
 */
CUpDownClient* CUploadQueue::FindBestClientInQueue(bool allowLowIdAddNextConnectToBeSet) {
	POSITION toadd = 0;
	POSITION toaddPS = 0; // [TPT] - Powershare
	POSITION toaddlow = 0;
	POSITION toaddlowPS = 0; // [TPT] - Powershare
	uint32	bestscore = 0;
	uint32	bestscorePS = 0; // [TPT] - Powershare
	uint32  bestlowscore = 0;
	uint32  bestlowscorePS = 0; // [TPT] - Powershare
	//CUpDownClient* newclient = NULL;
    //CUpDownClient* lowclient = NULL;

	// [TPT] - Maella code Improvement
	for (POSITION pos = waitinglist.GetHeadPosition(); pos != NULL;)
	{
		POSITION cur_pos = pos;
		CUpDownClient* cur_client =	waitinglist.GetNext(pos);
		//While we are going through this list.. Lets check if a client appears to have left the network..
		ASSERT ( cur_client->GetLastUpRequest() );
		if ((::GetTickCount() - cur_client->GetLastUpRequest() > MAX_PURGEQUEUETIME) || !theApp.sharedfiles->GetFileByID(cur_client->GetUploadFileID()) )
		{
			//This client has either not been seen in a long time, or we no longer share the file he wanted anymore..
			cur_client->ClearWaitStartTime();
			RemoveFromWaitingQueue(cur_pos,true);					
			continue;
		}
		//<<< [TPT] - eWombat SNAFU v2 ignore banned clients
		if (!cur_client->UploadAllowed())
			continue;
		//>>> [TPT] - eWombat SNAFU v2
		// finished clearing
		uint32 cur_score = cur_client->GetScore(false);
		// [TPT] - Powershare
		if (cur_client->GetPowerShared())
		{
			if ( cur_score > bestscorePS)
			{
				// cur_client is more worthy than current best client that is ready to go (connected).
                if(!cur_client->HasLowID() || (cur_client->socket && cur_client->socket->IsConnected())) 
				{
					// this client is a HighID or a lowID client that is ready to go (connected)
                    // and it is more worthy
					bestscorePS = cur_score;
					toaddPS = cur_pos;
					//newclient = waitinglist.GetAt(toaddPS);
				} else if(allowLowIdAddNextConnectToBeSet && !cur_client->m_bAddNextConnect) 
				{
                    // this client is a lowID client that is not ready to go (not connected)    
                    // now that we know this client is not ready to go, compare it to the best not ready client
                    // the best not ready client may be better than the best ready client, so we need to check
                    // against that client
			        if (cur_score > bestlowscore)
			        {
                        // it is more worthy, keep it
						bestlowscore = cur_score;
						toaddlowPS = cur_pos;
						//lowclient = waitinglist.GetAt(toaddlowPS);
					}
				}
            } else {
                // cur_client is more worthy. Save it.
			}
		}
		else
		{
			// [TPT] - Powershare
			if ( cur_score > bestscore)
			{
				// cur_client is more worthy than current best client that is ready to go (connected).
                if(!cur_client->HasLowID() || (cur_client->socket && cur_client->socket->IsConnected())) 
				{
                    // this client is a HighID or a lowID client that is ready to go (connected)
                    // and it is more worthy
					bestscore = cur_score;
					toadd = cur_pos;
					//newclient = waitinglist.GetAt(toadd);
                } else if(allowLowIdAddNextConnectToBeSet && !cur_client->m_bAddNextConnect) 
				{
                    // this client is a lowID client that is not ready to go (not connected)
    
                    // now that we know this client is not ready to go, compare it to the best not ready client
                    // the best not ready client may be better than the best ready client, so we need to check
                    // against that client
			        if (cur_score > bestlowscore)
			        {
                        // it is more worthy, keep it
						bestlowscore = cur_score;
						toaddlow = cur_pos;
						//lowclient = waitinglist.GetAt(toaddlow);
					}
				}		
			} 
			else {
                // cur_client is more worthy. Save it.
            }	
		}
	}
			
	// [TPT] - Powershare
	if (bestlowscorePS > bestscorePS && toaddlowPS && allowLowIdAddNextConnectToBeSet){
		waitinglist.GetAt(toaddlowPS)->m_bAddNextConnect = true;
	}
	else if (bestlowscore > bestscore && toaddlow && allowLowIdAddNextConnectToBeSet && !toaddPS)
	{
		waitinglist.GetAt(toaddlow)->m_bAddNextConnect = true;
	}

	if (!toaddPS && !toadd)
	{
		return NULL;
	}
	
	// Check if normal client has a friend slot or a release one
	if (toadd == 0)
		return waitinglist.GetAt(toaddPS);
	else if (toaddPS == 0)
		return waitinglist.GetAt(toadd);
	else
	{
		// [TPT] - Friend Slot
		CUpDownClient* normalClient = waitinglist.GetAt(toadd);
		if ((normalClient->GetFriendSlot() && GetFriendSlotInUploadQueue() == NULL) ||		
			(((theApp.sharedfiles->GetFileByID(normalClient->GetUploadFileID()))->GetUpPriority() == PR_VERYHIGH) && // Release slot
			!IsReleaseSlotUsed()))
			return normalClient;
		// [TPT] - Friend Slot
	}

	return waitinglist.GetAt(toaddPS);
	// [TPT] - Powershare END
}

void CUploadQueue::InsertInUploadingList(CUpDownClient* newclient) {
 	//Lets make sure any client that is added to the list has this flag reset!
	newclient->m_bAddNextConnect = false;
   // Add it last
    theApp.uploadBandwidthThrottler->AddToStandardList(uploadinglist.GetCount(), newclient->GetFileUploadSocket());
	uploadinglist.AddTail(newclient);
    newclient->SetSlotNumber(uploadinglist.GetCount());
	// [TPT] - Friend Slot
	if (newclient->GetFriendSlot())
		SetFriendSlotInUploadQueue(newclient);
	// [TPT] - Friend Slot
}

bool CUploadQueue::AddUpNextClient(LPCTSTR pszReason, CUpDownClient* directadd){
	CUpDownClient* newclient = NULL;
	// select next client or use given client
	if (!directadd)
	{
        newclient = FindBestClientInQueue(true);

        if(newclient) 
		{
		    RemoveFromWaitingQueue(newclient, true);
			//[TPT] - We are doing in RemoveFromWaitingQueue
			//theApp.emuledlg->transferwnd->ShowQueueCount(waitinglist.GetCount());
		}
	}
	else 
	{
	    // [TPT] - eWombat SNAFU v2
		if (directadd->IsSnafu() || directadd->IsNotSUI() || directadd->IsBanned())
			return false;

		newclient = directadd;
	}

    if(newclient == NULL) 
        return false;

	if (!thePrefs.TransferFullChunks())
		UpdateMaxClientScore(); // refresh score caching, now that the highest score is removed

	if (IsDownloading(newclient))
		return false;



	// [TPT] - eWombat SNAFU v2
	//CHECK Snafus and suis
	if (newclient->IsSnafu() || newclient->IsNotSUI() || newclient->IsBanned())
	{
		newclient->ClearWaitStartTime(); //Xman modification
		return false;
	}
	// [TPT] - eWombat SNAFU v2
	// tell the client that we are now ready to upload
	if (!newclient->socket || !newclient->socket->IsConnected())
	{
		newclient->SetUploadState(US_CONNECTING);
		if (!newclient->TryToConnect(true))
			return false;
		if (!newclient->socket) // Pawcio: BC
		{
			LogWarning(false,_T("---- Trying to add new client in queue with NULL SOCKET: %s"),newclient->DbgGetClientInfo());
			newclient->SetUploadState(US_NONE);
			return false;
		}
	}
	else
	{
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__AcceptUploadReq", newclient);
		Packet* packet = new Packet(OP_ACCEPTUPLOADREQ,0);
		theStats.AddUpDataOverheadFileRequest(packet->size);
		newclient->socket->SendPacket(packet,true);
		newclient->SetUploadState(US_UPLOADING);
	}
	newclient->SetUpStartTime();
	newclient->ResetSessionUp();

    	InsertInUploadingList(newclient);

    if(pszReason && thePrefs.GetLogUlDlEvents())
        AddDebugLogLine(false, _T("Adding client to upload list: %s Client: %s"), pszReason, newclient->DbgGetClientInfo());

	
	// [TPT] - Maella -Pseudo overhead datarate control-
	//m_nLastStartUpload = ::GetTickCount();
	// Let 20 seconds to the system to stabilize the upload datarate
	if(::GetTickCount() - m_nextAncTime < 20000){ // [TPT] - MUS
		m_nextAncTime = GetTickCount() + 20000;
	}
	// Maella end
	
	// statistic
	CKnownFile* reqfile = theApp.sharedfiles->GetFileByID((uchar*)newclient->GetUploadFileID());
	if (reqfile){
		reqfile->statistic.AddAccepted();
	}
		
	theApp.emuledlg->transferwnd->uploadlistctrl.AddClient(newclient);

	return true;
}

void CUploadQueue::UpdateActiveClientsInfo(DWORD curTick) {
    // Save number of active clients for statistics
    uint32 tempHighest = theApp.uploadBandwidthThrottler->GetHighestNumberOfFullyActivatedSlotsSinceLastCallAndReset();

    if(thePrefs.GetLogUlDlEvents() && theApp.uploadBandwidthThrottler->GetStandardListSize() > (uint32)uploadinglist.GetSize()) {
        // debug info, will remove this when I'm done.
        //AddDebugLogLine(false, _T("UploadQueue: Error! Throttler has more slots than UploadQueue! Throttler: %i UploadQueue: %i Tick: %i"), theApp.uploadBandwidthThrottler->GetStandardListSize(), uploadinglist.GetSize(), ::GetTickCount());
		if(tempHighest > (uint32)uploadinglist.GetSize()) {
        	tempHighest = uploadinglist.GetSize();
		}
    }

    m_iHighestNumberOfFullyActivatedSlotsSinceLastCall = tempHighest;

    // save 15 minutes of data about number of fully active clients
    uint32 tempMaxRemoved = -1;
    while(!activeClients_tick_list.IsEmpty() && !activeClients_list.IsEmpty() && curTick-activeClients_tick_list.GetHead() > 20*1000) {
        activeClients_tick_list.RemoveHead();
	    uint32 removed = activeClients_list.RemoveHead();

        if(removed > tempMaxRemoved) {
            tempMaxRemoved = removed;
        }
    }

	activeClients_list.AddTail(m_iHighestNumberOfFullyActivatedSlotsSinceLastCall);
    activeClients_tick_list.AddTail(curTick);

    if(activeClients_tick_list.GetSize() > 1) {
        uint32 tempMaxActiveClients = m_iHighestNumberOfFullyActivatedSlotsSinceLastCall;
        uint32 tempMaxActiveClientsShortTime = m_iHighestNumberOfFullyActivatedSlotsSinceLastCall;
        POSITION activeClientsTickPos = activeClients_tick_list.GetTailPosition();
        POSITION activeClientsListPos = activeClients_list.GetTailPosition();
        while(activeClientsListPos != NULL && (tempMaxRemoved > tempMaxActiveClients && tempMaxRemoved >= m_MaxActiveClients || curTick - activeClients_tick_list.GetAt(activeClientsTickPos) < 10 * 1000)) {
            DWORD activeClientsTickSnapshot = activeClients_tick_list.GetAt(activeClientsTickPos);
            uint32 activeClientsSnapshot = activeClients_list.GetAt(activeClientsListPos);

            if(activeClientsSnapshot > tempMaxActiveClients) {
                tempMaxActiveClients = activeClientsSnapshot;
            }

            if(activeClientsSnapshot > tempMaxActiveClientsShortTime && curTick - activeClientsTickSnapshot < 10 * 1000) {
                tempMaxActiveClientsShortTime = activeClientsSnapshot;
            }

            activeClients_tick_list.GetPrev(activeClientsTickPos);
            activeClients_list.GetPrev(activeClientsListPos);
        }

        if(tempMaxRemoved > m_MaxActiveClients) {
            m_MaxActiveClients = tempMaxActiveClients;
        }

        m_MaxActiveClientsShortTime = tempMaxActiveClientsShortTime;
    } else {
        m_MaxActiveClients = m_iHighestNumberOfFullyActivatedSlotsSinceLastCall;
        m_MaxActiveClientsShortTime = m_iHighestNumberOfFullyActivatedSlotsSinceLastCall;
    }
}

// [TPT] - Maella -New bandwidth control-
/**
 * Maintenance method for the uploading slots. It adds and removes clients to the
 * uploading list. It also makes sure that all the uploading slots' Sockets always have
 * enough packets in their queues, etc.
 *
 * This method is called approximately once every 100 milliseconds.
 */
void CUploadQueue::Process() 
{
	DWORD curTick = ::GetTickCount();

    UpdateActiveClientsInfo(curTick);
    
	// Only every 1 second
    static s_counter;
    if(s_counter++ >= (1000/TIMER_PERIOD))
	{
		// [TPT] - WebCache 
		////JP Proxy configuration testing START!!! This should probably be somewhere else.
		if (thePrefs.expectingWebCachePing && (::GetTickCount() - thePrefs.WebCachePingSendTime > SEC2MS(30)))
		{
			thePrefs.expectingWebCachePing = false;
			thePrefs.WebCacheDisabledThisSession = true; //Disable webcache downloads for the current proxy settings
			//JP we need a modeless dialogue here!!
//			AfxMessageBox(_T("Proxy configuration Test Failed please review your proxy-settings"));
			theApp.QueueLogLine(false, _T("Proxy configuration Test Failed please review your proxy-settings. Webcache downloads have been deactivated until emule is restarted."));
		}
		////JP Proxy configuration testing END!!! This should probably be somewhere else.

        s_counter = 0;

        // Maella -One-queue-per-file- (idea bloodymad)
		endingTransfer = 0; // [TPT] - Pawcio: MUS
	    for(POSITION pos = uploadinglist.GetHeadPosition(); pos != NULL; )
		{
			POSITION cur_pos = pos;
		    CUpDownClient* cur_client = uploadinglist.GetNext(pos);
			if (cur_client)
			{
				// [TPT] - Pawcio: MUS
				if (cur_client->IsRequest() && !cur_client->toRemove && CheckForTimeOver(cur_client) && !cur_client->GetFriendSlot())
				{
					cur_client->toRemove = true;
					endingTransfer++;
				}
				else if (cur_client->toRemove)
					endingTransfer++;
				// [TPT] - Pawcio: MUS
				}
			else
					uploadinglist.RemoveAt(cur_pos);
        }

	    // Check if new client can be added to upload queue
        if(AcceptNewClient() && waitinglist.GetCount())
		{
			AddUpNextClient(_T("We need another upload slot"));
		}
	}

	if (!uploadinglist.GetCount()){
		clientsrdy = 0; // [TPT] Pawcio: MUS
		return;
	}
	
	ResortUploadClients(); // [TPT] - Resort		

	POSITION pos = uploadinglist.GetHeadPosition();
	// [TPT] // Don't use an iterator => swap in loop	
	// Count number of client ready to send block
	clientsrdy = 0;  // [TPT] - Pawcio: MUS
	for (int i = 0; i < uploadinglist.GetCount(); i++)	
	{
		POSITION cur_pos = pos;
		CUpDownClient* cur_client = uploadinglist.GetNext(pos);
		if (thePrefs.m_iDbgHeap >= 2)
			ASSERT_VALID(cur_client);
		//It seems chatting or friend slots can get stuck at times in upload.. This needs looked into..
		if (cur_client && !cur_client->socket)
		{			
			RemoveFromUploadQueue(cur_client, _T("Uploading to client without socket? (CUploadQueue::Process)"), CUpDownClient::USR_NONE);
			if(cur_client->Disconnected(_T("CUploadQueue::Process")))
			{
				delete cur_client;
			}
		} 
		else if(cur_client) 
		{
				// [TPT] - MUS
			if (!cur_client->socket->isBusy() && cur_client->HasBlocks() == true)
			{
					++clientsrdy;
			}

			cur_client->SendBlockData();
        }
	}	
};

// [TPT] - Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
void CUploadQueue::CompUploadRate(){	
	// Compute the upload datarate of all clients
	for(POSITION pos = uploadinglist.GetHeadPosition(); pos != NULL; ){
		uploadinglist.GetNext(pos)->CompUploadRate();
	}
}
// Maella end

// [TPT] - Maella
bool CUploadQueue::AcceptNewClient()
{
	// [TPT] - Pawcio: MUS
	if (thePrefs.MinimizeNumberOfSlots())
	{ 	
		/*if (uploadinglist.GetCount() < thePrefs.GetMinUploadSlot())
		{
		m_nextAncTime = GetTickCount() + 3000;
		return true;
		}*/
		if (::GetTickCount() < m_nextAncTime)
			return false;
		uint16 openedSlots = uploadinglist.GetCount();
		uint16 nMaxSlots = SlotsInRange();
		if (!nMaxSlots)		
			return false;
		if (endingTransfer && openedSlots < thePrefs.GetMinUploadSlot())
		{
			m_nextAncTime = GetTickCount() + 5000;
			return true;
		}
		uint16 nMaxReadySlots = 2;
		if (openedSlots >= nMaxSlots/2)
			nMaxReadySlots = 1;
		if (clientsrdy < nMaxReadySlots + endingTransfer)
		{
			m_nextAncTime = GetTickCount() + 3000;
			return true;
		}
		return false;
	}
	else if(thePrefs.GetNAFCEnable() == false && thePrefs.GetNAFCFullControl() == false)
		// [TPT] - Pawcio: MUS
	{
		// Check hard lower/upper bounds
        if (uploadinglist.GetCount() < thePrefs.GetMinUploadSlot())
		{
			m_nextAncTime = GetTickCount() + 2000;
			return true;
		}
		else if (uploadinglist.GetCount() >= MAX_UP_CLIENTS_ALLOWED)
		{
				return false;
		}

		// check if we can allow a new client to start downloading form us
		if(GetTickCount() < m_nextAncTime)
			return false;

		// Time stamp for next evaluation
		m_nextAncTime = GetTickCount() + 2000;

		float MaxSpeed;

		if (thePrefs.IsDynUpEnabled())
			MaxSpeed = theApp.lastCommonRouteFinder->GetUpload()/1024.0f;        
		else
			MaxSpeed = thePrefs.GetMaxUpload();
		
		uint32 upPerClient = UPLOAD_CLIENT_DATARATE;
		uint32 curUploadSlots = (uint32)uploadinglist.GetCount();

		// Compute all datarates elapsed for the last 5 seconds
		uint32 eMuleIn;	uint32 eMuleInOverall;
		uint32 eMuleOut; uint32 eMuleOutOverall;
		uint32 networkIn; uint32 networkOut;
		theApp.pBandWidthControl->GetDatarates(5, // 5 seconds
												eMuleIn, eMuleInOverall,
												eMuleOut, eMuleOutOverall,
												networkIn, networkOut);

		if (curUploadSlots < thePrefs.GetMinUploadSlot()) // [TPT] - Maella -Minimum Upload Slot-
			return true;
		else if (curUploadSlots >= MAX_UP_CLIENTS_ALLOWED ||
				curUploadSlots >= 4 &&
				(
				curUploadSlots >= (eMuleOut/UPLOAD_CHECK_CLIENT_DR) ||
				curUploadSlots >= ((uint32)MaxSpeed)*1024/UPLOAD_CLIENT_DATARATE ||
				!theApp.lastCommonRouteFinder->AcceptNewClient() || // upload speed sense can veto a new slot if USS enabled
				(
				thePrefs.GetMaxUpload() >= UNLIMITED && // [TPT]
				!thePrefs.IsDynUpEnabled() &&
				thePrefs.GetMaxGraphUploadRate() > 0 &&
				curUploadSlots >= ((uint32)thePrefs.GetMaxGraphUploadRate())*1024/UPLOAD_CLIENT_DATARATE
				)
				)
				) // max number of clients to allow for all circumstances
			return false;

		if(theApp.uploadBandwidthThrottler->GetHighestNumberOfFullyActivatedSlotsSinceLastCallAndReset() > (uint32)uploadinglist.GetSize()) {
			// uploadThrottler requests another slot. If throttler says it needs another slot, we will allow more slots
			// than what we require ourself. Never allow more slots than to give each slot high enough average transfer speed, though (checked above).
			return true;
		}

		// if throttler doesn't require another slot, go with a slightly more restrictive method
		if( MaxSpeed > 20 || MaxSpeed >= UNLIMITED) // [TPT]
			upPerClient += eMuleOut/43;

		if( upPerClient > 7680 )
			upPerClient = 7680;

		//now the final check

		if ( MaxSpeed >= UNLIMITED ) // [TPT]
		{
			if (curUploadSlots < (eMuleOut/upPerClient))
				return true;
		}
		else{
			uint16 nMaxSlots;
			if (MaxSpeed > 12)
				nMaxSlots = (uint16)(((float)(MaxSpeed*1024)) / upPerClient);
			else if (MaxSpeed > 7)
				nMaxSlots = thePrefs.GetMinUploadSlot() + 2; // [TPT] - Maella -Minimum Upload Slot-
			else if (MaxSpeed > 3)
				nMaxSlots = thePrefs.GetMinUploadSlot() + 1; // [TPT] - Maella -Minimum Upload Slot-
			else
				nMaxSlots = thePrefs.GetMinUploadSlot(); // [TPT] - Maella -Minimum Upload Slot-
//		AddLogLine(true,"maxslots=%u, upPerClient=%u, datarateslot=%u|%u|%u",nMaxSlots,upPerClient,datarate/UPLOAD_CHECK_CLIENT_DR, datarate, UPLOAD_CHECK_CLIENT_DR);

			if ( curUploadSlots < nMaxSlots )
			{
				return true;
			}
		}
		//nope
		return false;
	}	
	else 
	// [TPT] - New Upload slot sharping
	{
		// Check hard lower/upper bounds
		if (uploadinglist.GetCount() < thePrefs.GetMinUploadSlot())
		{
			m_nextAncTime = GetTickCount() + 5000;
			return true;
		}
		else if (uploadinglist.GetCount() >= MAX_UP_CLIENTS_ALLOWED) // 350
		{
			return false;
		}

		// Proceed evaluation only every n seconds
		if(GetTickCount() >= m_nextAncTime) 
		{
			// Time stamp for next evaluation
			m_nextAncTime = GetTickCount() + 5000;

			// Compute all datarates elapsed for the last 5 seconds
			uint32 eMuleIn;	uint32 eMuleInOverall;
			uint32 eMuleOut; uint32 eMuleOutOverall;
			uint32 networkIn; uint32 networkOut;
			theApp.pBandWidthControl->GetDatarates(5, // 5 seconds
												eMuleIn, eMuleInOverall,
												eMuleOut, eMuleOutOverall,
												networkIn, networkOut);

			// Calculate the max average upper limit per client
			uint32 upPerClient = 4000 + eMuleOut/50;
			if(upPerClient > 7680)			
				upPerClient = 7680;            

			// Calculate max Slot
			uint16 nMaxSlots = thePrefs.GetMinUploadSlot(); // range 2..4
			if (thePrefs.GetEnableNewUSS() == true)
				{
					const uint32 NAFCnetworkOut = (uint32)(1024.0f * thePrefs.GetMaxUpload());
					const uint32 overHead = (networkOut > eMuleOut) ? (networkOut - eMuleOut) : 0;
					const uint32 availableOut = overHead > NAFCnetworkOut ? 0 : NAFCnetworkOut - overHead;				
					const uint32 dynMaxSlots = 2 + (availableOut + upPerClient - 1) / upPerClient;
					if(nMaxSlots < dynMaxSlots)
						nMaxSlots = dynMaxSlots;
				}
				else
				{
					const uint32 availableOut = (uint32)(1024.0f * thePrefs.GetMaxUpload());
					// e.g. MaxUpload = 25 [KB/s]
					//  no upload   => upPerClient = 4000 + 0*1024/50 = 4000
					//                 nMaxSlot = 2 + (25*1024 + 4000 - 1) / 4000 = 2 + 7.39 = 9
					//
					//  full upload => upPerClient = 4000 + 25*1024/50 = 4512
					//                 nMaxSlot = 2 + (25*1024 + 4512 - 1) / 4512 = 2 + 6.67 = 8
					//
					// e.g. MaxUpload = 12.47 [KB/s]
					//  full upload => upPerClient = 4000 + 12.47*1024/50 = 4255
					//                 nMaxSlot = 2 + (12.47*1024 + 4255 - 1) / 4255 = 2 + 4.00 = 6
					//
					// e.g. MaxUpload = 8.14 [KB/s]
					//  full upload => upPerClient = 4000 + 8.14*1024/50 = 4166
					//                 nMaxSlot = 2 + (8.14*1024 + 4166 - 1) / 4166 = 2 + 3.00 = 5
					//
					// e.g. MaxUpload = 3.99 [KB/s]
					//  full upload => upPerClient = 4000 + 3.99*1024/50 = 4082
					//                 nMaxSlot = 2 + (3.99*1024 + 4082 - 1) / 4082 = 2 + 2.00 = 4
					//
					const uint32 dynMaxSlots = 2 + (availableOut + upPerClient - 1) / upPerClient;
				if(nMaxSlots < dynMaxSlots)
					nMaxSlots = dynMaxSlots;
			}

				if(uploadinglist.GetCount() < nMaxSlots)
				{
					// Check if the upload is unbalanced between the clients
				if (thePrefs.IsCumulateBandwidthEnabled() == false)
					for(POSITION pos = uploadinglist.GetHeadPosition(); pos != NULL; )
					{
						if(uploadinglist.GetNext(pos)->GetUploadDatarate(5) > 2 * upPerClient)
							return true;
					}

					// Check the NAFC condition
					// => Detect network load caused by other applications
					if(thePrefs.GetNAFCEnable() == true)
					{
						if(networkOut > (uint32)(1024.0f * thePrefs.GetMaxUpload()))
						{
							return false;
						}
					}

					// Check datarate
				if (thePrefs.GetNAFCEnable() == true && thePrefs.GetNAFCFullControl() == true)
					{
						if(networkOut < (uint32)(0.9 * 1024.0f * thePrefs.GetMaxUpload()) || // up to 90%
							networkOut + 1000 < (uint32)(1024.0f * thePrefs.GetMaxUpload())) 
							return true;
					}
					else 
					{
						if(eMuleOut < (uint32)(0.9 * 1024.0f * thePrefs.GetMaxUpload()) || // up to 90%
							eMuleOut + 1000 < (uint32)(1024.0f * thePrefs.GetMaxUpload())) 
							return true;
					}
				}

			}
		}

		return false;
}
// Maella end
    
CUploadQueue::~CUploadQueue(){
	// [TPT]
}

CUpDownClient* CUploadQueue::GetWaitingClientByIP_UDP(uint32 dwIP, uint16 nUDPPort){
	for (POSITION pos = waitinglist.GetHeadPosition();pos != 0;){
		CUpDownClient* cur_client = waitinglist.GetNext(pos);
		if (dwIP == cur_client->GetIP() && nUDPPort == cur_client->GetUDPPort())
			return cur_client;
	}
	return 0;
}

CUpDownClient* CUploadQueue::GetWaitingClientByIP(uint32 dwIP){
	for (POSITION pos = waitinglist.GetHeadPosition();pos != 0;){
		CUpDownClient* cur_client = waitinglist.GetNext(pos);
		if (dwIP == cur_client->GetIP())
			return cur_client;
	}
	return 0;
}

/**
 * Add a client to the waiting queue for uploads.
 *
 * @param client address of the client that should be added to the waiting queue
 *
 * @param bIgnoreTimelimit don't check timelimit to possibly ban the client.
 */
void CUploadQueue::AddClientToQueue(CUpDownClient* client, bool bIgnoreTimelimit)
{
	//This is to keep users from abusing the limits we put on lowID callbacks.
	//1)Check if we are connected to any network and that we are a lowID.
	//(Although this check shouldn't matter as they wouldn't have found us..
	// But, maybe I'm missing something, so it's best to check as a precaution.)
	//2)Check if the user is connected to Kad. We do allow all Kad Callbacks.
	//3)Check if the user is in our download list or a friend..
	//We give these users a special pass as they are helping us..
	//4)Are we connected to a server? If we are, is the user on the same server?
	//TCP lowID callbacks are also allowed..
	//5)If the queue is very short, allow anyone in as we want to make sure
	//our upload is always used.
	if (theApp.IsConnected() 
		&& theApp.IsFirewalled()
		&& !client->GetKadPort()
		&& client->GetDownloadState() == DS_NONE 
		&& !client->IsFriend()
		&& theApp.serverconnect
		&& !theApp.serverconnect->IsLocalServer(client->GetServerIP(),client->GetServerPort())
		&& GetWaitingUserCount() > 50)
		return;
	if (client->IsNotSUI())
		theApp.clientlist->AddSnafuClient(client);// [TPT] - eWombat SNAFU v2

	client->AddAskedCount();
	client->SetLastUpRequest();
	if (!bIgnoreTimelimit)
		client->AddRequestCount(client->GetUploadFileID());
	if (client->IsBanned() || client->IsSnafu())
		return;

	// [TPT] - Webcache 1.9 beta3
	// this file is shared but not a single chunk is complete, so don't enqueue the clients asking for it
	CKnownFile* uploadReqfile = theApp.sharedfiles->GetFileByID(client->requpfileid);
	if (uploadReqfile && uploadReqfile->IsPartFile() && ((CPartFile*)uploadReqfile)->GetAvailablePartCount() == 0)
		return;
	// [TPT] - Webcache 1.9 beta3

	uint16 cSameIP = 0;
	// check for double
	// [TPT] Patch. Code Improvement	
	for (POSITION pos = waitinglist.GetHeadPosition(); pos != NULL;){
		POSITION cur_pos = pos;		
		CUpDownClient* cur_client= waitinglist.GetNext(pos);

		if (cur_client == client)
		{	
			if (client->m_bAddNextConnect && AcceptNewClient())
			{
				//Special care is given to lowID clients that missed their upload slot
				//due to the saving bandwidth on callbacks.
                    if(thePrefs.GetLogUlDlEvents())
                        AddDebugLogLine(true, _T("Adding ****lowid when reconneting. Client: %s"), client->DbgGetClientInfo());
					client->m_bAddNextConnect = false;
					RemoveFromWaitingQueue(client, true);
				AddUpNextClient(_T("Adding ****lowid when reconneting."), client);
					return;
				}
			client->SendRankingInfo();
			theApp.emuledlg->transferwnd->queuelistctrl.RefreshClient(client);
			return;			
		}
		else if ( client->Compare(cur_client) ) 
		{
			theApp.clientlist->AddTrackClient(client); // in any case keep track of this client

			// another client with same ip:port or hash
			// this happens only in rare cases, because same userhash / ip:ports are assigned to the right client on connecting in most cases
			if (cur_client->credits != NULL && cur_client->credits->GetCurrentIdentState(cur_client->GetIP()) == IS_IDENTIFIED)
			{
				//cur_client has a valid secure hash, don't remove him
				if (thePrefs.GetVerbose())
					AddDebugLogLine(false,CString(GetResString(IDS_SAMEUSERHASH)),client->GetUserName(),cur_client->GetUserName(),client->GetUserName() );
				return;
			}
			if (client->credits != NULL && client->credits->GetCurrentIdentState(client->GetIP()) == IS_IDENTIFIED)
			{
				//client has a valid secure hash, add him remove other one
				if (thePrefs.GetVerbose())
					AddDebugLogLine(false,CString(GetResString(IDS_SAMEUSERHASH)),client->GetUserName(),cur_client->GetUserName(),cur_client->GetUserName() );
				waitinglist.GetAt(cur_pos)->ClearWaitStartTime(); // [TPT] - SUQWT				
				RemoveFromWaitingQueue(cur_pos,true);	
				if (!cur_client->socket)
				{
					if(cur_client->Disconnected(_T("AddClientToQueue - same userhash 1")))
						delete cur_client;
					}
				}
			else
			{
				// [TPT] - eWombat SNAFU v2	
				if (thePrefs.GetAntiCreditTheft())
				{
					//cur_client has higher Trust, dont remove him
					if (cur_client->GetTrust() > client->GetTrust())
					{
						client->DoSnafu(SNAFU_ACT,false);
						return;
					}
					if (client->GetTrust() > cur_client->GetTrust())
					{
						cur_client->DoSnafu(SNAFU_ACT,false);
						waitinglist.GetAt(cur_pos)->ClearWaitStartTime(); // [TPT] - SUQWT				
						RemoveFromWaitingQueue(cur_pos,true);	
						if (!cur_client->socket){
							if(cur_client->Disconnected(_T("Disconnected due to S.N.A.F.U"), false, CUpDownClient::USR_SNAFU)){ // [TPT] - Maella -Upload Stop Reason-
								delete cur_client;
							}
						}
					}
					else
					{
						// remove both since we dont know who the bad on is
						client->DoSnafu(SNAFU_ACT,false);
						cur_client->DoSnafu(SNAFU_ACT,false);
						waitinglist.GetAt(cur_pos)->ClearWaitStartTime(); // [TPT] - SUQWT				
						RemoveFromWaitingQueue(cur_pos,true);	
						if (!cur_client->socket){
							if(cur_client->Disconnected(_T("Disconnected due to S.N.A.F.U"), false, CUpDownClient::USR_SNAFU)){ // [TPT] - Maella -Upload Stop Reason-
								delete cur_client;
							}
						}
						return;
					}
				}
				else
				{
				// remove both since we do not know who the bad one is
				if (thePrefs.GetVerbose())
					AddDebugLogLine(false,CString(GetResString(IDS_SAMEUSERHASH)),client->GetUserName(),cur_client->GetUserName(),_T("Both") );
				waitinglist.GetAt(cur_pos)->ClearWaitStartTime(); // [TPT] - SUQWT				
				RemoveFromWaitingQueue(cur_pos,true);	
				if (!cur_client->socket)
				{
					if(cur_client->Disconnected(_T("AddClientToQueue - same userhash 2")))
						delete cur_client;
					}
				return;
			}
		}
		}
		// [TPT] - eWombat SNAFU v2
		else if (client->GetIP() == cur_client->GetIP())
		{
			// same IP, different port, different userhash
			cSameIP++;
		}
	}
	if (cSameIP >= 3)
	{
		// do not accept more than 3 clients from the same IP
		if (thePrefs.GetVerbose())
			DEBUG_ONLY( AddDebugLogLine(false,_T("%s's (%s) request to enter the queue was rejected, because of too many clients with the same IP"), client->GetUserName(), ipstr(client->GetConnectIP())) );
		return;
	}
	else if (theApp.clientlist->GetClientsFromIP(client->GetIP()) >= 3)
	{
		if (thePrefs.GetVerbose())
			DEBUG_ONLY( AddDebugLogLine(false,_T("%s's (%s) request to enter the queue was rejected, because of too many clients with the same IP (found in TrackedClientsList)"), client->GetUserName(), ipstr(client->GetConnectIP())) );
		return;
	}
	// done

	// statistic values
	CKnownFile* reqfile = theApp.sharedfiles->GetFileByID((uchar*)client->GetUploadFileID());
	if (reqfile)
		reqfile->statistic.AddRequest();
	
	// cap the list
    // the queue limit in prefs is only a soft limit. Hard limit is 25% higher, to let in powershare clients and other
    // high ranking clients after soft limit has been reached
    uint32 softQueueLimit = thePrefs.GetQueueSize();
    uint32 hardQueueLimit = thePrefs.GetQueueSize() + max(thePrefs.GetQueueSize()/4, 200);

    // if soft queue limit has been reached, only let in high ranking clients
	if (!thePrefs.IsInfiniteQueueEnabled())	// [TPT] - SLUGFILLER: infiniteQueue
	    if ((uint32)waitinglist.GetCount() >= hardQueueLimit ||
        (uint32)waitinglist.GetCount() >= softQueueLimit && // soft queue limit is reached
        (client->IsFriend() && client->GetFriendSlot()) == false && // client is not a friend with friend slot
	        client->GetCombinedFilePrioAndCredit() < GetAverageCombinedFilePrioAndCredit()) { // and client has lower credits/wants lower prio file than average client in queue

        // then block client from getting on queue
		return;
	}
	if (client->IsDownloading())
	{
		// he's already downloading and wants probably only another file
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__AcceptUploadReq", client);
		Packet* packet = new Packet(OP_ACCEPTUPLOADREQ,0);
		theStats.AddUpDataOverheadFileRequest(packet->size);
		client->socket->SendPacket(packet,true);
		return;
	}
		
	client->Credits()->SetSecWaitStartTime(); // [TPT] - SUQWT
			
	if (waitinglist.IsEmpty() && AcceptNewClient())
	{
		AddUpNextClient(_T("Direct add with empty queue."), client);
	}
	else
	{
		waitinglist.AddTail(client);
		client->SetUploadState(US_ONUPLOADQUEUE);
		theApp.emuledlg->transferwnd->queuelistctrl.AddClient(client,true);
		theApp.emuledlg->transferwnd->ShowQueueCount(waitinglist.GetCount());
		client->SendRankingInfo();
	}
}

float CUploadQueue::GetAverageCombinedFilePrioAndCredit() {
    DWORD curTick = ::GetTickCount();

    if (curTick - m_dwLastCalculatedAverageCombinedFilePrioAndCredit > 5*1000) {
        m_dwLastCalculatedAverageCombinedFilePrioAndCredit = curTick;
        
        // TODO: is there a risk of overflow? I don't think so...
        double sum = 0;
	    for (POSITION pos = waitinglist.GetHeadPosition(); pos != NULL; /**/){
		    CUpDownClient* cur_client =	waitinglist.GetNext(pos);
            sum += cur_client->GetCombinedFilePrioAndCredit();
        }
        m_fAverageCombinedFilePrioAndCredit = sum/waitinglist.GetSize();
    }

    return m_fAverageCombinedFilePrioAndCredit;
}

// [TPT] - Maella -Code Improvement-
bool CUploadQueue::RemoveFromUploadQueue(CUpDownClient* client, LPCTSTR pszReason, CUpDownClient::UpStopReason reason, bool updatewindow, bool earlyabort){ // Maella -Upload Stop Reason-	
	bool result = false;
    uint32 slotCounter = 1;
	for (POSITION pos = uploadinglist.GetHeadPosition();pos != NULL;){
        POSITION cur_pos = pos;
        CUpDownClient* curClient = uploadinglist.GetNext(pos);
		if (client == curClient){
			if (updatewindow)
				theApp.emuledlg->transferwnd->uploadlistctrl.RemoveClient(client);

			if (thePrefs.GetLogUlDlEvents())
                AddDebugLogLine(DLP_VERYLOW, true,_T("Removing client from upload list: %s Client: %s Transferred: %s SessionUp: %s QueueSessionPayload: %s"), pszReason==NULL ? _T("") : pszReason, client->DbgGetClientInfo(), CastSecondsToHM( client->GetUpStartTimeDelay()/1000), CastItoXBytes(client->GetSessionUp(), false, false), CastItoXBytes(client->GetQueueSessionPayloadUp(), false, false));
			client->m_bAddNextConnect = false;
			uploadinglist.RemoveAt(cur_pos);
			// [TPT] - Friend Slot
			if (client->GetFriendSlot())
				SetFriendSlotInUploadQueue(NULL);
			// [TPT] - Friend Slot
			bool removed = theApp.uploadBandwidthThrottler->RemoveFromStandardList(client->socket);
			bool pcRemoved = theApp.uploadBandwidthThrottler->RemoveFromStandardList((CClientReqSocket*)client->m_pPCUpSocket);
			// [TPT] - WebCache
			bool wcRemoved = theApp.uploadBandwidthThrottler->RemoveFromStandardList((CClientReqSocket*)client->m_pWCUpSocket);
			// [TPT] - WebCache
			(void)removed;
			(void)pcRemoved;
			(void)wcRemoved;//[TPT] - Webcache
			//if(thePrefs.GetLogUlDlEvents() && !(removed || pcRemoved)) {
			//    AddDebugLogLine(false, _T("UploadQueue: Didn't find socket to delete. Adress: 0x%x"), client->socket);
			//}

			// [TPT] - SUQWT
			if(client->GetSessionUp() > 0 && client->GetSessionUp() < SESSIONMAXTRANS) {
				int keeppct = ((100 * client->GetSessionUp()) >> 23) - 15;
				if (keeppct < 0)    keeppct = 0;
				client->Credits()->SaveUploadQueueWaitTime(keeppct);
				client->Credits()->SetSecWaitStartTime(); // EastShare - Added by TAHO, modified SUQWT
				++successfullupcount;
				totaluploadtime += client->GetUpStartTimeDelay()/1000;
				CUpDownClient::AddUpStopCount(false, reason);
			} else if(client->GetSessionUp() > 0)
			{
				client->Credits()->ClearUploadQueueWaitTime();	// Moonlight: SUQWT
				client->Credits()->ClearWaitStartTime(); // EastShare - Added by TAHO, modified SUQWT
				++successfullupcount;
				totaluploadtime += client->GetUpStartTimeDelay()/1000;
				CUpDownClient::AddUpStopCount(false, reason);
			}
			else if(earlyabort == false) {
				++failedupcount;
				client->Credits()->SaveUploadQueueWaitTime(90);
				CUpDownClient::AddUpStopCount(true, reason);
			}
			// [TPT] - SUQWT
			// Maella end

			CKnownFile* requestedFile = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());

			if(requestedFile != NULL) {
				if(requestedFile->IsPartFile())
					((CPartFile*)requestedFile)->UpdatePartsInfo();
				else
					requestedFile->UpdatePartsInfo();
			}
			theApp.clientlist->AddTrackClient(client); // Keep track of this client
			client->SetUploadState(US_NONE);
			client->ClearUploadBlockRequests();

			m_iHighestNumberOfFullyActivatedSlotsSinceLastCall = 0;

			result = true;
			
		} else {
			curClient->SetSlotNumber(slotCounter);
			slotCounter++;
		}
	}
	return result;	
}

uint32 CUploadQueue::GetAverageUpTime(){
	if( successfullupcount ){
		return totaluploadtime/successfullupcount;
	}
	return 0;
}

bool CUploadQueue::RemoveFromWaitingQueue(CUpDownClient* client, bool updatewindow){
	POSITION pos = waitinglist.Find(client);
	if (pos){
		RemoveFromWaitingQueue(pos,updatewindow);
		if (updatewindow)
			theApp.emuledlg->transferwnd->ShowQueueCount(waitinglist.GetCount());
		client->m_bAddNextConnect = false;
		return true;
	}
	else
		return false;
}

void CUploadQueue::RemoveFromWaitingQueue(POSITION pos, bool updatewindow){	
	CUpDownClient* todelete = waitinglist.GetAt(pos);
	waitinglist.RemoveAt(pos);
	if (updatewindow)
		theApp.emuledlg->transferwnd->queuelistctrl.RemoveClient(todelete);
	// [TPT] - SUQWT
	if (theApp.clientcredits->IsSaveUploadQueueWaitTime())
		todelete->Credits()->SaveUploadQueueWaitTime();	// Moonlight: SUQWT			
	// [TPT] - SUQWT
	todelete->SetUploadState(US_NONE);
}

// [TPT] - Multiqueue
void CUploadQueue::UpdateMaxClientScore()
{
	m_imaxscore=0;
	for(POSITION pos = waitinglist.GetHeadPosition(); pos != 0; ) {
		// [TPT] - Powershare
		CUpDownClient* client = waitinglist.GetNext(pos);
		if (client && client->GetPowerShared())
			continue;
		// [TPT] - Powershare end
		uint32 score = client->GetScore(true, false);
		if(score > m_imaxscore )
			m_imaxscore=score;
	}
}

// [TPT] - Powershare
bool CUploadQueue::CheckForTimeOver(CUpDownClient* client){
	//If we have nobody in the queue, do NOT remove the current uploads..
	//This will save some bandwidth and some unneeded swapping from upload/queue/upload..
	if ( waitinglist.IsEmpty() || client->GetFriendSlot() )
		return false;
	if ((!thePrefs.TransferFullChunks() && !client->GetPowerShared()) || (client->GetWebCacheUpState() == WCUS_UPLOADING)){ // [TPT] - Prevent WebCache 0 uploads with infinite time
	    if( client->GetUpStartTimeDelay() > SESSIONMAXTIME){ // Try to keep the clients from downloading for ever
		    if (thePrefs.GetLogUlDlEvents())
			    AddDebugLogLine(DLP_LOW, false, _T("%s: Upload session ended due to max time %s."), client->GetUserName(), CastSecondsToHM(SESSIONMAXTIME/1000));
		    return true;
	    }
		if (!client->m_dwShieldTime) // MinToMB
			return false;

		// Cache current client score
		const uint32 score = client->GetScore(true, true);

		// Check if another client has a bigger score
		if (score < GetMaxClientScore() && m_dwRemovedClientByScore < GetTickCount()) {
			if (thePrefs.GetLogUlDlEvents())
				AddDebugLogLine(DLP_VERYLOW, false, _T("%s: Upload session ended due to score."), client->GetUserName());
			//Set timer to prevent to many uploadslot getting kick do to score.
			//Upload slots are delayed by a min of 1 sec and the maxscore is reset every 5 sec.
			//So, I choose 6 secs to make sure the maxscore it updated before doing this again.
			m_dwRemovedClientByScore = GetTickCount()+SEC2MS(6);
			return true;
		}
	}
	else{
		// Allow the client to download a specified amount per session
		if( client->GetQueueSessionPayloadUp() > SESSIONMAXTRANS ){
			if (thePrefs.GetLogUlDlEvents())
				AddDebugLogLine(DLP_DEFAULT, false, _T("%s: Upload session ended due to max transferred amount. %s"), client->GetUserName(), CastItoXBytes(SESSIONMAXTRANS, false, false));
			return true;
		}
	}
	return false;
}

void CUploadQueue::DeleteAll(){
	waitinglist.RemoveAll();
	uploadinglist.RemoveAll();
    // PENDING: Remove from UploadBandwidthThrottler as well!
}

// [TPT] - Maella -One-queue-per-file- (idea bloodymad)
uint16 CUploadQueue::GetWaitingPosition(CUpDownClient* client)
{
	if (!IsOnUploadQueue(client))
		return 0;
	UINT rank = 1;
	const uint32 myscore = client->GetScore(false);

	if(thePrefs.GetEnableMultiQueue() == false){
		for(POSITION pos = waitinglist.GetHeadPosition(); pos != NULL; ){
			if(waitinglist.GetNext(pos)->GetScore(false) > myscore)
				rank++;
				}
			}
	else {
		// Compare score only with others clients waiting for the same file
		for(POSITION pos = waitinglist.GetHeadPosition(); pos != NULL; ){
			CUpDownClient* pOtherClient = waitinglist.GetNext(pos);
			if(md4cmp(client->GetUploadFileID(), pOtherClient->GetUploadFileID()) == 0 && 
			   pOtherClient->GetScore(false) > myscore){
				rank++;
			}
		}
	}
	return rank;
}
// Maella end

CUpDownClient* CUploadQueue::GetNextClient(const CUpDownClient* lastclient){
	if (waitinglist.IsEmpty())
		return 0;
	if (!lastclient)
		return waitinglist.GetHead();
	POSITION pos = waitinglist.Find(const_cast<CUpDownClient*>(lastclient));
	if (!pos){
		TRACE("Error: CUploadQueue::GetNextClient");
		return waitinglist.GetHead();
	}
	waitinglist.GetNext(pos);
	if (!pos)
		return NULL;
	else
		return waitinglist.GetAt(pos);
}

/*void CUploadQueue::FindSourcesForFileById(CUpDownClientPtrList* srclist, const uchar* filehash) {
	POSITION pos;
	pos = uploadinglist.GetHeadPosition();
	while(pos) 
	{
		CUpDownClient *potential = uploadinglist.GetNext(pos);
		if(md4cmp(potential->GetUploadFileID(), filehash) == 0)
			srclist->AddTail(potential);
	}

	pos = waitinglist.GetHeadPosition();
	while(pos) 
	{
		CUpDownClient *potential = waitinglist.GetNext(pos);
		if(md4cmp(potential->GetUploadFileID(), filehash) == 0)
			srclist->AddTail(potential);
	}
}
*/

// [TPT] - Resort
bool CUploadQueue::CompareParts(POSITION pos1, POSITION pos2){
	bool ret = false;
	CUpDownClient* client1 = uploadinglist.GetAt(pos1);
	CUpDownClient* client2 = uploadinglist.GetAt(pos2);

	if ((client2->GetPowerShared() && !client1->GetPowerShared()) || // client2 is PS
		(((client1->GetPowerShared() && client2->GetPowerShared()) ||
		  (!client1->GetPowerShared() && !client2->GetPowerShared())) &&
		  (client2->GetUpStartTimeDelay() > client1->GetUpStartTimeDelay()))) // client2 is older in upload queue
		  ret = true;
	
	return ret;
}

void CUploadQueue::SwapParts(POSITION pos1, POSITION pos2){
	CUpDownClient* client1 = uploadinglist.GetAt(pos1);
	CUpDownClient* client2 = uploadinglist.GetAt(pos2);
	uploadinglist.SetAt(pos1, client2);
	uploadinglist.SetAt(pos2, client1);
}

void CUploadQueue::HeapSort(uint16 first, uint16 last){
	uint16 r;
	POSITION pos1 = uploadinglist.FindIndex(first);
	for ( r = first; !(r & 0x8000) && (r<<1) < last; ){
		uint16 r2 = (r<<1)+1;
		POSITION pos2 = uploadinglist.FindIndex(r2);
		if (r2 != last){
			POSITION pos3 = pos2;
			uploadinglist.GetNext(pos3);
			if (!CompareParts(pos2, pos3)){
				pos2 = pos3;
				r2++;
			}
		}
		if (!CompareParts(pos1, pos2)) {
			SwapParts(pos1, pos2);
			r = r2;
			pos1 = pos2;
		}
		else
			break;
	}
}

void CUploadQueue::Sort(){
	uint16 n = uploadinglist.GetCount();
	if (!n)
		return;
	uint16 i;
	for ( i = n/2; i--; )
		HeapSort(i, n-1);
	for ( i = n; --i; ){
		SwapParts(uploadinglist.FindIndex(0), uploadinglist.FindIndex(i));
		HeapSort(0, i-1);
	}
}

void CUploadQueue::ResortUploadClients()
{
	// order by PS and upload time
	Sort();

	// Release client
	m_bIsReleaseSlotUsed = false; 
	CUpDownClient* releaseClient = NULL;
	for(POSITION pos = uploadinglist.GetHeadPosition(); pos != NULL; )
	{			
		POSITION cur_pos = pos;
		CUpDownClient* cur_client = uploadinglist.GetNext(pos);
		if (cur_client)
		{
			CKnownFile* curUpFile = theApp.sharedfiles->GetFileByID(cur_client->GetUploadFileID()); 
			if(curUpFile != NULL && !m_bIsReleaseSlotUsed && curUpFile->GetUpPriority() == PR_VERYHIGH)
			{                
				m_bIsReleaseSlotUsed = true;
				// [TPT] - Move release slot to first pos.
				uploadinglist.RemoveAt(cur_pos);
				releaseClient = cur_client;					
				break; // exit loop for()
			}				
		}
	}		
	if (releaseClient != NULL)
		uploadinglist.AddHead(releaseClient);

	// Reasign slot numbers
	uint32 slotCounter = 1;
	for(POSITION pos = uploadinglist.GetHeadPosition(); pos != NULL; )
	{			
		CUpDownClient* cur_client = uploadinglist.GetNext(pos);
		cur_client->SetSlotNumber(slotCounter);
		slotCounter++;
	}
    // Maella end

	ReSortUploadSlots(false); 
}
// [TPT] - Resort

// [TPT] - Code rework
void CUploadQueue::ReSortUploadSlots(bool force) {
    DWORD curtick = ::GetTickCount();
    if(force ||  curtick - m_dwLastResortedUploadSlots >= SEC2MS(10)) {
        m_dwLastResortedUploadSlots = curtick;

        theApp.uploadBandwidthThrottler->Pause(true);

        // Remove all clients from Throttler
        POSITION ulpos = uploadinglist.GetHeadPosition();
        while (ulpos != NULL) {
			POSITION cur_pos = ulpos;

            // Get and remove the client from upload list.
		    CUpDownClient* cur_client = uploadinglist.GetNext(ulpos);

            // Remove the found Client from UploadBandwidthThrottler
			if (cur_client == NULL)
			{
				AddDebugLogLine(false, _T("Error in ReSortUploadSlots, socket NULL found"));
				uploadinglist.RemoveAt(cur_pos);
			}
			else{
				theApp.uploadBandwidthThrottler->RemoveFromStandardList(cur_client->socket);
				theApp.uploadBandwidthThrottler->RemoveFromStandardList((CClientReqSocket*)cur_client->m_pPCUpSocket);
				theApp.uploadBandwidthThrottler->RemoveFromStandardList((CClientReqSocket*)cur_client->m_pWCUpSocket);
			}
		}

        // Resort Throttler slots
        ulpos = uploadinglist.GetHeadPosition();
		int pos = 0;
        while(ulpos != NULL) {            
            
            // Get the client from upload list.
		    CUpDownClient* cur_client = uploadinglist.GetNext(ulpos);           

            // This will insert in correct place
            theApp.uploadBandwidthThrottler->AddToStandardList(pos, cur_client->GetFileUploadSocket());

			pos++;
        }

        theApp.uploadBandwidthThrottler->Pause(false);
    }
}

// [TPT] - Pawcio: MUS
uint16 CUploadQueue::SlotsInRange(bool lowID)
{
	if(thePrefs.IsDynUpEnabled())
	{
	uint32 MaxUpload;
		MaxUpload = theApp.lastCommonRouteFinder->GetUpload();
	uint32 sendPerClient = UPLOAD_CLIENT_DATARATE + MaxUpload/250;

	if( sendPerClient > 4000 )
		sendPerClient = 4000;
	uint16 retVal;
		if (uploadinglist.GetCount() < (retVal = thePrefs.GetMinUploadSlot())) // [TPT] - Maella -Minimum Upload Slot-
		return retVal;
	if (uploadinglist.GetCount() < (retVal = (uint16)(MaxUpload/sendPerClient) + (uint16)lowID))
		return retVal;
	return 0;
}
	else
	{
		// Compute all datarates elapsed for the last 5 seconds
		uint32 eMuleIn;	uint32 eMuleInOverall;
		uint32 eMuleOut; uint32 eMuleOutOverall;
		uint32 networkIn; uint32 networkOut;
		theApp.pBandWidthControl->GetDatarates(5, // 5 seconds
											eMuleIn, eMuleInOverall,
											eMuleOut, eMuleOutOverall,
											networkIn, networkOut);

		// Calculate the max average upper limit per client
		uint32 upPerClient = UPLOAD_CLIENT_DATARATE + eMuleOut/250;
		if(upPerClient > 4000)			
			upPerClient = 4000;            
		
		uint16 dynMaxSlots;
		if (thePrefs.GetEnableNewUSS() == true)
		{
			const uint32 NAFCnetworkOut = (uint32)(1024.0f * thePrefs.GetMaxUpload());
			const uint32 overHead = (networkOut > eMuleOut) ? (networkOut - eMuleOut) : 0;
			const uint32 availableOut = overHead > NAFCnetworkOut ? 0 : NAFCnetworkOut - overHead;			
			dynMaxSlots = availableOut/ upPerClient;
		}
		else 
		{
			const uint32 availableOut = (uint32)(1024.0f * thePrefs.GetMaxUpload());				
			dynMaxSlots = availableOut/ upPerClient;
		}

		uint16 retVal;
		if (uploadinglist.GetCount() < (retVal = thePrefs.GetMinUploadSlot())) // [TPT] - Maella -Minimum Upload Slot-
			return retVal;		
		if (uploadinglist.GetCount() < (retVal = dynMaxSlots + (uint16)lowID))
			return retVal;
		return 0;
	}
}
// [TPT] - Pawcio: MUS

// [TPT] - MFCK [addon] - New Tooltips [Rayita]
void CUploadQueue::GetTransferTipInfo(CString &info)
{
	// [TPT]
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	uint32 eMuleIn;	uint32 eMuleInOverall;
	uint32 eMuleOut; uint32 eMuleOutOverall;
	uint32 notUsed;
	theApp.pBandWidthControl->GetDatarates(thePrefs.GetDatarateSamples(),
										   eMuleIn, eMuleInOverall,
										   eMuleOut, eMuleOutOverall,
										   notUsed, notUsed);
	info.Format(GetResString(IDS_TT_UL_SP), (float)eMuleOut/1024.0f, (float)(eMuleOutOverall- eMuleOut) / 1024.0f);
	info.AppendFormat(GetResString(IDS_TT_UL_UL), GetUploadQueueLength());
	info.AppendFormat(GetResString(IDS_TT_UL_OQ), GetWaitingUserCount(), thePrefs.GetQueueSize());
	info.AppendFormat(GetResString(IDS_TT_UL_BAN), theApp.clientlist->GetSnafuCount()); // [TPT] - eWombat SNAFU v2
}
// [TPT] - MFCK [addon] - New Tooltips [Rayita]

// [TPT] - WebCache 
CUpDownClient*	CUploadQueue::FindClientByWebCacheUploadId(const uint32 id) // Superlexx - webcache - can be made more efficient
{
	for (POSITION pos = uploadinglist.GetHeadPosition(); pos != NULL;)
	{
		CUpDownClient* cur_client = uploadinglist.GetNext(pos);
		if ( cur_client->m_uWebCacheUploadId == id )
			return cur_client;
	}
	return 0;
}
// [TPT] - WebCache 
