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
#include <io.h>

#include "emule.h"
#include "DownloadQueue.h"
#include "UpDownClient.h"
#include "PartFile.h"
#include "PartFileIndex.h"	// [TPT] - SLUGFILLER: indexPartFiles
#include "ed2kLink.h"
#include "SearchList.h"
#include "BandWidthControl.h" // [TPT]
#include "ClientList.h"
#include "Statistics.h"
#include "SharedFileList.h"
#include "OtherFunctions.h"
#include "SafeFile.h"
#include "Sockets.h"
#include "ServerList.h"
#include "Server.h"
#include "Packets.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "kademlia/utils/uint128.h"
#include "ipfilter.h"
#include "emuledlg.h"
#include "TransferWnd.h"
#include "TaskbarNotifier.h"
#include "MenuCmds.h"
#include "Log.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif


CDownloadQueue::CDownloadQueue()
{
	filesrdy = 0;
	// [TPT]
	//datarate = 0;
	cur_udpserver = 0;
	lastfile = 0;
	lastcheckdiskspacetime = 0;	// SLUGFILLER: checkDiskspace
	lastudpsearchtime = 0;
	lastudpstattime = 0;
	SetLastKademliaFileRequest();
	udcounter = 0;

	m_iSearchedServers = 0;
	//m_datarateMS=0;
	m_nUDPFileReasks = 0;
	m_nFailedUDPFileReasks = 0;
	// Maella -Overhead compensation (pseudo full download rate control)-
	m_lastProcessTime = ::GetTickCount();
	m_lastOverallReceivedBytes = 0;
	m_lastReceivedBytes = 0;
	m_nDownloadSlopeControl = 0;
	// Maella end
	m_dwNextTCPSrcReq = 0;
	m_cRequestsSentToServer = 0;
	partfileindex = new CPartFileIndex();	// [TPT] - SLUGFILLER: indexPartFiles

    m_dwLastA4AFtime = 0; // ZZ:DownloadManager
    
    // [TPT] - khaos::categorymod+
	m_iLastLinkQueuedTick = 0;
	m_bBusyPurgingLinks = false;
	m_ED2KLinkQueue.RemoveAll();
	// [TPT] - khaos::categorymod-

	// [TPT] - quick start
	quickflag = 0;
	quickflags = 0;	
	// [TPT] - quick start

	//[TPT] - Unlimited upload with no downloads
	unlimitedFlag = false;
	upTemp = -1;
	timeAllowUnlimitedUP = ::GetTickCount();
}

void CDownloadQueue::AddPartFilesToShare()
{
	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; )
	{
		CPartFile* cur_file = filelist.GetNext(pos);
		if (cur_file->GetStatus(true) == PS_READY)
			theApp.sharedfiles->SafeAddKFile(cur_file, true);
	}
}

void CDownloadQueue::Init(){
	// find all part files, read & hash them if needed and store into a list
	CFileFind ff;
	int count = 0;

	CString searchPath(thePrefs.GetTempDir());
	searchPath += "\\*.part.met";

	//check all part.met files
	bool end = !ff.FindFile(searchPath, 0);
	while (!end){
		end = !ff.FindNextFile();
		if (ff.IsDirectory())
			continue;
		CPartFile* toadd = new CPartFile();
		if (toadd->LoadPartFile(thePrefs.GetTempDir(),ff.GetFileName().GetBuffer())){
			count++;
			filelist.AddTail(toadd);			// to downloadqueue
			if (toadd->GetStatus(true) == PS_READY)
				theApp.sharedfiles->SafeAddKFile(toadd); // part files are always shared files
			theApp.emuledlg->transferwnd->downloadlistctrl.AddFile(toadd);// show in downloadwindow
			partfileindex->AddPartFile(toadd->GetPartMetFileName(), toadd->GetFileName(), toadd->GetFileSize(), toadd->GetFileHash(), toadd->GetHashset(), (toadd->GetAICHHashset() && toadd->GetAICHHashset()->HasValidMasterHash() && (toadd->GetAICHHashset()->GetStatus() == AICH_VERIFIED))?toadd->GetAICHHashset()->GetMasterHash().GetString():_T(""), false);	// SLUGFILLER: indexPartFiles
		}
		// [TPT] - SLUGFILLER: indexPartFiles
		else {
			delete toadd;
			partfileindex->RecoverPartFile(ff.GetFileName());
		}
		// [TPT] - SLUGFILLER: indexPartFiles
	}
	ff.Close();

	//try recovering any part.met files
	searchPath += ".backup";
	end = !ff.FindFile(searchPath, 0);
	while (!end){
		end = !ff.FindNextFile();
		if (ff.IsDirectory())
			continue;
		CPartFile* toadd = new CPartFile();
		if (toadd->LoadPartFile(thePrefs.GetTempDir(),ff.GetFileName().GetBuffer())){
			toadd->SavePartFile(); // resave backup
			count++;
			filelist.AddTail(toadd);			// to downloadqueue
			if (toadd->GetStatus(true) == PS_READY)
				theApp.sharedfiles->SafeAddKFile(toadd); // part files are always shared files
			theApp.emuledlg->transferwnd->downloadlistctrl.AddFile(toadd);// show in downloadwindow
			partfileindex->AddPartFile(toadd->GetPartMetFileName(), toadd->GetFileName(), toadd->GetFileSize(), toadd->GetFileHash(), toadd->GetHashset(), (toadd->GetAICHHashset() && toadd->GetAICHHashset()->HasValidMasterHash() && (toadd->GetAICHHashset()->GetStatus() == AICH_VERIFIED))?toadd->GetAICHHashset()->GetMasterHash().GetString():_T(""), false);	// SLUGFILLER: indexPartFiles

			AddLogLine(false, GetResString(IDS_RECOVERED_PARTMET), toadd->GetFileName());
		}
		else {
			delete toadd;
		}
	}
	ff.Close();
	partfileindex->RecoverAllPartFiles();	// [TPT] - SLUGFILLER: indexPartFiles

	if(count == 0) {
		AddLogLine(false,GetResString(IDS_NOPARTSFOUND));
	} else {
		AddLogLine(false,GetResString(IDS_FOUNDPARTS),count);
		SortByPriority();
		CheckDiskspace();	// SLUGFILLER: checkDiskspace
	}
	VERIFY( m_srcwnd.CreateEx(0, AfxRegisterWndClass(0), _T("eMule Async DNS Resolve Socket Wnd #2"), WS_OVERLAPPED, 0, 0, 0, 0, NULL, NULL));

	// [TPT] - SLUGFILLER: indexPartFiles remove - Just export? Text file? You must be joking.
}

CDownloadQueue::~CDownloadQueue(){
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;)
		delete filelist.GetNext(pos);
	m_srcwnd.DestroyWindow(); // just to avoid a MFC warning
	delete partfileindex; // [TPT] - SLUGFILLER: indexPartFiles
}

// [TPT] - khaos::categorymod+
// New Param: uint16 useOrder (Def: 0)
void CDownloadQueue::AddSearchToDownload(CSearchFile* toadd, uint8 paused, uint8 cat, uint16 useOrder){
	//[TPT] - SLUGFILLER: searchCatch
	CPartFile* newfile;
	if (!IsFileExisting(toadd->GetFileHash())){
		newfile = new CPartFile(toadd);
		if (newfile->GetStatus() == PS_ERROR)
		{
			delete newfile;
			return;
		}
		newfile->SetCategory(cat);
		newfile->SetCatResumeOrder(useOrder); // Added
		if (paused == 2)
			paused = (uint8)thePrefs.AddNewFilesPaused();
		AddDownload(newfile, (paused==1));
	}
	else if ((newfile = GetFileByID(toadd->GetFileHash())) == NULL)
		return;
	//[TPT] - SLUGFILLER: searchCatch	

	// If the search result is from OP_GLOBSEARCHRES there may also be a source
	if (toadd->GetClientID() && toadd->GetClientPort()){
		CSafeMemFile sources(1+4+2);
		try{
			sources.WriteUInt8(1);
			sources.WriteUInt32(toadd->GetClientID());
			sources.WriteUInt16(toadd->GetClientPort());
		    sources.SeekToBegin();
		    newfile->AddSources(&sources, toadd->GetClientServerIP(), toadd->GetClientServerPort());
		}
		catch(CFileException* error){
			ASSERT(0);
			error->Delete();
		}
	}

	// Add more sources which were found via global UDP search
	const CSimpleArray<CSearchFile::SClient>& aClients = toadd->GetClients();
	for (int i = 0; i < aClients.GetSize(); i++){
		CSafeMemFile sources(1+4+2);
		try{
			sources.WriteUInt8(1);
			sources.WriteUInt32(aClients[i].m_nIP);
			sources.WriteUInt16(aClients[i].m_nPort);
		    sources.SeekToBegin();
			newfile->AddSources(&sources,aClients[i].m_nServerIP, aClients[i].m_nServerPort);
	    }
		catch(CFileException* error){
			ASSERT(0);
			error->Delete();
			break;
		}
	}
	// [TPT] - itsonlyme: cacheUDPsearchResults
	const CSimpleArray<CSearchFile::SServer>& aServers = toadd->GetServers();
	for (int i = 0; i < aServers.GetSize(); i++){
		CPartFile::SServer tmpServer(aServers[i].m_nIP, aServers[i].m_nPort);
		tmpServer.m_uAvail = aServers[i].m_uAvail;
		newfile->AddAvailServer(tmpServer);
		Debug(_T("Caching server with %i sources for %s"), aServers[i].m_uAvail, newfile->GetFileName());
	}
	// [TPT] - itsonlyme: cacheUDPsearchResults
}


// New Param: uint16 useOrder (Def: 0)
void CDownloadQueue::AddSearchToDownload(CString link, uint8 paused, uint8 cat, uint16 useOrder){
	CPartFile* newfile = new CPartFile(link);
	if (newfile->GetStatus() == PS_ERROR){
		delete newfile;
		return;
	}
	newfile->SetCategory(cat);
	newfile->SetCatResumeOrder(useOrder); // Added
	if (paused == 2)
		paused = (uint8)thePrefs.AddNewFilesPaused();
	AddDownload(newfile, (paused==1));
}

void CDownloadQueue::StartNextFileIfPrefs(int cat) {	
	// [TPT] - ZZ:DownloadManager
	// if download in lp category is active, it will start next file in this category
	// although startnextfile isn't selected, because I suppose you want to download all files
	// in this category
	if (thePrefs.GetCategory(cat) && 
		thePrefs.GetCategory(cat)->downloadInLinealPriorityOrder && 
		(theApp.downloadqueue->GetCategoryFileCount(cat) > 0))
		if (StartNextFile(cat, false))
			return;
	// [TPT] - ZZ:DownloadManager
    if (thePrefs.StartNextFile())
		StartNextFile((thePrefs.StartNextFile() > 1?cat:-1), (thePrefs.StartNextFile()!=3));
}

bool CDownloadQueue::StartNextFile(int cat, bool force){

	CPartFile*  pfile = NULL;
	CPartFile* cur_file ;
	POSITION pos;

	//[TPT] - Commented LP
	//1) we check along all files within our category.
	//2) If not, we check along all files
	if (cat != -1) {
        // try to find in specified category
		for (pos = filelist.GetHeadPosition();pos != 0;)
		{
			cur_file = filelist.GetNext(pos);
			if (cur_file->GetStatus()==PS_PAUSED &&
				cur_file->GetCategory()==cat &&
                CPartFile::RightFileHasHigherPrio(pfile, cur_file)) 
			{
						pfile = cur_file; 					
			}
		}
		if (pfile == NULL && !force)
			return false;
	}

	if(cat == -1 || pfile == NULL && force) {
	    for (pos = filelist.GetHeadPosition();pos != 0;){
		    cur_file = filelist.GetNext(pos);
		    if (cur_file->GetStatus() == PS_PAUSED &&
                CPartFile::RightFileHasHigherPrio(pfile, cur_file))
		    {
                // pick first found matching file, since they are sorted in prio order with most important file first.
			    pfile = cur_file;
		    }
	    }
    }
	if (pfile) 
	{
		pfile->ResumeFile();
		return true;
	}

	return false;
}
// [TPT] - end


// This function is used for the category commands Stop Last and Pause Last.
// This is a new function.
void CDownloadQueue::StopPauseLastFile(int Mode, int Category)
{
	CPartFile*  pfile = NULL;
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;)
	{
		CPartFile* cur_file = filelist.GetNext(pos);
		if ((cur_file->GetStatus() < 4) && (cur_file->GetCategory() == Category || Category == -1))
		{
			if (!pfile)
				pfile = cur_file;
			else
			{
				if (cur_file->GetCatResumeOrder() > pfile->GetCatResumeOrder())
					pfile = cur_file;
				else if (cur_file->GetCatResumeOrder() == pfile->GetCatResumeOrder() && (cur_file->GetDownPriority() < pfile->GetDownPriority()))
					pfile = cur_file;
			}
		}
	}
	if (pfile)
	{
		Mode == MP_STOP ? pfile->StopFile() : pfile->PauseFile();
			}
}

// This function returns the highest linear priority in a given category.
// It can be used to automatically assign a linear priority to a partfile
// when it is added...  Or maybe it will have other uses in the future.
uint16 CDownloadQueue::GetMaxCatResumeOrder(uint8 iCategory /* = 0*/)
{
	uint16		max   = 0;
	
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;)
	{
		CPartFile* cur_file = filelist.GetNext(pos);
		if (cur_file->GetCategory() == iCategory && cur_file->GetCatResumeOrder() > max)
			max = cur_file->GetCatResumeOrder();
		}

	return max;
}

// This function has been modified in order
// to accomodate the category selection.
// NEW PARAM:  bool AllocatedLink = false by default
// [TPT] - khaos::categorymod added new param AddedPaused(bool)
void CDownloadQueue::AddFileLinkToDownload(CED2KFileLink* pLink, bool AllocatedLink, bool SkipQueue, bool AddedPaused)
{
	if (thePrefs.SelectCatForNewDL() && !SkipQueue)
	{
		m_ED2KLinkQueue.AddTail(pLink);
		m_iLastLinkQueuedTick = GetTickCount();
			return;
	}

	int useCat = pLink->GetCat();

	// [TPT] : Change selection order.
	if (thePrefs.UseActiveCatForLinks() && useCat == -1)
		useCat = theApp.emuledlg->transferwnd->GetActiveCategory();
	else if (thePrefs.UseAutoCat() && useCat == -1)
		useCat = theApp.downloadqueue->GetAutoCat(CString(pLink->GetName()), (ULONG)pLink->GetSize());
	else if (useCat == -1)
		useCat = 0;

	// Just in case...
	if (m_ED2KLinkQueue.GetCount() && !thePrefs.SelectCatForNewDL()) PurgeED2KLinkQueue();
	m_iLastLinkQueuedTick = 0;
	// khaos::categorymod-

	CPartFile* pNewFile = new CPartFile(pLink);

	if (pNewFile->GetStatus() == PS_ERROR)
	{
		delete pNewFile;
		pNewFile=NULL;
	}
	else
	{
		// khaos::categorymod+ Pass useCat instead of cat and autoset linear priority.
		pNewFile->SetCategory(useCat);
		if (thePrefs.AutoSetResumeOrder()) pNewFile->SetCatResumeOrder(GetMaxCatResumeOrder(useCat)+1); // Morph

		// khaos::categorymod-
		AddDownload(pNewFile, AddedPaused);
	}

	CPartFile* partfile = pNewFile;
	if (partfile == NULL)
		partfile = GetFileByID(pLink->GetHashKey());
	if (partfile)
	{
		if (pLink->HasValidSources())
			partfile->AddClientSources(pLink->SourcesList,1);
		if (pLink->HasValidAICHHash() ){
			if ( !(partfile->GetAICHHashset()->HasValidMasterHash() && partfile->GetAICHHashset()->GetMasterHash() == pLink->GetAICHHash())){
				partfile->GetAICHHashset()->SetMasterHash(pLink->GetAICHHash(), AICH_VERIFIED);
				partfile->GetAICHHashset()->FreeHashSet();
			}
		}
	}

	// [TPT] - WebCache	
	// Superlexx - IFP
	if (!theApp.sharedfiles->GetFileByID(pLink->GetHashKey())	// not already in the shared files list
		&& partfile							// valid pointer
		&& !partfile->hashsetneeded			// hash set not needed
		&& thePrefs.IsWebCacheDownloadEnabled()			// webcache downloading on
		&& partfile->GetStatus() == PS_EMPTY)	// file not stopped or paused
	{
			partfile->SetStatus(PS_READY);
		theApp.sharedfiles->SafeAddKFile(partfile);		
	}
	// [TPT] - WebCache	

	if (pLink->HasHostnameSources())
	{
		POSITION pos = pLink->m_HostnameSourcesList.GetHeadPosition();
		while (pos != NULL)
		{
			const SUnresolvedHostname* pUnresHost = pLink->m_HostnameSourcesList.GetNext(pos);
			m_srcwnd.AddToResolve(pLink->GetHashKey(), pUnresHost->strHostname, pUnresHost->nPort, pUnresHost->strURL);
		}
	}
	
	// khaos::categorymod+ Deallocate memory, because if we've gotten here,
	// this link wasn't added to the queue and therefore there's no reason to
	// not delete it.
	if (AllocatedLink) {
		delete pLink;
		pLink = NULL;
	}
	// khaos::categorymod-
}

// khaos::categorymod+ New function, used to add all of the
// ED2K links on the link queue to the downloads.  This is
// called when no new links have been added to the download
// list for half a second, or when there are queued links
// and the user has disabled the SelectCatForLinks feature.
bool CDownloadQueue::PurgeED2KLinkQueue()
{
	if (m_ED2KLinkQueue.IsEmpty()) return false;
	
	m_bBusyPurgingLinks = true;

	uint8	useCat;
	int		addedFiles = 0;
	bool	bCreatedNewCat = false;

	if (thePrefs.SelectCatForNewDL())
	{
		CSelCategoryDlg* getCatDlg = new CSelCategoryDlg((CWnd*)theApp.emuledlg);
		int nResult = getCatDlg->DoModal();

		if (nResult != IDOK)
		{
			delete getCatDlg;
			m_ED2KLinkQueue.RemoveAll();
			m_bBusyPurgingLinks = false;
			m_iLastLinkQueuedTick = 0;
			return false;
		}
		
		// Returns 0 on 'Cancel', otherwise it returns the selected category
		// or the index of a newly created category.  Users can opt to add the
		// links into a new category.
		useCat = getCatDlg->GetInput();
		bCreatedNewCat = getCatDlg->CreatedNewCat();
		delete getCatDlg;
	}
	else if (thePrefs.UseActiveCatForLinks())
		useCat = theApp.emuledlg->transferwnd->GetActiveCategory();
	else
		useCat = 0;

	for (POSITION pos = m_ED2KLinkQueue.GetHeadPosition(); pos != 0;)
	{
		CED2KFileLink*	pLink = m_ED2KLinkQueue.GetNext(pos);
		CPartFile*		pNewFile =	new CPartFile(pLink);
		
		if (pNewFile->GetStatus() == PS_ERROR)
		{
			delete pNewFile;
			pNewFile = NULL;
		}
		else
		{
			if (!thePrefs.SelectCatForNewDL() && thePrefs.UseAutoCat())
			{
				useCat = GetAutoCat(CString(pNewFile->GetFileName()), (ULONG)pNewFile->GetFileSize());
				if (!useCat && thePrefs.UseActiveCatForLinks())
					useCat = theApp.emuledlg->transferwnd->GetActiveCategory();
			}
			pNewFile->SetCategory(useCat);
			if (thePrefs.AutoSetResumeOrder()) 
				pNewFile->SetCatResumeOrder(GetMaxCatResumeOrder(useCat)+1);
			AddDownload(pNewFile,thePrefs.AddNewFilesPaused());
			addedFiles++;
		}

		CPartFile* partfile = pNewFile;
	if (partfile == NULL)
		partfile = GetFileByID(pLink->GetHashKey());
	if (partfile)
	{
		if (pLink->HasValidSources())
			partfile->AddClientSources(pLink->SourcesList,1);
			if (pLink->HasValidAICHHash() ){
				if ( !(partfile->GetAICHHashset()->HasValidMasterHash() && partfile->GetAICHHashset()->GetMasterHash() == pLink->GetAICHHash())){
					partfile->GetAICHHashset()->SetMasterHash(pLink->GetAICHHash(), AICH_VERIFIED);
					partfile->GetAICHHashset()->FreeHashSet();
				}
			}
	}
	    // [TPT] - MORPH START - Added by SiRoB, WebCache 1.2f
		if (!theApp.sharedfiles->GetFileByID(pLink->GetHashKey())	// not already in the shared files list
			&& partfile							// valid pointer
			&& !partfile->hashsetneeded			// hash set not needed
			&& thePrefs.IsWebCacheDownloadEnabled()			// webcache downloading on
			&& partfile->GetStatus() == PS_EMPTY)	// file not stopped or paused
		{
				partfile->SetStatus(PS_READY);
			theApp.sharedfiles->SafeAddKFile(partfile);
		}
		// [TPT] - MORPH END - Added by SiRoB, WebCache 1.2f

	if (pLink->HasHostnameSources())
	{
		POSITION pos = pLink->m_HostnameSourcesList.GetHeadPosition();
		while (pos != NULL)
		{
			const SUnresolvedHostname* pUnresHost = pLink->m_HostnameSourcesList.GetNext(pos);
			m_srcwnd.AddToResolve(pLink->GetHashKey(), pUnresHost->strHostname, pUnresHost->nPort, pUnresHost->strURL);
		}
	}		
		// We're done with this link.
		delete pLink;
		pLink = NULL;
	}
	
	m_ED2KLinkQueue.RemoveAll();

	// This bit of code will resume the number of files that the user specifies in preferences (Off by default)
	if (thePrefs.StartDLInEmptyCats() > 0 && bCreatedNewCat && thePrefs.AddNewFilesPaused())
		for (int i = 0; i < thePrefs.StartDLInEmptyCats(); i++)
			if (!StartNextFile(useCat)) break;

	m_bBusyPurgingLinks = false;
	m_iLastLinkQueuedTick = 0;
	return true;
}

// Returns statistics about a category's files' states.
void CDownloadQueue::GetCategoryFileCounts(uint8 iCategory, int cntFiles[])
{
	// cntFiles Array Indices:
	// 0 = Total
	// 1 = Transferring (Transferring Sources > 0)
	// 2 = Active (READY and EMPTY)
	// 3 = Unactive (PAUSED)
	// 4 = Complete (COMPLETE and COMPLETING)
	// 5 = Error (ERROR)
	// 6 = Other (Everything else...)
	for (int i = 0; i < 7; i++) cntFiles[i] = 0;

	for (POSITION pos = filelist.GetHeadPosition(); pos != 0;)
	{
		CPartFile* cur_file = filelist.GetNext(pos);
		if (cur_file->GetCategory() != iCategory) continue;

		cntFiles[0]++;
		if (cur_file->GetTransferringSrcCount() > 0) cntFiles[1]++;

		switch (cur_file->GetStatus(false))
		{
			case	PS_READY:
			case	PS_EMPTY:
				cntFiles[2]++;
				break;

			case	PS_PAUSED:
			//case	PS_INSUFFICIENT:
				cntFiles[3]++;
				break;

			case	PS_COMPLETING:
			case	PS_COMPLETE:
				cntFiles[4]++;
				break;

			case	PS_ERROR:
				cntFiles[5]++;
				break;
			
			case	PS_WAITINGFORHASH:
			case	PS_HASHING:
			case	PS_UNKNOWN:
				cntFiles[6]++;
				break;
	}
	}
}

// Returns the number of active files in a category.
uint16 CDownloadQueue::GetCatActiveFileCount(uint8 iCategory)
{
	uint16 iCount = 0;

	for (POSITION pos = filelist.GetHeadPosition(); pos != 0;)
	{
		CPartFile* cur_file = filelist.GetNext(pos);
		if (cur_file->GetCategory() != iCategory) continue;

		switch (cur_file->GetStatus(false))
		{
			case	PS_READY:
			case	PS_EMPTY:
			case	PS_COMPLETING:
				iCount++;
				break;
			default:
				break;
		}
	}

	return iCount;
}

// Returns the number of files in a category.
uint16 CDownloadQueue::GetCategoryFileCount(uint8 iCategory)
{
	uint16 iCount = 0;

	for (POSITION pos = filelist.GetHeadPosition(); pos != 0;)
	{
		CPartFile* cur_file = filelist.GetNext(pos);
		if (cur_file->GetCategory() == iCategory) iCount++;
	}

	return iCount;
}

// Returns the source count of the file with the highest available source count.
// nCat is optional and allows you to specify a certain category.
uint16 CDownloadQueue::GetHighestAvailableSourceCount(int nCat)
{
	uint16 nCount = 0;

	for (POSITION pos = filelist.GetHeadPosition(); pos != 0;)
	{
		CPartFile* curFile = filelist.GetNext(pos);
		if (nCount < curFile->GetAvailableSrcCount() && (nCat == -1 || curFile->GetCategory() == nCat))
			nCount = curFile->GetAvailableSrcCount();
	}

	return nCount;
}

// [TPT] - khaos::categorymod+ GetAutoCat returns a category index of a category
// that passes the filters.
// Idea by HoaX_69.
uint8 CDownloadQueue::GetAutoCat(CString sFullName, ULONG nFileSize)
{
	if (sFullName.IsEmpty())
		return 0;

	if (thePrefs.GetCatCount() <= 1)
		return 0;

	for (int ix = 1; ix < thePrefs.GetCatCount(); ix++)
	{ 
		Category_Struct* curCat = thePrefs.GetCategory(ix);
		if (!curCat->selectioncriteria.bAdvancedFilterMask && !curCat->selectioncriteria.bFileSize)
			continue;
		if (curCat->selectioncriteria.bAdvancedFilterMask && !ApplyFilterMask(sFullName, ix))
			continue;
		if (curCat->selectioncriteria.bFileSize && (nFileSize < curCat->viewfilters.nFSizeMin || (curCat->viewfilters.nFSizeMax == 0 || nFileSize > curCat->viewfilters.nFSizeMax)))
			continue;
		return ix;
	}

	return 0;
}

// Checks a part-file's "pretty filename" against a filter mask and returns
// true if it passes.  See read-me for details.
bool CDownloadQueue::ApplyFilterMask(CString sFullName, uint8 nCat)
{
	CString sFilterMask = thePrefs.GetCategory(nCat)->viewfilters.sAdvancedFilterMask;
	if (!sFilterMask.IsEmpty())
		sFilterMask.Trim();

	if (sFilterMask == _T(""))
		return false;

	sFullName.MakeLower();
	sFilterMask.MakeLower();

	if (sFilterMask.Left(1) == _T("<"))
	{
		bool bPassedGlobal[3];
		bPassedGlobal[0] = false;
		bPassedGlobal[1] = true;
		bPassedGlobal[2] = false;

		for (int i = 0; i < 3; i++)
	{
			int iStart = 0;
			switch (i)
			{
			case 0: iStart = sFilterMask.Find(_T("<all(")); break;
			case 1: iStart = sFilterMask.Find(_T("<any(")); break;
			case 2: iStart = sFilterMask.Find(_T("<none(")); break;
	}

			if (iStart == -1)
	{
				bPassedGlobal[i] = true; // We need to do this since not all criteria are needed in order to match the category.
				continue; // Skip this criteria block.
			}

			i !=2 ? (iStart += 5) : (iStart += 6);

			int iEnd = sFilterMask.Find(_T(")>"), iStart);
			int iLT = sFilterMask.Find(_T("<"), iStart);
			int iGT = sFilterMask.Find(_T(">"), iStart);

			if (iEnd == -1 || (iLT != -1 && iLT < iEnd) || iGT < iEnd)
		{
				if (thePrefs.GetVerbose())
					AddDebugLogLine(false, _T("Category '%s' has invalid Category Mask String."), thePrefs.GetCategory(nCat)->title);
				break; // Move on to next category.
			}
			if (iStart == iEnd)
			{
				bPassedGlobal[i] = true; // Just because this criteria block is empty doesn't mean the mask should fail.
				continue; // Skip this criteria block.
			}

			CString sSegment = sFilterMask.Mid(iStart, iEnd - iStart);

			int curPosBlock = 0;
			CString cmpSubBlock = sSegment.Tokenize(_T(":"), curPosBlock);

			while (cmpSubBlock != _T(""))
			{
				bool bPassed = (i == 1) ? false : true;

				int curPosToken = 0;
				CString cmpSubStr = cmpSubBlock.Tokenize(_T("|"), curPosToken);

				while (cmpSubStr != _T(""))
				{
					int cmpResult;

					if (cmpSubStr.Find(_T("*")) != -1 || cmpSubStr.Find(_T("?")) != -1)
						cmpResult = (wildcmp(cmpSubStr.GetBuffer(), sFullName.GetBuffer()) == 0) ? -1 : 1;
					else
						cmpResult = sFullName.Find(cmpSubStr);

					switch (i)
					{
					case 0:	if (cmpResult == -1) bPassed = false; break;
					case 1:	if (cmpResult != -1) bPassed = true; break;
					case 2:	if (cmpResult != -1) bPassed = false; break;
					}
					cmpSubStr = cmpSubBlock.Tokenize(_T("|"), curPosToken);
				}
				switch (i)
				{
				case 0:
				case 2: if (bPassed) bPassedGlobal[i] = true; break;
				case 1: if (!bPassed) bPassedGlobal[i] = false; break;
				}
				cmpSubBlock = sSegment.Tokenize(_T(":"), curPosBlock);
		}
	}
		for (int i = 0; i < 3; i++)
			if (!bPassedGlobal[i]) return false;
		return true;
	}
	else
	{
		int curPos = 0;
		CString cmpSubStr = sFilterMask.Tokenize(_T("|"), curPos);

		while (cmpSubStr != _T(""))
		{
			int cmpResult;

			if (cmpSubStr.Find(_T("*")) != -1 || cmpSubStr.Find(_T("?")) != -1)
				cmpResult = (wildcmp(cmpSubStr.GetBuffer(), sFullName.GetBuffer()) == 0) ? -1 : 1;
			else
				cmpResult = sFullName.Find(cmpSubStr);

			if(cmpResult != -1)
				return true;
			cmpSubStr = sFilterMask.Tokenize(_T("|"), curPos);
		}
	}
	return false;
}
// [TPT] - khaos::categorymod-

void CDownloadQueue::AddToResolved( CPartFile* pFile, SUnresolvedHostname* pUH )
{
	if( pFile && pUH )
		m_srcwnd.AddToResolve( pFile->GetFileHash(), pUH->strHostname, pUH->nPort, pUH->strURL);
}

void CDownloadQueue::AddDownload(CPartFile* newfile,bool paused) {
	// Barry - Add in paused mode if required
	if (paused)
		newfile->PauseFile();
	
	timeAllowUnlimitedUP = ::GetTickCount();//Reset the time to maxUp

	//SetAutoCat(newfile);// HoaX_69 / Slugfiller: AutoCat // [TPT] - khaos::categorymod

	filelist.AddTail(newfile);
	SortByPriority();
	CheckDiskspace();	// SLUGFILLER: checkDiskspace
	theApp.emuledlg->transferwnd->downloadlistctrl.AddFile(newfile);
	AddLogLine(true,GetResString(IDS_NEWDOWNLOAD),newfile->GetFileName());
	CString msgTemp;
	msgTemp.Format(GetResString(IDS_NEWDOWNLOAD) + _T("\n"), newfile->GetFileName());
	theApp.emuledlg->ShowNotifier(msgTemp, TBN_DLOADADDED);

	partfileindex->AddPartFile(newfile->GetPartMetFileName(), newfile->GetFileName(), newfile->GetFileSize(), newfile->GetFileHash(), newfile->GetHashset(), (newfile->GetAICHHashset() && newfile->GetAICHHashset()->HasValidMasterHash() && (newfile->GetAICHHashset()->GetStatus() == AICH_VERIFIED))?newfile->GetAICHHashset()->GetMasterHash().GetString():_T(""));	// SLUGFILLER: indexPartFiles
	// [TPT] - WebCache
	thePrefs.UpdateWebcacheReleaseAllowed(); //JP webcache release
}

bool CDownloadQueue::IsFileExisting(const uchar* fileid, bool bLogWarnings)
{
	const CKnownFile* file = theApp.sharedfiles->GetFileByID(fileid);
	if (file){
		if (bLogWarnings){
			if (file->IsPartFile())
				LogWarning(LOG_STATUSBAR, GetResString(IDS_ERR_ALREADY_DOWNLOADING), file->GetFileName());
			else
				LogWarning(LOG_STATUSBAR, GetResString(IDS_ERR_ALREADY_DOWNLOADED), file->GetFileName());
		}
		return true;
	}
	else if ((file = GetFileByID(fileid)) != NULL){
		if (bLogWarnings)
			LogWarning(LOG_STATUSBAR, GetResString(IDS_ERR_ALREADY_DOWNLOADING), file->GetFileName());
		return true;
	}
	return false;
}


// [TPT]
// Maella -New bandwidth control-
void CDownloadQueue::Process(){

    	ProcessLocalRequests(); // send src requests to local server

	QuickStart(); // [TPT] - quick start	
	
	CanSwitchUnlimited();//[TPT] - Unlimited upload with no downloads

    // Elapsed time (TIMER_PERIOD not accurate) 
    uint32 deltaTime = ::GetTickCount() - m_lastProcessTime;
    m_lastProcessTime += deltaTime;

    // Anticipate high CPU load => unregular cycle
    if(deltaTime > 0)
    {
		const float maxDownload = (float)thePrefs.GetMaxDownloadInBytesPerSec(true)/1024.0f;

		const bool isLimited = (maxDownload < UNLIMITED);

        if(isLimited == false)
        {
            m_nDownloadSlopeControl = 0;
        }
        else 
        {
            // Bandwidth control
            const sint32 slopeCredit = (sint32)(maxDownload * 1.024f * (float)deltaTime); // it's 1.024 due to deltaTime is in milliseconds
            const sint32 maxSlop = 12 * slopeCredit / 10; // 120%

            // The Bandwitch control should be valid for an AVERAGE value
            m_nDownloadSlopeControl += slopeCredit; // [bytes/period]
            m_nDownloadSlopeControl -= (sint32)(theApp.pBandWidthControl->GeteMuleIn() - m_lastReceivedBytes);

            // Trunk negative value => possible when Overhead compensation activated
            if(m_nDownloadSlopeControl > maxSlop)
            {
                m_nDownloadSlopeControl = maxSlop;
            }
			//Xman download fix
			/*else if(m_nDownloadSlopeControl < 0)
            {
				m_nDownloadSlopeControl = 0;
            } */           
            
        }

        // Keep current value for next processing
        //m_lastOverallReceivedBytes = theApp.pBandWidthControl->GeteMuleInOverall();
        m_lastReceivedBytes = theApp.pBandWidthControl->GeteMuleIn();

        sint32 nDownloadSlopeControl = m_nDownloadSlopeControl;  

		ProcessDL(nDownloadSlopeControl, isLimited);
    }

    udcounter++;

    // Server statistic + UDP socket    
    if (udcounter == (500/TIMER_PERIOD)) { // Maella -Small latency- every 0.5 second  
        if (theApp.serverconnect->IsUDPSocketAvailable())
        {
            if((!lastudpstattime) || (::GetTickCount() - lastudpstattime) > UDPSERVERSTATTIME)
            {
                lastudpstattime = ::GetTickCount();
                theApp.serverlist->ServerStats();
            }
        }
    }

    if (udcounter >= (1000/TIMER_PERIOD))  // Maella -Small latency- every 1 second
    {
        udcounter = 0;

        // [TPT] - Patch
        // This will avoid reordering list while filelist processing (due to Maella bandwidth)
        for (POSITION pos = filelist.GetHeadPosition(); pos != NULL;)
        {
            CPartFile* cur_file =  filelist.GetNext(pos);

	    if(cur_file)
	    {
            cur_file->UpdateAutoDownPriority();
            // This will make sure we don't keep old sources to paused and stoped files..
            // Remark: don't need to be processed every 50/100 ms
            cur_file->StopPausedFile();
	    }
        }
        // [TPT] - Patch

        if (theApp.serverconnect->IsUDPSocketAvailable())
        {
            if ((!lastudpsearchtime) || (::GetTickCount() - lastudpsearchtime) > UDPSERVERREASKTIME)
                SendNextUDPPacket();
        }
    }

    // [TPT] - khaos::categorymod+ Purge ED2K Link Queue
    if (m_iLastLinkQueuedTick && !m_bBusyPurgingLinks && (GetTickCount() - m_iLastLinkQueuedTick) > 400)
        PurgeED2KLinkQueue();
    else if (m_ED2KLinkQueue.GetCount() && !thePrefs.SelectCatForNewDL()) // This should not happen.
    {
        PurgeED2KLinkQueue();
        if (thePrefs.GetVerbose())
            AddDebugLogLine(false, _T("ERROR: Links in ED2K Link Queue while SelectCatForNewDL was disabled!"));
    }
    // [TPT] - khaos::categorymod-

    CheckDiskspaceTimed();

    // ZZ:DownloadManager -->
    //if((!m_dwLastA4AFtime) || (::GetTickCount() - m_dwLastA4AFtime) > 2*60*1000) {
    //    theApp.clientlist->ProcessA4AFClients();
    //    m_dwLastA4AFtime = ::GetTickCount();
    //}
    // <-- ZZ:DownloadManager
}
// Maella end


// [TPT] - Maella -New bandwidth control-
void CDownloadQueue::ProcessDL(sint32 nDownloadSlopeControl, bool isLimited)
{
	// Remark: filelist is not sorted by priority (see 'balancing' below), needed to priorize the connection (e.g. during start-up)
	for(int priority = 0; priority < 3; priority++)
    {
		POSITION next_pos = filelist.GetHeadPosition();
        for(int i=0; i<filelist.GetCount(); i++)
		{
			POSITION cur_pos = next_pos;
			const int count = filelist.GetCount(); // Could changed => to check
			CPartFile* cur_file = filelist.GetNext(next_pos); // Already point to the next element

            if(cur_file && (cur_file->GetStatus() == PS_READY || cur_file->GetStatus() == PS_EMPTY))
            { 
                if((priority == 0 && cur_file->GetDownPriority() == PR_HIGH) ||
                    (priority == 1 && cur_file->GetDownPriority() == PR_NORMAL) ||
                    (priority == 2 && (cur_file->GetDownPriority() == PR_LOW)))
                {						
                    // The method must be called regardless of nDownloadSlopeControl
                    // The method returns the size of the received blocks if the download rate is limited (otherwise zero)
                    uint32 maxAmmount = (nDownloadSlopeControl <= 0) ? 0 : (uint32)nDownloadSlopeControl;
                    uint32 receivedBlock = cur_file->Process(maxAmmount, isLimited, udcounter == 0);						
                    if(receivedBlock > 0)
                    {

                        // Try to 'balance' the download between sources (=> clients).
                        // Move the 'uploaded' at the end of the list.
                        if(isLimited == true && count == filelist.GetCount() && cur_file->GetStatus() == PS_READY)
                        {
                            // To check if these line are a source of bug
                            filelist.RemoveAt(cur_pos);
                            filelist.AddTail(cur_file); 
                        }
                    }
                }
            }
        }
    }
}


//[TPT] - Unlimited upload with no downloads
bool CDownloadQueue::IsThereAnyDownload()
{
	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; )
	{
		CPartFile* cur_file = filelist.GetNext(pos);
		if(cur_file->GetTransferringSrcCount()>0)
			return true;
	}

	return false;
}

// [TPT] - quick start
void CDownloadQueue::ResetQuickStart()
{
	quickflag = 0;
	quickflags = 0;
}

void CDownloadQueue::QuickStart()
{	
	if(thePrefs.GetQuickStart()&& (theApp.serverconnect->IsConnected() || Kademlia::CKademlia::isConnected()) && quickflag == 0)
	{
		if(quickflags == 0)
		{
			quicktime = ::GetTickCount();
			MaxconnPerFiveBack = thePrefs.GetMaxConperFive();
			MaxconBack = thePrefs.GetMaxCon();
			manageConnection = thePrefs.IsManageConnection();
			if (MaxconnPerFiveBack < thePrefs.GetQuickStartMaxConPerFive())
				thePrefs.SetMaxConsPerFive(thePrefs.GetQuickStartMaxConPerFive());
			if (MaxconBack < thePrefs.GetQuickStartMaxCon())
				thePrefs.SetMaxCon(thePrefs.GetQuickStartMaxCon());
			if(manageConnection)
				thePrefs.SetManageConnection(false);
			quickflags = 1;
			countfiles = GetFileCount();
			for(POSITION pos = filelist.GetHeadPosition(); pos != NULL;) 
			{ 
				CPartFile* cur_file = filelist.GetNext(pos); 
				if( cur_file->GetStatus() == PS_PAUSED )
					countfiles --;
			}
		}
		DWORD dwCurTick = ::GetTickCount();
		if((dwCurTick > quicktime) &&  // [TPT] modified
			((dwCurTick - quicktime > uint32 ((countfiles*0.70)*1000*60)) || (dwCurTick - quicktime > 20*1000*60)))
		{
			thePrefs.SetMaxConsPerFive(MaxconnPerFiveBack);
			thePrefs.SetMaxCon(MaxconBack);
			thePrefs.SetManageConnection(manageConnection);
			quickflag = 1;
		}
	}
}
// [TPT] - quick start


void CDownloadQueue::CanSwitchUnlimited()
{
	uint32 newTime = ::GetTickCount();

	//5 minutes to get sources
	if((newTime - timeAllowUnlimitedUP) < MIN2MS(5))
	{
		if(unlimitedFlag)
		{
			thePrefs.SetMaxUpload(upTemp);
			unlimitedFlag = false;
			if (thePrefs.GetVerbose())
				AddPhoenixLogLine(true, GetResString(IDS_UNSPEED_NEWDOWN), upTemp);
			upTemp = -1;
			return;
		}
	}
	else
	{

	if(thePrefs.GetUnlimitedUp())
	{
		//Not actived and no downloads > Active it
		if(unlimitedFlag == false && !IsThereAnyDownload())
		{
			unlimitedFlag = true;
			upTemp = thePrefs.GetMaxUpload();
			thePrefs.SetMaxUpload(thePrefs.GetMaxGraphUploadRate());
			if (thePrefs.GetVerbose()) AddPhoenixLogLine(true, GetResString(IDS_UNSPEED_UNLIM),thePrefs.GetMaxGraphUploadRate());
		}
		//Actived and no downloads > we check if we have changed it in preferences and correct it
		if(unlimitedFlag == true && !IsThereAnyDownload())
		{		
			if(thePrefs.GetMaxUpload() != thePrefs.GetMaxGraphUploadRate())
			{
				upTemp = thePrefs.GetMaxUpload();
				if (thePrefs.GetVerbose()) AddPhoenixLogLine(true, GetResString(IDS_UNSPEED_CHANGEPRE), upTemp);
			}
			thePrefs.SetMaxUpload(thePrefs.GetMaxGraphUploadRate());
		}
		//Actived and downloads > we have to disable it
		if(unlimitedFlag == true && IsThereAnyDownload())
		{
			thePrefs.SetMaxUpload((upTemp > -1)?upTemp:10.0f);
			unlimitedFlag = false;
			if (thePrefs.GetVerbose()) AddPhoenixLogLine(true, GetResString(IDS_UNSPEED_RESTORING), upTemp,thePrefs.GetMaxGraphUploadRate());
			upTemp = -1;
		}

	}
	else
	{	//not enabled but it is running -> disabling
		if(unlimitedFlag == true)
		{
			thePrefs.SetMaxUpload((upTemp > -1)?upTemp:10.0f);
			unlimitedFlag = false;
			if (thePrefs.GetVerbose()) AddPhoenixLogLine(true, GetResString(IDS_UNSPEED_RESTORING), upTemp, thePrefs.GetMaxGraphUploadRate());
			upTemp = -1;
		}
	}
		}
}
//[TPT] - Unlimited upload with no downloads


// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
void CDownloadQueue::CompDownloadRate()
{
	// Compute the download datarate of all clients
	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; )
		filelist.GetNext(pos)->CompDownloadRate();
}
// Maella end

CPartFile* CDownloadQueue::GetFileByIndex(int index) const
{
	POSITION pos = filelist.FindIndex(index);
	if (pos)
		return filelist.GetAt(pos);
	return NULL;
}

// [TPT] - SLUGFILLER: SafeHash
CPartFile* CDownloadQueue::GetFileByMetFileName(const CString& rstrName) const
{
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){
		CPartFile* cur_file = filelist.GetNext(pos);
		if (!rstrName.CompareNoCase(cur_file->GetPartMetFileName()))
			return cur_file;
	}
	return 0;
}
// [TPT] - SLUGFILLER: SafeHash

CPartFile* CDownloadQueue::GetFileByID(const uchar* filehash) const
{
	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; )
	{
		CPartFile* cur_file = filelist.GetNext(pos);
		if (!md4cmp(filehash, cur_file->GetFileHash()))
			return cur_file;
	}
	return NULL;
}

CPartFile* CDownloadQueue::GetFileByKadFileSearchID(uint32 id) const
{
	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; )
	{
		CPartFile* cur_file = filelist.GetNext(pos);
		if (id == cur_file->GetKadFileSearchID())
			return cur_file;
	}
	return NULL;
}

bool CDownloadQueue::IsPartFile(const CKnownFile* file) const
{
	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; )
	{
		if (file == filelist.GetNext(pos))
			return true;
	}
	return false;
}

// SLUGFILLER: SafeHash
bool CDownloadQueue::IsTempFile(const CString& rstrDirectory, const CString& rstrName) const
{
	// do not share a part file from the temp directory, if there is still a corresponding entry in
	// the download queue -- because that part file is not yet complete.
	CString othername = rstrName;
	int extpos = othername.ReverseFind(_T('.'));
	if (extpos == -1)
		return false;
	CString ext = othername.Mid(extpos);
	if (ext.CompareNoCase(_T(".met"))) {
		if (!ext.CompareNoCase(_T(".part")))
			othername += _T(".met");
		else if (!ext.CompareNoCase(PARTMET_BAK_EXT) || !ext.CompareNoCase(PARTMET_TMP_EXT))
			othername = othername.Left(extpos);
		else
			return false;
	}
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){
		CPartFile* cur_file = filelist.GetNext(pos);
		if (!othername.CompareNoCase(cur_file->GetPartMetFileName()))
			return true;
	}

	return false;
}
// SLUGFILLER: SafeHash

bool CDownloadQueue::CheckAndAddSource(CPartFile* sender,CUpDownClient* source){
	if (sender->IsStopped()){
		delete source;
		return false;
	}

	if (source->HasValidHash())
	{
		if(!md4cmp(source->GetUserHash(), thePrefs.GetUserHash()))
		{
			if (thePrefs.GetVerbose())
				AddDebugLogLine(false, _T("Tried to add source with matching hash to your own."));
			delete source;
			return false;
		}
	}
	// filter sources which are known to be dead/useless
	if (theApp.clientlist->m_globDeadSourceList.IsDeadSource(source) || sender->m_DeadSourceList.IsDeadSource(source)){
		if (thePrefs.GetLogFilteredIPs() && (thePrefs.GetBlockDeadSourcesMsg() == false)) // [TPT] - Filter dead sources
			AddDebugLogLine(DLP_DEFAULT, false, _T("Rejected source because it was found on the DeadSourcesList (%s) for file %s : %s")
			,sender->m_DeadSourceList.IsDeadSource(source)? _T("Local") : _T("Global"), sender->GetFileName(), source->DbgGetClientInfo() );
		delete source;
		return false;
	}

	// "Filter LAN IPs" and/or "IPfilter" is not required here, because it was already done in parent functions

	// uses this only for temp. clients
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){
		CPartFile* cur_file = filelist.GetNext(pos);
		for (POSITION pos2 = cur_file->srclist.GetHeadPosition();pos2 != 0; ){
			CUpDownClient* cur_client = cur_file->srclist.GetNext(pos2);
			if (cur_client->Compare(source, true) || cur_client->Compare(source, false)){
				if (cur_file == sender){ // this file has already this source
					delete source;
					return false;
				}
				// set request for this source
				if (cur_client->AddRequestForAnotherFile(sender)){
					theApp.emuledlg->transferwnd->downloadlistctrl.AddSource(sender,cur_client,true);
					delete source;
                    if(cur_client->GetDownloadState() != DS_CONNECTED) {
                        cur_client->SwapToAnotherFile(_T("New A4AF source found. CDownloadQueue::CheckAndAddSource()"), false, false, false, NULL, true, false); // ZZ:DownloadManager
                    }
					return false;
				}
				else{
					delete source;
					return false;
				}
			}
		}
	}
	//our new source is real new but maybe it is already uploading to us?
	//if yes the known client will be attached to the var "source"
	//and the old sourceclient will be deleted
	if (theApp.clientlist->AttachToAlreadyKnown(&source,0)){
#ifdef _DEBUG
		if (thePrefs.GetVerbose() && source->GetRequestFile()){
			// if a client sent us wrong sources (sources for some other file for which we asked but which we are also
			// downloading) we may get a little in trouble here when "moving" this source to some other partfile without
			// further checks and updates.
			if (md4cmp(source->GetRequestFile()->GetFileHash(), sender->GetFileHash()) != 0)
				AddDebugLogLine(false, _T("*** CDownloadQueue::CheckAndAddSource -- added potential wrong source (%u)(diff. filehash) to file \"%s\""), source->GetUserIDHybrid(), sender->GetFileName());
			if (source->GetRequestFile()->GetPartCount() != 0 && source->GetRequestFile()->GetPartCount() != sender->GetPartCount())
				AddDebugLogLine(false, _T("*** CDownloadQueue::CheckAndAddSource -- added potential wrong source (%u)(diff. partcount) to file \"%s\""), source->GetUserIDHybrid(), sender->GetFileName());
		}
#endif
		source->SetRequestFile(sender);
	}
	else{
		// here we know that the client instance 'source' is a new created client instance (see callers) 
		// which is therefor not already in the clientlist, we can avoid the check for duplicate client list entries 
		// when adding this client
		theApp.clientlist->AddClient(source,true);
	}
	
#ifdef _DEBUG
	if (thePrefs.GetVerbose() && source->GetPartCount()!=0 && source->GetPartCount()!=sender->GetPartCount()){
		DEBUG_ONLY(AddDebugLogLine(false, _T("*** CDownloadQueue::CheckAndAddSource -- New added source (%u, %s) had still value in partcount"), source->GetUserIDHybrid(), sender->GetFileName()));
	}
#endif

	sender->srclist.AddTail(source);
	theApp.emuledlg->transferwnd->downloadlistctrl.AddSource(sender,source,false);
	return true;
}

bool CDownloadQueue::CheckAndAddKnownSource(CPartFile* sender,CUpDownClient* source, bool bIgnoreGlobDeadList){
	if (sender->IsStopped())
		return false;

	// filter sources which are known to be dead/useless
	if ( (theApp.clientlist->m_globDeadSourceList.IsDeadSource(source) && !bIgnoreGlobDeadList) || sender->m_DeadSourceList.IsDeadSource(source)){
		if (thePrefs.GetLogFilteredIPs() && (thePrefs.GetBlockDeadSourcesMsg() == false)) // [TPT] - Filter dead sources
			AddDebugLogLine(DLP_DEFAULT, false, _T("Rejected source because it was found on the DeadSourcesList (%s) for file %s : %s")
			,sender->m_DeadSourceList.IsDeadSource(source)? _T("Local") : _T("Global"), sender->GetFileName(), source->DbgGetClientInfo() );
		return false;
	}

	// "Filter LAN IPs" -- this may be needed here in case we are connected to the internet and are also connected
	// to a LAN and some client from within the LAN connected to us. Though this situation may be supported in future
	// by adding that client to the source list and filtering that client's LAN IP when sending sources to
	// a client within the internet.
	//
	// "IPfilter" is not needed here, because that "known" client was already IPfiltered when receiving OP_HELLO.
	if (!source->HasLowID()){
		uint32 nClientIP = ntohl(source->GetUserIDHybrid());
		if (!IsGoodIP(nClientIP)){ // check for 0-IP, localhost and LAN addresses
			if (thePrefs.GetLogFilteredIPs())
				AddDebugLogLine(false, _T("Ignored already known source with IP=%s"), ipstr(nClientIP));
			return false;
		}
	}

	// use this for client which are already know (downloading for example)
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){
		CPartFile* cur_file = filelist.GetNext(pos);
		if (cur_file->srclist.Find(source)){
			if (cur_file == sender)
				return false;
			if (source->AddRequestForAnotherFile(sender))
				theApp.emuledlg->transferwnd->downloadlistctrl.AddSource(sender,source,true);
                if(source->GetDownloadState() != DS_CONNECTED) {
                    source->SwapToAnotherFile(_T("New A4AF source found. CDownloadQueue::CheckAndAddKnownSource()"), false, false, false, NULL, true, false); // ZZ:DownloadManager
                }
			return false;
		}
	}
#ifdef _DEBUG
	if (thePrefs.GetVerbose() && source->GetRequestFile()){
		// if a client sent us wrong sources (sources for some other file for which we asked but which we are also
		// downloading) we may get a little in trouble here when "moving" this source to some other partfile without
		// further checks and updates.
		if (md4cmp(source->GetRequestFile()->GetFileHash(), sender->GetFileHash()) != 0)
			AddDebugLogLine(false, _T("*** CDownloadQueue::CheckAndAddKnownSource -- added potential wrong source (%u)(diff. filehash) to file \"%s\""), source->GetUserIDHybrid(), sender->GetFileName());
		if (source->GetRequestFile()->GetPartCount() != 0 && source->GetRequestFile()->GetPartCount() != sender->GetPartCount())
			AddDebugLogLine(false, _T("*** CDownloadQueue::CheckAndAddKnownSource -- added potential wrong source (%u)(diff. partcount) to file \"%s\""), source->GetUserIDHybrid(), sender->GetFileName());
	}
#endif
	source->SetRequestFile(sender);
	sender->srclist.AddTail(source);
	source->SetSourceFrom(SF_PASSIVE);
#ifdef _DEBUG
	if (thePrefs.GetVerbose() && source->GetPartCount()!=0 && source->GetPartCount()!=sender->GetPartCount()){
		DEBUG_ONLY(AddDebugLogLine(false, _T("*** CDownloadQueue::CheckAndAddKnownSource -- New added source (%u, %s) had still value in partcount"), source->GetUserIDHybrid(), sender->GetFileName()));
	}
#endif

	theApp.emuledlg->transferwnd->downloadlistctrl.AddSource(sender,source,false);
	//UpdateDisplayedInfo();
	return true;
}

bool CDownloadQueue::RemoveSource(CUpDownClient* toremove, bool bDoStatsUpdate)
{
	bool bRemovedSrcFromPartFile = false;
	// [TPT] - Code Improvement
	POSITION pos;
	for (pos = filelist.GetHeadPosition();pos != 0;) 
	{
		CPartFile* cur_file = filelist.GetNext(pos);
		for (POSITION pos2 = cur_file->srclist.GetHeadPosition();pos2 != NULL;)
		{
			POSITION cur_pos = pos2;
			cur_file->srclist.GetNext(pos2);
			if (toremove == cur_file->srclist.GetAt(cur_pos))
			{
				cur_file->srclist.RemoveAt(cur_pos);
				
				bRemovedSrcFromPartFile = true;
				if ( bDoStatsUpdate ){
					cur_file->RemoveDownloadingSource(toremove);
					cur_file->UpdatePartsInfo();
					cur_file->NewSrcIncPartsInfo(); // [TPT] - enkeyDev: ICS
				}
				break;
			}
		}
		if ( bDoStatsUpdate )
			cur_file->UpdateAvailablePartsCount();
	}
	
	// remove this source on all files in the downloadqueue who link this source
	// pretty slow but no way arround, maybe using a Map is better, but that's slower on other parts
	for(pos = toremove->m_OtherRequests_list.GetHeadPosition(); pos != NULL;)
	{
		POSITION cur_pos = pos;
		toremove->m_OtherRequests_list.GetNext(pos);
		POSITION pos2 = toremove->m_OtherRequests_list.GetAt(cur_pos)->A4AFsrclist.Find(toremove); 
		if(pos2)
		{ 
			toremove->m_OtherRequests_list.GetAt(cur_pos)->A4AFsrclist.RemoveAt(pos2);
			theApp.emuledlg->transferwnd->downloadlistctrl.RemoveSource(toremove,toremove->m_OtherRequests_list.GetAt(cur_pos));
			toremove->m_OtherRequests_list.RemoveAt(cur_pos);
		}
	}

	for(pos = toremove->m_OtherNoNeeded_list.GetHeadPosition();pos != NULL;)
	{
		POSITION cur_pos = pos;
		toremove->m_OtherNoNeeded_list.GetNext(pos);				
		POSITION pos2 = toremove->m_OtherNoNeeded_list.GetAt(cur_pos)->A4AFsrclist.Find(toremove); 
		if(pos2)
	{
			toremove->m_OtherNoNeeded_list.GetAt(cur_pos)->A4AFsrclist.RemoveAt(pos2);
			theApp.emuledlg->transferwnd->downloadlistctrl.RemoveSource(toremove,toremove->m_OtherNoNeeded_list.GetAt(cur_pos));
			toremove->m_OtherNoNeeded_list.RemoveAt(cur_pos);
		}
	}
	// [TPT] - Code Improvement end

	// [TPT] - SLUGFILLER: showComments remove - removed per-client comments

	toremove->SetDownloadState(DS_NONE);
	theApp.emuledlg->transferwnd->downloadlistctrl.RemoveSource(toremove,0);
	toremove->SetRequestFile(NULL);
	return bRemovedSrcFromPartFile;
}

void CDownloadQueue::RemoveFile(CPartFile* toremove)
{
	RemoveLocalServerRequest(toremove);

	// [TPT] - Maella -Code Improvement-
	POSITION pos = filelist.Find(toremove);
	if (pos != NULL)
	{
			filelist.GetAt(pos)->srclist.RemoveAll(); // [TPT] - Xman Security 
			filelist.RemoveAt(pos);
	}
	// Maella end

	SortByPriority();
	CheckDiskspace();	// SLUGFILLER: checkDiskspace
	
	partfileindex->RemovePartFile(toremove->GetPartMetFileName());	// [TPT] - SLUGFILLER: indexPartFiles
}

void CDownloadQueue::DeleteAll(){
	// [TPT] - Code improvement
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){
		CPartFile* cur_file = filelist.GetNext(pos);
		cur_file->srclist.RemoveAll();
		// Barry - Should also remove all requested blocks
		// Don't worry about deleting the blocks, that gets handled 
		// when CUpDownClient is deleted in CClientList::DeleteAll()
		cur_file->RemoveAllRequestedBlocks();
	}
}

// Max. file IDs per UDP packet
// ----------------------------
// 576 - 30 bytes of header (28 for UDP, 2 for "E3 9A" edonkey proto) = 546 bytes
// 546 / 16 = 34
#define MAX_FILES_PER_UDP_PACKET	31	// 2+16*31 = 498 ... is still less than 512 bytes!!

#define MAX_REQUESTS_PER_SERVER		35

int CDownloadQueue::GetMaxFilesPerUDPServerPacket() const
{
	int iMaxFilesPerPacket;
	if (cur_udpserver && cur_udpserver->GetUDPFlags() & SRV_UDPFLG_EXT_GETSOURCES)
	{
		// get max. file ids per packet
		if (m_cRequestsSentToServer < MAX_REQUESTS_PER_SERVER)
			iMaxFilesPerPacket = min(MAX_FILES_PER_UDP_PACKET, MAX_REQUESTS_PER_SERVER - m_cRequestsSentToServer);
		else{
			ASSERT(0);
			iMaxFilesPerPacket = 0;
		}
	}
	else
		iMaxFilesPerPacket = 1;

	return iMaxFilesPerPacket;
}

bool CDownloadQueue::SendGlobGetSourcesUDPPacket(CSafeMemFile* data)
{
	bool bSentPacket = false;

	if (   cur_udpserver
		&& (theApp.serverconnect->GetCurrentServer() == NULL || 
			cur_udpserver != theApp.serverlist->GetServerByAddress(theApp.serverconnect->GetCurrentServer()->GetAddress(),theApp.serverconnect->GetCurrentServer()->GetPort())))
	{
		ASSERT( data->GetLength() > 0 && data->GetLength() % 16 == 0 );
		int iFileIDs = data->GetLength() / 16;
		if (thePrefs.GetDebugServerUDPLevel() > 0)
			Debug(_T(">>> Sending OP__GlobGetSources to server(#%02x) %-15s (%3u of %3u); FileIDs=%u\n"), cur_udpserver->GetUDPFlags(), cur_udpserver->GetAddress(), m_iSearchedServers + 1, theApp.serverlist->GetServerCount(), iFileIDs);
		Packet packet(data);
		packet.opcode = OP_GLOBGETSOURCES;
		theStats.AddUpDataOverheadServer(packet.size);
		theApp.serverconnect->SendUDPPacket(&packet,cur_udpserver,false);
		
		m_cRequestsSentToServer += iFileIDs;
		bSentPacket = true;
	}

	return bSentPacket;
}

bool CDownloadQueue::SendNextUDPPacket()
{
	if (   filelist.IsEmpty() 
        || !theApp.serverconnect->IsUDPSocketAvailable() 
        || !theApp.serverconnect->IsConnected())
		return false;
	if (!cur_udpserver){
		if ((cur_udpserver = theApp.serverlist->GetNextServer(cur_udpserver)) == NULL){
			TRACE("ERROR:SendNextUDPPacket() no server found\n");
			StopUDPRequests();
		};
		m_cRequestsSentToServer = 0;
	}

	// get max. file ids per packet for current server
	int iMaxFilesPerPacket = GetMaxFilesPerUDPServerPacket();

	// loop until the packet is filled or a packet was sent
	bool bSentPacket = false;
	CSafeMemFile dataGlobGetSources(16);
	int iFiles = 0;
	while (iFiles < iMaxFilesPerPacket && !bSentPacket)
	{
		// get next file to search sources for
		CPartFile* nextfile = NULL;
		while (!bSentPacket && !(nextfile && (nextfile->GetStatus() == PS_READY || nextfile->GetStatus() == PS_EMPTY)))
		{
			if (lastfile == NULL) // we just started the global source searching or have switched the server
			{
				// get first file to search sources for
				nextfile = filelist.GetHead();
				lastfile = nextfile;
			}
			else
			{
				POSITION pos = filelist.Find(lastfile);
				if (pos == 0) // the last file is no longer in the DL-list (may have been finished or canceld)
				{
					// get first file to search sources for
					nextfile = filelist.GetHead();
					lastfile = nextfile;
				}
				else
				{
					filelist.GetNext(pos);
					if (pos == 0) // finished asking the current server for all files
					{
						// if there are pending requests for the current server, send them
						if (dataGlobGetSources.GetLength() > 0)
						{
							if (SendGlobGetSourcesUDPPacket(&dataGlobGetSources))
								bSentPacket = true;
							dataGlobGetSources.SetLength(0);
						}

						// get next server to ask
						cur_udpserver = theApp.serverlist->GetNextServer(cur_udpserver, lastfile);	// [TPT] - itsonlyme: cacheUDPsearchResults
						m_cRequestsSentToServer = 0;
						if (cur_udpserver == NULL)
						{
							// finished asking all servers for all files
							if (thePrefs.GetDebugServerUDPLevel() > 0 && thePrefs.GetDebugServerSourcesLevel() > 0)
								Debug(_T("Finished UDP search processing for all servers (%u)\n"), theApp.serverlist->GetServerCount());

							lastudpsearchtime = ::GetTickCount();
							lastfile = NULL;
							m_iSearchedServers = 0;
							return false; // finished (processed all file & all servers)
						}
						m_iSearchedServers++;

						// if we already sent a packet, switch to the next file at next function call
						if (bSentPacket){
							lastfile = NULL;
							break;
						}

						// get max. file ids per packet for current server
						iMaxFilesPerPacket = GetMaxFilesPerUDPServerPacket();

						// have selected a new server; get first file to search sources for
						nextfile = filelist.GetHead();
						lastfile = nextfile;
					}
					else
					{
						nextfile = filelist.GetAt(pos);
						lastfile = nextfile;
					}
				}
			}
		}

		// [TPT] - Sivka AutoHL
		if (!bSentPacket && nextfile && nextfile->GetSourceCount() < nextfile->GetMaxSourcesUDPLimit())
		{
			dataGlobGetSources.WriteHash16(nextfile->GetFileHash());
			iFiles++;
			if (thePrefs.GetDebugServerUDPLevel() > 0 && thePrefs.GetDebugServerSourcesLevel() > 0)
				Debug(_T(">>> Queued  OP__GlobGetSources to server(#%02x) %-15s (%3u of %3u); Buff  %u=%s\n"), cur_udpserver->GetUDPFlags(), cur_udpserver->GetAddress(), m_iSearchedServers + 1, theApp.serverlist->GetServerCount(), iFiles, DbgGetFileInfo(nextfile->GetFileHash()));
		}
	}

	ASSERT( dataGlobGetSources.GetLength() == 0 || !bSentPacket );

	if (!bSentPacket && dataGlobGetSources.GetLength() > 0)
		SendGlobGetSourcesUDPPacket(&dataGlobGetSources);

	// send max 35 UDP request to one server per interval
	// if we have more than 35 files, we rotate the list and use it as queue
	if (m_cRequestsSentToServer >= MAX_REQUESTS_PER_SERVER)
	{
		if (thePrefs.GetDebugServerUDPLevel() > 0 && thePrefs.GetDebugServerSourcesLevel() > 0)
			Debug(_T("Rotating file list\n"));

		// move the last 35 files to the head
		if (filelist.GetCount() >= MAX_REQUESTS_PER_SERVER){
			for (int i = 0; i != MAX_REQUESTS_PER_SERVER; i++){
				filelist.AddHead( filelist.RemoveTail() );
			}
		}

		// and next server
		cur_udpserver = theApp.serverlist->GetNextServer(cur_udpserver);
		m_cRequestsSentToServer = 0;
		if (cur_udpserver == NULL){
			lastudpsearchtime = ::GetTickCount();
			lastfile = NULL;
			return false; // finished (processed all file & all servers)
		}
		m_iSearchedServers++;
		lastfile = NULL;
	}

	return true;
}

void CDownloadQueue::StopUDPRequests(){
	cur_udpserver = 0;
	lastudpsearchtime = ::GetTickCount();
	lastfile = 0;
}

// SLUGFILLER: checkDiskspace
bool CDownloadQueue::CompareParts(POSITION pos1, POSITION pos2){
	CPartFile* file1 = filelist.GetAt(pos1);
	CPartFile* file2 = filelist.GetAt(pos2);
    return CPartFile::RightFileHasHigherPrio(file1, file2);
}

void CDownloadQueue::SwapParts(POSITION pos1, POSITION pos2){
	CPartFile* file1 = filelist.GetAt(pos1);
	CPartFile* file2 = filelist.GetAt(pos2);
	filelist.SetAt(pos1, file2);
	filelist.SetAt(pos2, file1);
}

void CDownloadQueue::HeapSort(uint16 first, uint16 last){
	uint16 r;
	POSITION pos1 = filelist.FindIndex(first);
	for ( r = first; !(r & 0x8000) && (r<<1) < last; ){
		uint16 r2 = (r<<1)+1;
		POSITION pos2 = filelist.FindIndex(r2);
		if (r2 != last){
			POSITION pos3 = pos2;
			filelist.GetNext(pos3);
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

void CDownloadQueue::SortByPriority(){
	uint16 n = filelist.GetCount();
	if (!n)
		return;
	uint16 i;
	for ( i = n/2; i--; )
		HeapSort(i, n-1);
	for ( i = n; --i; ){
		SwapParts(filelist.FindIndex(0), filelist.FindIndex(i));
		HeapSort(0, i-1);
	}
}

void CDownloadQueue::CheckDiskspaceTimed()
{
	if ((!lastcheckdiskspacetime) || (::GetTickCount() - lastcheckdiskspacetime) > DISKSPACERECHECKTIME)
		CheckDiskspace();
}

void CDownloadQueue::CheckDiskspace(bool bNotEnoughSpaceLeft)
{
	lastcheckdiskspacetime = ::GetTickCount();

	// sorting the list could be done here, but I prefer to "see" that function call in the calling functions.
	//SortByPriority();

	// If disabled, resume any previously paused files
	if (!thePrefs.IsCheckDiskspaceEnabled())
	{
		if (!bNotEnoughSpaceLeft) // avoid worse case, if we already had 'disk full'
		{
			for( POSITION pos1 = filelist.GetHeadPosition(); pos1 != NULL; )
			{
				CPartFile* cur_file = filelist.GetNext(pos1);
				switch(cur_file->GetStatus())
				{
				case PS_PAUSED:
				case PS_ERROR:
				case PS_COMPLETING:
				case PS_COMPLETE:
					continue;
				}
				cur_file->ResumeFileInsufficient();
			}
		}
		return;
	}

	// 'bNotEnoughSpaceLeft' - avoid worse case, if we already had 'disk full'
	uint64 nTotalAvailableSpace = bNotEnoughSpaceLeft ? 0 : GetFreeDiskSpaceX(thePrefs.GetTempDir());
	if (thePrefs.GetMinFreeDiskSpace() == 0)
	{
		for( POSITION pos1 = filelist.GetHeadPosition(); pos1 != NULL; )
		{
			CPartFile* cur_file = filelist.GetNext(pos1);
			switch(cur_file->GetStatus())
			{
			case PS_PAUSED:
			case PS_ERROR:
			case PS_COMPLETING:
			case PS_COMPLETE:
				continue;
			}

			// Pause the file only if it would grow in size and would exceed the currently available free space
			uint32 nSpaceToGo = cur_file->GetNeededSpace();
			if (nSpaceToGo <= nTotalAvailableSpace)
			{
				nTotalAvailableSpace -= nSpaceToGo;
				cur_file->ResumeFileInsufficient();
			}
			else
				cur_file->PauseFile(true/*bInsufficient*/);
		}
	}
	else
	{
		for( POSITION pos1 = filelist.GetHeadPosition(); pos1 != NULL; )
		{
			CPartFile* cur_file = filelist.GetNext(pos1);
			switch(cur_file->GetStatus())
			{
			case PS_PAUSED:
			case PS_ERROR:
			case PS_COMPLETING:
			case PS_COMPLETE:
				continue;
			}

			if (nTotalAvailableSpace < thePrefs.GetMinFreeDiskSpace())
			{
				if (cur_file->IsNormalFile())
				{
					// Normal files: pause the file only if it would still grow
					uint32 nSpaceToGrow = cur_file->GetNeededSpace();
					if (nSpaceToGrow)
						cur_file->PauseFile(true/*bInsufficient*/);
				}
				else
				{
					// Compressed/sparse files: always pause the file
					cur_file->PauseFile(true/*bInsufficient*/);
				}
			}
			else
			{
				// doesn't work this way. resuming the file without checking if there is a chance to successfully
				// flush any available buffered file data will pause the file right after it was resumed and disturb
				// the StopPausedFile function.
				//cur_file->ResumeFileInsufficient();
			}
		}
	}
}
// SLUGFILLER: checkDiskspace

void CDownloadQueue::GetDownloadStats(SDownloadStats& results)
{
	MEMZERO(&results, sizeof results);
	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; )
	{
		const CPartFile* cur_file = filelist.GetNext(pos);

		results.a[0]  += cur_file->GetSourceCount();
		results.a[1]  += cur_file->GetTransferringSrcCount();
		results.a[2]  += cur_file->GetSrcStatisticsValue(DS_ONQUEUE);
		results.a[3]  += cur_file->GetSrcStatisticsValue(DS_REMOTEQUEUEFULL);
		results.a[4]  += cur_file->GetSrcStatisticsValue(DS_NONEEDEDPARTS);
		results.a[5]  += cur_file->GetSrcStatisticsValue(DS_CONNECTED);
		results.a[6]  += cur_file->GetSrcStatisticsValue(DS_REQHASHSET);
		results.a[7]  += cur_file->GetSrcStatisticsValue(DS_CONNECTING);
		results.a[8]  += cur_file->GetSrcStatisticsValue(DS_WAITCALLBACK);
		results.a[8]  += cur_file->GetSrcStatisticsValue(DS_WAITCALLBACKKAD);
		results.a[9]  += cur_file->GetSrcStatisticsValue(DS_TOOMANYCONNS);
		results.a[9]  += cur_file->GetSrcStatisticsValue(DS_TOOMANYCONNSKAD);
		results.a[10] += cur_file->GetSrcStatisticsValue(DS_LOWTOLOWIP);
		results.a[11] += cur_file->GetSrcStatisticsValue(DS_NONE);
		results.a[12] += cur_file->GetSrcStatisticsValue(DS_ERROR);
		results.a[13] += cur_file->GetSrcStatisticsValue(DS_BANNED);
		results.a[14] += cur_file->src_stats[3];
		results.a[15] += cur_file->GetSrcA4AFCount();
		results.a[16] += cur_file->src_stats[0];
		results.a[17] += cur_file->src_stats[1];
		results.a[18] += cur_file->src_stats[2];
		results.a[19] += cur_file->net_stats[0];
		results.a[20] += cur_file->net_stats[1];
		results.a[21] += cur_file->net_stats[2];
		results.a[22] += cur_file->m_DeadSourceList.GetDeadSourcesCount();
	}
}

CUpDownClient* CDownloadQueue::GetDownloadClientByIP(uint32 dwIP){
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){
		CPartFile* cur_file = filelist.GetNext(pos);
		for (POSITION pos2 = cur_file->srclist.GetHeadPosition();pos2 != 0; ){
			CUpDownClient* cur_client = cur_file->srclist.GetNext(pos2);
			if (dwIP == cur_client->GetIP()){
				return cur_client;
			}
		}
	}
	return NULL;
}

CUpDownClient* CDownloadQueue::GetDownloadClientByIP_UDP(uint32 dwIP, uint16 nUDPPort){
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){
		CPartFile* cur_file = filelist.GetNext(pos);
		for (POSITION pos2 = cur_file->srclist.GetHeadPosition();pos2 != 0;){
			CUpDownClient* cur_client = cur_file->srclist.GetNext(pos2);
			if (dwIP == cur_client->GetIP() && nUDPPort == cur_client->GetUDPPort()){
				return cur_client;
			}
		}
	}
	return NULL;
}

bool CDownloadQueue::IsInList(const CUpDownClient* client) const
{
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){
		CPartFile* cur_file = filelist.GetNext(pos);
		for (POSITION pos2 = cur_file->srclist.GetHeadPosition();pos2 != 0;){
			if (cur_file->srclist.GetNext(pos2) == client)
				return true;
		}
	}
	return false;
}

// [TPT] - khaos::categorymod+ We need to reset the linear priority, too, so that these files don't
// screw up the order of 'All' category.  This function is modified.
void CDownloadQueue::ResetCatParts(int cat, uint8 useCat)
{
	int useOrder = GetMaxCatResumeOrder(useCat);
	CPartFile* cur_file;
	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; )
	{
		cur_file = filelist.GetNext(pos);
		if (cur_file->GetCategory() == cat)
		{
			useOrder++;
			cur_file->SetCategory(useCat);
			cur_file->SetCatResumeOrder(useOrder);
		}
		else if (cur_file->GetCategory() > cat)
			cur_file->SetCategory(cur_file->GetCategory() - 1, false);
	}
}
// [TPT] - khaos::categorymod-

void CDownloadQueue::SetCatPrio(int cat, uint8 newprio)
{
	// [TPT] - itsonlyme: selFix
	CArray<CPartFile*> filesList;
	CPartFile* cur_file;
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;)
		filesList.Add(filelist.GetNext(pos)); // [TPT] - Code Improvement
	for (int i = 0; i < filesList.GetSize(); i++){
		cur_file = filesList[i];
		if (cat==0 || cur_file->GetCategory()==cat)
			if (newprio==PR_AUTO) {
				cur_file->SetAutoDownPriority(true);
				cur_file->SetDownPriority(PR_HIGH, false);
			}
			else {
				cur_file->SetAutoDownPriority(false);
				cur_file->SetDownPriority(newprio, false);
			}
	}
	// [TPT] - itsonlyme: selFix
	theApp.downloadqueue->SortByPriority();
	theApp.downloadqueue->CheckDiskspaceTimed();
}

// ZZ:DownloadManager -->
void CDownloadQueue::RemoveAutoPrioInCat(int cat, uint8 newprio){
	CPartFile* cur_file;
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;filelist.GetNext(pos)){
		cur_file = filelist.GetAt(pos);
        if (cur_file->IsAutoDownPriority() && (cat==0 || cur_file->GetCategory()==cat)) {
			cur_file->SetAutoDownPriority(false);
			cur_file->SetDownPriority(newprio, false);
		}
	}

    theApp.downloadqueue->SortByPriority();
	theApp.downloadqueue->CheckDiskspaceTimed(); // SLUGFILLER: checkDiskspace
}
// <-- ZZ:DownloadManager

void CDownloadQueue::SetCatStatus(int cat, int newstatus)
{
	bool reset = false;
    bool resort = false;

	POSITION pos= filelist.GetHeadPosition();
	while (pos != 0)
	{
		CPartFile* cur_file = filelist.GetAt(pos);
		if (!cur_file)
			continue;

		if (cat==-1 || 
			(cat==-2 && cur_file->GetCategory()==0) ||
			(cat==0 && cur_file->CheckShowItemInGivenCat(cat)) || 
			(cat>0 && cat==cur_file->GetCategory()))
		{
			switch (newstatus){
				case MP_CANCEL:
					cur_file->DeleteFile();
					reset = true;
					break;
				case MP_PAUSE:
					cur_file->PauseFile(false, false);
                    resort = true;
					break;
				case MP_STOP:
					cur_file->StopFile(false, false);
                    resort = true;
					break;
				case MP_RESUME: 
					if (cur_file->CanResumeFile()){
						if (cur_file->GetStatus() == PS_INSUFFICIENT)
							cur_file->ResumeFileInsufficient();
                        else {
							cur_file->ResumeFile(false);
                            resort = true;
                        }
					}
					break;
			}
		}
		filelist.GetNext(pos);
		if (reset)
		{
			reset = false;
			pos = filelist.GetHeadPosition();
		}
	}

    if(resort) {
	    theApp.downloadqueue->SortByPriority();
	    theApp.downloadqueue->CheckDiskspace(); // SLUGFILLER: checkDiskspace
    }
}

void CDownloadQueue::MoveCat(uint8 from, uint8 to)
{
	if (from < to)
		--to;

	POSITION pos= filelist.GetHeadPosition();
	while (pos != 0)
	{
		CPartFile* cur_file = filelist.GetAt(pos);
		if (!cur_file)
			continue;

		uint8 mycat = cur_file->GetCategory();
		if ((mycat>=min(from,to) && mycat<=max(from,to)))
		{
			//if ((from<to && (mycat<from || mycat>to)) || (from>to && (mycat>from || mycat<to)) )	continue; //not affected

			if (mycat == from)
				cur_file->SetCategory(to);
			else{
				if (from < to)
					cur_file->SetCategory(mycat - 1);
				else
					cur_file->SetCategory(mycat + 1);
			}
		}
		filelist.GetNext(pos);
	}
}

UINT CDownloadQueue::GetDownloadingFileCount() const
{
	UINT result = 0;
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){
		UINT uStatus = filelist.GetNext(pos)->GetStatus();
		if (uStatus == PS_READY || uStatus == PS_EMPTY)
			result++;
	}
	return result;
}

uint16 CDownloadQueue::GetPausedFileCount(){
	uint16 result = 0;
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){
		CPartFile* cur_file = filelist.GetNext(pos);
		if (cur_file->GetStatus() == PS_PAUSED)
			result++;
	}
	return result;
}

void CDownloadQueue::DisableAllA4AFAuto(void)
{
	for (POSITION pos = filelist.GetHeadPosition(); pos != NULL; )
		filelist.GetNext(pos)->SetA4AFAuto(false);
}

// [TPT] - khaos::categorymod
// HoaX_69: BEGIN AutoCat function
/*
void CDownloadQueue::SetAutoCat(CPartFile* newfile){
	if(thePrefs.GetCatCount()==1)
		return;
	CString catExt;

		for (int ix=1;ix<thePrefs.GetCatCount();ix++){	
		catExt= thePrefs.GetCategory(ix)->autocat;
		if (catExt.IsEmpty())
			continue;

		if (!thePrefs.GetCategory(ix)->ac_regexpeval) {
			// simple string comparison

			int curPos = 0;
			catExt.MakeLower();

			CString fullname = newfile->GetFileName();
			fullname.MakeLower();
			CString cmpExt = catExt.Tokenize(_T("|"), curPos);

			while (!cmpExt.IsEmpty()) {
				// HoaX_69: Allow wildcards in autocat string
				//  thanks to: bluecow, khaos and SlugFiller
				if(cmpExt.Find(_T("*")) != -1 || cmpExt.Find(_T("?")) != -1){
					// Use wildcards
					if(PathMatchSpec(fullname, cmpExt)){
						newfile->SetCategory(ix);
						return;
					}
				}else{
					if(fullname.Find(cmpExt) != -1){
						newfile->SetCategory(ix);
						return;
					}
				}
				cmpExt = catExt.Tokenize(_T("|"),curPos);
			}
		} else {
			// regular expression evaluation
			if (RegularExpressionMatch(catExt,newfile->GetFileName()))
				newfile->SetCategory(ix);
		}
		}
	}
}*/
// HoaX_69: END
// [TPT] - khaos::categorymod END

void CDownloadQueue::ResetLocalServerRequests()
{
	m_dwNextTCPSrcReq = 0;
	m_localServerReqQueue.RemoveAll();

	POSITION pos = filelist.GetHeadPosition();
	while (pos != NULL)
	{ 
		CPartFile* pFile = filelist.GetNext(pos);
		UINT uState = pFile->GetStatus();
		if (uState == PS_READY || uState == PS_EMPTY)
			pFile->ResumeFile();
		pFile->m_bLocalSrcReqQueued = false;
	}
}

void CDownloadQueue::RemoveLocalServerRequest(CPartFile* pFile)
{
	// [TPT] - Code Improvement
	for( POSITION pos = m_localServerReqQueue.GetHeadPosition(); pos != 0;)
	{
		POSITION cur_pos = pos;
		m_localServerReqQueue.GetNext(pos);
		if (m_localServerReqQueue.GetAt(cur_pos) == pFile)
		{
			m_localServerReqQueue.RemoveAt(cur_pos);
			pFile->m_bLocalSrcReqQueued = false;
			// could 'break' here.. fail safe: go through entire list..
		}
	}
}

void CDownloadQueue::ProcessLocalRequests()
{
	if ( (!m_localServerReqQueue.IsEmpty()) && (m_dwNextTCPSrcReq < ::GetTickCount()) )
	{
		CSafeMemFile dataTcpFrame(22);
		const int iMaxFilesPerTcpFrame = 15;
		int iFiles = 0;
		while (!m_localServerReqQueue.IsEmpty() && iFiles < iMaxFilesPerTcpFrame)
		{
			// find the file with the longest waitingtime
			uint32 dwBestWaitTime = 0xFFFFFFFF;
			POSITION posNextRequest = NULL;
			CPartFile* cur_file;
			// [TPT] - Code Improvement
			for(  POSITION pos = m_localServerReqQueue.GetHeadPosition(); pos != 0; ){
				POSITION cur_pos = pos;
				m_localServerReqQueue.GetNext(pos);
				cur_file = m_localServerReqQueue.GetAt(cur_pos);
				if (cur_file->GetStatus() == PS_READY || cur_file->GetStatus() == PS_EMPTY)
				{
					uint8 nPriority = cur_file->GetDownPriority();
					if (nPriority > PR_HIGH){
						ASSERT(0);
						nPriority = PR_HIGH;
					}

					if (cur_file->m_LastSearchTime + (PR_HIGH-nPriority) < dwBestWaitTime ){
						dwBestWaitTime = cur_file->m_LastSearchTime + (PR_HIGH-nPriority);
						posNextRequest = cur_pos;
					}
				}
				else{
					m_localServerReqQueue.RemoveAt(cur_pos);
					cur_file->m_bLocalSrcReqQueued = false;
					if (thePrefs.GetDebugSourceExchange())
						AddDebugLogLine(false, _T("SXSend: Local server source request for file \"%s\" not sent because of status '%s'"), cur_file->GetFileName(), cur_file->getPartfileStatus());
				}
			}
			
			if (posNextRequest != NULL)
			{
				cur_file = m_localServerReqQueue.GetAt(posNextRequest);
				cur_file->m_bLocalSrcReqQueued = false;
				cur_file->m_LastSearchTime = ::GetTickCount();
				m_localServerReqQueue.RemoveAt(posNextRequest);
				iFiles++;
				
				// create request packet
				Packet* packet = new Packet(OP_GETSOURCES,16);
				md4cpy(packet->pBuffer,cur_file->GetFileHash());
				if (thePrefs.GetDebugServerTCPLevel() > 0)
					Debug(_T(">>> Sending OP__GetSources(%2u/%2u); %s\n"), iFiles, iMaxFilesPerTcpFrame, DbgGetFileInfo(cur_file->GetFileHash()));
				dataTcpFrame.Write(packet->GetPacket(), packet->GetRealPacketSize());
				delete packet;

				if (thePrefs.GetDebugSourceExchange())
					AddDebugLogLine(false, _T("SXSend: Local server source request; File=\"%s\""), cur_file->GetFileName());
			}
		}

		int iSize = dataTcpFrame.GetLength();
		if (iSize > 0)
		{
			// create one 'packet' which contains all buffered OP_GETSOURCES eD2K packets to be sent with one TCP frame
			// server credits: 16*iMaxFilesPerTcpFrame+1 = 241
			Packet* packet = new Packet(new char[iSize], dataTcpFrame.GetLength(), true, false);
			dataTcpFrame.Seek(0, CFile::begin);
			dataTcpFrame.Read(packet->GetPacket(), iSize);
			theStats.AddUpDataOverheadServer(packet->size);
			theApp.serverconnect->SendPacket(packet, true);
		}

		// next TCP frame with up to 15 source requests is allowed to be sent in..
		m_dwNextTCPSrcReq = ::GetTickCount() + SEC2MS(iMaxFilesPerTcpFrame*(16+4));
	}
}

void CDownloadQueue::SendLocalSrcRequest(CPartFile* sender){
	ASSERT ( !m_localServerReqQueue.Find(sender) );
	m_localServerReqQueue.AddTail(sender);
}

void CDownloadQueue::GetDownloadStats(int results[],
									  uint64& rui64TotFileSize,
									  uint64& rui64TotBytesLeftToTransfer,
									  uint64& rui64TotNeededSpace)
{
	results[0] = 0;
	results[1] = 0;
	results[2] = 0;
	for (POSITION pos = filelist.GetHeadPosition();pos != 0; )
	{
		const CPartFile* cur_file = filelist.GetNext(pos);
		UINT uState = cur_file->GetStatus();
		if (uState == PS_READY || uState == PS_EMPTY)
		{
			uint32 ui32SizeToTransfer = 0;
			uint32 ui32NeededSpace = 0;
			cur_file->GetSizeToTransferAndNeededSpace(ui32SizeToTransfer, ui32NeededSpace);
			rui64TotFileSize += cur_file->GetFileSize();
			rui64TotBytesLeftToTransfer += ui32SizeToTransfer;
			rui64TotNeededSpace += ui32NeededSpace;
			results[2]++;
		}
		results[0] += cur_file->GetSourceCount();
		results[1]+=cur_file->GetTransferringSrcCount();
	}
}

///////////////////////////////////////////////////////////////////////////////
// CSourceHostnameResolveWnd

#define WM_HOSTNAMERESOLVED		(WM_USER + 0x101)	// does not need to be placed in "UserMsgs.h"

BEGIN_MESSAGE_MAP(CSourceHostnameResolveWnd, CWnd)
	ON_MESSAGE(WM_HOSTNAMERESOLVED, OnHostnameResolved)
END_MESSAGE_MAP()

CSourceHostnameResolveWnd::CSourceHostnameResolveWnd()
{
}

CSourceHostnameResolveWnd::~CSourceHostnameResolveWnd()
{
	while (!m_toresolve.IsEmpty())
		delete m_toresolve.RemoveHead();
}

void CSourceHostnameResolveWnd::AddToResolve(const uchar* fileid, LPCSTR pszHostname, uint16 port, LPCTSTR pszURL)
{
	bool bResolving = !m_toresolve.IsEmpty();

	// double checking
	if (!theApp.downloadqueue->GetFileByID(fileid))
		return;

	Hostname_Entry* entry = new Hostname_Entry;
	md4cpy(entry->fileid, fileid);
	entry->strHostname = pszHostname;
	entry->port = port;
	entry->strURL = pszURL;
	m_toresolve.AddTail(entry);

	if (bResolving)
		return;

	MEMZERO(m_aucHostnameBuffer, sizeof(m_aucHostnameBuffer));
	if (WSAAsyncGetHostByName(m_hWnd, WM_HOSTNAMERESOLVED, entry->strHostname, m_aucHostnameBuffer, sizeof m_aucHostnameBuffer) != 0)
		return;
	m_toresolve.RemoveHead();
	delete entry;
}

LRESULT CSourceHostnameResolveWnd::OnHostnameResolved(WPARAM wParam,LPARAM lParam)
{
	if (m_toresolve.IsEmpty())
		return TRUE;
	Hostname_Entry* resolved = m_toresolve.RemoveHead();
	if (WSAGETASYNCERROR(lParam) == 0)
	{
		int iBufLen = WSAGETASYNCBUFLEN(lParam);
		if (iBufLen >= sizeof(HOSTENT))
		{
			LPHOSTENT pHost = (LPHOSTENT)m_aucHostnameBuffer;
			if (pHost->h_length == 4 && pHost->h_addr_list && pHost->h_addr_list[0])
			{
				uint32 nIP = ((LPIN_ADDR)(pHost->h_addr_list[0]))->s_addr;

				CPartFile* file = theApp.downloadqueue->GetFileByID(resolved->fileid);
				if (file)
				{
					if (resolved->strURL.IsEmpty())
					{
					    CSafeMemFile sources(1+4+2);
					    sources.WriteUInt8(1);
					    sources.WriteUInt32(nIP);
					    sources.WriteUInt16(resolved->port);
					    sources.SeekToBegin();
					    file->AddSources(&sources,0,0);
				    }
					else
					{
						file->AddSource(resolved->strURL, nIP);
					}
				}
			}
		}
	}
	delete resolved;

	while (!m_toresolve.IsEmpty())
	{
		Hostname_Entry* entry = m_toresolve.GetHead();
		MEMZERO(m_aucHostnameBuffer, sizeof(m_aucHostnameBuffer));
		if (WSAAsyncGetHostByName(m_hWnd, WM_HOSTNAMERESOLVED, entry->strHostname, m_aucHostnameBuffer, sizeof m_aucHostnameBuffer) != 0)
			return TRUE;
		m_toresolve.RemoveHead();
		delete entry;
	}
	return TRUE;
}

bool CDownloadQueue::DoKademliaFileRequest()
{
	return ((::GetTickCount() - lastkademliafilerequest) > KADEMLIAASKTIME);
}

void CDownloadQueue::KademliaSearchFile(uint32 searchID, const Kademlia::CUInt128* pcontactID, const Kademlia::CUInt128* pbuddyID, uint8 type, uint32 ip, uint16 tcp, uint16 udp, uint32 serverip, uint16 serverport, uint32 clientid)
{
	//Safty measure to make sure we are looking for these sources
	CPartFile* temp = GetFileByKadFileSearchID(searchID);
	if( !temp )
		return;
	//Do we need more sources?
	if(!(!temp->IsStopped() && temp->GetMaxSourcesPerFile()/*thePrefs.GetMaxSourcePerFile()*/ > temp->GetSourceCount())) // [TPT] - Sivka AutoHL
		return;

	uint32 ED2Kip = ntohl(ip);
	if (theApp.ipfilter->IsFiltered(ED2Kip))
	{
		if (thePrefs.GetLogFilteredIPs())
			AddDebugLogLine(false, _T("IPfiltered source IP=%s (%s) received from Kademlia"), ipstr(ED2Kip), theApp.ipfilter->GetLastHit());
		return;
	}
	if( (ip == Kademlia::CKademlia::getIPAddress() || ED2Kip == theApp.serverconnect->GetClientID()) && tcp == thePrefs.GetPort())
		return;
	CUpDownClient* ctemp = NULL; 
	switch( type )
	{
		case 1:
		{
			//NonFirewalled users
			if(!tcp)
			{
				if (thePrefs.GetVerbose())
					AddDebugLogLine(false, _T("Ignored source (IP=%s) received from Kademlia, no tcp port received"), ipstr(ip));
				return;
			}
			if (!IsGoodIP(ED2Kip))
			{
				if (thePrefs.GetLogFilteredIPs())
					AddDebugLogLine(false, _T("Ignored source (IP=%s) received from Kademlia"), ipstr(ED2Kip));
				return;
			}
			ctemp = new CUpDownClient(temp,tcp,ip,0,0,false);
			ctemp->SetSourceFrom(SF_KADEMLIA);
			ctemp->SetServerIP(serverip);
			ctemp->SetServerPort(serverport);
			ctemp->SetKadPort(udp);
			byte cID[16];
			pcontactID->toByteArray(cID);
			ctemp->SetUserHash(cID);
			break;
		}
		case 2:
		{
			//Don't use this type... Some clients will process it wrong..
			break;
		}
		case 3:
		{
			//This will be a firewaled client connected to Kad only.
			//We set the clientID to 1 as a Kad user only has 1 buddy.
			ctemp = new CUpDownClient(temp,tcp,1,0,0,false);
			//The only reason we set the real IP is for when we get a callback
			//from this firewalled source, the compare method will match them.
			ctemp->SetSourceFrom(SF_KADEMLIA);
			ctemp->SetKadPort(udp);
			byte cID[16];
			pcontactID->toByteArray(cID);
			ctemp->SetUserHash(cID);
			pbuddyID->toByteArray(cID);
			ctemp->SetBuddyID(cID);
			ctemp->SetBuddyIP(serverip);
			ctemp->SetBuddyPort(serverport);
			break;
		}
	}

	if (ctemp)
		CheckAndAddSource(temp, ctemp);
}

// [TPT] - SLUGFILLER: indexPartFiles remove - obsolete from the moment of conception

void CDownloadQueue::OnConnectionState(bool bConnected)
{
	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; )
	{
		CPartFile* pPartFile = filelist.GetNext(pos);
		if (pPartFile->IsPartFile())
			pPartFile->SetActive(bConnected);
	}
}

// [TPT] - Sivka AutoHL Begin
void CDownloadQueue::InitTempVariables(CPartFile* file)
{
	thePrefs.SetTakeOverFileSettings(false);

	thePrefs.SetMaxSourcesPerFileTemp(file->GetMaxSourcesPerFile());
}

void CDownloadQueue::UpdateFileSettings(CPartFile* file)
{
	if(thePrefs.GetMaxSourcesPerFileTakeOver())
		file->SetMaxSourcesPerFile(thePrefs.GetMaxSourcesPerFileTemp());
}
// [TPT] - Sivka AutoHL end

// [TPT] - Add sources
void CDownloadQueue::AddSources(CED2KFileLink* pFileLink)
{
	const CKnownFile* file = theApp.sharedfiles->GetFileByID(pFileLink->GetHashKey());
	if (file)
	{
		if (file->IsPartFile())
			if(pFileLink->HasValidSources())
				((CPartFile*)file)->AddClientSources(pFileLink->SourcesList,1);
	}
	else if ((file = GetFileByID(pFileLink->GetHashKey())) != NULL)
	{
		if(pFileLink->HasValidSources())
			((CPartFile*)file)->AddClientSources(pFileLink->SourcesList,1);

		if (pFileLink->HasHostnameSources())
		{
			POSITION pos = pFileLink->m_HostnameSourcesList.GetHeadPosition();
			while (pos != NULL)
			{
				const SUnresolvedHostname* pUnresHost = pFileLink->m_HostnameSourcesList.GetNext(pos);
				m_srcwnd.AddToResolve(pFileLink->GetHashKey(), pUnresHost->strHostname, pUnresHost->nPort, pUnresHost->strURL);
			}
		}
	}
}
// [TPT] - Add sources

// [TPT] - MFCK [addon] - New Tooltips [Rayita]
void CDownloadQueue::GetTipInfoByCat(uint8 cat, CString &info)
{
	uint16 count,dwl,err,compl,paus;
	count=dwl=err=compl=paus=0;
	float speed = 0;
	uint64 size = 0;
	uint64 trsize = 0;
	uint64 disksize = 0;

	for (POSITION pos = filelist.GetHeadPosition(); pos; )
	{
		CPartFile* cur_file = filelist.GetNext(pos);
		//if (CheckShowItemInGivenCat(cur_file,cat)) // modified by rayita
		if (cur_file && cur_file->CheckShowItemInGivenCat(cat))
		{
			count++;
			if (cur_file->GetTransferringSrcCount()>0) ++dwl;
			speed += cur_file->GetDownloadFileDatarate()/1024.0f; // [TPT]
			size += cur_file->GetFileSize();
			trsize += cur_file->GetCompletedSize();
			disksize += cur_file->GetRealFileSize();
			if (cur_file->GetStatus()==PS_ERROR) ++err;
			if (cur_file->GetStatus()==PS_PAUSED) ++paus;
		}
	}

	int total;
	compl=theApp.emuledlg->transferwnd->downloadlistctrl.GetCompleteDownloads(cat,total);

	// ZZ:DownloadManager -->
	CString prio = GetResString(IDS_PRIONORMAL);
	if (thePrefs.GetCategory(cat) != NULL)
	{
		switch(thePrefs.GetCategory(cat)->prio) {
			case PR_LOW:
				prio = GetResString(IDS_PRIOLOW);
				break;

			case PR_HIGH:
				prio = GetResString(IDS_PRIOHIGH);
				break;		
		}
	}
// ZZ:DownloadManager <--

	info.Format(GetResString(IDS_TT_DLTAB_TL), thePrefs.GetCategory(cat) ? thePrefs.GetCategory(cat)->title : _T(""));
	info.AppendFormat(GetResString(IDS_TT_FILES), count);
	info.AppendFormat(GetResString(IDS_TT_DOWNLOADING), dwl);
	info.AppendFormat(GetResString(IDS_TT_PAUSED), paus);
	info.AppendFormat(GetResString(IDS_TT_ERRONEOUS), err);
	info.AppendFormat(GetResString(IDS_TT_COMPLETED), compl);
	info.AppendFormat(GetResString(IDS_TT_DLTAB_SP), speed);
	info.AppendFormat(GetResString(IDS_TT_DLTAB_SZ), CastItoXBytes(trsize, false, false),CastItoXBytes(size, false, false));
	info.AppendFormat(GetResString(IDS_TT_DLTAB_DS), CastItoXBytes(disksize, false, false));
	info.AppendFormat(_T("<br><b>Priority:</b><t>%s<br>"), prio); // ZZ:DownloadManager
}

void CDownloadQueue::GetTransferTipInfo(CString &info)
{
	uint16 count = 0;
	uint16 dwl = 0;	
	uint64 size = 0;
	uint64 trsize = 0;
	uint64 disksize = 0;

	for (POSITION pos = filelist.GetHeadPosition(); pos; )
	{
		CPartFile* cur_file = filelist.GetNext(pos);
		count++;
		if (cur_file->GetTransferringSrcCount() > 0) ++dwl;		
		size += cur_file->GetFileSize();
		trsize += cur_file->GetCompletedSize();
		disksize += cur_file->GetRealFileSize();
	}

	// [TPT] - Retrieve the current datarates
	uint32 eMuleIn;	uint32 eMuleInOverall;
	uint32 eMuleOut; uint32 eMuleOutOverall;
	uint32 notUsed;
	theApp.pBandWidthControl->GetDatarates(thePrefs.GetDatarateSamples(),
										   eMuleIn, eMuleInOverall,
										   eMuleOut, eMuleOutOverall,
										   notUsed, notUsed);

	info.Format(GetResString(IDS_TT_DL_SP), (float)eMuleIn/1024.0f, (float)(eMuleInOverall- eMuleIn) / 1024.0f);
	info.AppendFormat(GetResString(IDS_TT_DLTAB_DL), dwl, count);
	info.AppendFormat(GetResString(IDS_TT_DLTAB_SZ), CastItoXBytes(trsize, false, false), CastItoXBytes(size, false, false));
	info.AppendFormat(GetResString(IDS_TT_DLTAB_DS), CastItoXBytes(disksize, false, false));
}
// [TPT] - MFCK [addon] - New Tooltips [Rayita]

// [TPT] - WebCache
//jp webcache release START
bool CDownloadQueue::ContainsUnstoppedFiles()
{
	bool returnval = false;
	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; )
	{
		CPartFile* pPartFile = filelist.GetNext(pos);
		if (pPartFile->IsPartFile() && !pPartFile->IsStopped())
		{
			returnval = true;
			break; // [TPT] - Code Improvement
		}
	}
	return returnval;
}
//jp webcache release END
// [TPT] - WebCache