// PgTweaks.cpp : implementation file
//

#include "stdafx.h"
#include "emule.h"
#include "PPgPhoenix.h"
#include "preferences.h"
#include "PreferencesDlg.h"
#include "HttpDownloadDlg.h"
#include "BandwidthControl.h"
#include "EmuleDlg.h"
#include "SharedFilesWnd.h"
#include "TransferWnd.h"
#include "statisticsdlg.h"
#include "otherfunctions.h"

#include "HelpIDs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CPPgPhoenix dialog

IMPLEMENT_DYNAMIC(CPPgPhoenix, CPropertyPage) // [TPT] - SLUGFILLER: modelessDialogs
CPPgPhoenix::CPPgPhoenix()
	: CPropertyPage(CPPgPhoenix::IDD) // [TPT] - SLUGFILLER: modelessDialogs
{	
	// [TPT] - Select process priority 
	switch (thePrefs.GetWindowsVersion())
	{
		case _WINVER_98_:
		case _WINVER_95_:	
		case _WINVER_ME_:
			ExtraPrios = false;
			break;
		default: 
			ExtraPrios = true;
	}
	// [TPT] - Select process priority 
}

CPPgPhoenix::~CPPgPhoenix()
{
}

void CPPgPhoenix::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PPG_PHOENIX_TAB, m_tabCtr);
}

BEGIN_MESSAGE_MAP(CPPgPhoenix, CPropertyPage) // [TPT] - SLUGFILLER: modelessDialogs
	// TAB control
	ON_NOTIFY(TCN_SELCHANGE, IDC_PPG_PHOENIX_TAB, OnTabSelectionChange)

	//VISUAL --------------------------------------------------------------
	ON_BN_CLICKED(IDC_PPG_PHOENIX_SHOWFILESYSTEMICON, OnSettingsChange)   // [TPT] - itsonlyme: displayOptions
	ON_BN_CLICKED(IDC_PPG_PHOENIX_SHOWLOCALRATING, OnSettingsChange)    	// [TPT] - itsonlyme: displayOptions
	ON_BN_CLICKED(IDC_PPG_PHOENIX_HIGHCONTRAST, OnSettingsChange)    	// [TPT] - MoNKi: -Support for High Contrast Mode-
	ON_BN_CLICKED(IDC_PPG_PHOENIX_BOLD, OnSettingsChange)
	// [TPT] - IP Country
	ON_BN_CLICKED(IDC_PPG_PHOENIX_SHOW_COUNTRYNAMES, OnSettingsChange)    	
	ON_BN_CLICKED(IDC_PPG_PHOENIX_SHOW_COUNTRYFLAGS, OnSettingsChange)    	
	// [TPT] - IP Country
	ON_BN_CLICKED(IDC_PPG_PHOENIX_SHOWUPPRIO, OnSettingsChange)
	// emulEspaña: added by [TPT]-MoNKi [MoNKi: -invisible mode-]
	ON_CBN_SELCHANGE(IDC_PPG_PHOENIX_INVISIBLE_MODE_SELECT_COMBO, OnSettingsChange)
	ON_CBN_SELCHANGE(IDC_PPG_PHOENIX_INVISIBLE_MODE_KEY_COMBO, OnCbnSelchangeKeymodcombo)
	ON_BN_CLICKED(IDC_PPG_PHOENIX_INVISIBLE_MODE, OnBnClickedInvisiblemode)
	// End [TPT]-MoNKi

	ON_BN_CLICKED(IDC_PHOENIX_SHOWBACKGROUNDSINMENUS, OnSettingsChange)


	//ARCHIVOS ------------------------------------------------------------
	// Maella -One-queue-per-file- (idea bloodymad)
	ON_BN_CLICKED(IDC_PPG_MAELLA_MULTI_QUEUE_CHECK01, OnMultiQueueChange) 
	ON_BN_CLICKED(IDC_PPG_MAELLA_MULTI_QUEUE_CHECK02, OnSettingsChange) 
	
	ON_EN_CHANGE(IDC_PPG_PHOENIX_HIDEOS, OnSettingsChange) // [TPT] - SLUGFILLER: hideOS
	ON_BN_CLICKED(IDC_PPG_PHOENIX_SELECTIVESHARE, OnSettingsChange) // [TPT] - SLUGFILLER: hideOS

	// [TPT] - Powershare
	ON_EN_CHANGE(IDC_PPG_POWERSHARELIMITEDIT, OnSettingsChange) // [TPT] - SLUGFILLER: hideOS


	//MISC ----------------------------------------------------------------
	ON_BN_CLICKED(IDC_PPG_PHOENIX_SHOWSPREADBARS, OnSettingsChange)    	 // [TPT] - SLUGFILLER: Spreadbars	
	ON_WM_VSCROLL() //Para las prioridades y seleccion de creditos y powershare
	ON_BN_CLICKED(IDC_PPG_PHOENIX_SHOWINMSN7, OnSettingsChange)


	//CATEGORIAS ----------------------------------------------------------
	// [TPT] - khaos::categorymod+
	ON_BN_CLICKED(IDC_PPG_PHOENIX_SHOW_CATNAMES, OnSettingsChange)    	
	ON_BN_CLICKED(IDC_PPG_PHOENIX_SELECT_CAT, OnSettingsChange)    	
	ON_BN_CLICKED(IDC_PPG_PHOENIX_USE_ACTIVECAT, OnSettingsChange)    	
	ON_BN_CLICKED(IDC_PPG_PHOENIX_AUTOSETRESORDER, OnSettingsChange)    	
	ON_EN_CHANGE(IDC_PPG_PHOENIX_RESUMEFILEINNEWCAT, OnSettingsChange)    	
	ON_BN_CLICKED(IDC_PPG_PHOENIX_USEAUTOCAT, OnSettingsChange)    	
	ON_BN_CLICKED(IDC_PPG_PHOENIX_SHOWPRIORITYINTAB, OnSettingsChange)
	// [TPT] - khaos::categorymod-	
	//ESTADISTICAS --------------------------------------------------------
	ON_BN_CLICKED(IDC_PPG_MEMORYCONSUMING, OnSettingsChange)// [TPT] - Memory Consuming
	ON_BN_CLICKED(IDC_PPG_NAFCGRAPH, OnSettingsChange)
	ON_BN_CLICKED(IDC_OVERHEADGRAPH, OnSettingsChange)
	ON_BN_CLICKED(IDC_VERTICALLINES, OnSettingsChange)
	//MINIMULE ------------------------------------------------------------
	// [TPT] - TBH: minimule
	ON_BN_CLICKED(IDC_PPG_PHOENIX_MINIMULE, OnSettingsChange)    	
	ON_BN_CLICKED(IDC_PPG_PHOENIX_MINIMULELIVES, OnSettingsChange)    	
	ON_EN_CHANGE(IDC_PPG_PHOENIX_MINIMULEUPDATE, OnSettingsChange)    	
	// [TPT] - TBH: minimule

	ON_WM_HSCROLL()//para el minimule y la config de estadisticas

	ON_WM_HELPINFO()
END_MESSAGE_MAP()

// CPPgPhoenix message handlers

BOOL CPPgPhoenix::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// Init the Tab control
	InitTab();

	// Create and Init all controls
	InitControl();
	// [TPT] - Tooltips in preferences
	m_Tip2.Create(this);
	m_Tip2.SetEffectBk(CPPToolTip::PPTOOLTIP_EFFECT_HGRADIENT);
	m_Tip2.SetGradientColors(RGB(255,255,225),RGB(0,0,0), RGB(255,198,167));
	// [TPT] - Tooltips in preferences end
	// Set default tab
	m_currentTab = NONE;
	m_tabCtr.SetCurSel(0);
	SetTab(VISUALIZATION);

	// added by [TPT]-MoNKi [MoNKi: -invisible mode-]
	// Add keys to ComboBox
	for(int i='A'; i<='Z'; i++)
		m_invmode_keys.AddString(CString((TCHAR)(i)));
	for(int i='0'; i<='9'; i++)
		m_invmode_keys.AddString(CString((TCHAR)(i)));
	// End [TPT]-MoNKi

	// Load setting
	LoadSettings();
	Localize();
	UpdatepHoeniXReloadTooltips();// [TPT] - Tooltips in preferences
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


// [TPT] - Tooltips in preferences
BOOL CPPgPhoenix::PreTranslateMessage(MSG* pMsg) 
{
	m_Tip2.RelayEvent(pMsg);

	return CDialog::PreTranslateMessage(pMsg);
}

void CPPgPhoenix::UpdatepHoeniXReloadTooltips()
{

	//Visual
	m_Tip2.AddTool(GetDlgItem(IDC_PPG_PHOENIX_SHOWLOCALRATING), GetResString(IDS_TOOLTIP_COMMENTEDRATED));
	m_Tip2.AddTool(GetDlgItem(IDC_PPG_PHOENIX_HIGHCONTRAST), GetResString(IDS_TOOLTIP_HIGHCONTRAST));
	m_Tip2.AddTool(GetDlgItem(IDC_PPG_PHOENIX_INVISIBLE_MODE), GetResString(IDS_TOOLTIP_INVISIBLEMODE));

	//Files
	m_Tip2.AddTool(GetDlgItem(IDC_PPG_MAELLA_MULTI_QUEUE_CHECK01), GetResString(IDS_TOOLTIP_MULTIQUEUE));
	m_Tip2.AddTool(GetDlgItem(IDC_PPG_MAELLA_MULTI_QUEUE_CHECK02), GetResString(IDS_TOOLTIP_SLOTRELEASE));
	m_Tip2.AddTool(GetDlgItem(IDC_PPG_PHOENIX_HIDEOS), GetResString(IDS_TOOLTIP_OVERSHARES));
	m_Tip2.AddTool(GetDlgItem(IDC_PPG_PHOENIX_SELECTIVESHARE), GetResString(IDS_TOOLTIP_SMART));

	//Misc
	m_Tip2.AddTool(GetDlgItem(IDC_PPG_PHOENIX_SHOWSPREADBARS), GetResString(IDS_TOOLTIP_SHOWSPREAD));
}

// [TPT] - Tooltips in preferences END

void CPPgPhoenix::LoadSettings()
{

	//VISUAL --------------------------------------------------------------
	// [TPT] - itsonlyme: displayOptions START	
	m_iShowFileSystemIcon.SetCheck(thePrefs.ShowFileSystemIcon());
	m_iShowLocalRating.SetCheck(thePrefs.ShowLocalRating()); // [TPT] - SLUGFILLER: showComments
	// [TPT] - itsonlyme: displayOptions END
	// Added by [TPT]-MoNKi [MoNKi: -invisible mode-]
	m_iActualKeyModifier = thePrefs.GetInvisibleModeHKKeyModifier();
	m_invmode_keys.SelectString(-1, CString(thePrefs.GetInvisibleModeHKKey()));
	if (!thePrefs.GetInvisibleMode()){
		m_iInvisibleModeSelectStatic.EnableWindow(FALSE);
		m_iInvisibleModeModifierStatic.EnableWindow(FALSE);
		m_iInvisibleModeKeyStatic.EnableWindow(FALSE);
		m_iInvisibleModePlus.EnableWindow(FALSE);
		m_invmode_keymodifiers.EnableWindow(FALSE);
		m_invmode_keys.EnableWindow(FALSE);
		CheckDlgButton(IDC_PPG_PHOENIX_INVISIBLE_MODE, 0);
	} else 
		CheckDlgButton(IDC_PPG_PHOENIX_INVISIBLE_MODE, 1);
	// End [TPT]-MoNKi
	// [TPT] - IP Country
	m_iEnableShowCountryNames.SetCheck(thePrefs.GetEnableShowCountryNames());
	m_iEnableShowCountryFlags.SetCheck(thePrefs.GetEnableShowCountryFlags());
	// [TPT] - IP Country
	m_iHighContrast.SetCheck(thePrefs.WindowsTextColorOnHighContrast()); // [TPT] - MoNKi: -Support for High Contrast Mode-
	m_iBold.SetCheck(thePrefs.GetShowActiveDownloadsBold());

	m_iShowBackgroundInMenus.SetCheck(thePrefs.GetShowBitmapInMenus());

	m_iShowUpPrio.SetCheck(thePrefs.GetShowUpPrioInDownloadList());
	//ARCHIVOS ------------------------------------------------------------
	// [TPT] - SLUGFILLER: hideOS
	{
		CString buffer;
		buffer.Format(_T("%u"), thePrefs.GetHideOvershares());
		m_iHideOS.SetWindowText(buffer);
	}	
	m_iSelectiveShare.SetCheck(thePrefs.IsSelectiveShareEnabled());
	// [TPT] - SLUGFILLER: hideOS
	
	// Maella -One-queue-per-file- (idea bloodymad)
	m_multiQueueCheck01.SetCheck((thePrefs.GetEnableMultiQueue() == true) ? BST_CHECKED : BST_UNCHECKED);
	m_multiQueueCheck02.EnableWindow(m_multiQueueCheck01.GetCheck() != BST_UNCHECKED);
	m_multiQueueCheck02.SetCheck((thePrefs.GetEnableReleaseMultiQueue() == true) ? BST_CHECKED : BST_UNCHECKED);

	// [TPT] - Powershare
	m_iPowershare.SetPos(thePrefs.GetPowerShareMode());
	CString buffer2;
	buffer2.Format(_T("%u"), thePrefs.GetPowerShareLimit());
	m_powersharelimitedit.SetWindowText(buffer2);

	//MISC ----------------------------------------------------------------
	m_iSpreadBars.SetCheck(thePrefs.IsSpreadBarsEnable()); // [TPT] - SLUGFILLER: Spreadbars
	m_iShowInMSN7.SetCheck(thePrefs.GetShowMSN7());

	// [TPT] - Select process priority 
	if (!ExtraPrios)
		switch (thePrefs.GetMainProcessPriority())
			{
				case ABOVE_NORMAL_PRIORITY_CLASS: m_selectPriorityProcess.SetPos(0); break;
				case BELOW_NORMAL_PRIORITY_CLASS: m_selectPriorityProcess.SetPos(2); break;
				default: m_selectPriorityProcess.SetPos(1);
			}
		else
			for (int i = 0; i < PROCESSPRIORITYNUMBER; i++)
			{
				if (PriorityClasses[i] == thePrefs.GetMainProcessPriority())
				{
					m_selectPriorityProcess.SetPos(i);
					break;
				}
			}			
	// [TPT] - Select process priority 
	
	// [TPT] - Credit System
	m_iCredits.SetPos(thePrefs.GetCreditSystem());


	//CATEGORIAS ----------------------------------------------------------
	// [TPT] - khaos::categorymod+
	m_iShowCatNames.SetCheck(thePrefs.ShowCatNameInDownList());
	m_iSelectCat.SetCheck(thePrefs.SelectCatForNewDL());
	m_iUseActiveCat.SetCheck(thePrefs.UseActiveCatForLinks());
	m_iAutoSetResOrder.SetCheck(thePrefs.AutoSetResumeOrder());		
	{
		CString buffer;
		buffer.Format(_T("%u"), thePrefs.StartDLInEmptyCats());
		m_iResumeFileInNewCat.SetWindowText(buffer);
	}	
	m_iUseAutoCat.SetCheck(thePrefs.UseAutoCat());
	m_iShowPriorityInTab.SetCheck(thePrefs.ShowPriorityInTab());
	// [TPT] - khaos::categorymod-
	

	//ESTADISTICAS --------------------------------------------------------
	// Maella -Graph: display zoom-
	m_zoomSlider.SetPos(thePrefs.GetZoomFactor());
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	m_sampleRateSlider.SetPos(thePrefs.GetDatarateSamples());
	
	m_memoryConsuming.SetCheck(thePrefs.GetMemoryConsumingGraph());// [TPT] - Memory consuming
	m_NAFCGraph.SetCheck(thePrefs.GetNAFCGraph());
	m_OverheadGraph.SetCheck(thePrefs.GetOverheadGraph());
	m_showVerticalLine.SetCheck(thePrefs.m_bShowVerticalHourMarkers);

	//MINIMULE ------------------------------------------------------------
	// [TPT] - TBH: minimule
	m_iMiniMule.SetCheck(thePrefs.IsMiniMuleEnabled());
	m_iMiniMuleLives.SetCheck(thePrefs.GetMiniMuleLives());
	{
		CString buffer;
		buffer.Format(_T("%u"), thePrefs.GetMiniMuleUpdate());
		m_iMiniMuleUpdate.SetWindowText(buffer);
	}	
	m_MiniMuleTransparency.SetPos(thePrefs.GetMiniMuleTransparency());
	// [TPT] - TBH: minimule

	
}

BOOL CPPgPhoenix::OnApply()
{	

	//VISUAL --------------------------------------------------------------
	thePrefs.SetWindowsTextColorOnHighContrast(m_iHighContrast.GetCheck()); // [TPT] - MoNKi: -Support for High Contrast Mode-
	// [TPT] - itsonlyme: displayOptions START
	thePrefs.showFileSystemIcon = m_iShowFileSystemIcon.GetCheck();
	thePrefs.showLocalRating = m_iShowLocalRating.GetCheck();	// [TPT] - SLUGFILLER: showComments
	// [TPT] - itsonlyme: displayOptions END
	// [TPT] - itsonlyme: displayOptions
	theApp.emuledlg->sharedfileswnd->Invalidate(false);
	theApp.emuledlg->sharedfileswnd->UpdateWindow();
	theApp.emuledlg->transferwnd->Invalidate(false);
	theApp.emuledlg->transferwnd->UpdateWindow();
	// [TPT] - itsonlyme: displayOptions
	// Added by [TPT]-MoNKi [MoNKi: -invisible mode-]
	CString sKey;
	int nIndex = m_invmode_keys.GetCurSel();
	if (nIndex != LB_ERR)
	{
		m_invmode_keys.GetLBText(m_invmode_keys.GetCurSel(), sKey);
		if (IsDlgButtonChecked(IDC_PPG_PHOENIX_INVISIBLE_MODE))
			thePrefs.SetInvisibleMode(true,m_iActualKeyModifier,sKey[0]);
		else
			thePrefs.SetInvisibleMode(false,m_iActualKeyModifier,sKey[0]);
	}
	// End [TPT]-MoNKi
	// [TPT] - IP Country

	if((m_iEnableShowCountryNames.GetCheck() == BST_UNCHECKED) 
		&& (m_iEnableShowCountryFlags.GetCheck() == BST_UNCHECKED))
	{
		if(thePrefs.GetEnableShowCountryNames()){
		theApp.emuledlg->transferwnd->uploadlistctrl.HideIPCountryColumn(true);
		theApp.emuledlg->transferwnd->queuelistctrl.HideIPCountryColumn(true);
		}
	}

	thePrefs.SetEnableShowCountryFlags(m_iEnableShowCountryFlags.GetCheck());
	thePrefs.SetEnableShowCountrNames(m_iEnableShowCountryNames.GetCheck());
	// [TPT] - IP Country
	thePrefs.m_bShowActiveDownloadsBold = m_iBold.GetCheck();

	thePrefs.SetShowBitmapInMenus(m_iShowBackgroundInMenus.GetCheck() == BST_CHECKED);

	thePrefs.SetShowUpPrioInDownloadList(m_iShowUpPrio.GetCheck() == BST_CHECKED);

	//ARCHIVOS ------------------------------------------------------------
	// [TPT] - SLUGFILLER: hideOS	
	{
		CString buffer;
		m_iHideOS.GetWindowText(buffer);
		int hideOS = _tstoi(buffer);
		if(hideOS > 5) hideOS = 5;
		if(hideOS < 0) hideOS = 0;
		thePrefs.hideOS = hideOS;		
	}
	thePrefs.selectiveShare = m_iSelectiveShare.GetCheck();
	// [TPT] - SLUGFILLER: hideOS

	// Maella -One-queue-per-file- (idea bloodymad)
	thePrefs.SetEnableMultiQueue(m_multiQueueCheck01.GetCheck() != BST_UNCHECKED);
	thePrefs.SetEnableReleaseMultiQueue(m_multiQueueCheck02.GetCheck() != BST_UNCHECKED);

	// [TPT] - Powershare
	thePrefs.m_iPowershareMode = m_iPowershare.GetPos();
	CString buffer2;
	m_powersharelimitedit.GetWindowText(buffer2);
	int limite = _tstoi(buffer2);
	if(limite < 0) limite = 0;
	thePrefs.PowerShareLimit = limite;
	// [TPT] - Powershare END
	//MISC ----------------------------------------------------------------
	
	thePrefs.m_bSpreadBars = m_iSpreadBars.GetCheck(); // [TPT] - SLUGFILLER: Spreadbars

	bool killMSN = (thePrefs.GetShowMSN7() && (m_iShowInMSN7.GetCheck() != BST_CHECKED));
	thePrefs.SetShowMSN7(m_iShowInMSN7.GetCheck() == BST_CHECKED);

	// [TPT] - Select process priority 
	if (!ExtraPrios)
		switch (m_selectPriorityProcess.GetPos())
		{
			case 0: thePrefs.m_MainProcessPriority = HIGH_PRIORITY_CLASS; break;			
			case 2: thePrefs.m_MainProcessPriority = IDLE_PRIORITY_CLASS; break;
			default: thePrefs.m_MainProcessPriority = NORMAL_PRIORITY_CLASS;
		}
	else
		thePrefs.m_MainProcessPriority = PriorityClasses[m_selectPriorityProcess.GetPos()];	
	// [TPT] - Select process priority 
	// [TPT] - Credit System
	thePrefs.SetCreditSystem(m_iCredits.GetPos());

	
	//CATEGORIAS ----------------------------------------------------------
	// [TPT] - khaos::categorymod+
	thePrefs.m_bShowCatNames = m_iShowCatNames.GetCheck();
	thePrefs.m_bSelCatOnAdd = m_iSelectCat.GetCheck();
	thePrefs.m_bActiveCatDefault = m_iUseActiveCat.GetCheck();
	thePrefs.m_bAutoSetResumeOrder = m_iAutoSetResOrder.GetCheck();		
	{
		CString buffer;
		m_iResumeFileInNewCat.GetWindowText(buffer);
		int resumeFiles = _tstoi(buffer);		
		if(resumeFiles < 0) resumeFiles = 0;
		thePrefs.m_iStartDLInEmptyCats = resumeFiles;		
	}	
	thePrefs.m_bUseAutoCat = m_iUseAutoCat.GetCheck();
	thePrefs.m_bShowPriorityInTab = m_iShowPriorityInTab.GetCheck();
	// [TPT] - khaos::categorymod-
	

	//ESTADISTICAS --------------------------------------------------------
	// Maella -Graph: display zoom-
	bool redrawMeter = (thePrefs.GetZoomFactor() != m_zoomSlider.GetPos());
	thePrefs.SetZoomFactor(m_zoomSlider.GetPos());

	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	thePrefs.SetDatarateSamples(m_sampleRateSlider.GetPos());

	thePrefs.SetMemoryConsumingGraph(m_memoryConsuming.GetCheck());// [TPT] - Memory consuming
	thePrefs.SetNAFCGraph(m_NAFCGraph.GetCheck());
	thePrefs.SetOverheadGraph(m_OverheadGraph.GetCheck());
	bool invalidateGraph = false;
	if(thePrefs.m_bShowVerticalHourMarkers == (m_showVerticalLine.GetCheck() == BST_UNCHECKED) ||
		!thePrefs.m_bShowVerticalHourMarkers == (m_showVerticalLine.GetCheck() == BST_CHECKED))
		invalidateGraph = true;
	thePrefs.m_bShowVerticalHourMarkers = m_showVerticalLine.GetCheck();
	//MINIMULE ------------------------------------------------------------
	// [TPT] - TBH: minimule
	thePrefs.SetMiniMuleEnabled(m_iMiniMule.GetCheck());
	thePrefs.SetMiniMuleLives(m_iMiniMuleLives.GetCheck());	
	{
		CString buffer;
		m_iMiniMuleUpdate.GetWindowText(buffer);
		int update = _tstoi(buffer);		
		if(update < 0) update = 0;
		thePrefs.SetMiniMuleUpdate(update);		
	}	
	thePrefs.SetMiniMuleTransparency(m_MiniMuleTransparency.GetPos());
	// [TPT] - TBH: minimule


	// Refresh Setting
	LoadSettings();
	SetModified(FALSE);	

	if(invalidateGraph){
		theApp.emuledlg->statisticswnd->m_DownloadOMeter.InvalidateCtrl();
		theApp.emuledlg->statisticswnd->m_UploadOMeter.InvalidateCtrl();
		theApp.emuledlg->statisticswnd->m_Statistics.InvalidateCtrl();
	}

	if(redrawMeter){
		theApp.emuledlg->statisticswnd->ShowInterval();
		theApp.emuledlg->statisticswnd->RepaintMeters();
		theApp.emuledlg->statisticswnd->GetDlgItem(IDC_STATTREE)->EnableWindow(thePrefs.GetStatsInterval()>0);
	}

	if(killMSN)
		UpdateMSN(0,0,0,0,true);
	
	return CPropertyPage::OnApply();
}

void CPPgPhoenix::Localize(void)
{	
	if(m_hWnd)
	{
		// Create an icon list for the tab control
		m_ImageList.DeleteImageList();
		m_ImageList.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
		m_ImageList.SetBkColor(CLR_NONE);
		m_ImageList.Add(CTempIconLoader(_T("DISPLAY")));//imagen visualizacion
		m_ImageList.Add(CTempIconLoader(_T("category")));//imagen archivos
		m_ImageList.Add(CTempIconLoader(_T("misc")));//imagen miscelaneo
		m_ImageList.Add(CTempIconLoader(_T("categories")));//imagen categorias
		m_ImageList.Add(CTempIconLoader(_T("PRIORITY")));//imagen estadisticas
		m_ImageList.Add(CTempIconLoader(_T("clientcompatible")));//imagen minimule
		CString Buffer;
		SetWindowText(_T("pHoeniX RelOad"));		

		int row = m_tabCtr.GetRowCount();
		InitTab(); // To update string, could be improved
		if(row != 0 && row != m_tabCtr.GetRowCount()){
			// Shift all windows object
			// .. to do
		}
	
	//VISUAL --------------------------------------------------------------
		// [TPT] - itsonlyme: displayOptions START	
		m_iShowFileSystemIcon.SetWindowText(GetResString(IDS_SHOWFILESYSICON));
		m_iShowLocalRating.SetWindowText(GetResString(IDS_SHOWLOCALRATING)); // [TPT] - SLUGFILLER: showComments
		// [TPT] - itsonlyme: displayOptions END
		m_iHighContrast.SetWindowText(GetResString(IDS_HIGH_CONTRAST)); // [TPT] - MoNKi: -Support for High Contrast Mode-
		// Added by [TPT]-MoNKi [MoNKi: -invisible mode-]
		// Add key modifiers to ComboBox
		m_invmode_keymodifiers.ResetContent();
		m_invmode_keymodifiers.AddString(GetResString(IDS_CTRLKEY));
		m_invmode_keymodifiers.AddString(GetResString(IDS_ALTKEY));
		m_invmode_keymodifiers.AddString(GetResString(IDS_SHIFTKEY));
		m_invmode_keymodifiers.AddString(GetResString(IDS_CTRLKEY) + _T(" + ") + GetResString(IDS_ALTKEY));
		m_invmode_keymodifiers.AddString(GetResString(IDS_CTRLKEY) + _T(" + ") + GetResString(IDS_SHIFTKEY));
		m_invmode_keymodifiers.AddString(GetResString(IDS_ALTKEY) + _T(" + ") + GetResString(IDS_SHIFTKEY));
		m_invmode_keymodifiers.AddString(GetResString(IDS_CTRLKEY) + _T(" + ") + GetResString(IDS_ALTKEY) + _T(" + ") + GetResString(IDS_SHIFTKEY));

		CString key_modifier;
		if (m_iActualKeyModifier & MOD_CONTROL)
			key_modifier=GetResString(IDS_CTRLKEY);
		if (m_iActualKeyModifier & MOD_ALT){
			if (!key_modifier.IsEmpty()) key_modifier += " + ";
			key_modifier+=GetResString(IDS_ALTKEY);
		}
		if (m_iActualKeyModifier & MOD_SHIFT){
			if (!key_modifier.IsEmpty()) key_modifier += " + ";
			key_modifier+=GetResString(IDS_SHIFTKEY);
		}
		m_invmode_keymodifiers.SelectString(-1,key_modifier);
		
		m_InvisibleModeGroupBox.SetWindowText(GetResString(IDS_INVMODE_GROUP));
		m_iInvisibleMode.SetWindowText(GetResString(IDS_INVMODE));
		m_iInvisibleModeSelectStatic.SetWindowText(GetResString(IDS_INVMODE_HOTKEY));
		m_iInvisibleModeModifierStatic.SetWindowText(GetResString(IDS_INVMODE_MODKEY));
		m_iInvisibleModeKeyStatic.SetWindowText(GetResString(IDS_INVMODE_VKEY));
		m_iInvisibleModePlus.SetWindowText(_T("+"));
		// End [TPT]-MoNKi
		
		// [TPT] - IP Country
		m_iEnableShowCountryNames.SetWindowText(GetResString(IDS_SHOW_COUNTRYNAMES));
		m_iEnableShowCountryFlags.SetWindowText(GetResString(IDS_SHOW_COUNTRYFLAGS));
		// [TPT] - IP Country
		m_iBold.SetWindowText(GetResString(IDS_DOWNLOAD_IN_BOLD));
		m_iShowBackgroundInMenus.SetWindowText(GetResString(IDS_SHOWBACKGROUNDSINMENUS));
		m_iShowUpPrio.SetWindowText(GetResString(IDS_SHOWUPPRIO));
		//ARCHIVOS ------------------------------------------------------------
		// [TPT] - SLUGFILLER: hideOS
		m_iHideOSStatic.SetWindowText(GetResString(IDS_HIDEOVERSHARES));
		m_iSelectiveShare.SetWindowText(GetResString(IDS_SELECTIVESHARE));
		// [TPT] - SLUGFILLER: hideOS

		// FILES
		m_multiQueueGroupBox.SetWindowText(GetResString(IDS_PPG_MAELLA_MULTI_QUEUE_STATIC));
		m_multiQueueCheck01.SetWindowText(GetResString(IDS_PPG_MAELLA_MULTI_QUEUE_CHECK01));
		m_multiQueueCheck02.SetWindowText(GetResString(IDS_PPG_MAELLA_MULTI_QUEUE_CHECK02));

		//powershare
		m_powersharelimiteditstatic.SetWindowText(GetResString(IDS_POWERSHARE_LIMIT));
		m_powersharebox.SetWindowText(GetResString(IDS_POWERSHARE));
		m_powersharedisable.SetWindowText(GetResString(IDS_POWERSHARE_DISABLED));
		m_powershareenable.SetWindowText(GetResString(IDS_POWERSHARE_ACTIVATED));
		m_powershareauto.SetWindowText(GetResString(IDS_POWERSHARE_AUTO));
		m_powersharelimit.SetWindowText(GetResString(IDS_POWERSHARE_LIMITED));

	//MISC ----------------------------------------------------------------
		
		m_iSpreadBars.SetWindowText(GetResString(IDS_SPREADBARS)); // [TPT] - SLUGFILLER: Spreadbars
		m_iShowInMSN7.SetWindowText(GetResString(IDS_SHOWINMSN7));

		// [TPT] - Select process priority 
		m_PriorityGroupBox.SetWindowText(GetResString(IDS_PRIO_GROUP));
		m_HighPriority.SetWindowText(GetResString(IDS_PRIO_HIGH));		
		m_NormalPriority.SetWindowText(GetResString(IDS_PRIO_NORMAL));		
		m_IdlePriority.SetWindowText(GetResString(IDS_PRIO_IDLE));
		if (ExtraPrios)
		{
			m_RealTimePriority.SetWindowText(GetResString(IDS_PRIO_TIME_CRITICAL));
			m_AboveNormalPriority.SetWindowText(GetResString(IDS_PRIO_ABOVE_NORMAL));
			m_BelowNormalPriority.SetWindowText(GetResString(IDS_PRIO_BELOW_NORMAL));
		}
		// [TPT] - Select process priority 
		// [TPT] - Credit System
		m_iCreditsGroupBox.SetWindowText(GetResString(IDS_CREDITS_SEL)); // [TPT] - CreditSystem
		m_iCreditsPawcioStatic.SetWindowText(GetResString(IDS_CREDITS_PAWCIO));
		m_iCreditsLovelaceStatic.SetWindowText(GetResString(IDS_CREDITS_LOVELACE));
		m_iCreditsOfficialStatic.SetWindowText(GetResString(IDS_CREDITS_OFICIAL));
		m_iCreditsNoneStatic.SetWindowText(GetResString(IDS_CREDITS_NONE));
		// [TPT] - Credit System


	//CATEGORIAS ----------------------------------------------------------
		// [TPT] - khaos::categorymod+
		m_iShowCatNames.SetWindowText(GetResString(IDS_CAT_SHOWCATNAME));
		m_iSelectCat.SetWindowText(GetResString(IDS_CAT_SHOWSELCATDLG));
		m_iUseAutoCat.SetWindowText(GetResString(IDS_CAT_USEAUTOCAT));
		m_iUseActiveCat.SetWindowText(GetResString(IDS_CAT_USEACTIVE));
		m_iAutoSetResOrder.SetWindowText(GetResString(IDS_CAT_AUTORESUMEORD));		
		m_iResumeFileInNewCatStatic.SetWindowText(GetResString(IDS_CAT_RESUMENEXT));
		m_iShowPriorityInTab.SetWindowText(GetResString(IDS_SHOW_PRIORITY_IN_TAB));
		// [TPT] - khaos::categorymod-


	//ESTADISTICAS --------------------------------------------------------
		{   // zoom
			m_zoomGroupBox.SetWindowText(GetResString(IDS_GRAPHS));
			CString text;
			text.Format(GetResString(IDS_PPG_MAELLA_ZOOM_STATIC02), m_zoomSlider.GetPos());
			m_zoomStatic.SetWindowText(text);
		}
		{   // Datarate
			m_sampleRateGroupBox.SetWindowText(GetResString(IDS_PPG_MAELLA_SAMPLERATE_STATIC01));
			int position = m_sampleRateSlider.GetPos();
			CString text;
			if(position == 1)
				text = GetResString(IDS_PPG_MAELLA_SAMPLERATE_STATIC02a);
			else
				text.Format(GetResString(IDS_PPG_MAELLA_SAMPLERATE_STATIC02b), position);
			m_sampleRateStatic.SetWindowText(text);
		}	

		m_memoryConsuming.SetWindowText(GetResString(IDS_PPG_MEMORYCONSUMING));//memory consuming
		m_NAFCGraph.SetWindowText(GetResString(IDS_PPG_NAFCGRAPH));
		m_OverheadGraph.SetWindowText(GetResString(IDS_OVERHEADGRAPH));
		m_showVerticalLine.SetWindowText(GetResString(IDS_VERTICALLINES));
	//MINIMULE ------------------------------------------------------------
		// [TPT] - The Black Hand: MiniMule		[TPT] - TBH: minimule
		m_MiniMuleGroupBox.SetWindowText(GetResString(IDS_MINIMULE));
		m_iMiniMule.SetWindowText(GetResString(IDS_SHOWMINIMULE));		
		m_iMiniMuleLives.SetWindowText(GetResString(IDS_MINIMULELIVES));
		m_iMiniMuleUpdateStatic.SetWindowText(GetResString(IDS_MINIMULEUPDATE));
		{   // Transparency			
			CString text;
			text.Format(GetResString(IDS_MINIMULETRANS), m_MiniMuleTransparency.GetPos());
			m_MiniMuleTransparencyStatic.SetWindowText(text);
		}
		m_MiniMuleTransparencyMinimum.SetWindowText(GetResString(IDS_MINIMUM));
		m_MiniMuleTransparencyMaximum.SetWindowText(GetResString(IDS_MAXIMUM));
		// [TPT] - TBH: minimule


	}
}

void CPPgPhoenix::InitTab(){
	// Clear all to be sure
	m_tabCtr.DeleteAllItems();
	
	// Change style
	// Remark: It seems that the multi-row can not be activated with the properties
	m_tabCtr.ModifyStyle(0, TCS_MULTILINE);

	// Add all items with icon (connection, tweak, etc...)
	m_tabCtr.SetImageList(&m_ImageList);
	m_tabCtr.InsertItem(TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM, VISUALIZATION, GetResString(IDS_PREF_VISUALIZATION), 0, (LPARAM)VISUALIZATION); 	
	m_tabCtr.InsertItem(TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM, FILES, GetResString(IDS_FILES), 1, (LPARAM)FILES); 	
	m_tabCtr.InsertItem(TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM, MISCELLANEOUS, GetResString(IDS_PW_MISC), 2, (LPARAM)MISCELLANEOUS); 		
	m_tabCtr.InsertItem(TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM, CATEGORIES, GetResString(IDS_PREF_CATEGORIES), 3, (LPARAM)CATEGORIES); 
	m_tabCtr.InsertItem(TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM, STATSSETUPINFO, GetResString(IDS_STATSSETUPINFO), 4, (LPARAM)STATSSETUPINFO); 				
	m_tabCtr.InsertItem(TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM, MINIMULE, GetResString(IDS_MINIMULE), 5, (LPARAM)MINIMULE); 				
	
}

void CPPgPhoenix::InitControl(){
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
	
	//VISUAL --------------------------------------------------------------
	// [TPT] - itsonlyme: displayOptions START	
	m_iShowFileSystemIcon.CreateEx(0, _T("BUTTON"), _T(""), 
									 WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									 BS_AUTOCHECKBOX, 
									 CRect(right, top+20, right+300, top+40), this, IDC_PPG_PHOENIX_SHOWFILESYSTEMICON);
	m_iShowFileSystemIcon.SetFont(GetFont());	
	// [TPT] - SLUGFILLER: showComments
	m_iShowLocalRating.CreateEx(0, _T("BUTTON"), _T(""),
									 WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									 BS_AUTOCHECKBOX, 
									 CRect(right, top+40, right+300, top+60), this, IDC_PPG_PHOENIX_SHOWLOCALRATING);
	m_iShowLocalRating.SetFont(GetFont());
	// [TPT] - itsonlyme: displayOptions END
	// [TPT] - MoNKi: -Support for High Contrast Mode-
	m_iHighContrast.CreateEx(0, _T("BUTTON"), _T(""),
									 WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									 BS_AUTOCHECKBOX, 
									 CRect(right, top+60, right+300, top+80), this, IDC_PPG_PHOENIX_HIGHCONTRAST);
	m_iHighContrast.SetFont(GetFont());	
	// [TPT] - MoNKi: -Support for High Contrast Mode-
	// [TPT] - IP Country
	m_iEnableShowCountryNames.CreateEx(0, _T("BUTTON"), _T(""),
									 WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									 BS_AUTOCHECKBOX, 
									 CRect(right, top+80, right+300, top+100), this, IDC_PPG_PHOENIX_SHOW_COUNTRYNAMES);
	m_iEnableShowCountryNames.SetFont(GetFont());

	m_iEnableShowCountryFlags.CreateEx(0, _T("BUTTON"), _T(""),
									 WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									 BS_AUTOCHECKBOX, 
									 CRect(right, top+100, right+300, top+120), this, IDC_PPG_PHOENIX_SHOW_COUNTRYFLAGS);
	m_iEnableShowCountryFlags.SetFont(GetFont());
	// [TPT] - IP Country
	m_iBold.CreateEx(0, _T("BUTTON"), _T(""),
									 WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									 BS_AUTOCHECKBOX, 
									 CRect(right, top+120, right+300, top+140), this, IDC_PPG_PHOENIX_BOLD);
	m_iBold.SetFont(GetFont());

	m_iShowBackgroundInMenus.CreateEx(0, _T("BUTTON"), _T(""),
		WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
		BS_AUTOCHECKBOX, 
		CRect(right, top+140, right+300, top+160), this, IDC_PHOENIX_SHOWBACKGROUNDSINMENUS);
	m_iShowBackgroundInMenus.SetFont(GetFont());

	m_iShowUpPrio.CreateEx(0, _T("BUTTON"), _T(""),
		WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
		BS_AUTOCHECKBOX, 
		CRect(right, top+160, right+300, top+180), this, IDC_PPG_PHOENIX_SHOWUPPRIO);
	m_iShowUpPrio.SetFont(GetFont());
	

	// emulEspaña: added by MoNKi [TPT] - [MoNKi: -invisible mode-]
	m_InvisibleModeGroupBox.CreateEx(0, _T("BUTTON"), _T(""),
							   WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
							   BS_GROUPBOX,
							   CRect(right, top+200, right+300, top+200+120), this, IDC_PPG_PHOENIX_INVISIBLE_MODE_GROUP_BOX);
	m_InvisibleModeGroupBox.SetFont(GetFont());

	m_iInvisibleMode.CreateEx(0, _T("BUTTON"), _T(""),
									 WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									 BS_AUTOCHECKBOX, 
									 CRect(right+10, top+200+15, right+10+150, top+200+15+20), this, IDC_PPG_PHOENIX_INVISIBLE_MODE);
	m_iInvisibleMode.SetFont(GetFont());		
	
	m_iInvisibleModeSelectStatic.CreateEx(0, _T("STATIC"), _T(""),
							WS_CHILD /*| WS_VISIBLE*/, 
							CRect(right+10, top+200+40, right+10+250, top+200+40+20), this, IDC_PPG_PHOENIX_INVISIBLE_MODE_SELECT_STATIC);
	m_iInvisibleModeSelectStatic.SetFont(GetFont());	

	m_iInvisibleModeModifierStatic.CreateEx(0, _T("STATIC"), _T(""), 
							WS_CHILD /*| WS_VISIBLE*/, 
							CRect(right+10, top+265, right+10+140, top+265+20), this, IDC_PPG_PHOENIX_INVISIBLE_MODE_MODIFIER_STATIC);
	m_iInvisibleModeModifierStatic.SetFont(GetFont());	

	m_iInvisibleModeKeyStatic.CreateEx(0, _T("STATIC"), _T(""),
							WS_CHILD /*| WS_VISIBLE*/, 
							CRect(right+180, top+265, right+180+90, top+265+20), this, IDC_PPG_PHOENIX_INVISIBLE_KEY_STATIC);
	m_iInvisibleModeKeyStatic.SetFont(GetFont());	

	m_iInvisibleModePlus.CreateEx(0, _T("STATIC"), _T(""), 
							WS_CHILD /*| WS_VISIBLE*/, 
							CRect(right+160, top+290, right+160+10, top+290+20), this, IDC_PLUSLABEL);
	m_iInvisibleModePlus.SetFont(GetFont());	

	m_invmode_keymodifiers.Create(CBS_DROPDOWNLIST,							  
							  CRect(right+10, top+290, right+10+140, top+290+90), this, IDC_PPG_PHOENIX_INVISIBLE_MODE_KEY_COMBO);
	m_invmode_keymodifiers.SetFont(GetFont());	

	m_invmode_keys.Create(CBS_DROPDOWNLIST,							  
							  CRect(right+180, top+290, right+180+90, top+290+90), this, IDC_PPG_PHOENIX_INVISIBLE_MODE_SELECT_COMBO);
	m_invmode_keys.SetFont(GetFont());
	// emulEspaña: added by MoNKi [TPT] - [MoNKi: -invisible mode-]
	

	//ARCHIVOS ------------------------------------------------------------
	// [TPT] - SLUGFILLER: hideOS
	m_iHideOS.CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), 
						WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
						ES_RIGHT | ES_AUTOHSCROLL | ES_NUMBER, 
						CRect(right, top+20, right+20, top+20+20), this,  IDC_PPG_PHOENIX_HIDEOS);
	m_iHideOS.SetFont(GetFont());
	m_iHideOS.SetLimitText(1);
	m_iHideOSStatic.CreateEx(0, _T("STATIC"), _T(""), 
							WS_CHILD /*| WS_VISIBLE*/, 
							CRect(right+25, top+20, right+25+260, top+20+25), this, IDC_PPG_PHOENIX_HIDEOS_STATIC);
	m_iHideOSStatic.SetFont(GetFont());

	m_iSelectiveShare.CreateEx(0, _T("BUTTON"), _T(""), 
									 WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									 BS_AUTOCHECKBOX, 
									 CRect(right, top+20+30, right+300, top+20+30+20), this, IDC_PPG_PHOENIX_SELECTIVESHARE);
	m_iSelectiveShare.SetFont(GetFont());	
	// [TPT] - SLUGFILLER: hideOS

	// Maella -MultiQueue-
	m_multiQueueGroupBox.CreateEx(0, _T("BUTTON"), _T(""), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_GROUPBOX,
								CRect(right, top+90, right+300, top+90+75), this, IDC_PPG_MAELLA_MULTI_QUEUE_STATIC);
	m_multiQueueGroupBox.SetFont(GetFont());

	m_multiQueueCheck01.CreateEx(0, _T("BUTTON"), _T(""), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_AUTOCHECKBOX, 
									CRect(right+10, top+90+20, right+295, top+90+20+20), this, IDC_PPG_MAELLA_MULTI_QUEUE_CHECK01);
	m_multiQueueCheck01.SetFont(GetFont());

	m_multiQueueCheck02.CreateEx(0, _T("BUTTON"), _T(""), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_AUTOCHECKBOX, 
									CRect(right+10+10, top+90+40, right+295, top+90+40+20), this, IDC_PPG_MAELLA_MULTI_QUEUE_CHECK02);
	m_multiQueueCheck02.SetFont(GetFont());

	//powershare
	m_powersharebox.CreateEx(0, _T("BUTTON"), _T(""), 
							WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
							BS_GROUPBOX,
							CRect(right, top+170, right+110, top+310), this, IDC_PPG_POWERSHAREBOX);
	m_powersharebox.SetFont(GetFont());		

	m_iPowershare.CreateEx(WS_EX_STATICEDGE,
							WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP | WS_BORDER |
							TBS_TOOLTIPS | TBS_BOTH | TBS_VERT | TBS_NOTICKS,
							CRect(right+10, top+190, right+10+20, top+300), this, IDC_PPG_POWERSHARESLIDER);
	m_iPowershare.SetFont(GetFont());
	m_iPowershare.SetRange(0,3);
	m_powersharedisable.CreateEx(0, _T("STATIC"), _T(""), 
							WS_CHILD /*| WS_VISIBLE*/, 
							CRect(right+40, top+190, right+100, top+210), this, IDC_PPG_POWERSHAREDISABLE);
	m_powersharedisable.SetFont(GetFont());
	m_powershareenable.CreateEx(0, _T("STATIC"), _T(""), 
							WS_CHILD /*| WS_VISIBLE*/, 
							CRect(right+40, top+220, right+100, top+240), this, IDC_PPG_POWERSHAREENABLE);
	m_powershareenable.SetFont(GetFont());
	m_powershareauto.CreateEx(0, _T("STATIC"), _T(""), 
							WS_CHILD /*| WS_VISIBLE*/, 
							CRect(right+40, top+250, right+100, top+270), this, IDC_PPG_POWERSHAREAUTO);
	m_powershareauto.SetFont(GetFont());
	m_powersharelimit.CreateEx(0, _T("STATIC"), _T(""), 
							WS_CHILD /*| WS_VISIBLE*/, 
							CRect(right+40, top+280, right+100, top+300), this, IDC_PPG_POWERSHARELIMIT);
	m_powersharelimit.SetFont(GetFont());
	
	m_powersharelimitedit.CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), 
						WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
						ES_RIGHT | ES_AUTOHSCROLL | ES_NUMBER, 
						CRect(right+200, top+220, right+250, top+240), this,  IDC_PPG_POWERSHARELIMITEDIT);
	m_powersharelimitedit.SetFont(GetFont());
	m_powersharelimitedit.SetLimitText(4);
	m_powersharelimiteditstatic.CreateEx(0, _T("STATIC"), _T(""), 
							WS_CHILD /*| WS_VISIBLE*/, 
							CRect(right+120, top+200, right+300, top+220), this, IDC_PPG_POWERSHARELIMITEDITSTATIC);
	m_powersharelimiteditstatic.SetFont(GetFont());

		
	//MISC ----------------------------------------------------------------
	
	// [TPT] - SLUGFILLER: Spreadbars
	m_iSpreadBars.CreateEx(0, _T("BUTTON"), _T(""), 
									 WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									 BS_AUTOCHECKBOX, 
									 CRect(right, top+190, right+300, top+190+20), this, IDC_PPG_PHOENIX_SHOWSPREADBARS);
	m_iSpreadBars.SetFont(GetFont());
	// [TPT] - SLUGFILLER: Spreadbars

	m_iShowInMSN7.CreateEx(0, _T("BUTTON"), _T(""), 
		WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
		BS_AUTOCHECKBOX, 
		CRect(right, top+210, right+300, top+230), this, IDC_PPG_PHOENIX_SHOWINMSN7);
	m_iShowInMSN7.SetFont(GetFont());

	// [TPT] - Select process priority 
	m_PriorityGroupBox.CreateEx(0, _T("BUTTON"), _T(""), 
							   WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
							   BS_GROUPBOX,
							   CRect(right, top+20, right+150, top+20+160), this, IDC_PPG_PHOENIX_PRIORITY_GROUP_BOX);
	m_PriorityGroupBox.SetFont(GetFont());

	m_selectPriorityProcess.CreateEx(WS_EX_STATICEDGE,
							  WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP | WS_BORDER |
							  TBS_TOOLTIPS | TBS_BOTH | TBS_VERT | TBS_NOTICKS,
							  CRect(right+10, top+20+30, right+10+20, top+20+30+120), this, IDC_PPG_PHOENIX_PROCESSPRIORITY);
	m_selectPriorityProcess.SetFont(GetFont());
	m_selectPriorityProcess.SetRange(0, 5);	
	if (ExtraPrios)
	{
		m_selectPriorityProcess.SetRange(0, 5);
		m_RealTimePriority.CreateEx(0, _T("STATIC"), _T(""), 
								WS_CHILD /*| WS_VISIBLE*/, 
								CRect(right+10+25, top+20+30, right+10+25+105, top+20+30+15), this, IDC_PPG_PHOENIX_REALTIMEPRIORITY);
		m_RealTimePriority.SetFont(GetFont());
		m_HighPriority.CreateEx(0, _T("STATIC"), _T(""), 
								WS_CHILD /*| WS_VISIBLE*/, 
								CRect(right+10+25, top+20+30+20, right+10+25+105, top+20+30+20+15), this, IDC_PPG_PHOENIX_HIGHPRIORITY);
		m_HighPriority.SetFont(GetFont());
		m_AboveNormalPriority.CreateEx(0, _T("STATIC"), _T(""), 
								WS_CHILD /*| WS_VISIBLE*/, 
								CRect(right+10+25, top+20+30+40, right+10+25+105, top+20+30+40+15), this, IDC_PPG_PHOENIX_ABOVENORMALPRIORITY);
		m_AboveNormalPriority.SetFont(GetFont());
		m_NormalPriority.CreateEx(0, _T("STATIC"), _T(""), 
								WS_CHILD /*| WS_VISIBLE*/, 
								CRect(right+10+25, top+20+30+60, right+10+25+105, top+20+30+60+15), this, IDC_PPG_PHOENIX_NORMALPRIORITY);
		m_NormalPriority.SetFont(GetFont());
		m_BelowNormalPriority.CreateEx(0, _T("STATIC"), _T(""), 
								WS_CHILD /*| WS_VISIBLE*/, 
								CRect(right+10+25, top+20+30+80, right+10+25+105, top+20+30+80+15), this, IDC_PPG_PHOENIX_BELOWNORMALPRIORITY);
		m_BelowNormalPriority.SetFont(GetFont());
		m_IdlePriority.CreateEx(0, _T("STATIC"), _T(""), 
								WS_CHILD /*| WS_VISIBLE*/, 
								CRect(right+10+25, top+20+30+100, right+10+25+105, top+20+30+100+15), this, IDC_PPG_PHOENIX_IDLEPRIORITY);
		m_IdlePriority.SetFont(GetFont());
	}
	else
	{
		m_selectPriorityProcess.SetRange(0, 2);		
		m_HighPriority.CreateEx(0, _T("STATIC"), _T(""), 
								WS_CHILD /*| WS_VISIBLE*/, 
								CRect(right+10+25, top+20+30, right+10+25+105, top+20+30+15), this, IDC_PPG_PHOENIX_HIGHPRIORITY);
		m_HighPriority.SetFont(GetFont());		
		m_NormalPriority.CreateEx(0, _T("STATIC"), _T(""), 
								WS_CHILD /*| WS_VISIBLE*/, 
								CRect(right+10+25, top+20+30+50, right+10+25+105, top+20+30+50+15), this, IDC_PPG_PHOENIX_NORMALPRIORITY);
		m_NormalPriority.SetFont(GetFont());		
		m_IdlePriority.CreateEx(0, _T("STATIC"), _T(""), 
								WS_CHILD /*| WS_VISIBLE*/, 
								CRect(right+10+25, top+20+30+100, right+10+25+105, top+20+30+100+15), this, IDC_PPG_PHOENIX_IDLEPRIORITY);
		m_IdlePriority.SetFont(GetFont());
	}
	// [TPT] - Select process priority 

	// [TPT] - Credit System
	m_iCreditsGroupBox.CreateEx(0, _T("BUTTON"), _T(""), 
							   WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
							   BS_GROUPBOX,
							   CRect(right+160, top+20, right+310, top+20+150), this, IDC_PPG_PHOENIX_CREDITS_GROUP_BOX);
	m_iCreditsGroupBox.SetFont(GetFont());		

	m_iCredits.CreateEx(WS_EX_STATICEDGE,
							  WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP | WS_BORDER |
							  TBS_TOOLTIPS | TBS_BOTH | TBS_VERT | TBS_NOTICKS,
							  CRect(right+160+10, top+20+30, right+160+30, top+20+30+110), this, IDC_PPG_PHOENIX_CREDITS);
	m_iCredits.SetFont(GetFont());
	m_iCredits.SetRange(0, 3);	

	m_iCreditsLovelaceStatic.CreateEx(0, _T("STATIC"), _T(""), 
							WS_CHILD /*| WS_VISIBLE*/, 
							CRect(right+160+10+40, top+20+30, right+160+10+130, top+20+30+20), this, IDC_PPG_PHOENIX_LOVELACE_CREDITS_STATIC);
	m_iCreditsLovelaceStatic.SetFont(GetFont());	
	
	m_iCreditsPawcioStatic.CreateEx(0, _T("STATIC"), _T(""), 
							WS_CHILD /*| WS_VISIBLE*/, 
							CRect(right+160+10+40, top+20+30+30, right+160+10+130, top+20+30+30+20), this, IDC_PPG_PHOENIX_PAWCIO_CREDITS_STATIC);
	m_iCreditsPawcioStatic.SetFont(GetFont());		
		
	m_iCreditsOfficialStatic.CreateEx(0, _T("STATIC"), _T(""), 
							WS_CHILD /*| WS_VISIBLE*/, 
							CRect(right+160+10+40, top+20+30+60, right+160+10+130, top+20+30+60+20), this, IDC_PPG_PHOENIX_OFFICIAL_CREDITS_STATIC);
	m_iCreditsOfficialStatic.SetFont(GetFont());	

	m_iCreditsNoneStatic.CreateEx(0, _T("STATIC"), _T(""), 
							WS_CHILD /*| WS_VISIBLE*/, 
							CRect(right+160+10+40, top+20+30+90, right+160+10+130, top+20+30+90+20), this, IDC_PPG_PHOENIX_NONE_CREDITS_STATIC);
	m_iCreditsNoneStatic.SetFont(GetFont());	
	// [TPT] - Credit System

	//CATEGORIAS ----------------------------------------------------------
	//-- CATEGORIES	
	// [TPT] - khaos::categorymod+
	m_iShowCatNames.CreateEx(0, _T("BUTTON"), _T(""), 
									 WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									 BS_AUTOCHECKBOX, 
									 CRect(right, top+20, right+300, top+20+20), this, IDC_PPG_PHOENIX_SHOW_CATNAMES);
	m_iShowCatNames.SetFont(GetFont());	

	m_iSelectCat.CreateEx(0, _T("BUTTON"), _T(""), 
									 WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									 BS_AUTOCHECKBOX, 
									 CRect(right, top+20+25, right+300, top+20+25+20), this, IDC_PPG_PHOENIX_SELECT_CAT);
	m_iSelectCat.SetFont(GetFont());	
	
	m_iUseActiveCat.CreateEx(0, _T("BUTTON"), _T(""), 
									 WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									 BS_AUTOCHECKBOX, 
									 CRect(right, top+20+50, right+300, top+20+50+20), this, IDC_PPG_PHOENIX_USE_ACTIVECAT);
	m_iUseActiveCat.SetFont(GetFont());	
	
	m_iAutoSetResOrder.CreateEx(0, _T("BUTTON"), _T(""), 
									 WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									 BS_AUTOCHECKBOX, 
									 CRect(right, top+20+75, right+300, top+20+75+20), this, IDC_PPG_PHOENIX_AUTOSETRESORDER);
	m_iAutoSetResOrder.SetFont(GetFont());	

	m_iResumeFileInNewCat.CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), 
						WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
						ES_RIGHT | ES_AUTOHSCROLL | ES_NUMBER, 
						CRect(right, top+20+100, right+25, top+20+100+20), this,  IDC_PPG_PHOENIX_RESUMEFILEINNEWCAT);
	m_iResumeFileInNewCat.SetFont(GetFont());
	m_iResumeFileInNewCat.SetLimitText(2);
	m_iResumeFileInNewCatStatic.CreateEx(0, _T("STATIC"), _T(""), 
							WS_CHILD /*| WS_VISIBLE*/, 
							CRect(right+30, top+25+100, right+30+240, top+25+100+25), this, IDC_PPG_PHOENIX_RESUMEFILEINNEWCAT_STATIC);
	m_iResumeFileInNewCatStatic.SetFont(GetFont());
	
	m_iUseAutoCat.CreateEx(0, _T("BUTTON"), _T(""), 
									 WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									 BS_AUTOCHECKBOX, 
									 CRect(right, top+20+125, right+300, top+20+125+20), this, IDC_PPG_PHOENIX_USEAUTOCAT);
	m_iUseAutoCat.SetFont(GetFont());

	m_iShowPriorityInTab.CreateEx(0, _T("BUTTON"), _T(""), 
									 WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									 BS_AUTOCHECKBOX, 
									 CRect(right, top+20+145, right+300, top+20+145+20), this, IDC_PPG_PHOENIX_SHOWPRIORITYINTAB);
	m_iShowPriorityInTab.SetFont(GetFont());
	// [TPT] - khaos::categorymod-


	//ESTADISTICAS --------------------------------------------------------
	// Maella -Graph: display zoom-
	m_zoomGroupBox.CreateEx(0, _T("BUTTON"), _T(""), 
							WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
							BS_GROUPBOX,
							CRect(right, top+20, right+300, top+20+75), this, IDC_PPG_MAELLA_ZOOM_STATIC01);
	m_zoomGroupBox.SetFont(GetFont());

	m_zoomStatic.CreateEx(0, _T("STATIC"), _T(""), 
							WS_CHILD /*| WS_VISIBLE*/,
							CRect(right+10, top+20+20, right+295, top+20+20+20), this, IDC_PPG_MAELLA_ZOOM_STATIC02);
	m_zoomStatic.SetFont(GetFont());

	m_zoomSlider.CreateEx(WS_EX_STATICEDGE,
							WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP | WS_BORDER |
							TBS_TOOLTIPS | TBS_BOTH | TBS_HORZ | TBS_NOTICKS,
							CRect(right+10, top+20+40, right+290, top+20+40+20), this, IDC_PPG_MAELLA_ZOOM_SLIDER);
	m_zoomSlider.SetFont(GetFont());
	m_zoomSlider.SetRange(1, 10);

	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	m_sampleRateGroupBox.CreateEx(0, _T("BUTTON"), _T(""), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_GROUPBOX,
									CRect(right, top+20+85, right+300, top+20+85+75), this, IDC_PPG_MAELLA_SAMPLERATE_STATIC01);
	m_sampleRateGroupBox.SetFont(GetFont());

	m_sampleRateStatic.CreateEx(0, _T("STATIC"), _T(""), 
								WS_CHILD /*| WS_VISIBLE*/,
								CRect(right+10, top+20+85+20, right+295, top+20+85+20+20), this, IDC_PPG_MAELLA_SAMPLERATE_STATIC02);
	m_sampleRateStatic.SetFont(GetFont());

	m_sampleRateSlider.CreateEx(WS_EX_STATICEDGE,
								WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP | WS_BORDER |
								TBS_TOOLTIPS | TBS_BOTH | TBS_HORZ | TBS_NOTICKS,
								CRect(right+10, top+20+85+40, right+290, top+20+85+40+20), this, IDC_PPG_MAELLA_SAMPLERATE_SLIDER);
	m_sampleRateSlider.SetFont(GetFont());
	m_sampleRateSlider.SetRange(1, 10);

	m_memoryConsuming.CreateEx(0, _T("BUTTON"), _T(""), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_AUTOCHECKBOX, 
									CRect(right, top+200, right+300, top+200+20), this, IDC_PPG_MEMORYCONSUMING);
	m_memoryConsuming.SetFont(GetFont());
	m_NAFCGraph.CreateEx(0, _T("BUTTON"), _T(""), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_AUTOCHECKBOX, 
									CRect(right, top+230, right+300, top+250), this, IDC_PPG_NAFCGRAPH);
	m_NAFCGraph.SetFont(GetFont());	
	m_OverheadGraph.CreateEx(0, _T("BUTTON"), _T(""), 
									WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									BS_AUTOCHECKBOX, 
									CRect(right, top+260, right+300, top+280), this, IDC_OVERHEADGRAPH);
	m_OverheadGraph.SetFont(GetFont());	
	m_showVerticalLine.CreateEx(0, _T("BUTTON"), _T(""), 
		WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
		BS_AUTOCHECKBOX, 
		CRect(right, top+290, right+300, top+310), this, IDC_VERTICALLINES);
	m_showVerticalLine.SetFont(GetFont());	


	//MINIMULE ------------------------------------------------------------
	// [TPT] - TBH: minimule
	m_MiniMuleGroupBox.CreateEx(0, _T("BUTTON"), _T(""), 
							   WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
							   BS_GROUPBOX,
							   CRect(right, top+20, right+300, top+20+190), this, IDC_PPG_PHOENIX_MINIMULE_GROUP_BOX);
	m_MiniMuleGroupBox.SetFont(GetFont());

	m_iMiniMule.CreateEx(0, _T("BUTTON"), _T(""), 
									 WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									 BS_AUTOCHECKBOX, 
									 CRect(right+10, top+20+15, right+10+280, top+20+15+20), this, IDC_PPG_PHOENIX_MINIMULE);
	m_iMiniMule.SetFont(GetFont());	
	
	m_iMiniMuleLives.CreateEx(0, _T("BUTTON"), _T(""), 
									 WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
									 BS_AUTOCHECKBOX, 
									 CRect(right+10, top+20+45, right+10+280, top+20+45+20), this, IDC_PPG_PHOENIX_MINIMULELIVES);
	m_iMiniMuleLives.SetFont(GetFont());	
	
	m_iMiniMuleUpdate.CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), 
						WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP |
						ES_RIGHT | ES_AUTOHSCROLL | ES_NUMBER, 
						CRect(right+10, top+20+75, right+10+20, top+20+75+20), this,  IDC_PPG_PHOENIX_MINIMULEUPDATE);;
	m_iMiniMuleUpdate.SetFont(GetFont());
	m_iMiniMuleUpdate.SetLimitText(4);
	
	m_iMiniMuleUpdateStatic.CreateEx(0, _T("STATIC"), _T(""), 
							WS_CHILD /*| WS_VISIBLE*/, 
							CRect(right+10+25, top+20+75, right+10+25+255, top+20+75+20), this, IDC_PPG_PHOENIX_MINIMULEUPDATE_STATIC);
	m_iMiniMuleUpdateStatic.SetFont(GetFont());	

	m_MiniMuleTransparency.CreateEx(WS_EX_STATICEDGE,
							  WS_CHILD /*| WS_VISIBLE*/ | WS_TABSTOP | WS_BORDER |
							  TBS_TOOLTIPS | TBS_BOTH | TBS_HORZ | TBS_NOTICKS,
							  CRect(right+10+15, top+20+140, right+10+15+255, top+20+140+20), this, IDC_PPG_PHOENIX_MINIMULE_TRANSPARENCY);
	m_MiniMuleTransparency.SetFont(GetFont());
	m_MiniMuleTransparency.SetRange(0, 255);	
	
	m_MiniMuleTransparencyStatic.CreateEx(0, _T("STATIC"), _T(""),
							WS_CHILD /*| WS_VISIBLE*/, 
							CRect(right+10, top+20+115, right+10+150, top+20+115+20), this, IDC_PPG_PHOENIX_MINIMULE_TRANSP_STATIC);
	m_MiniMuleTransparencyStatic.SetFont(GetFont());	

	m_MiniMuleTransparencyMinimum.CreateEx(0, _T("STATIC"), _T(""),
							WS_CHILD /*| WS_VISIBLE*/, 
							CRect(right+10+15, top+20+165, right+10+15+55, top+20+165+20), this, IDC_PPG_PHOENIX_MINIMULE_TRANSP_MINIMUM);
	m_MiniMuleTransparencyMinimum.SetFont(GetFont());	

	m_MiniMuleTransparencyMaximum.CreateEx(0, _T("STATIC"), _T(""),
							WS_CHILD /*| WS_VISIBLE*/, 
							CRect(right+10+15+210, top+20+165, right+10+15+265, top+20+165+20), this, IDC_PPG_PHOENIX_MINIMULE_TRANSP_MAXIMUM);
	m_MiniMuleTransparencyMaximum.SetFont(GetFont());	
	// [TPT] - TBH: minimule


}

void CPPgPhoenix::OnTabSelectionChange(NMHDR *pNMHDR, LRESULT *pResult)
{
	// Retrieve tab to display
	TCITEM tabCtrlItem; 
	tabCtrlItem.mask = TCIF_PARAM;
	if(m_tabCtr.GetItem(m_tabCtr.GetCurSel(), &tabCtrlItem) == TRUE){
		SetTab(static_cast<eTab>(tabCtrlItem.lParam));
	}

	*pResult = 0;
}

void CPPgPhoenix::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CSliderCtrl* slider =(CSliderCtrl*)pScrollBar;
	if(slider == &m_iPowershare)
	{
		int position = slider->GetPos();
		if(position == 3)
		{
			m_powersharelimitedit.EnableWindow(TRUE);
			m_powersharelimiteditstatic.EnableWindow(TRUE);
		}
		else
		{
			m_powersharelimitedit.EnableWindow(FALSE);
			m_powersharelimiteditstatic.EnableWindow(FALSE);
		}
	}
	SetModified(TRUE);	

	CPropertyPage::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CPPgPhoenix::OnMultiQueueChange(){
	m_multiQueueCheck02.EnableWindow(m_multiQueueCheck01.GetCheck() != BST_UNCHECKED);
	SetModified(TRUE);
}


void CPPgPhoenix::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	SetModified(TRUE);
	
	CSliderCtrl* slider =(CSliderCtrl*)pScrollBar;
	int position = slider->GetPos();
	// [TPT] - TBH: minimule
	if(slider == &m_MiniMuleTransparency){
		CString text;
		text.Format(GetResString(IDS_MINIMULETRANS), position);
		m_MiniMuleTransparencyStatic.SetWindowText(text);
	}
	// [TPT] - TBH: minimule

	if(slider == &m_zoomSlider){
		CString text;
		text.Format(GetResString(IDS_PPG_MAELLA_ZOOM_STATIC02), position);
		m_zoomStatic.SetWindowText(text);
	}
	else if(slider == &m_sampleRateSlider){
		CString text;
		if(position == 1)
			text = GetResString(IDS_PPG_MAELLA_SAMPLERATE_STATIC02a);
		else
			text.Format(GetResString(IDS_PPG_MAELLA_SAMPLERATE_STATIC02b), position);
		m_sampleRateStatic.SetWindowText(text);
	}



	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CPPgPhoenix::SetTab(eTab tab){
	if(m_currentTab != tab){
		// Hide all control
		switch(m_currentTab){
			case FILES:
				// [TPT] - SLUGFILLER: hideOS
				m_iHideOS.ShowWindow(SW_HIDE);
				m_iHideOS.EnableWindow(FALSE);		
				m_iHideOSStatic.ShowWindow(SW_HIDE);
				m_iHideOSStatic.EnableWindow(FALSE);
				m_iSelectiveShare.ShowWindow(SW_HIDE);
				m_iSelectiveShare.EnableWindow(FALSE);			
				m_multiQueueGroupBox.ShowWindow(SW_HIDE);
				m_multiQueueGroupBox.EnableWindow(FALSE);
				m_multiQueueCheck01.ShowWindow(SW_HIDE);
				m_multiQueueCheck01.EnableWindow(FALSE);
				m_multiQueueCheck02.ShowWindow(SW_HIDE);
				m_multiQueueCheck02.EnableWindow(FALSE);
				//powershare
				m_powersharebox.ShowWindow(SW_HIDE);
				m_powersharebox.EnableWindow(FALSE);
				m_powersharedisable.ShowWindow(SW_HIDE);
				m_powersharedisable.EnableWindow(FALSE);
				m_powershareenable.ShowWindow(SW_HIDE);
				m_powershareenable.EnableWindow(FALSE);
				m_powershareauto.ShowWindow(SW_HIDE);
				m_powershareauto.EnableWindow(FALSE);
				m_powersharelimit.ShowWindow(SW_HIDE);
				m_powersharelimit.EnableWindow(FALSE);
				m_powersharelimitedit.ShowWindow(SW_HIDE);
				m_powersharelimitedit.EnableWindow(FALSE);
				m_powersharelimiteditstatic.ShowWindow(SW_HIDE);
				m_powersharelimiteditstatic.EnableWindow(FALSE);
				m_iPowershare.ShowWindow(SW_HIDE);
				m_iPowershare.EnableWindow(FALSE);
				break;
			case MISCELLANEOUS:
				// [TPT] - SLUGFILLER: Spreadbars
				m_iSpreadBars.ShowWindow(SW_HIDE);
				m_iSpreadBars.EnableWindow(FALSE);				
				// [TPT] - Select process priority
				m_iShowInMSN7.ShowWindow(SW_HIDE);
				m_iShowInMSN7.EnableWindow(FALSE);
				m_selectPriorityProcess.ShowWindow(SW_HIDE);
				m_selectPriorityProcess.EnableWindow(FALSE);				
				m_PriorityGroupBox.ShowWindow(SW_HIDE);
				m_PriorityGroupBox.EnableWindow(FALSE);				
				m_HighPriority.ShowWindow(SW_HIDE);
				m_HighPriority.EnableWindow(FALSE);				
				m_NormalPriority.ShowWindow(SW_HIDE);
				m_NormalPriority.EnableWindow(FALSE);				
				m_IdlePriority.ShowWindow(SW_HIDE);
				m_IdlePriority.EnableWindow(FALSE);
				if (ExtraPrios)
				{
					m_RealTimePriority.ShowWindow(SW_HIDE);
					m_RealTimePriority.EnableWindow(FALSE);
					m_AboveNormalPriority.ShowWindow(SW_HIDE);
					m_AboveNormalPriority.EnableWindow(FALSE);
					m_BelowNormalPriority.ShowWindow(SW_HIDE);
					m_BelowNormalPriority.EnableWindow(FALSE);
				}		
				// [TPT] - Credit System
				m_iCredits.ShowWindow(SW_HIDE);
				m_iCredits.EnableWindow(FALSE);	
				m_iCreditsGroupBox.ShowWindow(SW_HIDE);
				m_iCreditsGroupBox.EnableWindow(FALSE);	
				m_iCreditsPawcioStatic.ShowWindow(SW_HIDE);
				m_iCreditsPawcioStatic.EnableWindow(FALSE);	
				m_iCreditsLovelaceStatic.ShowWindow(SW_HIDE);
				m_iCreditsLovelaceStatic.EnableWindow(FALSE);	
				m_iCreditsOfficialStatic.ShowWindow(SW_HIDE);
				m_iCreditsOfficialStatic.EnableWindow(FALSE);	
				m_iCreditsNoneStatic.ShowWindow(SW_HIDE);
				m_iCreditsNoneStatic.EnableWindow(FALSE);	
				// [TPT] - Credit System
				break;
			case CATEGORIES:
				// [TPT] - khaos::categorymod
				m_iShowCatNames.ShowWindow(SW_HIDE);
				m_iShowCatNames.EnableWindow(FALSE);	
				m_iSelectCat.ShowWindow(SW_HIDE);
				m_iSelectCat.EnableWindow(FALSE);	
				m_iUseActiveCat.ShowWindow(SW_HIDE);
				m_iUseActiveCat.EnableWindow(FALSE);	
				m_iAutoSetResOrder.ShowWindow(SW_HIDE);
				m_iAutoSetResOrder.EnableWindow(FALSE);	
				m_iResumeFileInNewCat.ShowWindow(SW_HIDE);
				m_iResumeFileInNewCat.EnableWindow(FALSE);	
				m_iResumeFileInNewCatStatic.ShowWindow(SW_HIDE);
				m_iResumeFileInNewCatStatic.EnableWindow(FALSE);	
				m_iUseAutoCat.ShowWindow(SW_HIDE);
				m_iUseAutoCat.EnableWindow(FALSE);
				m_iShowPriorityInTab.ShowWindow(SW_HIDE);
				m_iShowPriorityInTab.EnableWindow(FALSE);
				break;

			case VISUALIZATION:
				// [TPT] - itsonlyme: displayOptions
				m_iShowFileSystemIcon.ShowWindow(SW_HIDE);
				m_iShowFileSystemIcon.EnableWindow(FALSE);								
				m_iShowLocalRating.ShowWindow(SW_HIDE);
				m_iShowLocalRating.EnableWindow(FALSE);
			
				// [TPT] - IP Country
				m_iEnableShowCountryNames.ShowWindow(SW_HIDE);
				m_iEnableShowCountryNames.EnableWindow(FALSE);
				m_iEnableShowCountryFlags.ShowWindow(SW_HIDE);
				m_iEnableShowCountryFlags.EnableWindow(FALSE);
				// [TPT] - IP Country
				// [TPT] - MoNKi: -Support for High Contrast Mode-
				m_iHighContrast.ShowWindow(SW_HIDE);
				m_iHighContrast.EnableWindow(FALSE);
				// emulEspaña: added by MoNKi [TPT] - [MoNKi: -invisible mode-]
				m_InvisibleModeGroupBox.ShowWindow(SW_HIDE);
				m_InvisibleModeGroupBox.EnableWindow(FALSE);
				m_iInvisibleMode.ShowWindow(SW_HIDE);
				m_iInvisibleMode.EnableWindow(FALSE);
				m_iInvisibleModeSelectStatic.ShowWindow(SW_HIDE);
				m_iInvisibleModeSelectStatic.EnableWindow(FALSE);
				m_iInvisibleModeModifierStatic.ShowWindow(SW_HIDE);
				m_iInvisibleModeModifierStatic.EnableWindow(FALSE);
				m_iInvisibleModeKeyStatic.ShowWindow(SW_HIDE);
				m_iInvisibleModeKeyStatic.EnableWindow(FALSE);
				m_iInvisibleModePlus.ShowWindow(SW_HIDE);
				m_iInvisibleModePlus.EnableWindow(FALSE);
				m_invmode_keymodifiers.ShowWindow(SW_HIDE);
				m_invmode_keymodifiers.EnableWindow(FALSE);
				m_invmode_keys.ShowWindow(SW_HIDE);
				m_invmode_keys.EnableWindow(FALSE);	
				m_iBold.ShowWindow(SW_HIDE);
				m_iBold.EnableWindow(FALSE);
				m_iShowBackgroundInMenus.ShowWindow(SW_HIDE);
				m_iShowBackgroundInMenus.EnableWindow(FALSE);
				m_iShowUpPrio.ShowWindow(SW_HIDE);
				m_iShowUpPrio.EnableWindow(FALSE);
				break;
			case MINIMULE:
				m_MiniMuleGroupBox.ShowWindow(SW_HIDE);
				m_MiniMuleGroupBox.EnableWindow(FALSE);	
				m_iMiniMule.ShowWindow(SW_HIDE);
				m_iMiniMule.EnableWindow(FALSE);	
				m_iMiniMuleLives.ShowWindow(SW_HIDE);
				m_iMiniMuleLives.EnableWindow(FALSE);	
				m_iMiniMuleUpdate.ShowWindow(SW_HIDE);
				m_iMiniMuleUpdate.EnableWindow(FALSE);	
				m_iMiniMuleUpdateStatic.ShowWindow(SW_HIDE);
				m_iMiniMuleUpdateStatic.EnableWindow(FALSE);	
				m_MiniMuleTransparency.ShowWindow(SW_HIDE);
				m_MiniMuleTransparency.EnableWindow(FALSE);	
				m_MiniMuleTransparencyStatic.ShowWindow(SW_HIDE);
				m_MiniMuleTransparencyStatic.EnableWindow(FALSE);	
				m_MiniMuleTransparencyMinimum.ShowWindow(SW_HIDE);
				m_MiniMuleTransparencyMinimum.EnableWindow(FALSE);	
				m_MiniMuleTransparencyMaximum.ShowWindow(SW_HIDE);
				m_MiniMuleTransparencyMaximum.EnableWindow(FALSE);	
				break;

			case STATSSETUPINFO:
				m_zoomGroupBox.ShowWindow(SW_HIDE);
				m_zoomGroupBox.EnableWindow(FALSE);
				m_zoomStatic.ShowWindow(SW_HIDE);
				m_zoomStatic.EnableWindow(FALSE);
				m_zoomSlider.ShowWindow(SW_HIDE);
				m_zoomSlider.EnableWindow(FALSE);

				m_sampleRateGroupBox.ShowWindow(SW_HIDE);
				m_sampleRateGroupBox.EnableWindow(FALSE);
				m_sampleRateStatic.ShowWindow(SW_HIDE);
				m_sampleRateStatic.EnableWindow(FALSE);
				m_sampleRateSlider.ShowWindow(SW_HIDE);
				m_sampleRateSlider.EnableWindow(FALSE);

				m_memoryConsuming.ShowWindow(SW_HIDE);
				m_memoryConsuming.EnableWindow(FALSE);

				m_NAFCGraph.ShowWindow(SW_HIDE);
				m_NAFCGraph.EnableWindow(FALSE);
				m_OverheadGraph.ShowWindow(SW_HIDE);
				m_OverheadGraph.EnableWindow(FALSE);
				m_showVerticalLine.ShowWindow(SW_HIDE);
				m_showVerticalLine.EnableWindow(FALSE);

				break;

		}

		// Show new controls
		m_currentTab = tab;
		switch(m_currentTab){
			case FILES:
				// [TPT] - SLUGFILLER: hideOS
				m_iHideOS.ShowWindow(SW_SHOW);
				m_iHideOS.EnableWindow(TRUE);
				m_iHideOSStatic.ShowWindow(SW_SHOW);
				m_iHideOSStatic.EnableWindow(TRUE);				
				m_iSelectiveShare.ShowWindow(SW_SHOW);
				m_iSelectiveShare.EnableWindow(TRUE);																					
				m_multiQueueGroupBox.ShowWindow(SW_SHOW);
				m_multiQueueGroupBox.EnableWindow(TRUE);
				m_multiQueueCheck01.ShowWindow(SW_SHOW);
				m_multiQueueCheck01.EnableWindow(TRUE);
				m_multiQueueCheck02.ShowWindow(SW_SHOW);
				m_multiQueueCheck02.EnableWindow(m_multiQueueCheck01.GetCheck() != BST_UNCHECKED);
				m_powersharebox.ShowWindow(SW_SHOW);
				m_powersharebox.EnableWindow(TRUE);
				m_powersharedisable.ShowWindow(SW_SHOW);
				m_powersharedisable.EnableWindow(TRUE);
				m_powershareenable.ShowWindow(SW_SHOW);
				m_powershareenable.EnableWindow(TRUE);
				m_powershareauto.ShowWindow(SW_SHOW);
				m_powershareauto.EnableWindow(TRUE);
				m_powersharelimit.ShowWindow(SW_SHOW);
				m_powersharelimit.EnableWindow(TRUE);
				m_powersharelimitedit.ShowWindow(SW_SHOW);
				m_powersharelimitedit.EnableWindow(m_iPowershare.GetPos() == 3);
				m_powersharelimiteditstatic.ShowWindow(SW_SHOW);
				m_powersharelimiteditstatic.EnableWindow(m_iPowershare.GetPos() == 3);
				m_iPowershare.ShowWindow(SW_SHOW);
				m_iPowershare.EnableWindow(TRUE);				
				break;			
			case MISCELLANEOUS:				
				// [TPT] - SLUGFILLER: Spreadbars
				m_iSpreadBars.ShowWindow(SW_SHOW);
				m_iSpreadBars.EnableWindow(TRUE);
				// [TPT] - Select process priority
				m_iShowInMSN7.ShowWindow(SW_SHOW);
				m_iShowInMSN7.EnableWindow(TRUE);
			    m_selectPriorityProcess.ShowWindow(SW_SHOW);
				m_selectPriorityProcess.EnableWindow(TRUE);
				m_PriorityGroupBox.ShowWindow(SW_SHOW);
				m_PriorityGroupBox.EnableWindow(TRUE);
				m_HighPriority.ShowWindow(SW_SHOW);
				m_HighPriority.EnableWindow(TRUE);				
				m_NormalPriority.ShowWindow(SW_SHOW);
				m_NormalPriority.EnableWindow(TRUE);				
				m_IdlePriority.ShowWindow(SW_SHOW);
				m_IdlePriority.EnableWindow(TRUE);
				if (ExtraPrios)
				{
					m_RealTimePriority.ShowWindow(SW_SHOW);
					m_RealTimePriority.EnableWindow(TRUE);
					m_AboveNormalPriority.ShowWindow(SW_SHOW);
					m_AboveNormalPriority.EnableWindow(TRUE);
					m_BelowNormalPriority.ShowWindow(SW_SHOW);
					m_BelowNormalPriority.EnableWindow(TRUE);
				} 
			   // [TPT] - Credit System
				m_iCredits.ShowWindow(SW_SHOW);
				m_iCredits.EnableWindow(TRUE);	
				m_iCreditsGroupBox.ShowWindow(SW_SHOW);
				m_iCreditsGroupBox.EnableWindow(TRUE);	
				m_iCreditsPawcioStatic.ShowWindow(SW_SHOW);
				m_iCreditsPawcioStatic.EnableWindow(TRUE);	
				m_iCreditsLovelaceStatic.ShowWindow(SW_SHOW);
				m_iCreditsLovelaceStatic.EnableWindow(TRUE);	
				m_iCreditsOfficialStatic.ShowWindow(SW_SHOW);
				m_iCreditsOfficialStatic.EnableWindow(TRUE);	
				m_iCreditsNoneStatic.ShowWindow(SW_SHOW);
				m_iCreditsNoneStatic.EnableWindow(TRUE);	
				// [TPT] - Credit System
				break;
			case CATEGORIES:
				// [TPT] - khaos::categorymod
				m_iShowCatNames.ShowWindow(SW_SHOW);
				m_iShowCatNames.EnableWindow(TRUE);	
				m_iSelectCat.ShowWindow(SW_SHOW);
				m_iSelectCat.EnableWindow(TRUE);	
				m_iUseActiveCat.ShowWindow(SW_SHOW);
				m_iUseActiveCat.EnableWindow(TRUE);	
				m_iAutoSetResOrder.ShowWindow(SW_SHOW);
				m_iAutoSetResOrder.EnableWindow(TRUE);	
				m_iResumeFileInNewCat.ShowWindow(SW_SHOW);
				m_iResumeFileInNewCat.EnableWindow(TRUE);	
				m_iResumeFileInNewCatStatic.ShowWindow(SW_SHOW);
				m_iResumeFileInNewCatStatic.EnableWindow(TRUE);	
				m_iUseAutoCat.ShowWindow(SW_SHOW);
				m_iUseAutoCat.EnableWindow(TRUE);
				m_iShowPriorityInTab.ShowWindow(SW_SHOW);
				m_iShowPriorityInTab.EnableWindow(TRUE);
				break;
			case VISUALIZATION:
				// [TPT] - itsonlyme: displayOptions
				m_iShowFileSystemIcon.ShowWindow(SW_SHOW);
				m_iShowFileSystemIcon.EnableWindow(TRUE);		
				m_iShowLocalRating.ShowWindow(SW_SHOW);
				m_iShowLocalRating.EnableWindow(TRUE);		
				// [TPT] - IP Country
				m_iEnableShowCountryNames.ShowWindow(SW_SHOW);
				m_iEnableShowCountryNames.EnableWindow(TRUE);
				m_iEnableShowCountryFlags.ShowWindow(SW_SHOW);
				m_iEnableShowCountryFlags.EnableWindow(TRUE);
				// [TPT] - IP Country
				// [TPT] - MoNKi: -Support for High Contrast Mode-
				m_iHighContrast.ShowWindow(SW_SHOW);
				m_iHighContrast.EnableWindow(TRUE);		
				// emulEspaña: added by MoNKi [TPT] - [MoNKi: -invisible mode-]
				m_iBold.ShowWindow(SW_SHOW);
				m_iBold.EnableWindow(TRUE);
				m_iShowBackgroundInMenus.ShowWindow(SW_SHOW);
				m_iShowBackgroundInMenus.EnableWindow(TRUE);
				m_InvisibleModeGroupBox.ShowWindow(SW_SHOW);
				m_InvisibleModeGroupBox.EnableWindow(TRUE);
				m_iInvisibleMode.ShowWindow(SW_SHOW);
				m_iInvisibleMode.EnableWindow(TRUE);
				m_iInvisibleModeSelectStatic.ShowWindow(SW_SHOW);
				m_iInvisibleModeModifierStatic.ShowWindow(SW_SHOW);
				m_iInvisibleModeKeyStatic.ShowWindow(SW_SHOW);
				m_iInvisibleModePlus.ShowWindow(SW_SHOW);
				m_invmode_keymodifiers.ShowWindow(SW_SHOW);
				m_invmode_keys.ShowWindow(SW_SHOW);
				m_iShowUpPrio.ShowWindow(SW_SHOW);
				m_iShowUpPrio.EnableWindow(TRUE);
				if (thePrefs.GetInvisibleMode()){					
					m_iInvisibleModeSelectStatic.EnableWindow(TRUE);					
					m_iInvisibleModeModifierStatic.EnableWindow(TRUE);					
					m_iInvisibleModeKeyStatic.EnableWindow(TRUE);					
					m_iInvisibleModePlus.EnableWindow(TRUE);					
					m_invmode_keymodifiers.EnableWindow(TRUE);					
					m_invmode_keys.EnableWindow(TRUE);												
				}
				break;
			case MINIMULE:
				m_MiniMuleGroupBox.ShowWindow(SW_SHOW);
				m_MiniMuleGroupBox.EnableWindow(TRUE);					
				m_iMiniMule.ShowWindow(SW_SHOW);
				m_iMiniMule.EnableWindow(TRUE);	
				m_iMiniMuleLives.ShowWindow(SW_SHOW);
				m_iMiniMuleLives.EnableWindow(TRUE);	
				m_iMiniMuleUpdate.ShowWindow(SW_SHOW);
				m_iMiniMuleUpdate.EnableWindow(TRUE);	
				m_iMiniMuleUpdateStatic.ShowWindow(SW_SHOW);
				m_iMiniMuleUpdateStatic.EnableWindow(TRUE);
				m_MiniMuleTransparency.ShowWindow(SW_SHOW);
				m_MiniMuleTransparency.EnableWindow(TRUE);
				m_MiniMuleTransparencyStatic.ShowWindow(SW_SHOW);
				m_MiniMuleTransparencyStatic.EnableWindow(TRUE);
				m_MiniMuleTransparencyMinimum.ShowWindow(SW_SHOW);
				m_MiniMuleTransparencyMinimum.EnableWindow(TRUE);
				m_MiniMuleTransparencyMaximum.ShowWindow(SW_SHOW);
				m_MiniMuleTransparencyMaximum.EnableWindow(TRUE);
				break;

			case STATSSETUPINFO:
				m_zoomGroupBox.ShowWindow(SW_SHOW);
				m_zoomGroupBox.EnableWindow(TRUE);
				m_zoomStatic.ShowWindow(SW_SHOW);
				m_zoomStatic.EnableWindow(TRUE);
				m_zoomSlider.ShowWindow(SW_SHOW);
				m_zoomSlider.EnableWindow(TRUE);

				m_sampleRateGroupBox.ShowWindow(SW_SHOW);
				m_sampleRateGroupBox.EnableWindow(TRUE);
				m_sampleRateStatic.ShowWindow(SW_SHOW);
				m_sampleRateStatic.EnableWindow(TRUE);
				m_sampleRateSlider.ShowWindow(SW_SHOW);
				m_sampleRateSlider.EnableWindow(TRUE);

				m_memoryConsuming.ShowWindow(SW_SHOW);
				m_memoryConsuming.EnableWindow(TRUE);

				m_NAFCGraph.ShowWindow(SW_SHOW);
				m_NAFCGraph.EnableWindow(TRUE);

				m_OverheadGraph.ShowWindow(SW_SHOW);
				m_OverheadGraph.EnableWindow(TRUE);

				m_showVerticalLine.ShowWindow(SW_SHOW);
				m_showVerticalLine.EnableWindow(TRUE);
				break;
		}
	}
}

// [TPT] - quick start


// added by [TPT]-MoNKi [MoNKi: -invisible mode-]
void CPPgPhoenix::OnBnClickedInvisiblemode()
{	
	if(IsDlgButtonChecked(IDC_PPG_PHOENIX_INVISIBLE_MODE)){
		m_iInvisibleModeSelectStatic.EnableWindow(TRUE);
		m_iInvisibleModeModifierStatic.EnableWindow(TRUE);
		m_iInvisibleModeKeyStatic.EnableWindow(TRUE);
		m_iInvisibleModePlus.EnableWindow(TRUE);
		m_invmode_keymodifiers.EnableWindow(TRUE);
		m_invmode_keys.EnableWindow(TRUE);
	} else {
		m_iInvisibleModeSelectStatic.EnableWindow(FALSE);
		m_iInvisibleModeModifierStatic.EnableWindow(FALSE);
		m_iInvisibleModeKeyStatic.EnableWindow(FALSE);
		m_iInvisibleModePlus.EnableWindow(FALSE);
		m_invmode_keymodifiers.EnableWindow(FALSE);
		m_invmode_keys.EnableWindow(FALSE);
	}
	SetModified();
}

void CPPgPhoenix::OnCbnSelchangeKeymodcombo()
{
	CString sKeyMod;
	
	m_invmode_keymodifiers.GetLBText(m_invmode_keymodifiers.GetCurSel(), sKeyMod);
	m_iActualKeyModifier = 0;
	if (sKeyMod.Find(GetResString(IDS_CTRLKEY))!=-1)
		m_iActualKeyModifier |= MOD_CONTROL;
	if (sKeyMod.Find(GetResString(IDS_ALTKEY))!=-1)
		m_iActualKeyModifier |= MOD_ALT;
	if (sKeyMod.Find(GetResString(IDS_SHIFTKEY))!=-1)
		m_iActualKeyModifier |= MOD_SHIFT;
	
	SetModified();
}
// End [TPT]-MoNKi

void CPPgPhoenix::OnHelp()
{
	//theApp.ShowHelp(eMule_FAQ_Preferences_Extended_Settings);
}

BOOL CPPgPhoenix::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPPgPhoenix::OnHelpInfo(HELPINFO* pHelpInfo)
{
	OnHelp();
	return TRUE;
}