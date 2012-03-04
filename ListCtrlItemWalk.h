#pragma once

class CListCtrlItemWalk
{
public:
	CListCtrlItemWalk(CListCtrl* pListCtrl) { m_pListCtrl = pListCtrl; }

	// [TPT] - SLUGFILLER: modelessDialogs - account for multiple dialogs
	virtual CObject* GetNextSelectableItem(CObject* pCurrentObj);
	virtual CObject* GetPrevSelectableItem(CObject* pCurrentObj);
	// [TPT] - SLUGFILLER: modelessDialogs

	CListCtrl* GetListCtrl() const { return m_pListCtrl; }

protected:
	CListCtrl* m_pListCtrl;
};
