#include "StdAfx.h"
#include "updownclient.h"
#include "emule.h"
#include "uploadqueue.h"
#include "Clientlist.h"
#include "SearchList.h"
#include <zlib/zlib.h>
#include "otherfunctions.h"
#include "emuleDlg.h"
#include "TransferWnd.h"
#include "Packets.h"
#include "opcodes.h"
#include "listensocket.h"
#include "log.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//<<< eWombat [SUI]
/* //Not used
void CUpDownClient::GetSUIStateStr(CString &string)
{
	if (!credits)
	{
		string.Empty();
		return;
	}
	switch(credits->GetCurrentIdentState(GetIP()))
	{
	case IS_IDNEEDED:
		string=GetResString(IDS_SUI_IDNEEDED);
		return;
	case IS_IDENTIFIED:
		string=GetResString(IDS_SUI_IDENTIFIED);
		return;
	case IS_IDFAILED:
		string=GetResString(IDS_SUI_IDFAILED);
		return;
	case IS_IDBADGUY:
		string=GetResString(IDS_SUI_IDBADGUY);
		return;
	}
	string.Empty();
	return;
}
*/
//not used
/*
int CUpDownClient::GetSUISort()
{
	if (!credits)
		return 5;
	switch(credits->GetCurrentIdentState(GetIP()))
		{
		case IS_IDENTIFIED:
			return 4;
		case IS_IDNEEDED:
			return 3;
		case IS_IDBADGUY:
			return 2;
		case IS_IDFAILED:
			return 1;
		}
	return 5;
}
*/
int CUpDownClient::GetBanSort(void) const
{
	return IsBanned()+(10*GetSnafuReason())+(2*IsNotSUI())+(4*IsSUIFailed());
}

CString CUpDownClient::GetBanStr(void) const
{
	if (IsSnafu())
		return GetSnafuReasonStr();
	else if (IsNotSUI())
		return _T("[S.U.I.]");
	else
		return (IsBanned() ? GetResString(IDS_YES):GetResString(IDS_NO));
}
//<<< eWombat [SUI]

// <<< eWombat [SNAFU]
void CUpDownClient::UnSnafu(bool bCheckTrust) 
{
	if (bCheckTrust && m_nSnafuReason != SNAFU_ACT && m_nSnafuReason != SNAFU_HASHCHG)
		return;
	ResetTrust();
	m_nSnafuReason=SNAFU_NONE;
	m_nSnafuTimer=0;
	if (m_strSnafuTag)	
	{
		free(m_strSnafuTag);		
		m_strSnafuTag = NULL;
	}
	theApp.emuledlg->transferwnd->queuelistctrl.RefreshClient(this);
	theApp.emuledlg->transferwnd->clientlistctrl.RefreshClient(this);
}

void CUpDownClient::DoSnafu(eSnafuReason reason ,bool bCheckTrust,bool bLog)
{
	if (!thePrefs.GetAntiSnafu() || m_bSnafuExclude)
		return;
	if (IsSnafu())
	{
		if (m_nTrust > 0)
			m_nTrust--;
			m_nSnafuReason=reason;
		return;
	}
	if (bCheckTrust && m_nTrust > 0)
	{
		m_nTrust--;
		return;
	}

	//If it is on Upload-queue remove it
	if(GetUploadState() == US_UPLOADING || GetUploadState() == US_WAITCALLBACK || GetUploadState() == US_CONNECTING )
		theApp.uploadqueue->RemoveFromUploadQueue(this, _T("Snafu"), USR_EXCEPTION);

	m_nSnafuReason   = reason;
	m_nSnafuTimer	 = ::GetTickCount();
	theApp.clientlist->AddSnafuClient(this);
	theApp.emuledlg->transferwnd->queuelistctrl.RefreshClient(this);
	theApp.emuledlg->transferwnd->clientlistctrl.RefreshClient(this);
	if (bLog)
		DoSnafuInfo();
}

void CUpDownClient::DoSnafuInfo()
{
	CString text;
	
	text.Format(_T("S.N.A.F.U. %s [%s:%u]"), GetSnafuReasonStr(),ipstr(GetConnectIP()),GetUserPort());
	if (m_pszUsername!=NULL)
		text.AppendFormat(_T(" User: %s"),m_pszUsername);
	else if (HasValidHash())
		text.AppendFormat(_T(" Hash: %s"),md4str(GetUserHash()));	
	text.AppendFormat(_T(" Mod: %s"), DbgGetFullClientSoftVer()); // [TPT]
	text+=_T(" banned");
	if (thePrefs.GetVerbose()) AddPhoenixLogLine(false, text);
}

CString	CUpDownClient::GetSnafuReasonStr() const
{
	if (!thePrefs.GetAntiSnafu())
		return _T("");

	switch(m_nSnafuReason)
	{
	case SNAFU_WOMBAT:
		return _T("Bad Wombat");
	case SNAFU_FRIENDSHAREMOD:
		return _T("Friendshare Mod");
	case SNAFU_NAME:
	case SNAFU_ACTION:
	case SNAFU_OPCODE:
		return _T("suspect behaviour");
	case SNAFU_NICK:
		return _T("suspect nickname");
	case SNAFU_MODSTR:		
		return _T("suspect client version");
	case SNAFU_CLIENT:
		return _T("eDonkey client with eMule property");
	case SNAFU_HASHCHG:
		return _T("hash changer");
	case SNAFU_MYHASH:
		return _T("hash bastard");
	case SNAFU_ACT:
		return _T("hash thief (act)");
	case SNAFU_HASH:
		return _T("hash thief");
	case SNAFU_CLONE:
		return _T("clone (InfoPacket thief)");
	case SNAFU_TAG:
		if (m_strSnafuTag)
			return m_strSnafuTag;			
		return _T("suspect tag");
	default:
		return _T("shit happens");
	}
}
void CUpDownClient::SnafuCheckMod(LPCTSTR pMod)
{
	if (!pMod)
		return;

	//<<< eWombat [GOOFY_MODS] don't punish	
	if (stristrex(pMod, _T("hardmule"))!=NULL || stristrex(pMod, _T("eastshare"))!=NULL)
	{
		m_bGoofy=true;
		return;
	}
	//other goofy-mods but with leecher behavior, punish them
	if (stristrex(pMod, _T("mison")) !=NULL || stristrex(pMod, _T("evort")) !=NULL || stristrex(pMod, _T("booster")) !=NULL ||
		stristrex(pMod, _T("eifen")) !=NULL ||
		stristrex(pMod, _T("elfen")) !=NULL || stristrex(pMod, _T("imp delta")) !=NULL || stristrex(pMod, _T("father")) !=NULL)
		m_bGoofy=true;

	if (!thePrefs.GetAntiSnafu())
		return;
	if (IsSnafu())
		return;
//CString test=m_strMod;
//test.MakeLower();
if (m_bGoofy ||
		stristrex(pMod, _T("bomb"))!=NULL||
		stristrex(pMod, _T("fick"))!=NULL||
		stristrex(pMod, _T("dmx"))!=NULL||
		stristrex(pMod, _T("fincan"))!=NULL||
		stristrex(pMod, _T("dodgethis"))!=NULL|| //Updated
		stristrex(pMod, _T("freeza"))!=NULL||
		stristrex(pMod, _T("d-unit"))!=NULL||
		stristrex(pMod, _T("nos"))!=NULL||
		stristrex(pMod, _T("imperator"))!=NULL||
		stristrex(pMod, _T("speedload"))!=NULL||
		stristrex(pMod, _T("gt mod"))!=NULL||
		stristrex(pMod, _T("egomule"))!=NULL||
		stristrex(pMod, _T("darkmule"))!=NULL||
		stristrex(pMod, _T("legolas"))!=NULL||
		stristrex(pMod, _T("dm-"))!=NULL|| //hotfix
		stristrex(pMod, _T("|x|"))!=NULL||
	stristrex(pMod, _T("esladevil"))!=NULL ||
	stristrex(pMod, _T("00de"))!=NULL ||
	stristrex(pMod, _T("heartbreaker"))!=NULL ||
	stristrex(pMod, _T("g@m3r"))!=NULL ||
	stristrex(pMod, _T("imparator"))!=NULL ||
		stristrex(pMod, _T("Rappi"))!=NULL ||
		stristrex(pMod, _T("Speed-Unit"))!=NULL ||
		stristrex(pMod, _T("eVortex"))!=NULL ||
		stristrex(pMod, _T("dragon"))!=NULL ||
		stristrex(pMod, _T("HARDMULE"))!=NULL ||
		stristrex(pMod, _T("WWW.HARDMULE.COM"))!=NULL)
	{	
		if (thePrefs.GetVerbose()) AddPhoenixLogLine(false, _T("Suspect Mod detected: %s"), pMod);
		DoSnafu(SNAFU_MODSTR,false,false);
	}
}

void CUpDownClient::SnafuCheckNick(LPCTSTR pNick)
{
	if (!thePrefs.GetAntiSnafu())
		return;
	if (pNick==NULL)
		return;
	if (IsSnafu())
		return;
	if(	
		(stristrex(pNick,_T("emule")) && stristrex(pNick,_T("booster"))) ||
		stristrex(pNick,_T("pharao"))		|| 
		stristrex(pNick,_T("reverse")) 		||
		stristrex(pNick,_T("rammstein"))	||
		stristrex(pNick,_T("leecha"))		||
		stristrex(pNick,_T("energyfaker"))	||
		stristrex(pNick,_T("edevil"))		||
		stristrex(pNick,_T("darkmule"))		||
		stristrex(pNick,_T("evortex"))		||
		stristrex(pNick,_T("|evorte|x|"))	||
		stristrex(pNick,_T("$gam3r$"))		||
		stristrex(pNick,_T("g@m3r"))		||
		stristrex(pNick,_T("dodgethis"))		||
		stristrex(pNick,_T("Gate-eMule"))		||
		stristrex(pNick,_T("00de.de"))		||
		stristrex(pNick,_T("$warez$"))		||
		stristrex(pNick,_T("celinesexy")))
	{	
		if (thePrefs.GetVerbose()) AddPhoenixLogLine(false, _T("Suspect nickname: %s"), pNick);
		DoSnafu(SNAFU_NICK,false,false);
	}
}
void CUpDownClient::SnafuCheckVersion()
{
	if (IsSnafu())
		return;
	if ((GetVersion()>589) && ((GetSourceExchangeVersion()>0) || (GetExtendedRequestsVersion()>0)) && (GetClientSoft()==SO_EDONKEY)) //Fake Donkeys
	{	
	DoSnafu(SNAFU_CLIENT,false,false);
	}
	if ( (GetClientSoft() == SO_EDONKEYHYBRID) &&  ( (GetSourceExchangeVersion()>0) || (GetExtendedRequestsVersion()>0))) //Fake Hybrids
	{
	DoSnafu(SNAFU_CLIENT,false,false);
	}
	if (m_bySupportSecIdent!=0 && (GetClientSoft()==SO_EDONKEYHYBRID || GetClientSoft()==SO_EDONKEY))
	{
	DoSnafu(SNAFU_CLIENT,false,false);
	}
}

ULONG CUpDownClient::SnafuNameCRC(CString tocalc)
{   
	return theApp.wombatlist->GetHashKey(tocalc);
}

//>>> eWombat [ACT]

// >>> eWombat [SNAFU]
#ifdef _DEBUGWOMBAT
void CUpDownClient::GetDebugStr(CString &string)
{
	string="...";//m_strCommunity;
}
#endif

void  CUpDownClient::UnBan()
{
	theApp.clientlist->AddTrackClient(this); // [TPT] - Patch	
	SetUploadState(US_NONE);

	ClearWaitStartTime();

	// [TPT] - Code Improvement
	for (POSITION pos = m_RequestedFiles_list.GetHeadPosition();pos != 0;)
	{
		Requested_File_Struct* cur_struct = m_RequestedFiles_list.GetNext(pos);
		cur_struct->badrequests = 0;
		cur_struct->lastasked = 0;	
	}
	m_nBanTimer = 0;
	m_bBan		= false;
	theApp.emuledlg->transferwnd->queuelistctrl.RefreshClient(this);
	theApp.emuledlg->transferwnd->clientlistctrl.RefreshClient(this);
}

bool CUpDownClient::IsBanned() const
{	
	return( m_bBan && (m_nDownloadState != DS_DOWNLOADING));
}

void CUpDownClient::Ban(LPCTSTR pszReason)
{
	if(SupportsWebCache()) 
		return; // [TPT] - WebCache // yonatan tmp - for testing - don't ban webcache clients
	theApp.clientlist->AddTrackClient(this); // [TPT] - Patch
	// [TPT] - SUQWT
	if(theApp.clientcredits->IsSaveUploadQueueWaitTime()) 
	{
		ClearWaitStartTime();
		if (credits != NULL)
			credits->ClearUploadQueueWaitTime();		
	}
	// [TPT] - SUQWT
	if ((!IsBanned()) && (thePrefs.GetVerbose()))
	{
		if (thePrefs.GetLogBannedClients())
			AddDebugLogLine(false,_T("Banned: %s; %s"), pszReason==NULL ? _T("Aggressive behaviour") : pszReason, DbgGetClientInfo());			
	}
#ifdef _DEBUG
	else{
		if (thePrefs.GetLogBannedClients())
			AddDebugLogLine(false,_T("Banned: (refreshed): %s; %s"), pszReason==NULL ? _T("Aggressive behaviour") : pszReason, DbgGetClientInfo());
	}
#endif
	m_nBanTimer = ::GetTickCount();
	m_bBan		= true;
	theApp.clientlist->AddSnafuClient(this);	
	SetUploadState(US_BANNED); // [TPT] - Patch
	theApp.emuledlg->transferwnd->queuelistctrl.RefreshClient(this);
	theApp.emuledlg->transferwnd->clientlistctrl.RefreshClient(this);
	if (this->socket != NULL && socket->IsConnected())
		this->socket->ShutDown(SD_RECEIVE); // let the socket timeout, since we dont want to risk to delete the client right now. This isnt acutally perfect, could be changed later
}

//<<< eWombat [SNAFU_V3]
void CUpDownClient::ProcessUnknownHelloTag(CTag *tag)
{
	if (!thePrefs.GetAntiSnafu() || IsSnafu())
		return;

	CString buffer;
	switch(tag->GetNameID())
	{
	case CT_UNKNOWNx12:
	case CT_UNKNOWNx13:
	case CT_UNKNOWNx14:
	case CT_UNKNOWNx16:
	case CT_UNKNOWNx17:
	case CT_UNKNOWNxE6:			buffer=_T("DodgeBoards");break;
						
	case CT_UNKNOWNx15:			buffer=_T("DodgeBoards & DarkMule |eVorte|X|");break;
	case CT_UNKNOWNx22:			buffer=_T("DarkMule v6 |eVorte|X|");break;
	case CT_UNKNOWNx5D:
	case CT_UNKNOWNx6B:
	case CT_UNKNOWNx6C:			buffer=_T("md4");break;

	case CT_UNKNOWNx69:			buffer=_T("DarkMule v6 |eVorte|X|");break;
	case CT_UNKNOWNx79:			buffer=_T("Bionic");break;
	case CT_UNKNOWNx83:			buffer=_T("Fusspi");break;
	case CT_UNKNOWNx76:			
	case CT_UNKNOWNxCD:			buffer=_T("www.donkey2002.to");break;
	case CT_UNKNOWNx88:
		//If its a LSD its o.k
		if (m_pMod==NULL || !stristrex(m_pMod->Left(3), _T("lsd")))
			buffer=_T("DarkMule v6 |eVorte|X|");
		break;
	case CT_UNKNOWNx8c:			buffer=_T("[LSD7c]");break; 
	case CT_UNKNOWNx8d:			buffer=_T("[0x8d] unknown Leecher - (client version:60)");break;
	case CT_UNKNOWNx99:			buffer=_T("[RAMMSTEIN]");break;		//STRIKE BACK
	case CT_UNKNOWNx98:
	case CT_UNKNOWNx9C:			buffer=_T("Emulereactor Community Mod");break;

	case CT_UNKNOWNxc4:			buffer=_T("[MD5 Community]");break;	//USED BY NEW BIONIC => 0x12 Sender
	case CT_FRIENDSHARING:		//STRIKE BACK
		if (thePrefs.GetAntiFriendshare())
		{
			if (tag->IsInt() && tag->GetInt() == FRIENDSHARING_ID) //Mit dieser ID Definitiv
			{
				DoSnafu(SNAFU_FRIENDSHAREMOD,false,false);
				return;				
			}			
		}
		break;
	case CT_DARK:				//STRIKE BACK				
	case CT_UNKNOWNx7A:
	case CT_UNKNOWNxCA:
		buffer=_T("new DarkMule");
		break;
	}
	
	if (!buffer.IsEmpty())
	{
		AddPhoenixLogLine(false, _T("Suspect Hello-Tag: %s (%s)"), tag->GetFullInfo(), buffer);
		DoSnafu(SNAFU_TAG,false,false);		
		if (m_strSnafuTag)
			free(m_strSnafuTag);
		m_strSnafuTag = NULL;
		m_strSnafuTag = _tcsdup((LPCTSTR)buffer);
	}
}

void CUpDownClient::ProcessUnknownInfoTag(CTag *tag)
{
	if (!thePrefs.GetAntiSnafu() || IsSnafu())
		return;
	CString buffer;
	switch(tag->GetNameID())
	{
	case ET_MOD_UNKNOWNx12:
	case ET_MOD_UNKNOWNx13:
	case ET_MOD_UNKNOWNx14:
	case ET_MOD_UNKNOWNx17:		buffer=_T("[DodgeBoards]");break;
	case ET_MOD_UNKNOWNx2F:		buffer=_T("[OMEGA v.07 Heiko]");break;
	case ET_MOD_UNKNOWNx36:
	case ET_MOD_UNKNOWNx5B:
	case ET_MOD_UNKNOWNxA6:		buffer=_T("eMule v0.26 Leecher");break;
	case ET_MOD_UNKNOWNx60:		buffer=_T("[Hunter]");break; //STRIKE BACK
	case ET_MOD_UNKNOWNx76:		buffer=_T("[DodgeBoards]");break;
	case ET_MOD_UNKNOWNx50:		
	case ET_MOD_UNKNOWNxB1:		
	case ET_MOD_UNKNOWNxB4:		
	case ET_MOD_UNKNOWNxC8:		
	case ET_MOD_UNKNOWNxC9:		buffer=_T("[Bionic 0.20 Beta]");break;
	case ET_MOD_UNKNOWNxDA:		buffer=_T("[Rumata (rus)(Plus v1f)]");break;
	}
	if (!buffer.IsEmpty())
	{
		AddPhoenixLogLine(false, _T("Suspect eMuleInfo-Tag: %s (%s)"), tag->GetFullInfo(), buffer);
		DoSnafu(SNAFU_TAG,false,false);				
		if (m_strSnafuTag)
			free(m_strSnafuTag);
		m_strSnafuTag = NULL;
		m_strSnafuTag = _tcsdup((LPCTSTR)buffer);		
	}
}
//>>> eWombat [SNAFU_V3]
