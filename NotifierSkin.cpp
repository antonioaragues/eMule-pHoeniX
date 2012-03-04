#include "StdAfx.h"
#include "notifierskin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CNotifierSkin::CNotifierSkin(void)
{	
	m_bReadOnly=false;
    m_strSkinFile="";
//    m_strVersion="";	
//    m_strAuthor="N/D";    
//    m_nSkinFormatVersion=CURRENT_SKINFILE_VERSION;
	m_bValidConfig=false;
	m_colorNormalText=RGB(255,255,255);	
	m_colorSelectedText=RGB(0,0,0);
	m_colorBmpTransparent=RGB(255,0,255);	
	m_rcTextArea.SetRectEmpty();
	m_rcCloseBtnArea.SetRectEmpty();	
	m_rcHistoryBtnArea.SetRectEmpty();    
	m_dwTimeToStay=4000;
	m_dwTimeToShow=500;
	m_dwTimeToHide=200;    
	m_strFontType="Arial";	
	m_nFontSize=70;	
	m_strBmpFileName="";
}

CNotifierSkin::CNotifierSkin(const CNotifierSkin& itbn) {
	//m_nIniVersion=itbn.;
	m_bValidConfig=itbn.m_bValidConfig;
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

CNotifierSkin::~CNotifierSkin(void)
{
}

//-----Property access methods---------------------------------------------------
/*void CNotifierSkin::SetVersion(CString Version)
{ 
	if (!m_bReadOnly) {
		if (Version.IsEmpty())
			m_strVersion="N/D";
		else
			m_strVersion=Version;
	}
}//SetVersion

void CNotifierSkin::SetAuthor(CString Author)
{
	if (!m_bReadOnly) {
		if (Author.IsEmpty())
			m_strAuthor="N/D";
		else
			m_strAuthor=Author;
	}
}//SetAuthor

void CNotifierSkin::SetSkinFormatVersion(uint8 SkinFormatVersion)
{
	if (!m_bReadOnly) m_nSkinFormatVersion=SkinFormatVersion;
} //SkinFormatVersion
*/

void CNotifierSkin::SetNormalTextColor(uint8 red, uint8 green, uint8 blue)
{
	if (!m_bReadOnly) {
		m_colorNormalText=RGB(red, green, blue);
	}
} //SetNormalTextColor

void CNotifierSkin::SetSelectedTextColor(uint8 red, uint8 green, uint8 blue)
{
	if (!m_bReadOnly) { 
		m_colorSelectedText=RGB(red, green, blue);
	}
} //SetSelectedTextColor

void CNotifierSkin::SetBmpTransparentColor(uint8 red, uint8 green, uint8 blue)
{
	if (!m_bReadOnly) { 
		m_colorBmpTransparent=RGB(red, green, blue);
	}
} //SetBmpTransparentColor

void CNotifierSkin::SetTextArea(uint16 left, uint16 top, uint16 right, uint16 bottom) 
{
	if (!m_bReadOnly) { 
		m_rcTextArea.SetRect(left, top, right, bottom);
	}
}//SetTextArea

void CNotifierSkin::SetCloseBtnArea(uint16 left, uint16 top, uint16 right, uint16 bottom)
{
	if (!m_bReadOnly) { 
		m_rcCloseBtnArea.SetRect(left, top, right, bottom);
	}
}//SetCloseBtnArea

void CNotifierSkin::SetHistoryBtnArea(uint16 left, uint16 top, uint16 right, uint16 bottom)
{
	if (!m_bReadOnly) { 
		m_rcHistoryBtnArea.SetRect(left, top, right, bottom);
	}
}//SetHistoryBtnArea

void CNotifierSkin::SetTimeToStay(ulong timeToStay)
{
	if (!m_bReadOnly) {
		ASSERT(timeToStay > 0);
		m_dwTimeToStay=timeToStay;
	}
}//SetTimeToStay

void CNotifierSkin::SetTimeToShow(ulong timeToShow)
{
	if (!m_bReadOnly) {
		ASSERT(timeToShow > 0);
		m_dwTimeToShow=timeToShow;
	}
}//SetTimeToShow

void CNotifierSkin::SetTimeToHide(ulong timeToHide)
{
	if (!m_bReadOnly) {
		ASSERT(timeToHide > 0);
		m_dwTimeToHide=timeToHide;
	}
}//SetTimeToHide

void CNotifierSkin::SetFontType(CString fontType)
{
	if (!m_bReadOnly) {
		ASSERT(!fontType.IsEmpty());
		m_strFontType=fontType;
	}
}//SetFontType

void CNotifierSkin::SetFontSize(uint8 fontSize)
{
	if (!m_bReadOnly) {
		ASSERT(fontSize > 0);
		m_nFontSize=fontSize;
	}
}//SetFontSize

void CNotifierSkin::SetBmpFileName(CString bmpFileName)
{
	if (!m_bReadOnly) {
		ASSERT(!bmpFileName.IsEmpty());
		m_strBmpFileName=bmpFileName;
	}
}//SetBmpFileName

//-----End of Property access methods--------------------------------------------
