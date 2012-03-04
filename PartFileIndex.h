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

#pragma once

class CPartFileIndex
{
public:
	CPartFileIndex();
	~CPartFileIndex();

	void AddPartFile(CString met, CString filename, uint32 size, const uchar* hash, const CArray<uchar*, uchar*>& hashlist, CString AICHhash, bool savenow = true);
	void RecoverPartFile(CString met);
	void RemovePartFile(CString met);
	void RecoverAllPartFiles();
private:
	void Load();
	void Save();

	struct IndexedFile{
		CString filename;
		uint32 size;
		uchar hash[16];
		CString hashset;
		CString AICHhash;
		bool valid;
	};
	CRBMap<CString, IndexedFile> m_mapFiles;

	void RecoverPartFileInternal(CString met, IndexedFile recoverfile);
};
