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
#include <zlib/zlib.h>
#include "UpDownClient.h"
#include "PartFile.h"
#include "OtherFunctions.h"
#include "ListenSocket.h"
#include "PeerCacheSocket.h"
#include "Preferences.h"
#include "SafeFile.h"
#include "Packets.h"
#include "Statistics.h"
#include "ClientCredits.h"
#include "DownloadQueue.h"
#include "ClientUDPSocket.h"
#include "emuledlg.h"
#include "TransferWnd.h"
#include "PeerCacheFinder.h"
#include "Exceptions.h"
#include "clientlist.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "Kademlia/Kademlia/Prefs.h"
#include "Kademlia/Kademlia/Search.h"
#include "SHAHashSet.h"
#include "SharedFileList.h"
#include "Log.h"
#include "ipfilter.h" // [TPT]
// [TPT] - WebCache	
#include "WebCacheSocket.h" // yonatan http
#include "SharedFileList.h"	// Superlexx - IFP
// [TPT] - WebCache	

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//	members of CUpDownClient
//	which are mainly used for downloading functions 
//CBarShader CUpDownClient::s_StatusBar(16); // [TPT] - Maella

// [TPT] - Maella -Code Improvement-
void CUpDownClient::DrawStatusBar(CDC* dc, LPCRECT rect, bool onlygreyrect, bool  bFlat) const
{
	const COLORREF crNeither = (bFlat) ? RGB(224, 224, 224) : RGB(240, 240, 240); // Flat => Grey

	ASSERT(reqfile);
	CBarShader statusBar(rect->bottom - rect->top, rect->right - rect->left);
	statusBar.SetFileSize((reqfile != NULL) ? (reqfile->GetFileSize()-1) : 1); 
	statusBar.Fill(crNeither); 

	if (!onlygreyrect && reqfile && (m_abyPartStatus || m_abyIncPartStatus || m_abySeenPartStatus)) { // [TPT] - enkeyDev: ICS, // [TPT] - netfinity: Anti HideOS
		const COLORREF crBoth = (bFlat) ? RGB(0, 150, 0) : RGB(0, 192, 0); // Flat => Green
		const COLORREF crClientOnly = (bFlat) ? RGB(0, 0, 0) : RGB(104, 104, 104); // Flat => Black
		const COLORREF crPending = (bFlat) ? RGB(255,208,0) : RGB(255, 208, 0); // Flat => yellow-orange
		const COLORREF crNextPending = (bFlat) ? RGB(255,255,100) : RGB(255,255,100); // Flat => yellow
		const COLORREF crMeOnly = (bFlat) ? RGB(112,112,112) : RGB(172,172,172); //--- [TPT] - xrmb:seeTheNeed ---
		const COLORREF crClientPartial = (bFlat) ? RGB(170,50,224) : RGB(170,50,224); // [TPT] - enkeyDev: ICS
		const COLORREF crHidden = (bFlat) ? RGB(255,240,240) : RGB(255,240,240); // [TPT] - netfinity: Anti HideOS
	

		char* pcNextPendingBlks = NULL;
		if (m_nDownloadState == DS_DOWNLOADING){
			pcNextPendingBlks = new char[m_nPartCount];
			MEMSET(pcNextPendingBlks, 'N', m_nPartCount); // do not use '_strnset' for uninitialized memory!
			for (POSITION pos = m_PendingBlocks_list.GetHeadPosition(); pos != 0; ){
				UINT uPart = m_PendingBlocks_list.GetNext(pos)->block->StartOffset / PARTSIZE;
				if (uPart < m_nPartCount)
					pcNextPendingBlks[uPart] = 'Y';
			}
		}

		for (uint32 i = 0;i < m_nPartCount;i++){
			if (m_abyPartStatus && m_abyPartStatus[i]){ // [TPT] - enkeyDev: ICS
				uint32 uEnd;
				if (PARTSIZE*(i+1) > reqfile->GetFileSize())
					uEnd = reqfile->GetFileSize();
				else
					uEnd = PARTSIZE*(i+1);

				if (reqfile->IsComplete(PARTSIZE*i,PARTSIZE*(i+1)-1))
					statusBar.FillRange(PARTSIZE*i, uEnd, crBoth);
				else if (m_nDownloadState == DS_DOWNLOADING && m_nLastBlockOffset >= PARTSIZE*i && m_nLastBlockOffset < uEnd)
					statusBar.FillRange(PARTSIZE*i, uEnd, crPending);
				else if (pcNextPendingBlks != NULL && pcNextPendingBlks[i] == 'Y')
					statusBar.FillRange(PARTSIZE*i, uEnd, crNextPending);
				else
					statusBar.FillRange(PARTSIZE*i, uEnd, crClientOnly);
			}
			// [TPT] - enkeyDev: ICS
			else if (m_abyIncPartStatus && m_abyIncPartStatus[i]){
				uint32 uEnd;
				if ((uint32)PARTSIZE*(i+1) > reqfile->GetFileSize()) 
					uEnd = reqfile->GetFileSize(); 
				else 
					uEnd = PARTSIZE*(i+1); 
				statusBar.FillRange(PARTSIZE*i, uEnd, crClientPartial);
			}
			// [TPT] - enkeyDev: ICS
			//--- [TPT] - xrmb:seeTheNeed ---
			else if (reqfile->IsComplete(PARTSIZE*i,PARTSIZE*(i+1)-1)){ 
				uint32 uEnd; 
				if ((uint32)(PARTSIZE*(i+1)) > reqfile->GetFileSize()) 
					uEnd = reqfile->GetFileSize(); 
				else 
					uEnd = PARTSIZE*(i+1); 

				statusBar.FillRange(PARTSIZE*i, uEnd, crMeOnly);
			} 
			//--- [TPT] - xrmb:seeTheNeed ---
			// [TPT] - netfinity: Anti HideOS
			else if (m_abySeenPartStatus && m_abySeenPartStatus[i]){
				uint32 uEnd;
				if ((uint32)PARTSIZE*(i+1) > reqfile->GetFileSize())
					uEnd = reqfile->GetFileSize(); 
				else 
					uEnd = PARTSIZE*(i+1); 

				statusBar.FillRange(PARTSIZE*i, uEnd, crHidden);				
			}
			// [TPT] - netfinity: Anti HideOS
		}
		delete[] pcNextPendingBlks;
	}

	// Finally draw the graphical object
	statusBar.Draw(dc, rect->left, rect->top, bFlat); 
} 
// Maella end

bool CUpDownClient::Compare(const CUpDownClient* tocomp, bool bIgnoreUserhash) const
{
	//Compare only the user hash..
	if(!bIgnoreUserhash && HasValidHash() && tocomp->HasValidHash())
	    return !md4cmp(this->GetUserHash(), tocomp->GetUserHash());

	if (HasLowID())
	{
		//User is firewalled.. Must do two checks..
		if (GetIP()!=0	&& GetIP() == tocomp->GetIP())
		{
			//The IP of both match
            if (GetUserPort()!=0 && GetUserPort() == tocomp->GetUserPort())
				//IP-UserPort matches
                return true;
			if (GetKadPort()!=0	&& GetKadPort() == tocomp->GetKadPort())
				//IP-KadPort Matches
				return true;
		}
        if (GetUserIDHybrid()!=0
			&& GetUserIDHybrid() == tocomp->GetUserIDHybrid()
			&& GetServerIP()!=0
			&& GetServerIP() == tocomp->GetServerIP()
			&& GetServerPort()!=0
			&& GetServerPort() == tocomp->GetServerPort())
			//Both have the same lowID, Same serverIP and Port..
            return true;

		#if defined(_DEBUG)
		if ( HasValidBuddyID() && tocomp->HasValidBuddyID() )
		{
			//JOHNTODO: This is for future use to see if this will be needed...
			if(!md4cmp(GetBuddyID(), tocomp->GetBuddyID()))
            return true;
		}
		#endif

		//Both IP, and Server do not match..
        return false;
    }

	//User is not firewalled.
    if (GetUserPort()!=0)
	{
		//User has a Port, lets check the rest.
		if (GetIP() != 0 && tocomp->GetIP() != 0)
		{
			//Both clients have a verified IP..
			if(GetIP() == tocomp->GetIP() && GetUserPort() == tocomp->GetUserPort())
				//IP and UserPort match..
            return true;
		}
		else
		{
			//One of the two clients do not have a verified IP
			if (GetUserIDHybrid() == tocomp->GetUserIDHybrid() && GetUserPort() == tocomp->GetUserPort())
				//ID and Port Match..
            return true;
    }
    }
	if(GetKadPort()!=0)
	{
		//User has a Kad Port.
		if(GetIP() != 0 && tocomp->GetIP() != 0)
		{
			//Both clients have a verified IP.
			if(GetIP() == tocomp->GetIP() && GetKadPort() == tocomp->GetKadPort())
				//IP and KadPort Match..
			return true;
		}
		else
		{
			//One of the users do not have a verified IP.
            if (GetUserIDHybrid() == tocomp->GetUserIDHybrid() && GetKadPort() == tocomp->GetKadPort())
				//ID and KadProt Match..
			return true;
	}
	}
	//No Matches..
	return false;
}

// Return bool is not if you asked or not..
// false = Client was deleted!
// true = client was not deleted!
bool CUpDownClient::AskForDownload()
{
	if (theApp.listensocket->TooManySockets() && !(socket && socket->IsConnected()) )
	{
		if (GetDownloadState() != DS_TOOMANYCONNS)
			SetDownloadState(DS_TOOMANYCONNS);
		return true;
	}

	if (m_bUDPPending)
	{
		m_nFailedUDPPackets++;
		theApp.downloadqueue->AddFailedUDPFileReasks();
	}
	m_bUDPPending = false;	
    	SwapToAnotherFile(_T("A4AF check before tcp file reask. CUpDownClient::AskForDownload()"), true, false, false, NULL, true, true); // ZZ:DownloadManager    	
	SetDownloadState(DS_CONNECTING);
	return TryToConnect();
}

bool CUpDownClient::IsSourceRequestAllowed() const
{
    return IsSourceRequestAllowed(reqfile); // ZZ:DownloadManager
} // ZZ:DownloadManager

bool CUpDownClient::IsSourceRequestAllowed(CPartFile* partfile, bool sourceExchangeCheck) const // ZZ:DownloadManager
{ // ZZ:DownloadManager
	DWORD dwTickCount = ::GetTickCount() + CONNECTION_LATENCY;
	unsigned int nTimePassedClient = dwTickCount - GetLastSrcAnswerTime();
	unsigned int nTimePassedFile   = dwTickCount - partfile->GetLastAnsweredTime(); // ZZ:DownloadManager
	bool bNeverAskedBefore = GetLastAskedForSources() == 0;

// ZZ:DownloadManager -->
	UINT uSources = partfile->GetSourceCount();
    UINT uValidSources = partfile->GetValidSourcesCount();

    if(partfile != reqfile) {
        uSources++;
        uValidSources++;
    }

    UINT uReqValidSources = reqfile->GetValidSourcesCount();
// <-- ZZ:DownloadManager

	return (
	         //if client has the correct extended protocol
	         ExtProtocolAvailable() && GetSourceExchangeVersion() > 1 &&
	         //AND if we need more sources
			 // [TPT] - Sivka AutoHL
	         reqfile->GetMaxSourcesLimitSoft() > uSources && 
	         //AND if...
	         (
	           //source is not complete and file is very rare
	           ( !m_bCompleteSource
				 && (bNeverAskedBefore || nTimePassedClient > SOURCECLIENTREASKS)
			     && (uSources <= RARE_FILE/5)
				 && (!sourceExchangeCheck || partfile == reqfile || uValidSources < uReqValidSources && uReqValidSources > 3) // ZZ:DownloadManager
	           ) ||
	           //source is not complete and file is rare
	           ( !m_bCompleteSource
				 && (bNeverAskedBefore || nTimePassedClient > SOURCECLIENTREASKS)
			     && (uSources <= RARE_FILE || (!sourceExchangeCheck || partfile == reqfile) && uSources <= RARE_FILE / 2 + uValidSources) // ZZ:DownloadManager
				 && (nTimePassedFile > SOURCECLIENTREASKF)
				 && (!sourceExchangeCheck || partfile == reqfile || uValidSources < SOURCECLIENTREASKS/SOURCECLIENTREASKF && uValidSources < uReqValidSources) // ZZ:DownloadManager
	           ) ||
	           // OR if file is not rare
			   ( (bNeverAskedBefore || nTimePassedClient > (unsigned)(SOURCECLIENTREASKS * MINCOMMONPENALTY)) 
				 && (nTimePassedFile > (unsigned)(SOURCECLIENTREASKF * MINCOMMONPENALTY))
				 && (!sourceExchangeCheck || partfile == reqfile || uValidSources < SOURCECLIENTREASKS/SOURCECLIENTREASKF && uValidSources < uReqValidSources) // ZZ:DownloadManager
	           )
	         )
	       );
}

void CUpDownClient::SendFileRequest()
{
    // normally asktime has already been reset here, but then SwapToAnotherFile will return without much work, so check to make sure
    SwapToAnotherFile(_T("A4AF check before tcp file reask. CUpDownClient::SendFileRequest()"), true, false, false, NULL, true, true); // ZZ:DownloadManager

	ASSERT(reqfile != NULL);
	if(!reqfile)
		return;
	AddAskedCountDown();

	CSafeMemFile dataFileReq(16+16+64);	// [TPT] - WebCache	// Superlexx - webcache - 64 extra bytes for webcache info data should be enough
	dataFileReq.WriteHash16(reqfile->GetFileHash());

	if( SupportMultiPacket() )
	{
		dataFileReq.WriteUInt8(OP_REQUESTFILENAME);
		//Extended information
		if( GetExtendedRequestsVersion() > 0 )
			reqfile->WritePartStatus(&dataFileReq);
		if( GetExtendedRequestsVersion() > 1 )
			reqfile->WriteCompleteSourcesCount(&dataFileReq);
		if (reqfile->GetPartCount() > 1
			|| SupportsWebCache()) // [TPT] - WebCache	// Superlexx - webcache
			dataFileReq.WriteUInt8(OP_SETREQFILEID);
		if( IsEmuleClient() )
		{
			SetRemoteQueueFull( true );
			SetRemoteQueueRank(0);
		}	
		if(IsSourceRequestAllowed())
		{
			dataFileReq.WriteUInt8(OP_REQUESTSOURCES);
			reqfile->SetLastAnsweredTimeTimeout();
			SetLastAskedForSources();
			if (thePrefs.GetDebugSourceExchange())
				AddDebugLogLine(false, _T("SXSend: Client source request; %s, File=\"%s\""), DbgGetClientInfo(), reqfile->GetFileName());
        }
		if (IsSupportingAICH()){
			if (thePrefs.GetDebugClientTCPLevel() > 0)
				DebugSend("OP__MPAichFileHashReq", this, (char*)reqfile->GetFileHash());
			dataFileReq.WriteUInt8(OP_AICHFILEHASHREQ);
		}
		
		// [TPT] - WebCache	
		// Superlexx - webcache - the webcache-only tags, moved here from the hello packet
		if (SupportsWebCache() && WebCacheInfoNeeded())
		{
			dataFileReq.WriteUInt8(WC_TAG_WEBCACHENAME);
			dataFileReq.WriteString(thePrefs.webcacheName);

			dataFileReq.WriteUInt8(WC_TAG_MASTERKEY);
			dataFileReq.Write( Crypt.remoteMasterKey, WC_KEYLENGTH );
			SetWebCacheInfoNeeded(false);

			byte tmpID[4];
			for (int i=0; i<4; i++)
				tmpID[i] = Crypt.remoteMasterKey[i] ^ (thePrefs.GetUserHash())[i];
			m_uWebCacheUploadId =*((uint32*)tmpID);
		}
		// Superlexx end
		// [TPT] - WebCache	

		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__MultiPacket", this, (char*)reqfile->GetFileHash());
		Packet* packet = new Packet(&dataFileReq, OP_EMULEPROT);
		packet->opcode = OP_MULTIPACKET;
		theStats.AddUpDataOverheadFileRequest(packet->size);
		socket->SendPacket(packet, true);
	}
	else
	{
		//This is extended information
		if( GetExtendedRequestsVersion() > 0 ){
			reqfile->WritePartStatus(&dataFileReq);
		}
		if( GetExtendedRequestsVersion() > 1 ){
			reqfile->WriteCompleteSourcesCount(&dataFileReq);
		}
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__FileRequest", this, (char*)reqfile->GetFileHash());
		Packet* packet = new Packet(&dataFileReq);
		packet->opcode=OP_REQUESTFILENAME;
		theStats.AddUpDataOverheadFileRequest(packet->size);
		socket->SendPacket(packet, true);
	
		// 26-Jul-2003: removed requesting the file status for files <= PARTSIZE for better compatibility with ed2k protocol (eDonkeyHybrid).
		// if the remote client answers the OP_REQUESTFILENAME with OP_REQFILENAMEANSWER the file is shared by the remote client. if we
		// know that the file is shared, we know also that the file is complete and don't need to request the file status.
		if (reqfile->GetPartCount() > 1
			|| SupportsWebCache())	// [TPT] - WebCache	// Superlexx - webcache - webcache-enabled clients might use IFP
		{
			if (thePrefs.GetDebugClientTCPLevel() > 0)
				DebugSend("OP__SetReqFileID", this, (char*)reqfile->GetFileHash());
		    CSafeMemFile dataSetReqFileID(16);
			dataSetReqFileID.WriteHash16(reqfile->GetFileHash());
		    packet = new Packet(&dataSetReqFileID);
		    packet->opcode = OP_SETREQFILEID;
		    theStats.AddUpDataOverheadFileRequest(packet->size);
		    socket->SendPacket(packet, true);
		}
		
		if( IsEmuleClient() )
		{
			SetRemoteQueueFull( true );
			SetRemoteQueueRank(0);
		}	

		if(thePrefs.GetDisabledXS() == false && IsSourceRequestAllowed()) // [TPT] - Maella -Enable/Disable source exchange in preference- (Tarod)	   
		{
		    if (thePrefs.GetDebugClientTCPLevel() > 0){
			    DebugSend("OP__RequestSources", this, (char*)reqfile->GetFileHash());
			    if (GetLastAskedForSources() == 0)
				    Debug(_T("  first source request\n"));
			    else
				    Debug(_T("  last source request was before %s\n"), CastSecondsToHM((GetTickCount() - GetLastAskedForSources())/1000));
		    }
			reqfile->SetLastAnsweredTimeTimeout();
			Packet* packet = new Packet(OP_REQUESTSOURCES,16,OP_EMULEPROT);
			md4cpy(packet->pBuffer,reqfile->GetFileHash());
			theStats.AddUpDataOverheadSourceExchange(packet->size);
			socket->SendPacket(packet,true,true);
			SetLastAskedForSources();
			if (thePrefs.GetDebugSourceExchange())
				AddDebugLogLine(false, _T("SXSend: Client source request; %s, File=\"%s\""), DbgGetClientInfo(), reqfile->GetFileName());
        }
		if (IsSupportingAICH()){
			Packet* packet = new Packet(OP_AICHFILEHASHREQ,16,OP_EMULEPROT);
			md4cpy(packet->pBuffer,reqfile->GetFileHash());
			socket->SendPacket(packet,true,true);
        }
	}
	SetLastAskedTime(); // ZZ:DownloadManager
	// [TPT] - Maella -Unnecessary Protocol Overload-
	// Delay the next refresh of the download session initiated from CPartFile::Process() 
	m_dwNextTCPAskedTime = ::GetTickCount() + GetJitteredFileReaskTime();
	// Maella end
    
}

void CUpDownClient::SendStartupLoadReq()
{
	if (socket==NULL || reqfile==NULL)
	{
		ASSERT(0);
		return;
	}
	SetDownloadState(DS_ONQUEUE);
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__StartupLoadReq", this);
	CSafeMemFile dataStartupLoadReq(16);
	dataStartupLoadReq.WriteHash16(reqfile->GetFileHash());
	Packet* packet = new Packet(&dataStartupLoadReq);
	packet->opcode = OP_STARTUPLOADREQ;
	theStats.AddUpDataOverheadFileRequest(packet->size);
	socket->SendPacket(packet, true, true);
	m_fQueueRankPending = 1;
	m_fUnaskQueueRankRecv = 0;
	
	// [TPT] - Maella -Unnecessary Protocol Overload-
	// Remark: force a TCP refresh of the download session in 2 hours
	//Xman: we increase this time if udpver >3, because we have a valid partstatus
	if(GetUDPVersion()>3)
		m_dwNextTCPAskedTime = GetLastAskedTime() + 6 * GetJitteredFileReaskTime();
	else
		m_dwNextTCPAskedTime = GetLastAskedTime() + 4 * GetJitteredFileReaskTime();
	// Maella end
	
	// [TPT]
	// Keep a track when this file was asked for the last time to avoid a Ban()
	m_partStatusMap[reqfile].dwStartUploadReqTime = ::GetTickCount();
	// Maella end
}

void CUpDownClient::ProcessFileInfo(CSafeMemFile* data, CPartFile* file)
{
	if (file==NULL)
		throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (ProcessFileInfo; file==NULL)");
	if (reqfile==NULL)
		throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (ProcessFileInfo; reqfile==NULL)");
	if (file != reqfile)
		throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (ProcessFileInfo; reqfile!=file)");
	m_strClientFilename = data->ReadString(GetUnicodeSupport());
	// 26-Jul-2003: removed requesting the file status for files <= PARTSIZE for better compatibility with ed2k protocol (eDonkeyHybrid).
	// if the remote client answers the OP_REQUESTFILENAME with OP_REQFILENAMEANSWER the file is shared by the remote client. if we
	// know that the file is shared, we know also that the file is complete and don't need to request the file status.
	if (reqfile->GetPartCount() == 1
		&& !SupportsWebCache()) // [TPT] - WebCache	// Superlexx - webcache - webcache-enabled clients might use IFP
	{
		if (m_abyPartStatus)
		{
			delete[] m_abyPartStatus;
			m_abyPartStatus = NULL;
		}
		// [TPT] - enkeyDev: ICS
		if (m_abyIncPartStatus){
			delete[] m_abyIncPartStatus;
			m_abyIncPartStatus = NULL;
		}
		// [TPT] - enkeyDev: ICS

		// [TPT] - netfinity: Anti HideOS
		if (m_abySeenPartStatus){
			delete[] m_abySeenPartStatus;
			m_abySeenPartStatus = NULL;
		}
		// [TPT] - netfinity: Anti HideOS

		m_nPartCount = reqfile->GetPartCount();
		m_abyPartStatus = new uint8[m_nPartCount];
		MEMSET(m_abyPartStatus,1,m_nPartCount);
		// [TPT] - enkeyDev: ICS
		m_abyIncPartStatus = new uint8[m_nPartCount];
		MEMSET(m_abyIncPartStatus,1,m_nPartCount);
		// [TPT] - enkeyDev: ICS

		// [TPT] - netfinity: Anti HideOS
		m_abySeenPartStatus = new uint8[m_nPartCount];
		MEMSET(m_abySeenPartStatus,1,m_nPartCount);
		// [TPT] - netfinity: Anti HideOS

		m_bCompleteSource = true;

		if (thePrefs.GetDebugClientTCPLevel() > 0)
		{
		    int iNeeded = 0;
		    for (int i = 0; i < m_nPartCount; i++)
			    if (!reqfile->IsComplete(i*PARTSIZE,((i+1)*PARTSIZE)-1))
				    iNeeded++;
			char* psz = new char[m_nPartCount + 1];
			for (int i = 0; i < m_nPartCount; i++)
				psz[i] = m_abyPartStatus[i] ? '#' : '.';
			psz[i] = '\0';
			Debug(_T("  Parts=%u  %hs  Needed=%u\n"), m_nPartCount, psz, iNeeded);
			delete[] psz;
		}
		UpdateDisplayedInfo();
		reqfile->UpdateAvailablePartsCount();
		// even if the file is <= PARTSIZE, we _may_ need the hashset for that file (if the file size == PARTSIZE)
		if (reqfile->hashsetneeded)
		{
			RequestHashset();
		}
		else
		{
			SendStartupLoadReq();
		}
		reqfile->UpdatePartsInfo();
		reqfile->NewSrcIncPartsInfo(); // [TPT] - enkeyDev: ICS
	}
	// [TPT] - SLUGFILLER: showComments - send comment while you're at it.
	if(m_byAcceptCommentVer >= 2)
		SendCommentInfo(reqfile);
	// [TPT] - SLUGFILLER: showComments
}

void CUpDownClient::RequestHashset(){
	if (socket)
	{
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__HashSetRequest", this, (char*)reqfile->GetFileHash());
		Packet* packet = new Packet(OP_HASHSETREQUEST,16);
		md4cpy(packet->pBuffer,reqfile->GetFileHash());
		theStats.AddUpDataOverheadFileRequest(packet->size);
		socket->SendPacket(packet, true, true);
		SetDownloadState(DS_REQHASHSET);
		m_fHashsetRequesting = 1;
		reqfile->hashsetneeded = false;
	}
	else
		ASSERT(0);
}

void CUpDownClient::ProcessFileStatus(bool bUdpPacket, CSafeMemFile* data, CPartFile* file)
{
	if ( !reqfile || file != reqfile )
	{
		if (reqfile==NULL)
			throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (ProcessFileStatus; reqfile==NULL)");
		throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (ProcessFileStatus; reqfile!=file)");
	}
	uint16 nED2KPartCount = data->ReadUInt16();
	if (m_abyPartStatus)
	{
		delete[] m_abyPartStatus;
		m_abyPartStatus = NULL;
	}
	bool bPartsNeeded = false;
	int iNeeded = 0;
	if (!nED2KPartCount)
	{
		m_nPartCount = reqfile->GetPartCount();
		m_abyPartStatus = new uint8[m_nPartCount];
		MEMSET(m_abyPartStatus,1,m_nPartCount);
		bPartsNeeded = true;
		m_bCompleteSource = true;
		if (bUdpPacket ? (thePrefs.GetDebugClientUDPLevel() > 0) : (thePrefs.GetDebugClientTCPLevel() > 0))
		{
			for (int i = 0; i < m_nPartCount; i++)
			{
				if (!reqfile->IsComplete(i*PARTSIZE,((i+1)*PARTSIZE)-1))
					iNeeded++;
			}
		}
	}
	else
	{
		if (reqfile->GetED2KPartCount() != nED2KPartCount)
		{
			CString strError;
			strError.Format(_T("ProcessFileStatus - wrong part number recv=%u  expected=%u  %s"), nED2KPartCount, reqfile->GetED2KPartCount(), DbgGetFileInfo(reqfile->GetFileHash()));
			m_nPartCount = 0;
			throw strError;
		}
		m_nPartCount = reqfile->GetPartCount();

		m_bCompleteSource = false;
		m_abyPartStatus = new uint8[m_nPartCount];
		uint16 done = 0;
		// [TPT] - netfinity: Anti HideOS
		bool checkSeenParts = false;
		if (IsPartialSource() && m_abySeenPartStatus)
			checkSeenParts = true;
		// [TPT] - netfinity: Anti HideOS
		while (done != m_nPartCount)
		{
			uint8 toread = data->ReadUInt8();
			for (sint32 i = 0;i != 8;i++)
			{
				m_abyPartStatus[done] = ((toread>>i)&1)? 1:0; 	
				// [TPT] - netfinity: Anti HideOS
				if (m_abyPartStatus[done] || (checkSeenParts && m_abySeenPartStatus[done]))
				{
					if (!reqfile->IsComplete(done*PARTSIZE,((done+1)*PARTSIZE)-1)){
						bPartsNeeded = true;
						iNeeded++;
					}
				}
				done++;
				if (done == m_nPartCount)
					break;
			}
		}
	}
	
	if (bUdpPacket ? (thePrefs.GetDebugClientUDPLevel() > 0) : (thePrefs.GetDebugClientTCPLevel() > 0))
	{
		TCHAR* psz = new TCHAR[m_nPartCount + 1];
		for (int i = 0; i < m_nPartCount; i++)
			psz[i] = m_abyPartStatus[i] ? _T('#') : _T('.');
		psz[i] = _T('\0');
		Debug(_T("  Parts=%u  %s  Needed=%u\n"), m_nPartCount, psz, iNeeded);
		delete[] psz;
	}

	UpdateDisplayedInfo();
	reqfile->UpdateAvailablePartsCount();

	// NOTE: This function is invoked from TCP and UDP socket!
	if (!bUdpPacket)
	{
		if (!bPartsNeeded)
			SetDownloadState(DS_NONEEDEDPARTS, _T("No Needed Parts"), CUpDownClient::DSR_NONEEDEDPARTS); // [TPT] - Maella -Download Stop Reason-
		//If we are using the eMule filerequest packets, this is taken care of in the Multipacket!
		else if (reqfile->hashsetneeded)
		{
			RequestHashset();
		}
		else
		{
			SendStartupLoadReq();
		}
	}
	else
	{
		if (!bPartsNeeded)
			SetDownloadState(DS_NONEEDEDPARTS, _T("No needed Parts"),  CUpDownClient::DSR_NONEEDEDPARTS); // [TPT] - Maella -Download Stop Reason-
		else
			SetDownloadState(DS_ONQUEUE);
	}
	reqfile->UpdatePartsInfo();
}

// [TPT] - enkeyDev: ICS
void CUpDownClient::ProcessFileIncStatus(CSafeMemFile* data,uint32 size, bool readHash){
	if (readHash){
		uchar cfilehash[16];
		data->ReadHash16(cfilehash);
		if ((!reqfile) || md4cmp(cfilehash,reqfile->GetFileHash())){
			throw CString(GetResString(IDS_ERR_WRONGFILEID)+ _T(" (ProcessFileIncStatus)"));	
		}
	}
	m_nPartCount = data->ReadUInt16();
	if (m_abyIncPartStatus) {
		delete[] m_abyIncPartStatus;
		m_abyIncPartStatus = NULL;
	}
	if (!m_nPartCount){
		m_nPartCount = reqfile->GetPartCount();
		m_abyIncPartStatus = new uint8[m_nPartCount];
		MEMSET(m_abyIncPartStatus,1,m_nPartCount);
	}
	else{
		if (reqfile->GetPartCount() != m_nPartCount){
			throw GetResString(IDS_ERR_WRONGPARTNUMBER);
		}
		m_abyIncPartStatus = new uint8[m_nPartCount];
		uint16 done = 0;
		while (done != m_nPartCount){
			uint8 toread = data->ReadUInt8();
			for (sint32 i = 0;i != 8;i++){
				m_abyIncPartStatus[done] = ((toread>>i)&1)? 1:0; 	
				done++;
				if (done == m_nPartCount)
					break;
			}
		}
	}

	reqfile->NewSrcIncPartsInfo();
}
// [TPT] - enkeyDev: ICS

// [TPT] - Maella -Code Improvement-
bool CUpDownClient::AddRequestForAnotherFile(CPartFile* file){
	if(m_OtherRequests_list.Find(file) != NULL) return false; // Found
	if(m_OtherNoNeeded_list.Find(file) != NULL) return false; // Found
	m_OtherRequests_list.AddTail(file);
	file->A4AFsrclist.AddTail(this); // [enkeyDEV(Ottavio84) -A4AF-]

	return true;
}
// Maella end

void CUpDownClient::ClearDownloadBlockRequests()
{
	for (POSITION pos = m_DownloadBlocks_list.GetHeadPosition();pos != 0;){
		Requested_Block_Struct* cur_block = m_DownloadBlocks_list.GetNext(pos);
		if (reqfile){
			reqfile->RemoveBlockFromList(cur_block->StartOffset,cur_block->EndOffset);
		}
		delete cur_block;
	}
	m_DownloadBlocks_list.RemoveAll();

	for (POSITION pos = m_PendingBlocks_list.GetHeadPosition();pos != 0;){
		Pending_Block_Struct *pending = m_PendingBlocks_list.GetNext(pos);
		if (reqfile){
			reqfile->RemoveBlockFromList(pending->block->StartOffset, pending->block->EndOffset);
		}

		delete pending->block;
		// Not always allocated
		if (pending->zStream){
			inflateEnd(pending->zStream);
			delete pending->zStream;
		}
		delete pending;
	}
	m_PendingBlocks_list.RemoveAll();
}

void CUpDownClient::SetDownloadState(EDownloadState nNewState, LPCTSTR pszReason, DownStopReason reason){ // [TPT] - Maella -Download Stop Reason-
	if (m_nDownloadState != nNewState){
		switch( nNewState )
		{
			case DS_CONNECTING:
	            m_dwLastTriedToConnect = ::GetTickCount();
				// [TPT] - Maella -Unnecessary Protocol Overload-
				// Delay the next refresh of the download session initiated from CPartFile::Process()
				m_dwNextTCPAskedTime = ::GetTickCount() + GetJitteredFileReaskTime();
				// Maella end
				break;
			case DS_TOOMANYCONNSKAD:
				//This client had already been set to DS_CONNECTING.
				//So we reset this time so it isn't stuck at TOOMANYCONNS for 20mins.
				m_dwLastTriedToConnect = ::GetTickCount()-MIN2MS(20);
				break;
			case DS_WAITCALLBACKKAD:
			case DS_WAITCALLBACK:
				break;
			default:
				switch( m_nDownloadState )
				{
					case DS_WAITCALLBACK:
					case DS_WAITCALLBACKKAD:
						break;
					default:
						m_dwLastTriedToConnect = ::GetTickCount()-MIN2MS(20);
						break;
				}
				break;
		}

		if (reqfile){
		    if(nNewState == DS_DOWNLOADING){
			    reqfile->AddDownloadingSource(this);
				theApp.emuledlg->transferwnd->downloadclientsctrl.AddClient(this); // [TPT] - TBH Transfers Window Style
		    }
		    else if(m_nDownloadState == DS_DOWNLOADING){
			    reqfile->RemoveDownloadingSource(this);
		    }
		}

        if(nNewState == DS_DOWNLOADING){
			if(socket)
				socket->SetTimeOut(CONNECTION_TIMEOUT*4);
        }

		if (m_nDownloadState == DS_DOWNLOADING ){
			if(socket)
				socket->SetTimeOut(CONNECTION_TIMEOUT);

			if (thePrefs.GetLogUlDlEvents()) {
				switch( nNewState )
				{
					case DS_NONEEDEDPARTS:
						pszReason = _T("NNP. You don't need any parts from this client.");
				}

				AddDebugLogLine(DLP_VERYLOW, false, _T("Download session ended. User: %s in SetDownloadState(). New State: %i, Length: %s, Transferred: %s. Reason: %s"), DbgGetClientInfo(), nNewState, CastSecondsToHM(GetDownTimeDifference(false)/1000), CastItoXBytes(GetSessionDown(), false, false), pszReason);
			}

			ResetSessionDown();
			theApp.emuledlg->transferwnd->downloadclientsctrl.RemoveClient(this); // [TPT] - TBH Transfers Window Style
			// -khaos--+++> Extended Statistics (Successful/Failed Download Sessions)
			if ( m_bTransferredDownMini && nNewState != DS_ERROR )
			{
				CUpDownClient::AddDownStopCount(false, reason); // [TPT] - Maella -Download Stop Reason-
				thePrefs.Add2DownSuccessfulSessions(); // Increment our counters for successful sessions (Cumulative AND Session)
			}
			else {
				CUpDownClient::AddDownStopCount(true, reason); // [TPT] - Maella -Download Stop Reason-
				thePrefs.Add2DownFailedSessions(); // Increment our counters failed sessions (Cumulative AND Session)
			}
			thePrefs.Add2DownSAvgTime(GetDownTimeDifference()/1000);
			// <-----khaos-

			m_nDownloadState = nNewState;

			ClearDownloadBlockRequests();
				
			// [TPT] - Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
			m_nDownDatarate = 0;
			//m_AvarageDDR_list.RemoveAll();
			//m_nSumForAvgDownDataRate = 0;
			m_nDownDatarateMeasure = 0;
			m_sumDownHistory = 0;
			m_downHistory_list.RemoveAll();
			// Maella end

			// Maella -Code Improvement-
			if(nNewState == DS_ONQUEUE && reqfile != NULL){
				// The downloading session was terminated by a OP_OUTOFPARTREQS (e.g. Hybrid, shareaza)
				// or a Disconnect() (e.g. eMule). In this second case it might be necessary to reask if
				// the disconnection was caused by a timeout, otherwise we could be thrown out of the 
				// remote waiting list.				
					// Reask for the file as soon as possible.
					TrigNextSafeAskForDownload(reqfile);
			}
			// Maella end

			if (nNewState == DS_NONE){
				if (m_abyPartStatus)
					delete[] m_abyPartStatus;
				m_abyPartStatus = NULL;
				// [TPT] - netfinity: Anti HideOS
				if (m_abySeenPartStatus)
					delete[] m_abySeenPartStatus;
				m_abySeenPartStatus = NULL;
				// [TPT] - netfinity: Anti HideOS
				m_nPartCount = 0;
				// [TPT] - enkeyDev: ICS
				if (m_abyIncPartStatus)
					delete[] m_abyIncPartStatus;
				m_abyIncPartStatus = NULL;				
				// [TPT] - enkeyDev: ICS
			}
			if (socket && nNewState != DS_ERROR )
				socket->DisableDownloadLimit();
		}
		m_nDownloadState = nNewState;
		if( GetDownloadState() == DS_DOWNLOADING ){
			if ( IsEmuleClient() )
				SetRemoteQueueFull(false);
			SetRemoteQueueRank(0);
			SetAskedCountDown(0);
		}
		UpdateDisplayedInfo(true);
	}
}

void CUpDownClient::ProcessHashSet(char* packet,uint32 size){
	if (!m_fHashsetRequesting)
		throw CString(_T("unwanted hashset"));
	if ( (!reqfile) || md4cmp(packet,reqfile->GetFileHash())){
		CheckFailedFileIdReqs((uchar*)packet);
		throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (ProcessHashSet)");	
	}
	CSafeMemFile data((BYTE*)packet,size);
	if (reqfile->LoadHashsetFromFile(&data,true)){
		m_fHashsetRequesting = 0;
	}
	else{
		reqfile->hashsetneeded = true;
		throw GetResString(IDS_ERR_BADHASHSET);
	}
	// [TPT] - WebCache		
	// Superlexx - IFP
	if (thePrefs.IsWebCacheDownloadEnabled() && reqfile->GetStatus() == PS_EMPTY)
	{
		reqfile = STATIC_DOWNCAST(CPartFile, reqfile);
			reqfile->SetStatus(PS_READY);
		theApp.sharedfiles->SafeAddKFile(reqfile);
	}
	// [TPT] - WebCache	
	SendStartupLoadReq();
}

void CUpDownClient::CreateBlockRequests(int iMaxBlocks)
{
	ASSERT( iMaxBlocks >= 1 /*&& iMaxBlocks <= 3*/ );
	if (m_DownloadBlocks_list.IsEmpty())
	{
		uint16 count = iMaxBlocks - m_PendingBlocks_list.GetCount();
		Requested_Block_Struct** toadd = new Requested_Block_Struct*[count];
		if (reqfile->GetNextRequestedBlock(this,toadd,&count)){
			for (int i = 0; i < count; i++)
				m_DownloadBlocks_list.AddTail(toadd[i]);
		}
		delete[] toadd;
	}

	while (m_PendingBlocks_list.GetCount() < iMaxBlocks && !m_DownloadBlocks_list.IsEmpty())
	{
		Pending_Block_Struct* pblock = new Pending_Block_Struct;
		pblock->block = m_DownloadBlocks_list.RemoveHead();
		m_PendingBlocks_list.AddTail(pblock);
	}
}

void CUpDownClient::SendBlockRequests(){	
	if(reqfile && thePrefs.IsWebCacheDownloadEnabled()
		&& UsesCachedTCPPort() // uses a port that is usually cached
		&& SupportsWebCache() // client knows webcache protocol
		&& !HasLowID()	// has highID
		&& AllowProxyConnection() // not too many open sockets to proxy
// yonatan tmp -	&& ((thePrefs.WebCacheIsTransparent() && GetUserPort() == 80)|| (!thePrefs.WebCacheIsTransparent() && ResolveWebCacheName())) // found HTTP proxy // Superlexx - TPS
		&& ( thePrefs.WebCacheIsTransparent() ?
			GetUserPort() == 80			// yes - proceed if uploader uses port 80
			: ResolveWebCacheName() )	// no - proceed if proxy address can be resolved
		&& (thePrefs.GetWebCacheCachesLocalTraffic() || !IsBehindOurWebCache()) //JP changed to new IsBehindOurWebCache-function 		// WC-TODO: Shorter names?
		&& reqfile->GetNumberOfCurrentWebcacheConnectionsForThisFile() < reqfile->GetMaxNumberOfWebcacheConnectionsForThisFile() ) //JP Throttle OHCB-production
	{
	if (thePrefs.GetLogWebCacheEvents()) //JP log webcache events
		AddDebugLogLine(false, _T("Proxy-Connections for %s: %u Allowed: %u"), reqfile->GetFileName(), reqfile->GetNumberOfCurrentWebcacheConnectionsForThisFile(), reqfile->GetMaxNumberOfWebcacheConnectionsForThisFile());
		if (!m_PendingBlocks_list.IsEmpty()) return; //Added by SiRoB
		// Superlexx - COtN - start
		byte WC_TestFileHash[16] = { 0xE9, 0x05, 0x7A, 0xDC, 0x38, 0x05, 0x4A, 0xFA, 0x24, 0x81, 0x6E, 0x86, 0xBB, 0x08, 0xD2, 0x70 };
		bool isTestFile = !(bool)md4cmp(reqfile->GetFileHash(), WC_TestFileHash);
		bool doCache = false;
		CreateBlockRequests(1);
		if (m_PendingBlocks_list.IsEmpty())
		{
			SendCancelTransfer();
			SetDownloadState(DS_ONQUEUE, _T("noNeededRequeue"));	// SLUGFILLER: noNeededRequeue
			return;
		}
		if (isTestFile)
			doCache = true;
		else
		{
			Pending_Block_Struct* pending = m_PendingBlocks_list.GetHead();
#ifndef _DEBUG
			//[TPT] - Webcache 1.9 beta3
			//JP take successrate into account when deciding to do webcache download (rounded down)
			// Superlexx: modified this to actually work ;)
			uint32 minOHCBRecipients = (thePrefs.ses_WEBCACHEREQUESTS > 50 && thePrefs.ses_successfull_WCDOWNLOADS > 0) ? thePrefs.ses_WEBCACHEREQUESTS / thePrefs.ses_successfull_WCDOWNLOADS : 1;
			if (theApp.clientlist->GetNumberOfClientsBehindOurWebCacheHavingSameFileAndNeedingThisBlock(pending, minOHCBRecipients) >= minOHCBRecipients)
#endif _DEBUG
				doCache = true;
		}
	
		if( doCache ) {
			ASSERT( GetDownloadState() == DS_DOWNLOADING );
			//[TPT] - Webcache 1.9 beta3
			//Crypt.useNewKey = true;	// Superlexx - moved this to SendWebCacheBlockRequests()
			SendWebCacheBlockRequests();
			return;
		}
	}

// WebCache ////////////////////////////////////////////////////////////////////////////////////
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__RequestParts", this, reqfile!=NULL ? (char*)reqfile->GetFileHash() : NULL);
	m_dwLastBlockReceived = ::GetTickCount();
	if (!reqfile)
		return;
	CreateBlockRequests(3);					
	if (m_PendingBlocks_list.IsEmpty()){
			SendCancelTransfer();
			SetDownloadState(DS_ONQUEUE, _T("noNeededRequeue"));	// SLUGFILLER: noNeededRequeue
			return;
		}
	const int iPacketSize = 16+(3*4)+(3*4); // 40
	Packet* packet = new Packet(OP_REQUESTPARTS,iPacketSize);
	CSafeMemFile data((BYTE*)packet->pBuffer,iPacketSize);
	data.WriteHash16(reqfile->GetFileHash());
	POSITION pos = m_PendingBlocks_list.GetHeadPosition();
	for (uint32 i = 0; i != 3; i++){
		if (pos){
			Pending_Block_Struct* pending = m_PendingBlocks_list.GetNext(pos);
			ASSERT( pending->block->StartOffset <= pending->block->EndOffset );
			//ASSERT( pending->zStream == NULL );
			//ASSERT( pending->totalUnzipped == 0 );
			pending->fZStreamError = 0;
			pending->fRecovered = 0;
			data.WriteUInt32(pending->block->StartOffset);
		}
		else
			data.WriteUInt32(0);
	}
	pos = m_PendingBlocks_list.GetHeadPosition();
	for (uint32 i = 0; i != 3; i++){
		if (pos){
			Requested_Block_Struct* block = m_PendingBlocks_list.GetNext(pos)->block;
			uint32 endpos = block->EndOffset+1;
			data.WriteUInt32(endpos);
			if (thePrefs.GetDebugClientTCPLevel() > 0){
				CString strInfo;
				strInfo.Format(_T("  Block request %u: "), i);
				strInfo += DbgGetBlockInfo(block);
				strInfo.AppendFormat(_T(",  Complete=%s"), reqfile->IsComplete(block->StartOffset, block->EndOffset) ? _T("Yes(NOTE:)") : _T("No"));
				strInfo.AppendFormat(_T(",  PureGap=%s"), reqfile->IsPureGap(block->StartOffset, block->EndOffset) ? _T("Yes") : _T("No(NOTE:)"));
				strInfo.AppendFormat(_T(",  AlreadyReq=%s"), reqfile->IsAlreadyRequested(block->StartOffset, block->EndOffset) ? _T("Yes") : _T("No(NOTE:)"));
				strInfo += _T('\n');
				Debug(strInfo);
			}
		}
		else
		{
			data.WriteUInt32(0);
			if (thePrefs.GetDebugClientTCPLevel() > 0)
				Debug(_T("  Block request %u: <empty>\n"), i);
		}
	}
	theStats.AddUpDataOverheadFileRequest(packet->size);
	socket->SendPacket(packet,true,true);
}

/* Barry - Originally this only wrote to disk when a full 180k block 
           had been received from a client, and only asked for data in 
		   180k blocks.

		   This meant that on average 90k was lost for every connection
		   to a client data source. That is a lot of wasted data.

		   To reduce the lost data, packets are now written to a buffer
		   and flushed to disk regularly regardless of size downloaded.
		   This includes compressed packets.

		   Data is also requested only where gaps are, not in 180k blocks.
		   The requests will still not exceed 180k, but may be smaller to
		   fill a gap.
*/
void CUpDownClient::ProcessBlockPacket(char *packet, uint32 size, bool packed)
{
	uint32 nDbgStartPos = *((uint32*)(packet+16));
	if (thePrefs.GetDebugClientTCPLevel() > 1){
		if (packed)
			Debug(_T("  Start=%u  BlockSize=%u  Size=%u  %s\n"), nDbgStartPos, *((uint32*)(packet + 16+4)), size-24, DbgGetFileInfo((uchar*)packet));
		else
			Debug(_T("  Start=%u  End=%u  Size=%u  %s\n"), nDbgStartPos, *((uint32*)(packet + 16+4)), *((uint32*)(packet + 16+4)) - nDbgStartPos, DbgGetFileInfo((uchar*)packet));
	}

	// Ignore if no data required
	if (!(GetDownloadState() == DS_DOWNLOADING || GetDownloadState() == DS_NONEEDEDPARTS)){
		TRACE("%s - Invalid download state\n", __FUNCTION__);
		return;
	}

	theApp.emuledlg->transferwnd->downloadclientsctrl.RefreshClient(this);// [TPT] - TBH Transfers Window Style
	const int HEADER_SIZE = 24;

	// Update stats
	m_dwLastBlockReceived = ::GetTickCount();

	// Read data from packet
	CSafeMemFile data((BYTE*)packet, size);
	uchar fileID[16];
	data.ReadHash16(fileID);

	// Check that this data is for the correct file
	if ( (!reqfile) || md4cmp(packet, reqfile->GetFileHash()))
	{
		throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (ProcessBlockPacket)");
	}

	// Find the start & end positions, and size of this chunk of data
	uint32 nStartPos;
	uint32 nEndPos;
	uint32 nBlockSize = 0;
	uint32 uTransferredFileDataSize = size - HEADER_SIZE;
	nStartPos = data.ReadUInt32();
	if (packed)
	{
		nBlockSize = data.ReadUInt32();
		nEndPos = nStartPos + uTransferredFileDataSize;
	}
	else
		nEndPos = data.ReadUInt32();

	// Check that packet size matches the declared data size + header size (24)
	if (nEndPos == nStartPos || size != ((nEndPos - nStartPos) + HEADER_SIZE))
		throw GetResString(IDS_ERR_BADDATABLOCK) + _T(" (ProcessBlockPacket)");

	// -khaos--+++>
	// Extended statistics information based on which client and remote port sent this data.
	// The new function adds the bytes to the grand total as well as the given client/port.
	// bFromPF is not relevant to downloaded data.  It is purely an uploads statistic.
	thePrefs.Add2SessionTransferData(GetClientSoft(), GetUserPort(), false, false, uTransferredFileDataSize, false);
	// <-----khaos-

	//m_nDownDataRateMS += uTransferredFileDataSize; // [TPT]
	if (credits)
		credits->AddDownloaded(uTransferredFileDataSize, GetIP());
	//<<< [TPT] - eWombat SNAFU v2
	if (thePrefs.GetAntiSnafu())
		{
		AddTrustedTransfer(uTransferredFileDataSize); 
		if (GetTrustedTransfer()>3072000)
			{
			SetTrustedTransfer(0);
			if (!IsNotSUI())
				{
				Trust();
				if (IsSnafu())
					UnSnafu(true);
				}
			}
		}
	// >>> [TPT] - eWombat SNAFU v2

	// Move end back one, should be inclusive
	nEndPos--;

	// Loop through to find the reserved block that this is within
	for (POSITION pos = m_PendingBlocks_list.GetHeadPosition(); pos != NULL; )
	{
		POSITION posLast = pos;
		Pending_Block_Struct *cur_block = m_PendingBlocks_list.GetNext(pos);
		if ((cur_block->block->StartOffset <= nStartPos) && (cur_block->block->EndOffset >= nStartPos))
		{
			// Found reserved block

			if (cur_block->fZStreamError){
				if (thePrefs.GetVerbose())
					AddDebugLogLine(false, _T("PrcBlkPkt: Ignoring %u bytes of block starting at %u because of errornous zstream state for file \"%s\" - %s"), uTransferredFileDataSize, nStartPos, reqfile->GetFileName(), DbgGetClientInfo());
				reqfile->RemoveBlockFromList(cur_block->block->StartOffset, cur_block->block->EndOffset);
				return;
			}

			// Remember this start pos, used to draw part downloading in list
			m_nLastBlockOffset = nStartPos;  

			// Occasionally packets are duplicated, no point writing it twice
			// This will be 0 in these cases, or the length written otherwise
			uint32 lenWritten = 0;

			// Handle differently depending on whether packed or not
			if (!packed)
			{
				// Write to disk (will be buffered in part file class)
				lenWritten = reqfile->WriteToBuffer(uTransferredFileDataSize, 
													(BYTE *) (packet + HEADER_SIZE),
													nStartPos,
													nEndPos,
													cur_block->block,
													this);
			}
			else // Packed
			{
				ASSERT( (int)size > 0 );
				// Create space to store unzipped data, the size is only an initial guess, will be resized in unzip() if not big enough
				uint32 lenUnzipped = (size * 2); 
				// Don't get too big
				if (lenUnzipped > (EMBLOCKSIZE + 300))
					lenUnzipped = (EMBLOCKSIZE + 300);
				BYTE *unzipped = new BYTE[lenUnzipped];

				// Try to unzip the packet
				int result = unzip(cur_block, (BYTE*)(packet + HEADER_SIZE), uTransferredFileDataSize, &unzipped, &lenUnzipped);
				// no block can be uncompressed to >2GB, 'lenUnzipped' is obviously errornous.
				if (result == Z_OK && (int)lenUnzipped >= 0)
				{
					if (lenUnzipped > 0) // Write any unzipped data to disk
					{
						ASSERT( (int)lenUnzipped > 0 );

						// Use the current start and end positions for the uncompressed data
						nStartPos = cur_block->block->StartOffset + cur_block->totalUnzipped - lenUnzipped;
						nEndPos = cur_block->block->StartOffset + cur_block->totalUnzipped - 1;

						if (nStartPos > cur_block->block->EndOffset || nEndPos > cur_block->block->EndOffset){
							if (thePrefs.GetVerbose())
								DebugLogError(_T("PrcBlkPkt: ") + GetResString(IDS_ERR_CORRUPTCOMPRPKG),reqfile->GetFileName(),666);
							reqfile->RemoveBlockFromList(cur_block->block->StartOffset, cur_block->block->EndOffset);
							// There is no chance to recover from this error
						}
						else{
							// Write uncompressed data to file
							lenWritten = reqfile->WriteToBuffer(uTransferredFileDataSize,
								unzipped,
								nStartPos,
								nEndPos,
								cur_block->block,
								this);
						}
					}
				}
				else
				{
					if (thePrefs.GetVerbose())
					{
						CString strZipError;
						if (cur_block->zStream && cur_block->zStream->msg)
							strZipError.Format(_T(" - %hs"), cur_block->zStream->msg);
						if (result == Z_OK && (int)lenUnzipped < 0){
							ASSERT(0);
							strZipError.AppendFormat(_T("; Z_OK,lenUnzipped=%d"), lenUnzipped);
						}
						DebugLogError(_T("PrcBlkPkt: ") + GetResString(IDS_ERR_CORRUPTCOMPRPKG) + strZipError, reqfile->GetFileName(), result);
					}
					reqfile->RemoveBlockFromList(cur_block->block->StartOffset, cur_block->block->EndOffset);

					// If we had an zstream error, there is no chance that we could recover from it nor that we
					// could use the current zstream (which is in error state) any longer.
					if (cur_block->zStream){
						inflateEnd(cur_block->zStream);
						delete cur_block->zStream;
						cur_block->zStream = NULL;
					}

					// Although we can't further use the current zstream, there is no need to disconnect the sending 
					// client because the next zstream (a series of 10K-blocks which build a 180K-block) could be
					// valid again. Just ignore all further blocks for the current zstream.
					cur_block->fZStreamError = 1;
					cur_block->totalUnzipped = 0;
				}
				delete [] unzipped;
			}

			// These checks only need to be done if any data was written
			if (lenWritten > 0)
			{
				m_nTransferredDown += uTransferredFileDataSize;
				SetTransferredDownMini();

				// If finished reserved block
				if (nEndPos == cur_block->block->EndOffset)
				{
					reqfile->RemoveBlockFromList(cur_block->block->StartOffset, cur_block->block->EndOffset);
					delete cur_block->block;
					// Not always allocated
					if (cur_block->zStream){
						inflateEnd(cur_block->zStream);
						delete cur_block->zStream;
					}
					delete cur_block;
					m_PendingBlocks_list.RemoveAt(posLast);

					// Request next block
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugSend("More block requests", this);
					SendBlockRequests();	
				}
			}

			// Stop looping and exit method
			return;
		}
	}

	TRACE("%s - Dropping packet\n", __FUNCTION__);
}

int CUpDownClient::unzip(Pending_Block_Struct* block, BYTE* zipped, uint32 lenZipped, BYTE** unzipped, uint32* lenUnzipped, int iRecursion)
{
#define TRACE_UNZIP	/*TRACE*/

	TRACE_UNZIP("unzip: Zipd=%6u Unzd=%6u Rcrs=%d", lenZipped, *lenUnzipped, iRecursion);
  	int err = Z_DATA_ERROR;
  	try
	{
	    // Save some typing
	    z_stream *zS = block->zStream;
    
	    // Is this the first time this block has been unzipped
	    if (zS == NULL)
	    {
		    // Create stream
		    block->zStream = new z_stream;
		    zS = block->zStream;
    
		    // Initialise stream values
		    zS->zalloc = (alloc_func)0;
		    zS->zfree = (free_func)0;
		    zS->opaque = (voidpf)0;
    
		    // Set output data streams, do this here to avoid overwriting on recursive calls
		    zS->next_out = (*unzipped);
		    zS->avail_out = (*lenUnzipped);
    
		    // Initialise the z_stream
		    err = inflateInit(zS);
			if (err != Z_OK){
				TRACE_UNZIP("; Error: new stream failed: %d\n", err);
			    return err;
			}

			ASSERT( block->totalUnzipped == 0 );
		}

	    // Use whatever input is provided
	    zS->next_in  = zipped;
	    zS->avail_in = lenZipped;
    
	    // Only set the output if not being called recursively
	    if (iRecursion == 0)
	    {
		    zS->next_out = (*unzipped);
		    zS->avail_out = (*lenUnzipped);
	    }
    
	    // Try to unzip the data
		TRACE_UNZIP("; inflate(ain=%6u tin=%6u aout=%6u tout=%6u)", zS->avail_in, zS->total_in, zS->avail_out, zS->total_out);
	    err = inflate(zS, Z_SYNC_FLUSH);
    
	    // Is zip finished reading all currently available input and writing all generated output
	    if (err == Z_STREAM_END)
	    {
		    // Finish up
		    err = inflateEnd(zS);
			if (err != Z_OK){
				TRACE_UNZIP("; Error: end stream failed: %d\n", err);
			    return err;
			}
			TRACE_UNZIP("; Z_STREAM_END\n");

		    // Got a good result, set the size to the amount unzipped in this call (including all recursive calls)
		    (*lenUnzipped) = (zS->total_out - block->totalUnzipped);
		    block->totalUnzipped = zS->total_out;
	    }
	    else if ((err == Z_OK) && (zS->avail_out == 0) && (zS->avail_in != 0))
	    {
		    // Output array was not big enough, call recursively until there is enough space
			TRACE_UNZIP("; output array not big enough (ain=%u)\n", zS->avail_in);
    
		    // What size should we try next
		    uint32 newLength = (*lenUnzipped) *= 2;
		    if (newLength == 0)
			    newLength = lenZipped * 2;
    
		    // Copy any data that was successfully unzipped to new array
		    BYTE *temp = new BYTE[newLength];
			ASSERT( zS->total_out - block->totalUnzipped <= newLength );
		    MEMCOPY(temp, (*unzipped), (zS->total_out - block->totalUnzipped));
		    delete [] (*unzipped);
		    (*unzipped) = temp;
		    (*lenUnzipped) = newLength;
    
		    // Position stream output to correct place in new array
		    zS->next_out = (*unzipped) + (zS->total_out - block->totalUnzipped);
		    zS->avail_out = (*lenUnzipped) - (zS->total_out - block->totalUnzipped);
    
		    // Try again
		    err = unzip(block, zS->next_in, zS->avail_in, unzipped, lenUnzipped, iRecursion + 1);
	    }
	    else if ((err == Z_OK) && (zS->avail_in == 0))
	    {
			TRACE_UNZIP("; all input processed\n");
		    // All available input has been processed, everything ok.
		    // Set the size to the amount unzipped in this call (including all recursive calls)
		    (*lenUnzipped) = (zS->total_out - block->totalUnzipped);
		    block->totalUnzipped = zS->total_out;
	    }
	    else
	    {
		    // Should not get here unless input data is corrupt
			if (thePrefs.GetVerbose())
			{
				CString strZipError;
				if (zS->msg)
					strZipError.Format(_T(" %d: '%hs'"), err, zS->msg);
				else if (err != Z_OK)
					strZipError.Format(_T(" %d"), err);
				TRACE_UNZIP("; Error: %s\n", strZipError);
				DebugLogError(_T("Unexpected zip error%s in file \"%s\""), strZipError, reqfile ? reqfile->GetFileName() : NULL);
			}
	    }
    
	    if (err != Z_OK)
		    (*lenUnzipped) = 0;
  	}
  	catch (...){
		if (thePrefs.GetVerbose())
			DebugLogError(_T("Unknown exception in %hs: file \"%s\""), __FUNCTION__, reqfile ? reqfile->GetFileName() : NULL);
		err = Z_DATA_ERROR;
		ASSERT(0);
	}

	return err;
}

// [TPT]
// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
void CUpDownClient::CompDownloadRate(){
	// Add new sample
	TransferredData newItem = {m_nDownDatarateMeasure,::GetTickCount()};
	m_downHistory_list.AddTail(newItem);
	m_nDownDatarateMeasure = 0;

	
	// Remove old sample(s)
	uint32 oldTime = m_downHistory_list.GetHead().timeStamp - 1000; // must not be zero
	while(m_downHistory_list.GetSize() > thePrefs.GetDatarateSamples()){
		const TransferredData& oldItem = m_downHistory_list.GetHead();
		m_sumDownHistory -= oldItem.dataLength;	
		oldTime = oldItem.timeStamp;
		m_downHistory_list.RemoveHead();
	}

	// Compute datarate
	m_sumDownHistory += newItem.dataLength;
	uint32 deltaTime = newItem.timeStamp - oldTime; // [ms]
	m_nDownDatarate = (deltaTime > 0) ? (1000 * m_sumDownHistory / deltaTime) : 0;   // [bytes/s]

	// Check and then refresh GUI
	m_displayDownDatarateCounter++;

	if(m_displayDownDatarateCounter >= (100/TIMER_PERIOD)*DISPLAY_REFRESH && GetDownloadState() == DS_DOWNLOADING){
		m_displayDownDatarateCounter = 0;
		UpdateDisplayedInfo();
	}
}

void CUpDownClient::CheckDownloadTimeout()
{
	if (IsDownloadingFromPeerCache() && m_pPCDownSocket && m_pPCDownSocket->IsConnected())
	{
		ASSERT( DOWNLOADTIMEOUT < m_pPCDownSocket->GetTimeOut() );
		if (GetTickCount() - m_dwLastBlockReceived > DOWNLOADTIMEOUT)
		{
			OnPeerCacheDownSocketTimeout();
		}
	}	
	// [TPT] - WebCache		
	// yonatan http start
	else if (IsDownloadingFromWebCache() && m_pWCDownSocket) // jp proxy stall fix removed: && m_pWCDownSocket->IsConnected())
	{
		ASSERT( DOWNLOADTIMEOUT < m_pWCDownSocket->GetTimeOut() );
		if (m_pWCDownSocket->IsConnected())
		{
		if (GetTickCount() - m_dwLastBlockReceived > DOWNLOADTIMEOUT)
		{
			OnWebCacheDownSocketTimeout();
		}
	}	
	else //this shouldn't happen but aparently does maybe fixed by WebCacheDownSocket::OnConnect() function??
		{
			if (GetTickCount() - m_dwLastBlockReceived > DOWNLOADTIMEOUT)
			{
				if( thePrefs.GetLogWebCacheEvents() ) // yonatan tmp logging
				AddDebugLogLine( false, _T("Disconnected WCDownSocket Timed Out!!! PLEASE TELL THE WEBCACHE DEVELOPERS IF YOU EVER SEE THIS") );
			OnWebCacheDownSocketTimeout();
		}
	}
	}
	else if( !IsProxy() ) // proxies don't have a socket!
	// yonatan http end
	// [TPT] - WebCache	
	{
		if ((::GetTickCount() - m_dwLastBlockReceived) > DOWNLOADTIMEOUT)
		{
			ASSERT( socket != NULL );
			if (socket != NULL)
			{
				ASSERT( !socket->IsRawDataMode() );
				if (!socket->IsRawDataMode())
					SendCancelTransfer();
			}
			SetDownloadState(DS_ONQUEUE, _T("Timeout. More than 100 seconds since last complete block was received."), CUpDownClient::DSR_TIMEOUT); // [TPT] - Maella -Download Stop Reason-
		}
	}
}

UINT CUpDownClient::GetAvailablePartCount() const
{
	UINT result = 0;
	for (int i = 0;i < m_nPartCount;i++){
		if (IsPartAvailable(i))
			result++;
	}
	return result;
}

void CUpDownClient::SetRemoteQueueRank(uint16 nr){
	// [TPT] - itsonlyme: displayOptions
	if (nr != m_nRemoteQueueRank)
		m_nDifferenceQueueRank = (nr-m_nRemoteQueueRank);
	// [TPT] - itsonlyme: displayOptions
	m_nRemoteQueueRank = nr;
	UpdateDisplayedInfo();
}

void CUpDownClient::UDPReaskACK(uint16 nNewQR){
	m_bUDPPending = false;
	SetRemoteQueueRank(nNewQR);
    SetLastAskedTime(); // ZZ:DownloadManager
}

void CUpDownClient::UDPReaskFNF(){
	m_bUDPPending = false;
	if (GetDownloadState() != DS_DOWNLOADING){ // avoid premature deletion of 'this' client
		if (thePrefs.GetVerbose())
			AddDebugLogLine(DLP_LOW, false, _T("UDP FNF-Answer: %s - %s"),DbgGetClientInfo(), DbgGetFileInfo(reqfile ? reqfile->GetFileHash() : NULL));
		if (reqfile)
			reqfile->m_DeadSourceList.AddDeadSource(this);
		switch (GetDownloadState()) {
			case DS_ONQUEUE:
			case DS_NONEEDEDPARTS:
                DontSwapTo(reqfile); // ZZ:DownloadManager
                if (SwapToAnotherFile(_T("Source says it doesn't have the file. CUpDownClient::UDPReaskFNF()"), true, true, true, NULL, false, false))
					break;
				/*fall through*/
			default:
				theApp.downloadqueue->RemoveSource(this);
				if (!socket){
					if (Disconnected(_T("UDPReaskFNF socket=NULL")))
						delete this;
				}
		}
	}
	else
	{
		if (thePrefs.GetVerbose())
			DebugLogWarning(_T("UDP FNF-Answer: %s - did not remove client because of current download state"),GetUserName());
	}
}

// [TPT] - Maella -Unnecessary Protocol Overload-
void CUpDownClient::UDPReaskForDownload(){
	// This method can be called only every 30 seconds
	if((reqfile == NULL) || // No file to ping
	   (m_bUDPPending) ||    // there is an UDP reask in progress
	   (m_abyPartStatus == NULL) || // File status unknown yet
	   (::GetTickCount() - m_dwLastUDPReaskTime < SEC2MS(30))) // Every 30 seconds only
		return;
	
	if( m_nTotalUDPPackets > 3 && ((float)(m_nFailedUDPPackets/m_nTotalUDPPackets) > .3))
		return;
	// Time stamp
	m_dwLastUDPReaskTime = ::GetTickCount();

	//the line "m_bUDPPending = true;" use to be here
	// deadlake PROXYSUPPORT
	const ProxySettings& proxy = thePrefs.GetProxy();
	if(m_nUDPPort != 0 && thePrefs.GetUDPPort() != 0 &&
		!theApp.IsFirewalled() && !(socket && socket->IsConnected())&& (!proxy.UseProxy))
	{ 
		if( !HasLowID() )
	{ 
		//don't use udp to ask for sources
		if(IsSourceRequestAllowed())
			return;

        if(SwapToAnotherFile(_T("A4AF check before OP__ReaskFilePing. CUpDownClient::UDPReaskForDownload()"), true, false, false, NULL, true, true)) {
            return; // we swapped, so need to go to tcp
        }

		m_bUDPPending = true;
		CSafeMemFile data(128);
		data.WriteHash16(reqfile->GetFileHash());
		if (GetUDPVersion() > 3)
		{
			/*
			if (reqfile->IsPartFile())
				((CPartFile*)reqfile)->WritePartStatus(&data);
			else
				data.WriteUInt16(0);
				*/
			// [TPT] - SLUGFILLER: hideOS from eWombat
			if (reqfile->IsPartFile())
				((CPartFile*)reqfile)->WritePartStatus(&data,this);
			else if (!reqfile->HideOvershares(&data, this))
				data.WriteUInt16(0);
			// [TPT] - SLUGFILLER: hideOS
		}
		if (GetUDPVersion() > 2)
			data.WriteUInt16(reqfile->m_nCompleteSourcesCount);
		//[TPT] - Webcache 1.9 beta3
		if (SupportsMultiOHCBs() &&	AttachMultiOHCBsRequest(data))
		{
			DebugSend("OP__MultiFileReask", this, (char*)reqfile->GetFileHash());
			Packet* response = new Packet(&data, OP_WEBCACHEPROT);
			response->opcode = OP_MULTI_FILE_REASK;
			theStats.AddUpDataOverheadFileRequest(response->size);
			theApp.downloadqueue->AddUDPFileReasks();
			theApp.clientudp->SendPacket(response,GetIP(),GetUDPPort());
		}
		else
		{
			if (thePrefs.GetDebugClientUDPLevel() > 0)
				DebugSend("OP__ReaskFilePing", this, (char*)reqfile->GetFileHash());
			Packet* response = new Packet(&data, OP_EMULEPROT);
			response->opcode = OP_REASKFILEPING;
			theStats.AddUpDataOverheadFileRequest(response->size);
			theApp.downloadqueue->AddUDPFileReasks();
			theApp.clientudp->SendPacket(response,GetIP(),GetUDPPort());
		}
		//[TPT] - Webcache 1.9 beta3
		m_nTotalUDPPackets++;
	}
		else if (HasLowID() && GetBuddyIP() && GetBuddyPort() && HasValidBuddyID())
		{
			m_bUDPPending = true;
			CSafeMemFile data(128);
			data.WriteHash16(GetBuddyID());
			data.WriteHash16(reqfile->GetFileHash());
			if (GetUDPVersion() > 3)
			{
				if (reqfile->IsPartFile())
					((CPartFile*)reqfile)->WritePartStatus(&data);
				else
					data.WriteUInt16(0);
			}
			if (GetUDPVersion() > 2)
				data.WriteUInt16(reqfile->m_nCompleteSourcesCount);
			if (thePrefs.GetDebugClientUDPLevel() > 0)
				DebugSend("OP__ReaskFilePing", this, (char*)reqfile->GetFileHash());
			Packet* response = new Packet(&data, OP_EMULEPROT);
			response->opcode = OP_REASKCALLBACKUDP;
			theStats.AddUpDataOverheadFileRequest(response->size);
			theApp.downloadqueue->AddUDPFileReasks();
			theApp.clientudp->SendPacket(response, GetBuddyIP(), GetBuddyPort() );
			m_nTotalUDPPackets++;
		}
	}
}
// Maella end

void CUpDownClient::UpdateDisplayedInfo(bool force)
{
#ifdef _DEBUG
	force = true;
#endif
    DWORD curTick = ::GetTickCount();
    if(force || curTick-m_lastRefreshedDLDisplay > MINWAIT_BEFORE_DLDISPLAY_WINDOWUPDATE+m_random_update_wait) {
	    theApp.emuledlg->transferwnd->downloadlistctrl.UpdateItem(this);
		theApp.emuledlg->transferwnd->clientlistctrl.RefreshClient(this);
		theApp.emuledlg->transferwnd->downloadclientsctrl.RefreshClient(this);//[TPT] - New Transfers Window
        m_lastRefreshedDLDisplay = curTick;
    }
}

// ZZ:DownloadManager -->
const bool CUpDownClient::IsInNoNeededList(const CPartFile* fileToCheck) const {
    for (POSITION pos = m_OtherNoNeeded_list.GetHeadPosition();pos != 0;m_OtherNoNeeded_list.GetNext(pos)) {
        if(m_OtherNoNeeded_list.GetAt(pos) == fileToCheck) {
            return true;
        }
    }

    return false;
}
// <-- ZZ:DownloadManager

// ZZ:DownloadManager -->
const bool CUpDownClient::SwapToRightFile(CPartFile* SwapTo, CPartFile* cur_file, bool ignoreSuspensions, bool SwapToIsNNPFile, bool curFileisNNPFile, bool& wasSkippedDueToSourceExchange, bool doAgressiveSwapping, bool debug) {
    bool printDebug = debug && thePrefs.GetLogA4AF();

    if(printDebug) {
        AddDebugLogLine(DLP_LOW, false, _T("oooo Debug: SwapToRightFile. Start compare SwapTo: %s and cur_file %s"), SwapTo?SwapTo->GetFileName():_T("null"), cur_file->GetFileName());
        AddDebugLogLine(DLP_LOW, false, _T("oooo Debug: doAgressiveSwapping: %s"), doAgressiveSwapping?_T("true"):_T("false"));
    }

    if (!SwapTo) {
        return true;
    }

	// [TPT] - Sivka AutoHL Begin
    if(!curFileisNNPFile && cur_file->GetSourceCount() < cur_file->GetMaxSourcesPerFile() /*thePrefs.GetMaxSourcePerFile()*/ ||
        curFileisNNPFile && cur_file->GetSourceCount() < cur_file->GetMaxSourcesPerFile()*.8 /*thePrefs.GetMaxSourcePerFile()*.8*/) {
	// [TPT] - Sivka AutoHL End
            if(printDebug)
                AddDebugLogLine(DLP_VERYLOW, false, _T("oooo Debug: cur_file does probably not have too many sources."));

			// [TPT] - Sivka AutoHL Begin
            if(SwapTo->GetSourceCount() > SwapTo->GetMaxSourcesPerFile() /*thePrefs.GetMaxSourcePerFile()*/ ||
               SwapTo->GetSourceCount() >= SwapTo->GetMaxSourcesPerFile()*.8 /*thePrefs.GetMaxSourcePerFile()*.8*/ &&
			   // [TPT] - Sivka AutoHL End
               SwapTo == reqfile &&
               (
                GetDownloadState() == DS_LOWTOLOWIP ||
                GetDownloadState() == DS_REMOTEQUEUEFULL
               )
              ) {
                if(printDebug)
                    AddDebugLogLine(DLP_VERYLOW, false, _T("oooo Debug: SwapTo is about to be deleted due to too many sources on that file, so we can steal it."));
                return true;
            }

                if(ignoreSuspensions  || !IsSwapSuspended(cur_file, doAgressiveSwapping, curFileisNNPFile)) {
                    if(printDebug)
                        AddDebugLogLine(DLP_VERYLOW, false, _T("oooo Debug: No suspend block."));

                DWORD tempTick = ::GetTickCount();
                bool rightFileHasHigherPrio = CPartFile::RightFileHasHigherPrio(SwapTo, cur_file);
				// [TPT] - Maella -Spread Request- (idea SlugFiller)
                uint32 allNnpReaskTime = GetJitteredFileReaskTime()*2*(m_OtherNoNeeded_list.GetSize() + (GetDownloadState() == DS_NONEEDEDPARTS)?1:0); // wait two reask interval for each nnp file before reasking an nnp file
                if(!SwapToIsNNPFile && (!curFileisNNPFile || GetLastAskedTime(cur_file) == 0 || tempTick-GetLastAskedTime(cur_file) > allNnpReaskTime) && rightFileHasHigherPrio ||
                   SwapToIsNNPFile && curFileisNNPFile &&
                        (
                    GetLastAskedTime(SwapTo) != 0 &&
                            (
                     GetLastAskedTime(cur_file) == 0 ||
                     tempTick-GetLastAskedTime(SwapTo) < tempTick-GetLastAskedTime(cur_file)
                    ) ||
                    rightFileHasHigherPrio && GetLastAskedTime(SwapTo) == 0 && GetLastAskedTime(cur_file) == 0
                   ) ||
                   SwapToIsNNPFile && !curFileisNNPFile) {
                    if(printDebug)
                        if(!SwapToIsNNPFile && !curFileisNNPFile && rightFileHasHigherPrio)
                            AddDebugLogLine(DLP_VERYLOW, false, _T("oooo Debug: Higher prio."));
                        else if(!SwapToIsNNPFile && (GetLastAskedTime(cur_file) == 0 || tempTick-GetLastAskedTime(cur_file) > allNnpReaskTime) && rightFileHasHigherPrio)
                            AddDebugLogLine(DLP_VERYLOW, false, _T("oooo Debug: Time to reask nnp and it had higher prio."));
                        else if(GetLastAskedTime(SwapTo) != 0 &&
                                (
                                 GetLastAskedTime(cur_file) == 0 ||
                                 tempTick-GetLastAskedTime(SwapTo) < tempTick-GetLastAskedTime(cur_file)
                            )
                        )
                            AddDebugLogLine(DLP_VERYLOW, false, _T("oooo Debug: Both nnp and cur_file has longer time since reasked."));
                        else if(SwapToIsNNPFile && !curFileisNNPFile)
                            AddDebugLogLine(DLP_VERYLOW, false, _T("oooo Debug: SwapToIsNNPFile && !curFileisNNPFile"));
                        else
                            AddDebugLogLine(DLP_VERYLOW, false, _T("oooo Debug: Higher prio for unknown reason!"));

                    if(IsSourceRequestAllowed(cur_file) && (cur_file->AllowSwapForSourceExchange() || cur_file == reqfile && RecentlySwappedForSourceExchange()) ||
                       !(IsSourceRequestAllowed(SwapTo) && (SwapTo->AllowSwapForSourceExchange() || SwapTo == reqfile && RecentlySwappedForSourceExchange())) ||
                           (GetDownloadState()==DS_ONQUEUE && GetRemoteQueueRank() <= 50)) {
                            if(printDebug)
                                AddDebugLogLine(DLP_LOW, false, _T("oooo Debug: Source Request check ok."));
                            return true;
                        } else {
                            if(printDebug)
                                AddDebugLogLine(DLP_VERYLOW, false, _T("oooo Debug: Source Request check failed."));
                            wasSkippedDueToSourceExchange = true;
                        }
                    }

                if(IsSourceRequestAllowed(cur_file, true) && (cur_file->AllowSwapForSourceExchange() || cur_file == reqfile && RecentlySwappedForSourceExchange()) &&
                   !(IsSourceRequestAllowed(SwapTo, true) && (SwapTo->AllowSwapForSourceExchange() || SwapTo == reqfile && RecentlySwappedForSourceExchange())) &&
                       (GetDownloadState()!=DS_ONQUEUE || GetDownloadState()==DS_ONQUEUE && GetRemoteQueueRank() > 50)) {
                        wasSkippedDueToSourceExchange = true;

                        if(printDebug)
                            AddDebugLogLine(DLP_LOW, false, _T("oooo Debug: Source Exchange."));
                        return true;
                    }
                } else if(printDebug) {
                    AddDebugLogLine(DLP_VERYLOW, false, _T("oooo Debug: Suspend block."));
                }
            } else if(printDebug) {
        AddDebugLogLine(DLP_VERYLOW, false, _T("oooo Debug: cur_file probably has too many sources."));
    }

    if(printDebug)
        AddDebugLogLine(DLP_LOW, false, _T("oooo Debug: Return false"));

    return false;
}
// <-- ZZ:DownloadManager

// ZZ:DownloadManager -->
bool CUpDownClient::SwapToAnotherFile(LPCTSTR reason, bool bIgnoreNoNeeded, bool ignoreSuspensions, bool bRemoveCompletely, CPartFile* toFile, bool allowSame, bool isAboutToAsk, bool debug){
    bool printDebug = debug && thePrefs.GetLogA4AF();

    if(printDebug)
        AddDebugLogLine(DLP_LOW, false, _T("ooo Debug: Switching source %s Remove = %s; bIgnoreNoNeeded = %s; allowSame = %s; Reason = \"%s\""), DbgGetClientInfo(), (bRemoveCompletely ? _T("Yes") : _T("No")), (bIgnoreNoNeeded ? _T("Yes") : _T("No")), (allowSame ? _T("Yes") : _T("No")), reason);

    if(!bRemoveCompletely && allowSame && thePrefs.GetA4AFSaveCpu()) {
        // Only swap if we can't keep the old source
        if(printDebug)
            AddDebugLogLine(DLP_LOW, false, _T("ooo Debug: return false since prefs setting to save cpu is enabled."));
        return false;
    }

	bool doAgressiveSwapping = (bRemoveCompletely || !allowSame || isAboutToAsk);
    if(printDebug)
        AddDebugLogLine(DLP_LOW, false, _T("ooo Debug: doAgressiveSwapping: %s"), doAgressiveSwapping?_T("true"):_T("false"));

    if(!bRemoveCompletely && !ignoreSuspensions && allowSame && IsSwapSuspended(reqfile, doAgressiveSwapping, false)) {
        if(printDebug)
            AddDebugLogLine(DLP_LOW, false, _T("ooo Debug: return false due to IsSwapSuspended(reqfile)."));

        return false;
    }

    if(!bRemoveCompletely && allowSame && m_OtherRequests_list.IsEmpty() && (!bIgnoreNoNeeded || m_OtherNoNeeded_list.IsEmpty())) {
        // no file to swap too, and it's ok to keep it
        if(printDebug)
            AddDebugLogLine(DLP_LOW, false, _T("ooo Debug: return false due to no file to swap too, and it's ok to keep it."));
        return false;
    }

    if (!bRemoveCompletely &&
        (GetDownloadState() != DS_ONQUEUE &&
         GetDownloadState() != DS_NONEEDEDPARTS &&
         GetDownloadState() != DS_TOOMANYCONNS &&
         GetDownloadState() != DS_REMOTEQUEUEFULL &&
         GetDownloadState() != DS_CONNECTED
        )) {
        if(printDebug)
            AddDebugLogLine(DLP_LOW, false, _T("ooo Debug: return false due to wrong state."));
		return false;
    }

	CPartFile* SwapTo = NULL;
	CPartFile* cur_file = NULL;
	//int cur_prio= -1; //ZZ:DownloadManager
	POSITION finalpos = NULL;
	CTypedPtrList<CPtrList, CPartFile*>* usedList = NULL;

    if(allowSame && !bRemoveCompletely) {
        SwapTo = reqfile;
        if(printDebug)
            AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: allowSame: File %s SourceReq: %s"), reqfile->GetFileName(), IsSourceRequestAllowed(reqfile)?_T("true"):_T("false"));
    }

    bool SwapToIsNNP = (SwapTo != NULL && SwapTo == reqfile && GetDownloadState() == DS_NONEEDEDPARTS);

    CPartFile* skippedDueToSourceExchange = NULL;
    bool skippedIsNNP = false;

	if (!m_OtherRequests_list.IsEmpty()){
        if(printDebug)
            AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: m_OtherRequests_list"));

        usedList = &m_OtherRequests_list;
		for (POSITION pos = m_OtherRequests_list.GetHeadPosition();pos != 0;m_OtherRequests_list.GetNext(pos)){
			cur_file = m_OtherRequests_list.GetAt(pos);

            if(printDebug)
                AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Checking file: %s SoureReq: %s"), cur_file->GetFileName(), IsSourceRequestAllowed(cur_file)?_T("true"):_T("false"));

            if(!bRemoveCompletely && !ignoreSuspensions && allowSame && IsSwapSuspended(cur_file, doAgressiveSwapping, false)) {
                if(printDebug)
                    AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: continue due to IsSwapSuspended(file) == true"));
                continue;
            }

            if (cur_file != reqfile && theApp.downloadqueue->IsPartFile(cur_file) && !cur_file->IsStopped() 
				&& (cur_file->GetStatus(false) == PS_READY || cur_file->GetStatus(false) == PS_EMPTY))	
			{
                if(printDebug)
                    AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: It's a partfile, not stopped, etc."));

				if (toFile != NULL){
					if (cur_file == toFile){
                        if(printDebug)
                            AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Found toFile."));

                        SwapTo = cur_file;
                        SwapToIsNNP = false;
						finalpos = pos;
						break;
					}
				} else {
                    bool wasSkippedDueToSourceExchange = false;
                    if(SwapToRightFile(SwapTo, cur_file, ignoreSuspensions, SwapToIsNNP, false, wasSkippedDueToSourceExchange, doAgressiveSwapping, debug)) {
                        if(printDebug)
                            AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Swapping to file %s"), cur_file->GetFileName());

                        if(SwapTo && wasSkippedDueToSourceExchange) {
                            if(debug && thePrefs.GetLogA4AF()) AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Swapped due to source exchange possibility"));
                            bool discardSkipped = false;
                            if(SwapToRightFile(skippedDueToSourceExchange, SwapTo, ignoreSuspensions, skippedIsNNP, SwapToIsNNP, discardSkipped, doAgressiveSwapping, debug)) {
                                skippedDueToSourceExchange = SwapTo;
                                skippedIsNNP = skippedIsNNP?true:(SwapTo == reqfile && GetDownloadState() == DS_NONEEDEDPARTS);
                                if(printDebug)
                                    AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Skipped file was better than last skipped file."));
                            }
                        }

                        SwapTo = cur_file;
                        SwapToIsNNP = false;
					    finalpos=pos;
                    } else {
                        if(printDebug)
                            AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Keeping file %s"), SwapTo->GetFileName());
                        if(wasSkippedDueToSourceExchange) {
                            if(printDebug)
                                AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Kept the file due to source exchange possibility"));
                            bool discardSkipped = false;
                            if(SwapToRightFile(skippedDueToSourceExchange, cur_file, ignoreSuspensions, skippedIsNNP, false, discardSkipped, doAgressiveSwapping, debug)) {
                                skippedDueToSourceExchange = cur_file;
                                skippedIsNNP = false;
                                if(printDebug)
                                    AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Skipped file was better than last skipped file."));
                            }
                        }
                    }
                }
			}
		}
	}

    if ((!SwapTo || SwapTo == reqfile && GetDownloadState() == DS_NONEEDEDPARTS) && bIgnoreNoNeeded){
        if(printDebug)
            AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: m_OtherNoNeeded_list"));

        usedList = &m_OtherNoNeeded_list;
		for (POSITION pos = m_OtherNoNeeded_list.GetHeadPosition();pos != 0;m_OtherNoNeeded_list.GetNext(pos)){
			cur_file = m_OtherNoNeeded_list.GetAt(pos);

            if(printDebug)
                AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Checking file: %s "), cur_file->GetFileName());

            if(!bRemoveCompletely && !ignoreSuspensions && allowSame && IsSwapSuspended(cur_file, doAgressiveSwapping, true)) {
                if(printDebug)
                    AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: continue due to !IsSwapSuspended(file) == true"));
                continue;
            }

			if (cur_file != reqfile && theApp.downloadqueue->IsPartFile(cur_file) && !cur_file->IsStopped() 
				&& (cur_file->GetStatus(false) == PS_READY || cur_file->GetStatus(false) == PS_EMPTY) )	
			{
                if(printDebug)
                    AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: It's a partfile, not stopped, etc."));

				if (toFile != NULL){
					if (cur_file == toFile){
                        if(printDebug)
                            AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Found toFile."));
    
                        SwapTo = cur_file;
						finalpos = pos;
						break;
					}
				} else {
                    bool wasSkippedDueToSourceExchange = false;
                    if(SwapToRightFile(SwapTo, cur_file, ignoreSuspensions, SwapToIsNNP, true, wasSkippedDueToSourceExchange, doAgressiveSwapping, debug)) {
                        if(printDebug)
                            AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Swapping to file %s"), cur_file->GetFileName());

                        if(SwapTo && wasSkippedDueToSourceExchange) {
                            if(printDebug)
                                AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Swapped due to source exchange possibility"));
                            bool discardSkipped = false;
                            if(SwapToRightFile(skippedDueToSourceExchange, SwapTo, ignoreSuspensions, skippedIsNNP, SwapToIsNNP, discardSkipped, doAgressiveSwapping, debug)) {
                                skippedDueToSourceExchange = SwapTo;
                                skippedIsNNP = skippedIsNNP?true:(SwapTo == reqfile && GetDownloadState() == DS_NONEEDEDPARTS);
                                if(printDebug)
                                    AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Skipped file was better than last skipped file."));
                            }
                        }

                        SwapTo = cur_file;
                        SwapToIsNNP = true;
					    finalpos=pos;
                    } else {
                        if(printDebug)
                            AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Keeping file %s"), SwapTo->GetFileName());
                        if(wasSkippedDueToSourceExchange) {
                            if(debug && thePrefs.GetVerbose()) AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Kept the file due to source exchange possibility"));
                            bool discardSkipped = false;
                            if(SwapToRightFile(skippedDueToSourceExchange, cur_file, ignoreSuspensions, skippedIsNNP, true, discardSkipped, doAgressiveSwapping, debug)) {
                                skippedDueToSourceExchange = cur_file;
                                skippedIsNNP = true;
                                if(printDebug)
                                    AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Skipped file was better than last skipped file."));
                            }
                        }
                    }
				}
			}
		}
	}

    if (SwapTo){
        if(printDebug) {
            if(SwapTo != reqfile) {
                AddDebugLogLine(DLP_LOW, false, _T("ooo Debug: Found file to swap to %s"), SwapTo->GetFileName());
            } else {
                AddDebugLogLine(DLP_LOW, false, _T("ooo Debug: Will keep current file. %s"), SwapTo->GetFileName());
            }
        }

		CString strInfo(reason);
        if(skippedDueToSourceExchange) {
            bool wasSkippedDueToSourceExchange = false;
            bool skippedIsBetter = SwapToRightFile(SwapTo, skippedDueToSourceExchange, ignoreSuspensions, SwapToIsNNP, skippedIsNNP, wasSkippedDueToSourceExchange, doAgressiveSwapping, debug);
            if(skippedIsBetter || wasSkippedDueToSourceExchange) {
                SwapTo->SetSwapForSourceExchangeTick();
                SetSwapForSourceExchangeTick();

                strInfo = _T("******SourceExchange-Swap****** ") + strInfo;
                if(printDebug) {
                    AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Due to sourceExchange."));
                } else if(thePrefs.GetLogA4AF() && reqfile == SwapTo) {
                    AddDebugLogLine(DLP_LOW, false, _T("ooo Didn't swap source due to source exchange possibility. %s Remove = %s '%s' Reason: %s"), DbgGetClientInfo(), (bRemoveCompletely ? _T("Yes") : _T("No") ), (this->reqfile)?this->reqfile->GetFileName():_T("null"), strInfo);
                }
            } else if(printDebug) {
				AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Normal. SwapTo better than skippedDueToSourceExchange."));
            }
        } else if(printDebug) {
			AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Normal. skippedDueToSourceExchange == NULL"));
        }

		if (SwapTo != reqfile && DoSwap(SwapTo,bRemoveCompletely, strInfo)){
            if(debug && thePrefs.GetLogA4AF()) AddDebugLogLine(DLP_LOW, false, _T("ooo Debug: Swap successful."));
			usedList->RemoveAt(finalpos);
			return true;
        } else if(printDebug) {
            AddDebugLogLine(DLP_LOW, false, _T("ooo Debug: Swap didn't happen."));
        }
    }

    if(printDebug)
        AddDebugLogLine(DLP_LOW, false, _T("ooo Debug: Done %s"), DbgGetClientInfo());

	return false;
}
// <-- ZZ:DownloadManager

bool CUpDownClient::DoSwap(CPartFile* SwapTo, bool bRemoveCompletely, LPCTSTR reason) // ZZ:DownloadManager
{
    if (thePrefs.GetLogA4AF()) // ZZ:DownloadManager
        AddDebugLogLine(DLP_LOW, false, _T("ooo Swapped source %s Remove = %s '%s'   -->   %s Reason: %s"), DbgGetClientInfo(), (bRemoveCompletely ? _T("Yes") : _T("No") ), (this->reqfile)?this->reqfile->GetFileName():_T("null"), SwapTo->GetFileName(), reason); // ZZ:DownloadManager

	// 17-Dez-2003 [bc]: This "reqfile->srclists[sourcesslot].Find(this)" was the only place where 
	// the usage of the "CPartFile::srclists[100]" is more effective than using one list. If this
	// function here is still (again) a performance problem there is a more effective way to handle
	// the 'Find' situation. Hint: usage of a node ptr which is stored in the CUpDownClient.
    if(reqfile){
	POSITION pos = reqfile->srclist.Find(this);
	if(pos)
    {
    	reqfile->srclist.RemoveAt(pos);
    } else {
        AddDebugLogLine(DLP_HIGH, true, _T("o-o Unsync between parfile->srclist and client otherfiles list. Swapping client where client has file as reqfile, but file doesn't have client in srclist. %s Remove = %s '%s'   -->   '%s'  SwapReason: %s"), DbgGetClientInfo(), (bRemoveCompletely ? _T("Yes") : _T("No") ), (this->reqfile)?this->reqfile->GetFileName():_T("null"), SwapTo->GetFileName(), reason); // ZZ:DownloadManager
    	}
    }
	// remove this client from the A4AF list of our new reqfile
	POSITION pos2 = SwapTo->A4AFsrclist.Find(this);
	if (pos2) {
		SwapTo->A4AFsrclist.RemoveAt(pos2);
    } else {
        AddDebugLogLine(DLP_HIGH, true, _T("o-o Unsync between parfile->srclist and client otherfiles list. Swapping client where client has file in another list, but file doesn't have client in a4af srclist. %s Remove = %s '%s'   -->   '%s'  SwapReason: %s"), DbgGetClientInfo(), (bRemoveCompletely ? _T("Yes") : _T("No") ), (this->reqfile)?this->reqfile->GetFileName():_T("null"), SwapTo->GetFileName(), reason); // ZZ:DownloadManager
    }
	theApp.emuledlg->transferwnd->downloadlistctrl.RemoveSource(this,SwapTo);
	if(reqfile)
		reqfile->RemoveDownloadingSource(this);

	if(!bRemoveCompletely && reqfile)
	{
        reqfile->A4AFsrclist.AddTail(this);
		if (GetDownloadState() == DS_NONEEDEDPARTS)
			m_OtherNoNeeded_list.AddTail(reqfile);
		else
			m_OtherRequests_list.AddTail(reqfile);

		theApp.emuledlg->transferwnd->downloadlistctrl.AddSource(reqfile,this,true);
    } else {
        m_fileReaskTimes.RemoveKey(reqfile); // ZZ:DownloadManager (one resk timestamp for each file)
    }

	SetDownloadState(DS_NONE);
	CPartFile* pOldRequestFile = reqfile;	
	m_nDifferenceQueueRank = 0;	// [TPT] - itsonlyme: displayOptions
	SetRequestFile(SwapTo);
	pOldRequestFile->UpdatePartsInfo();
	pOldRequestFile->NewSrcIncPartsInfo(); // [TPT] - enkeyDev: ICS
	pOldRequestFile->UpdateAvailablePartsCount();

	SwapTo->srclist.AddTail(this);
	theApp.emuledlg->transferwnd->downloadlistctrl.AddSource(SwapTo,this,false);

	return true;
}

void CUpDownClient::DontSwapTo(/*const*/ CPartFile* file)
{
	DWORD dwNow = ::GetTickCount();

	for (POSITION pos = m_DontSwap_list.GetHeadPosition(); pos != 0; m_DontSwap_list.GetNext(pos))
		if(m_DontSwap_list.GetAt(pos).file == file) {
			m_DontSwap_list.GetAt(pos).timestamp = dwNow ;
			return;
		}
	PartFileStamp newfs = {file, dwNow };
	m_DontSwap_list.AddHead(newfs);
}

bool CUpDownClient::IsSwapSuspended(const CPartFile* file, const bool allowShortReaskTime, const bool fileIsNNP) // ZZ:DownloadManager
{
// ZZ:DownloadManager -->
    // Don't swap if we have reasked this client too recently
    if(GetTimeUntilReask(file, allowShortReaskTime, true, fileIsNNP) > 0)
        return true;
// <-- ZZ:DownloadManager

	if (m_DontSwap_list.GetCount()==0)
		return false;

	for (POSITION pos = m_DontSwap_list.GetHeadPosition(); pos != 0 && m_DontSwap_list.GetCount()>0; m_DontSwap_list.GetNext(pos)){
		if(m_DontSwap_list.GetAt(pos).file == file){
			if ( ::GetTickCount() - m_DontSwap_list.GetAt(pos).timestamp  >= PURGESOURCESWAPSTOP ) {
				m_DontSwap_list.RemoveAt(pos);
				return false;
			}
			else
				return true;
		}
		else if (m_DontSwap_list.GetAt(pos).file == NULL) // in which cases should this happen?
			m_DontSwap_list.RemoveAt(pos);
	}

	return false;
}

uint32 CUpDownClient::GetTimeUntilReask(const CPartFile* file, const bool allowShortReaskTime, const bool useGivenNNP, const bool givenNNP) const {
    DWORD lastAskedTimeTick = GetLastAskedTime(file);
    if(lastAskedTimeTick != 0) {
        DWORD tick = ::GetTickCount();

        DWORD reaskTime;
        // [TPT] - WebCache
        if(allowShortReaskTime)		{
            reaskTime = MIN_REQUESTTIME;
        } else if(useGivenNNP && givenNNP ||

                  file == reqfile && GetDownloadState() == DS_NONEEDEDPARTS && !SupportsWebCache() || // Superlexx - webcache - reask webcache-enabled NNP-sources more often
                   file != reqfile && IsInNoNeededList(file)) {
        // [TPT] - WebCache
	    // [TPT] - Maella -Spread Request- (idea SlugFiller)
            reaskTime = GetJitteredFileReaskTime()*2;
        } else {
            reaskTime = GetJitteredFileReaskTime();
        }
		// [TPT] - Maella -Spread Request- (idea SlugFiller)

        if(tick-lastAskedTimeTick < reaskTime) {
            return reaskTime-(tick-lastAskedTimeTick);
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

uint32 CUpDownClient::GetTimeUntilReask(const CPartFile* file) const {
    return GetTimeUntilReask(file, false);
}

uint32 CUpDownClient::GetTimeUntilReask() const {
    return GetTimeUntilReask(reqfile);
}

bool CUpDownClient::IsValidSource() const
{
	bool valid = false;
	switch(GetDownloadState())
	{
		case DS_DOWNLOADING:
		case DS_ONQUEUE:
		case DS_CONNECTED:
		case DS_NONEEDEDPARTS:
		case DS_REMOTEQUEUEFULL:
		case DS_REQHASHSET:
			valid = IsEd2kClient();
	}
	return valid;
}

void CUpDownClient::StartDownload()
{
	SetDownloadState(DS_DOWNLOADING);
	InitTransferredDownMini();
	SetDownStartTime();
	m_lastPartAsked = 0xffff;
	SendBlockRequests();
}

void CUpDownClient::SendCancelTransfer(Packet* packet)
{
	if (socket == NULL || !IsEd2kClient()){
		ASSERT(0);
		return;
	}
	
	if (!GetSentCancelTransfer())
	{
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__CancelTransfer", this);

		bool bDeletePacket;
		Packet* pCancelTransferPacket;
		if (packet)
		{
			pCancelTransferPacket = packet;
			bDeletePacket = false;
		}
		else
		{
			pCancelTransferPacket = new Packet(OP_CANCELTRANSFER, 0);
			bDeletePacket = true;
		}
		theStats.AddUpDataOverheadFileRequest(pCancelTransferPacket->size);
		socket->SendPacket(pCancelTransferPacket,bDeletePacket,true);
		SetSentCancelTransfer(1);
	}

	if (m_pPCDownSocket)
	{
		m_pPCDownSocket->Safe_Delete();
		m_pPCDownSocket = NULL;
		SetPeerCacheDownState(PCDS_NONE);
	}

// [TPT] - WebCache	
// yonatan http start //////////////////////////////////////////////////////////////////////////
	if (m_pWCDownSocket)
	{
		m_pWCDownSocket->Safe_Delete();
		m_pWCDownSocket = NULL;
		SetWebCacheDownState(WCDS_NONE);
	}
// yonatan http end ////////////////////////////////////////////////////////////////////////////
// [TPT] - WebCache	
}

void CUpDownClient::SetRequestFile(CPartFile* pReqFile)
{
	if (pReqFile != reqfile || reqfile == NULL)
		ResetFileStatusInfo();
	reqfile = pReqFile;
}

void CUpDownClient::ProcessAcceptUpload()
{
	m_fQueueRankPending = 1;
	if (reqfile && !reqfile->IsStopped() && (reqfile->GetStatus()==PS_READY || reqfile->GetStatus()==PS_EMPTY))
	{
		SetSentCancelTransfer(0);
		if (GetDownloadState() == DS_ONQUEUE)
		{
			// PC-TODO: If remote client does not answer the PeerCache query within a timeout, 
			// automatically fall back to ed2k download.
			if ( !SupportPeerCache() // client knows peercahce protocol
				||!thePrefs.IsPeerCacheDownloadEnabled() // user has enabled peercache downlaods
				|| !theApp.m_pPeerCache->IsCacheAvailable() // we have found our cache and its usable
				|| !theApp.m_pPeerCache->IsClientPCCompatible(m_nClientVersion, GetClientSoft()) // the client version is accepted by the cahce
				|| !SendPeerCacheFileRequest()) // request made
			{
				StartDownload();
			}
		}
	}
	else
	{
		SendCancelTransfer();
		SetDownloadState((reqfile==NULL || reqfile->IsStopped()) ? DS_NONE : DS_ONQUEUE);
	}
}

void CUpDownClient::ProcessEdonkeyQueueRank(char* packet, UINT size)
{
	CSafeMemFile data((BYTE*)packet, size);
	uint32 rank = data.ReadUInt32();
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		Debug(_T("  %u (prev. %d)\n"), rank, IsRemoteQueueFull() ? (UINT)-1 : (UINT)GetRemoteQueueRank());
	SetRemoteQueueRank(rank);
	CheckQueueRankFlood();
}

void CUpDownClient::ProcessEmuleQueueRank(char* packet, UINT size)
{
	if (size != 12)
		throw GetResString(IDS_ERR_BADSIZE);
	uint16 rank = PeekUInt16(packet);
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		Debug(_T("  %u (prev. %d)\n"), rank, IsRemoteQueueFull() ? (UINT)-1 : (UINT)GetRemoteQueueRank());
	SetRemoteQueueFull(false);
	SetRemoteQueueRank(rank);
	CheckQueueRankFlood();
}

void CUpDownClient::CheckQueueRankFlood()
{
	if (m_fQueueRankPending == 0)
	{
		if (GetDownloadState() != DS_DOWNLOADING)
		{
			if (m_fUnaskQueueRankRecv < 3) // NOTE: Do not increase this nr. without increasing the bits for 'm_fUnaskQueueRankRecv'
				m_fUnaskQueueRankRecv++;
			if (m_fUnaskQueueRankRecv == 3)
			{
				if (theApp.clientlist->GetBadRequests(this) < 2)
					theApp.clientlist->TrackBadRequest(this, 1);
				if (theApp.clientlist->GetBadRequests(this) == 2){
					theApp.clientlist->TrackBadRequest(this, -2); // reset so the client will not be rebanned right after the ban is lifted
					Ban(_T("QR flood"));
				}
				throw CString(thePrefs.GetLogBannedClients() ? _T("QR flood") : _T(""));
			}
		}
	}
	else
	{
		m_fQueueRankPending = 0;
		m_fUnaskQueueRankRecv = 0;
	}
}

uint32 CUpDownClient::GetLastAskedTime(const CPartFile* partFile) const
{
	// ZZ:DownloadManager (one resk timestamp for each file)
	CPartFile* file = (CPartFile*)partFile;

	if(file == NULL) {
		file = reqfile;
	}

	DWORD lastChangedTick;
	return m_fileReaskTimes.Lookup(file, lastChangedTick)?lastChangedTick:0;
}

void CUpDownClient::SetReqFileAICHHash(CAICHHash* val){
	if(m_pReqFileAICHHash != NULL && m_pReqFileAICHHash != val)
		delete m_pReqFileAICHHash;
	m_pReqFileAICHHash = val;
}

void CUpDownClient::SendAICHRequest(CPartFile* pForFile, uint16 nPart){
	CAICHRequestedData request;
	request.m_nPart = nPart;
	request.m_pClient = this;
	request.m_pPartFile = pForFile;
	CAICHHashSet::m_liRequestedData.AddTail(request);
	m_fAICHRequested = TRUE;
	CSafeMemFile data;
	data.WriteHash16(pForFile->GetFileHash());
	data.WriteUInt16(nPart);
	pForFile->GetAICHHashset()->GetMasterHash().Write(&data);
	Packet* packet = new Packet(&data, OP_EMULEPROT, OP_AICHREQUEST);
	SafeSendPacket(packet);
}

void CUpDownClient::ProcessAICHAnswer(char* packet, UINT size)
{
	if (m_fAICHRequested == FALSE){
		throw CString(_T("Received unrequested AICH Packet"));
	}
	m_fAICHRequested = FALSE;

	CSafeMemFile data((BYTE*)packet, size);
	if (size <= 16){	
		CAICHHashSet::ClientAICHRequestFailed(this);
		return;
	}
	uchar abyHash[16];
	data.ReadHash16(abyHash);
	CPartFile* pPartFile = theApp.downloadqueue->GetFileByID(abyHash);
	CAICHRequestedData request = CAICHHashSet::GetAICHReqDetails(this);
	uint16 nPart = data.ReadUInt16();
	if (pPartFile != NULL && request.m_pPartFile == pPartFile && request.m_pClient == this && nPart == request.m_nPart){
		CAICHHash ahMasterHash(&data);
		if ( (pPartFile->GetAICHHashset()->GetStatus() == AICH_TRUSTED || pPartFile->GetAICHHashset()->GetStatus() == AICH_VERIFIED)
			 && ahMasterHash == pPartFile->GetAICHHashset()->GetMasterHash())
		{
			if(pPartFile->GetAICHHashset()->ReadRecoveryData(request.m_nPart*PARTSIZE, &data)){
				// finally all checks passed, everythings seem to be fine
				// [TPT] - WebCache 
				if(thePrefs.GetLogICHEvents()) //JP log ICH events
					AddDebugLogLine(DLP_DEFAULT, false, _T("AICH Packet Answer: Succeeded to read and validate received recoverydata"));
				CAICHHashSet::RemoveClientAICHRequest(this);
				pPartFile->AICHRecoveryDataAvailable(request.m_nPart);
				return;
			}
			else
				// [TPT] - WebCache 
				if(thePrefs.GetLogICHEvents()) //JP log ICH events
					AddDebugLogLine(DLP_DEFAULT, false, _T("AICH Packet Answer: Succeeded to read and validate received recoverydata"));
		}
		else
			// [TPT] - WebCache 
			if(thePrefs.GetLogICHEvents()) //JP log ICH events
				AddDebugLogLine(DLP_HIGH, false, _T("AICH Packet Answer: Masterhash differs from packethash or hashset has no trusted Masterhash"));
	}
	else
		// [TPT] - WebCache 
		if(thePrefs.GetLogICHEvents()) //JP log ICH events
			AddDebugLogLine(DLP_HIGH, false, _T("AICH Packet Answer: requested values differ from values in packet"));

	CAICHHashSet::ClientAICHRequestFailed(this);
}

void CUpDownClient::ProcessAICHRequest(char* packet, UINT size){
	if (size != 16 + 2 + CAICHHash::GetHashSize())
		throw CString(_T("Received AICH Request Packet with wrong size"));
	
	CSafeMemFile data((BYTE*)packet, size);
	uchar abyHash[16];
	data.ReadHash16(abyHash);
	uint16 nPart = data.ReadUInt16();
	CAICHHash ahMasterHash(&data);
	CKnownFile* pKnownFile = theApp.sharedfiles->GetFileByID(abyHash);
	if (pKnownFile != NULL){
		if (pKnownFile->GetAICHHashset()->GetStatus() == AICH_HASHSETCOMPLETE && pKnownFile->GetAICHHashset()->HasValidMasterHash()
			&& pKnownFile->GetAICHHashset()->GetMasterHash() == ahMasterHash && pKnownFile->GetPartCount() > nPart
			&& pKnownFile->GetFileSize() > EMBLOCKSIZE && pKnownFile->GetFileSize() - PARTSIZE*nPart > EMBLOCKSIZE)
		{
			CSafeMemFile fileResponse;
			fileResponse.WriteHash16(pKnownFile->GetFileHash());
			fileResponse.WriteUInt16(nPart);
			pKnownFile->GetAICHHashset()->GetMasterHash().Write(&fileResponse);
			if (pKnownFile->GetAICHHashset()->CreatePartRecoveryData(nPart*PARTSIZE, &fileResponse)){
				// [TPT] - WebCache 
				if(thePrefs.GetLogICHEvents()) //JP log ICH events
					AddDebugLogLine(DLP_HIGH, false, _T("AICH Packet Request: Sucessfully created and send recoverydata for %s to %s"), pKnownFile->GetFileName(), DbgGetClientInfo());
				Packet* packAnswer = new Packet(&fileResponse, OP_EMULEPROT, OP_AICHANSWER);
				SafeSendPacket(packAnswer);
				return;
			}
			else
				// [TPT] - WebCache 
				if(thePrefs.GetLogICHEvents()) //JP log ICH events
					AddDebugLogLine(DLP_HIGH, false, _T("AICH Packet Request: Failed to create recoverydata for %s to %s"), pKnownFile->GetFileName(), DbgGetClientInfo());
		}
		else{
			// [TPT] - WebCache 
			if(thePrefs.GetLogICHEvents()) //JP log ICH events
				AddDebugLogLine(DLP_HIGH, false, _T("AICH Packet Request: Failed to create ecoverydata - Hashset not ready or requested Hash differs from Masterhash for %s to %s"), pKnownFile->GetFileName(), DbgGetClientInfo());
		}

	}
	else
		// [TPT] - WebCache 
		if(thePrefs.GetLogICHEvents()) //JP log ICH events
			AddDebugLogLine(DLP_HIGH, false, _T("AICH Packet Request: Failed to find requested shared file -  %s"), DbgGetClientInfo());
	
	Packet* packAnswer = new Packet(OP_AICHANSWER, 16, OP_EMULEPROT);
	md4cpy(packAnswer->pBuffer, abyHash);
	SafeSendPacket(packAnswer);
}

void CUpDownClient::ProcessAICHFileHash(CSafeMemFile* data, CPartFile* file){
	CPartFile* pPartFile = file;
	if (pPartFile == NULL){
		uchar abyHash[16];
		data->ReadHash16(abyHash);
		pPartFile = theApp.downloadqueue->GetFileByID(abyHash);
	}
	CAICHHash ahMasterHash(data);
	if(pPartFile != NULL && pPartFile == GetRequestFile()){
		SetReqFileAICHHash(new CAICHHash(ahMasterHash));
		pPartFile->GetAICHHashset()->UntrustedHashReceived(ahMasterHash, GetConnectIP());
	}
	else
		// [TPT] - WebCache 
		if(thePrefs.GetLogICHEvents()) //JP log ICH events
			AddDebugLogLine(DLP_HIGH, false, _T("ProcessAICHFileHash(): PartFile not found or Partfile differs from requested file, %s"), DbgGetClientInfo());
}

// Maella -Extended clean-up II-
void CUpDownClient::CleanUp(CPartFile* pDeletedFile){
	// Check if all pointers to the delete file have been removed
	if(reqfile == pDeletedFile){
		ASSERT(FALSE);
		reqfile = NULL;
		if (thePrefs.GetVerbose()) PhoenixLogWarning(_T("CleanUp() reports an error with reqfile"));
	}

	for(POSITION pos = m_OtherRequests_list.GetHeadPosition(); pos != 0; ){
		POSITION cur_pos = pos;
		CPartFile* cur_file = m_OtherRequests_list.GetNext(pos);
		if(cur_file == pDeletedFile){
			m_OtherRequests_list.RemoveAt(cur_pos);
			if (thePrefs.GetVerbose()) PhoenixLogWarning(_T("CleanUp() reports an error with m_OtherRequests_list"));
		}
	}

	for(POSITION pos = m_OtherNoNeeded_list.GetHeadPosition(); pos != 0; ){
		POSITION cur_pos = pos;
		CPartFile* cur_file = m_OtherNoNeeded_list.GetNext(pos);
		if(cur_file == pDeletedFile){
			m_OtherNoNeeded_list.RemoveAt(cur_pos);
			if (thePrefs.GetVerbose()) PhoenixLogWarning(_T("CleanUp() reports an error with m_OtherNoNeeded_list"));
		}
	}	

	// [TPT] Maella -Unnecesary protocol overload-
	PartStatusMap::iterator it = m_partStatusMap.find(pDeletedFile); 
	if(it != m_partStatusMap.end()){
		m_partStatusMap.erase(it);
	}
	// Maella end	
}
// Maella end

// Maella -Unnecesary protocol overload-
void CUpDownClient::TrigNextSafeAskForDownload(CPartFile* pFile){
	// Check when the specified file has been asked for the last time and
	// define when the file can be asked again without risking to be banished.
	if(pFile != NULL){
		PartStatusMap::const_iterator it = m_partStatusMap.find(pFile);
		if(it != m_partStatusMap.end()){
			// Compute then the next AskForDownload() might be 
			// performed without risk of Ban() (=> 11 minutes)
			if(it->second.dwStartUploadReqTime == 0){
				// File has never been asked before
				m_dwNextTCPAskedTime = 0; // Safe immediate reask
			}
			else {
				m_dwNextTCPAskedTime = it->second.dwStartUploadReqTime + MIN_REQUESTTIME + 60000;
			}
		}
		else {
			// File has never been asked before
			m_dwNextTCPAskedTime = 0; // Safe immediate reask
		}

		if(m_dwNextTCPAskedTime != 0 && m_dwNextTCPAskedTime > GetTickCount()){
			// Maella -Filter verbose messages-
			if(thePrefs.GetBlockMaellaSpecificDebugMsg() == false){
				if (thePrefs.GetVerbose()) AddPhoenixLogLine(false, _T("Reask '%s' for '%s' delayed to %u seconds"),
												GetUserName(),
												pFile->GetFileName(), 
												(m_dwNextTCPAskedTime - GetTickCount()) / 1000);
			}
			// Maella end
		}
	}
}
// Maella end
