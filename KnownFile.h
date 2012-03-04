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
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#pragma once
#include "BarShader.h"
#include <list>

#define	PARTSIZE			9728000
#define	MAX_EMULE_FILE_SIZE	4290048000	// (4294967295/PARTSIZE)*PARTSIZE

class CTag;
class CxImage;
namespace Kademlia{
	class CUInt128;
	class CEntry;
	typedef std::list<CStringW> WordList;
};
class CUpDownClient;
class Packet;
class CFileDataIO;
class CAICHHashTree;
class CAICHHashSet;
class CSafeMemFile;	// [TPT] - SLUGFILLER: hideOS
class CSharedFileDetailsSheetInterface;	// [TPT] - SLUGFILLER: modelessDialogs
typedef CTypedPtrList<CPtrList, CUpDownClient*> CUpDownClientPtrList;

class CFileStatistic
{
	friend class CKnownFile;
	friend class CPartFile;
public:
	CFileStatistic()
	{
		requested = 0;
		transferred = 0;
		accepted = 0;
		alltimerequested = 0;
		alltimetransferred = 0;
		alltimeaccepted = 0;
	}

	void	MergeFileStats( CFileStatistic* toMerge );
	void	AddRequest();
	void	AddAccepted();
	// [TPT] - SLUGFILLER: Spreadbars
	void	AddTransferred(uint64 start, uint64 bytes);
	void	AddBlockTransferred(uint64 start, uint64 end, uint32 count);	

	void	DrawSpreadBar(CDC* dc, RECT* rect, bool bFlat) const;
	float	GetSpreadSortValue() const;
	float	GetFullSpreadCount() const;
	// [TPT] - SLUGFILLER: Spreadbars

	UINT	GetRequests() const				{return requested;}
	UINT	GetAccepts() const				{return accepted;}
	uint64	GetTransferred() const			{return transferred;}
	UINT	GetAllTimeRequests() const		{return alltimerequested;}
	UINT	GetAllTimeAccepts() const		{return alltimeaccepted;}
	uint64	GetAllTimeTransferred() const	{return alltimetransferred;}
	
	CKnownFile* fileParent;

private:
	// [TPT] - SLUGFILLER: Spreadbars
	CRBMap<uint64, uint64> spreadlist;	
	// [TPT] - SLUGFILLER: Spreadbars
	
	uint32 requested;
	uint32 accepted;
	uint64 transferred;
	uint32 alltimerequested;
	uint64 alltimetransferred;
	uint32 alltimeaccepted;
};

/*
					   CPartFile
					 /
		  CKnownFile
		/
CAbstractFile
		\
		  CSearchFile
*/
class CAbstractFile: public CObject
{
	DECLARE_DYNAMIC(CAbstractFile)

public:
	CAbstractFile();
	virtual ~CAbstractFile() { }

	const CString& GetFileName() const { return m_strFileName; }
	virtual void SetFileName(LPCTSTR pszFileName, bool bReplaceInvalidFileSystemChars = false, bool bAutoSetFileType = true); // 'bReplaceInvalidFileSystemChars' is set to 'false' for backward compatibility!

	// returns the ED2K file type (an ASCII string)
	const CString& GetFileType() const { return m_strFileType; }
	virtual void SetFileType(LPCTSTR pszFileType);

	// returns the file type which is used to be shown in the GUI
	CString GetFileTypeDisplayStr() const;

	const uchar* GetFileHash() const { return m_abyFileHash; }
	void SetFileHash(const uchar* pucFileHash);
	bool HasNullHash() const;

	uint32 GetFileSize() const { return m_nFileSize; }
	virtual void SetFileSize(uint32 nFileSize) { m_nFileSize = nFileSize; }

	// [TPT]
	// Divers cached tag
	bool	IsMovie() const { return m_isMovie; }
	bool	IsCDImage() const { return m_isCDImage; }
	bool	IsArchive(bool onlyPreviewable=false) const { return m_isArchive; }	

	uint32 GetIntTagValue(uint8 tagname) const;
	uint32 GetIntTagValue(LPCSTR tagname) const;
	bool GetIntTagValue(uint8 tagname, uint32& ruValue) const;
	const CString& GetStrTagValue(uint8 tagname) const;
	const CString& GetStrTagValue(LPCSTR tagname) const;
	CTag* GetTag(uint8 tagname, uint8 tagtype) const;
	CTag* GetTag(LPCSTR tagname, uint8 tagtype) const;
	CTag* GetTag(uint8 tagname) const;
	CTag* GetTag(LPCSTR tagname) const;
	void AddTagUnique(CTag* pTag);
	const CArray<CTag*,CTag*>& GetTags() const { return taglist; }
	void AddNote(Kademlia::CEntry* pEntry);
	const CTypedPtrList<CPtrList, Kademlia::CEntry*>& getNotes() const { return CKadEntryPtrList; }

#ifdef _DEBUG
	// Diagnostic Support
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	uchar	m_abyFileHash[16];	
	CString m_strComment;
	uint8	m_uRating;
	CArray<CTag*,CTag*> taglist;
	CTypedPtrList<CPtrList, Kademlia::CEntry*> CKadEntryPtrList;

private:
	CString m_strFileName;
	uint32  m_nFileSize;	
	CString m_strFileType;	
	bool    m_isMovie;
	bool    m_isCDImage;
	bool    m_isArchive;	

	// Don't allow canonical behavior
	CAbstractFile(const CAbstractFile&);
	CAbstractFile& operator=(const CAbstractFile&);
};
// Maella end
// [TPT] - SLUGFILLER: showComments
struct Comment_Struct{
	uint32		m_iUserIP;
	uint32		m_iUserPort;
	CString		m_strUserName;
	CString		m_strFileName;
	uint8		m_uRating; 
	CString		m_strComment;
};
// [TPT] - SLUGFILLER: showComments

class CKnownFile : public CAbstractFile
{
	DECLARE_DYNAMIC(CKnownFile)
	friend class CImportPartsFileThread; 	//[TPT] - SR13: Import Parts

public:
	CKnownFile();
	~CKnownFile();

	// [TPT] - WebCache
	bool ReleaseViaWebCache; //JP webcache release
	uint32 GetNumberOfClientsRequestingThisFileUsingThisWebcache(CString webcachename, uint32 maxCount); //JP webcache release
	void SetReleaseViaWebCache(bool WCRelease) {ReleaseViaWebCache=WCRelease;} //JP webcache release
	// [TPT] - WebCache	

	virtual void SetFileName(LPCTSTR pszFileName, bool bReplaceInvalidFileSystemChars = false); // 'bReplaceInvalidFileSystemChars' is set to 'false' for backward compatibility!

	CString GetPath(bool returnVirtual = false) const;	// itsonlyme: virtualDirs
	void SetPath(LPCTSTR path);

	const CString& GetFilePath() const { return m_strFilePath; }
	void SetFilePath(LPCTSTR pszFilePath);

	// [TPT] - SLUGFILLER: mergeKnown
	void	SetLastSeen()	{ m_dwLastSeen = time(NULL); }
	uint32	GetLastSeen()	{ return m_dwLastSeen; }
	// [TPT] - SLUGFILLER: mergeKnown
	
	virtual bool CreateFromFile(LPCTSTR directory, LPCTSTR filename, LPVOID pvProgressParam); // create date, hashset and tags from a file
	virtual bool IsPartFile() const { return false; }
	virtual bool LoadFromFile(CFileDataIO* file);	//load date, hashset and tags from a .met file
	bool	WriteToFile(CFileDataIO* file);
	//[TPT] - SR13: Import Parts
	bool	SR13_ImportParts();
	//[TPT] - SR13: Import Parts
	bool	CreateAICHHashSetOnly();

	// last file modification time in (DST corrected, if NTFS) real UTC format
	// NOTE: this value can *not* be compared with NT's version of the UTC time
	CTime	GetUtcCFileDate() const { return CTime(m_tUtcLastModified); }
	uint32	GetUtcFileDate() const { return m_tUtcLastModified; }

	virtual void SetFileSize(uint32 nFileSize);

	// local available part hashs
	uint16	GetHashCount() const { return hashlist.GetCount(); }
	uchar*	GetPartHash(uint16 part) const;
	const CArray<uchar*, uchar*>& GetHashset() const { return hashlist; }
	bool	SetHashset(const CArray<uchar*, uchar*>& aHashset);

	// nr. of part hashs according the file size wrt ED2K protocol
	UINT	GetED2KPartHashCount() const { return m_iED2KPartHashCount; }

	// nr. of 9MB parts (file data)
	__inline uint16 GetPartCount() const { return m_iPartCount; }

	// nr. of 9MB parts according the file size wrt ED2K protocol (OP_FILESTATUS)
	__inline uint16 GetED2KPartCount() const { return m_iED2KPartCount; }

	// file upload priority
	uint8	GetUpPriority(void) const { return m_iUpPriority; }
	void	SetUpPriority(uint8 iNewUpPriority, bool bSave = true);
	bool	IsAutoUpPriority(void) const { return m_bAutoUpPriority; }
	void	SetAutoUpPriority(bool NewAutoUpPriority) { m_bAutoUpPriority = NewAutoUpPriority; }
	void	UpdateAutoUpPriority();

	// This has lost it's meaning here.. This is the total clients we know that want this file..
	// Right now this number is used for auto priorities..
	// This may be replaced with total complete source known in the network..
	uint32	GetQueuedCount() { return m_ClientUploadList.GetCount();}
	// [TPT] - xMule_MOD: showSharePermissions
	// shared file view permissions (all, only friends, no one)
	uint8	GetPermissions(void) const	{ return m_iPermissions; };
	void	SetPermissions(uint8 iNewPermissions) {m_iPermissions = iNewPermissions;};
	// [TPT] - xMule_MOD: showSharePermissions
	bool	LoadHashsetFromFile(CFileDataIO* file, bool checkhash);

	bool	HideOvershares(CSafeMemFile* file, CUpDownClient* client);	// [TPT] - SLUGFILLER: hideOS

	void	AddUploadingClient(CUpDownClient* client);
	void	RemoveUploadingClient(CUpDownClient* client);
	virtual void	UpdatePartsInfo();
	virtual	void	DrawShareStatusBar(CDC* dc, LPCRECT rect, bool onlygreyrect, bool bFlat) const;

	// comment
	// [TPT] - MFCK [addon] - New Tooltips [Rayita]	Adapted by TPT
	//Comment_Struct GetClientComment(const CUpDownClient* client);	
	// [TPT] - MFCK [addon] - New Tooltips [Rayita] Adapted by TPT

	const CString& GetFileComment() /*const*/;
	void	SetFileComment(LPCTSTR pszComment);

	uint8	GetFileRating() /*const*/;
	void	SetFileRating(uint8 uRating);
	// [TPT] - SLUGFILLER: showComments
	bool	HasComment() const { return m_bHasComment; }
	bool	HasRating() const { return m_bHasRating; }
	bool	HasBadRating() const { return m_bHasBadRating; }
	void	AddComment(Comment_Struct cs);
	const CList<Comment_Struct>& GetCommentList() const { return m_CommentList; }
	// [TPT] - SLUGFILLER: showComments

	bool	GetPublishedED2K() const { return m_PublishedED2K; }
	void	SetPublishedED2K(bool val);

	uint32	GetKadFileSearchID() const { return kadFileSearchID; }
	void	SetKadFileSearchID(uint32 id) { kadFileSearchID = id; } //Don't use this unless you know what your are DOING!! (Hopefully I do.. :)

	const Kademlia::WordList& GetKadKeywords() const { return wordlist; }

	uint32	GetLastPublishTimeKadSrc() const { return m_lastPublishTimeKadSrc; }
	void	SetLastPublishTimeKadSrc(uint32 time, uint32 buddyip) { m_lastPublishTimeKadSrc = time; m_lastBuddyIP = buddyip;}
	uint32	GetLastPublishBuddy() const { return m_lastBuddyIP; }
	void	SetLastPublishTimeKadNotes(uint32 time) {m_lastPublishTimeKadNotes = time;}
	uint32	GetLastPublishTimeKadNotes() const { return m_lastPublishTimeKadNotes; }

	bool	PublishSrc();
	bool	PublishNotes();

	// file sharing
	virtual Packet* CreateSrcInfoPacket(CUpDownClient* forClient) const;
	UINT	GetMetaDataVer() const { return m_uMetaDataVer; }
	void	UpdateMetaDataTags();
	void	RemoveMetaDataTags();

	// preview
	bool	IsMovie() const;
	virtual bool GrabImage(uint8 nFramesToGrab, double dStartTime, bool bReduceColor, uint16 nMaxWidth, void* pSender);
	virtual void GrabbingFinished(CxImage** imgResults, uint8 nFramesGrabbed, void* pSender);

	// aich
	CAICHHashSet*	GetAICHHashset() const							{return m_pAICHHashSet;}
	void			SetAICHHashset(CAICHHashSet* val)				{m_pAICHHashSet = val;}
	// last file modification time in (DST corrected, if NTFS) real UTC format
	// NOTE: this value can *not* be compared with NT's version of the UTC time
	uint32	m_tUtcLastModified;

	CFileStatistic statistic;
	time_t m_nCompleteSourcesTime;
	uint16 m_nCompleteSourcesCount;
	uint16 m_nCompleteSourcesCountLo;
	uint16 m_nCompleteSourcesCountHi;
	CUpDownClientPtrList m_ClientUploadList;
	CArray<uint16, uint16> m_AvailPartFrequency;
	CArray<uint16> m_PartSentCount;	// [TPT] - SLUGFILLER: hideOS
	uint16 m_nVirtualCompleteSourcesCount;// [TPT] - Powershare
	// [TPT] - SLUGFILLER: modelessDialogs
	CSharedFileDetailsSheetInterface*	GetDetailsSheetInterface() const { return m_detailsSheetInterface; }
	// [TPT] - SLUGFILLER: modelessDialogs

#ifdef _DEBUG
	// Diagnostic Support
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

    // [TPT] - Powershare
	void    SetPowerShared(int newValue) {m_powershared = newValue;};
	bool    GetPowerShared() const;
	void	SetHideOS(int newValue) {m_iHideOS = newValue;};
	int		GetHideOS() const {return m_iHideOS;}
	void	SetSelectiveChunk(int newValue) {m_iSelectiveChunk = newValue;};
	int		GetSelectiveChunk() const {return m_iSelectiveChunk;}
	int		GetPowerSharedMode() const {return m_powershared;}
	bool	GetPowerShareAuthorized() const {return m_bPowerShareAuthorized;}
	bool	GetPowerShareAuto() const {return m_bPowerShareAuto;}
	void	SetPowerShareLimit(int newValue) {m_iPowerShareLimit = newValue;};
	int		GetPowerShareLimit() const {return m_iPowerShareLimit;}
	bool	GetPowerShareLimited() const {return m_bPowerShareLimited;}
	void	UpdatePowerShareLimit(bool authorizepowershare,bool autopowershare, bool limitedpowershare) {m_bPowerShareAuthorized = authorizepowershare;m_bPowerShareAuto = autopowershare;m_bPowerShareLimited = limitedpowershare;}
    // [TPT] - Powershare END

	//[TPT] - SR13: Import Parts
	bool	CreateHash(const uchar* pucData, UINT uSize, uchar* pucHash, CAICHHashTree* pShaHashOut = NULL) const;
	//[TPT] - SR13: Import Parts

	// [TPT]	
	// Maella -One-queue-per-file- (idea bloodymad)
public:
	uint32 GetFileScore(bool isDownloading, uint32 downloadingTime);
	uint32 GetStartUploadTime() const {return m_startUploadTime;}
	void   UpdateStartUploadTime() {m_startUploadTime = GetTickCount();}

private:
	uint32 m_startUploadTime;
	// Maella end
protected:
	//preview
	bool	GrabImage(CString strFileName, uint8 nFramesToGrab, double dStartTime, bool bReduceColor, uint16 nMaxWidth, void* pSender);
	bool	LoadTagsFromFile(CFileDataIO* file);
	bool	LoadDateFromFile(CFileDataIO* file);
	void	CreateHash(CFile* pFile, UINT uSize, uchar* pucHash, CAICHHashTree* pShaHashOut = NULL) const;
	bool	CreateHash(FILE* fp, UINT uSize, uchar* pucHash, CAICHHashTree* pShaHashOut = NULL) const;
	//bool	CreateHash(const uchar* pucData, UINT uSize, uchar* pucHash, CAICHHashTree* pShaHashOut = NULL) const;
	void	LoadComment();
	uint16	CalcPartSpread(CArray<uint32>& partspread, CUpDownClient* client);	// [TPT] - SLUGFILLER: hideOS
	CArray<uchar*, uchar*> hashlist;
	CString m_strDirectory;
	CString m_strFilePath;
	CAICHHashSet*			m_pAICHHashSet;

private:	
	//static CBarShader s_ShareStatusBar; // [TPT]
	uint16	m_iPartCount;
	uint16	m_iED2KPartCount;
	uint16	m_iED2KPartHashCount;
	uint8	m_iUpPriority;
	uint8	m_iPermissions; // [TPT] - xMule_MOD: showSharePermissions
	bool	m_bAutoUpPriority;
	bool	m_bCommentLoaded;
	// [TPT] - SLUGFILLER: showComments
	bool	m_bHasRating;
	bool	m_bHasBadRating;
	bool	m_bHasComment;
	CList<Comment_Struct> m_CommentList;	// received comments list
	// [TPT] - SLUGFILLER: showComments
	bool	m_PublishedED2K;
	uint32	kadFileSearchID;
	uint32	m_lastPublishTimeKadSrc;
	uint32	m_lastPublishTimeKadNotes;
	uint32	m_lastBuddyIP;
	Kademlia::WordList wordlist;
	UINT	m_uMetaDataVer;
	uint32	m_dwLastSeen;	// [TPT] - SLUGFILLER: mergeKnown
	CSharedFileDetailsSheetInterface*	m_detailsSheetInterface;	// SLUGFILLER: modelessDialogs
// [TPT] - MoNKi: -Downloaded History-
private:
	bool	m_isShared;
	// [TPT] - Powershare
	int		m_iHideOS;
	int		m_iSelectiveChunk;
	int		m_powershared;
	bool	m_bPowerShareAuthorized;
	bool	m_bPowerShareAuto;
	int		m_iPowerShareLimit;
	bool	m_bPowerShareLimited;
    // [TPT] - Powershare END
public:
	void	SetShared(bool shared)	{ m_isShared = shared; }
	bool	IsShared()				{ return m_isShared; }
// [TPT] - MoNKi: -Downloaded History-
};

// permission values for shared files
#define PERM_ALL		0
#define PERM_FRIENDS	1
#define PERM_NOONE		2

