#pragma once
//#include "TreeOptionsCtrlEx.h"

class CPPgWebCache : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgWebCache)

public:
	CPPgWebCache();
	virtual ~CPPgWebCache();

// Dialog Data
	enum { IDD = IDD_PPG_WEBCACHE };

	void Localize(void);

protected:
	bool m_bEnableDownloadLimit;
	CString m_strProxyAddress;
	int m_iProxyPort;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnKillActive();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam);
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
};
