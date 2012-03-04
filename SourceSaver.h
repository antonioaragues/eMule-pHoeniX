#pragma once

// Maella
// New rules after a restart:
// - If elapsed time < 1 hour, reask all sources
// - If elapsed time > 1 hour, reask 10 sources
// - If elapsed time > 3 days, reask none

#include "types.h"

class CPartFile;
class CUpDownClient;

class CSourceSaver
{
public:
	CSourceSaver(CPartFile* file);
	~CSourceSaver(void);
	bool Process();
	void DeleteFile();

protected:
	class CSourceData
	{
	public:
		CSourceData() {} // Necessary for class CList

		CSourceData(uint32 dwID, uint16 wPort, const CString& expiration90mins, const CString& expirationNdays) : sourceID(dwID), 
																	                                       sourcePort(wPort),
																		                                   partsavailable(0),
                                                                                                           expiration90mins(expiration90mins),
																		                                   expirationNdays(expirationNdays) {}

		CSourceData(const CSourceData& ref) : sourceID(ref.sourceID), 
											  sourcePort(ref.sourcePort),
											  partsavailable(ref.partsavailable),
											  expiration90mins(ref.expiration90mins),
                                              expirationNdays(ref.expirationNdays){}

		CSourceData(CUpDownClient* client, const CString& expiration90mins, const CString& expirationNdays);

		bool Compare(const CSourceData& tocompare) const { return (sourceID == tocompare.sourceID) &&
																  (sourcePort == tocompare.sourcePort); }

		uint32	sourceID;
		uint16	sourcePort;
		uint32	partsavailable;
		CString expiration90mins; // 1.5 hours
        CString expirationNdays; // N days
	};
	typedef CList<CSourceData> SourceList;

	void LoadSourcesFromFile(const CString& slsfile);
	void SaveSources(const CString& slsfile);
	void AddSourcesToDownload();
	
	uint32     m_dwLastTimeLoaded;
	uint32     m_dwLastTimeSaved;	
	CPartFile* m_pFile;
	SourceList m_sourceList;

	CString CSourceSaver::CalcExpiration(); // 1.5 hours (8 chars)
	CString CSourceSaver::CalcExpirationLong(); // N days (6 chars)
	bool IsExpired(const CString& expiration90mins) const;
};
