// TBHMM.cpp : implementation file
//

#include "stdafx.h"
#include "emule.h"
#include "TBHMM.h"
#include "BandWidthControl.h" // [TPT]
#include "GetCpuUsage.h"
#include "preferences.h"
#include "downloadqueue.h"
#include "uploadqueue.h"
#include "server.h"
#include "sockets.h"
#include "emuledlg.h"
#include "menuCmds.h"
#include "opcodes.h"
#include "WebServices.h"
#include "mod_version.h" // [TPT]


// TBHMM dialog
//IMPLEMENT_DYNAMIC(CTBHMM, CSnapDialog)
CTBHMM::CTBHMM(CWnd* pParent /*=NULL*/)
: CSnapDialog(CTBHMM::IDD, pParent) 
{
	m_hCSConn = NULL;
	m_hCSCing = NULL;
	m_hCSDconn = NULL;
	smmin = thePrefs.GetSpeedMeterMin();
	smmax = thePrefs.GetSpeedMeterMax();
	running = false;
}

CTBHMM::~CTBHMM()
{
	running = false;
	if(m_hCSConn)
		DestroyIcon(m_hCSConn);
	if(m_hCSCing)
		DestroyIcon(m_hCSCing);
	if(m_hCSDconn)
		DestroyIcon(m_hCSDconn);
	m_ctrlSpeedMeter.DestroyWindow();	
}

void CTBHMM::DoDataExchange(CDataExchange* pDX)
{
	CSnapDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MM_CONNSTATE, m_ctrlMMConnState);
}


BEGIN_MESSAGE_MAP(CTBHMM, CSnapDialog)
	ON_BN_CLICKED(IDC_MM_MENU, OnMenuButtonClicked)
END_MESSAGE_MAP()


// TBHMM message handlers
BOOL CTBHMM::OnInitDialog(){
	CSnapDialog::OnInitDialog();
	InitWindowStyles(this);
	sysinfo.Init();
	CString buffer = MOD_VERSION;	
	CString buffer2;
	buffer += _T(" ");
	buffer += (GetResString(IDS_MINIMULE));
	buffer += _T("       ||       ");	
	buffer2.Format(_T("CPU: %3d%% Mem: %dK"),GetCpuUsage(_T("emule")), sysinfo.GetProcessMemoryUsage());
	buffer += buffer2;
	SetWindowText(buffer);
	//ika: changing the con-dis icons, I think is more logical :)
	m_hCSConn = theApp.LoadIcon(_T("DISCONNECT"), 32, 32);
	m_hCSCing = theApp.LoadIcon(_T("STOPCONNECTING"), 32, 32);
	m_hCSDconn = theApp.LoadIcon(_T("CONNECT"), 32, 32);
	RECT rect;
	UINT nPlaceholderID = IDC_SM_PLACEHOLDER;
	this->GetDlgItem(nPlaceholderID)->GetWindowRect(&rect);
	this->ScreenToClient(&rect);
	m_ctrlSpeedMeter.CreateEx(WS_EX_STATICEDGE, NULL, NULL, WS_CHILD | WS_VISIBLE, rect, this, 123, 0);
	m_ctrlSpeedMeter.SetRange(smmin,smmax);
	m_ctrlSpeedMeter.EnableWindow(true);
	RunMiniMule();
	return true;
}


// [TPT] - Improved minimule
void CTBHMM::RunMiniMule(bool resetMiniMule)
{
	reset = resetMiniMule;
	if (!running)
	{
		running = true;
		AfxBeginThread(run, this, THREAD_PRIORITY_LOWEST);	
	}
}

UINT CTBHMM::run(LPVOID p)
{
	CTBHMM * minimule = (CTBHMM *)p;
	minimule->run();
	return 0;
}

void CTBHMM::run()
{
	MMUpdate();
    running = FALSE;
}
// [TPT] - Improved minimule


void CTBHMM::MMUpdate()
{

	if (!theApp.emuledlg->IsRunning()) 
		return;

	uint32 mmUpdateIt = thePrefs.GetMiniMuleUpdate();
	if (::GetTickCount() - m_nLastUpdate < mmUpdateIt*1000)
		return;
	m_nLastUpdate = ::GetTickCount();
	if (reset)
	{
		smmin = thePrefs.GetSpeedMeterMin();
		smmax = thePrefs.GetSpeedMeterMax();
		m_ctrlSpeedMeter.SetRange(smmin,smmax);
		m_ctrlSpeedMeter.SoftReset();
	}
	TCHAR buffer3[50];
	CString buffer ;
	CString buffer2;
	
	SetTransparent(thePrefs.GetMiniMuleTransparency());
	
	buffer = MOD_VERSION;	
	buffer += _T(" ");
	buffer += (GetResString(IDS_MINIMULE));
	buffer += _T("       ||       ");	
	buffer2.Format(_T("CPU: %3d%% Mem: %dK"),GetCpuUsage(_T("emule")), sysinfo.GetProcessMemoryUsage());
	buffer += buffer2;
	SetWindowText(buffer);

	// [TPT] - Retrieve the current datarates
	uint32 eMuleIn;	uint32 eMuleInOverall;
	uint32 eMuleOut; uint32 eMuleOutOverall;
	uint32 notUsed;
	theApp.pBandWidthControl->GetDatarates(thePrefs.GetDatarateSamples(),
										   eMuleIn, eMuleInOverall,
										   eMuleOut, eMuleOutOverall,
										   notUsed, notUsed);

	const float upratekb = (float)eMuleOut / 1024.0f;
	const float downratekb = (float)eMuleIn / 1024.0f;
	const float lastuprateoverheadkb = (float)(eMuleOutOverall- eMuleOut) / 1024.0f;
	const float lastdownrateoverheadkb = (float)(eMuleInOverall- eMuleIn) / 1024.0f;		
	// [TPT]
	CDownloadQueue::SDownloadStats myStats;
	theApp.downloadqueue->GetDownloadStats(myStats);

	if (theApp.serverconnect->IsConnected())
		m_ctrlMMConnState.SetIcon(m_hCSDconn);
	else if (theApp.serverconnect->IsConnecting())
		m_ctrlMMConnState.SetIcon(m_hCSCing);
	else
		m_ctrlMMConnState.SetIcon(m_hCSConn);
	if (theApp.serverconnect->IsConnected())
	{ 
		buffer.Format(_T("%u"),theApp.serverconnect->GetClientID());
		if (theApp.serverconnect->IsLowID()) buffer2 = GetResString(IDS_IDLOW);
		else buffer2 = GetResString(IDS_IDHIGH);
		GetDlgItem(IDC_MM_SERVER)->SetWindowText(theApp.serverconnect->GetCurrentServer()->GetListName());
		GetDlgItem(IDC_MM_ID)->SetWindowText(buffer + _T(" [") + buffer2 + _T("]"));
	} else {
		GetDlgItem(IDC_MM_SERVER)->SetWindowText(GetResString(IDS_NOTCONNECTED));
		GetDlgItem(IDC_MM_ID)->SetWindowText(GetResString(IDS_NOTCONNECTED));
	}
	buffer.Format( GetResString( IDS_MM_ACTDL ) , myStats.a[1] );
	GetDlgItem(IDC_MM_DLCOUNT)->SetWindowText(buffer);
	buffer.Format(GetResString(IDS_STATS_ACTUL),theApp.uploadqueue->GetUploadQueueLength());
	GetDlgItem(IDC_MM_ULCOUNT)->SetWindowText(buffer);
		buffer.Format(GetResString(IDS_MM_DATA),CastItoXBytes(theApp.pBandWidthControl->GeteMuleOut()/*+thePrefs.GetTotalUploaded()*/, false, false),CastItoXBytes( theApp.pBandWidthControl->GeteMuleIn()/*+thePrefs.GetTotalDownloaded()*/, false, false));
	GetDlgItem(IDC_MM_ULDLTRANS)->SetWindowText(buffer);
	if( thePrefs.ShowOverhead() )
		_stprintf(buffer3,GetResString(IDS_UPDOWN), (float)upratekb, (float)lastuprateoverheadkb, (float)downratekb, (float)lastdownrateoverheadkb);
	else
		_stprintf(buffer3,GetResString(IDS_UPDOWNSMALL),(float)upratekb, (float)downratekb);
	GetDlgItem(IDC_MM_ULDL)->SetWindowText(buffer3);
	SetSpeedMeterValues((float)upratekb, (float)downratekb);
}

void CTBHMM::OnMenuButtonClicked()
{
	CRect rectBn;
	CPoint thePoint;
	GetDlgItem(IDC_MM_MENU)->GetWindowRect(&rectBn);
	thePoint = rectBn.BottomRight();
	DoMenu(thePoint);
}

void CTBHMM::DoMenu(CPoint doWhere)
{
	DoMenu( doWhere, TPM_RIGHTALIGN | TPM_RIGHTBUTTON );
}

void CTBHMM::DoMenu(CPoint doWhere, UINT nFlags)
{
	// [TPT] - New Menu Styles BEGIN
	//Menu Configuration
	CMenuXP	*pMenu = new CMenuXP;
	pMenu->CreatePopupMenu();
	pMenu->SetMenuStyle(CMenuXP::STYLE_STARTMENU);
	pMenu->AddSideBar(new CMenuXPSideBar(17, GetResString(IDS_PHOENIX)));
	pMenu->SetSideBarStartColor(RGB(255,0,0));
	pMenu->SetSideBarEndColor(RGB(255,128,0));
	pMenu->SetSelectedBarColor(RGB(242,120,114));
		
	pMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_MM_FORUM, GetResString(IDS_MMMENU_PHOENIX_FORUM)));	
	pMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_MM_PROJECT, GetResString(IDS_MMMENU_PHOENIX_PROJECT)));	
	pMenu->AppendODMenu(MF_STRING, new CMenuXPText(MP_HM_LINK4, GetResString(IDS_PHOENIX_LINKHP)));	
	
	CMenuXP	*pMenuOth = new CMenuXP;
	pMenuOth->CreatePopupMenu();
	pMenuOth->SetMenuStyle(CMenuXP::STYLE_STARTMENU);	
	if (theApp.IsConnected())
		pMenuOth->AppendODMenu(MF_STRING, new CMenuXPText(MP_MM_DIS, GetResString(IDS_IRC_DISCONNECT)));			
	else
		pMenuOth->AppendODMenu(MF_STRING, new CMenuXPText(MP_MM_CON, GetResString(IDS_IRC_CONNECT)));			
	pMenuOth->AppendSeparator();
	pMenuOth->AppendODPopup(MF_STRING|MF_POPUP, pMenu, new CMenuXPText(0,GetResString(IDS_MMMENU_WEB)));	
	pMenuOth->AppendSeparator();	
	pMenuOth->AppendODMenu(MF_STRING, new CMenuXPText(MP_MM_EXIT, GetResString(IDS_MMMENU_EXIT)));	
	
	CMenuXP	*pMenuMM = new CMenuXP;
	pMenuMM->CreatePopupMenu();
	pMenuMM->SetMenuStyle(CMenuXP::STYLE_STARTMENU);	
	pMenuMM->AddSideBar(new CMenuXPSideBar(17, GetResString(IDS_MINIMULE)));
	pMenuMM->SetSideBarStartColor(RGB(255,0,0));
	pMenuMM->SetSideBarEndColor(RGB(255,128,0));
	pMenuMM->SetSelectedBarColor(RGB(242,120,114));
    pMenuMM->AppendODMenu(MF_STRING, new CMenuXPText(MP_MM_HIDE, GetResString(IDS_MMMENU_HIDE)));		
	pMenuMM->AppendODMenu(MF_STRING, new CMenuXPText(MP_MM_RESTORE, GetResString(IDS_MMMENU_RESTORE)));		
	pMenuMM->AppendSeparator();
	pMenuMM->AppendODMenu(MF_STRING, new CMenuXPText(MP_MM_RESET, GetResString(IDS_MMMENU_RESET)));		
	pMenuMM->AppendODPopup(MF_STRING|MF_POPUP, pMenuOth, new CMenuXPText(0,GetResString(IDS_WEB_ACTIONS)));	
	
	pMenuMM->TrackPopupMenu(nFlags, doWhere.x, doWhere.y, this);
	
	delete pMenu;	
	delete pMenuOth;	
	delete pMenuMM;
	// [TPT] - New Menu Styles END
	
}

// [TPT] - New Menu Styles BEGIN
void CTBHMM::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	HMENU hMenu = AfxGetThreadState()->m_hTrackingMenu;
	CMenu	*pMenu = CMenu::FromHandle(hMenu);
	pMenu->MeasureItem(lpMeasureItemStruct);
	
	CWnd::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}
// [TPT] - New Menu Styles END

BOOL CTBHMM::OnCommand( WPARAM wParam, LPARAM lParam )
{
	if (wParam>=MP_WEBURL && wParam<=MP_WEBURL+99) {
		theWebServices.RunURL(NULL, wParam);
	}

	switch (wParam) {
		case MP_MM_HIDE:
			{
				theApp.minimule->ShowWindow(SW_HIDE);
				break;
			}
		case MP_MM_RESTORE:
			{
				theApp.emuledlg->RestoreWindow();
				theApp.minimule->ShowWindow(SW_HIDE);
				break;
			}
		case MP_MM_RESET:
			{
				smmin = thePrefs.GetSpeedMeterMin();
				smmax = thePrefs.GetSpeedMeterMax();
				m_ctrlSpeedMeter.SetRange(smmin,smmax);
				m_ctrlSpeedMeter.SoftReset();
				break;
			}
		case MP_MM_DIS:
			{
				theApp.emuledlg->CloseConnection();
				break;
			}
		case MP_MM_CON:
			{
				theApp.emuledlg->StartConnection();
				break;
			}
		/*case MP_MM_HOME:
			{
				ShellExecute(NULL, NULL, "http://sourceforge.net/projects/emulephoenix/", NULL, thePrefs.GetAppDir(), SW_SHOWDEFAULT);
				break;
			}*/
		case MP_MM_FORUM:
			{
				ShellExecute(NULL, NULL, _T("http://www.emulespana.net/foros/index.php?showforum=41"), NULL, thePrefs.GetAppDir(), SW_SHOWDEFAULT);
				break;
			}
		case MP_MM_PROJECT:
			{
				ShellExecute(NULL, NULL, _T("http://sourceforge.net/projects/emulephoenix/"), NULL, thePrefs.GetAppDir(), SW_SHOWDEFAULT);
				break;
			}
		case MP_HM_LINK4: // [TPT]
			ShellExecute(NULL, NULL, _T("http://www.emulespana.net/foros/index.php?showtopic=55050"), NULL, thePrefs.GetAppDir(), SW_SHOWDEFAULT);
			break;
		case MP_MM_EXIT:
			{
				this->ShowWindow(SW_HIDE);
				theApp.emuledlg->OnClose();
				return true;
			}
	}

	return CSnapDialog::OnCommand(wParam, lParam);
}