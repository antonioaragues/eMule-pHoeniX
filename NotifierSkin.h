#pragma once
#include "ini2.h"
#include "types.h"

#define CURRENT_SKINFILE_VERSION	0x01

class CNotifierSkin
{
public:
	CNotifierSkin(void);
	CNotifierSkin(const CNotifierSkin& itbn);
	~CNotifierSkin(void);

	//-----Load and Save methods-----------------------------------------------------
	//load and save must be overloaded
	//virtual bool Open(CString skinFile, bool readOnly);
	//virtual bool Save(CString skinFile);
	//-----End of Load and Save methods----------------------------------------------

	//-----Property access methods---------------------------------------------------
	//get
	/*CString GetSkinFile() {return m_strSkinFile;};
	CString GetVersion()  {return m_strVersion;};
	CString GetAuthor()	  {return m_strAuthor;};
	uint8   GetSkinFormatVersion()    {return m_nSkinFormatVersion;};*/

	COLORREF GetNormalTextColor()     {return m_colorNormalText;};
	COLORREF GetSelectedTextColor()   {return m_colorSelectedText;};
	COLORREF GetBmpTransparentColor() {return m_colorBmpTransparent;};

	CRect GetTextArea()		  {return m_rcTextArea;};
	CRect GetCloseBtnArea()	  {return m_rcCloseBtnArea;};
	CRect GetHistoryBtnArea() {return m_rcHistoryBtnArea;};

	ulong GetTimeToStay() {return m_dwTimeToStay;};
	ulong GetTimeToShow() {return m_dwTimeToShow;};
	ulong GetTimeToHide() {return m_dwTimeToHide;};

	CString GetFontType()    {return m_strFontType;};
	uint8   GetFontSize()    {return m_nFontSize;};
	CString GetBmpFileName() {return m_strBmpFileName;};

	bool GetReadOnly() {return m_bReadOnly;};
	bool isValid() {return m_bValidConfig;};

	//set
	//there is no SetSkinFile. Use Open(...) instead
	//void SetVersion(CString Version);
	//void SetAuthor(CString Author);
	//void SetSkinFormatVersion(uint8 SkinFormatVersion);
	
	void SetNormalTextColor(uint8 red, uint8 green, uint8 blue);
	void SetSelectedTextColor(uint8 red, uint8 green, uint8 blue);
	void SetBmpTransparentColor(uint8 red, uint8 green, uint8 blue);

	void SetTextArea(uint16 left, uint16 top, uint16 right, uint16 bottom);
	void SetCloseBtnArea(uint16 left, uint16 top, uint16 right, uint16 bottom);
	void SetHistoryBtnArea(uint16 left, uint16 top, uint16 right, uint16 bottom);

	void SetTimeToStay(ulong timeToStay);
	void SetTimeToShow(ulong timeToShow);
	void SetTimeToHide(ulong timeToHide);

	void SetFontType(CString fontType);
	void SetFontSize(uint8 fontSize);
	void SetBmpFileName(CString bmpFileName);

	//-----End of Property access methods--------------------------------------------
protected:
	//ValidConfig=indicates when opened configuration is valid
	bool m_bValidConfig;

	//SkinFile=full path of the .enk(Emule Notifier sKin) skin file
    CString m_strSkinFile;

	//ReadOnly=indicates if is possibile to alterate skin parameters
	bool m_bReadOnly;

	//-----Skin definition elements--------------------------------------------------
	//Version=Skin Version
    //CString m_strVersion;
	//Author=Skin Author
    //CString m_strAuthor;
    //SkinFileVersion=Used to identify skin version format
    //uint8 m_nSkinFormatVersion;	
	//NormalText[Red, Green, Blue]=Normal text color RGB components    	
	COLORREF m_colorNormalText;
	//SelectedText[Red, Green, Blue]=Selected text color RGB components
	COLORREF m_colorSelectedText;
	//bmpTransparent[Red, Green, Blue]=Transparent skin color components
	COLORREF m_colorBmpTransparent;
	//rcText[Left, Top, Right, Bottom]=Text area coordinates
	CRect m_rcTextArea;
    //rcCloseBtn[Left, Top, Right, Bottom]=Notifier close button skin sensible area
	CRect m_rcCloseBtnArea;
	//rcHistoryBtn[Left, Top, Right, Bottom]=Notifier Message History Button area
	CRect m_rcHistoryBtnArea;
    //TimeTo[Stay, Show, Hide]=Timings used for timer events
	ulong m_dwTimeToStay, m_dwTimeToShow, m_dwTimeToHide;
    //FontType=Font skin will use (i.e Arial, Tahoma, Verdana...)
	CString m_strFontType;
	//FontSize=recommended values > 40
	uint8 m_nFontSize;
	//bmpFileName=skin bitmap filename
	CString m_strBmpFileName;
	//-----END of Skin definition elements-------------------------------------------
};

