#pragma once

#include "ResizableLib/ResizableSheet.h"
#include "ListViewWalkerPropertySheet.h"
#include "TreePropSheet.h"

// SLUGFILLER: modelessDialogs

// CModelessPropertySheet

class CModelessTreePropSheet : public CTreePropSheet {
	DECLARE_DYNAMIC(CModelessTreePropSheet)

public:
	CModelessTreePropSheet(BOOL bDeleteOnClose = FALSE);
	void OpenDialog();
	void CloseDialog();
	bool IsDialogOpen();

protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnOK();
	afx_msg void OnCancel();
	afx_msg void PostNcDestroy();
	DECLARE_MESSAGE_MAP()

private:
	BOOL			m_bActive;
	BOOL			m_bDeleteOnClose;
};

// CModelessPropertySheet

class CModelessPropertySheet : public CListViewWalkerPropertySheet {
	friend class CModelessPropertySheetInterface;
	DECLARE_DYNAMIC(CModelessPropertySheet)

public:
	CModelessPropertySheet(CListCtrlItemWalk* pListCtrl, BOOL bDeleteOnClose = TRUE);
	~CModelessPropertySheet();
	void OpenDialog();
	void CloseDialog();

	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnOK();
	afx_msg void OnCancel();
	afx_msg void PostNcDestroy();
	DECLARE_MESSAGE_MAP()

private:
	void RemoveData(CObject* toremove);
	BOOL			m_bActive;
	BOOL			m_bDeleteOnClose;
	CSimpleArray<CModelessPropertySheetInterface*> m_interfaces;
};

// CModelessPropertySheetInterface

class CModelessPropertySheetInterface : public CObject {
	friend class CModelessPropertySheet;
	DECLARE_DYNAMIC(CModelessPropertySheetInterface)

public:
	CModelessPropertySheetInterface(CObject* owner);
	~CModelessPropertySheetInterface();
	bool IsDialogOpen() const;

protected:
	void OpenPropertySheet(const CSimpleArray<CModelessPropertySheetInterface*>* paOthers, ...);
	virtual CModelessPropertySheet* CreatePropertySheet(va_list) = 0;
	CObject* GetOwner() const;
	int GetPropertySheetCount() const;
	CModelessPropertySheet* GetPropertySheet(int i) const;

private:
	CObject* m_owner;
	CSimpleArray<CModelessPropertySheet*> m_propertySheets;
};
