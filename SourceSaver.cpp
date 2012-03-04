#include "StdAfx.h"
#include "sourcesaver.h"
#include "PartFile.h"
#include "emule.h"
#include "updownclient.h"
#include "preferences.h"
#include "downloadqueue.h"
#include "log.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define RELOADTIME	3600000 //60 minutes	
#define RESAVETIME	 600000 //10 minutes
#define MINIMUMSOURCES	 20 // [TPT] - SLS

CSourceSaver::CSourceSaver(CPartFile* file)
{
	m_dwLastTimeLoaded = 0;
	m_dwLastTimeSaved = 0;
	m_pFile = file;
}

CSourceSaver::~CSourceSaver(void)
{
}

CSourceSaver::CSourceData::CSourceData(CUpDownClient* client, const CString& expiration90mins, const CString& expirationNdays) 
:	sourceID(client->GetUserIDHybrid()), 
	sourcePort(client->GetUserPort()),
	partsavailable(client->GetAvailablePartCount()),
	expiration90mins(expiration90mins),
    expirationNdays(expirationNdays)
{
}

bool CSourceSaver::Process() // return false if sources not saved
{
	// Load only one time the list and keep it in memory (=> reduce CPU load)
	if (m_dwLastTimeLoaded == 0){
		m_dwLastTimeLoaded = ::GetTickCount();
		m_dwLastTimeSaved = ::GetTickCount() + (rand() * 30000 / RAND_MAX) - 15000; // Don't save all files at the same time

		// Load sources from the file
		CString slsfilepath;
		slsfilepath.Format(_T("%s\\%s\\%s.txtsrc"), thePrefs.GetTempDir(), _T("Source Lists"), m_pFile->GetPartMetFileName());
		LoadSourcesFromFile(slsfilepath);

		// Try to add the sources
		AddSourcesToDownload();
	}
	// Save the list every n minutes (default 10 minutes)
	else if ((int)(::GetTickCount() - m_dwLastTimeSaved) > RESAVETIME) {
		m_dwLastTimeSaved = ::GetTickCount() + (rand() * 30000 / RAND_MAX) - 15000; // Don't save all files at the same time

		// Save sources to the file
		CString slsfilepath;
		slsfilepath.Format(_T("%s\\%s\\%s.txtsrc"), thePrefs.GetTempDir(), _T("Source Lists"), m_pFile->GetPartMetFileName());
		SaveSources(slsfilepath);

		// Try to reload the unsuccessfull source
		// if ((int)(::GetTickCount() - m_dwLastTimeLoaded) > RELOADTIME) {
		//	m_dwLastTimeLoaded = ::GetTickCount() + (rand() * 30000 / RAND_MAX) - 15000;
		//	 AddSourcesToDownload(false);
		// }

		return true;
	}
	return false;
}

void CSourceSaver::DeleteFile()
{
	CString slsfilepath;
	// [TPT] - khaos:kmod+ Source Lists directory
	slsfilepath.Format(_T("%s\\%s\\%s.txtsrc"), thePrefs.GetTempDir(), _T("Source Lists"), m_pFile->GetPartMetFileName());
	if (_tremove(slsfilepath)) if (errno != ENOENT)
		AddLogLine(true, _T("Failed to delete %s, you will need to do this by hand"), slsfilepath);    
}

void CSourceSaver::LoadSourcesFromFile(const CString& slsfile)
{
	CString strLine;
	CStdioFile f;
	if (!f.Open(slsfile, CFile::modeRead | CFile::typeText))
		return;
	while(f.ReadString(strLine)) {
		// Skip comment (e.g. title)
		if (strLine.GetAt(0) == '#')
			continue;

		// Load IP
		int pos = strLine.Find(':');
		if (pos == -1)
			continue;
		CStringA strIP(strLine.Left(pos));
		strLine = strLine.Mid(pos+1);
		uint32 dwID = inet_addr(strIP);
		if (dwID == INADDR_NONE) 
			continue;

		// Load Port
		pos = strLine.Find(',');
		if (pos == -1)
			continue;
		CString strPort = strLine.Left(pos);
		strLine = strLine.Mid(pos+1);
		uint16 wPort = _tstoi(strPort);
		if (!wPort)
			continue;

		// Load expiration time (short version => usualy for 3 days)
		pos = strLine.Find(';');
		if (pos == -1)
			continue;
		CString expirationNdays = strLine.Left(pos);
		strLine = strLine.Mid(pos+1);

		// Load expiration time (short version => usualy for 1.5 hours)
		pos = strLine.Find(';');
        CString expiration90mins;
		if (pos != -1){
		    expiration90mins = strLine.Left(pos);
		    strLine = strLine.Mid(pos+1);
        }

		if (IsExpired(expirationNdays) == true && IsExpired(expiration90mins) == true)
            continue;
		else if (IsExpired(expiration90mins) == true)
            expiration90mins.Empty(); // Erase

		// Add source to list
		m_sourceList.AddTail(CSourceData(dwID, wPort, expiration90mins, expirationNdays));
	}
    f.Close();
}

void CSourceSaver::AddSourcesToDownload(){
	uint16 count = 0;
	for(POSITION pos = m_sourceList.GetHeadPosition(); pos != NULL; ){
		// Check if the limit of allowed source was reached
		if(/*thePrefs.GetMaxSourcePerFile()*/m_pFile->GetMaxSourcesPerFile() <= m_pFile->GetSourceCount()) // [TPT] - Sivka AutoHL
			return;

		// Check if the file has more than MINIMUMSOURCES
		if (m_pFile->GetSourceCount() > MINIMUMSOURCES)
			return;

		// Try to add new sources
        // within 3 days => load only MINIMUMSOURCES
        // within 1.5 hours => load all		
	    const CSourceData& cur_src = m_sourceList.GetNext(pos);
        if(count < MINIMUMSOURCES || IsExpired(cur_src.expiration90mins) == false){
            count++;
		    CUpDownClient* newclient = new CUpDownClient(m_pFile, cur_src.sourcePort, cur_src.sourceID, 0, 0, false);
			newclient->SetSourceFrom(SF_SLS);
 		    theApp.downloadqueue->CheckAndAddSource(m_pFile, newclient);
        }
	}

	if (thePrefs.GetVerbose())
	AddDebugLogLine(false, _T("%u %s loaded for the file '%s'"), 
									 count, (count>1) ?  _T("sources") : _T("source"), m_pFile->GetFileName());
}

void CSourceSaver::SaveSources(const CString& slsfile)
{
	const CString expiration90mins = CalcExpiration();
	const CString expirationNdays = CalcExpirationLong();

	// Update sources list for the file
	for(POSITION pos1 = m_pFile->srclist.GetHeadPosition(); pos1 != NULL; ){
		CUpDownClient* cur_src = m_pFile->srclist.GetNext(pos1);

		// Skip lowID source
		if (cur_src->HasLowID())
			continue;

		CSourceData sourceData(cur_src, expiration90mins, expirationNdays);

		// Update or add a source
		for(POSITION pos2 = m_sourceList.GetHeadPosition(); pos2 != NULL;) {
			POSITION cur_pos = pos2;
			const CSourceData& cur_sourcedata = m_sourceList.GetNext(pos2);
            if(cur_sourcedata.Compare(sourceData) == true){
                m_sourceList.RemoveAt(cur_pos);
				break; // exit loop for()
			}
		}

		// Add source to the list
        if(m_sourceList.IsEmpty() == TRUE){
            m_sourceList.AddHead(sourceData);
        }
        else{
	        for(POSITION pos2 = m_sourceList.GetHeadPosition(); pos2 != NULL;) {
		        POSITION cur_pos = pos2;
		        const CSourceData& cur_sourcedata = m_sourceList.GetNext(pos2);
		        if((cur_src->GetDownloadState() == DS_ONQUEUE || cur_src->GetDownloadState() == DS_DOWNLOADING) && 
                    (sourceData.partsavailable >= cur_sourcedata.partsavailable)){
                    // Use the state and the number of available part to sort the list
			        m_sourceList.InsertBefore(cur_pos, sourceData);
			        break; // Exit loop
		        } 
                else if(cur_sourcedata.partsavailable == 0){
                    // Use the number of available part to sort the list
			        m_sourceList.InsertBefore(cur_pos, sourceData);
			        break; // Exit loop
                }
		        else if(pos2 == NULL){						
			        m_sourceList.AddTail(sourceData);
			        break; // Exit loop
		        }
	        }
        }
	}
	
	CString strLine;
	CStdioFile f;
	if (!f.Open(slsfile, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
		return;
	f.WriteString(_T("#format: a.b.c.d:port,expirationdate-Nday(yymmddhhmm);expirationdate-1.5hour(yymmddhhmm);\r\n"));
	uint16 counter = 0;
	for(POSITION pos = m_sourceList.GetHeadPosition(); pos != NULL; ){
        POSITION cur_pos = pos;
		const CSourceData& cur_src = m_sourceList.GetNext(pos);
        if(cur_src.partsavailable > 0){
			// [TPT] - SLS
            if(counter < MINIMUMSOURCES){
		        strLine.Format(_T("%i.%i.%i.%i:%i,%s;%s;\r\n"), 
					        (uint8)cur_src.sourceID, (uint8)(cur_src.sourceID>>8), (uint8)(cur_src.sourceID>>16), (uint8)(cur_src.sourceID>>24),
					        cur_src.sourcePort, 
                            cur_src.expirationNdays,
					        cur_src.expiration90mins);
            }
			/*
            else {
		        strLine.Format("%i.%i.%i.%i:%i,%s;%s;\r\n", 
					        (uint8)cur_src.sourceID, (uint8)(cur_src.sourceID>>8), (uint8)(cur_src.sourceID>>16), (uint8)(cur_src.sourceID>>24),
					        cur_src.sourcePort, 
                            "0001010000",
					        cur_src.expiration90mins);
            }*/
			// [TPT] - SLS
            ++counter;
		    f.WriteString(strLine);
        }
        else if(counter < 20){ // [TPT] - SLS
		    strLine.Format(_T("%i.%i.%i.%i:%i,%s;%s;\r\n"), 
					    (uint8)cur_src.sourceID, (uint8)(cur_src.sourceID>>8), (uint8)(cur_src.sourceID>>16), (uint8)(cur_src.sourceID>>24),
					    cur_src.sourcePort, 
                        cur_src.expirationNdays,
					    "");
            ++counter;
		    f.WriteString(strLine);
        }
        else
        {
            m_sourceList.RemoveAt(cur_pos);
        }
	}
	f.Close();
}

CString CSourceSaver::CalcExpiration()
{
	CTime expiration90mins = CTime::GetCurrentTime();
	CTimeSpan timediff(0, 1, 30, 0);
	expiration90mins += timediff;
    
	CString strExpiration;
	strExpiration.Format(_T("%02i%02i%02i%02i%02i"), 
						 (expiration90mins.GetYear() % 100), 
						 expiration90mins.GetMonth(), 
						 expiration90mins.GetDay(),
						 expiration90mins.GetHour(),
						 expiration90mins.GetMinute());

	return strExpiration;
}

CString CSourceSaver::CalcExpirationLong()
{
	CTime expirationNdays = CTime::GetCurrentTime();
	CTimeSpan timediff(0, 24, 0, 0);
	expirationNdays += timediff;
    
	CString strExpiration;
	strExpiration.Format(_T("%02i%02i%02i%02i%02i"), 
						 (expirationNdays.GetYear() % 100), 
						 expirationNdays.GetMonth(), 
						 expirationNdays.GetDay(),
						 expirationNdays.GetHour(),
						 expirationNdays.GetMinute());

	return strExpiration;
}

bool CSourceSaver::IsExpired(const CString& expiration) const
{
	// example: "yymmddhhmm"
	if(expiration.GetLength() == 10 || expiration.GetLength() == 6){
		int year = _tstoi(expiration.Mid(0, 2)) + 2000;
		int month = _tstoi(expiration.Mid(2, 2));
		int day = _tstoi(expiration.Mid(4, 2));
		int hour = (expiration.GetLength() == 10) ? _tstoi(expiration.Mid(6, 2)) : 0;
		int minute = (expiration.GetLength() == 10) ? _tstoi(expiration.Mid(8, 2)) : 0;

		CTime date(year, month, day, hour, minute, 0);
		return (date < CTime::GetCurrentTime());
	}

	return true;
}
