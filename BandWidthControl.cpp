//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
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

#include "StdAfx.h"
#include "BandWidthControl.h"
#include "Emule.h"
#include "opcodes.h"
#include "preferences.h"
#include "Log.h"
#include "uploadbandwidththrottler.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CBandWidthControl::CBandWidthControl()
:	m_statisticHistory(1024) // size  ~= 1024*(4*8+3*4) = 1024*44 bytes = 44 KBytes
{
	m_statistic.eMuleOutOctets = 0;
	m_statistic.eMuleInOctets = 0;
	m_statistic.eMuleOutOverallOctets = 0;
	m_statistic.eMuleInOverallOctets = 0;
	m_statistic.networkOutOctets = 0;
	m_statistic.networkInOctets = 0;	
	m_statistic.eMuleOutFriendOctets = 0; // [TPT] - Friend data
	m_statistic.timeStamp = ::GetTickCount(); 
    m_maxDownloadLimit = 0.0f;
    m_errorTraced = false;

    // [TPT] - Xman
    // Keep last result to detect an overflow
    m_networkOutOctets = 0;
    m_networkInOctets = 0;
    // [TPT] - Xman

	// Maella -Pseudo overhead datarate control-
	m_lastProcessTime = ::GetTickCount();
	m_lastSentBytes = 0;
	m_lastOverallSentBytes = 0;
    m_lastNetworkOut = 0;
	m_nUploadSlopeControl = 0;

	// Dynamic load library iphlpapi.dll => user of win95
	m_hIphlpapi = ::LoadLibrary(_T("iphlpapi.dll"));
	if(m_hIphlpapi != NULL){
		m_fGetIfTable = (GETIFTABLE)GetProcAddress(m_hIphlpapi, "GetIfTable");
        m_fGetIpAddrTable = (GETIPADDRTABLE)GetProcAddress(m_hIphlpapi, "GetIpAddrTable");
		m_fGetIfEntry = (GETIFENTRY)GetProcAddress(m_hIphlpapi, "GetIfEntry");
		m_fGetNumberOfInterfaces = (GETNUMBEROFINTERFACES)GetProcAddress(m_hIphlpapi, "GetNumberOfInterfaces");

		if (thePrefs.GetVerbose()) AddPhoenixLogLine(false, GetResString(IDS_NAFCSUCCESS)); // [TPT] - Debug log
	}
	else {
		m_fGetIfTable = NULL;
		m_fGetIfEntry = NULL;
		m_fGetNumberOfInterfaces = NULL;

        if (thePrefs.GetVerbose()) PhoenixLogError(false, GetResString(IDS_NAFCFAIL)); // [TPT] - Debug log
	}

	// Cache index value
	m_currentAdapterIndex = thePrefs.GetNAFCSelection(); // [TPT] - NAFC Selection
	selectedIndex = 0;
	selectedDescription = _T(" ");
  	wasNAFCLastActive=thePrefs.GetNAFCFullControl();


	// [TPT] - NAFC Selection
	if ((m_currentAdapterIndex != 0) && (checkAdapterIndex(m_currentAdapterIndex) == false))
		m_currentAdapterIndex = 0;
	// [TPT] - NAFC Selection
}

CBandWidthControl::~CBandWidthControl(){
	// Unload library
	if(m_hIphlpapi != NULL){
		::FreeLibrary(m_hIphlpapi);
	}
}

// [TPT] - NAFC Selection
bool CBandWidthControl::checkAdapterIndex(DWORD index)
{
    // Check if the library was successfully loaded
	if(m_fGetNumberOfInterfaces != NULL && m_fGetIfTable != NULL && m_fGetIpAddrTable != NULL)
	{
		DWORD dwNumIf = 0;
		if(m_fGetNumberOfInterfaces(&dwNumIf) == NO_ERROR && dwNumIf > 0 )
		{
			BYTE buffer[10*sizeof(MIB_IFROW)];
			ULONG size = sizeof(buffer);
			MIB_IFTABLE& mibIfTable = reinterpret_cast<MIB_IFTABLE&>(buffer[0]);
			if(m_fGetIfTable(&mibIfTable, &size, true) == NO_ERROR)
			{ 
				for(DWORD dwNumEntries = 0; dwNumEntries < mibIfTable.dwNumEntries; dwNumEntries++)
				{
					const MIB_IFROW& mibIfRow = mibIfTable.table[dwNumEntries];
                    if (mibIfRow.dwIndex == index)
						return true;			    
                }
			}
		}
	}
	return false;
}
// [TPT] - NAFC Selection

DWORD CBandWidthControl::getAdapterIndex(){
   // Check if the library was successfully loaded
	if(m_fGetNumberOfInterfaces != NULL && m_fGetIfTable != NULL && m_fGetIpAddrTable != NULL){
		DWORD dwNumIf = 0;
		if(m_fGetNumberOfInterfaces(&dwNumIf) == NO_ERROR && dwNumIf > 0 ){
			BYTE buffer[10*sizeof(MIB_IFROW)];
			ULONG size = sizeof(buffer);
			MIB_IFTABLE& mibIfTable = reinterpret_cast<MIB_IFTABLE&>(buffer[0]);
			if(m_fGetIfTable(&mibIfTable, &size, true) == NO_ERROR){
                // Trace list of Adapters
                if(m_errorTraced == false){
				    for(DWORD dwNumEntries = 0; dwNumEntries < mibIfTable.dwNumEntries; dwNumEntries++){
                        const MIB_IFROW& mibIfRow = mibIfTable.table[dwNumEntries];
                        if (thePrefs.GetVerbose()) AddPhoenixLogLine(false, GetResString(IDS_NAFCADAPTER), mibIfRow.dwIndex, (CString)mibIfRow.bDescr); // [TPT] - Debug log
				    }
                }

                // Retrieve the default used IP (=> in case of multiple adapters)
                char hostName[256];
                if(gethostname(hostName, sizeof(hostName)) == 0){
                    hostent* lphost = gethostbyname(hostName);
                    if(lphost != NULL){
                        DWORD dwAddr = ((LPIN_ADDR)lphost->h_addr)->s_addr;
                        // Pick the interface matching the IP
		                BYTE buffer[10*sizeof(MIB_IPADDRROW)];
		                ULONG size = sizeof(buffer);
		                MIB_IPADDRTABLE& mibIPAddrTable = reinterpret_cast<MIB_IPADDRTABLE&>(buffer[0]);
                        if(m_fGetIpAddrTable(&mibIPAddrTable, &size, FALSE) == 0){
                            for(DWORD i = 0; i < mibIPAddrTable.dwNumEntries; i++){
                                if(mibIPAddrTable.table[i].dwAddr == dwAddr){
                                    const MIB_IPADDRROW& row = mibIPAddrTable.table[i];

                                    m_errorTraced = false;
                                    if (thePrefs.GetVerbose()) AddPhoenixLogLine(false, GetResString(IDS_NAFCADAPTERINDEX), mibIPAddrTable.table[i].dwIndex);
                                    return mibIPAddrTable.table[i].dwIndex;
                                }
                            }
                        }
                        else {
                            if(m_errorTraced == false){
                                m_errorTraced = true;
                                if (thePrefs.GetVerbose()) PhoenixLogError(GetResString(IDS_NAFCIPTABLES), ::GetLastError()); // [TPT] - Debug log
                                return 0;
                            }
                        }
                    }
                }
			}
            if(m_errorTraced == false){
                m_errorTraced = true;
                if (thePrefs.GetVerbose()) PhoenixLogError(GetResString(IDS_NAFCTABLEINT), ::GetLastError()); // [TPT] - Debug log
                return 0;
            }
		}
        if(m_errorTraced == false){
            m_errorTraced = true;
            if (thePrefs.GetVerbose()) PhoenixLogError(GetResString(IDS_NAFCINTNUMBER), ::GetLastError()); // [TPT] - Debug log
            return 0;
        }
	}
	return 0;
}

void CBandWidthControl::Process()
{
	static DWORD processtime;
	if(::GetTickCount()-processtime >= 1000)
	{
		processtime = ::GetTickCount();

		// Try to get the Adapter Index to access to the right interface
		if(m_currentAdapterIndex == 0){
		    m_currentAdapterIndex = getAdapterIndex();

			//Xman new adapter selection
		    // Disable NAFC
			if(m_currentAdapterIndex == 0){
				wasNAFCLastActive=thePrefs.GetNAFCFullControl();
				thePrefs.SetNAFCFullControl(false);
			}
			else
			{
				if(wasNAFCLastActive)
					thePrefs.SetNAFCFullControl(true);
		    }
			//Xman end
	    }

		/*->*/m_statisticLocker.Lock();
		/**/ // Update the datarate directly from the network Adapter
		/**/ if(m_currentAdapterIndex != 0 && m_fGetIfEntry != NULL){
		/**/ 	static int s_Log; // Static initiate with zero
		/**/ 	MIB_IFROW ifRow;       
		/**/ 	ifRow.dwIndex = m_currentAdapterIndex;
		/**/ 	if(m_fGetIfEntry(&ifRow) == NO_ERROR){
		/**/ 		s_Log = 0;
		/**/ 
		/**/	// Add the delta, since the last measure (convert 32 to 64 bits)
		/**/	m_statistic.networkInOctets += (DWORD)(ifRow.dwInOctets - m_networkInOctets);
		/**/	m_statistic.networkOutOctets += (DWORD)(ifRow.dwOutOctets - m_networkOutOctets);
		/**/ 
		/**/	// Keep last measure
		/**/	m_networkOutOctets = ifRow.dwOutOctets; 
		/**/	m_networkInOctets = ifRow.dwInOctets;
		/**/ 	}
		/**/ 	else {
		/**/ 		if(s_Log == 0){
		/**/ 			s_Log = 1;
		/**/ 			PhoenixLogError(GetResString(IDS_NAFCTRAFFIC), ::GetLastError()); // [TPT] - Debug log
		/**/ 		}
		/**/ 
		/**/ 		// Disable NAFC
		/**/ 		thePrefs.SetNAFCFullControl(false);
		/**/ 	}
		/**/ }
		/**/ 
		/**/ // Update large history list for the history graph
		/**/ m_statisticHistory.AddHead(Statistic(m_statistic, ::GetTickCount()));
		/**/ 
		/**/// Trunk size of the list (The timestamp is more accurate than the period of Process())
		/**/const uint32 averageMinTime = (uint32)(60000 * thePrefs.GetStatsAverageMinutes());
		/**/while(true){
		/**/	const uint32 deltaTime = m_statisticHistory.GetHead().timeStamp - m_statisticHistory.GetTail().timeStamp;
		/**/	if(deltaTime <= averageMinTime){
		/**/		break; // exit loop
		/**/	}
		/**/	m_statisticHistory.RemoveTail(); // Trunk size
		/**/}
		/*->*/ m_statisticLocker.Unlock();

		// Calculate the dynamic download limit
        m_maxDownloadLimit = thePrefs.GetMaxDownload();

		// [TPT] - Xman Bugfix
        //if(thePrefs.GetNAFCEnable() == true && thePrefs.GetNAFCFullControl() == true)
        //{
        //    // Remark: Because there is only ONE writer thread to m_statisticHistory, there is no need to 
        //    //         protect its access inside this method. 
        //    //         => m_statisticHistory is only modified inside this method
        //    //         => Its access in GetDatarates() + GetFullHistoryDatarates() still must be protected
        //    if(m_statisticHistory.GetSize() >= 2)
        //    {
        //        const Statistic& newestSample = m_statisticHistory.GetHead();
        //        const Statistic& oldestSample = m_statisticHistory.GetAt(m_statisticHistory.FindIndex(1));

        //        const uint32 deltaTime = (newestSample.timeStamp - oldestSample.timeStamp); // in [ms]
        //        if (deltaTime == 0) 
        //        {
        //            if(thePrefs.GetBlockMaellaSpecificDebugMsg() == false && thePrefs.GetVerbose()) 
        //                PhoenixLogError(_T("Process error in bandwidth control. Deltatime = 0"));                   
        //            return;
        //        }
        //        const uint32 eMuleOutOverall = (1000 * (uint32)(newestSample.eMuleOutOverallOctets - oldestSample.eMuleOutOverallOctets) / deltaTime); // in [Bytes/s]
        //        const uint32 networkOut = (1000 * (uint32)(newestSample.networkOutOctets - oldestSample.networkOutOctets) / deltaTime); // in [Bytes/s]

        //        // Dynamic download limit with ratio
        //        if(eMuleOutOverall < 4*1024)
        //        {
        //            // Ratio 3x
        //            float maxDownloadLimit = (float)(3*eMuleOutOverall) / 1024.0f; // [KB/s]
        //            if(maxDownloadLimit < m_maxDownloadLimit)
        //                m_maxDownloadLimit = maxDownloadLimit;
        //            }
        //        else if(eMuleOutOverall < 10*1024)
        //        {
        //            // Ratio 4x
        //            float maxDownloadLimit = (float)(4*eMuleOutOverall) / 1024.0f; // [KB/s]
        //            if(maxDownloadLimit < m_maxDownloadLimit)
        //                m_maxDownloadLimit = maxDownloadLimit;
        //            }

        //        if(m_maxDownloadLimit < 1.0f)
        //            m_maxDownloadLimit = 1.0f;

        //        if(thePrefs.GetBlockMaellaSpecificDebugMsg() == false)
        //        {
        //            if (thePrefs.GetVerbose()) AddPhoenixLogLine(false, GetResString(IDS_NAFCNEWLIMITS),  // [TPT] - Debug log
        //                                                         GetMaxDownload(), 
        //                                                         thePrefs.GetMaxUpload(),
        //                                                         eMuleOutOverall,
        //                                                         networkOut);
        //        }
        //    }
        //}
	}
	//else of: tickount
	else if(thePrefs.GetNAFCEnable() == true && thePrefs.GetNAFCFullControl() == true )
	{
        // Retrieve the network flow => necessary for the NAFC with 'Auto U/D limit'
		if(m_currentAdapterIndex != 0 && m_fGetIfEntry != NULL){
		    MIB_IFROW ifRow;        
		    ifRow.dwIndex = m_currentAdapterIndex;
			if(m_fGetIfEntry(&ifRow) == NO_ERROR){
				
              // Add the delta, since the last measure (convert 32 to 64 bits)
				m_statisticLocker.Lock();  
				/**/ m_statistic.networkInOctets += (DWORD)(ifRow.dwInOctets - m_networkInOctets);
				/**/ m_statistic.networkOutOctets += (DWORD)(ifRow.dwOutOctets - m_networkOutOctets);
				m_statisticLocker.Unlock();

              // Keep last measure
				m_networkOutOctets = ifRow.dwOutOctets;
				m_networkInOctets = ifRow.dwInOctets;
		    }
	    }
    }

		ProcessUpload();
}

void CBandWidthControl::GetDatarates(uint8 samples, 
									 uint32& eMuleIn, uint32& eMuleInOverall, 
									 uint32& eMuleOut, uint32& eMuleOutOverall,
									 uint32& networkIn, uint32& networkOut) const 
{
	eMuleIn = 0;
	eMuleInOverall = 0;
	eMuleOut = 0;
	eMuleOutOverall = 0;
	networkIn = 0;
	networkOut = 0;

    // Check if the list is already long enough
	/*->*/ m_statisticLocker.Lock();
    /**/ if(m_statisticHistory.GetSize() >= 2 && samples >= 1){
	/**/ 
    /**/	// Retrieve the location of the n previous sample
    /**/	POSITION pos = m_statisticHistory.FindIndex(samples);
    /**/    if(pos == NULL){
    /**/		pos = m_statisticHistory.GetTailPosition();
    /**/    }           
	/**/
    /**/    const Statistic& newestSample = m_statisticHistory.GetHead();
    /**/    const Statistic& oldestSample = m_statisticHistory.GetAt(pos);
    /**/    const uint32 deltaTime = (newestSample.timeStamp - oldestSample.timeStamp); // in [ms]
    /**/     
    /**/    if(deltaTime > 0){
    /**/		eMuleIn = (uint32)(1000 * (newestSample.eMuleInOctets - oldestSample.eMuleInOctets) / deltaTime); // in [Bytes/s]
    /**/        eMuleInOverall = (uint32)(1000 * (newestSample.eMuleInOverallOctets - oldestSample.eMuleInOverallOctets) / deltaTime); // in [Bytes/s]
    /**/        eMuleOut = (uint32)(1000 * (newestSample.eMuleOutOctets - oldestSample.eMuleOutOctets) / deltaTime); // in [Bytes/s]
    /**/        eMuleOutOverall = (uint32)(1000 * (newestSample.eMuleOutOverallOctets - oldestSample.eMuleOutOverallOctets) / deltaTime); // in [Bytes/s]
    /**/        networkIn = (uint32)(1000 * (newestSample.networkInOctets - oldestSample.networkInOctets) / deltaTime); // in [Bytes/s]
    /**/        networkOut = (uint32)(1000 * (newestSample.networkOutOctets - oldestSample.networkOutOctets) / deltaTime); // in [Bytes/s]
    /**/    }
    /**/ }
    /*->*/ m_statisticLocker.Unlock();
}

void CBandWidthControl::GetFullHistoryDatarates(uint32& eMuleInHistory, uint32& eMuleOutHistory, 
                                                uint32& eMuleInSession, uint32& eMuleOutSession) const 
{
	eMuleInHistory = 0;
	eMuleOutHistory = 0;
	eMuleInSession = 0;
	eMuleOutSession = 0;

    // Check if the list is already long enough
   /*->*/ m_statisticLocker.Lock();
    /**/ if(m_statisticHistory.GetSize() >= 2){
	/**/
    /**/	const Statistic& newestSample = m_statisticHistory.GetHead();
    /**/    const Statistic& oldestSample = m_statisticHistory.GetTail();
	/**/
    /**/    // Average value since the last n minutes
    /**/    uint32 deltaTime = (newestSample.timeStamp - oldestSample.timeStamp); // in [ms]
    /**/    if(deltaTime > 0){
    /**/        eMuleInHistory = (uint32)(1000 * (newestSample.eMuleInOctets - oldestSample.eMuleInOctets) / deltaTime); // in [Bytes/s]
    /**/        eMuleOutHistory = (uint32)(1000 * (newestSample.eMuleOutOctets - oldestSample.eMuleOutOctets) / deltaTime); // in [Bytes/s]
    /**/    }
    /**/ 
    /**/    // Average value since the start of the client
    /**/    deltaTime = (::GetTickCount() - GetStartTick()) / 1000; // in [s]
    /**/    if(deltaTime > 0){
    /**/        eMuleInSession = (uint32)(newestSample.eMuleInOctets / deltaTime); // in [Bytes/s]
    /**/        eMuleOutSession = (uint32)(newestSample.eMuleOutOctets / deltaTime); // in [Bytes/s]
    /**/    }
    /**/ }
   /*->*/ m_statisticLocker.Unlock();
}

// [TPT] - Friend data
void CBandWidthControl::GetFriendDataRate(uint8 samples, uint32& friendOut) const 
{
	m_statisticLocker.Lock();
	// Check if the list is already long enough
	if(m_statisticHistory.GetSize() > 2 && samples >= 1)
	{
		// Retieve the location of the n previous sample
		POSITION pos = m_statisticHistory.FindIndex(samples);
		if(pos == NULL){
			pos = m_statisticHistory.GetTailPosition();
		}
			
		const Statistic& newestSample = m_statisticHistory.GetHead();
		const Statistic& oldestSample = m_statisticHistory.GetAt(pos);
		const uint32 deltaTime = (newestSample.timeStamp - oldestSample.timeStamp); // in [ms]	
		if(deltaTime > 0){
			friendOut = (uint32)(1000 * (newestSample.eMuleOutFriendOctets - oldestSample.eMuleOutFriendOctets) / deltaTime); // in [Bytes/s]
			 m_statisticLocker.Unlock();
			return; // over
		}
	}
	 m_statisticLocker.Unlock();
	friendOut = 0;	
}
// [TPT] - Friend data

void CBandWidthControl::ProcessUpload()
{
	// Elapsed time (TIMER_PERIOD not accurate) 
	uint32 deltaTime = ::GetTickCount() - m_lastProcessTime;
	m_lastProcessTime += deltaTime;

    // Anticipate high CPU load => unregular cycle
	if(deltaTime > 0)
	{       
		// EMule overhead
		sint32 overhead;

		// Update the slope
		float maxUpload = thePrefs.GetMaxUpload();
		m_nUploadSlopeControl += (uint32)(maxUpload * 1.024f * (float)deltaTime); // [bytes/period]         

		if(thePrefs.GetNAFCEnable() == true && thePrefs.GetNAFCFullControl() == true)
		{
			// Correct the slope with the bytes sent during the last cycle
			m_statisticLocker.Lock();
			/**/ m_nUploadSlopeControl -= (m_statistic.networkOutOctets - m_lastNetworkOut);            
			/**/ overhead = (m_statistic.eMuleOutOverallOctets - m_lastOverallSentBytes) - (m_statistic.eMuleOutOctets - m_lastSentBytes);
			/**/ m_lastOverallSentBytes = m_statistic.eMuleOutOverallOctets;
			/**/ m_lastSentBytes = m_statistic.eMuleOutOctets;
			/**/ m_lastNetworkOut = m_statistic.networkOutOctets;
			m_statisticLocker.Unlock();
		}
		else 
		{
			// Correct the slope with the bytes sent during the last cycle
			m_statisticLocker.Lock();
			/**/ m_nUploadSlopeControl -= (uint32)(m_statistic.eMuleOutOctets - m_lastSentBytes);
			/**/ overhead = (m_statistic.eMuleOutOverallOctets - m_lastOverallSentBytes) - (m_statistic.eMuleOutOctets - m_lastSentBytes);
			/**/ m_lastOverallSentBytes = m_statistic.eMuleOutOverallOctets;
			/**/ m_lastSentBytes = m_statistic.eMuleOutOctets;
			/**/ m_lastNetworkOut = m_statistic.networkOutOctets;
			m_statisticLocker.Unlock();
		}

		// Compensate up to 1 second
		if(m_nUploadSlopeControl > (sint32)(1024.0f * maxUpload))
		{
			m_nUploadSlopeControl = (sint32)(1024.0f * maxUpload);

			// But be sure to be able to send at least one packet
			if(m_nUploadSlopeControl < (thePrefs.GetMTU() - 40/*TCP+IP headers*/))
			{
				m_nUploadSlopeControl = thePrefs.GetMTU() - 40/*TCP+IP headers*/;
			}
		}

        // Trunk negative value (up to 0.5 second) 
        // => possible when Overhead compensation activated
        if(m_nUploadSlopeControl < (sint32)(-0.5 * 1024.0f * maxUpload))
        {
            m_nUploadSlopeControl = (sint32)(-0.5 * 1024.0f * maxUpload);
        }

		// Anticipate overhead (80%)
		sint32 nUploadSlopeControl = m_nUploadSlopeControl;
		if(thePrefs.GetNAFCEnable() == true && thePrefs.GetNAFCFullControl() == true)
		{
			sint32 overhead80 = 8 * overhead / 10;
			if(overhead80 > 0)                
				nUploadSlopeControl -= overhead80;
		}

		// Correct datarate with Ratio between period/elapsed time 
		// => wrong: if is the CPU is overloaded
		//           e.g. deltaTime = 200 [ms] instead of 100 [ms] => datarate = 1/2 of settings
        // float datarate = (float)nUploadSlopeControl/((float)deltaTime/TIMER_PERIOD); 

        // Reserve 20% of the upload (min 1KB) for the ctrl packets
        // Remark: With the previous versions of eMule, the bandwidth control  
        //         applied only to the data packets, but not the ctrls
        float datarate = (nUploadSlopeControl > (0.2f * 1024.0f) * maxUpload) ? 
                         (float)nUploadSlopeControl : ((0.2f * 1024.0f) * maxUpload);
        if(datarate < 1024) datarate = 1024;

		// Set new 'instantanious' data rate
        // Remark: a zero is interpreted as 'unlimited'
		theApp.uploadBandwidthThrottler->SetNAFCdatarate(datarate);

		#ifdef _DEBUG
			sint32 overhead80 = 8 * overhead / 10;
			TRACE(_T("Time=%d, overhead=%d, ov80%%=%d, sc=%d, sc80=%d, datarate=%g\n"), 
					deltaTime, 
					overhead, 
					overhead80,
					m_nUploadSlopeControl,
					nUploadSlopeControl,
					datarate);
		#endif
	}   
}

// [TPT] - NAFC Selection
void CBandWidthControl::SelectNAFC()
{
	if(m_fGetNumberOfInterfaces != NULL && m_fGetIfTable != NULL && m_fGetIpAddrTable != NULL)
	{
		DWORD dwNumIf = 0;
		if(m_fGetNumberOfInterfaces(&dwNumIf) == NO_ERROR && dwNumIf > 0 ){
			BYTE buffer[10*sizeof(MIB_IFROW)];
			ULONG size = sizeof(buffer);
			MIB_IFTABLE& mibIfTable = reinterpret_cast<MIB_IFTABLE&>(buffer[0]);
			BYTE buffer2[10*sizeof(MIB_IPADDRROW)];
		    ULONG size2 = sizeof(buffer2);
		    MIB_IPADDRTABLE& mibIPAddrTable = reinterpret_cast<MIB_IPADDRTABLE&>(buffer2[0]);
			if(m_fGetIfTable(&mibIfTable, &size, true) == NO_ERROR)
			{
                // Trace list of Adapters
                if(m_errorTraced == false)
				{		
                    CSelNAFCDlg* getNAFCDlg = new CSelNAFCDlg(theApp.emuledlg);                  
					NafcNumber = mibIfTable.dwNumEntries;
				    for(DWORD dwNumEntries = 0; dwNumEntries < mibIfTable.dwNumEntries; dwNumEntries++)
					{
                        const MIB_IFROW& mibIfRow = mibIfTable.table[dwNumEntries];
						m_NAFCInfo[dwNumEntries].description = (CString)mibIfRow.bDescr;
						m_NAFCInfo[dwNumEntries].index = mibIfRow.dwIndex;
						m_NAFCInfo[dwNumEntries].type = mibIfRow.dwType;
						m_NAFCInfo[dwNumEntries].mtu = mibIfRow.dwMtu;
						m_NAFCInfo[dwNumEntries].ip = 0;
						if(m_fGetIpAddrTable(&mibIPAddrTable, &size2, FALSE) == 0)
						{
							for(DWORD i = 0; i < mibIPAddrTable.dwNumEntries; i++)
							{
								if (mibIPAddrTable.table[i].dwIndex == mibIfRow.dwIndex)
									m_NAFCInfo[dwNumEntries].ip = mibIPAddrTable.table[i].dwAddr;
							}
						}							
					}
					getNAFCDlg->DoModal();
					int selection = getNAFCDlg->selection;
					if((selection > -1) && (selection < 9))
					{
						selectedIndex = m_NAFCInfo[selection].index;
						selectedDescription = m_NAFCInfo[selection].description;
					}
					delete getNAFCDlg;
				}
			}
		}

		if(selectedIndex != 0 && selectedIndex != m_currentAdapterIndex)
		{
			m_currentAdapterIndex = selectedIndex;
			AddPhoenixLogLine(false,_T("New NAFC: %s with index %u"), selectedDescription, m_currentAdapterIndex);
		}
	}
}
// [TPT] - NAFC Selection

//[TPT] - Check Nafc on ID change (Xman)
//well...we must rework it, we can do it in less lines
void CBandWidthControl::CheckNafcOnIDChange(uint32 id)
{
	// Check if the library was succefully loaded
	if(m_fGetNumberOfInterfaces != NULL && m_fGetIfTable != NULL && m_fGetIpAddrTable != NULL){
		DWORD dwNumIf = 0;
		if(m_fGetNumberOfInterfaces(&dwNumIf) == NO_ERROR && dwNumIf > 0 ){
			BYTE buffer[10*sizeof(MIB_IFROW)];
			ULONG size = sizeof(buffer);
			MIB_IFTABLE& mibIfTable = reinterpret_cast<MIB_IFTABLE&>(buffer[0]);
			if(m_fGetIfTable(&mibIfTable, &size, true) == NO_ERROR){
				// Trace list of Adapters
				if(m_errorTraced == false){
					for(DWORD dwNumEntries = 0; dwNumEntries < mibIfTable.dwNumEntries; dwNumEntries++){
						const MIB_IFROW& mibIfRow = mibIfTable.table[dwNumEntries];
					}
				}

				// Retrieve the default used IP (=> in case of multiple adapters)
				char hostName[256];
				if(gethostname(hostName, sizeof(hostName)) == 0){
					hostent* lphost = gethostbyname(hostName);
					if(lphost != NULL){
						DWORD dwAddr = ((LPIN_ADDR)lphost->h_addr)->s_addr;
						// Pick the interface matching the IP
						BYTE buffer[10*sizeof(MIB_IPADDRROW)];
						ULONG size = sizeof(buffer);
						MIB_IPADDRTABLE& mibIPAddrTable = reinterpret_cast<MIB_IPADDRTABLE&>(buffer[0]);
						if(m_fGetIpAddrTable(&mibIPAddrTable, &size, FALSE) == 0){
							//Xman: first we seek the highid from the server
							for(DWORD i = 0; i < mibIPAddrTable.dwNumEntries; i++){
								if(mibIPAddrTable.table[i].dwAddr == id){
									const MIB_IPADDRROW& row = mibIPAddrTable.table[i];
									m_errorTraced = false;
									AddDebugLogLine(false, GetResString(IDS_NAFCADAPTERINDEX), mibIPAddrTable.table[i].dwIndex);
									m_currentAdapterIndex= mibIPAddrTable.table[i].dwIndex;
									//reactivate NAFC	
									if(wasNAFCLastActive)
										thePrefs.SetNAFCFullControl(true);
									return;
								}
							}
							//Xman: if highid not found, search the hostip
							for(DWORD i = 0; i < mibIPAddrTable.dwNumEntries; i++){
								if(mibIPAddrTable.table[i].dwAddr == dwAddr){
									const MIB_IPADDRROW& row = mibIPAddrTable.table[i];
									m_errorTraced = false;
									AddDebugLogLine(false, GetResString(IDS_NAFCADAPTERINDEX), mibIPAddrTable.table[i].dwIndex);
									m_currentAdapterIndex= mibIPAddrTable.table[i].dwIndex;
									//reactivate NAFC	
									if(wasNAFCLastActive)
										thePrefs.SetNAFCFullControl(true);
									return;
								}
							}
						}
						else {
							if(m_errorTraced == false){
								m_errorTraced = true;
								AddDebugLogLine(false, GetResString(IDS_NAFCIPTABLES), ::GetLastError());
								return ;
							}
						}
					}
				}
			}
			if(m_errorTraced == false){
				m_errorTraced = true;
				AddDebugLogLine(false, GetResString(IDS_NAFCIPTABLES), ::GetLastError());
				return ;
			}
		}
		if(m_errorTraced == false){
			m_errorTraced = true;
			AddDebugLogLine(false, GetResString(IDS_NAFCINTNUMBER), ::GetLastError());
			return ;
		}
	}
}

uint64 CBandWidthControl::GeteMuleOut() const 
{
   uint64 value;
   m_statisticLocker.Lock();
   /**/ value = m_statistic.eMuleOutOctets;
   m_statisticLocker.Unlock();
   return value;
}

uint64 CBandWidthControl::GeteMuleIn() const 
{
   uint64 value;
   m_statisticLocker.Lock();
   /**/ value = m_statistic.eMuleInOctets;
   m_statisticLocker.Unlock();
   return value;
}

uint64 CBandWidthControl::GeteMuleOutOverall() const 
{
   uint64 value;
   m_statisticLocker.Lock();
   /**/ value = m_statistic.eMuleOutOverallOctets;
   m_statisticLocker.Unlock();
   return value;
}

uint64 CBandWidthControl::GeteMuleInOverall() const 
{
   uint64 value;
   m_statisticLocker.Lock();
   /**/ value = m_statistic.eMuleInOverallOctets;
   m_statisticLocker.Unlock();
   return value;
}

uint64 CBandWidthControl::GetNetworkOut() const 
{
   uint64 value;
   m_statisticLocker.Lock();
   /**/ value = m_statistic.networkOutOctets;
   m_statisticLocker.Unlock();
   return value;
}

uint64 CBandWidthControl::GetNetworkIn() const 
{
   uint64 value;
   m_statisticLocker.Lock();
   /**/ value = m_statistic.networkInOctets;
   m_statisticLocker.Unlock();
   return value;
}

uint64 CBandWidthControl::GeteMuleOutFriend() const
{
	uint64 value;
	m_statisticLocker.Lock();
	/**/ value = m_statistic.eMuleOutFriendOctets;
	m_statisticLocker.Unlock();
	return value;
}

uint32 CBandWidthControl::GetStartTick() const 
{
   uint32 value;
   m_statisticLocker.Lock();
   /**/ value = m_statistic.timeStamp;
   m_statisticLocker.Unlock();
   return value;
}

void CBandWidthControl::AddeMuleOutUDPOverall(uint32 octets)
{
   //octets += (20 /* IP */ + 8 /* UDP */);
   m_statisticLocker.Lock();
   /**/ m_statistic.eMuleOutOverallOctets += octets;
   m_statisticLocker.Unlock();
}

void CBandWidthControl::AddeMuleOutTCPOverall(uint32 octets) 
{
   //octets += (20 /* IP */ + 20 /* TCP */);
   m_statisticLocker.Lock();
   /**/ m_statistic.eMuleOutOverallOctets += octets;
   m_statisticLocker.Unlock();
}

void CBandWidthControl::AddeMuleOutFriend(uint32 octets) 
{
   m_statisticLocker.Lock();
   /**/ m_statistic.eMuleOutFriendOctets += octets;
   m_statisticLocker.Unlock();
}
void CBandWidthControl::AddeMuleInUDPOverall(uint32 octets) 
{
   //octets += (20 /* IP */ + 8 /* UDP */);
   m_statisticLocker.Lock();
   /**/ m_statistic.eMuleInOverallOctets += octets;
   m_statisticLocker.Unlock();
}

void CBandWidthControl::AddeMuleInTCPOverall(uint32 octets)
{
   //octets += (20 /* IP */ + 20 /* TCP */);
   m_statisticLocker.Lock();
   /**/ m_statistic.eMuleInOverallOctets += octets;
   m_statisticLocker.Unlock();
}

void CBandWidthControl::AddeMuleOut(uint32 octets)
{
   m_statisticLocker.Lock();
   /**/ m_statistic.eMuleOutOctets += octets;
   m_statisticLocker.Unlock();
}

void CBandWidthControl::AddeMuleIn(uint32 octets)
{
   m_statisticLocker.Lock();
   /**/ m_statistic.eMuleInOctets += octets;
   m_statisticLocker.Unlock();
}
