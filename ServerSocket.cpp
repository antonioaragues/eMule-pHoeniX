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
#include "stdafx.h"
#include "emule.h"
#include "ServerSocket.h"
#include "SearchList.h"
#include "DownloadQueue.h"
#include "Statistics.h"
#include "ClientList.h"
#include "Server.h"
#include "ServerList.h"
#include "Sockets.h"
#include "OtherFunctions.h"
#include "Opcodes.h"
#include "Preferences.h"
#include "SafeFile.h"
#include "PartFile.h"
#include "Packets.h"
#include "UpDownClient.h"
#include "emuleDlg.h"
#include "ServerWnd.h"
#include "SearchDlg.h"
#include "IPFilter.h"
#include "Log.h"
#include "BandWidthControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif


#pragma pack(1)
struct LoginAnswer_Struct {
	uint32	clientid;
};
#pragma pack()


CServerSocket::CServerSocket(CServerConnect* in_serverconnect){
	serverconnect = in_serverconnect;
	connectionstate = 0;
	cur_server = 0;
	m_bIsDeleting = false;
	m_dwLastTransmission = 0;
}

CServerSocket::~CServerSocket(){
	if (cur_server)
		delete cur_server;
	cur_server = NULL;
}

void CServerSocket::OnConnect(int nErrorCode){
	// [TPT]
	// MAella -QOS-
	CEMSocket::OnConnect(nErrorCode); // deadlake PROXYSUPPORT - changed to AsyncSocketEx
	// Maella end
	switch (nErrorCode){
		case 0:{
			if (cur_server->HasDynIP()){
				SOCKADDR_IN sockAddr = {0};
				int nSockAddrLen = sizeof(sockAddr);
				GetPeerName((SOCKADDR*)&sockAddr, &nSockAddrLen);
				cur_server->SetIP(sockAddr.sin_addr.S_un.S_addr);
				CServer* pServer = theApp.serverlist->GetServerByAddress(cur_server->GetAddress(),cur_server->GetPort());
				if (pServer)
					pServer->SetIP(sockAddr.sin_addr.S_un.S_addr);
			}
			SetConnectionState(CS_WAITFORLOGIN);
			break;
		}
		case WSAEADDRNOTAVAIL:
		case WSAECONNREFUSED:
		//case WSAENETUNREACH:	// let this error default to 'fatal error' as it does not inrease the server's failed count
		case WSAETIMEDOUT: 	
		case WSAEADDRINUSE:
			if (thePrefs.GetVerbose())
				DebugLogError(_T("Failed to connect to server %s; %s"), cur_server->GetAddress(), GetErrorMessage(nErrorCode, 1));
			m_bIsDeleting = true;
			SetConnectionState(CS_SERVERDEAD);
			serverconnect->DestroySocket(this);
			return;
		// deadlake PROXYSUPPORT
		case WSAECONNABORTED:
			if (m_ProxyConnectFailed)
			{
				if (thePrefs.GetVerbose())
					DebugLogError(_T("Failed to connect to server %s; %s"), cur_server->GetAddress(), GetErrorMessage(nErrorCode, 1));
				m_ProxyConnectFailed = false;
				m_bIsDeleting = true;
				SetConnectionState(CS_SERVERDEAD);
				serverconnect->DestroySocket(this);
				return;
			}
		default:	
			if (thePrefs.GetVerbose())
				DebugLogError(_T("Failed to connect to server %s; %s"), cur_server->GetAddress(), GetErrorMessage(nErrorCode, 1));
			m_bIsDeleting = true;
			SetConnectionState(CS_FATALERROR);
			serverconnect->DestroySocket(this);
			return;
	}	 
}

void CServerSocket::OnReceive(int nErrorCode){
	if (connectionstate != CS_CONNECTED && !this->serverconnect->IsConnecting()){
		serverconnect->DestroySocket(this);
		return;
	}
	CEMSocket::OnReceive(nErrorCode);
	m_dwLastTransmission = GetTickCount();
}

bool CServerSocket::ProcessPacket(char* packet, uint32 size, uint8 opcode){
	try{
		switch(opcode){
			case OP_SERVERMESSAGE:{
				if (thePrefs.GetDebugServerTCPLevel() > 0)
					Debug(_T("ServerMsg - OP_ServerMessage\n"));

				CServer* pServer = cur_server ? theApp.serverlist->GetServerByAddress(cur_server->GetAddress(),cur_server->GetPort()) : NULL;
				CSafeMemFile data((const BYTE*)packet, size);
				CString strMessages(data.ReadString(pServer ? pServer->GetUnicodeSupport() : false));

				if (thePrefs.GetDebugServerTCPLevel() > 0){
					UINT uAddData = data.GetLength() - data.GetPosition();
					if (uAddData > 0){
						Debug(_T("*** NOTE: OP_ServerMessage: ***AddData: %u bytes\n"), uAddData);
						DebugHexDump((uint8*)packet + data.GetPosition(), uAddData);
					}
				}

				// 16.40 servers do not send separate OP_SERVERMESSAGE packets for each line;
				// instead of this they are sending all text lines with one OP_SERVERMESSAGE packet.
				int iPos = 0;
				CString message = strMessages.Tokenize(_T("\r\n"), iPos);
				while (!message.IsEmpty())
				{
					bool bOutputMessage = true;
					if (_tcsnicmp(message, _T("server version"), 14) == 0){
						CString strVer = message.Mid(14);
						strVer.Trim();
						strVer = strVer.Left(64); // truncate string to avoid misuse by servers in showing ads
						if (pServer){
							pServer->SetVersion(strVer);
							theApp.emuledlg->serverwnd->serverlistctrl.RefreshServer(pServer);
							theApp.emuledlg->serverwnd->UpdateMyInfo();
						}
						if (thePrefs.GetDebugServerTCPLevel() > 0)
							Debug(_T("%s\n"), message);
					}
					else if (_tcsncmp(message, _T("ERROR"), 5) == 0){
						LogError(LOG_STATUSBAR, _T("%s %s (%s:%u) - %s"), 
							GetResString(IDS_ERROR),
							pServer ? pServer->GetListName() : GetResString(IDS_PW_SERVER), 
							cur_server ? cur_server->GetAddress() : _T(""), 
							cur_server ? cur_server->GetPort() : 0, message.Mid(5).Trim(_T(" :")));
						bOutputMessage = false;
					}
					else if (_tcsncmp(message, _T("WARNING"), 7) == 0){
						LogWarning(LOG_STATUSBAR, _T("%s %s (%s:%u) - %s"), 
							GetResString(IDS_WARNING),
							pServer ? pServer->GetListName() : GetResString(IDS_PW_SERVER), 
							cur_server ? cur_server->GetAddress() : _T(""),
							cur_server ? cur_server->GetPort() : 0, message.Mid(7).Trim(_T(" :")));
						bOutputMessage = false;
					}

					if (message.Find(_T("[emDynIP: ")) != -1 && message.Find(_T("]")) != -1 && message.Find(_T("[emDynIP: ")) < message.Find(_T("]"))){
						CString dynip = message.Mid(message.Find(_T("[emDynIP: ")) + 10, message.Find(_T("]")) - (message.Find(_T("[emDynIP: ")) + 10));
						dynip.Trim(_T(" "));
						if (dynip.GetLength() && dynip.GetLength() < 51){
							if (pServer){
								pServer->SetDynIP(dynip);
								if (cur_server)
									cur_server->SetDynIP(dynip);
								theApp.emuledlg->serverwnd->serverlistctrl.RefreshServer(pServer);
								theApp.emuledlg->serverwnd->UpdateMyInfo();
							}
						}
					}
					if (bOutputMessage)
						theApp.emuledlg->AddServerMessageLine(message);

					message = strMessages.Tokenize(_T("\r\n"), iPos);
				}
				break;
			}
			case OP_IDCHANGE:{
				if (thePrefs.GetDebugServerTCPLevel() > 0)
					Debug(_T("ServerMsg - OP_IDChange\n"));
				if (size < sizeof(LoginAnswer_Struct)){
					throw GetResString(IDS_ERR_BADSERVERREPLY);
				}
				LoginAnswer_Struct* la = (LoginAnswer_Struct*)packet;

				// save TCP flags in 'cur_server'
				ASSERT( cur_server );
				// [TPT] - Aux Ports
				if (cur_server){
				uint32 ConnPort = 0 ;
				uint32 rport = cur_server->GetConnPort() ;
				// [TPT] - Aux Ports
					if (size >= sizeof(LoginAnswer_Struct)+4){					
						DWORD dwFlags = *((uint32*)(packet + sizeof(LoginAnswer_Struct)));
						if (thePrefs.GetDebugServerTCPLevel() > 0){
							CString strInfo;
							strInfo.AppendFormat(_T("  TCP Flags=0x%08x"), dwFlags);
							const DWORD dwKnownBits = SRV_TCPFLG_COMPRESSION | SRV_TCPFLG_NEWTAGS | SRV_TCPFLG_UNICODE;
							if (dwFlags & ~dwKnownBits)
								strInfo.AppendFormat(_T("  ***UnkBits=0x%08x"), dwFlags & ~dwKnownBits);
							if (dwFlags & SRV_TCPFLG_COMPRESSION)
								strInfo.AppendFormat(_T("  Compression=1"));
							if (dwFlags & SRV_TCPFLG_NEWTAGS)
								strInfo.AppendFormat(_T("  NewTags=1"));
							if (dwFlags & SRV_TCPFLG_UNICODE)
								strInfo.AppendFormat(_T("  Unicode=1"));
							Debug(_T("%s\n"), strInfo);
						}
						cur_server->SetTCPFlags(dwFlags);
						// [TPT] - Aux Ports
						if (size >= sizeof(LoginAnswer_Struct)+8) 
						{
							// aux port login : we should use the 'standard' port of this server to advertize to other clients
							ConnPort = *((uint32*)(packet + sizeof(LoginAnswer_Struct) + 4)) ;
							cur_server->SetPort(ConnPort) ;
						}				
						// [TPT] - Aux Ports
					}
					else
						cur_server->SetTCPFlags(0);

					// copy TCP flags into the server in the server list
					// [TPT] - Aux Ports
					CServer* pServer = theApp.serverlist->GetServerByAddress(cur_server->GetAddress(), rport);
					if (pServer)
					{
						pServer->SetTCPFlags(cur_server->GetTCPFlags());
						if (ConnPort) pServer->SetPort(ConnPort);
						theApp.emuledlg->serverwnd->serverlistctrl.RefreshServer(pServer);
						theApp.emuledlg->serverwnd->UpdateMyInfo();
				}
					}
					// [TPT] - Aux Ports
				// [TPT] - Maella -Activate Smart Low ID check-
				if(la->clientid == 0)
				{
					// Reset attempts counter
							thePrefs.SetSmartIdState(0);
					}
				else if(la->clientid >= 16777216)
				{
					// High ID => reset attempts counter
					thePrefs.SetSmartIdState(0);
				}
				else if( thePrefs.GetSmartIdCheck() )
				{
					if (!IsLowID(la->clientid))
						thePrefs.SetSmartIdState(1);
							else
					{
						// Low ID => Check and increment attempts counter
						uint8 attempts = thePrefs.GetSmartIdState();
						if(attempts < 3)
						{																				
							thePrefs.SetSmartIdState(++attempts);
							AddLogLine(true, _T("LowID -- Trying Again (attempts %i)"), attempts);
							break; // Retries
						}
					}
				}
				
				// we need to know our client's HighID when sending our shared files (done indirectly on SetConnectionState)
				serverconnect->clientid = la->clientid;

				if (connectionstate != CS_CONNECTED) {
					SetConnectionState(CS_CONNECTED);
					theApp.OnlineSig();       // Added By Bouc7 
				}
				serverconnect->SetClientID(la->clientid);
				AddLogLine(false, GetResString(IDS_NEWCLIENTID), la->clientid);

				//Check for fake High ID
				if((IsLowID(la->clientid) == FALSE) && (thePrefs.m_bHighIdPossible == false))
					AddLogLine(false, _T("Server sent High ID on a Low ID connection"));


				if(!serverconnect->IsLowID() && thePrefs.GetDinamicNafc())
				{
					theApp.pBandWidthControl->CheckNafcOnIDChange(la->clientid);
				}

				// [TPT]
				// Maella -Inform sources of an ID change-
				// Maella -Reask sources after IP change-
				static uint32 s_lastValidId;
				static uint32 s_lastChangeId;
				if (thePrefs.GetReaskSourceAfterIPChange()) // [TPT] - Patch
				{
					if(s_lastValidId != 0 && serverconnect->GetClientID() != 0 && s_lastValidId != serverconnect->GetClientID()){
						// Public IP has been changed, it's necessary to inform all sources about it
						// All sources would be informed during their next session refresh (with TCP)
						// about the new IP.
						if(s_lastChangeId == 0 || GetTickCount() - s_lastChangeId > FILEREASKTIME + 60000){
							theApp.clientlist->TrigReaskForDownload(true);

							theApp.downloadqueue->ResetQuickStart();  // [TPT] - quick start

							AddLogLine(false, _T("Change from %u (%s ID) to %u (%s ID) detected%s"), 
								s_lastValidId,
								IsLowID(s_lastValidId) ? _T("low") : _T("high"),
								serverconnect->GetClientID(),
								IsLowID(serverconnect->GetClientID())  ? _T("low") : _T("high"),
								(IsLowID(s_lastValidId) && IsLowID(serverconnect->GetClientID())) ? 
								_T("") : _T(", all sources will be reasked immediately"));
						}
						else {
							theApp.clientlist->TrigReaskForDownload(false);

							AddLogLine(false, _T("Change from %u (%s ID) to %u (%s ID) detected%s"), 
								s_lastValidId,
								IsLowID(s_lastValidId) ? _T("low") : _T("high"),
								serverconnect->GetClientID(),
								IsLowID(serverconnect->GetClientID())  ? _T("low") : _T("high"),
								(IsLowID(s_lastValidId) && IsLowID(serverconnect->GetClientID())) ? 
								_T("") : _T(", all sources will be reasked within the next 10 minutes"));
						}
					}
				}
				if(serverconnect->GetClientID() != 0 && serverconnect->GetClientID() != s_lastValidId){
					// Keep track of a change of the global IP
					s_lastValidId = serverconnect->GetClientID();
					s_lastChangeId = GetTickCount();
				}
				// Maella end

				theApp.downloadqueue->ResetLocalServerRequests();
				break;
			}
			case OP_SEARCHRESULT:{
				if (thePrefs.GetDebugServerTCPLevel() > 0)
					Debug(_T("ServerMsg - OP_SearchResult\n"));
				CServer* cur_srv = (serverconnect) ? serverconnect->GetCurrentServer() : NULL;
				CServer* pServer = cur_srv ? theApp.serverlist->GetServerByAddress(cur_srv->GetAddress(), cur_srv->GetPort()) : NULL;
				(void)pServer;
				bool bMoreResultsAvailable;
				uint16 uSearchResults = theApp.searchlist->ProcessSearchanswer(packet, size, true/*pServer ? pServer->GetUnicodeSupport() : false*/, cur_srv ? cur_srv->GetIP() : 0, cur_srv ? cur_srv->GetPort() : 0, &bMoreResultsAvailable);
				theApp.emuledlg->searchwnd->LocalSearchEnd(uSearchResults, bMoreResultsAvailable);
				break;
			}
			case OP_FOUNDSOURCES:{
				if (thePrefs.GetDebugServerTCPLevel() > 0)
					Debug(_T("ServerMsg - OP_FoundSources; Sources=%u  %s\n"), (UINT)(uchar)packet[16], DbgGetFileInfo((uchar*)packet));
				ASSERT( cur_server );
				if (cur_server)
				{
				CSafeMemFile sources((BYTE*)packet,size);
				uchar fileid[16];
				sources.ReadHash16(fileid);
				if (CPartFile* file = theApp.downloadqueue->GetFileByID(fileid))
					file->AddSources(&sources,cur_server->GetIP(), cur_server->GetPort());
				}
				break;
			}
			case OP_SERVERSTATUS:{
				if (thePrefs.GetDebugServerTCPLevel() > 0)
					Debug(_T("ServerMsg - OP_ServerStatus\n"));
				// FIXME some statuspackets have a different size -> why? structur?
				if (size < 8)
					break;//throw "Invalid status packet";
				uint32 cur_user = PeekUInt32(packet);
				uint32 cur_files = PeekUInt32(packet+4);
				CServer* update = cur_server ? theApp.serverlist->GetServerByAddress(cur_server->GetAddress(), cur_server->GetPort()) : NULL;
				if (update){
					update->SetUserCount(cur_user); 
					update->SetFileCount(cur_files);
					theApp.emuledlg->ShowUserCount();
					theApp.emuledlg->serverwnd->serverlistctrl.RefreshServer( update );
					theApp.emuledlg->serverwnd->UpdateMyInfo();
				}
				if (thePrefs.GetDebugServerTCPLevel() > 0){
					if (size > 8){
						Debug(_T("*** NOTE: OP_ServerStatus: ***AddData: %u bytes\n"), size - 8);
						DebugHexDump((uint8*)packet + 8, size - 8);
					}
				}
				break;
			}
			case OP_SERVERIDENT:{
				// OP_SERVERIDENT - this is sent by the server only if we send a OP_GETSERVERLIST
				if (thePrefs.GetDebugServerTCPLevel() > 0)
					Debug(_T("ServerMsg - OP_ServerIdent\n"));
				if (size<16+4+2+4){
					if (thePrefs.GetVerbose())
						DebugLogError(_T("%s"), GetResString(IDS_ERR_KNOWNSERVERINFOREC));
					break;// throw "Invalid server info received"; 
				} 

				CServer* pServer = cur_server ? theApp.serverlist->GetServerByAddress(cur_server->GetAddress(),cur_server->GetPort()) : NULL;
				CString strInfo;
				CSafeMemFile data((BYTE*)packet, size);
				
				uint8 aucHash[16];
				data.ReadHash16(aucHash);
				if (thePrefs.GetDebugServerTCPLevel() > 0)
					strInfo.AppendFormat(_T("Hash=%s (%s)"), md4str(aucHash), DbgGetHashTypeString(aucHash));
				uint32 nServerIP = data.ReadUInt32();
				uint16 nServerPort = data.ReadUInt16();
				if (thePrefs.GetDebugServerTCPLevel() > 0)
					strInfo.AppendFormat(_T("  IP=%s:%u"), ipstr(nServerIP), nServerPort);
				UINT nTags = data.ReadUInt32();
				if (thePrefs.GetDebugServerTCPLevel() > 0)
					strInfo.AppendFormat(_T("  Tags=%u"), nTags);

				CString strName;
				CString strDescription;
				for (UINT i = 0; i < nTags; i++){
					CTag tag(&data, pServer ? pServer->GetUnicodeSupport() : false);
					if (tag.GetNameID() == ST_SERVERNAME){
						if (tag.IsStr()){
							strName = tag.GetStr();
							if (thePrefs.GetDebugServerTCPLevel() > 0)
								strInfo.AppendFormat(_T("  Name=%s"), strName);
						}
					}
					else if (tag.GetNameID() == ST_DESCRIPTION){
						if (tag.IsStr()){
							strDescription = tag.GetStr();
							if (thePrefs.GetDebugServerTCPLevel() > 0)
								strInfo.AppendFormat(_T("  Desc=%s"), strDescription);
						}
					}
					else if (thePrefs.GetDebugServerTCPLevel() > 0)
						strInfo.AppendFormat(_T("  ***UnkTag: 0x%02x=%u"), tag.GetNameID(), tag.GetInt());
				}
				if (thePrefs.GetDebugServerTCPLevel() > 0){
					strInfo += _T('\n');
					Debug(_T("%s"), strInfo);

					UINT uAddData = data.GetLength() - data.GetPosition();
					if (uAddData > 0){
						Debug(_T("*** NOTE: OP_ServerIdent: ***AddData: %u bytes\n"), uAddData);
						DebugHexDump((uint8*)packet + data.GetPosition(), uAddData);
					}
				}

				if (pServer){
					pServer->SetListName(strName);
					pServer->SetDescription(strDescription);
					if (((uint32*)aucHash)[0] == 0x2A2A2A2A){
						const CString& rstrVersion = pServer->GetVersion();
						if (!rstrVersion.IsEmpty())
							pServer->SetVersion(_T("eFarm ") + rstrVersion);
						else
							pServer->SetVersion(_T("eFarm"));
					}
					theApp.emuledlg->ShowConnectionState(); 
					theApp.emuledlg->serverwnd->serverlistctrl.RefreshServer(pServer); 
				}
				break;
			} 
			// tecxx 1609 2002 - add server's serverlist to own serverlist
			case OP_SERVERLIST:{
				if (thePrefs.GetDebugServerTCPLevel() > 0)
					Debug(_T("ServerMsg - OP_ServerList\n"));
				try{
					CSafeMemFile servers((BYTE*)packet,size);
					UINT count = servers.ReadUInt8();
					// check if packet is valid
					if (1 + count*(4+2) > size)
						count = 0;
					int addcount = 0;
					while(count)
					{
						uint32 ip = servers.ReadUInt32();
						uint16 port = servers.ReadUInt16();
						CServer* srv = new CServer(port, ipstr(ip));
						srv->SetListName(srv->GetFullIP());
						if (!theApp.emuledlg->serverwnd->serverlistctrl.AddServer(srv, true))
							delete srv;
						else
							addcount++;
						count--;
					}
					if (addcount)
						AddLogLine(false, GetResString(IDS_NEWSERVERS), addcount);
					if (thePrefs.GetDebugServerTCPLevel() > 0){
						UINT uAddData = servers.GetLength() - servers.GetPosition();
						if (uAddData > 0){
							Debug(_T("*** NOTE: OP_ServerList: ***AddData: %u bytes\n"), uAddData);
							DebugHexDump((uint8*)packet + servers.GetPosition(), uAddData);
						}
					}
				}
				catch(CFileException* error){
					if (thePrefs.GetVerbose())
						DebugLogError(_T("%s"), GetResString(IDS_ERR_BADSERVERLISTRECEIVED));
					error->Delete();
				}
				break;
			}
			case OP_CALLBACKREQUESTED:{
				if (thePrefs.GetDebugServerTCPLevel() > 0)
					Debug(_T("ServerMsg - OP_CallbackRequested\n"));
				if (size == 6)
				{
					uint32 dwIP = PeekUInt32(packet);

					if (theApp.ipfilter->IsFiltered(dwIP)){
						theStats.filteredclients++;
						if (thePrefs.GetLogFilteredIPs())
							AddDebugLogLine(false, _T("Ignored callback request (IP=%s) - IP filter (%s)"), ipstr(dwIP), theApp.ipfilter->GetLastHit());
						break;
					}

					if (theApp.clientlist->IsSnafuClient(dwIP)){ // [TPT] - eWombat
						if (thePrefs.GetLogBannedClients()){
							CUpDownClient* pClient = theApp.clientlist->FindClientByIP(dwIP);
							AddDebugLogLine(false, _T("Ignored callback request from banned client %s; %s"), ipstr(dwIP), pClient->DbgGetClientInfo());
						}
						break;
					}

					uint16 nPort = PeekUInt16(packet+4);
					CUpDownClient* client = theApp.clientlist->FindClientByIP(dwIP,nPort);
					if (client)
						client->TryToConnect();
					else
					{
						client = new CUpDownClient(0,nPort,dwIP,0,0,true);
						theApp.clientlist->AddClient(client);
						client->TryToConnect();
					}
				}
				break;
			}
			case OP_CALLBACK_FAIL:{
				if (thePrefs.GetDebugServerTCPLevel() > 0)
					Debug(_T("ServerMsg - OP_Callback_Fail %s\n"), DbgGetHexDump((uint8*)packet, size));
				break;
			}
			case OP_REJECT:{
				if (thePrefs.GetDebugServerTCPLevel() > 0)
					Debug(_T("ServerMsg - OP_Reject %s\n"), DbgGetHexDump((uint8*)packet, size));
				// this could happen if we send a command with the wrong protocol (e.g. sending a compressed packet to
				// a server which does not support that protocol).
				if (thePrefs.GetVerbose())
					DebugLogError(_T("Server rejected last command"));
				break;
			}
			default:
				if (thePrefs.GetDebugServerTCPLevel() > 0)
					Debug(_T("***NOTE: ServerMsg - Unknown message; opcode=0x%02x  %s\n"), opcode, DbgGetHexDump((uint8*)packet, size));
				;
		}

		return true;
	}
	catch(CFileException* error)
	{
		if (thePrefs.GetVerbose())
		{
			TCHAR szError[MAX_CFEXP_ERRORMSG];
			error->m_strFileName = _T("server packet");
			error->GetErrorMessage(szError, ARRSIZE(szError));
		    DebugLogError(GetResString(IDS_ERR_PACKAGEHANDLING), szError);
		}
		error->Delete();
		ASSERT(0);
		if (opcode==OP_SEARCHRESULT || opcode==OP_FOUNDSOURCES)
			return true;
	}
	catch(CMemoryException* error)
	{
		if (thePrefs.GetVerbose())
			DebugLogError(GetResString(IDS_ERR_PACKAGEHANDLING), _T("CMemoryException"));
		error->Delete();
		ASSERT(0);
		if (opcode==OP_SEARCHRESULT || opcode==OP_FOUNDSOURCES)
			return true;
	}
	catch(CString error)
	{
		if (thePrefs.GetVerbose())
			DebugLogError(GetResString(IDS_ERR_PACKAGEHANDLING), error);
		ASSERT(0);
	}
	catch(...)
	{
		if (thePrefs.GetVerbose())
			DebugLogError(GetResString(IDS_ERR_PACKAGEHANDLING), _T("Unknown exception"));
		ASSERT(0);
	}

	SetConnectionState(CS_DISCONNECTED);
	return false;
}

void CServerSocket::ConnectToServer(CServer* server){
	if (cur_server){
		ASSERT(0);
		delete cur_server;
		cur_server = NULL;
	}

	cur_server = new CServer(server);
	Log(GetResString(IDS_CONNECTINGTO), cur_server->GetListName(), cur_server->GetFullIP(), cur_server->GetPort());

	if (thePrefs.IsProxyASCWOP() )
	{
		if (thePrefs.GetProxy().UseProxy == true)
		{
			thePrefs.SetProxyASCWOP(true);
			thePrefs.SetUseProxy(false);
			AddLogLine(false, GetResString(IDS_ASCWOP_PROXYSUPPORT) + GetResString(IDS_DISABLED));
		}
		else
			thePrefs.SetProxyASCWOP(false);
	}

	SetConnectionState(CS_CONNECTING);
	if (!Connect(server->GetAddress(),server->GetConnPort())){//[TPT] - Aux Ports
		DWORD dwError = GetLastError();
		if (dwError != WSAEWOULDBLOCK){
			LogError(GetResString(IDS_ERR_CONNECTIONERROR), cur_server->GetListName(), cur_server->GetFullIP(), cur_server->GetPort(), GetErrorMessage(dwError, 1));
			SetConnectionState(CS_FATALERROR);
			return;
		}
	}
	SetConnectionState(CS_CONNECTING);
}

void CServerSocket::OnError(int nErrorCode)
{
	SetConnectionState(CS_DISCONNECTED);
	if (thePrefs.GetVerbose())
		DebugLogError(GetResString(IDS_ERR_SOCKET), cur_server->GetListName(), cur_server->GetFullIP(), cur_server->GetPort(), GetErrorMessage(nErrorCode, 1));
}

bool CServerSocket::PacketReceived(Packet* packet)
{
	try
	{
		theStats.AddDownDataOverheadServer(packet->size);
		if (packet->prot == OP_PACKEDPROT)
		{
			uint32 uComprSize = packet->size;
			if (!packet->UnPackPacket(250000)){
				if (thePrefs.GetVerbose())
					DebugLogError(_T("Failed to decompress server TCP packet: protocol=0x%02x  opcode=0x%02x  size=%u"), packet ? packet->prot : 0, packet ? packet->opcode : 0, packet ? packet->size : 0);
				return true;
			}
			packet->prot = OP_EDONKEYPROT;
			if (thePrefs.GetDebugServerTCPLevel() > 1)
				Debug(_T("Received compressed server TCP packet; opcode=0x%02x  size=%u  uncompr size=%u\n"), packet->opcode, uComprSize, packet->size);
		}

		if (packet->prot == OP_EDONKEYPROT)
		{
			ProcessPacket(packet->pBuffer,packet->size,packet->opcode);
		}
		else
		{
			if (thePrefs.GetVerbose())
				DebugLogWarning(_T("Received server TCP packet with unknown protocol: protocol=0x%02x  opcode=0x%02x  size=%u"), packet ? packet->prot : 0, packet ? packet->opcode : 0, packet ? packet->size : 0);
		}
	}
	catch(...)
	{
		if (thePrefs.GetVerbose())
			DebugLogError(_T("Error: Unhandled exception while processing server TCP packet: protocol=0x%02x  opcode=0x%02x  size=%u"), packet ? packet->prot : 0, packet ? packet->opcode : 0, packet ? packet->size : 0);
		ASSERT(0);
		return false;
	}
	return true;
}

void CServerSocket::OnClose(int nErrorCode){
	CEMSocket::OnClose(0);
	if (connectionstate == CS_WAITFORLOGIN){	 	
		SetConnectionState(CS_SERVERFULL);
	}
	else if (connectionstate == CS_CONNECTED){
		SetConnectionState(CS_DISCONNECTED);		
	}
	else{
		SetConnectionState(CS_NOTCONNECTED);
	}
	serverconnect->DestroySocket(this);	
}

void CServerSocket::SetConnectionState(sint8 newstate){
	connectionstate = newstate;
	if (newstate < 1){
		serverconnect->ConnectionFailed(this);
	}
	else if (newstate == CS_CONNECTED || newstate == CS_WAITFORLOGIN){
		if (serverconnect)
			serverconnect->ConnectionEstablished(this);
	}
}

void CServerSocket::SendPacket(Packet* packet, bool delpacket, bool controlpacket, uint32 actualPayloadSize){
	m_dwLastTransmission = GetTickCount();
	CEMSocket::SendPacket(packet, delpacket, controlpacket, actualPayloadSize);
}
