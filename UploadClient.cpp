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
#include "UrlClient.h"
#include "Opcodes.h"
#include "Packets.h"
#include "UploadQueue.h"
#include "Statistics.h"
#include "ClientList.h"
#include "ClientUDPSocket.h"
#include "SharedFileList.h"
#include "KnownFileList.h"
#include "PartFile.h"
#include "ClientCredits.h"
#include "ListenSocket.h"
#include "PeerCacheSocket.h"
#include "Sockets.h"
#include "OtherFunctions.h"
#include "SafeFile.h"
#include "DownloadQueue.h"
#include "emuledlg.h"
#include "TransferWnd.h"
#include "Log.h"
#include "LastCommonRouteFinder.h" // MinToMB
#include "BandwidthControl.h" // MinToMB
#include "WebCacheSocket.h" // [TPT] - WebCache // yonatan http
#include "mod_version.h" // [TPT]

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif


//	members of CUpDownClient
//	which are mainly used for uploading functions 

// [TPT]
// Maella -Code Improvement-
void CUpDownClient::DrawUpStatusBar(CDC* dc, RECT* rect, bool onlygreyrect, bool  bFlat) const
{
	//TODOika: grey triclinkg
	const COLORREF crNeither = RGB(224, 224, 224);
	const COLORREF crNextSending = RGB(255,208,0);
	const COLORREF crBoth = bFlat ? RGB(0, 0, 0) : RGB(104, 104, 104);
	const COLORREF crSending = RGB(0, 150, 0);
	const COLORREF crBuffer = RGB(255, 100, 100);

	// wistily: UpStatusFix
	CKnownFile* currequpfile = theApp.sharedfiles->GetFileByID(requpfileid);
	uint32 filesize;
	if (currequpfile)
		filesize=currequpfile->GetFileSize();
	else
		filesize=PARTSIZE*m_nUpPartCount;
	// wistily: UpStatusFix

    if(filesize > 0) {
	    // Set size and fill with default color (grey)
		CBarShader statusBar(rect->bottom - rect->top, rect->right - rect->left);
		statusBar.SetFileSize(filesize); 
		statusBar.Fill(crNeither); 

		// [TPT] - Powershare - Added by SiRoB, See chunk that we hide
	    if (!onlygreyrect && m_abyUpPartStatus && currequpfile) { 	    
			uint32 i;
			const COLORREF crHiddenPartBySOTN = RGB(192, 96, 255);
			const COLORREF crHiddenPartByHideOS = RGB(96, 192, 255);
			for (i = 0;i < m_nUpPartCount;i++)
			    if(m_abyUpPartStatus[i])
				    statusBar.FillRange(PARTSIZE*(i),PARTSIZE*(i+1),crBoth);
				else if (m_abyUpPartStatusHidden)
					if (m_abyUpPartStatusHidden[i])
						statusBar.FillRange(PARTSIZE*(i),PARTSIZE*(i+1),m_bUpPartStatusHiddenBySOTN?crHiddenPartBySOTN:crHiddenPartByHideOS);
			for (i;i < currequpfile->GetED2KPartCount();i++)
				 if (m_abyUpPartStatusHidden)
					if (m_abyUpPartStatusHidden[i])
						statusBar.FillRange(PARTSIZE*(i),PARTSIZE*(i+1),m_bUpPartStatusHiddenBySOTN?crHiddenPartBySOTN:crHiddenPartByHideOS);						
	    }
		// [TPT] - Powershare - Added by SiRoB, See chunk that we hide
	    const Requested_Block_Struct* block;
	    if (!m_BlockRequests_queue.IsEmpty()){
		    block = m_BlockRequests_queue.GetHead();
		    if(block){
			    uint32 start = block->StartOffset/PARTSIZE;
			    statusBar.FillRange(start*PARTSIZE, (start+1)*PARTSIZE, crNextSending);
		    }
	    }
	    if (!m_DoneBlocks_list.IsEmpty()){
		    block = m_DoneBlocks_list.GetTail();
		    if(block){
			    uint32 start = block->StartOffset/PARTSIZE;
			    statusBar.FillRange(start*PARTSIZE, (start+1)*PARTSIZE, crNextSending);
		    }
	    }
	    if (!m_DoneBlocks_list.IsEmpty()){		    
		    for(POSITION pos=m_DoneBlocks_list.GetHeadPosition();pos!=0;){
			    block = m_DoneBlocks_list.GetNext(pos);
				statusBar.FillRange(block->StartOffset, block->EndOffset + 1, crSending);
			}

            // Also show what data is buffered (with color crBuffer)
            uint32 total = 0;
    
		    for(POSITION pos=m_DoneBlocks_list.GetTailPosition();pos!=0; ){
			    Requested_Block_Struct* block = m_DoneBlocks_list.GetPrev(pos);
    
                if(total + (block->EndOffset-block->StartOffset) <= GetQueueSessionPayloadUp()) {
                    // block is sent
			        statusBar.FillRange(block->StartOffset, block->EndOffset, crSending);
                    total += block->EndOffset-block->StartOffset;
                }
                else if (total < GetQueueSessionPayloadUp()){
                    // block partly sent, partly in buffer
                    total += block->EndOffset-block->StartOffset;
                    uint32 rest = total - GetQueueSessionPayloadUp();
                    uint32 newEnd = block->EndOffset-rest;
    
    			    statusBar.FillRange(block->StartOffset, newEnd, crSending);
    			    statusBar.FillRange(newEnd, block->EndOffset, crBuffer);
                }
                else{
                    // entire block is still in buffer
                    total += block->EndOffset-block->StartOffset;
    			    statusBar.FillRange(block->StartOffset, block->EndOffset, crBuffer);
                }
		    }
	    }
   	    statusBar.Draw(dc, rect->left, rect->top, bFlat);
    }
} 
// Maella end 

// Maella -Code Improvement-
void CUpDownClient::SetUploadState(EUploadState eNewState)
{
	if (eNewState != m_nUploadState)
	{

		// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
		if(m_nUploadState == US_UPLOADING || eNewState == US_UPLOADING){
			m_nUpDatarate = 0;
			m_nUpDatarateMeasure = 0;
			m_upHistory_list.RemoveAll();

			// Maella -New bandwidth control-
			if(eNewState == US_UPLOADING)
			{
				m_fSentOutOfPartReqs = 0;
				// Reduce the size of the buffer => avoid latency caused by buffer
				int oldValue = 0;
				int newValue = thePrefs.GetSendSocketBufferSize();
				int size = sizeof(oldValue);
				if(socket != NULL &&
				   socket->GetSockOpt(SO_SNDBUF, &oldValue, &size, SOL_SOCKET) == TRUE &&
				   socket->SetSockOpt(SO_SNDBUF, &newValue, sizeof(newValue), SOL_SOCKET) == TRUE)
				{
					// Maella -Filter verbose messages-
					if(thePrefs.GetBlockMaellaSpecificDebugMsg() == false)
					{
						if (thePrefs.GetVerbose()) 
							AddPhoenixLogLine(false, _T("Change size from %u to %u of the sending buffer"), oldValue, newValue);
					}
					// Maella end
				}
				else 
				{
					if (thePrefs.GetVerbose())
						AddDebugLogLine(false, _T("Failure to change size from %u to %u of the sending buffer"), oldValue, newValue);
				}
			}
			// Maella end
		}
		// Maella end
				
		// don't add any final cleanups for US_NONE here
		m_nUploadState = eNewState;
		theApp.emuledlg->transferwnd->clientlistctrl.RefreshClient(this);
	}
}
// Maella end

/**
 * Gets the queue score multiplier for this client, taking into consideration client's credits
 * and the requested file's priority.
 */
float CUpDownClient::GetCombinedFilePrioAndCredit() {
	if (credits == 0){
		ASSERT ( IsKindOf(RUNTIME_CLASS(CUrlClient)) );
		return 0;
	}

    return (uint32)(10.0f*credits->GetScoreRatio(GetIP())*float(GetFilePrioAsNumber()));
}

/**
 * Gets the file multiplier for the file this client has requested.
 */
int CUpDownClient::GetFilePrioAsNumber() const {
	CKnownFile* currequpfile = theApp.sharedfiles->GetFileByID(requpfileid);
	if(!currequpfile)
		return 0;
	
	// TODO coded by tecxx & herbert, one yet unsolved problem here:
	// sometimes a client asks for 2 files and there is no way to decide, which file the 
	// client finally gets. so it could happen that he is queued first because of a 
	// high prio file, but then asks for something completely different.
	int filepriority = 10; // standard
	// [TPT] - SUQWT
	if (theApp.clientcredits->IsSaveUploadQueueWaitTime()){
		switch(currequpfile->GetUpPriority()){
		// --> Moonlight: SUQWT - Changed the priority distribution for a wider spread.
			case PR_VERYHIGH:
				filepriority = 27;  // 18, 50% boost    <-- SUQWT - original values commented.
				break;
			case PR_HIGH: 
				filepriority = 12;  // 9, 33% boost
				break; 
			case PR_LOW: 
				filepriority = 5;   // 6, 17% reduction
				break; 
			case PR_VERYLOW:
				filepriority = 2;   // 2, no change
				break;
			case PR_NORMAL: 
				default: 
				filepriority = 8;   // 7, 14% boost
			break; 
		// <-- Moonlight: SUQWT
		} 
	}
	else{
	// [TPT] - SUQWT
	switch(currequpfile->GetUpPriority()){
		case PR_VERYHIGH:
			filepriority = 18;
			break;
		case PR_HIGH: 
			filepriority = 9; 
			break; 
		case PR_LOW: 
			filepriority = 6; 
			break; 
		case PR_VERYLOW:
			filepriority = 2;
			break;
		case PR_NORMAL: 
			default: 
			filepriority = 7; 
		break; 
	} 
	} 	// [TPT] - SUQWT
    return filepriority;
}

/**
 * Gets the current waiting score for this client, taking into consideration waiting
 * time, priority of requested file, and the client's credits.
 */
uint32 CUpDownClient::GetScore(bool sysvalue, bool isdownloading, bool onlybasevalue) const
{
	// Maella -Code Improvement-
	if (m_pszUsername == NULL || GetUploadFileID() == NULL)	
		return 0;

	// [TPT] - WebCache
	if (IsProxy())  // JP Proxies don't have credits
		return 0;	// JP Proxies don't have credits

	if (credits == 0){
		ASSERT ( IsKindOf(RUNTIME_CLASS(CUrlClient)) );
		return 0;
	}
	CKnownFile* currequpfile = theApp.sharedfiles->GetFileByID(requpfileid);
	if(!currequpfile)
		return 0;

	// bad clients (see note in function)
	if (credits->GetCurrentIdentState(GetIP()) == IS_IDBADGUY)
		return 0;
	// friend slot
	if (IsFriend() && GetFriendSlot() && !HasLowID())
		return 0x0FFFFFFF;

	if (IsSnafu() || IsNotSUI() || IsBanned() || m_bGPLEvildoer)// [TPT] - eWombat SNAFU v2
		return 0;

	if (sysvalue && HasLowID() && !(socket && socket->IsConnected())){
		return 0;
	}

    // Maella -One-queue-per-file- (idea bloodymad)
	if(thePrefs.GetEnableMultiQueue() == false)
	{

		int filepriority = GetFilePrioAsNumber();

		// calculate score, based on waitingtime and other factors
		float fBaseValue;
		if (onlybasevalue)
			fBaseValue = 100;
		else if (!isdownloading)
			fBaseValue = (float)(::GetTickCount()-GetWaitStartTime())/1000.0f;
		else{
			// we dont want one client to download forever
			// the first 15 min downloadtime counts as 15 min waitingtime and you get a 15 min bonus while you are in the first 15 min :)
			// (to avoid 20 sec downloads) after this the score won't raise anymore 
			fBaseValue = (float)(m_dwUploadTime-GetWaitStartTime());
			// [TPT] - SUQWT
			// Moonlight: SUQWT - I'm exploiting negative overflows to adjust wait start times. Overflows should not be an issue as long
			// as queue turnover rate is faster than 49 days.
			// ASSERT ( m_dwUploadTime-GetWaitStartTime() >= 0 ); //oct 28, 02: changed this from "> 0" to ">= 0"//original commented out
			// [TPT] - SUQWT
			//fBaseValue += (float)(::GetTickCount() - m_dwUploadTime > 900000)? 900000.0f : 1800000.0f;
		    fBaseValue += m_dwShieldTime ? (float) m_dwShieldTime : 1800000.0f; // MinToMB
			fBaseValue /= 1000.0f;
		}
		if(thePrefs.UseCreditSystem())
		{
			float modif = credits->GetScoreRatio(GetIP());
			fBaseValue *= modif;
		}
		// [TPT] - WebCache 	
		// Superlexx - TPS - reward clients using port 80
		if(thePrefs.IsWebCacheDownloadEnabled() // only if we have webcache downloading on
			&& SupportsWebCache()				// and if the remote client supports webcache
			&& thePrefs.WebCacheIsTransparent()	// our proxy is transparent
			&& GetUserPort() == 80				// remote client uses port 80
			&& !HasLowID())						// remote client has HighID
			fBaseValue *= (float)1.2;
		
		//	JP Webcache release START
		// boost clients if webcache upload will likely result in 3 or more proxy-downloads
		if (SupportsWebCache() 
			&& GetWebCacheName() != _T("") 
			&& thePrefs.IsWebcacheReleaseAllowed()
			&& currequpfile->ReleaseViaWebCache)
			{
				uint32 WebCacheClientCounter = currequpfile->GetNumberOfClientsRequestingThisFileUsingThisWebcache(GetWebCacheName(), 10);
				if (WebCacheClientCounter >= 3)
				{
					fBaseValue *= WebCacheClientCounter;
					fBaseValue += 5000;
				}
			}
		//	JP Webcache release END
		// [TPT] - WebCache	
	
		if (!onlybasevalue)
			fBaseValue *= (float(filepriority)/10.0f); 
			
		if( (IsEmuleClient() || this->GetClientSoft() < 10) && m_byEmuleVersion <= 0x19 )
			fBaseValue *= 0.5f;

		return (uint32)(fBaseValue + (GetPowerShared() && !onlybasevalue ? 1000000.0f : 0.0f)); // [TPT] - PowerShare
	}
	else 
	{
		if(onlybasevalue == true)
		{
			// Default value
			float fBaseValue = 100.0f;

			// Bonus Credit
			if(thePrefs.UseCreditSystem() == true && credits != NULL)
				fBaseValue *= credits->GetScoreRatio(GetIP());
			
			// Penality client types
			if((IsEmuleClient() || this->GetClientSoft() < 10) && m_byEmuleVersion <= 0x19)
				fBaseValue *= 0.5f;
			
			return (uint32)fBaseValue;
		}
		else 
		{
			// Client score
			// Remark: there is an overflow after ~49 days
			uint32 clientScore;
			if(isdownloading == false)
			{
				clientScore = ::GetTickCount()-GetWaitStartTime();
			}
			else 
			{
				// we dont want one client to download forever
				// the first 15 min downloadtime counts as 15 min waitingtime and you get a 15 min bonus while you are in the first 15 min :)
				// (to avoid 20 sec downloads) after this the score won't raise anymore
				clientScore = (m_dwUploadTime-GetWaitStartTime());
				clientScore += (::GetTickCount() - m_dwUploadTime > 900000) ? 900000 : 1800000;
			}
			clientScore /= 1000; // about +1 point each second

			// Bonus Credit
			if(thePrefs.UseCreditSystem() == true && credits != NULL)
				clientScore = (uint32)(credits->GetScoreRatio(GetIP()) * clientScore);

			// Penality client types
			if((IsEmuleClient() || this->GetClientSoft() < 10) && m_byEmuleVersion <= 0x19)
				clientScore /= 2;

			// File Score
			// Remark: there is an overflow after ~49 days
			uint32 fileScore = currequpfile->GetFileScore(isdownloading, GetUpStartTimeDelay()); // about +1 point each second 

			// Final score	
			// Remark: The whole timming of eMule should be rewritten. 
			//         The rollover of the main timer (GetTickCount) is not supported.
			//         A logarithmic scale would fit better here, but it might be slower.
			uint32 runTime = (GetTickCount() - theStats.starttime) / 1000;
			if(runTime <= 3600)
				// Less than 1 hour
				return 1000*fileScore + clientScore; // 1 second resolution
			if(runTime <= 2*3600)
				// Less than 2 hours
				return 1000*fileScore + clientScore/10; // 10 seconds resolution
			else
				// More than 2 hours
				return 1000*fileScore + clientScore/100; // 100 seconds resolution
		}
	}
	// Maella end
}


// [TPT] - Powershare
bool CUpDownClient::GetPowerShared() const {
	//MORPH START - Changed by SiRoB, Keep PowerShare State when client have been added in uploadqueue
	if(IsSnafu()) 
		return false;

	bool bPowerShared = false;
	if (GetUploadFileID() != NULL && theApp.sharedfiles->GetFileByID(GetUploadFileID()) != NULL)
		bPowerShared = theApp.sharedfiles->GetFileByID(GetUploadFileID())->GetPowerShared();
	
	return bPowerShared;
	//MORPH END   - Changed by SiRoB, Keep PowerShare State when client have been added in uploadqueue
}
// [TPT] - Powershare END



class CSyncHelper
{
public:
	CSyncHelper()
	{
		m_pObject = NULL;
	}
	~CSyncHelper()
	{
		if (m_pObject)
			m_pObject->Unlock();
	}
	CSyncObject* m_pObject;
};


// [TPT] - MinToMB
// Change 15 minutes to bytes (because of slot focus)
uint32 CUpDownClient::MinToMB() const{
	float upload;
	
	if(thePrefs.IsDynUpEnabled())
		upload = theApp.lastCommonRouteFinder->GetUpload() / 1024.0f;
	else
	{
		// Compute all datarates elapsed for the last 5 seconds		
		uint32 eMuleOut; uint32 none;
		theApp.pBandWidthControl->GetDatarates(5, // 5 seconds
											none, none,
											eMuleOut, none,
											none, none);
		upload = (float)eMuleOut / 1024.0f;
	}

	uint32 upPerClient = UPLOAD_CLIENT_DATARATE;
	if( upload > 20.0f || upload >= UNLIMITED) // [TPT]
		upPerClient += upload * 23; // 23 ~ 1000 / 43

	if( upPerClient > 7680 )
		upPerClient = 7680;

	uint32 result = upPerClient * MIN2S(15); //900 second ~ 15 minutes;
	if (result < 1000000)
		result = 1000000;

	return result;
}


// [TPT]
// Add a return bool value to check if this client has been removed from uploadqueue
bool CUpDownClient::CreateNextBlockPackage(){
	if (!m_dwShieldTime && GetQueueSessionPayloadUp() > MinToMB()){ // Pawcio: BC for Slot Focus (minutes changed to send MB) // MinToMB
		m_dwShieldTime = ::GetTickCount() - m_dwUploadTime;
		if (m_dwShieldTime > MIN2MS(15)) //Max 15 min shield
			m_dwShieldTime = MIN2MS(15);
	}
	//<<< [TPT] - eWombat SNAFU v2
	// check if we should kick this client
	if (!UploadAllowed()) //if it banned, kick it from upload, should only happen if it gets banned during upload
	{
		theApp.uploadqueue->RemoveFromUploadQueue(this, _T("Client banned"), CUpDownClient::USR_SNAFU);
		// Signal to the remote side the end of the upload session
		SendOutOfPartReqsAndAddToWaitingQueue();
		return false;
	}
	//<<< [TPT] - eWombat SNAFU v2
	
	// [TPT] - MUS
	if (m_addedPayloadQueueSession > 0 && m_addedPayloadQueueSession-GetQueueSessionPayloadUp() < 50 * 1024)
		toRemove = true;
	else
		toRemove = false;
	// [TPT] - MUS
    // See if we can do an early return. There may be no new blocks to load from disk and add to buffer, or buffer may be large enough allready.
    if(m_BlockRequests_queue.IsEmpty() || // There are no new blocks requested
       m_addedPayloadQueueSession > GetQueueSessionPayloadUp() && m_addedPayloadQueueSession-GetQueueSessionPayloadUp() > 150*1024) { // the buffered data is large enough allready // [TPT] - MUS
        return false;
    }

    CFile file;
	byte* filedata = 0;
	CString fullname;
	bool bFromPF = true; // Statistic to breakdown uploaded data by complete file vs. partfile.
	CSyncHelper lockFile;
	try{
        // Buffer new data if current buffer is less than 1 MBytes
        while (!m_BlockRequests_queue.IsEmpty() &&
               (m_addedPayloadQueueSession <= GetQueueSessionPayloadUp() || m_addedPayloadQueueSession-GetQueueSessionPayloadUp() < 100*1024)) {

			Requested_Block_Struct* currentblock = m_BlockRequests_queue.GetHead();
			CKnownFile* srcfile = theApp.sharedfiles->GetFileByID(currentblock->FileID);
			if (!srcfile)
				throw GetResString(IDS_ERR_REQ_FNF);

			if (srcfile->IsPartFile() && ((CPartFile*)srcfile)->GetStatus() != PS_COMPLETE){
				// Do not access a part file, if it is currently moved into the incoming directory.
				// Because the moving of part file into the incoming directory may take a noticable 
				// amount of time, we can not wait for 'm_FileCompleteMutex' and block the main thread.
				if (!((CPartFile*)srcfile)->m_FileCompleteMutex.Lock(0)){ // just do a quick test of the mutex's state and return if it's locked.
					return false;
				}
				lockFile.m_pObject = &((CPartFile*)srcfile)->m_FileCompleteMutex;
				// If it's a part file which we are uploading the file remains locked until we've read the
				// current block. This way the file completion thread can not (try to) "move" the file into
				// the incoming directory.

				fullname = RemoveFileExtension(((CPartFile*)srcfile)->GetFullName());
			}
			else{
				fullname.Format(_T("%s\\%s"),srcfile->GetPath(),srcfile->GetFileName());
			}
		
			uint32 togo;
			if (currentblock->StartOffset > currentblock->EndOffset){
				togo = currentblock->EndOffset + (srcfile->GetFileSize() - currentblock->StartOffset);
			}
			else{
				togo = currentblock->EndOffset - currentblock->StartOffset;
				if (srcfile->IsPartFile() && !((CPartFile*)srcfile)->IsRangeShareable(currentblock->StartOffset,currentblock->EndOffset-1))	// SLUGFILLER: SafeHash - final safety precaution
					throw GetResString(IDS_ERR_INCOMPLETEBLOCK);
			}

			// [TPT] - Maella -Allow Hybrid to download from eMule-	
			if(togo > EMBLOCKSIZE*3 && GetClientSoft() != SO_EDONKEYHYBRID)
				throw GetResString(IDS_ERR_LARGEREQBLOCK);
			else if(togo > 243200)
				// Only Hybrid is allowed to request larger blocks, otherwise 
				// it could be exploited by leechers to stay longer in the 
				// upload queue (see when CUploadQueue::CheckForTimeOver() is called)
				throw GetResString(IDS_ERR_LARGEREQBLOCK);
			// Maella end
			
			if (!srcfile->IsPartFile()){
				bFromPF = false; // This is not a part file...
				if (!file.Open(fullname,CFile::modeRead|CFile::osSequentialScan|CFile::shareDenyNone))
					throw GetResString(IDS_ERR_OPEN);
				file.Seek(currentblock->StartOffset,0);
				
				filedata = new byte[togo+500];
				if (uint32 done = file.Read(filedata,togo) != togo){
					file.SeekToBegin();
					file.Read(filedata + done,togo-done);
				}
				file.Close();
			}
			else{
				CPartFile* partfile = (CPartFile*)srcfile;

				partfile->m_hpartfile.Seek(currentblock->StartOffset,0);
				
				filedata = new byte[togo+500];
				if (uint32 done = partfile->m_hpartfile.Read(filedata,togo) != togo){
					partfile->m_hpartfile.SeekToBegin();
					partfile->m_hpartfile.Read(filedata + done,togo-done);
				}
			}
			if (lockFile.m_pObject){
				lockFile.m_pObject->Unlock(); // Unlock the (part) file as soon as we are done with accessing it.
				lockFile.m_pObject = NULL;
			}

			SetUploadFileID(srcfile);
			
			// [TPT] - WebCache 			
			if (IsUploadingToWebCache()) // Superlexx - encryption: encrypt here
			{
				Crypt.RefreshLocalKey();
				Crypt.encryptor.SetKey(Crypt.localKey, WC_KEYLENGTH);
				Crypt.encryptor.DiscardBytes(16); // we must throw away 16 bytes of the key stream since they were already used once, 16 is the file hash length
				Crypt.encryptor.ProcessString(filedata, togo);
			}
			// [TPT] - WebCache 

			// check extention to decide whether to compress or not
			CString ext = srcfile->GetFileName();
			ext.MakeLower();
			int pos = ext.ReverseFind(_T('.'));
			if (pos>-1)
				ext = ext.Mid(pos);
			bool compFlag = (ext!=_T(".zip") && ext!=_T(".rar") && ext!=_T(".ace") && ext!=_T(".ogm"));
			if (ext==_T(".avi") && thePrefs.GetDontCompressAvi())
				compFlag=false;

			// [TPT] - Maella -Check for file name extension- fix for lower/upper case in filename
			if(!IsUploadingToPeerCache() && !IsUploadingToWebCache() && m_byDataCompVer == 1 && compFlag){ // [TPT] - WebCache // yonatan http
				// Compression is supported by the remote client
				CreatePackedPackets(filedata,togo,currentblock,bFromPF);
			}
			else {
				// Remote client doesn't support compression or
				// File already compressed. Don't waste CPU time to recompress it.
				CreateStandartPackets(filedata,togo,currentblock,bFromPF);
			}
			// Maella end
			
			// file statistic
			srcfile->statistic.AddTransferred(currentblock->StartOffset, togo);	// [TPT] - SLUGFILLER: Spreadbars

            m_addedPayloadQueueSession += togo;

			m_DoneBlocks_list.AddHead(m_BlockRequests_queue.RemoveHead());
			delete[] filedata;
			filedata = 0;
			// [TPT] - Maella -One-queue-per-file- (idea bloodymad)
			srcfile->UpdateStartUploadTime();
			// Maella end
		}
	}
	catch(CString error)
	{
		if (thePrefs.GetVerbose())
			DebugLogWarning(GetResString(IDS_ERR_CLIENTERRORED), GetUserName(), error);
		theApp.uploadqueue->RemoveFromUploadQueue(this, _T("Client error: ") + error, CUpDownClient::USR_EXCEPTION); // Maella -Upload Stop Reason-
			delete[] filedata;
		return false;
	}
	catch(CFileException* e)
	{
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		e->GetErrorMessage(szError, ARRSIZE(szError));
		if (thePrefs.GetVerbose())
			DebugLogWarning(_T("Failed to create upload package for %s - %s"), GetUserName(), szError);
		theApp.uploadqueue->RemoveFromUploadQueue(this, ((CString)_T("Failed to create upload package.")) + szError, CUpDownClient::USR_EXCEPTION);	
			delete[] filedata;
		e->Delete();
		return false;
	}
	return true;
}

void CUpDownClient::ProcessExtendedInfo(CSafeMemFile* data, CKnownFile* tempreqfile)
{
	if (m_abyUpPartStatus) 
	{
		delete[] m_abyUpPartStatus;
		m_abyUpPartStatus = NULL;	// added by jicxicmic
	}
    // [TPT] - Powershare
	if (m_abyUpPartStatusHidden)
	{
		delete[] m_abyUpPartStatusHidden;
		m_abyUpPartStatusHidden = NULL;
	}
    // [TPT] - Powershare
	m_nUpPartCount = 0;
	m_nUpCompleteSourcesCount= 0;
	if( GetExtendedRequestsVersion() == 0 )
		return;
	uint16 nED2KUpPartCount = data->ReadUInt16();
	// [TPT] - netfinity: Anti HideOS Pawcio add-on
	bool isPartialSource = (tempreqfile == reqfile);
	if (isPartialSource && m_abySeenPartStatus == NULL)
	{
		m_abySeenPartStatus = new uint8[tempreqfile->GetPartCount()];
		for (int i = 0; i < tempreqfile->GetPartCount(); i++)
			m_abySeenPartStatus[i] = 0;
	}
	// [TPT] - netfinity: Anti HideOS
	if (!nED2KUpPartCount)
	{
		m_nUpPartCount = tempreqfile->GetPartCount();
		m_abyUpPartStatus = new uint8[m_nUpPartCount];
		MEMZERO(m_abyUpPartStatus, m_nUpPartCount);
	}
	else
	{
		if (tempreqfile->GetED2KPartCount() != nED2KUpPartCount)
		{
			//We already checked if we are talking about the same file.. So if we get here, something really strange happened!
			m_nUpPartCount = 0;
			return;
		}
		m_nUpPartCount = tempreqfile->GetPartCount();
		m_abyUpPartStatus = new uint8[m_nUpPartCount];
		uint16 done = 0;
		while (done != m_nUpPartCount)
		{
			uint8 toread = data->ReadUInt8();
			for (sint32 i = 0;i != 8;i++){
				m_abyUpPartStatus[done] = ((toread>>i)&1)? 1:0;
//				We may want to use this for another feature..
//				if (m_abyUpPartStatus[done] && !tempreqfile->IsComplete(done*PARTSIZE,((done+1)*PARTSIZE)-1))
//					bPartsNeeded = true;
				// [TPT] - netfinity: Anti HideOS Pawcio add-on
				if (isPartialSource && m_abyUpPartStatus[done])
					m_abySeenPartStatus[done] = 1;
				// [TPT] - netfinity: Anti HideOS
				done++;
				if (done == m_nUpPartCount)
					break;
			}
		}
		bool alreadyUpdated = false;  // [TPT] - netfinity: Anti HideOS
		if (GetExtendedRequestsVersion() > 1)
		{
			uint16 nCompleteCountLast = GetUpCompleteSourcesCount();
			uint16 nCompleteCountNew = data->ReadUInt16();
			SetUpCompleteSourcesCount(nCompleteCountNew);
			if (nCompleteCountLast != nCompleteCountNew)
			{
	                        if(tempreqfile->IsPartFile())	
 	                        	((CPartFile*)tempreqfile)->UpdatePartsInfo();	
 	                        else
					tempreqfile->UpdatePartsInfo();
				alreadyUpdated = true; // [TPT] - netfinity: Anti HideOS
			}
		}
		// [TPT] - netfinity: Anti HideOS
		if (isPartialSource && !alreadyUpdated)
			tempreqfile->UpdatePartsInfo();
		// [TPT] - netfinity: Anti HideOS
	}
	theApp.emuledlg->transferwnd->queuelistctrl.RefreshClient(this);

	// [TPT]
	// Maella -Code Improvement-
	if(tempreqfile->IsPartFile() == true && m_nUpPartCount != 0){
		// Check if a source has now chunk that we can need
		POSITION pos = m_OtherNoNeeded_list.Find(tempreqfile);
		if(pos != NULL){
			for(uint16 i = 0; i < m_nUpPartCount; i++){ 
				if(m_abyUpPartStatus[i] != 0){ 
					const uint32 uStart = PARTSIZE*i;
					const uint32 uEnd = (tempreqfile->GetFileSize()-1 <= (uStart+PARTSIZE-1)) ? (tempreqfile->GetFileSize()-1) : (uStart+PARTSIZE-1);
					if(((CPartFile*)tempreqfile)->IsComplete(uStart, uEnd) == false){
						// Swap source to the other list
						m_OtherNoNeeded_list.RemoveAt(pos);
                        m_OtherRequests_list.AddHead((CPartFile*)tempreqfile);

						if (thePrefs.GetVerbose())
							AddDebugLogLine(false, _T("Source '%s' has some part(s) available now for '%s'"), 
														 GetUserName(),
														 tempreqfile->GetFileName());					
						break; // [TPT] - Xman Bugfix
					}
				}
			}
		}	
	}
	// Maella end
}

void CUpDownClient::CreateStandartPackets(byte* data,uint32 togo, Requested_Block_Struct* currentblock, bool bFromPF){
	uint32 nPacketSize;
	CMemFile memfile((BYTE*)data,togo);
	if (togo > 10240) 
		nPacketSize = togo/(uint32)(togo/10240);
	else
		nPacketSize = togo;
	while (togo){
		if (togo < nPacketSize*2)
			nPacketSize = togo;
		ASSERT( nPacketSize );
		togo -= nPacketSize;

		uint32 statpos = (currentblock->EndOffset - togo) - nPacketSize;
		uint32 endpos = (currentblock->EndOffset - togo);
		if (IsUploadingToPeerCache())
		{
			if (m_pPCUpSocket == NULL){
				ASSERT(0);
				CString strError;
				strError.Format(_T("Failed to upload to PeerCache - missing socket; %s"), DbgGetClientInfo());
				throw strError;
			}
			USES_CONVERSION;
			CSafeMemFile dataHttp(10240);
			if (m_iHttpSendState == 0)
			{
				CKnownFile* srcfile = theApp.sharedfiles->GetFileByID(GetUploadFileID());
				CStringA str;
				str.AppendFormat("HTTP/1.0 206\r\n");
				str.AppendFormat("Content-Range: bytes %u-%u/%u\r\n", currentblock->StartOffset, currentblock->EndOffset - 1, srcfile->GetFileSize());
				str.AppendFormat("Content-Type: application/octet-stream\r\n");
				str.AppendFormat("Content-Length: %u\r\n", currentblock->EndOffset - currentblock->StartOffset);
				str.AppendFormat("Server: eMule/%s\r\n", T2CA(theApp.m_strCurVersionLong));
				str.AppendFormat("\r\n");
				dataHttp.Write((LPCSTR)str, str.GetLength());
				theStats.AddUpDataOverheadFileRequest(dataHttp.GetLength());

				m_iHttpSendState = 1;
				if (thePrefs.GetDebugClientTCPLevel() > 0){
					DebugSend("PeerCache-HTTP", this, (char*)GetUploadFileID());
					Debug(_T("  %hs\n"), str);
				}
			}
			dataHttp.Write(data, nPacketSize);
			data += nPacketSize;

			if (thePrefs.GetDebugClientTCPLevel() > 1){
				DebugSend("PeerCache-HTTP data", this, (char*)GetUploadFileID());
				Debug(_T("  Start=%u  End=%u  Size=%u\n"), statpos, endpos, nPacketSize);
			}

			UINT uRawPacketSize = dataHttp.GetLength();
			LPBYTE pRawPacketData = dataHttp.Detach();
			CRawPacket* packet = new CRawPacket((char*)pRawPacketData, uRawPacketSize, bFromPF);
			m_pPCUpSocket->SendPacket(packet, true, false, nPacketSize);
			free(pRawPacketData);
		}
// [TPT] - WebCache 	
// yonatan http start //////////////////////////////////////////////////////////////////////////
		else if (IsUploadingToWebCache())
		{
			if (m_pWCUpSocket == NULL){
				ASSERT(0);
				CString strError;
				strError.Format(_T("Failed to upload to WebCache - missing socket; %s"), DbgGetClientInfo());
				throw strError;
			}
			USES_CONVERSION;
			CSafeMemFile dataHttp(10240);
			if (m_iHttpSendState == 0) // yonatan - not sure it's wise to use this (also used by PC).
			{
				CKnownFile* srcfile = theApp.sharedfiles->GetFileByID(GetUploadFileID());
				CStringA str;
				//str.AppendFormat("HTTP/1.1 200 OK\r\n"); // DFA
				str.AppendFormat("HTTP/1.0 200 OK\r\n");
				str.AppendFormat("Content-Length: %u\r\n", currentblock->EndOffset - currentblock->StartOffset);
				str.AppendFormat("Expires: Mon, 03 Sep 2007 01:23:45 GMT\r\n" ); // rolled-back to 1.1b code (possible bug w/soothsayers' proxy)
				str.AppendFormat("Cache-Control: public\r\n");
				str.AppendFormat("Cache-Control: no-transform\r\n");
				str.AppendFormat("Connection: keep-alive\r\nProxy-Connection: keep-alive\r\n");
				str.AppendFormat("Server: eMule/%s %s\r\n", T2CA(theApp.m_strCurVersionLong), T2CA(MOD_VERSION));
				str.AppendFormat("\r\n");
				dataHttp.Write((LPCSTR)str, str.GetLength());
				theStats.AddUpDataOverheadFileRequest(dataHttp.GetLength());

				m_iHttpSendState = 1;
				if (thePrefs.GetDebugClientTCPLevel() > 0){
					DebugSend("WebCache-HTTP", this, (char*)GetUploadFileID());
					Debug(_T("  %hs\n"), str);
				}
			}
			dataHttp.Write(data, nPacketSize);
			data += nPacketSize;

			if (thePrefs.GetDebugClientTCPLevel() > 1){
				DebugSend("WebCache-HTTP data", this, (char*)GetUploadFileID());
				Debug(_T("  Start=%u  End=%u  Size=%u\n"), statpos, endpos, nPacketSize);
			}

			UINT uRawPacketSize = dataHttp.GetLength();
			LPBYTE pRawPacketData = dataHttp.Detach();
			CRawPacket* packet = new CRawPacket((char*)pRawPacketData, uRawPacketSize, bFromPF);
			m_pWCUpSocket->SendPacket(packet, true, false, nPacketSize);
			free(pRawPacketData);
		}
// yonatan http end ////////////////////////////////////////////////////////////////////////////
// [TPT] - WebCache 
		else
		{
			Packet* packet = new Packet(OP_SENDINGPART,nPacketSize+24, OP_EDONKEYPROT, bFromPF);
			md4cpy(&packet->pBuffer[0],GetUploadFileID());
			PokeUInt32(&packet->pBuffer[16], statpos);
			PokeUInt32(&packet->pBuffer[20], endpos);
			
			memfile.Read(&packet->pBuffer[24],nPacketSize);

			if (thePrefs.GetDebugClientTCPLevel() > 0){
				DebugSend("OP__SendingPart", this, (char*)GetUploadFileID());
				Debug(_T("  Start=%u  End=%u  Size=%u\n"), statpos, endpos, nPacketSize);
			}
			// put packet directly on socket
			theStats.AddUpDataOverheadFileRequest(24);
			socket->SendPacket(packet,true,false, nPacketSize);
		}
	}
}

void CUpDownClient::CreatePackedPackets(byte* data,uint32 togo, Requested_Block_Struct* currentblock, bool bFromPF){
	BYTE* output = new BYTE[togo+300];
	uLongf newsize = togo+300;
	uint16 result = compress2(output,&newsize,data,togo,9);
	if (result != Z_OK || togo <= newsize){
		delete[] output;
		CreateStandartPackets(data,togo,currentblock,bFromPF);
		return;
	}
	CMemFile memfile(output,newsize);
    uint32 oldSize = togo;
	togo = newsize;
	uint32 nPacketSize;
    if (togo > 10240) 
        nPacketSize = togo/(uint32)(togo/10240);
    else
        nPacketSize = togo;
    
    uint32 totalPayloadSize = 0;

    while (togo){
		if (togo < nPacketSize*2)
			nPacketSize = togo;
		ASSERT( nPacketSize );
		togo -= nPacketSize;
		Packet* packet = new Packet(OP_COMPRESSEDPART,nPacketSize+24,OP_EMULEPROT,bFromPF);
		md4cpy(&packet->pBuffer[0],GetUploadFileID());
		uint32 statpos = currentblock->StartOffset;
		PokeUInt32(&packet->pBuffer[16], statpos);
		PokeUInt32(&packet->pBuffer[20], newsize);
		memfile.Read(&packet->pBuffer[24],nPacketSize);

		if (thePrefs.GetDebugClientTCPLevel() > 0){
			DebugSend("OP__CompressedPart", this, (char*)GetUploadFileID());
			Debug(_T("  Start=%u  BlockSize=%u  Size=%u\n"), statpos, newsize, nPacketSize);
		}
        // approximate payload size
        uint32 payloadSize = nPacketSize*oldSize/newsize;

        if(togo == 0 && totalPayloadSize+payloadSize < oldSize) {
            payloadSize = oldSize-totalPayloadSize;
        }
        totalPayloadSize += payloadSize;

        // put packet directly on socket
		theStats.AddUpDataOverheadFileRequest(24);
        socket->SendPacket(packet,true,false, payloadSize);
	}
	delete[] output;
}

void CUpDownClient::SetUploadFileID(CKnownFile* newreqfile)
{
	CKnownFile* oldreqfile;
	//We use the knownfilelist because we may have unshared the file..
	//But we always check the download list first because that person may have decided to redownload that file.
	//Which will replace the object in the knownfilelist if completed.
	if ((oldreqfile = theApp.downloadqueue->GetFileByID(requpfileid)) == NULL )
		oldreqfile = theApp.knownfiles->FindKnownFileByID(requpfileid);

	if (newreqfile == oldreqfile)
		return;

	// clear old status
	if (m_abyUpPartStatus) 
	{
		delete[] m_abyUpPartStatus;
		m_abyUpPartStatus = NULL;
	}
	m_nUpPartCount = 0;
	m_nUpCompleteSourcesCount= 0;

	if (newreqfile){		
		newreqfile->AddUploadingClient(this);
		md4cpy(requpfileid, newreqfile->GetFileHash());
	}
	else
		md4clr(requpfileid);

	if (oldreqfile)
		oldreqfile->RemoveUploadingClient(this);
}

void CUpDownClient::AddReqBlock(Requested_Block_Struct* reqblock)
{	
	if(GetUploadState() != US_UPLOADING) {
        if(thePrefs.GetLogUlDlEvents())
            AddDebugLogLine(DLP_LOW, false, _T("UploadClient: Client tried to add req block when not in upload slot! Prevented req blocks from being added. %s"), DbgGetClientInfo());
		delete reqblock;
        return;
    }

	for (POSITION pos = m_DoneBlocks_list.GetHeadPosition(); pos != 0; ){
		const Requested_Block_Struct* cur_reqblock = m_DoneBlocks_list.GetNext(pos);
		if (reqblock->StartOffset == cur_reqblock->StartOffset && reqblock->EndOffset == cur_reqblock->EndOffset){
			delete reqblock;
			return;
		}
	}
	for (POSITION pos = m_BlockRequests_queue.GetHeadPosition(); pos != 0; ){
		const Requested_Block_Struct* cur_reqblock = m_BlockRequests_queue.GetNext(pos);
		if (reqblock->StartOffset == cur_reqblock->StartOffset && reqblock->EndOffset == cur_reqblock->EndOffset){
			delete reqblock;
			return;
		}
	}

    m_BlockRequests_queue.AddTail(reqblock);
}

uint32 CUpDownClient::SendBlockData(){
	DWORD curTick = ::GetTickCount();

    uint64 sentBytesCompleteFile = 0;
    uint64 sentBytesPartFile = 0;
    uint64 sentBytesPayload = 0;
    
    if (GetFileUploadSocket() && (m_ePeerCacheUpState != PCUS_WAIT_CACHE_REPLY))
	{
		CEMSocket* s = GetFileUploadSocket();
		UINT uUpStatsPort;
        if (m_pPCUpSocket && IsUploadingToPeerCache())
		{
			uUpStatsPort = (UINT)-1;

            // Check if filedata has been sent via the normal socket since last call.
            uint64 sentBytesCompleteFileNormalSocket = socket->GetSentBytesCompleteFileSinceLastCallAndReset();
            uint64 sentBytesPartFileNormalSocket = socket->GetSentBytesPartFileSinceLastCallAndReset();

			if(thePrefs.GetVerbose() && (sentBytesCompleteFileNormalSocket + sentBytesPartFileNormalSocket > 0)) {
                AddDebugLogLine(false, _T("Sent file data via normal socket when in PC mode. Bytes: %I64i."), sentBytesCompleteFileNormalSocket + sentBytesPartFileNormalSocket);
			}
        }
		else
			uUpStatsPort = GetUserPort();

		// [TPT] - WebCache
		// Superlexx - 0.44a port attempt
		if(m_pWCUpSocket && IsUploadingToWebCache()) {
			uUpStatsPort = (UINT)-1;

            // Check if filedata has been sent via the normal socket since last call.
            uint64 sentBytesCompleteFileNormalSocket = socket->GetSentBytesCompleteFileSinceLastCallAndReset();
            uint64 sentBytesPartFileNormalSocket = socket->GetSentBytesPartFileSinceLastCallAndReset();

			if(thePrefs.GetVerbose() && (sentBytesCompleteFileNormalSocket + sentBytesPartFileNormalSocket > 0)) {
                AddDebugLogLine(false, _T("Sent file data via normal socket when in WC mode. Bytes: %I64i."), sentBytesCompleteFileNormalSocket + sentBytesPartFileNormalSocket);
			}
        }
		else
			uUpStatsPort = GetUserPort(); //<<0.45a

	    // Extended statistics information based on which client software and which port we sent this data to...
	    // This also updates the grand total for sent bytes, etc.  And where this data came from.
        sentBytesCompleteFile = s->GetSentBytesCompleteFileSinceLastCallAndReset();
        sentBytesPartFile = s->GetSentBytesPartFileSinceLastCallAndReset();
		thePrefs.Add2SessionTransferData(GetClientSoft(), uUpStatsPort, false, true, sentBytesCompleteFile, (IsFriend() && GetFriendSlot()));
		thePrefs.Add2SessionTransferData(GetClientSoft(), uUpStatsPort, true, true, sentBytesPartFile, (IsFriend() && GetFriendSlot()));

		// [TPT] - Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
		AddUploadRate(sentBytesCompleteFile + sentBytesPartFile);
		//m_nTransferredUp += sentBytesCompleteFile + sentBytesPartFile;
        	//credits->AddUploaded(sentBytesCompleteFile + sentBytesPartFile, GetIP());
		// Maella end

        sentBytesPayload = s->GetSentPayloadSinceLastCallAndReset();
        m_nCurQueueSessionPayloadUp += sentBytesPayload;

        if(theApp.uploadqueue->CheckForTimeOver(this)) {
            theApp.uploadqueue->RemoveFromUploadQueue(this, _T("Completed transfer"), CUpDownClient::USR_TIMEOVER, true); // [TPT] - Maella -Upload Stop Reason-
 	    SendOutOfPartReqsAndAddToWaitingQueue();			
	    return 0;	// [TPT] - Allow to know if client has been removed from uploadqueue
        } 
		else {
            // read blocks from file and put on socket
            if (CreateNextBlockPackage() == false)
				return 0;	// [TPT] - Allow to know if client has been removed from uploadqueue
        }
    }  

    // Check if it's time to update the display.
    if (curTick-m_lastRefreshedULDisplay > MINWAIT_BEFORE_ULDISPLAY_WINDOWUPDATE+(uint32)(rand()*800/RAND_MAX)) {
        // Update display
        theApp.emuledlg->transferwnd->uploadlistctrl.RefreshClient(this);
        theApp.emuledlg->transferwnd->clientlistctrl.RefreshClient(this);
        m_lastRefreshedULDisplay = curTick;
    }

    return sentBytesCompleteFile + sentBytesPartFile;
}

void CUpDownClient::SendOutOfPartReqsAndAddToWaitingQueue()
{
	//OP_OUTOFPARTREQS will tell the downloading client to go back to OnQueue..
	//The main reason for this is that if we put the client back on queue and it goes
	//back to the upload before the socket times out... We get a situation where the
	//downloader thinks it already sent the requested blocks and the uploader thinks
	//the downloader didn't send any request blocks. Then the connection times out..
	//I did some tests with eDonkey also and it seems to work well with them also..
	//[TPT]
	if(socket != NULL){
		if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__OutOfPartReqs", this);
		Packet* pPacket = new Packet(OP_OUTOFPARTREQS, 0);
		theStats.AddUpDataOverheadFileRequest(pPacket->size);
		socket->SendPacket(pPacket, true, true);
		m_fSentOutOfPartReqs = 1;
    	theApp.uploadqueue->AddClientToQueue(this, true);
	}
}


// [TPT] - Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
uint32 CUpDownClient::GetUploadDatarate(uint32 samples) const 
{
	uint32 nUpDatarate = 0;
    if(m_upHistory_list.GetSize() > 1 && samples >= 1) // [TPT] - Xman Bugfix
	{
		// Retieve the location of the n previous sample
		POSITION pos = m_upHistory_list.FindIndex(samples);
		if(pos == NULL)
		{
			pos = m_upHistory_list.GetTailPosition();
		}

        const TransferredData& latestSample = m_upHistory_list.GetHead();
        const TransferredData& oldestSample = m_upHistory_list.GetAt(pos);
		const uint32 deltaTime = latestSample.timeStamp - oldestSample.timeStamp;
		const uint32 deltaByte = latestSample.dataLength - oldestSample.dataLength;

		nUpDatarate = (deltaTime > 0) ? (1000 * deltaByte / deltaTime) : 0;   // [bytes/s]
    }

    return nUpDatarate;
}

void CUpDownClient::CompUploadRate()
{
	// Add new sample
	TransferredData newSample;
	newSample.dataLength = m_nUpDatarateMeasure;
	newSample.timeStamp  = ::GetTickCount();
	m_upHistory_list.AddHead(newSample);

	// Keep up to 11 samples (=> 10 seconds)
	// Keep up to 21 samples (=> 20 seconds)
	while(m_upHistory_list.GetSize() > 21) // [TPT] Pawcio Bandwith
	{ 
		m_upHistory_list.RemoveTail();
	}

	if(m_upHistory_list.GetSize() > 1) // [TPT] - Xman Bugfix
	{
		// Compute datarate (=> display)
		POSITION pos = m_upHistory_list.FindIndex(thePrefs.GetDatarateSamples());
		if(pos == NULL)
		{
			pos = m_upHistory_list.GetTailPosition();
		}
		TransferredData& oldestSample = m_upHistory_list.GetAt(pos);
		uint32 deltaTime = newSample.timeStamp - oldestSample.timeStamp;
		uint32 deltaByte = newSample.dataLength - oldestSample.dataLength;
		m_nUpDatarate = (deltaTime > 0) ? (1000 * deltaByte / deltaTime) : 0;   // [bytes/s]
	}

	// Check and then refresh GUI
	m_displayUpDatarateCounter++;
	if(m_displayUpDatarateCounter >= (100/TIMER_PERIOD)*DISPLAY_REFRESH) // && GetUploadState() == US_UPLOADING){ => already in upload list
	{ 
		m_displayUpDatarateCounter = 0;
		theApp.emuledlg->transferwnd->uploadlistctrl.RefreshClient(this);
		theApp.emuledlg->transferwnd->clientlistctrl.RefreshClient(this);
	}
}
// Maella end

/**
 * See description for CEMSocket::TruncateQueues().
 */
void CUpDownClient::FlushSendBlocks(){ // call this when you stop upload, or the socket might be not able to send
    if (socket)      //socket may be NULL...
        socket->TruncateQueues();
}

void CUpDownClient::SendHashsetPacket(char* forfileid){
	CKnownFile* file = theApp.sharedfiles->GetFileByID((uchar*)forfileid);
	if (!file){
		CheckFailedFileIdReqs((uchar*)forfileid);
		throw GetResString(IDS_ERR_REQ_FNF) + _T(" (SendHashsetPacket)");
	}

	CSafeMemFile data(1024);
	data.WriteHash16(file->GetFileHash());
	UINT parts = file->GetHashCount();
	data.WriteUInt16(parts);
	for (UINT i = 0; i < parts; i++)
		data.WriteHash16(file->GetPartHash(i));
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__HashSetAnswer", this, forfileid);
	Packet* packet = new Packet(&data);
	packet->opcode = OP_HASHSETANSWER;
	theStats.AddUpDataOverheadFileRequest(packet->size);
	socket->SendPacket(packet,true,true);
}

void CUpDownClient::ClearUploadBlockRequests()
{
	FlushSendBlocks();

	for (POSITION pos = m_BlockRequests_queue.GetHeadPosition();pos != 0;)
		delete m_BlockRequests_queue.GetNext(pos);
	m_BlockRequests_queue.RemoveAll();
	
	for (POSITION pos = m_DoneBlocks_list.GetHeadPosition();pos != 0;)
		delete m_DoneBlocks_list.GetNext(pos);
	m_DoneBlocks_list.RemoveAll();
}

void CUpDownClient::SendRankingInfo(){
	if (!ExtProtocolAvailable())
		return;
//	uint16 nRank = theApp.uploadqueue->GetWaitingPosition(this);
	uint16 nRank = IsSnafu() ? 1+(rand()%100):theApp.uploadqueue->GetWaitingPosition(this);// [TPT] - eWombat SNAFU v2
	if (!nRank)
		return;
	Packet* packet = new Packet(OP_QUEUERANKING,12,OP_EMULEPROT);
	PokeUInt16(packet->pBuffer+0, nRank);
	MEMZERO(packet->pBuffer+2, 10);
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__QueueRank", this);
	theStats.AddUpDataOverheadFileRequest(packet->size);
	socket->SendPacket(packet,true,true);
}

void CUpDownClient::SendCommentInfo(/*const*/ CKnownFile *file)
{
	// [TPT] - SLUGFILLER: showComments - m_bCommentDirty only applies to the upload file
	if (file == NULL || !ExtProtocolAvailable() || m_byAcceptCommentVer < 1)
		return;
	if (file != theApp.sharedfiles->GetFileByID(requpfileid)){
		if(m_byAcceptCommentVer < 2)	// not supported by remote user
			return;
	} else {
		if (!m_bCommentDirty)	// only send upfile comment once
		return;
	m_bCommentDirty = false;
	}
	// [TPT] - SLUGFILLER: showComments

	uint8 rating = file->GetFileRating();
	const CString& desc = file->GetFileComment();
	if (file->GetFileRating() == 0 && desc.IsEmpty())
		return;

	CSafeMemFile data(256);
	// [TPT] - SLUGFILLER: showComments - send filehash
	if(m_byAcceptCommentVer >= 2)
		data.WriteHash16(file->GetFileHash());
	// [TPT] - SLUGFILLER: showComments
	data.WriteUInt8(rating);
	data.WriteLongString(desc, GetUnicodeSupport());
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__FileDesc", this, (char*)file->GetFileHash());
	Packet *packet = new Packet(&data,OP_EMULEPROT);
	packet->opcode = OP_FILEDESC;
	theStats.AddUpDataOverheadFileRequest(packet->size);
	socket->SendPacket(packet,true);
}

void  CUpDownClient::AddRequestCount(const uchar* fileid)
{
	for (POSITION pos = m_RequestedFiles_list.GetHeadPosition(); pos != 0; ){
		Requested_File_Struct* cur_struct = m_RequestedFiles_list.GetNext(pos);
		if (!md4cmp(cur_struct->fileid,fileid)){
			if (::GetTickCount() - cur_struct->lastasked < MIN_REQUESTTIME && !GetFriendSlot()){ 
				if (GetDownloadState() != DS_DOWNLOADING)
					cur_struct->badrequests++;
				if (cur_struct->badrequests == BADCLIENTBAN){
					Ban();
				}
			}
			else{
				if (cur_struct->badrequests)
					cur_struct->badrequests--;
			}
			cur_struct->lastasked = ::GetTickCount();
			return;
		}
	}
	Requested_File_Struct* new_struct = new Requested_File_Struct;
	md4cpy(new_struct->fileid,fileid);
	new_struct->lastasked = ::GetTickCount();
	new_struct->badrequests = 0;
	m_RequestedFiles_list.AddHead(new_struct);
}


// [TPT] - eWombat SNAFU v2
/*
void  CUpDownClient::UnBan(){
	theApp.clientlist->AddTrackClient(this);
	theApp.clientlist->RemoveBannedClient(GetIP());
	SetUploadState(US_NONE);
	ClearWaitStartTime();
	theApp.emuledlg->transferwnd->ShowQueueCount(theApp.uploadqueue->GetWaitingUserCount());
	for (POSITION pos = m_RequestedFiles_list.GetHeadPosition();pos != 0;)
	{
		Requested_File_Struct* cur_struct = m_RequestedFiles_list.GetNext(pos);
		cur_struct->badrequests = 0;
		cur_struct->lastasked = 0;	
	}
}

void CUpDownClient::Ban(LPCTSTR pszReason)
{
// [TPT] - WebCache	
#ifdef _DEBUG
	if(SupportsWebCache()) return; // for testing - don't ban webcache clients
#endif
	theApp.clientlist->AddTrackClient(this);
	if (!IsBanned()){
		if (thePrefs.GetLogBannedClients())
			AddDebugLogLine(false,_T("Banned: %s; %s"), pszReason==NULL ? _T("Aggressive behaviour") : pszReason, DbgGetClientInfo());
	}
#ifdef _DEBUG
	else{
		if (thePrefs.GetLogBannedClients())
			AddDebugLogLine(false,_T("Banned: (refreshed): %s; %s"), pszReason==NULL ? _T("Aggressive behaviour") : pszReason, DbgGetClientInfo());
	}
#endif
	theApp.clientlist->AddBannedClient(GetIP());
	SetUploadState(US_BANNED);
	theApp.emuledlg->transferwnd->ShowQueueCount(theApp.uploadqueue->GetWaitingUserCount());
	theApp.emuledlg->transferwnd->queuelistctrl.RefreshClient(this);
	if (socket != NULL && socket->IsConnected())
		socket->ShutDown(SD_RECEIVE); // let the socket timeout, since we dont want to risk to delete the client right now. This isnt acutally perfect, could be changed later
}
*/

// [TPT] - SUQWT
// Moonlight: SUQWT - Compare linear time instead of time indexes to avoid overflow-induced false positives.//Morph - added by AndCycle, Moonlight's Save Upload Queue Wait Time (MSUQWT)
sint64 CUpDownClient::GetWaitStartTime() const
{
	if (credits == NULL){
		ASSERT ( false );
		return 0;
	}
	
	sint64 dwResult = credits->GetSecureWaitStartTime(GetIP());
	uint32 now = ::GetTickCount();
	if ( dwResult > now) { 
		dwResult = now - 1;
	}
	if (IsDownloading() && (dwResult > m_dwUploadTime)) {
		//this happens only if two clients with invalid securehash are in the queue - if at all
		dwResult = m_dwUploadTime-1;
		if (thePrefs.GetVerbose())
			DEBUG_ONLY(AddDebugLogLine(false,_T("Warning: CUpDownClient::GetWaitStartTime() waittime Collision (%s)"),GetUserName()));
	}
	return dwResult;
}
// [TPT] - SUQWT

void CUpDownClient::SetWaitStartTime(){
	if (credits == NULL){
		return;
	}
	credits->SetSecWaitStartTime(GetIP());
	m_nSUITimer=::GetTickCount(); // eWombat [SUI][TPT]
}

void CUpDownClient::ClearWaitStartTime(){
	if (credits == NULL){
		return;
	}
	credits->ClearWaitStartTime();
}

bool CUpDownClient::GetFriendSlot() const
{
	if (credits && theApp.clientcredits->CryptoAvailable()){
		switch(credits->GetCurrentIdentState(GetIP())){
			case IS_IDFAILED:
			case IS_IDNEEDED:
			case IS_IDBADGUY:
				return false;
		}
	}
	return m_bFriendSlot;
}

CClientReqSocket* CUpDownClient::GetFileUploadSocket(bool log) { // [TPT] - Fix
    if(m_pPCUpSocket && (IsUploadingToPeerCache() || m_ePeerCacheUpState == PCUS_WAIT_CACHE_REPLY)) {
        if(thePrefs.GetVerbose() && log)
            AddDebugLogLine(false, _T("%s got peercache socket."), DbgGetClientInfo());

        return m_pPCUpSocket;
    // [TPT] - WebCache 
    } else if( m_pWCUpSocket && IsUploadingToWebCache() )
    {
    	if(thePrefs.GetVerbose() && log)
            AddDebugLogLine(false, _T("%s got webcache socket."), DbgGetClientInfo());

        return m_pWCUpSocket;
    }
	// [TPT] - WebCache 
    else {
        if(thePrefs.GetVerbose() && log)
            AddDebugLogLine(false, _T("%s got normal socket."), DbgGetClientInfo());

        return socket;
    }
}

// [TPT] - netfinity: Anti HideOS
bool CUpDownClient::IsPartialSource()
{
	CKnownFile* currequpfile = theApp.sharedfiles->GetFileByID(requpfileid);
	return (currequpfile == reqfile);
}
// [TPT] - netfinity: Anti HideOS