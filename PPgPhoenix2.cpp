#include "stdafx.h"
#include "emule.h"
#include "PPgPhoenix2.h"
#include "preferences.h"
#include "PreferencesDlg.h"
#include "webcache.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CPPgPhoenix2 dialog

IMPLEMENT_DYNAMIC(CPPgPhoenix2, CPropertyPage) // [TPT] - SLUGFILLER: modelessDialogs
CPPgPhoenix2::CPPgPhoenix2()
	: CPropertyPage(CPPgPhoenix2::IDD) // [TPT] - SLUGFILLER: modelessDialogs
{	
	//[TPT] - Webcache
	guardian=false;
	bCreated2 = false;
	showadvanced = false;

}

CPPgPhoenix2::~CPPgPhoenix2()
{
}

void CPPgPhoenix2::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PPG_PHOENIX2_TAB, m_tabCtr);
}

BEGIN_MESSAGE_MAP(CPPgPhoenix2, CPropertyPage) // [TPT] - SLUGFILLER: modelessDialogs
	// TAB control
	ON_NOTIFY(TCN_SELCHANGE, IDC_PPG_PHOENIX2_TAB, OnTabSelectionChange)
	//[TPT] - Webcache
	ON_EN_CHANGE(IDC_webcacheName, OnSettingsChange) 
	ON_EN_CHANGE(IDC_webcachePort, OnSettingsChange)
	ON_EN_CHANGE(IDC_BLOCKS, OnSettingsChange)
	ON_BN_CLICKED(IDC_Activatewebcachedownloads, OnEnChangeActivatewebcachedownloads)
	ON_BN_CLICKED(IDC_DETECTWEBCACHE, OnBnClickedDetectWebCache)
	ON_BN_CLICKED(IDC_EXTRATIMEOUT, OnSettingsChange)
	ON_BN_CLICKED(IDC_LOCALTRAFFIC, OnSettingsChange)
	ON_BN_CLICKED(IDC_PERSISTENT_PROXY_CONNS, OnSettingsChange)
	ON_BN_CLICKED(IDC_ADVANCEDCONTROLS, OnBnClickedAdvancedcontrols)
	ON_BN_CLICKED(IDC_TestProxy, OnBnClickedTestProxy)//JP TMP
	ON_BN_CLICKED(IDC_WCUPDATEAUTO, OnSettingsChange)
END_MESSAGE_MAP()

// CPPgPhoenix message handlers

BOOL CPPgPhoenix2::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	
	// Init the Tab control
	InitTab();

	// Create and Init all controls
	InitControl();

	// Set default tab
	m_currentTab = NONE;
	m_tabCtr.SetCurSel(0);
	SetTab(WEBCACHE);
	// Load setting
	LoadSettings();
	Localize();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CPPgPhoenix2::LoadSettings()
{
		// [TPT] - Webcache ----------------------------------------------------------
		CString strBuffer;
		if (!(thePrefs.UsesCachedTCPPort()))	// if the user doesn't use a cacheable port, disable everything and disable webcachedownload
		{
			m_enableWebcache.SetCheck(false);
			strBuffer.Format(_T("%s"), thePrefs.webcacheName);
			m_webcacheAddressEdit.SetWindowText(strBuffer);
			strBuffer.Format(_T("%d"), thePrefs.webcachePort);
			m_webcachePortEdit.SetWindowText(strBuffer);
			EnableWebcacheControls(false);
			// display wrong port warning
			AfxMessageBox(GetResString(IDS_WrongPortforWebcache), MB_ICONSTOP | MB_OK);
			thePrefs.webcacheEnabled=false;
			return;
		}

		// check/uncheck webcache
		m_enableWebcache.EnableWindow(true);
		m_enableWebcache.SetCheck(thePrefs.webcacheEnabled);
		// enter name 
		strBuffer.Format(_T("%s"), thePrefs.webcacheName);
		m_webcacheAddressEdit.SetWindowText(strBuffer);
		// enter Port
		strBuffer.Format(_T("%d"), thePrefs.webcachePort);
		m_webcachePortEdit.SetWindowText(strBuffer);
		// load parts to download before reconnect
		strBuffer.Format(_T("%d"), thePrefs.GetWebCacheBlockLimit());
		m_webcacheBlocksEdit.SetWindowText(strBuffer);
		// load extratimeoutsetting
		m_webcacheTimeout.SetCheck(thePrefs.GetWebCacheExtraTimeout());
		// load localtrafficsettings
		m_webcacheNotSameISP.SetCheck(thePrefs.GetWebCacheCachesLocalTraffic()==false);
		// load persistent proxy conns
		m_webcachePersistent.SetCheck(thePrefs.PersistentConnectionsForProxyDownloads);
		m_webcacheUpdateAuto.SetCheck(thePrefs.WCAutoupdate);
		EnableWebcacheControls(thePrefs.webcacheEnabled);

		// [TPT] - Webcache END ------------------------------------------------------

}

BOOL CPPgPhoenix2::OnApply()
{	

	bool bRestartApp = false;
	CString buffer;


	// [TPT] - Webcache BEGIN -----------------------------------------------------
	m_webcacheAddressEdit.GetWindowText(buffer);
	if (thePrefs.webcacheName != buffer)
	{
		thePrefs.webcacheName = buffer;
		bRestartApp = true;
	}

	m_webcachePortEdit.GetWindowText(buffer);
	uint16 nNewPort = (uint16)_tstol(buffer);
	if (nNewPort != thePrefs.webcachePort)
	{
		thePrefs.webcachePort = nNewPort;
	}

	// set thePrefs.webcacheEnabled
	thePrefs.webcacheEnabled = m_enableWebcache.GetCheck() == BST_CHECKED;
	
	// set thePrefs.webcacheBlockLimit
	m_webcacheBlocksEdit.GetWindowText(buffer);
	uint16 nNewBlocks = (uint16)_tstol(buffer);
	if ((!nNewBlocks) || (nNewBlocks > 50000) || (nNewBlocks < 0)) nNewBlocks=0;
	if (nNewBlocks != thePrefs.GetWebCacheBlockLimit())
	{
		thePrefs.SetWebCacheBlockLimit(nNewBlocks);
	}

	// set thePrefs.WebCacheExtraTimeout
	thePrefs.SetWebCacheExtraTimeout(m_webcacheTimeout.GetCheck() == BST_CHECKED);

	thePrefs.SetWebCacheCachesLocalTraffic(m_webcacheNotSameISP.GetCheck() == BST_UNCHECKED);//ojo aki...esta bien

	// set thePrefs.PersistentConnectionsForProxyDownloads
	thePrefs.PersistentConnectionsForProxyDownloads = m_webcachePersistent.GetCheck() == BST_CHECKED;
	thePrefs.WCAutoupdate = m_webcacheUpdateAuto.GetCheck() == BST_CHECKED;
	// [TPT] - Webcache END ------------------------------------------------------


	// Refresh Setting
	LoadSettings();
	SetModified(FALSE);	

	if (bRestartApp)
	{
		AfxMessageBox(GetResString(IDS_SETTINGCHANGED_RESTART));
		thePrefs.WebCacheDisabledThisSession = true;
	}

	return CPropertyPage::OnApply();
}

void CPPgPhoenix2::Localize(void)
{	
	if(m_hWnd)
	{
		// Create an icon list for the tab control
		m_ImageList.DeleteImageList();
		m_ImageList.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
		m_ImageList.SetBkColor(CLR_NONE);
		m_ImageList.Add(CTempIconLoader(_T("WEBCACHE")));//webcache image :D

		CString Buffer;
		SetWindowText(_T("pHoeniX eXtreme"));		

		int row = m_tabCtr.GetRowCount();
		InitTab(); // To update string, could be improved
		if(row != 0 && row != m_tabCtr.GetRowCount())
		{
			// Shift all windows object
			// .. to do
		}

			m_webcacheAddressStatic.SetWindowText(GetResString(IDS_WC_ADDRESS));
			m_webcachePortStatic.SetWindowText(GetResString(IDS_WC_PORT));
			m_webcacheAutodetect.SetWindowText(GetResString(IDS_WC_AUTODETECT));
			m_webcacheBlocksStatic.SetWindowText(GetResString(IDS_WC_BLOCK));
			m_webcacheBlocksSize.SetWindowText(GetResString(IDS_WC_BLOCKSIZE));
			m_webcacheTimeout.SetWindowText(GetResString(IDS_WC_TIMETOUT));
			m_webcacheNotSameISP.SetWindowText(GetResString(IDS_WC_NOTSAMEIP));
			m_webcachePersistent.SetWindowText(GetResString(IDS_WC_PERSISTENT));
			m_webcacheUpdateAuto.SetWindowText(GetResString(IDS_WC_AUTOUPDATE));
			m_hideControls.SetWindowText((showadvanced == true) ? GetResString(IDS_WC_HIDECONTROLS) : GetResString(IDS_WC_SHOWCONTROLS));
			m_webcacheBox.SetWindowText(GetResString(IDS_WC_BOX));
			m_enableWebcache.SetWindowText(GetResString(IDS_WC_ENABLEWC));
			m_testProxy.SetWindowText(GetResString(IDS_WC_TESTPROXY));
	}



}

void CPPgPhoenix2::InitTab(){
	// Clear all to be sure
	m_tabCtr.DeleteAllItems();
	
	// Change style
	// Remark: It seems that the multi-row can not be activated with the properties
	m_tabCtr.ModifyStyle(0, TCS_MULTILINE);

	// Add all items with icon (connection, tweak, etc...)
	m_tabCtr.SetImageList(&m_ImageList);
	m_tabCtr.InsertItem(TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM, WEBCACHE, MOD_WC_VERSION, 0, (LPARAM)WEBCACHE); 	
}

void CPPgPhoenix2::InitControl(){
	// Remark: don't use the dialog editor => avoid to merge rc

	// Retrieve the bottom of the tab's header
	RECT rect1;
	RECT rect2;
	m_tabCtr.GetWindowRect(&rect1);
	ScreenToClient(&rect1);
	m_tabCtr.GetItemRect(m_tabCtr.GetItemCount() - 1 , &rect2);
	const int top = rect1.top + (rect2.bottom - rect2.top + 1) * m_tabCtr.GetRowCount() + 10;
	const int right = rect1.left + 10;
	const int bottom = rect1.bottom;
	
	m_enableWebcache.CreateEx(0, _T("BUTTON"), _T(""), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_AUTOCHECKBOX, 
									CRect(right, top+10, right+300, top+30), this, IDC_Activatewebcachedownloads);
	m_enableWebcache.SetFont(GetFont());
	m_webcacheBox.CreateEx(0, _T("BUTTON"), _T(""), 
							   WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
							   BS_GROUPBOX,
							   CRect(right, top+40, right+300, top+350), this, IDC_WC_BOX);
	m_webcacheBox.SetFont(GetFont());

	m_webcacheAddressStatic.CreateEx(0, _T("STATIC"), _T(""), 
						WS_CHILD /*| WS_VISIBLE*/, 
						CRect(right+10, top+60, right+290, top+80), this, IDC_WC_ADDRESSTATIC);
	m_webcacheAddressStatic.SetFont(GetFont());
	m_webcacheAddressEdit.CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), 
						WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
						ES_LEFT | ES_AUTOHSCROLL, 
						CRect(right+10, top+80, right+290, top+100), this, IDC_webcacheName);
	m_webcacheAddressEdit.SetFont(GetFont());

	m_webcachePortEdit.CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), 
						WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
						ES_LEFT | ES_AUTOHSCROLL | ES_NUMBER, 
						CRect(right+10, top+105, right+60, top+125), this, IDC_webcachePort);
	m_webcachePortEdit.SetFont(GetFont());
	m_webcachePortEdit.SetLimitText(5);

	m_webcachePortStatic.CreateEx(0, _T("STATIC"), _T(""), 
						WS_CHILD /*| WS_VISIBLE*/, 
						CRect(right+65, top+105, right+290, top+125), this, IDC_WC_PORTSTATIC);
	m_webcachePortStatic.SetFont(GetFont());

	m_testProxy.CreateEx(0, _T("BUTTON"), _T(""), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_FLAT,
									CRect(right+10, top+125, right+290, top+145), this, IDC_TestProxy);
	m_testProxy.SetFont(GetFont());

	m_webcacheBlocksStatic.CreateEx(0, _T("STATIC"), _T(""), 
						WS_CHILD /*| WS_VISIBLE*/, 
						CRect(right+10, top+145, right+200, top+185), this, IDC_WC_BLOCKTEXT);
	m_webcacheBlocksStatic.SetFont(GetFont());

	m_webcacheBlocksSize.CreateEx(0, _T("STATIC"), _T(""), 
						WS_CHILD /*| WS_VISIBLE*/, 
						CRect(right+205, top+170, right+290, top+190), this, IDC_WC_BLOCKSSIZE);
	m_webcacheBlocksSize.SetFont(GetFont());

	m_webcacheBlocksEdit.CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), 
						WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
						ES_RIGHT | ES_AUTOHSCROLL | ES_NUMBER, 
						CRect(right+205, top+150, right+245, top+170), this, IDC_BLOCKS);
	m_webcacheBlocksEdit.SetFont(GetFont());
	m_webcacheBlocksEdit.SetLimitText(5);

	m_webcacheTimeout.CreateEx(0, _T("BUTTON"), _T(""), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_AUTOCHECKBOX, 
									CRect(right+10, top+195, right+290, top+215), this, IDC_EXTRATIMEOUT);
	m_webcacheTimeout.SetFont(GetFont());
	m_webcacheNotSameISP.CreateEx(0, _T("BUTTON"), _T(""), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_AUTOCHECKBOX, 
									CRect(right+10, top+220, right+290, top+240), this, IDC_LOCALTRAFFIC);
	m_webcacheNotSameISP.SetFont(GetFont());
	m_webcachePersistent.CreateEx(0, _T("BUTTON"), _T(""), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_AUTOCHECKBOX, 
									CRect(right+10, top+245, right+290, top+265), this, IDC_PERSISTENT_PROXY_CONNS);
	m_webcachePersistent.SetFont(GetFont());

	m_webcacheUpdateAuto.CreateEx(0, _T("BUTTON"), _T(""), 
		WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
		BS_AUTOCHECKBOX, 
		CRect(right+10, top+270, right+290, top+290), this, IDC_WCUPDATEAUTO);
	m_webcacheUpdateAuto.SetFont(GetFont());


	m_webcacheAutodetect.CreateEx(0, _T("BUTTON"), _T(""), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_FLAT,
									CRect(right+10, top+295, right+290, top+315), this, IDC_DETECTWEBCACHE);
	m_webcacheAutodetect.SetFont(GetFont());

	m_wndSubmitWebcacheLink2.CreateEx(0, 0, _T(""), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP | WS_BORDER |
									BS_FLAT | HTC_WORDWRAP | HTC_UNDERLINE_HOVER,
									CRect(right+10, top+320, right+290, top+340), this, IDC_WEBCACHELINK2);
	m_wndSubmitWebcacheLink2.SetBkColor(::GetSysColor(COLOR_3DFACE)); // still not the right color, will fix this later (need to merge the .rc file before it changes ;) )
	m_wndSubmitWebcacheLink2.SetFont(GetFont());
	
	if (!bCreated2){
		bCreated2 = true;
		CString URL = _T("http://ispcachingforemule.de.vu/index.php?show=submitProxy");
		CString proxyName, proxyPort, hostName;
		proxyName.Format(_T("%s"), thePrefs.webcacheName);
		proxyPort.Format(_T("%i"), thePrefs.webcachePort);
		hostName.Format(_T("%s"), thePrefs.GetLastResolvedName());

		URL += _T("&hostName=") + hostName + _T("&proxyName=") + proxyName + _T("&proxyPort=") + proxyPort;

		m_wndSubmitWebcacheLink2.AppendText(_T("Link: "));
		m_wndSubmitWebcacheLink2.AppendHyperLink(GetResString(IDS_WC_SENDBYWEB),0,URL,0,0);
	}


	m_hideControls.CreateEx(0, _T("BUTTON"), _T(""), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_FLAT,
									CRect(right+10, bottom-40, right+290, bottom-20), this, IDC_ADVANCEDCONTROLS);
	m_hideControls.SetFont(GetFont());



}


void CPPgPhoenix2::OnTabSelectionChange(NMHDR *pNMHDR, LRESULT *pResult)
{
	// Retrieve tab to display
	TCITEM tabCtrlItem; 
	tabCtrlItem.mask = TCIF_PARAM;
	if(m_tabCtr.GetItem(m_tabCtr.GetCurSel(), &tabCtrlItem) == TRUE){
		SetTab(static_cast<eTab>(tabCtrlItem.lParam));
	}

	*pResult = 0;
}


void CPPgPhoenix2::SetTab(eTab tab)
{
	if(m_currentTab != tab)
	{
		// Hide all control
		switch(m_currentTab)
		{
			case WEBCACHE:
				m_webcacheAddressEdit.ShowWindow(SW_HIDE);
				m_webcacheAddressStatic.ShowWindow(SW_HIDE);
				m_webcachePortEdit.ShowWindow(SW_HIDE);
				m_webcachePortStatic.ShowWindow(SW_HIDE);
				m_webcacheAutodetect.ShowWindow(SW_HIDE);
				m_webcacheBlocksStatic.ShowWindow(SW_HIDE);
				m_webcacheBlocksSize.ShowWindow(SW_HIDE);
				m_webcacheBlocksEdit.ShowWindow(SW_HIDE);
				m_webcacheTimeout.ShowWindow(SW_HIDE);
				m_webcacheNotSameISP.ShowWindow(SW_HIDE);
				m_webcachePersistent.ShowWindow(SW_HIDE);
				m_webcacheUpdateAuto.ShowWindow(SW_HIDE);
				m_hideControls.ShowWindow(SW_HIDE);
				m_webcacheBox.ShowWindow(SW_HIDE);
				m_enableWebcache.ShowWindow(SW_HIDE);
				m_enableWebcache.EnableWindow(false);
				m_wndSubmitWebcacheLink2.ShowWindow(SW_HIDE);
				m_wndSubmitWebcacheLink2.EnableWindow(false);
				m_testProxy.ShowWindow(SW_HIDE);
				m_testProxy.EnableWindow(false);
				EnableWebcacheControls(false);
				break;
		}

		// Show new controls
		m_currentTab = tab;
		switch(m_currentTab)
		{
			case WEBCACHE:
				m_webcacheAddressEdit.ShowWindow(SW_SHOW);
				m_webcacheAddressStatic.ShowWindow(SW_SHOW);
				m_webcachePortEdit.ShowWindow(SW_SHOW);
				m_webcachePortStatic.ShowWindow(SW_SHOW);
				m_hideControls.ShowWindow(SW_SHOW);
				m_webcacheBox.ShowWindow(SW_SHOW);
				m_enableWebcache.ShowWindow(SW_SHOW);
				m_enableWebcache.EnableWindow(true);
				m_wndSubmitWebcacheLink2.ShowWindow(SW_SHOW);
				m_wndSubmitWebcacheLink2.EnableWindow(true);
				m_testProxy.ShowWindow(SW_SHOW);
				m_testProxy.EnableWindow(true);
				EnableWebcacheControls(m_enableWebcache.GetCheck() == BST_CHECKED);
				if(showadvanced)
				{
					m_webcacheBlocksStatic.ShowWindow(SW_SHOW);
					m_webcacheBlocksEdit.ShowWindow(SW_SHOW);
					m_webcacheBlocksSize.ShowWindow(SW_SHOW);
					m_webcacheTimeout.ShowWindow(SW_SHOW);
					m_webcacheNotSameISP.ShowWindow(SW_SHOW);
					m_webcachePersistent.ShowWindow(SW_SHOW);
					m_webcacheUpdateAuto.ShowWindow(SW_SHOW);
					m_webcacheAutodetect.ShowWindow(SW_SHOW);
				}
				else
				{
					m_webcacheBlocksStatic.ShowWindow(SW_HIDE);
					m_webcacheBlocksEdit.ShowWindow(SW_HIDE);
					m_webcacheBlocksSize.ShowWindow(SW_HIDE);
					m_webcacheTimeout.ShowWindow(SW_HIDE);
					m_webcacheNotSameISP.ShowWindow(SW_HIDE);
					m_webcachePersistent.ShowWindow(SW_HIDE);
					m_webcacheUpdateAuto.ShowWindow(SW_HIDE);
					m_webcacheAutodetect.ShowWindow(SW_HIDE);
				}
				break;			


		}
	}
}

void CPPgPhoenix2::OnEnChangeActivatewebcachedownloads()
{
		if (guardian) 
			return;

		guardian=true;

		SetModified();

		bool enableWC = m_enableWebcache.GetCheck() == BST_CHECKED;
		m_webcacheAddressStatic.EnableWindow(enableWC);
		m_webcacheAddressEdit.EnableWindow(enableWC);
		m_webcachePortStatic.EnableWindow(enableWC);
		m_webcachePortEdit.EnableWindow(enableWC);
		m_webcacheBlocksStatic.EnableWindow(enableWC);
		m_webcacheBlocksEdit.EnableWindow(enableWC);
		m_webcacheBlocksSize.EnableWindow(enableWC);
		m_webcacheTimeout.EnableWindow(enableWC);
		m_webcacheNotSameISP.EnableWindow(enableWC);
		m_webcachePersistent.EnableWindow(enableWC);
		m_webcacheUpdateAuto.EnableWindow(enableWC);
		m_webcacheAutodetect.EnableWindow(enableWC);
		m_hideControls.EnableWindow(enableWC);

		guardian=false;
}

void CPPgPhoenix2::OnBnClickedDetectWebCache()
{
	WCInfo_Struct* detectedWebcache = new WCInfo_Struct();
	//int pos=0;
	bool reaskedDNS;	// tells if a DNS reverse lookup has been performed during detection

	try
	{
		reaskedDNS=DetectWebCache(detectedWebcache);
	}
	catch(CString strError)
	{
		delete detectedWebcache;
		AfxMessageBox(strError ,MB_OK | MB_ICONINFORMATION,0);
		return;
	}
	catch (...)
	{
		delete detectedWebcache;
		AfxMessageBox(_T("Autodetection failed") ,MB_OK | MB_ICONINFORMATION,0);
		return;
	}

	CString comment = detectedWebcache->comment;
	for (int i=1; i*45 < comment.GetLength(); i++) // some quick-n-dirty beautifying  
		comment = comment.Left(i*45) + _T(" \n\t\t\t") + comment.Right(comment.GetLength() - i*45);

	CString message =	_T("Your ISP is:\t\t") + detectedWebcache->isp + _T(", ") + detectedWebcache->country + _T("\n") +
		_T("Your proxy name is:\t") + detectedWebcache->webcache + _T("\n") +
		_T("The proxy port is:\t\t") + detectedWebcache->port + _T("\n") +
		(comment != _T("") ? _T("comment: \t\t") + comment : _T(""));
	if (detectedWebcache->active == "0")
		message += _T("\n\ndue to detection results, webcache downloading has been deactivated;\nsee the comment for more details");

	if (AfxMessageBox(message, MB_OKCANCEL | MB_ICONINFORMATION,0) == IDCANCEL)
	{
		delete detectedWebcache;
		return;
	}

	m_enableWebcache.SetCheck(detectedWebcache->active == "1" ? BST_CHECKED : BST_UNCHECKED);
	m_webcacheAddressEdit.SetWindowText(detectedWebcache->webcache);
	m_webcachePortEdit.SetWindowText(detectedWebcache->port);
	m_webcacheBlocksSize.SetWindowText(detectedWebcache->blockLimit);
	m_webcacheTimeout.SetCheck(detectedWebcache->extraTimeout == "1" ? BST_CHECKED : BST_UNCHECKED);
	m_webcacheNotSameISP.SetCheck(detectedWebcache->cachesLocal == "0" ? BST_CHECKED : BST_UNCHECKED);
	m_webcachePersistent.SetCheck(detectedWebcache->persistentconns == "1" ? BST_CHECKED : BST_UNCHECKED);

	delete detectedWebcache;
}



void CPPgPhoenix2::OnBnClickedAdvancedcontrols()
{
	if (showadvanced == false)
	{
		m_webcacheBlocksStatic.ShowWindow(SW_SHOW);
		m_webcacheBlocksEdit.ShowWindow(SW_SHOW);
		m_webcacheBlocksSize.ShowWindow(SW_SHOW);
		m_webcacheTimeout.ShowWindow(SW_SHOW);
		m_webcacheNotSameISP.ShowWindow(SW_SHOW);
		m_webcachePersistent.ShowWindow(SW_SHOW);
		m_webcacheUpdateAuto.ShowWindow(SW_SHOW);
		m_webcacheAutodetect.ShowWindow(SW_SHOW);
		m_hideControls.SetWindowText(GetResString(IDS_WC_HIDECONTROLS));	
		showadvanced = true;

	}
	else
	{
		m_webcacheBlocksStatic.ShowWindow(SW_HIDE);
		m_webcacheBlocksEdit.ShowWindow(SW_HIDE);
		m_webcacheBlocksSize.ShowWindow(SW_HIDE);
		m_webcacheTimeout.ShowWindow(SW_HIDE);
		m_webcacheNotSameISP.ShowWindow(SW_HIDE);
		m_webcachePersistent.ShowWindow(SW_HIDE);
		m_webcacheUpdateAuto.ShowWindow(SW_HIDE);
		m_webcacheAutodetect.ShowWindow(SW_HIDE);
		m_hideControls.SetWindowText(GetResString(IDS_WC_SHOWCONTROLS));
		showadvanced = false;
	}
}

void CPPgPhoenix2::EnableWebcacheControls(bool active)
{
			m_webcacheAddressEdit.EnableWindow(active);
			m_webcacheAddressStatic.EnableWindow(active);
			m_webcachePortEdit.EnableWindow(active);
			m_webcachePortStatic.EnableWindow(active);
			m_webcacheAutodetect.EnableWindow(active);
			m_webcacheBlocksStatic.EnableWindow(active);
			m_webcacheBlocksSize.EnableWindow(active);
			m_webcacheBlocksEdit.EnableWindow(active);
			m_webcacheTimeout.EnableWindow(active);
			m_webcacheNotSameISP.EnableWindow(active);
			m_webcachePersistent.EnableWindow(active);
			m_webcacheUpdateAuto.EnableWindow(active);
			m_hideControls.EnableWindow(active);
			m_webcacheBox.EnableWindow(active);
}

//JP proxy configuration test
void CPPgPhoenix2::OnBnClickedTestProxy()
{
	if (thePrefs.IsWebCacheTestPossible())
	{
		if (!thePrefs.expectingWebCachePing)
		{
			//get webcache name from IDC_webcacheName
			CString cur_WebCacheName;
			GetDlgItem(IDC_webcacheName)->GetWindowText(cur_WebCacheName);
			if (cur_WebCacheName.GetLength() > 15 && cur_WebCacheName.Left(12) == "transparent@") //doesn't work for transparent proxies
			{
				AfxMessageBox(GetResString(IDS_WC_NOTRANS));
				return;
			}
			//get webcache port from IDC_webcachePort
			CString buffer;			
			GetDlgItem(IDC_webcachePort)->GetWindowText(buffer);
			uint16 cur_WebCachePort = (uint16)_tstol(buffer);
			if (PingviaProxy(cur_WebCacheName, cur_WebCachePort))
			{
				thePrefs.WebCachePingSendTime = ::GetTickCount();
				thePrefs.expectingWebCachePing = true;
				AfxMessageBox(GetResString(IDS_WC_PERFTEST));
			}
			else
				AfxMessageBox(GetResString(IDS_WC_PROXYERROR));
		}
		else 
			AfxMessageBox(GetResString(IDS_WC_TESTINPROGRESS));
	}
	else
		AfxMessageBox(GetResString(IDS_WC_REQUERIMENTS));
}

BOOL CPPgPhoenix2::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

void CPPgPhoenix2::OnHelp()
{
	//theApp.ShowHelp(eMule_FAQ_Preferences_Extended_Settings);
}

BOOL CPPgPhoenix2::OnHelpInfo(HELPINFO* pHelpInfo)
{
	OnHelp();
	return TRUE;
}