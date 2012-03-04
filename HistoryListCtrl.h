//this file is part of eMule
//Copyright (C)2002 Merkur ( devs@emule-project.net / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#pragma once

#include "emule.h"
#include "types.h"
#include "MuleListCtrl.h"
#include "ListCtrlItemWalk.h"
#include "Modeless.h"	// SLUGFILLER: modelessDialogs

// CHistoryListCtrl

class CHistoryListCtrl : public CMuleListCtrl, public CListCtrlItemWalk
{
	DECLARE_DYNAMIC(CHistoryListCtrl)

public:
	CHistoryListCtrl();
	virtual ~CHistoryListCtrl();
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	void	Init(void);
	void	AddFile(CKnownFile* toadd);
	void	Localize();
	//void	CreateMenues(); // [TPT] - New Menu Styles
	void	Reload(void);
	void	ShowComments(CKnownFile* file);
	void	RemoveFile(CKnownFile* toremove);	
	void	ClearHistory();	
	void	SetDoubleBufferStyle();//[TPT] - Double buffer style in lists
	void	UpdateFile(const CKnownFile* file);

protected:
	DECLARE_MESSAGE_MAP()

	afx_msg	void OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);// [TPT] - New Menu Style
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	static int CALLBACK SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	void ShowFileDialog(CTypedPtrList<CPtrList, CKnownFile*>& aFiles, UINT uPshInvokePage = 0);
	int FindFile(const CKnownFile* pFile);

private:
	void		OpenFile(CKnownFile* file);
};

// SLUGFILLER: modelessDialogs
class CHistoryListDetailsSheetInterface : public CModelessPropertySheetInterface
{
public:
	CHistoryListDetailsSheetInterface(CKnownFile* owner);
	void	OpenDetailsSheet(CTypedPtrList<CPtrList, CKnownFile*>& aFiles, UINT uPshInvokePage = 0, CListCtrlItemWalk* pListCtrl = NULL);

protected:
	virtual CModelessPropertySheet* CreatePropertySheet(va_list);
};
// SLUGFILLER: modelessDialogs
