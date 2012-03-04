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
#include "Statistics.h"
#include "BandWidthControl.h" // [TPT]
#include "Preferences.h"
#include "Opcodes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

#ifdef _DEBUG
extern _CRT_ALLOC_HOOK g_pfnPrevCrtAllocHook;
#endif

// [TPT] - Maella rework

#define MAXAVERAGETIME			SEC2MS(40) //millisecs

///////////////////////////////////////////////////////////////////////////////
// CStatistics

CStatistics theStats;

// [TPT]
//float	CStatistics::maxDown;
//float	CStatistics::maxDownavg;
float	CStatistics::cumDownavg;
float	CStatistics::maxcumDownavg;
float	CStatistics::maxcumDown;
float	CStatistics::cumUpavg;
float	CStatistics::maxcumUpavg;
float	CStatistics::maxcumUp;
//float	CStatistics::maxUp;
//float	CStatistics::maxUpavg;
//float	CStatistics::rateDown;
//float	CStatistics::rateUp;
uint32	CStatistics::timeTransfers;
uint32	CStatistics::timeDownloads;
uint32	CStatistics::timeUploads;
uint32	CStatistics::start_timeTransfers;
uint32	CStatistics::start_timeDownloads;
uint32	CStatistics::start_timeUploads;
uint32	CStatistics::time_thisTransfer;
uint32	CStatistics::time_thisDownload;
uint32	CStatistics::time_thisUpload;
uint32	CStatistics::timeServerDuration;
uint32	CStatistics::time_thisServerDuration;
//uint32	CStatistics::m_nDownDatarateOverhead;
//uint32	CStatistics::m_nDownDataRateMSOverhead;
uint64	CStatistics::m_nDownDataOverheadSourceExchange;
uint64	CStatistics::m_nDownDataOverheadSourceExchangePackets;
uint64	CStatistics::m_nDownDataOverheadFileRequest;
uint64	CStatistics::m_nDownDataOverheadFileRequestPackets;
uint64	CStatistics::m_nDownDataOverheadServer;
uint64	CStatistics::m_nDownDataOverheadServerPackets;
uint64	CStatistics::m_nDownDataOverheadKad;
uint64	CStatistics::m_nDownDataOverheadKadPackets;
uint64	CStatistics::m_nDownDataOverheadOther;
uint64	CStatistics::m_nDownDataOverheadOtherPackets;
//uint32	CStatistics::m_nUpDatarateOverhead;
//uint32	CStatistics::m_nUpDataRateMSOverhead;
uint64	CStatistics::m_nUpDataOverheadSourceExchange;
uint64	CStatistics::m_nUpDataOverheadSourceExchangePackets;
uint64	CStatistics::m_nUpDataOverheadFileRequest;
uint64	CStatistics::m_nUpDataOverheadFileRequestPackets;
uint64	CStatistics::m_nUpDataOverheadServer;
uint64	CStatistics::m_nUpDataOverheadServerPackets;
uint64	CStatistics::m_nUpDataOverheadKad;
uint64	CStatistics::m_nUpDataOverheadKadPackets;
uint64	CStatistics::m_nUpDataOverheadOther;
uint64	CStatistics::m_nUpDataOverheadOtherPackets;
//uint32	CStatistics::m_sumavgDDRO;
//uint32	CStatistics::m_sumavgUDRO;

uint64	CStatistics::sessionReceivedBytes;
uint64	CStatistics::sessionSentBytes;
uint64	CStatistics::sessionSentBytesToFriend;
uint16	CStatistics::reconnects;
DWORD	CStatistics::transferStarttime;
DWORD	CStatistics::serverConnectTime;
uint32	CStatistics::filteredclients;
DWORD	CStatistics::starttime;
// [TPT]
float	CStatistics::currentUploadRate;
float	CStatistics::currentMaxUploadRate;
float	CStatistics::sessionUploadRate;
float	CStatistics::sessionMaxUploadRate;
float	CStatistics::currentDownloadRate;
float	CStatistics::currentMaxDownloadRate;
float	CStatistics::sessionDownloadRate;
float	CStatistics::sessionMaxDownloadRate;


CStatistics::CStatistics()
{
	// [TPT]
	//maxDown =				0;
	//maxDownavg =			0;
	maxcumDown =			0;
	cumUpavg =				0;
	maxcumDownavg =			0;
	cumDownavg =			0;
	maxcumUpavg =			0;
	maxcumUp =				0;
	//maxUp =					0;
	//maxUpavg =				0;
	//rateDown =				0;
	//rateUp =				0;
	timeTransfers =			0;
	timeDownloads =			0;
	timeUploads =			0;
	start_timeTransfers =	0;
	start_timeDownloads =	0;
	start_timeUploads =		0;
	time_thisTransfer =		0;
	time_thisDownload =		0;
	time_thisUpload =		0;
	timeServerDuration =	0;
	time_thisServerDuration=0;

	sessionReceivedBytes=0;
	sessionSentBytes=0;
    sessionSentBytesToFriend=0;
	reconnects=0;
	transferStarttime=0;
	serverConnectTime=0;
	filteredclients=0;
	starttime=0;
	
	//m_nDownDataRateMSOverhead = 0;
	//m_nDownDatarateOverhead = 0;
	m_nDownDataOverheadSourceExchange = 0;
	m_nDownDataOverheadSourceExchangePackets = 0;
	m_nDownDataOverheadFileRequest = 0;
	m_nDownDataOverheadFileRequestPackets = 0;
	m_nDownDataOverheadServer = 0;
	m_nDownDataOverheadServerPackets = 0;
	m_nDownDataOverheadKad = 0;
	m_nDownDataOverheadKadPackets = 0;
	m_nDownDataOverheadOther = 0;
	m_nDownDataOverheadOtherPackets = 0;
	//m_sumavgDDRO = 0;

	//m_nUpDataRateMSOverhead = 0;
	//m_nUpDatarateOverhead = 0;
	m_nUpDataOverheadSourceExchange = 0;
	m_nUpDataOverheadSourceExchangePackets = 0;
	m_nUpDataOverheadFileRequest = 0;
	m_nUpDataOverheadFileRequestPackets = 0;
	m_nUpDataOverheadServer = 0;
	m_nUpDataOverheadServerPackets = 0;
	m_nUpDataOverheadKad = 0;
	m_nUpDataOverheadKadPackets = 0;
	m_nUpDataOverheadOther = 0;
	m_nUpDataOverheadOtherPackets = 0;
	//m_sumavgUDRO = 0;
	
	// [TPT] - Maella Bandwidth
	currentUploadRate = 0;
	currentMaxUploadRate = 0;
	sessionUploadRate = 0;
	sessionMaxUploadRate = 0;

	currentDownloadRate = 0;
	currentMaxDownloadRate = 0;
	sessionDownloadRate = 0;
	sessionMaxDownloadRate = 0;
}

void CStatistics::Init()
{
	maxcumDown =			thePrefs.GetConnMaxDownRate();
	cumUpavg =				thePrefs.GetConnAvgUpRate();
	maxcumDownavg =			thePrefs.GetConnMaxAvgDownRate();
	cumDownavg =			thePrefs.GetConnAvgDownRate();
	maxcumUpavg =			thePrefs.GetConnMaxAvgUpRate();
	maxcumUp =				thePrefs.GetConnMaxUpRate();
}

// This function is going to basically calculate and save a bunch of averages.
//				I made a seperate funtion so that it would always run instead of having
//				the averages not be calculated if the graphs are disabled (Which is bad!).
void CStatistics::UpdateConnectionStats(void)
{
	// Wait at least 5 seconds
	if(::GetTickCount() - theApp.pBandWidthControl->GetStartTick() + 5000)
	{

		// Maella -Graph: code Improvement for rate display-
		uint32 plotOutData[ADAPTER+1];
		uint32 plotinData[ADAPTER+1];
		theApp.pBandWidthControl->GetDatarates(1,
											plotinData[CURRENT], plotinData[OVERALL],
											plotOutData[CURRENT], plotOutData[OVERALL],
											plotinData[ADAPTER], plotOutData[ADAPTER]);

		theApp.pBandWidthControl->GetFullHistoryDatarates(plotinData[MINUTE], plotOutData[MINUTE],
														  plotinData[SESSION], plotOutData[SESSION]);

		currentDownloadRate = (float)plotinData[CURRENT]/1024.0;
		sessionDownloadRate = (float)plotinData[SESSION]/1024.0;
		currentUploadRate   = (float)plotOutData[CURRENT]/1024.0;
		sessionUploadRate   = (float)plotOutData[SESSION]/1024.0;
		// Maella end

		// Max Current rates (graph refresh)
		if(currentMaxUploadRate < currentUploadRate) 
			currentMaxUploadRate = currentUploadRate;
		if(currentMaxDownloadRate < currentDownloadRate) 
			currentMaxDownloadRate = currentDownloadRate;

		// Session Max rates
		if(sessionMaxUploadRate < sessionUploadRate) 
			sessionMaxUploadRate = sessionUploadRate;
		if(sessionMaxDownloadRate < sessionDownloadRate) 
			sessionMaxDownloadRate = sessionDownloadRate;

		// Cumulative Max Current rates (graph refresh)
		if(maxcumUp < currentMaxUploadRate) 
		{
			maxcumUp = currentMaxUploadRate;
			thePrefs.SetConnMaxUpRate(maxcumUp);
		}

		if(maxcumDown < currentMaxDownloadRate) 
		{
			maxcumDown = currentMaxDownloadRate;
			thePrefs.SetConnMaxDownRate(maxcumDown);
		}

		// Cumulative Max Average rates
		cumDownavg = (sessionDownloadRate + thePrefs.GetConnAvgDownRate()) / 2;
		cumUpavg = (sessionUploadRate + thePrefs.GetConnAvgUpRate()) / 2;
		if(maxcumDownavg < cumDownavg) 
		{
			maxcumDownavg = cumDownavg;
			thePrefs.SetConnMaxAvgDownRate(maxcumDownavg);
		}

		if(maxcumUpavg < cumUpavg) 
		{
			maxcumUpavg = cumUpavg;
			thePrefs.SetConnMaxAvgUpRate(maxcumUpavg);
		}
		
		// Transfer Times (Increment Session)
		if (sessionUploadRate > 0.0f || sessionDownloadRate > 0.0f) 
		{
			if (start_timeTransfers == 0)
				start_timeTransfers = GetTickCount();
			else
				time_thisTransfer = (GetTickCount() - start_timeTransfers) / 1000;

			if (sessionUploadRate > 0.0f) 
			{
				if (start_timeUploads == 0)
					start_timeUploads = GetTickCount();
				else
					time_thisUpload = (GetTickCount() - start_timeUploads) / 1000;
			}
			
			if (sessionDownloadRate > 0.0f) 
			{
				if (start_timeDownloads == 0)
					start_timeDownloads = GetTickCount();
				else
					time_thisDownload = (GetTickCount() - start_timeDownloads) / 1000;
			}
		}

		if (sessionUploadRate == 0.0f && sessionDownloadRate == 0.0f && (time_thisTransfer > 0 || start_timeTransfers > 0)) 
		{
			timeTransfers += time_thisTransfer;
			time_thisTransfer = 0;
			start_timeTransfers = 0;
		}

		if (sessionUploadRate == 0.0f && (time_thisUpload > 0 || start_timeUploads > 0)) 
		{
			timeUploads += time_thisUpload;
			time_thisUpload = 0;
			start_timeUploads = 0;
		}

		if (sessionDownloadRate == 0.0f && (time_thisDownload > 0 || start_timeDownloads > 0)) 
		{
			timeDownloads += time_thisDownload;
			time_thisDownload = 0;
			start_timeDownloads = 0;
		}

		// Server Durations
	if (theStats.serverConnectTime == 0) 
			time_thisServerDuration = 0;
		else
			time_thisServerDuration = (GetTickCount() - theStats.serverConnectTime) / 1000;
	}
}
// <-----khaos-


