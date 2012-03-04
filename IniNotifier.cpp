#include "StdAfx.h"
#include "ininotifier.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CIniNotifier::CIniNotifier(void)
{
}

CIniNotifier::~CIniNotifier(void)
{
}

CIniNotifier::CIniNotifier(const CIniNotifier& itbn) {
	//m_nIniVersion=itbn.;
	m_bValidConfig=itbn.m_bValidConfig;
	m_strSkinDir=itbn.m_strSkinDir;
	m_colorNormalText=itbn.m_colorNormalText;	
	m_colorSelectedText=itbn.m_colorSelectedText;	
	m_colorBmpTransparent=itbn.m_colorBmpTransparent;	
	m_rcTextArea=itbn.m_rcTextArea;    
	m_rcCloseBtnArea=itbn.m_rcCloseBtnArea;	
	m_rcHistoryBtnArea=itbn.m_rcHistoryBtnArea;    
	m_dwTimeToStay=itbn.m_dwTimeToStay;
	m_dwTimeToShow=itbn.m_dwTimeToShow;
	m_dwTimeToHide=itbn.m_dwTimeToHide;   
	m_strFontType=itbn.m_strFontType;
	m_nFontSize=itbn.m_nFontSize;
	m_strBmpFileName=itbn.m_strBmpFileName;
}

bool CIniNotifier::Open(CString skinFile, bool readOnly) 
{
	if (skinFile.GetLength() > 255 || skinFile.IsEmpty()) {
		m_bValidConfig = false;
		return false;
	}
		
    SetFileName(skinFile);
    SetSection(_T("CONFIG"));

	m_strSkinDir = skinFile.Left(skinFile.ReverseFind('\\'));	

	LoadConfiguration();
	m_bValidConfig=true;
	return true;
}

bool CIniNotifier::Save(CString skinFile)
{
	if (GetReadOnly()) return false;

    ASSERT(skinFile);
	SetFileName(skinFile);
	SetSection(_T("CONFIG"));
	SaveConfiguration();
	return false;
}

void CIniNotifier::LoadConfiguration()
{	
	uint8 nRed, nGreen, nBlue, sRed, sGreen, sBlue;
	uint16 rcLeft, rcTop, rcRight, rcBottom;	
	uint8 bmpTrasparentRed, bmpTrasparentGreen, bmpTrasparentBlue;
	CString fontType, bmpFullPath;

	nRed   = GetInt(_T("TextNormalRed"),255);
	nGreen = GetInt(_T("TextNormalGreen"),255);
	nBlue  = GetInt(_T("TextNormalBlue"),255);

	SetNormalTextColor(nRed, nGreen, nBlue);

	sRed   = GetInt(_T("TextSelectedRed"),255);
	sGreen = GetInt(_T("TextSelectedGreen"),255);
	sBlue  = GetInt(_T("TextSelectedBlue"),255);

	SetSelectedTextColor(sRed, sGreen, sBlue);

	bmpTrasparentRed   = GetInt(_T("bmpTrasparentRed"),255);
	bmpTrasparentGreen = GetInt(_T("bmpTrasparentGreen"),0);
	bmpTrasparentBlue  = GetInt(_T("bmpTrasparentBlue"),255);

	SetBmpTransparentColor(bmpTrasparentRed, bmpTrasparentGreen, bmpTrasparentBlue);

	SetFontSize(GetInt(_T("TextFontSize"),70));
	SetTimeToStay(GetInt(_T("TimeToStay"), 4000));
	SetTimeToShow(GetInt(_T("TimeToShow"), 500));
	SetTimeToHide(GetInt(_T("TimeToHide"), 200));
	//TODO: cannot pass GetString as SetFontType parameter: it cause a crash but only in release build. why??
	fontType=GetString(_T("FontType"), _T("Arial"));
	SetFontType(fontType);
	bmpFullPath.Format(_T("%s\\%s"), m_strSkinDir, GetString(_T("bmpFileName"), _T("")));
	SetBmpFileName(bmpFullPath);

	// get text rectangle coordinates
	
	rcLeft = GetInt(_T("rcTextLeft"),1);
	rcTop  = GetInt(_T("rcTextTop"),1);	
	rcRight  = GetInt(_T("rcTextRight"), 1);
	rcBottom = GetInt(_T("rcTextBottom"), 1);

	SetTextArea(rcLeft,rcTop,rcRight,rcBottom);

	// get close button rectangle coordinates
	rcLeft = GetInt(_T("rcCloseBtnLeft"),1);
	rcTop  = GetInt(_T("rcCloseBtnTop"),1);	
	rcRight  = GetInt(_T("rcCloseBtnRight"), 1);
	rcBottom = GetInt(_T("rcCloseBtnBottom"), 1);

	SetCloseBtnArea(rcLeft,rcTop,rcRight,rcBottom);
	
	// get history button rectangle coordinates
	rcLeft = GetInt(_T("rcHistoryBtnLeft"),1);
	rcTop  = GetInt(_T("rcHistoryBtnTop"),1);	
	rcRight  = GetInt(_T("rcHistoryBtnRight"), 1);
	rcBottom = GetInt(_T("rcHistoryBtnBottom"), 1);

	SetHistoryBtnArea(rcLeft,rcTop,rcRight,rcBottom);
}

void CIniNotifier::SaveConfiguration()
{	
	WriteInt(_T("TextNormalRed"),GetRValue(GetNormalTextColor()));
	WriteInt(_T("TextNormalGreen"),GetGValue(GetNormalTextColor()));
	WriteInt(_T("TextNormalBlue"),GetBValue(GetNormalTextColor()));	

	WriteInt(_T("TextSelectedRed"),GetRValue(GetSelectedTextColor()));
	WriteInt(_T("TextSelectedGreen"),GetGValue(GetSelectedTextColor()));
	WriteInt(_T("TextSelectedBlue"),GetBValue(GetSelectedTextColor()));

	WriteInt(_T("bmpTrasparentRed"),GetRValue(GetBmpTransparentColor()));
	WriteInt(_T("bmpTrasparentGreen"),GetGValue(GetBmpTransparentColor()));
	WriteInt(_T("bmpTrasparentBlue"),GetBValue(GetBmpTransparentColor()));	

	WriteInt(_T("TextFontSize"),GetFontSize());
	WriteInt(_T("TimeToStay"), GetTimeToStay());
	WriteInt(_T("TimeToShow"), GetTimeToShow());
	WriteInt(_T("TimeToHide"), GetTimeToHide());
	WriteString(_T("FontType"), GetFontType());
	WriteString(_T("bmpFileName"), GetBmpFileName());		

	WriteInt(_T("rcTextLeft"),GetTextArea().TopLeft().x);
	WriteInt(_T("rcTextTop"),GetTextArea().TopLeft().y);	
	WriteInt(_T("rcTextRight"), GetTextArea().BottomRight().x);
	WriteInt(_T("rcTextBottom"), GetTextArea().BottomRight().y);

	WriteInt(_T("rcCloseBtnLeft"),GetCloseBtnArea().TopLeft().x);
	WriteInt(_T("rcCloseBtnTop"),GetCloseBtnArea().TopLeft().y);	
	WriteInt(_T("rcCloseBtnRight"), GetCloseBtnArea().BottomRight().x);
	WriteInt(_T("rcCloseBtnBottom"), GetCloseBtnArea().BottomRight().y);

	WriteInt(_T("rcHistoryBtnLeft"),GetHistoryBtnArea().TopLeft().x);
	WriteInt(_T("rcHistoryBtnTop"),GetHistoryBtnArea().TopLeft().y);	
	WriteInt(_T("rcHistoryBtnRight"), GetHistoryBtnArea().BottomRight().x);
	WriteInt(_T("rcHistoryBtnBottom"), GetHistoryBtnArea().BottomRight().y);
}
