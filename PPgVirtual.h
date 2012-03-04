// [TPT] - itsonlyme: virtualDirs

#pragma once

enum structTypes { IOM_VST_FILE, IOM_VST_DIR, IOM_VST_SUBDIR, IOM_VST_NEW };

struct VirtMapStruct {
	CString mapFrom;
	CString mapTo;
	CString fileID;
	structTypes type;
};

class CPPgVirtual : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgVirtual)

public:
	CPPgVirtual();
	~CPPgVirtual();

// Dialog Data
	enum { IDD = IDD_PPG_VIRTUAL };	

	void Localize(void);	

protected:
	CComboBox m_typesel;
	CListCtrl m_list;
	CList<VirtMapStruct *> structList;

	void FillList();

	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnNMClkList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedAdd();
	afx_msg void OnBnClickedApply();
	afx_msg void OnBnClickedRemove();
	afx_msg void OnNMRightClkList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSettingsChange() {SetModified();}
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);// [TPT] - New Menu Style
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
};

// [TPT] - itsonlyme: virtualDirs