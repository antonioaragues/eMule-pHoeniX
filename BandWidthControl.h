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

#pragma once
#include "Types.h"
#include <Iphlpapi.h>
#include "SelNAFCDlg.h"

// This class has two purposes:
//
// Collect all statistics relevant to the upload/download bandwidth
// Provide an interface to the LAN adapter (Ethernet, slip, etc...)

class CBandWidthControl
{
public:
	CBandWidthControl();
	~CBandWidthControl();

	// Update the history list. Must be called every cycle
	void Process();

	// Calculate maximum upload at a given time
	void ProcessUpload();

	// Actualize current datarate values (Data+Control)
	// Remark: the overhead for the IP/TCP/UDP headers is not included
   	void AddeMuleOutUDPOverall(uint32 octets);
   	void AddeMuleOutTCPOverall(uint32 octets);
   	void AddeMuleInUDPOverall(uint32 octets);
   	void AddeMuleInTCPOverall(uint32 octets);
   	void AddeMuleOut(uint32 octets);
   	void AddeMuleIn(uint32 octets);
	void AddeMuleOutFriend(uint32 octets);

	// Accessors, used for the control of the bandwidth (=> slope)
        /**/ uint64 GeteMuleOut() const;
        /**/ uint64 GeteMuleIn() const;
        /**/ uint64 GeteMuleOutOverall() const;
        /**/ uint64 GeteMuleInOverall() const;
        /**/ uint64 GetNetworkOut() const;
        /**/ uint64 GetNetworkIn() const;
        /**/ uint32 GetStartTick() const;
        /**/ uint64 GeteMuleOutFriend() const;

	// Retrieve datarates for barline + upload slot management + Graphic
	void GetDatarates(uint8 samples, 
					  uint32& eMuleIn, uint32& eMuleInOverall, 
					  uint32& eMuleOut, uint32& eMuleOutOverall,
					  uint32& networkIn, uint32& networkOut) const;

	// Retrieve datarates Graphic
	void GetFullHistoryDatarates(uint32& eMuleInHistory, uint32& eMuleOutHistory, 
								 uint32& eMuleInSession, uint32& eMuleOutSession) const;

	void GetFriendDataRate(uint8 samples, uint32& friendOut) const; // [TPT] - Friend data

	// Check if the NAFC is available on this computer
	bool IsNAFCAvailable() const {return (m_fGetIfEntry != NULL);}

	// Full NAFC bandwidth control    
	float GetMaxDownload() const {return m_maxDownloadLimit;}
    		
	void	SelectNAFC(); // [TPT] - NAFC Selection

	void	CheckNafcOnIDChange(uint32 id);//[TPT] - Check Nafc on ID change (Xman)
	bool checkAdapterIndex(DWORD index);

private:
	// Adapter access
	DWORD getAdapterIndex();
	DWORD m_currentAdapterIndex;

	//Xman new adapter selection
   	bool wasNAFCLastActive;

	// Type definition
	#pragma pack(1)
	struct Statistic {
		Statistic() {}
		Statistic(const Statistic& ref, uint32 time)
		:	eMuleOutOctets(ref.eMuleOutOctets),
			eMuleInOctets(ref.eMuleInOctets),
			eMuleOutOverallOctets(ref.eMuleOutOverallOctets),
			eMuleInOverallOctets(ref.eMuleInOverallOctets),
			networkOutOctets(ref.networkOutOctets),
			networkInOctets(ref.networkInOctets),
			eMuleOutFriendOctets(ref.eMuleOutFriendOctets), // [TPT] - Friend data
			timeStamp(time) {}

		uint64 eMuleOutOctets; // Data
		uint64 eMuleInOctets;
		uint64 eMuleOutOverallOctets; // Data+Control
		uint64 eMuleInOverallOctets;
      	uint64 networkOutOctets; // DataFlow of the network Adapter // [TPT] - Xman
      	uint64 networkInOctets; // [TPT] - Xman
		uint64 eMuleOutFriendOctets; // [TPT] - Friend data
		uint32 timeStamp; // Use a time stamp to compensate the inaccuracy of the timer (based on enkeyDEV(Ottavio84))
		
	};
	#pragma pack()
	typedef CList<Statistic> StatisticHistory; // Use MS container for its memory management

	StatisticHistory m_statisticHistory; // History for the graphic
   /**/ Statistic m_statistic; // Current value

	//Maella thread save
	mutable CCriticalSection m_statisticLocker;  

    float m_maxDownloadLimit; // Used for auto U/D limits

    // Keep last result to detect an overflow
    DWORD m_networkOutOctets; // [TPT] - Xman
    DWORD m_networkInOctets; // [TPT] - Xman

	// Maella -Pseudo overhead datarate control-
	uint32 m_lastProcessTime;      // Used by Process()
	uint64 m_lastSentBytes;        //
	uint64 m_lastOverallSentBytes; //
    uint32 m_lastNetworkOut;       //
	sint32 m_nUploadSlopeControl;  //

	// Dynamic access to the iphlpapi.dll
	typedef DWORD (WINAPI *GETNUMBEROFINTERFACES)(PDWORD pdwNumIf);
	typedef DWORD (WINAPI *GETIFTABLE)(PMIB_IFTABLE pIfTable, PULONG pdwSize, BOOL bOrder);
    typedef DWORD (WINAPI *GETIPADDRTABLE)(PMIB_IPADDRTABLE pIpAddrTable, PULONG pdwSize, BOOL bOrder);
    typedef DWORD (WINAPI *GETIFENTRY)(PMIB_IFROW pIfRow);

	HINSTANCE m_hIphlpapi;
	GETIFTABLE m_fGetIfTable;
    GETIPADDRTABLE m_fGetIpAddrTable;
	GETIFENTRY m_fGetIfEntry;
	GETNUMBEROFINTERFACES m_fGetNumberOfInterfaces;

	// [TPT] - NAFC Selection
	struct NAFCInfo
	{
		CString description;
		DWORD type;
		DWORD index;
		DWORD mtu;
		DWORD ip;
	};
		
	DWORD selectedIndex;
	CString selectedDescription;
   	bool m_errorTraced;

public:
	NAFCInfo m_NAFCInfo[8];
	DWORD NafcNumber;
    	DWORD GetSelectedIndex() const { return selectedIndex; }
	// [TPT] - NAFC Selection

private:
	// Don't allow canonical behavior
	CBandWidthControl(const CBandWidthControl&);
	CBandWidthControl& operator=(const CBandWidthControl&);
};