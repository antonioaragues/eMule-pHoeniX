// [TPT] - NAFC Selection

#include "stdafx.h"
#include "SelNAFCDlg.h"
#include "emule.h"
#include "BandwidthControl.h"
#include "CxImage/xImage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CSelNAFCDlg dialog



IMPLEMENT_DYNAMIC(CSelNAFCDlg, CDialog)

CSelNAFCDlg::CSelNAFCDlg(CWnd* pWnd)
	: CDialog(CSelNAFCDlg::IDD, pWnd)
{
	selection = -1;
	m_icnWnd = NULL;

}

CSelNAFCDlg::~CSelNAFCDlg()
{
	if (m_icnWnd)
		VERIFY( DestroyIcon(m_icnWnd) );
}

void CSelNAFCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SELNAFCLIST2, m_nafcBox);
	DDX_Control(pDX, IDC_NAFC_ACEPT, m_nafcAccept);
	DDX_Control(pDX, IDC_NAFC_CANCEL, m_nafcCancel);
	DDX_Control(pDX, IDC_DINAMICNAFC, m_dinamicNafc);
}

BEGIN_MESSAGE_MAP(CSelNAFCDlg, CDialog)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_NAFC_ACEPT, OnAceptClick)
	ON_BN_CLICKED(IDC_NAFC_CANCEL, OnAceptClick)
END_MESSAGE_MAP()

BOOL CSelNAFCDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	InitWindowStyles(this);

	//AddAnchor(IDC_SELNAFCLIST2, TOP_LEFT, BOTTOM_RIGHT);
	//AddAnchor(IDC_NAFC_ACEPT, BOTTOM_RIGHT);
	//AddAnchor(IDC_NAFC_CANCEL, BOTTOM_RIGHT);
	SetWindowText(GetResString(IDS_NETWORKSELECTION));
	SetIcon(m_icnWnd = theApp.LoadIcon(_T("PHOENIX")), FALSE);

	m_dinamicNafc.SetWindowText(GetResString(IDS_DINAMICNAFC));
	m_dinamicNafc.SetCheck(thePrefs.GetDinamicNafc());

	m_nafcBox.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	//Añado Columnas
	m_nafcBox.InsertColumn(0, GetResString(IDS_DESCRIPTION), LVCFMT_LEFT, 200, -1); 
	m_nafcBox.InsertColumn(1, _T("Index"), LVCFMT_LEFT, 60, -1); 
	m_nafcBox.InsertColumn(2, GetResString(IDS_TYPE), LVCFMT_LEFT, 40, 1); 
	m_nafcBox.InsertColumn(3, _T("MTU"), LVCFMT_LEFT, 40, 1); 
	m_nafcBox.InsertColumn(4, _T("IP"), LVCFMT_LEFT, 100, 1);


	m_nafcBox.DeleteAllItems();

	InitializeTable();

	return TRUE;
}

void CSelNAFCDlg::InitializeTable()
{
	TCHAR buffer[65];	
	DWORD numEntries = theApp.pBandWidthControl->NafcNumber;
	for(DWORD i = 0; i < numEntries; i++)
	{
		m_nafcBox.InsertItem(LVIF_TEXT, i, theApp.pBandWidthControl->m_NAFCInfo[i].description, 0, 0, 1, NULL);
		m_nafcBox.SetItemText(i, 1, _itot(theApp.pBandWidthControl->m_NAFCInfo[i].index, buffer, 10));
		m_nafcBox.SetItemText(i, 2, _itot(theApp.pBandWidthControl->m_NAFCInfo[i].type, buffer, 10));
		m_nafcBox.SetItemText(i, 3, _itot(theApp.pBandWidthControl->m_NAFCInfo[i].mtu, buffer, 10));
		m_nafcBox.SetItemText(i, 4, ipstr(theApp.pBandWidthControl->m_NAFCInfo[i].ip));				
		if (theApp.pBandWidthControl->m_NAFCInfo[i].index == theApp.pBandWidthControl->GetSelectedIndex())
			m_nafcBox.SetItemState(i, LVIS_SELECTED, LVIS_GLOW);
	}
}

void CSelNAFCDlg::OnPaint()
{
	CPaintDC dc(this);
	CxImage cImage;
	CRect rc;

	cImage.LoadResource(FindResource(NULL,MAKEINTRESOURCE(IDR_LETRAS),_T("PNG")), CXIMAGE_FORMAT_PNG);
	rc.SetRect(10,10, 10 + cImage.GetWidth(), 10 + cImage.GetHeight());
	cImage.Draw(dc.m_hDC,rc);
}

void CSelNAFCDlg::OnAceptClick()
{
	
	int iSel = m_nafcBox.GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1)
	{
		POSITION pos = m_nafcBox.GetFirstSelectedItemPosition(); 
		selection = m_nafcBox.GetNextSelectedItem(pos); 
		
	}

	bool returnval = false;
	if(m_dinamicNafc.GetCheck() == BST_CHECKED)
		returnval = true;
	thePrefs.SetDinamicNafc(returnval);

	CDialog::OnOK();
}

void CSelNAFCDlg::OnCancelClick()
{
	CDialog::OnCancel();
}
