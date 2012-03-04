//this file is part of eMule
// added by SLUGFILLER
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

// SLUGFILLER: indexPartFiles

#include "StdAfx.h"
#include <io.h>
#include "PartFileIndex.h"
#include "Opcodes.h"
#include "Packets.h"
#include "emule.h"
#include "emuleDlg.h"
#include "Preferences.h"
#include "OtherFunctions.h"
#include "DownloadQueue.h"
#include "PartFile.h"
#include "SafeFile.h"
#include "Log.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif


#define PARTINDEX_FILENAME	_T("downloads.met")

CPartFileIndex::CPartFileIndex()
{
	Load();
}

CPartFileIndex::~CPartFileIndex()
{
}

void CPartFileIndex::AddPartFile(CString met, CString filename, uint32 size, const uchar* hash, const CArray<uchar*, uchar*>& hashlist, CString AICHhash, bool savenow)
{
	IndexedFile file;
	if (!m_mapFiles.Lookup(met, file)) {
		file.filename = filename;
		file.size = size;
		md4cpy(file.hash, hash);
		file.hashset = _T("");
		for (int i = 0; i < hashlist.GetCount(); i++)
			file.hashset += md4str(hashlist[i]);
		file.AICHhash = AICHhash;
		file.valid = true;
		m_mapFiles.SetAt(met, file);
		if (savenow)
			Save();
		return;
	}
	if (file.size == size && !md4cmp(file.hash, hash)) {
		if (!filename.IsEmpty())
			file.filename = filename;
		if (!hashlist.IsEmpty()) {
			file.hashset = _T("");
			for (int i = 0; i < hashlist.GetCount(); i++)
				file.hashset += md4str(hashlist[i]);
		}
		if (!AICHhash.IsEmpty())
			file.AICHhash = AICHhash;
		file.valid = true;
		m_mapFiles.SetAt(met, file);
		return;
	}
	CString buf;
	buf.Format(GetResString(IDS_RECOVER_FILE_MISMATCH), met, file.filename, filename);
	int res = AfxMessageBox(buf, MB_YESNOCANCEL);
	if (res == IDYES)
		RecoverPartFileInternal(met, file);
	else if (res == IDNO) {
		file.filename = filename;
		file.size = size;
		md4cpy(file.hash, hash);
		file.hashset = _T("");
		for (int i = 0; i < hashlist.GetCount(); i++)
			file.hashset += md4str(hashlist[i]);
		file.AICHhash = AICHhash;
		file.valid = true;
		m_mapFiles.SetAt(met, file);
		if (savenow)
			Save();
	}
}

void CPartFileIndex::RecoverPartFile(CString met)
{
	IndexedFile file;
	if (!m_mapFiles.Lookup(met,file))
		return;
	CString buf;
	buf.Format(GetResString(IDS_RECOVER_FILE_CORRUPT), met, file.filename);
	int res = AfxMessageBox(buf, MB_YESNOCANCEL);
	if (res == IDYES)
		RecoverPartFileInternal(met, file);
	else if (res == IDNO)
		m_mapFiles.RemoveKey(met);
}

void CPartFileIndex::RemovePartFile(CString met)
{
	IndexedFile file;
	if (!m_mapFiles.Lookup(met,file))
		return;
	m_mapFiles.RemoveKey(met);
	Save();
}

void CPartFileIndex::RecoverAllPartFiles()
{
	CString met;
	IndexedFile file;
	POSITION pos = m_mapFiles.GetHeadPosition();
	while (pos){
		m_mapFiles.GetNextAssoc(pos, met, file);
		if (!file.valid) {
			CString buf;
			buf.Format(GetResString(IDS_RECOVER_FILE_NOT_FOUND), met, file.filename);
			int res = AfxMessageBox(buf, MB_YESNOCANCEL);
			if (res == IDYES)
				RecoverPartFileInternal(met, file);
			else if (res == IDNO)
				m_mapFiles.RemoveKey(met);
		}
	}
	Save();
}

void CPartFileIndex::Load()
{
	CString name = thePrefs.GetConfigDir() + CString(PARTINDEX_FILENAME);
	CSafeBufferedFile file;
	CFileException fexp;
	if (!file.Open(name,CFile::modeRead|CFile::osSequentialScan|CFile::typeBinary|CFile::shareDenyWrite, &fexp)){
		if (fexp.m_cause != CFileException::fileNotFound){
		CString strError(GetResString(IDS_ERR_LOADINDEXFILE));
			TCHAR szError[MAX_CFEXP_ERRORMSG];
			if (fexp.GetErrorMessage(szError, ARRSIZE(szError))){
				strError += _T(" - ");
				strError += szError;
			}
			LogError(LOG_STATUSBAR, _T("%s"), strError);
		}
		return;
	}
	setvbuf(file.m_pStream, NULL, _IOFBF, 16384);

	try {
		uint8 header = file.ReadUInt8();
		if (header != MET_HEADER){
			file.Close();
			return;
		}

		UINT RecordsNumber = file.ReadUInt32();
		for (UINT i = 0; i < RecordsNumber; i++) {
			CString met;
			IndexedFile loadfile;
			loadfile.valid = false;
			loadfile.size = 0;

			file.ReadHash16(loadfile.hash);

			UINT tagcount = file.ReadUInt32();
			for (UINT j = 0; j < tagcount; j++){
				CTag newtag(&file, false);
				switch(newtag.GetNameID()){
					case FT_PARTFILENAME:{
						ASSERT( newtag.IsStr() );
					    if (newtag.IsStr())
						met = newtag.GetStr();
						break;
					}
					case FT_FILENAME:{
						ASSERT( newtag.IsStr() );
					    if (newtag.IsStr())
							if (loadfile.filename.IsEmpty())
						loadfile.filename = newtag.GetStr();
						break;
					}
					case FT_FILESIZE:{
						ASSERT( newtag.IsInt() );
					    if (newtag.IsInt())
						loadfile.size = newtag.GetInt();
						break;
					}
					case FT_FILETYPE:{
						ASSERT( newtag.IsStr() );
					    if (newtag.IsStr()) {
							loadfile.hashset = _T("");
							CString remain = newtag.GetStr();
							while (!remain.IsEmpty()) {
								uchar hash[16];
								if (strmd4(remain.Left(32), hash))
									loadfile.hashset += remain.Left(32);
								remain = remain.Mid(32);
							}
						}
						break;
					}
					case FT_AICH_HASH:{
						ASSERT( newtag.IsStr() );
					    if (newtag.IsStr())
							loadfile.AICHhash = newtag.GetStr();
						break;
					}
				}
			}
			if (!met.IsEmpty() && !loadfile.filename.IsEmpty() && loadfile.size != 0) {
				m_mapFiles.SetAt(met, loadfile);
			}
		}
		file.Close();
	}
	catch(CFileException* error){		
		if (error->m_cause == CFileException::endOfFile)
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_INDEXFILECORRUPT));
		else{
			TCHAR buffer[MAX_CFEXP_ERRORMSG];
			error->GetErrorMessage(buffer, ARRSIZE(buffer));
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_INDEXFILEREAD),buffer);
		}
		error->Delete();
	}
}

void CPartFileIndex::Save()
{
	CString name = thePrefs.GetConfigDir() + CString(PARTINDEX_FILENAME);
	CSafeBufferedFile file;
	CFileException fexp;
	if (!file.Open(name, CFile::modeWrite|CFile::modeCreate|CFile::typeBinary|CFile::shareDenyWrite, &fexp)){
		CString strError(GetResString(IDS_ERR_SAVEINDEXFILE));
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (fexp.GetErrorMessage(szError, ARRSIZE(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		LogError(LOG_STATUSBAR, _T("%s"), strError);
		return;
	}
	setvbuf(file.m_pStream, NULL, _IOFBF, 16384);

	try{
		//header
		file.WriteUInt8(MET_HEADER);

		//count
		file.WriteUInt32(m_mapFiles.GetCount());

		CString met;
		IndexedFile savefile;
		POSITION pos = m_mapFiles.GetHeadPosition();
		while (pos){
			m_mapFiles.GetNextAssoc(pos, met, savefile);

			//hash
			file.WriteHash16(savefile.hash);

			//tags
			UINT uTagCount = 0;
			ULONG uTagCountFilePos = (ULONG)file.GetPosition();
			file.WriteUInt32(uTagCount);

			CTag partnametag(FT_PARTFILENAME, met);
			partnametag.WriteTagToFile(&file);
			uTagCount++;

			if (WriteOptED2KUTF8Tag(&file, savefile.filename, FT_FILENAME))
				uTagCount++;

			CTag nametag(FT_FILENAME, savefile.filename);
			nametag.WriteTagToFile(&file);
			uTagCount++;

			CTag sizetag(FT_FILESIZE, savefile.size);
			sizetag.WriteTagToFile(&file);
			uTagCount++;

			if (!savefile.hashset.IsEmpty()){
				CTag aichtag(FT_FILETYPE, savefile.hashset);
				aichtag.WriteTagToFile(&file);
				uTagCount++;
			}

			if (!savefile.AICHhash.IsEmpty()){
				CTag aichtag(FT_AICH_HASH, savefile.AICHhash);
				aichtag.WriteTagToFile(&file);
				uTagCount++;
			}

			file.Seek(uTagCountFilePos, CFile::begin);
			file.WriteUInt32(uTagCount);
			file.SeekToEnd();
		}
		file.Close();
	}
	catch(CFileException* error){
		CString strError(GetResString(IDS_ERR_SAVEINDEXFILE));
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (error->GetErrorMessage(szError, ARRSIZE(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		LogError(LOG_STATUSBAR, _T("%s"), strError);
		error->Delete();
	}
}

void CPartFileIndex::RecoverPartFileInternal(CString met, IndexedFile recoverfile)
{
	// delete old if exists
	m_mapFiles.RemoveKey(met);	// Prevent the entry from being removed and saved in RemovePartFile
	CPartFile* old = theApp.downloadqueue->GetFileByMetFileName(met);
	if (old) {
		theApp.downloadqueue->RemoveFile(old);
		delete old;
	}
	m_mapFiles.SetAt(met, recoverfile);	// Put the entry back in the index

	// create recovered part.met
	CString strFile = thePrefs.GetTempDir() + CString(_T("\\")) + met;

	// save file data to part.met file
	CSafeBufferedFile file;
	CFileException fexp;
	if (!file.Open(strFile, CFile::modeWrite|CFile::modeCreate|CFile::typeBinary|CFile::shareDenyWrite, &fexp)){
		CString strError;
		strError.Format(GetResString(IDS_ERR_SAVEMET), met, recoverfile.filename);
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (fexp.GetErrorMessage(szError, ARRSIZE(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		LogError(_T("%s"), strError);
		return;
	}
	setvbuf(file.m_pStream, NULL, _IOFBF, 16384);

	try{
		//version
		file.WriteUInt8(PARTFILE_VERSION);

		//date
		file.WriteUInt32(0);

		//hash
		file.WriteHash16(recoverfile.hash);
		if (recoverfile.hashset.IsEmpty())
		file.WriteUInt16(0);	// no part hashs backup
		else {
			CString remain = recoverfile.hashset;
			file.WriteUInt16(remain.GetLength()/32);
			while (!remain.IsEmpty()) {
				uchar hash[16];
				strmd4(remain.Left(32), hash);
				file.WriteHash16(hash);
				remain = remain.Mid(32);
			}
		}

		//tags
		UINT uTagCount = 0;
		ULONG uTagCountFilePos = (ULONG)file.GetPosition();
		file.WriteUInt32(uTagCount);

		CTag partnametag(FT_PARTFILENAME, RemoveFileExtension(met));
		partnametag.WriteTagToFile(&file);
		uTagCount++;

		if (WriteOptED2KUTF8Tag(&file, recoverfile.filename, FT_FILENAME))
			uTagCount++;
		CTag nametag(FT_FILENAME, recoverfile.filename);
		nametag.WriteTagToFile(&file);
		uTagCount++;

		CTag sizetag(FT_FILESIZE, recoverfile.size);
		sizetag.WriteTagToFile(&file);
		uTagCount++;

		if (!recoverfile.AICHhash.IsEmpty()){
			CTag aichtag(FT_AICH_HASH, recoverfile.AICHhash);
			aichtag.WriteTagToFile(&file);
			uTagCount++;
		}

		file.Seek(uTagCountFilePos, CFile::begin);
		file.WriteUInt32(uTagCount);
		file.SeekToEnd();

		if (thePrefs.GetCommitFiles() >= 2 || (thePrefs.GetCommitFiles() >= 1 && !theApp.emuledlg->IsRunning())){
			file.Flush(); // flush file stream buffers to disk buffers
			if (_commit(_fileno(file.m_pStream)) != 0) // commit disk buffers to disk
				AfxThrowFileException(CFileException::hardIO, GetLastError(), file.GetFileName());
		}
		file.Close();
	}
	catch(CFileException* error){
		CString strError;
		strError.Format(GetResString(IDS_ERR_SAVEMET), met, recoverfile.filename);
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (error->GetErrorMessage(szError, ARRSIZE(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		LogError(_T("%s"), strError);
		error->Delete();

		// remove the partially written or otherwise damaged temporary file
		file.Abort(); // need to close the file before removing it. call 'Abort' instead of 'Close', just to avoid an ASSERT.
		(void)_tremove(strFile);
		return;
	}

	// load recovered download from file
	CPartFile* toadd = new CPartFile();
	if (!toadd->LoadPartFile(thePrefs.GetTempDir(), met)){
		delete toadd;
		return;
	}
	theApp.downloadqueue->AddDownload(toadd,thePrefs.AddNewFilesPaused());	// This would cause the index to save itself
}
