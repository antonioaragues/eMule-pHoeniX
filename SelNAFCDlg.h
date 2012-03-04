//[TPT] - NAFC Selection

#include "emuleDlg.h"
#include "MuleListCtrl.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSelNAFCDlg : public CDialog
{
	DECLARE_DYNAMIC(CSelNAFCDlg)

public:
	CSelNAFCDlg(CWnd* pWnd = NULL);
	virtual	~CSelNAFCDlg();

	virtual BOOL	OnInitDialog();
	void InitializeTable();
	afx_msg void OnAceptClick();
	afx_msg void OnCancelClick();
	afx_msg void OnPaint();

// Dialog Data
	enum { IDD = IDD_SELECNAFCDLG };
	int selection;
	CMuleListCtrl			m_nafcBox;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
	HICON m_icnWnd;
private:
	CButton			m_dinamicNafc;
	CButton			m_nafcAccept;
	CButton			m_nafcCancel;
};