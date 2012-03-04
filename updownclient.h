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
#pragma once
#include "BarShader.h"
// [TPT]
#include "ClientCredits.h"
#include <map>
#include "SearchList.h"
// [TPT]
// [TPT] - WebCache 
// yonatan http start //////////////////////////////////////////////////////////////////////////
#include "Preferences.h"
#include "WebCache.h"
#include "WebCacheCryptography.h"
#include "WebCacheMFRList.h"//[TPT] - Webcache 1.9 beta3
//#include <crypto51/arc4.h>

// USING_NAMESPACE(CryptoPP)

enum EWebCacheDownState{
	WCDS_NONE = 0,
	WCDS_WAIT_CLIENT_REPLY,
	WCDS_WAIT_CACHE_REPLY,
	WCDS_DOWNLOADINGVIA,
	WCDS_DOWNLOADINGFROM
};

enum EWebCacheUpState{
	WCUS_NONE = 0,
	WCUS_UPLOADING
};


class CWebCacheDownSocket;
class CWebCacheUpSocket;
// yonatan http end ////////////////////////////////////////////////////////////////////////////
// [TPT] - WebCache 

class CClientReqSocket;
class CPeerCacheDownSocket;
class CPeerCacheUpSocket;
class CFriend;
class CPartFile;
class CClientCredits;
class CAbstractFile;
class CKnownFile;
class Packet;
class CxImage;
struct Requested_Block_Struct;
class CSafeMemFile;
class CEMSocket;
class CAICHHash;
enum EUtf8Str;
class CClientDetailDialogInterface;	// [TPT] - SLUGFILLER: modelessDialogs
class CSearchFile;	// [TPT] - itsonlyme: viewSharedFiles

// [TPT]
// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
#pragma pack(4 /*Bytes*/)
struct TransferredData {
	uint64	dataLength;
	DWORD	timeStamp; // => based on enkeyDEV(Ottavio84)
};
#pragma pack()
// Maella end

struct Pending_Block_Struct{
	Pending_Block_Struct()
	{
		block = NULL;
		zStream = NULL;
		totalUnzipped = 0;
		fZStreamError = 0;
		fRecovered = 0;
		fQueued = 0;
	}
	Requested_Block_Struct*	block;
	struct z_stream_s*      zStream;       // Barry - Used to unzip packets
	uint32                  totalUnzipped; // Barry - This holds the total unzipped bytes for all packets so far
	UINT					fZStreamError : 1,
							fRecovered    : 1,
							fQueued		  : 3;
};

#pragma pack(1)
struct Requested_File_Struct{
	uchar	  fileid[16];
	uint32	  lastasked;
	uint8	  badrequests;
};
#pragma pack()

enum EUploadState{
	US_UPLOADING,
	US_ONUPLOADQUEUE,
	US_WAITCALLBACK,
	US_CONNECTING,
	US_PENDING,
	US_LOWTOLOWIP,
	US_BANNED,
	US_ERROR,
	US_NONE
};

enum EDownloadState{
	DS_DOWNLOADING,
	DS_ONQUEUE,
	DS_CONNECTED,
	DS_CONNECTING,
	DS_WAITCALLBACK,
	DS_WAITCALLBACKKAD,
	DS_REQHASHSET,
	DS_NONEEDEDPARTS,
	DS_TOOMANYCONNS,
	DS_TOOMANYCONNSKAD,
	DS_LOWTOLOWIP,
	DS_BANNED,
	DS_ERROR,
	DS_NONE,
	DS_REMOTEQUEUEFULL  // not used yet, except in statistics
};

enum EPeerCacheDownState{
	PCDS_NONE = 0,
	PCDS_WAIT_CLIENT_REPLY,
	PCDS_WAIT_CACHE_REPLY,
	PCDS_DOWNLOADING
};

enum EPeerCacheUpState{
	PCUS_NONE = 0,
	PCUS_WAIT_CACHE_REPLY,
	PCUS_UPLOADING
};

enum EChatState{
	MS_NONE,
	MS_CHATTING,
	MS_CONNECTING,
	MS_UNABLETOCONNECT
};

enum EKadState{
	KS_NONE,
	KS_QUEUED_FWCHECK,
	KS_CONNECTING_FWCHECK,
	KS_CONNECTED_FWCHECK,
	KS_QUEUED_BUDDY,
	KS_INCOMING_BUDDY,
	KS_CONNECTING_BUDDY,
	KS_CONNECTED_BUDDY,
	KS_NONE_LOWID,
	KS_WAITCALLBACK_LOWID,
	KS_QUEUE_LOWID
};

enum EClientSoftware{
	SO_EMULE			= 0,	// default
	SO_CDONKEY			= 1,	// ET_COMPATIBLECLIENT
	SO_XMULE			= 2,	// ET_COMPATIBLECLIENT
	SO_AMULE			= 3,	// ET_COMPATIBLECLIENT
	SO_SHAREAZA			= 4,	// ET_COMPATIBLECLIENT
	SO_MLDONKEY			= 10,	// ET_COMPATIBLECLIENT
	SO_LPHANT			= 20,	// ET_COMPATIBLECLIENT
	// other client types which are not identified with ET_COMPATIBLECLIENT
	SO_EDONKEYHYBRID	= 50,
	SO_EDONKEY,
	SO_OLDEMULE,
	SO_URL,
	SO_WEBCACHE, // [TPT] - WebCache // Superlexx - webcache - statistics
	SO_UNKNOWN
};

enum ESecureIdentState{
	IS_UNAVAILABLE		= 0,
	IS_ALLREQUESTSSEND  = 0,
	IS_SIGNATURENEEDED	= 1,
	IS_KEYANDSIGNEEDED	= 2,
};

enum EInfoPacketState{
	IP_NONE				= 0,
	IP_EDONKEYPROTPACK  = 1,
	IP_EMULEPROTPACK	= 2,
	IP_BOTH				= 3,
};

enum ESourceFrom{
	SF_SERVER			= 0,
	SF_KADEMLIA			= 1,
	SF_SOURCE_EXCHANGE	= 2,
	SF_PASSIVE			= 3,
	SF_LINK				= 4,
	SF_SLS				= 5
};

enum eSnafuReason  
{
	SNAFU_ALL			= 0,
	SNAFU_NONE			= 1,
	SNAFU_NAME			= 2,
	SNAFU_ACTION		= 3,
	SNAFU_HASHCHG		= 4,
	SNAFU_ACT			= 5,
	SNAFU_MODSTR		= 6,
	SNAFU_WOMBAT		= 7,
	SNAFU_NICK			= 8,
	SNAFU_CLIENT		= 9,
	SNAFU_MYHASH		= 10,
	SNAFU_HASH			= 11,
	SNAFU_FRIENDSHAREMOD= 12,
	SNAFU_OPCODE		= 13,
	SNAFU_CLONE			= 14,
	SNAFU_TAG			= 15,
	SNAFU_UNKNOWN		= 16,
	SNAFU_NUM			= 17
	};

struct PartFileStamp{
	CPartFile*	file;
	DWORD		timestamp;
};

#define	MAKE_CLIENT_VERSION(mjr, min, upd) \
	((UINT)(mjr)*100U*10U*100U + (UINT)(min)*100U*10U + (UINT)(upd)*100U)

//#pragma pack(2)
class CUpDownClient : public CObject
{
	DECLARE_DYNAMIC(CUpDownClient)

	friend class CUploadQueue;
public:
	//base
	CUpDownClient(CClientReqSocket* sender = 0);
	CUpDownClient(CPartFile* in_reqfile, uint16 in_port, uint32 in_userid, uint32 in_serverup, uint16 in_serverport, bool ed2kID = false);
	virtual ~CUpDownClient();

	// [TPT] - WebCache 
	// yonatan http start 
private:
	bool m_bIsTrustedOHCBSender;
	bool m_bIsAllowedToSendOHCBs;
	uint32 m_uWebCacheFlags;
	EWebCacheDownState m_eWebCacheDownState;
	EWebCacheUpState m_eWebCacheUpState;
	bool b_webcacheInfoNeeded;
protected:
	bool m_bProxy;
public:
	bool m_bIsAcceptingOurOhcbs; // default - true, set to false on OP_DONT_SEND_OHCBS
	CWebCacheDownSocket* m_pWCDownSocket;
	CWebCacheUpSocket* m_pWCUpSocket;

	bool SupportsWebCacheUDP() const {return (m_uWebCacheFlags & WC_FLAGS_UDP) && SupportsUDP();}
	bool SupportsOhcbSuppression() const {return (m_uWebCacheFlags & WC_FLAGS_NO_OHCBS);}
	bool SupportsWebCacheProtocol() const {return SupportsOhcbSuppression();} // this is the first version that supports that
	bool IsProxy() const {return m_bProxy;}
	bool IsUploadingToWebCache() const;
	bool IsDownloadingFromWebCache() const;
	bool ProcessWebCacheDownHttpResponse(const CStringAArray& astrHeaders);
	bool ProcessWebCacheDownHttpResponseBody(const BYTE* pucData, UINT uSize);
	bool ProcessWebCacheUpHttpResponse(const CStringAArray& astrHeaders);
	UINT ProcessWebCacheUpHttpRequest(const CStringAArray& astrHeaders);
	void OnWebCacheDownSocketClosed(int nErrorCode);
	void OnWebCacheDownSocketTimeout();
	void SetWebCacheDownState(EWebCacheDownState eState);
	EWebCacheDownState CUpDownClient::GetWebCacheDownState() const {return m_eWebCacheDownState;}
	void SetWebCacheUpState(EWebCacheUpState eState);
	EWebCacheUpState CUpDownClient::GetWebCacheUpState() const {return m_eWebCacheUpState;}
	virtual bool SendWebCacheBlockRequests();
	void PublishWebCachedBlock( const Requested_Block_Struct* block );
	bool IsWebCacheUpSocketConnected() const;
	bool IsWebCacheDownSocketConnected() const;
//	uint16 blocksLoaded;	// Superlexx - block transfer limiter //JP blocks are counted in the socket code now
	uint16 GetNumberOfClientsBehindOurWebCacheAskingForSameFile();	// what a name ;)
	uint16 GetNumberOfClientsBehindOurWebCacheHavingSameFileAndNeedingThisBlock(Pending_Block_Struct* pending); // Superlexx - COtN - it's getting better all the time...
	bool WebCacheInfoNeeded() {return b_webcacheInfoNeeded;}
	void SetWebCacheInfoNeeded(bool value) {b_webcacheInfoNeeded = value;}
//JP trusted-OHCB-senders START
	uint32 WebCachedBlockRequests;
	uint32 SuccessfulWebCachedBlockDownloads;
	bool IsTrustedOHCBSender() const {return m_bIsTrustedOHCBSender;}
	void AddWebCachedBlockToStats( bool IsGood );
//JP trusted-OHCB-senders END
//JP stop sendig OHCBs START
	void SendStopOHCBSending();
	void SendResumeOHCBSendingTCP();
	void SendResumeOHCBSendingUDP();
//JP stop sendig OHCBs END
	CWebCacheCryptography Crypt; // Superlexx - encryption
	uint32 lastMultiOHCBPacketSent;
	void SendOHCBsNow();
	bool a(uint32 a) {return a <= WC_MAX_OHCBS_IN_UDP_PACKET;}
	bool b(uint32 a, uint32 b) { return a * 100.0 / (b + 1) < 0,2; }
	bool c(uint32 a, uint32 b) { return a > 4 && (b * 100) / a < 20; }
// yonatan http end
	// [TPT] - WebCache 

	// [TPT] - Maella -Upload Stop Reason-
	enum UpStopReason {USR_NONE, USR_TIMEOVER, USR_NEW_CHUNK, USR_CANCELLED, USR_DIFFERENT_FILE, USR_EXCEPTION, USR_SNAFU}; // [TPT] - added snafu

	void			StartDownload();
	virtual void	CheckDownloadTimeout();
	virtual void	SendCancelTransfer(Packet* packet = NULL);
	virtual bool	IsEd2kClient() const							{ return true; }	
	virtual bool	Disconnected(LPCTSTR pszReason, bool bFromSocket = false, UpStopReason reason = USR_NONE); // [TPT] - Maella - Stop Reason-
	
	virtual bool	TryToConnect(bool bIgnoreMaxCon = false, CRuntimeClass* pClassSocket = NULL);
	virtual bool	Connect();
	virtual void	ConnectionEstablished();
	virtual void	OnSocketConnected(int nErrorCode);
	bool			CheckHandshakeFinished(UINT protocol, UINT opcode) const;
	void			CheckFailedFileIdReqs(const uchar* aucFileHash);
	uint32			GetUserIDHybrid() const							{ return m_nUserIDHybrid; }
	void			SetUserIDHybrid(uint32 val)						{ m_nUserIDHybrid = val; }
	LPCTSTR			GetUserName() const								{ return m_pszUsername; }
	void			SetUserName(LPCTSTR pszNewName);
	uint32			GetIP() const									{ return m_dwUserIP; }
	void			SetIP( uint32 val ) //Only use this when you know the real IP or when your clearing it.
						{
							m_dwUserIP = val;
							m_nConnectIP = val;
						}
	bool			HasLowID() const;
	uint32			GetConnectIP() const							{ return m_nConnectIP; }
	uint16			GetUserPort() const								{ return m_nUserPort; }
	void			SetUserPort(uint16 val)							{ m_nUserPort = val; }
	uint32			GetTransferredUp() const							{ return m_nTransferredUp; }
	uint32			GetTransferredDown() const						{ return m_nTransferredDown; }
	uint32			GetServerIP() const								{ return m_dwServerIP; }
	void			SetServerIP(uint32 nIP)							{ m_dwServerIP = nIP; }
	uint16			GetServerPort() const							{ return m_nServerPort; }
	void			SetServerPort(uint16 nPort)						{ m_nServerPort = nPort; }
	const uchar*	GetUserHash() const								{ return (uchar*)m_achUserHash; }
	void			SetUserHash(const uchar* pUserHash);
	bool			HasValidHash() const
						{
							return ((const int*)m_achUserHash[0]) != 0 || ((const int*)m_achUserHash[1]) != 0 || ((const int*)m_achUserHash[2]) != 0 || ((const int*)m_achUserHash[3]) != 0;
						}
	int				GetHashType() const;
	const uchar*	GetBuddyID() const								{ return (uchar*)m_achBuddyID; }
	void			SetBuddyID(const uchar* m_achTempBuddyID);
	bool			HasValidBuddyID() const							{ return m_bBuddyIDValid; }
	void			SetBuddyIP( uint32 val )						{ m_nBuddyIP = val; }
	uint32			GetBuddyIP() const								{ return m_nBuddyIP; }
	void			SetBuddyPort( uint16 val )						{ m_nBuddyPort = val; }
	uint16			GetBuddyPort() const							{ return m_nBuddyPort; }
	EClientSoftware	GetClientSoft() const							{ return (EClientSoftware)m_clientSoft; }
	const CString&	GetClientSoftVer() const						{ return m_strClientSoftware; }
	const CString&	GetClientModVer() const							{ return m_strModVersion; }
	void			ReGetClientSoft();
	uint32			GetVersion() const								{ return m_nClientVersion; }
	uint8			GetMuleVersion() const							{ return m_byEmuleVersion; }
	bool			ExtProtocolAvailable() const					{ return m_bEmuleProtocol; }
	bool			SupportMultiPacket() const						{ return m_bMultiPacket; }
	bool			SupportPeerCache() const						{ return m_fPeerCache; }
	bool			IsEmuleClient() const							{ return m_byEmuleVersion; }
	uint8			GetSourceExchangeVersion() const				{ return m_bySourceExchangeVer; }
	CClientCredits* Credits() const									{ return credits; }
	bool			IsBanned() const;
	const CString&	GetClientFilename() const						{ return m_strClientFilename; }
	void			SetClientFilename(const CString& fileName)		{ m_strClientFilename = fileName; }
	uint16			GetUDPPort() const								{ return m_nUDPPort; }
	void			SetUDPPort(uint16 nPort)						{ m_nUDPPort = nPort; }
	uint8			GetUDPVersion() const							{ return m_byUDPVer; }
	bool			SupportsUDP() const								{ return GetUDPVersion() != 0 && m_nUDPPort != 0; }
	uint16			GetKadPort() const								{ return m_nKadPort; }
	void			SetKadPort(uint16 nPort)						{ m_nKadPort = nPort; }
	uint8			GetExtendedRequestsVersion() const				{ return m_byExtendedRequestsVer; }

	// [TPT] - itsonlyme: viewSharedFiles
	void			RequestSharedFileList(bool bForce = false);
	bool			SendDirRequest(CString path, bool bForce = false);
	void			ProcessSharedFileList(char* pachPacket, uint32 nSize, LPCTSTR pszDirectory = NULL);
	void			ProcessSharedDirsList(char* pachPacket, uint32 nSize);
	CRBMap<CString, bool>	*GetListDirs() { return &m_listDirs; }
	CList<CSearchFile*>	*GetListFiles() { return &m_listFiles; }
	void			SetDeniesShare(bool in = true, bool updateTree = true);
	bool			GetDeniesShare() { return m_bDeniesShare; }
	// [TPT] - itsonlyme: viewSharedFiles	
	
	void			ClearHelloProperties();
	bool			ProcessHelloAnswer(char* pachPacket, uint32 nSize);
	bool			ProcessHelloPacket(char* pachPacket, uint32 nSize);
	void			SendHelloAnswer();
	virtual bool	SendHelloPacket();
	void			SendMuleInfoPacket(bool bAnswer);
	void			ProcessMuleInfoPacket(char* pachPacket, uint32 nSize);
	void			ProcessMuleCommentPacket(char* pachPacket, uint32 nSize);
	void			ProcessEmuleQueueRank(char* packet, UINT size);
	void			ProcessEdonkeyQueueRank(char* packet, UINT size);
	void			CheckQueueRankFlood();
	//<<< eWombat [SNAFU_V3]
	void			ProcessUnknownHelloTag(CTag *tag);
	void			ProcessUnknownInfoTag(CTag *tag);
	//>>> eWombat [SNAFU_V3]
	bool			Compare(const CUpDownClient* tocomp, bool bIgnoreUserhash = false) const;
	void			ResetFileStatusInfo();
	uint32			GetLastSrcReqTime() const						{ return m_dwLastSourceRequest; }
	void			SetLastSrcReqTime()								{ m_dwLastSourceRequest = ::GetTickCount(); }
	uint32			GetLastSrcAnswerTime() const					{ return m_dwLastSourceAnswer; }
	void			SetLastSrcAnswerTime()							{ m_dwLastSourceAnswer = ::GetTickCount(); }
	uint32			GetLastAskedForSources() const					{ return m_dwLastAskedForSources; }
	void			SetLastAskedForSources()						{ m_dwLastAskedForSources = ::GetTickCount(); }
	bool			GetFriendSlot() const;
	void			SetFriendSlot(bool bNV)							{ m_bFriendSlot = bNV; }
	bool			IsFriend() const								{ return m_Friend != NULL; }
	void			SetCommentDirty(bool bDirty = true)				{ m_bCommentDirty = bDirty; }
	bool			GetSentCancelTransfer() const					{ return m_fSentCancelTransfer; }
	void			SetSentCancelTransfer(bool bVal)				{ m_fSentCancelTransfer = bVal; }
	void			ProcessPublicIPAnswer(const BYTE* pbyData, UINT uSize);
	void			SendPublicIPRequest();
	// [TPT] - SLUGFILLER: modelessDialogs
	CClientDetailDialogInterface*	GetDetailDialogInterface() const { return m_detailDialogInterface; }
	// [TPT] - SLUGFILLER: modelessDialogs
	uint8			GetKadVersion()									{ return m_byKadVersion; }
	bool			SendBuddyPingPong()								{ return m_dwLastBuddyPingPongTime < ::GetTickCount(); }
	bool			AllowIncomeingBuddyPingPong()					{ return m_dwLastBuddyPingPongTime < (::GetTickCount()-(3*60*1000)); }
	void			SetLastBuddyPingPongTime()						{ m_dwLastBuddyPingPongTime = (::GetTickCount()+(10*60*1000)); }
	// secure ident
	void			SendPublicKeyPacket();
	void			SendSignaturePacket();
	void			ProcessPublicKeyPacket(uchar* pachPacket, uint32 nSize);
	void			ProcessSignaturePacket(uchar* pachPacket, uint32 nSize);
	uint8			GetSecureIdentState() const						{ return m_SecureIdentState; }
	void			SendSecIdentStatePacket();
	void			ProcessSecIdentStatePacket(uchar* pachPacket, uint32 nSize);
	uint8			GetInfoPacketsReceived() const					{ return m_byInfopacketsReceived; }
	void			InfoPacketsReceived();
	// preview
	void			SendPreviewRequest(const CAbstractFile* pForFile);
	void			SendPreviewAnswer(const CKnownFile* pForFile, CxImage** imgFrames, uint8 nCount);
	void			ProcessPreviewReq(char* pachPacket, uint32 nSize);
	void			ProcessPreviewAnswer(char* pachPacket, uint32 nSize);
	bool			GetPreviewSupport() const						{ return m_fSupportsPreview && GetViewSharedFilesSupport(); }
	bool			GetViewSharedFilesSupport() const				{ return m_fNoViewSharedFiles==0; }
	bool			SafeSendPacket(Packet* packet);
	void			CheckForGPLEvilDoer();
	//upload
	EUploadState	GetUploadState() const							{ return (EUploadState)m_nUploadState; }
	void			SetUploadState(EUploadState news);
	
	sint64			GetWaitStartTime() const; // [TPT] - SUQWT
	
	void 			SetWaitStartTime();
	void 			ClearWaitStartTime();
	uint32			GetWaitTime() const								{ return m_dwUploadTime - GetWaitStartTime(); }
	bool			IsDownloading() const							{ return (m_nUploadState == US_UPLOADING); }
	bool			HasBlocks() const								{ return !m_BlockRequests_queue.IsEmpty(); }
	// [TPT] - Pawcio: MUS
	bool			IsRequest() const 								{ return !m_BlockRequests_queue.IsEmpty(); }
	// [TPT]
	/*uint32			GetDatarate() const								{ return m_nUpDatarate; }		*/
	
	uint32			GetScore(bool sysvalue, bool isdownloading = false, bool onlybasevalue = false) const;
	void			AddReqBlock(Requested_Block_Struct* reqblock);
	bool			CreateNextBlockPackage();	// [TPT]
	uint32			GetUpStartTimeDelay() const						{ return ::GetTickCount() - m_dwUploadTime; }
	void 			SetUpStartTime()
					{
						m_dwUploadTime = ::GetTickCount();
                        m_dwShieldTime = 0; // MinToMB
					}
	void			SendHashsetPacket(char* forfileid);
	const uchar*	GetUploadFileID() const							{ return requpfileid; }
	void			SetUploadFileID(CKnownFile* newreqfile);
	uint32			SendBlockData();
	void			ClearUploadBlockRequests();
	void			SendRankingInfo();
	void			SendCommentInfo(/*const*/ CKnownFile *file);
	void			AddRequestCount(const uchar* fileid);	
	void			UnBan();
	void			Ban(LPCTSTR pszReason = NULL);
	uint32			GetAskedCount() const							{ return m_cAsked; }
	void			AddAskedCount()									{ m_cAsked++; }
	void			SetAskedCount(uint32 m_cInAsked)				{ m_cAsked = m_cInAsked; }
	void			FlushSendBlocks(); // call this when you stop upload, or the socket might be not able to send
	uint32			GetLastUpRequest() const						{ return m_dwLastUpRequest; }
	void			SetLastUpRequest()								{ m_dwLastUpRequest = ::GetTickCount(); }

	uint32			GetSessionUp() const							{ return m_nTransferredUp - m_nCurSessionUp; }
	void			ResetSessionUp() {
						m_nCurSessionUp = m_nTransferredUp;
						m_addedPayloadQueueSession = 0; 
						m_nCurQueueSessionPayloadUp = 0;
					} 

	uint32			GetSessionDown() const							{ return m_nTransferredDown - m_nCurSessionDown; }
	void			ResetSessionDown() {
						m_nCurSessionDown = m_nTransferredDown;
					}
	uint32			GetQueueSessionPayloadUp() const				{ return m_nCurQueueSessionPayloadUp; }
    uint32          GetPayloadInBuffer() const						{ return m_addedPayloadQueueSession - GetQueueSessionPayloadUp(); }

	void			ProcessExtendedInfo(CSafeMemFile* packet, CKnownFile* tempreqfile);
	uint16			GetUpPartCount() const							{ return m_nUpPartCount; }
	void			DrawUpStatusBar(CDC* dc, RECT* rect, bool onlygreyrect, bool  bFlat) const;
	bool			IsUpPartAvailable(uint16 iPart) const {
						return (iPart>=m_nUpPartCount || !m_abyUpPartStatus) ? 0 : m_abyUpPartStatus[iPart];
					}
	uint8*			GetUpPartStatus() const							{ return m_abyUpPartStatus; }
    float           GetCombinedFilePrioAndCredit();
	
	// [TPT] - Powershare
	bool			GetPowerShared() const;
	
	//download
	uint32			GetAskedCountDown() const						{ return m_cDownAsked; }
	void			AddAskedCountDown()								{ m_cDownAsked++; }
	void			SetAskedCountDown(uint32 m_cInDownAsked)		{ m_cDownAsked = m_cInDownAsked; }
	EDownloadState	GetDownloadState() const						{ return (EDownloadState)m_nDownloadState; }
	
	uint32			GetLastAskedTime(const CPartFile* partFile = NULL) const;
    void            SetLastAskedTime()								{ m_fileReaskTimes.SetAt(reqfile, ::GetTickCount()); }
	bool			IsPartAvailable(uint16 iPart) const {
						return (iPart>=m_nPartCount || !m_abyPartStatus) ? 0 : m_abyPartStatus[iPart];
					}
	uint8*			GetPartStatus() const							{ return m_abyPartStatus; }
	uint16			GetPartCount() const							{ return m_nPartCount; }
	uint32			GetDownloadDatarate() const						{ return m_nDownDatarate; }
	// [TPT] - itsonlyme: displayOptions
	uint16   		GetDifference() const			
					{
						return m_nDifferenceQueueRank;
					}	
	// [TPT] - itsonlyme: displayOptions
	uint16			GetRemoteQueueRank() const						{ return m_nRemoteQueueRank; }
	void			SetRemoteQueueRank(uint16 nr);
	bool			IsRemoteQueueFull() const						{ return m_bRemoteQueueFull; }
	void			SetRemoteQueueFull(bool flag)					{ m_bRemoteQueueFull = flag; }
	void			DrawStatusBar(CDC* dc, LPCRECT rect, bool onlygreyrect, bool  bFlat) const;
	bool			AskForDownload();
	virtual void	SendFileRequest();
	void			SendStartupLoadReq();
	void			ProcessFileInfo(CSafeMemFile* data, CPartFile* file);
	void			ProcessFileStatus(bool bUdpPacket, CSafeMemFile* data, CPartFile* file);
	void			ProcessHashSet(char* data, uint32 size);
	void			ProcessAcceptUpload();
	bool			AddRequestForAnotherFile(CPartFile* file);
	void			CreateBlockRequests(int iMaxBlocks);
	virtual void	SendBlockRequests();
	virtual bool	SendHttpBlockRequests();
	virtual void	ProcessBlockPacket(char* packet, uint32 size, bool packed = false);
	virtual void	ProcessHttpBlockPacket(const BYTE* pucData, UINT uSize);
	void			ClearDownloadBlockRequests();
	void			SendOutOfPartReqsAndAddToWaitingQueue();
	uint32			CalculateDownloadRate();
	UINT			GetAvailablePartCount() const;
	bool			SwapToAnotherFile(LPCTSTR pszReason, bool bIgnoreNoNeeded, bool ignoreSuspensions, bool bRemoveCompletely, CPartFile* toFile = NULL, bool allowSame = true, bool isAboutToAsk = false, bool debug = false); // ZZ:DownloadManager
	void			DontSwapTo(/*const*/ CPartFile* file);
	bool			IsSwapSuspended(const CPartFile* file, const bool allowShortReaskTime = false, const bool fileIsNNP = false) /*const*/; // ZZ:DownloadManager
    uint32          GetTimeUntilReask() const;
    uint32          GetTimeUntilReask(const CPartFile* file) const;
    uint32			GetTimeUntilReask(const CPartFile* file, const bool allowShortReaskTime, const bool useGivenNNP = false, const bool givenNNP = false) const;
	void			UDPReaskACK(uint16 nNewQR);
	void			UDPReaskFNF();
	void			UDPReaskForDownload();
	void			RequestHashset();
	bool			IsSourceRequestAllowed() const;
    bool            IsSourceRequestAllowed(CPartFile* partfile, bool sourceExchangeCheck = false) const; // ZZ:DownloadManager

	bool			IsValidSource() const;
	ESourceFrom		GetSourceFrom() const							{ return (ESourceFrom)m_nSourceFrom; }
	void			SetSourceFrom(ESourceFrom val)					{ m_nSourceFrom = val; }

	void			SetDownStartTime()								{ m_dwDownStartTime = ::GetTickCount(); }
	uint32			GetDownTimeDifference(boolean clear = true)	{
						uint32 myTime = m_dwDownStartTime;
						if(clear) m_dwDownStartTime = 0;
						return ::GetTickCount() - myTime;
					}
	bool			GetTransferredDownMini() const					{ return m_bTransferredDownMini; }
	void			SetTransferredDownMini()						{ m_bTransferredDownMini = true; }
	void			InitTransferredDownMini()						{ m_bTransferredDownMini = false; }
	uint16			GetA4AFCount() const							{ return m_OtherRequests_list.GetCount(); }

	uint16			GetUpCompleteSourcesCount() const				{ return m_nUpCompleteSourcesCount; }
	void			SetUpCompleteSourcesCount(uint16 n)				{ m_nUpCompleteSourcesCount = n; }

	// [TPT] - Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
    uint32			GetUploadDatarate(uint32 samples) const;
    uint32			GetUploadDatarate() const 
    				{
    					return m_nUpDatarate;
    				}
	
	void			AddUploadRate(uint32 size) 
					{ 
						m_nUpDatarateMeasure += size; 
						m_nTransferredUp += size;
						credits->AddUploaded(size, GetIP());
					} 
	void			AddDownloadRate(uint32 size) 
					{
						m_nDownDatarateMeasure += size; 

					} 
	void			CompUploadRate(); // Actualize datarate
	void			CompDownloadRate(); // Actualize datarate		
	// Maella end

	//chat
	EChatState		GetChatState() const							{ return (EChatState)m_nChatstate; }
	void			SetChatState(EChatState nNewS)					{ m_nChatstate = nNewS; }

	//KadIPCheck
	EKadState		GetKadState() const								{ return (EKadState)m_nKadState; }
	void			SetKadState(EKadState nNewS)					{ m_nKadState = nNewS; }

	// [TPT] - SLUGFILLER: showComments remove - no per-client comments

	// Barry - Process zip file as it arrives, don't need to wait until end of block
	int				unzip(Pending_Block_Struct* block, BYTE* zipped, uint32 lenZipped, BYTE** unzipped, uint32* lenUnzipped, int iRecursion = 0);
	void			UpdateDisplayedInfo(bool force = false);
	int             GetFileListRequested() const					{ return m_iFileListRequested; }
    void            SetFileListRequested(int iFileListRequested)	{ m_iFileListRequested = iFileListRequested; }

	// message filtering
	uint8			GetMessagesReceived() const						{ return m_cMessagesReceived; }
	void			SetMessagesReceived(uint8 nCount)				{ m_cMessagesReceived = nCount; }
	void			IncMessagesReceived()							{ m_cMessagesReceived++; }
	uint8			GetMessagesSent() const							{ return m_cMessagesSent; }
	void			SetMessagesSent(uint8 nCount)					{ m_cMessagesSent = nCount; }
	void			IncMessagesSent()								{ m_cMessagesSent++; }
	bool			IsSpammer() const								{ return m_fIsSpammer; }
	void			SetSpammer(bool bVal)							{ m_fIsSpammer = bVal ? 1 : 0; }
	bool			GetMessageFiltered() const						{ return m_fMessageFiltered; }
	void			SetMessageFiltered(bool bVal)					{ m_fMessageFiltered = bVal ? 1 : 0; }

	virtual void	SetRequestFile(CPartFile* pReqFile);
	CPartFile*		GetRequestFile() const							{ return reqfile; }

	// AICH Stuff
	void			SetReqFileAICHHash(CAICHHash* val);
	CAICHHash*		GetReqFileAICHHash() const						{ return m_pReqFileAICHHash; }
	bool			IsSupportingAICH() const						{ return m_fSupportsAICH & 0x01; }
	void			SendAICHRequest(CPartFile* pForFile, uint16 nPart);
	bool			IsAICHReqPending() const						{ return m_fAICHRequested; }
	void			ProcessAICHAnswer(char* packet, UINT size);
	void			ProcessAICHRequest(char* packet, UINT size);
	void			ProcessAICHFileHash(CSafeMemFile* data, CPartFile* file);

	EUtf8Str		GetUnicodeSupport() const;

	CString			GiveMetheQRString() const;
	CString			GetDownloadStateDisplayString() const;
	CString			GetUploadStateDisplayString() const;

	LPCTSTR			DbgGetDownloadState() const;
	LPCTSTR			DbgGetUploadState() const;
	CString			DbgGetClientInfo(bool bFormatIP = false) const;
	CString			DbgGetFullClientSoftVer() const;
	const CString&	DbgGetHelloInfo() const							{ return m_strHelloInfo; }
	const CString&	DbgGetMuleInfo() const							{ return m_strMuleInfo; }

// ZZ:DownloadManager -->
    const bool      IsInNoNeededList(const CPartFile* fileToCheck) const;
    const bool      SwapToRightFile(CPartFile* SwapTo, CPartFile* cur_file, bool ignoreSuspensions, bool SwapToIsNNPFile, bool isNNPFile, bool& wasSkippedDueToSourceExchange, bool doAgressiveSwapping = false, bool debug = false);
    const DWORD     getLastTriedToConnectTime() { return m_dwLastTriedToConnect; }
// <-- ZZ:DownloadManager

#ifdef _DEBUG
	// Diagnostic Support
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	CClientReqSocket* socket;
	CClientCredits*	credits;
	CFriend*		m_Friend;
	uint8*			m_abyUpPartStatus;
	CTypedPtrList<CPtrList, CPartFile*> m_OtherRequests_list;
	CTypedPtrList<CPtrList, CPartFile*> m_OtherNoNeeded_list;
	uint16			m_lastPartAsked;
	bool			m_bAddNextConnect;  // VQB Fix for LowID slots only on connection

    void			SetSlotNumber(uint32 newValue)					{ m_slotNumber = newValue; }
    uint32			GetSlotNumber() const							{ return m_slotNumber; }
    CClientReqSocket* GetFileUploadSocket(bool log = false); // [TPT] - Fix
    
	bool		toRemove; // [TPT] - Pawcio: MUS
	// [TPT] - Show pHoeniX icon on client detect
	bool GetpHoeniXClient() const { return m_bpHoeniXClient;}
	uint32		MinToMB() const; // MinToMB
	uint32		m_dwShieldTime; // MinToMB
// [TPT] - WebCache 
// yonatan http start //////////////////////////////////////////////////////////////////////////
	// Squid defaults..
	bool			UsesCachedTCPPort() { return ( (GetUserPort()==80)
													|| (GetUserPort()==21)
													|| (GetUserPort()==443)
													|| (GetUserPort()==563)
													|| (GetUserPort()==70)
													|| (GetUserPort()==210)
													|| ((GetUserPort()>=1025) && (GetUserPort()<=65535)));}
	bool			SupportsWebCache() const { return m_bWebCacheSupport; }
	bool			SupportsMultiOHCBs() const {return m_bWebCacheSupportsMultiOHCBs;}
	bool			IsBehindOurWebCache() const
					{
						// WC-TODO: make this more efficient
						return( thePrefs.webcacheName == GetWebCacheName() );
					}
// jp webcache
	CString			GetWebCacheName() const 
					{ 
						if (SupportsWebCache())
							return WebCacheIndex2Name(m_WA_webCacheIndex);
						else
							return _T("");
					}
	bool			m_bWebCacheSupport;
	bool			m_bWebCacheSupportsMultiOHCBs;
	// Superlexx - webcache
	int		m_WA_webCacheIndex;	// index of the webcache name
	uint16	m_WA_HTTPPort;		// remote webserver port
	uint32	m_uWebCacheDownloadId;	// we must attach this ID when sending HTTP download request to the remote client.
	uint32	m_uWebCacheUploadId;	// incoming HTTP requests are identified as WC-requests,
									// if the header contains this ID and there is a known client with same ID in downloading state.
									// used for client authorization, should be substituted by a HttpIdList? later
									// for efficiency reasons.
	// Superlexx - MFR
	CWebCacheMFRList	requestedFiles; // the files this client requested from us
	Packet*	CreateMFRPacket();		// builds a separate MFR-packet
	uint8	AttachMultiOHCBsRequest(CSafeMemFile &data); // Superlexx - attaches a multiple files request
	uint8	IsPartAvailable(uint16 iPart, const byte* fileHash) {return requestedFiles.IsPartAvailable(iPart, fileHash);}

// yonatan http end ////////////////////////////////////////////////////////////////////////////
// [TPT] - WebCache 

	// [TPT] - Powershare
	uint8*			m_abyUpPartStatusHidden; //MORPH - Added by SiRoB, See chunk that we hide
	bool			m_bUpPartStatusHiddenBySOTN; //MORPH - Added by SiRoB, See chunk that we hide	
    // [TPT] - Powershare
	// [TPT] - enkeyDev: ICS
	void	ProcessFileIncStatus(CSafeMemFile* data,uint32 size, bool readHash = false);
	uint32	GetIncompletePartVersion()	{return m_incompletepartVer;}
	bool	IsIncPartAvailable(uint16 iPart)	{return	( (iPart >= m_nPartCount) || (!m_abyIncPartStatus) )? 0:m_abyIncPartStatus[iPart];}
	bool	IsPendingListEmpty()      {return m_PendingBlocks_list.IsEmpty();}
    // [TPT] - enkeyDev: ICS

	///////////////////////////////////////////////////////////////////////////
	// PeerCache client
	//
	bool IsDownloadingFromPeerCache() const;
	bool IsUploadingToPeerCache() const;
	void SetPeerCacheDownState(EPeerCacheDownState eState);
	void SetPeerCacheUpState(EPeerCacheUpState eState);

	int  GetHttpSendState() const									{ return m_iHttpSendState; }
	void SetHttpSendState(int iState)								{ m_iHttpSendState = iState; }

	bool SendPeerCacheFileRequest();
	bool ProcessPeerCacheQuery(const char* packet, UINT size);
	bool ProcessPeerCacheAnswer(const char* packet, UINT size);
	bool ProcessPeerCacheAcknowledge(const char* packet, UINT size);
	void OnPeerCacheDownSocketClosed(int nErrorCode);
	bool OnPeerCacheDownSocketTimeout();
	
	bool ProcessPeerCacheDownHttpResponse(const CStringAArray& astrHeaders);
	bool ProcessPeerCacheDownHttpResponseBody(const BYTE* pucData, UINT uSize);
	bool ProcessPeerCacheUpHttpResponse(const CStringAArray& astrHeaders);
	UINT ProcessPeerCacheUpHttpRequest(const CStringAArray& astrHeaders);

	virtual bool ProcessHttpDownResponse(const CStringAArray& astrHeaders);
	virtual bool ProcessHttpDownResponseBody(const BYTE* pucData, UINT uSize);

	CPeerCacheDownSocket* m_pPCDownSocket;
	CPeerCacheUpSocket* m_pPCUpSocket;

protected:
	int		m_iHttpSendState;
	uint32	m_uPeerCacheDownloadPushId;
	uint32	m_uPeerCacheUploadPushId;
	uint32	m_uPeerCacheRemoteIP;
	bool	m_bPeerCacheDownHit;
	bool	m_bPeerCacheUpHit;
	EPeerCacheDownState m_ePeerCacheDownState;
	EPeerCacheUpState m_ePeerCacheUpState;

protected:
	// base
	void	Init();
	bool	ProcessHelloTypePacket(CSafeMemFile* data);
	void	SendHelloTypePacket(CSafeMemFile* data);
	void	CreateStandartPackets(byte* data,uint32 togo, Requested_Block_Struct* currentblock, bool bFromPF = true);
	void	CreatePackedPackets(byte* data,uint32 togo, Requested_Block_Struct* currentblock, bool bFromPF = true);

	uint32	m_nConnectIP;		// holds the supposed IP or (after we had a connection) the real IP
	uint32	m_dwUserIP;			// holds 0 (real IP not yet available) or the real IP (after we had a connection)
	uint32	m_dwServerIP;
	uint32	m_nUserIDHybrid;
	uint16	m_nUserPort;
	uint16	m_nServerPort;
	uint32	m_nClientVersion;	
	//--group to aligned int32
	uint8	m_byEmuleVersion;
	uint8	m_byDataCompVer;
	bool	m_bEmuleProtocol;
	bool	m_bIsHybrid;
	//--group to aligned int32
	TCHAR*	m_pszUsername;
	uchar	m_achUserHash[16];
	uint16	m_nUDPPort;
	uint16	m_nKadPort;
	//--group to aligned int32
	uint8	m_byUDPVer;
	uint8	m_bySourceExchangeVer;
	uint8	m_byAcceptCommentVer;
	uint8	m_byExtendedRequestsVer;
	//--group to aligned int32
	uint8	m_byCompatibleClient;
	bool	m_bFriendSlot;
	bool	m_bCommentDirty;
	bool	m_bIsML;
	//--group to aligned int32
	bool	m_bGPLEvildoer;
	bool	m_bHelloAnswerPending;
	uint8	m_byInfopacketsReceived;	// have we received the edonkeyprot and emuleprot packet already (see InfoPacketsReceived() )
	uint8	m_bySupportSecIdent;
	//--group to aligned int32
	uint32	m_dwLastSignatureIP;
	CString m_strClientSoftware;
	CString m_strModVersion;
	uint32	m_dwLastSourceRequest;
	uint32	m_dwLastSourceAnswer;
	uint32	m_dwLastAskedForSources;
    int     m_iFileListRequested;
    // [TPT] - itsonlyme: viewSharedFiles
	bool	m_bRequestingFileList;
	bool	m_bFileListRequested;
	bool	m_bDeniesShare;
	CRBMap<CString, bool>	m_listDirs;
	CList<CSearchFile *>	m_listFiles;
	// [TPT] - itsonlyme: viewSharedFiles
	// [TPT] - SLUGFILLER: showComments remove - no per-client comments	
	uint8	m_cMessagesReceived;		// count of chatmessages he sent to me
	uint8	m_cMessagesSent;			// count of chatmessages I sent to him
	bool	m_bMultiPacket;
	//--group to aligned int32
	bool	m_bUnicodeSupport;
	bool	m_bBuddyIDValid;
	uint16	m_nBuddyPort;
	//--group to aligned int32
	uint32	m_nBuddyIP;
	uint32	m_dwLastBuddyPingPongTime;
	uchar	m_achBuddyID[16];
	CString m_strHelloInfo;
	CString m_strMuleInfo;
	uint8	m_byKadVersion;

	// States
#ifdef _DEBUG
	// use the 'Enums' only for debug builds, each enum costs 4 bytes (3 unused)
	EClientSoftware		m_clientSoft;
	EChatState			m_nChatstate;
	EKadState			m_nKadState;
	ESecureIdentState	m_SecureIdentState;
	EUploadState		m_nUploadState;
	EDownloadState		m_nDownloadState;
	ESourceFrom			m_nSourceFrom;
#else
	uint8 m_clientSoft;
	uint8 m_nChatstate;
	uint8 m_nKadState;
	uint8 m_SecureIdentState;
	uint8 m_nUploadState;
	uint8 m_nDownloadState;
	uint8 m_nSourceFrom;
#endif

	CTypedPtrList<CPtrList, Packet*> m_WaitingPackets_list;
	CList<PartFileStamp> m_DontSwap_list;

	////////////////////////////////////////////////////////////////////////
	// Upload
	//
    int GetFilePrioAsNumber() const;

	uint32		m_nTransferredUp;
	uint32		m_dwUploadTime;
	uint32		m_cAsked;
	uint32		m_dwLastUpRequest;
	uint32		m_nCurSessionUp;
	uint32		m_nCurSessionDown;
    uint32      m_nCurQueueSessionPayloadUp;
    uint32      m_addedPayloadQueueSession;
	uint16		m_nUpPartCount;
	uint16		m_nUpCompleteSourcesCount;
	uchar		requpfileid[16];
    uint32      m_slotNumber;

	bool m_bpHoeniXClient; 	// [TPT] - Show pHoeniX icon on client detect

// [TPT] - Maella -New bandwidth control-

/*typedef struct TransferredData {
		uint32	datalen;
		DWORD	timestamp;
	};
	CList<TransferredData,TransferredData>			 m_AvarageUDR_list; // By BadWolf*/
	// Maella end


	CTypedPtrList<CPtrList, Requested_Block_Struct*> m_BlockRequests_queue;
	CTypedPtrList<CPtrList, Requested_Block_Struct*> m_DoneBlocks_list;
	CTypedPtrList<CPtrList, Requested_File_Struct*>	 m_RequestedFiles_list;

	//////////////////////////////////////////////////////////
	// Download
	//
	CPartFile*	reqfile;
	CAICHHash*  m_pReqFileAICHHash; 
	uint32		m_cDownAsked;
	uint8*		m_abyPartStatus;
	CString		m_strClientFilename;
	uint32		m_nTransferredDown;
	uint32		m_dwDownStartTime;
	uint32      m_nLastBlockOffset;	
	uint16		m_nDifferenceQueueRank;	// [TPT] - itsonlyme: displayOptions
	uint32		m_dwLastBlockReceived;
	uint32		m_nTotalUDPPackets;
	uint32		m_nFailedUDPPackets;
	//--group to aligned int32
	uint16		m_cShowDR;
	uint16		m_nRemoteQueueRank;
	//--group to aligned int32
	uint16		m_nPartCount;
	bool		m_bRemoteQueueFull;
	bool		m_bCompleteSource;
	//--group to aligned int32
	bool		m_bReaskPending;
	bool		m_bUDPPending;
	bool		m_bTransferredDownMini;

	// Download from URL
	CStringA	m_strUrlPath;
	UINT		m_uReqStart;
	UINT		m_uReqEnd;
	UINT		m_nUrlStartPos;


	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-

	//////////////////////////////////////////////////////////
	// Upload data rate computation
	//
	//uint32		m_nUpDatarate;
	//uint32		m_nSumForAvgUpDataRate;
	//CList<TransferredData> m_AvarageUDR_list; // [TPT]
	uint32	m_nUpDatarate; // current datarate (updated every seconds)
	uint32	m_nUpDatarateMeasure; // transfered byte since the last measure
	CList<TransferredData> m_upHistory_list;
	uint8	m_displayUpDatarateCounter; // refresh display timer


	//////////////////////////////////////////////////////////
	// Download data rate computation
	//
	uint32		m_nDownDatarate;
	//uint32		m_nDownDataRateMS;
	//uint32		m_nSumForAvgDownDataRate;//[TPT]
	//CList<TransferredData> m_AvarageDDR_list; // [TPT]
	uint8	m_displayDownDatarateCounter; // refresh display timer
	uint32	m_nDownDatarateMeasure; // transfered byte since the last measure
	uint32	m_sumDownHistory; // max 4GB
	CList<TransferredData> m_downHistory_list;

	// [TPT] - Maella end
	//////////////////////////////////////////////////////////
	// GUI helpers
	//
	//static CBarShader s_StatusBar; // [TPT]
	//static CBarShader s_UpStatusBar; // [TPT]
	DWORD		m_lastRefreshedDLDisplay;
    DWORD		m_lastRefreshedULDisplay;
    uint32      m_random_update_wait;

	// using bitfield for less important flags, to save some bytes
	UINT m_fHashsetRequesting : 1, // we have sent a hashset request to this client in the current connection
		 m_fSharedDirectories : 1, // client supports OP_ASKSHAREDIRS opcodes
		 m_fSentCancelTransfer: 1, // we have sent an OP_CANCELTRANSFER in the current connection
		 m_fNoViewSharedFiles : 1, // client has disabled the 'View Shared Files' feature, if this flag is not set, we just know that we don't know for sure if it is enabled
		 m_fSupportsPreview   : 1,
		 m_fPreviewReqPending : 1,
		 m_fPreviewAnsPending : 1,
		 m_fIsSpammer		  : 1,
		 m_fMessageFiltered   : 1,
		 m_fPeerCache		  : 1,
		 m_fQueueRankPending  : 1,
		 m_fUnaskQueueRankRecv: 2,
		 m_fFailedFileIdReqs  : 4, // nr. of failed file-id related requests per connection
		 m_fNeedOurPublicIP	  : 1, // we requested our IP from this client
		 m_fSupportsAICH	  : 3,
		 m_fAICHRequested     : 1,
		 m_fSentOutOfPartReqs : 1;

	CTypedPtrList<CPtrList, Pending_Block_Struct*>	 m_PendingBlocks_list;
	CTypedPtrList<CPtrList, Requested_Block_Struct*> m_DownloadBlocks_list;

    bool    m_bSourceExchangeSwapped; // ZZ:DownloadManager
    DWORD   lastSwapForSourceExchangeTick; // ZZ:DownloadManaager
    bool    DoSwap(CPartFile* SwapTo, bool bRemoveCompletely, LPCTSTR reason); // ZZ:DownloadManager
    CMap<CPartFile*, CPartFile*, DWORD, DWORD> m_fileReaskTimes; // ZZ:DownloadManager (one resk timestamp for each file)
    DWORD   m_dwLastTriedToConnect; // ZZ:DownloadManager (one resk timestamp for each file)
    bool    RecentlySwappedForSourceExchange() { return ::GetTickCount()-lastSwapForSourceExchangeTick < 30*1000; } // ZZ:DownloadManager
    void    SetSwapForSourceExchangeTick() { lastSwapForSourceExchangeTick = ::GetTickCount(); } // ZZ:DownloadManager
CClientDetailDialogInterface*	m_detailDialogInterface;	// SLUGFILLER: modelessDialogs
	
	// [TPT] - enkeyDev: ICS
	uint32	m_incompletepartVer;
	uint8*	m_abyIncPartStatus;
	// [TPT] - enkeyDev: ICS
	// [TPT] - netfinity: Anti HideOS
public:
	bool	IsPartialSource();
	uint8*	m_abySeenPartStatus;
	bool	WasPartSeen(uint16 iPart) const
	{
		return ( (iPart >= m_nPartCount) || (!m_abySeenPartStatus) )? 0: m_abySeenPartStatus[iPart];
	}
	// [TPT] - netfinity: Anti HideOS
	
// Maella -Extended clean-up II-
public:
	void CleanUp(CPartFile* pDeletedFile);
	DWORD m_lastCleanUpCheck;
// Maella end
// Maella -AntiCrash/AntiFake handling- (Vorlost/Mortillo)
public:
	//bool	IsBanned() const {return ((m_bBanned || m_BannedForTriedCrash) && m_nDownloadState != DS_DOWNLOADING);}
	bool	IsBannedForTriedCrash() const {return m_BannedForTriedCrash;}

private:
	bool	m_BannedForTriedCrash;
// Maella end
// Maella -Upload Stop Reason-
public:
	
	static void   AddUpStopCount(bool failed, UpStopReason reason) {++m_upStopReason[failed?0:1][reason];}
	static uint32 GetUpStopCount(bool failed, UpStopReason reason) {return m_upStopReason[failed?0:1][reason];}

// Maella -Download Stop Reason-
	enum DownStopReason {DSR_NONE, DSR_PAUSED, DSR_NONEEDEDPARTS, DSR_CORRUPTEDBLOCK, DSR_TIMEOUT, DSR_CANCELLED, DSR_OUTOFPART, DSR_EXCEPTION};
	void           SetDownloadState(EDownloadState nNewState, LPCTSTR pszReason = _T("Unspecified"), DownStopReason reason = DSR_NONE);
	
	static uint32 GetDownStopCount(bool failed, DownStopReason reason) {return m_downStopReason[failed?0:1][reason];}

private:
	static void   AddDownStopCount(bool failed, DownStopReason reason) {++m_downStopReason[failed?0:1][reason];}

	static uint32 m_upStopReason[2][USR_SNAFU+1]; // [TPT] - added snafu
	static uint32 m_downStopReason[2][DSR_EXCEPTION+1]; // [TPT] - added snafu
// Maella end
// Maella -Spread Request- (idea SlugFiller)
public:
	uint32 GetJitteredFileReaskTime() const {return m_jitteredFileReaskTime;} // range 27..31 min 

private:
	uint32 m_jitteredFileReaskTime;
// Maella end
//<<< eWombat [SUI][TPT]
public:
	//not used //EIdentState	GetSUIState()		{if (credits) return credits->GetCurrentIdentState(GetIP()); else return IS_NOTAVAILABLE;}
	//not used //uint8 GetSUIVersion() const		{return (m_bySupportSecIdent & 3);}
	//not used //void  GetSUIStateStr(CString &string);
	//not used //int	  GetSUISort();
	inline bool	IsSUI() {if (credits && credits->IsSUI(m_dwUserIP))return true;return false;}
	inline bool	IsNotSUI() const {if (credits && credits->IsNotSUI(m_dwUserIP)) return true; return false;}
	inline bool	IsSUIFailed() const {if (credits && credits->IsSUIFailed(m_dwUserIP)) return true; return false;}
	//void SetSUITime()		{m_nSUITimer=::GetTickCount;}
	void ResetSUITime()		{m_nSUITimer=0;}
	uint32 GetSUITime()		{return m_nSUITimer;}
	//>>> eWombat [SUI][TPT]
	CString *GetModStringPtr() {return m_pMod;}
	//<<< [TPT] - eWombat SNAFU v2
	int		GetBanSort(void) const;
	CString	GetBanStr() const;
	bool	IsSnafu()	const		{return (bool)(m_nSnafuReason!=SNAFU_NONE);}
	eSnafuReason	GetSnafuReason()const {return m_nSnafuReason;}
	bool	UploadAllowed()		{return !(IsSnafu() || IsNotSUI() || IsBanned());}
	DWORD	GetSnafuTime()		{return m_nSnafuTimer;}
	//void	SetSnafuTime()		{m_nSnafuTimer=::GetTickCount;}
	void	ResetSnafuTime()	{m_nSnafuTimer=0;}
	void	UnSnafu(bool bCheckTrust=false);
    void	DoSnafu(eSnafuReason reason,bool bCheckTrust,bool bLog=true);
	void	DoSnafuInfo();
	void	SnafuCheckMod(LPCTSTR pMod);
	void	SnafuCheckNick(LPCTSTR pNick);
	void	SnafuCheckVersion();
	CString	GetSnafuReasonStr() const;
	void	Trust()				{if (m_nTrust < 8 ) m_nTrust++;}
	void	UnTrust()			{m_nTrust=0;}
	void	ResetTrust()		{if (m_nTrust < 2) m_nTrust=2;}
	uint32	GetTrust()			{return m_nTrust;}
	void	SnafuExclude()		{m_bSnafuExclude=true;}
	//>>> [TPT] - eWombat SNAFU v2
	//<<< eWombat [ACT][TPT]
	DWORD			GetTrustedTransfer() {return m_nTrustedtransfer;}
	void			AddTrustedTransfer(uint32 nTransferred) {m_nTrustedtransfer+=nTransferred;}
	void			SetTrustedTransfer(uint32 nTransferred) {m_nTrustedtransfer=nTransferred;}
	bool			IsCreditTheft() {return (bool)(m_nSnafuReason==SNAFU_ACT);}
	//>>> eWombat [ACT][TPT]
	DWORD			GetBannedTime() {return m_nBanTimer;}
// Maella end

// Maella -Reask After IP Change-
public:	
	void   TrigNextSafeAskForDownload(CPartFile* pFile);

protected:
	// I was not very keen on using these artifact because it's an open  
	// door for leechers, but since Khaos did it before...
	struct PartStatus{
		PartStatus() : dwStartUploadReqTime(0) {}
		uint32 dwStartUploadReqTime; // Used to avoid Ban()
	};
	typedef std::map<CPartFile*, PartStatus> PartStatusMap;
	PartStatusMap m_partStatusMap;
// Maella end
public:
	uint32 GetNextTCPAskedTime() const {return m_dwNextTCPAskedTime;}
	void   SetNextTCPAskedTime(uint32 time) {m_dwNextTCPAskedTime = time;}
    uint32 GetLastTCPAskedTime(CPartFile* pFile) {if (!pFile) return 0; return m_partStatusMap[pFile].dwStartUploadReqTime;}
private:
	//uint32 m_dwLastAskedTime;     // Last attempt to refresh the download session with TCP or UDP // [TPT] - ZZ:DownloadManager
	uint32 m_dwLastUDPReaskTime;  // Last attempt to refresh the download session with UDP
	uint32 m_dwNextTCPAskedTime;  // Time of the next refresh for the download session with TCP
// Maella end
protected:
	//<<< [TPT] - eWombat SNAFU v2
	CString *m_pMod;
	ULONG SnafuNameCRC(CString tocalc);
	eSnafuReason	m_nSnafuReason;
	
	DWORD			m_nSnafuTimer;
	DWORD			m_nSUITimer;
	DWORD			m_nTrustedtransfer; //eWombat [ACT]
	BYTE			m_nTrust;
	ULONG			m_nNameCRC;
	DWORD			m_nBanTimer;
	//TCHAR*		m_strCommunity;
	TCHAR*			m_strSnafuTag;
	//>>> eWombat [SNAFU]
	bool			m_bSnafuExclude:1, //eWombat [SNAFU]
					m_bGoofy:1,
					m_bBan:1;
	// [TPT] - MFCK [addon] - New Tooltips [Rayita]
public:
	void GetTooltipBaseInfo(CString &info);
	void GetTooltipDownloadInfo(CString &info, bool a4af, CPartFile* file = NULL, bool base = true);
	void GetTooltipUploadInfo(CString &info, bool base = true);
	void GetTooltipQueueInfo(CString &info, bool base = true);
	void GetTooltipClientInfo(CString &info);
	// [TPT] - MFCK [addon] - New Tooltips [Rayita]

	// [TPT] - IP Country
public:	
	CString   GetCountryIP() const {return m_dwUserCountryIP;}
	void	  SetCountryIP(CString country) { m_dwUserCountryIP = country;}
private:
	CString   m_dwUserCountryIP;
	// [TPT] - IP Country
};
//#pragma pack()