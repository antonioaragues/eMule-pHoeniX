#include "StdAfx.h"
#include "ircnicklistctrl.h"
#include "ircwnd.h"
#include "ircmain.h"
#include "emuledlg.h"
#include "OtherFunctions.h"
#include "MenuCmds.h"
#include "emule.h"
#include "HTRichEditCtrl.h"
#include "MenuXP.h"// [TPT] - New Menu Styles
#include "mod_version.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif


struct Nick
{
	CString nick;
	CString modes;
	int level;
};

struct Channel
{
	CString	name;
	CString modesA;
	CString modesB;
	CString modesC;
	CString modesD;
	CHTRichEditCtrl log;
	CString title;
	CPtrList nicks;
	uint8 type;
	CStringArray history;
	uint16 history_pos;
	// Type is mainly so that we can use this for IRC and the eMule Messages..
	// 1-Status, 2-Channel list, 4-Channel, 5-Private Channel, 6-eMule Message(Add later)
};

IMPLEMENT_DYNAMIC(CIrcNickListCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CIrcNickListCtrl, CMuleListCtrl)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnLvnColumnclick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
	ON_WM_MEASUREITEM()// [TPT] - New Menu Styles
END_MESSAGE_MAP()

CIrcNickListCtrl::CIrcNickListCtrl()
{
	MEMZERO(m_asc_sort, sizeof m_asc_sort);
	m_iSortIndex=1;
	m_pParent = NULL;
}

void CIrcNickListCtrl::Init() 
{
	InsertColumn(0,GetResString(IDS_IRC_NICK),LVCFMT_LEFT,90);
	InsertColumn(1,GetResString(IDS_STATUS),LVCFMT_LEFT,70);

	// TODO restore settings
	SetSortArrow(m_iSortIndex, m_asc_sort[m_iSortIndex]);
    SortItems(SortProc, m_iSortIndex + ((m_asc_sort[m_iSortIndex]) ? 0 : 10));
}

int CIrcNickListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	Nick* item1 = (Nick*)lParam1;
	Nick* item2 = (Nick*)lParam2;
	switch(lParamSort)
	{
		//This will sort the list like MIRC
		case 0:
		case 1:
		case 11:
		{
			//TODO - MUST FIX THIS NOW THAT MODES ARE DONE DIFFERENT>
			if( item1->level == item2->level )
				return item1->nick.CompareNoCase(item2->nick);
			if( item1->level == -1 )
				return 1;
			if( item2->level == -1 )
				return -1;
			return item1->level - item2->level;
		}
		case 10:
			//This will put them in alpha order..
			return item1->nick.CompareNoCase(item2->nick);
		default:
			return 0;
	}
}

void CIrcNickListCtrl::OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	m_asc_sort[pNMLV->iSubItem] = !m_asc_sort[pNMLV->iSubItem];
	m_iSortIndex=pNMLV->iSubItem;

	SetSortArrow(m_iSortIndex, m_asc_sort[m_iSortIndex]);
	SortItems(SortProc, m_iSortIndex + ((m_asc_sort[m_iSortIndex]) ? 0 : 10));
	*pResult = 0;
}

// [TPT] - New Menu Styles BEGIN
void CIrcNickListCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	int iCurSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iCurSel == -1)
	{
		return;
	}
	Nick* nick = (Nick*)GetItemData(iCurSel);
	if( !nick )
	{
		return;
	}

	//Menu Configuration
	CMenuXP	*pMenu = new CMenuXP;
	pMenu->CreatePopupMenu();
	pMenu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
	pMenu->AddSideBar(new CMenuXPSideBar(17, MOD_VERSION));
	pMenu->SetSideBarStartColor(RGB(255,0,0));
	pMenu->SetSideBarEndColor(RGB(255,128,0));
	pMenu->SetSelectedBarColor(RGB(242,120,114));

	pMenu->AppendODMenu(MF_STRING, new CMenuXPText(Irc_Priv, GetResString(IDS_IRC_PRIVMESSAGE), theApp.LoadIcon(_T("PREF_IRC"), 16, 16)));
	pMenu->AppendODMenu(MF_STRING, new CMenuXPText(Irc_AddFriend, GetResString(IDS_IRC_ADDTOFRIENDLIST), theApp.LoadIcon(_T("friend2"), 16, 16)));


	if (!m_pParent->GetSendFileString().IsEmpty())
		pMenu->AppendODMenu(MF_STRING, new CMenuXPText(Irc_SendLink, GetResString(IDS_IRC_SENDLINK) + m_pParent->GetSendFileString()));
	else
		pMenu->AppendODMenu(MF_STRING, new CMenuXPText(Irc_SendLink, GetResString(IDS_IRC_SENDLINK) + GetResString(IDS_IRC_NOSFS)));

	pMenu->AppendODMenu(MF_STRING, new CMenuXPText(Irc_Kick, GetResString(IDS_IRC_KICK)));
	pMenu->AppendODMenu(MF_STRING, new CMenuXPText(Irc_Ban, _T("Ban")));


	//Ban currently uses chanserv to ban which seems to kick also.. May change this later..
//	NickMenu.AppendMenu(MF_STRING, Irc_KB, _T("Kick/Ban"));
	
	pMenu->AppendODMenu(MF_STRING, new CMenuXPText(Irc_Slap, GetResString(IDS_IRC_SLAP)));
	
	int length = m_sUserModeSettings.GetLength();
	if( length > 0 )
	{
		for( int i = 0; i < length; i++)
		{
			CString mode = m_sUserModeSettings.Mid(i,1);
			CString modeSymbol = m_sUserModeSymbols.Mid(i,1);
			if( nick->modes.Find(modeSymbol[0]) == -1 )
				pMenu->AppendODMenu(MF_STRING, new CMenuXPText(Irc_OpCommands+i, _T("Set +") + mode + _T(" ( Add ") + modeSymbol + _T(" )")));
			else
				pMenu->AppendODMenu(MF_STRING, new CMenuXPText(Irc_OpCommands+i+25, _T("Set -") + mode + _T(" ( Remove ") + modeSymbol + _T(" )")));
		}
	}


	//GetPopupMenuPos(*this, point);
	pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);

	delete pMenu;
}
void CIrcNickListCtrl::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	HMENU hMenu = AfxGetThreadState()->m_hTrackingMenu;
	CMenu	*pMenu = CMenu::FromHandle(hMenu);
	pMenu->MeasureItem(lpMeasureItemStruct);
	
	CWnd::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}
// [TPT] - New Menu Styles END

void CIrcNickListCtrl::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	//We double clicked a nick.. Try to open private channel
	int nickItem = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (nickItem != -1) 
	{
		Nick* nick = (Nick*)GetItemData(nickItem);
		if (nick)
		{
			//Valid nick, send a info message to force open a new channel..
			m_pParent->AddInfoMessage(nick->nick, GetResString(IDS_IRC_PRIVATECHANSTART));
		}
	}
	*pResult = 0;
}

Nick* CIrcNickListCtrl::FindNickByName(CString channel, CString name)
{
	//Find a nick in a specific channel..
	Channel* curr_channel = m_pParent->m_channelselect.FindChannelByName(channel);
	if( !curr_channel)
	{
		//Channel does not exist.. Abort.
		return NULL;
	}
	//We have a channel, find nick..
	// [TPT]
	for (POSITION pos = curr_channel->nicks.GetHeadPosition(); pos != NULL;)
	{
		POSITION cur_pos = pos;
		Nick* cur_nick = (Nick*)curr_channel->nicks.GetNext(pos);
		if (cur_nick->nick == name)
		{
			//We found our nick, return it..
			return cur_nick;
		}
	}
	//Nick was not in channel..
	return NULL;
}

Nick* CIrcNickListCtrl::NewNick( CString channel, CString nick )
{
	//Add a new nick to a channel..
	Channel* toaddchan = m_pParent->m_channelselect.FindChannelByName( channel );
	if( !toaddchan )
	{
		//Channel wasn't found, abort..
		return NULL;
	}
	//This is a little clumsy and makes you think the previous check wasn't needed,
	//But we need the channel object and FindNickByName doesn't do it..
	//TODO: Maybe create a special method to merge the two checks..
	if( FindNickByName( channel, nick ) )
	{
		//Check if we already have this nick..
		return NULL;
	}
	Nick* toaddnick=NULL;
	toaddnick = new Nick;

	//Remove all modes from the front of this nick
	while( m_sUserModeSymbols.Find(nick.Left(1)) != -1 )
	{
		toaddnick->modes += nick.Left(1);
		nick = nick.Mid(1);
	}

	//We now know the true nick
	toaddnick->nick = nick;

	//Set user level
	if( toaddnick->modes.GetLength() > 0 )
		toaddnick->level = m_sUserModeSymbols.Find(toaddnick->modes[0]);
	else
		toaddnick->level = -1;

	//Add new nick to channel.
	toaddchan->nicks.AddTail(toaddnick);
	if( toaddchan == m_pParent->m_channelselect.m_pCurrentChannel )
	{
		//This is our current channel, add it to our nicklist..
		uint16 itemnr = GetItemCount();
		itemnr = InsertItem(LVIF_PARAM,itemnr,0,0,0,0,(LPARAM)toaddnick);
		SetItemText(itemnr,0,(LPCTSTR)toaddnick->nick);
		SetItemText(itemnr,1,(LPCTSTR)toaddnick->modes);
		UpdateNickCount();
	}
	return toaddnick;
}

void CIrcNickListCtrl::RefreshNickList( CString channel )
{
	//Hide nickList to speed things up..
	ShowWindow(SW_HIDE);
	DeleteAllItems();
	Channel* refresh = m_pParent->m_channelselect.FindChannelByName( channel );
	if(!refresh )
	{
		//This is not a channel??? shouldn't happen..
		UpdateNickCount();
		ShowWindow(SW_SHOW);
		return;
	}
	// [TPT]
	for (POSITION pos = refresh->nicks.GetHeadPosition(); pos != NULL;)
	{
		//Add all nicks to list..
		POSITION cur_pos = pos;
		Nick* curr_nick = (Nick*)refresh->nicks.GetNext(pos);
		uint16 itemnr = GetItemCount();
		itemnr = InsertItem(LVIF_PARAM,itemnr,0,0,0,0,(LPARAM)curr_nick);
		SetItemText(itemnr,0,(LPCTSTR)curr_nick->nick);
		SetItemText(itemnr,1,(LPCTSTR)curr_nick->modes);
	}
	UpdateNickCount();
	ShowWindow(SW_SHOW);
}

bool CIrcNickListCtrl::RemoveNick( CString channel, CString nick )
{
	//Remove nick from a channel..
	Channel* update = m_pParent->m_channelselect.FindChannelByName( channel );
	if( !update )
	{
		//There was no channel..
		return false;
	}
	// [TPT]
	for( POSITION pos = update->nicks.GetHeadPosition(); pos!=NULL;)
	{
		//Go through nicks
		POSITION cur_pos = pos;
		Nick* curr_nick = (Nick*)update->nicks.GetNext(pos);
		if( curr_nick->nick == nick )
		{
			//Found nick..
			if( update == m_pParent->m_channelselect.m_pCurrentChannel )
			{
				//If it's our current channel, delete the nick from nickList
				LVFINDINFO find;
				find.flags = LVFI_PARAM;
				find.lParam = (LPARAM)curr_nick;
				sint32 result = FindItem(&find);
				DeleteItem(result);
				UpdateNickCount();
			}
			//remove nick and delete.
			update->nicks.RemoveAt(cur_pos);
			delete curr_nick;
			return true;
		}
	}
	return false;
}

void CIrcNickListCtrl::DeleteAllNick( CString channel )
{
	//Remove all nicks from a channel..
	Channel* curr_channel = m_pParent->m_channelselect.FindChannelByName(channel);
	if( !curr_channel )
	{
		//Channel was not found?
		return;
	}
	POSITION pos3, pos4;
	for(pos3 = curr_channel->nicks.GetHeadPosition();( pos4 = pos3) != NULL;)
	{
		//Remove all nicks..
		curr_channel->nicks.GetNext(pos3);
		Nick* cur_nick = (Nick*)curr_channel->nicks.GetAt(pos4);
		curr_channel->nicks.RemoveAt(pos4);
		delete cur_nick;
	}
}

void CIrcNickListCtrl::DeleteNickInAll( CString nick, CString message )
{
	//Remove a nick in all Channels.
	//This is a client that Quit the network, so we have a quit message..
	// [TPT]
	for (POSITION pos = m_pParent->m_channelselect.channelPtrList.GetHeadPosition(); pos != NULL;)
	{
		//Go through all channels..
		POSITION cur_pos = pos;
		Channel* cur_channel = (Channel*)(m_pParent->m_channelselect.channelPtrList).GetNext(pos);
		if(RemoveNick( cur_channel->name, nick ))
		{
			//If nick was in channel, put message in it saying why user quit..
			if( !thePrefs.GetIrcIgnoreQuitMessage() )
				m_pParent->AddInfoMessage( cur_channel->name, GetResString(IDS_IRC_HASQUIT), nick, message);
		}
	}
}

bool CIrcNickListCtrl::ChangeNick( CString channel, CString oldnick, CString newnick )
{
	//Someone changed there nick..
	Channel* update = m_pParent->m_channelselect.FindChannelByName( channel );
	if( !update )
	{
		//Didn't find a channel??
		return false;
	}
	// [TPT]
	for( POSITION pos = update->nicks.GetHeadPosition(); pos!=NULL;)
	{
		//Go through channel nicks.
		POSITION cur_pos = pos;
		Nick* curr_nick = (Nick*)update->nicks.GetNext(pos);
		if( curr_nick->nick == oldnick )
		{
			//Found nick to change.
			if((update = m_pParent->m_channelselect.m_pCurrentChannel) != NULL)
			{
				//This channle is in focuse, update nick in nickList
				LVFINDINFO find;
				find.flags = LVFI_PARAM;
				find.lParam = (LPARAM)curr_nick;
				sint32 itemnr = FindItem(&find);
				if (itemnr != (-1))
					SetItemText(itemnr,0,(LPCTSTR)newnick);
			}
			//Set new nick name..
			curr_nick->nick = newnick;
			return true;
		}
	}
	return false;
}

bool CIrcNickListCtrl::ChangeNickMode( CString channel, CString nick, CString mode )
{
	if( channel.Left(1) != "#" )
	{
		//Not a channel, this shouldn't happen
		return true;
	}
	if( nick == "" )
	{
		//No name, this shouldn't happen..
		return true;
	}
	//We are changing a mode to something..
	Channel* update = m_pParent->m_channelselect.FindChannelByName( channel );
	if( !update )
	{
		//No channel found, this shouldn't happen.
		return false;
	}
	// [TPT]
	for( POSITION pos = update->nicks.GetHeadPosition(); pos!=NULL;)
	{
		//Go through nicks
		POSITION cur_pos = pos;
		Nick* curr_nick = (Nick*)update->nicks.GetNext(pos);
		if( curr_nick->nick == nick )
		{
			//Found nick.
			//Update modes.
			int modeLevel = m_sUserModeSettings.Find(mode[1]);
			if( modeLevel != -1 )
			{
				CString modeSymbol = m_sUserModeSymbols.Mid(modeLevel,1);
				//Remove the symbol. This takes care of "-" and makes sure we don't add the same symbol twice.
				curr_nick->modes.Remove(modeSymbol[0]);
				if( mode.Left(1) == "+" )
				{
					//The nick doesn't have any other modes.. Just set it..
					if( curr_nick->modes == "" )
						curr_nick->modes = modeSymbol;
					else
					{
						//The nick does have other modes.. Lets make sure we put things in order..
						int index = 0;
						//This will pad the mode string..
						while( index < m_sUserModeSymbols.GetLength() )
						{
							if( curr_nick->modes.Find(m_sUserModeSymbols[index]) == -1 )
							{
								curr_nick->modes.Insert(index, _T(" "));
							}
							index++;
						}
						//Insert the new mode
						curr_nick->modes.Insert(modeLevel, modeSymbol[0]);
						//Remove pads
						curr_nick->modes.Remove(' ');
					}
				}
			}
			else
			{
				//This should never happen
				curr_nick->modes = "";
				curr_nick->level = -1;
				ASSERT(0);
			}

			//Update user level
			if( curr_nick->modes.GetLength() > 0 )
				curr_nick->level = m_sUserModeSymbols.Find(curr_nick->modes[0]);
			else
				curr_nick->level = -1;

			sint32 itemnr = -1;
			if( (update = m_pParent->m_channelselect.m_pCurrentChannel) != NULL )
			{
				//Channel was in focus, update the nickList.
				LVFINDINFO find;
				find.flags = LVFI_PARAM;
				find.lParam = (LPARAM)curr_nick;
				itemnr = FindItem(&find);
				if( itemnr != (-1) )
				{
					SetItemText(itemnr,1,(LPCTSTR)curr_nick->modes);
				}
			}
			return true;
		}
	}
	//Nick was not found in list??
	return false;
}

void CIrcNickListCtrl::ChangeAllNick( CString oldnick, CString newnick )
{
	//Change a nick in ALL the channels..
	Channel* currchannel = m_pParent->m_channelselect.FindChannelByName( oldnick );
	if( currchannel )
	{
		//We had a private room open with this nick.. Update the title of the channel!
		currchannel->name = newnick;
		TCITEM item;
		item.mask = TCIF_PARAM;
		item.lParam = -1;
		//Find channel tab..
		int i;
		for (i = 0; i < m_pParent->m_channelselect.GetItemCount();i++)
		{
			m_pParent->m_channelselect.GetItem(i,&item);
			if (((Channel*)item.lParam) == currchannel)
			{
				//Found tab, update it..
				item.mask = TCIF_TEXT;
				item.pszText = newnick.GetBuffer();
				m_pParent->m_channelselect.SetItem( i, &item);
				break;
			}
		}
	}
	//Go through all other channel nicklists..
	// [TPT]
	for (POSITION pos = m_pParent->m_channelselect.channelPtrList.GetHeadPosition(); pos != NULL;)
	{
		POSITION cur_pos = pos;
		Channel* cur_channel = (Channel*)(m_pParent->m_channelselect.channelPtrList).GetNext(pos);
		if(ChangeNick( cur_channel->name, oldnick, newnick ))
		{
			//Nick change successful, add a message to inform you..
			if( !thePrefs.GetIrcIgnoreMiscMessage() )
				m_pParent->AddInfoMessage( cur_channel->name, GetResString(IDS_IRC_NOWKNOWNAS), oldnick, newnick);
		}
	}
}

void CIrcNickListCtrl::UpdateNickCount()
{
	//Get the header control
	CHeaderCtrl* pHeaderCtrl;
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
	CString strRes;
	pHeaderCtrl = GetHeaderCtrl();
	if( GetItemCount() )
	{
		//Set nick count to current channel.
		strRes.Format(_T("%s[%i]"), GetResString(IDS_IRC_NICK), GetItemCount());
	}
	else
	{
		//No nicks in the list.. Your in Status or ChannleList
		strRes.Format(_T("%s"), GetResString(IDS_IRC_NICK));
	}
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(0, &hdi);
	strRes.ReleaseBuffer();
}

void CIrcNickListCtrl::Localize()
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	CString strRes;
	strRes = GetResString(IDS_STATUS);
	HDITEM hdi;
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	hdi.mask = HDI_TEXT;
	pHeaderCtrl->SetItem(1, &hdi);
	UpdateNickCount();
}

BOOL CIrcNickListCtrl::OnCommand(WPARAM wParam,LPARAM lParam )
{
	int nickItem = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	int chanItem = m_pParent->m_channelselect.GetCurSel(); 

	switch( wParam )
	{
		case Irc_Priv: 
		{
			//Right clicked and choose start private chan.
			Nick* nick = (Nick*)GetItemData(nickItem);
			if(nick)
			{
				//Send a message with the nick as the channel name, this will create a new window with the default message.
				m_pParent->AddInfoMessage( nick->nick, GetResString(IDS_IRC_PRIVATECHANSTART));
			}
            return true;
		}
		case Irc_Kick: 
		{
			//Kick someone from a channel
			Nick* nick = (Nick*)GetItemData(nickItem);
			TCITEM item;
			item.mask = TCIF_PARAM;
			m_pParent->m_channelselect.GetItem(chanItem,&item);
			Channel* chan = (Channel*)item.lParam;
			if( nick && chan )
			{
				//We have a nick and chan, send the command.
				CString send;
				send.Format(_T("KICK %s %s"), chan->name, nick->nick );
				m_pParent->m_pIrcMain->SendString(send);
			}
			return true;
		}
		case Irc_Ban: 
		{
			//Kick someone from a channel
			Nick* nick = (Nick*)GetItemData(nickItem);
			TCITEM item;
			item.mask = TCIF_PARAM;
			m_pParent->m_channelselect.GetItem(chanItem,&item);
			Channel* chan = (Channel*)item.lParam;
			if( nick && chan )
			{
				//We have a nick and chan, send the command.
				CString send;
				send.Format(_T("cs ban %s %s"), chan->name, nick->nick );
				m_pParent->m_pIrcMain->SendString(send);
			}
			return true;
		}
		case Irc_Slap: 
		{
			//Do a silly slap on someone
			Nick* nick = (Nick*)GetItemData(nickItem);
			TCITEM item;
			item.mask = TCIF_PARAM;
			m_pParent->m_channelselect.GetItem(chanItem,&item);
			Channel* chan = (Channel*)item.lParam;
			if( nick && chan )
			{
				//We have a nick and chan, send the command.
				CString send;
				send.Format( GetResString(IDS_IRC_SLAPMSGSEND), chan->name, nick->nick );
				m_pParent->AddInfoMessage( chan->name, GetResString(IDS_IRC_SLAPMSG), m_pParent->m_pIrcMain->GetNick(), nick->nick);
				m_pParent->m_pIrcMain->SendString(send);
			}
			return true;
		}
		case Irc_AddFriend: 
		{
			//Attempt to add this person as a friend.
			Nick* nick = (Nick*)GetItemData(nickItem);
			TCITEM item;
			item.mask = TCIF_PARAM;
			m_pParent->m_channelselect.GetItem(chanItem,&item);
			Channel* chan = (Channel*)item.lParam;
			if( nick && chan )
			{
				//We have a nick and chan, send the command.
				//SetVerify() sets a new challenge which is required by the other end to respond with for some protection.
				CString send;
				send.Format(_T("PRIVMSG %s :\001RQSFRIEND|%i|\001"), nick->nick, m_pParent->m_pIrcMain->SetVerify() );
				m_pParent->m_pIrcMain->SendString(send);
			}
			return true;
		}
		case Irc_SendLink: 
		{
			//Send a ED2K link to someone..
			if(!m_pParent->GetSendFileString())
			{
				//There is no link in the buffer, abort.
				return true;
			}
			Nick* nick = (Nick*)GetItemData(nickItem);
			TCITEM item;
			item.mask = TCIF_PARAM;
			m_pParent->m_channelselect.GetItem(chanItem,&item);
			Channel* chan = (Channel*)item.lParam;
			if( nick && chan )
			{
				//We have a nick and chan, send the command.
				//We send our nick and ClientID to allow the other end to only accept links from friends..
				CString send;
				send.Format(_T("PRIVMSG %s :\001SENDLINK|%s|%s\001"), nick->nick, md4str(thePrefs.GetUserHash()), m_pParent->GetSendFileString() );
				m_pParent->m_pIrcMain->SendString(send);
			}
			return true;
		}
	}
	if( wParam >= Irc_OpCommands && wParam < Irc_OpCommands+25)
	{
		int index = wParam - Irc_OpCommands;
		CString mode = m_sUserModeSettings.Mid(index,1);
		Nick* nick = (Nick*)GetItemData(nickItem);
		TCITEM item;
		item.mask = TCIF_PARAM;
		m_pParent->m_channelselect.GetItem(chanItem,&item);
		Channel* chan = (Channel*)item.lParam;
		if( nick && chan )
		{
			//We have a nick and chan, send the command.
			CString send;
			send.Format(_T("MODE %s +%s %s"), chan->name, mode, nick->nick );
			m_pParent->m_pIrcMain->SendString(send);
		}
		return true;
	}
	if( wParam >= Irc_OpCommands+25 && wParam < Irc_OpCommands+50)
	{
		int index = wParam - Irc_OpCommands-25;
		CString mode = m_sUserModeSettings.Mid(index,1);
		Nick* nick = (Nick*)GetItemData(nickItem);
		TCITEM item;
		item.mask = TCIF_PARAM;
		m_pParent->m_channelselect.GetItem(chanItem,&item);
		Channel* chan = (Channel*)item.lParam;
		if( nick && chan )
		{
			//We have a nick and chan, send the command.
			CString send;
			send.Format(_T("MODE %s -%s %s"), chan->name, mode, nick->nick );
			m_pParent->m_pIrcMain->SendString(send);
		}
		return true;
	}
	return true;
}