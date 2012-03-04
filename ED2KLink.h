//this file is part of eMule
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
#include "shahashset.h"
class CSafeMemFile;

struct SUnresolvedHostname
{
	SUnresolvedHostname()
	{
		nPort = 0;
	}
	CStringA strHostname;
	uint16 nPort;
	CString strURL;
};


class CED2KLink
{
public:
	static CED2KLink* CreateLinkFromUrl(  const TCHAR * url);
	virtual ~CED2KLink();

	// [TPT] - Announ: -Friend eLinks-
	/*
	typedef enum { kServerList, kServer , kFile , kInvalid } LinkType;
	*/
	typedef enum { kServerList, kServer , kFile , kFriend, kFriendList, kInvalid } LinkType;
	// End -Friend eLinks-

	virtual LinkType GetKind() const =0;
	virtual void GetLink(CString& lnk) const = 0;
	virtual class CED2KServerListLink* GetServerListLink() =0;
	virtual class CED2KServerLink* GetServerLink() =0;
	virtual class CED2KFileLink* GetFileLink() =0;
};


class CED2KServerLink : public CED2KLink
{
public:
	CED2KServerLink(const TCHAR* ip,const TCHAR* port);
	virtual ~CED2KServerLink();

	virtual LinkType GetKind() const;
	virtual void GetLink(CString& lnk) const;
	virtual CED2KServerListLink* GetServerListLink();
	virtual CED2KServerLink* GetServerLink();
	virtual CED2KFileLink* GetFileLink();

	uint32 GetIP() const { return m_ip;}
	uint16 GetPort() const { return m_port;}
	void GetDefaultName(CString& defName) const { defName = m_defaultName; }

private:
	CED2KServerLink();
	CED2KServerLink(const CED2KServerLink&);
	CED2KServerLink& operator=(const CED2KServerLink&);

	uint32 m_ip;
	uint16 m_port;
	CString m_defaultName;
};


class CED2KFileLink : public CED2KLink
{
public:
	CED2KFileLink(const TCHAR* pszName, const TCHAR* pszSize, const TCHAR* pszHash, const CStringArray& param, const TCHAR* pszSources);
	// [TPT] - khaos::categorymod+
	CString GetSizeStr() { return m_size; }
	int		GetCat() { return m_cat; }
	void	SetCat(int in) { m_cat = in; }
	// [TPT] - khaos::categorymod-
	virtual ~CED2KFileLink();

	virtual LinkType GetKind() const;
	virtual void GetLink(CString& lnk) const;
	virtual CED2KServerListLink* GetServerListLink();
	virtual CED2KServerLink* GetServerLink();
	virtual CED2KFileLink* GetFileLink();
	
	const TCHAR* GetName() const { return m_name; }
	const uchar* GetHashKey() const { return m_hash;}
	const CAICHHash&	GetAICHHash() const		{ return m_AICHHash;}
	long	GetSize() const						{ return _tstol(m_size); }	
	bool	HasValidSources() const				{ return (SourcesList!=NULL); }
	bool	HasHostnameSources() const			{ return (!m_HostnameSourcesList.IsEmpty()); }
	bool	HasValidAICHHash() const			{ return m_bAICHHashValid; }

	CSafeMemFile* SourcesList;
	CSafeMemFile* m_hashset;
	CTypedPtrList<CPtrList, SUnresolvedHostname*> m_HostnameSourcesList;

private:
	CED2KFileLink();
	CED2KFileLink(const CED2KFileLink&);
	CED2KFileLink& operator=(const CED2KFileLink&);

	CString m_name;
	CString m_size;
	uchar m_hash[16];
	bool	m_bAICHHashValid;
	CAICHHash	m_AICHHash;
	// [TPT] - khaos::categorymod+ Sometimes we need to assign a static category to a file link...
	int m_cat;
	// [TPT] - khaos::categorymod-
};


class CED2KServerListLink : public CED2KLink
{
public:
	CED2KServerListLink(const TCHAR* pszAddress);
	virtual ~CED2KServerListLink();

	virtual LinkType GetKind() const;
	virtual void GetLink(CString& lnk) const;
	virtual CED2KServerListLink* GetServerListLink();
	virtual CED2KServerLink* GetServerLink();
	virtual CED2KFileLink* GetFileLink();

	const TCHAR* GetAddress() const { return m_address; }

private:
	CED2KServerListLink();
	CED2KServerListLink(const CED2KFileLink&);
	CED2KServerListLink& operator=(const CED2KFileLink&);

	CString m_address;
};
// [TPT] - Announ: -Friend eLinks-
class CED2KFriendLink : public CED2KLink
{
public:
	CED2KFriendLink(LPCTSTR userName, LPCTSTR userHash);
	CED2KFriendLink(LPCTSTR userName, uchar userHash[]);
	virtual ~CED2KFriendLink()	{ }

	// Inherited pure virtual functions
	virtual LinkType	GetKind() const					{ return kFriend; }
	virtual void	GetLink(CString& lnk) const;
	virtual CED2KServerListLink*	GetServerListLink()	{ return NULL; }
	virtual CED2KServerLink*		GetServerLink()		{ return NULL; }
	virtual CED2KFileLink*			GetFileLink()		{ return NULL; }

	CString	GetUserName() const						{ return m_sUserName; }
	void	GetUserHash(uchar userHash[]) const		{ MEMCOPY(userHash, m_hash, 16*sizeof(uchar)); }

private:
	CString	m_sUserName;
	uchar	m_hash[16];
};

class CED2KFriendListLink : public CED2KLink
{
public:
	CED2KFriendListLink(LPCTSTR address);
	virtual ~CED2KFriendListLink()	{ }

	// Inherited pure virtual functions
	virtual LinkType	GetKind() const					{ return kFriendList; }
	virtual void	GetLink(CString& lnk) const;
	virtual CED2KServerListLink*	GetServerListLink()	{ return NULL; }
	virtual CED2KServerLink*		GetServerLink()		{ return NULL; }
	virtual CED2KFileLink*			GetFileLink()		{ return NULL; }

	CString	GetAddress() const		{ return m_address; }

private:
	CString	m_address;
};
// End -Friend eLinks-
