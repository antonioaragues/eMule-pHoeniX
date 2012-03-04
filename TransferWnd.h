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
#include "ResizableLib\ResizableDialog.h"
#include "SplitterControl.h"
#include "BtnST.h"
#include "TabCtrl.hpp"
#include "UploadListCtrl.h"
#include "DownloadListCtrl.h"
#include "QueueListCtrl.h"
#include "ClientListCtrl.h"
#include "downloadclientsctrl.h"// [TPT] - TBH Transfers Window Style
#include "ToolTips\PPToolTip.h" // [TPT] - MFCK [addon] - New Tooltips [Rayita]
// [TPT] - MFCK [addon] - New Tooltips [Rayita]
#define UPLOAD_WND		0
#define	QUEUE_WND		1
#define KNOWN_WND		2
#define TRANSF_WND		3
#define UPDOWN_WND		4
#define DOWNLOAD_WND	5
// [TPT] - MFCK [addon] - New Tooltips [Rayita]

class CUploadListCtrl;
class CDownloadListCtrl;
class CQueueListCtrl;
class CClientListCtrl;

class CTransferWnd : public CResizableDialog
{
	friend class CSelCategoryDlg; // [TPT] - khaos::categorymod+
	
	DECLARE_DYNAMIC(CTransferWnd)
public:
	CTransferWnd(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTransferWnd();

	void ShowQueueCount(uint32 number);
	void UpdateListCount(uint8 listindex, int iCount = -1);
	void Localize();
	void UpdateCatTabTitles(bool force=true);
	void	UpdateDownloadClientsCount(int count); // [TPT] - TBH Transfers Window Style
	bool 	isUploadQueueVisible() { return (m_uWnd2 == 0); }// [TPT] - TBH Transfers Window Style
	bool	isDownloadListVisible() { return ((showlist == IDC_DOWNLOADLIST) || (showlist == IDC_DOWNLOADLIST+IDC_UPLOADLIST)); } // [TPT] - TransferWindow Fix
	void VerifyCatTabSize();
	void SwitchUploadList();
	void SetDlgItemFocus(int nID);// [TPT] - MFCK [addon] - New Tooltips [Rayita]	
	// [TPT] - khaos::categorymod+
	int		GetCategoryTab()			{ return m_dlTab.GetCurSel(); }
	int		GetActiveCategory()			{ return m_dlTab.GetCurSel(); }
	// [TPT] - khaos::categorymod-
	// [TBT]
	void SetActiveCategory(int index)    { m_dlTab.SetCurSel(index); }
	// [TBT]

	// Dialog Data
	enum { IDD = IDD_TRANSFER };
	CUploadListCtrl		uploadlistctrl;
	CDownloadListCtrl	downloadlistctrl;
	CQueueListCtrl		queuelistctrl;
	CClientListCtrl		clientlistctrl;
	CDownloadClientsCtrl	downloadclientsctrl;// [TPT] - TBH Transfers Window Style
	//CToolTipCtrl m_tooltipCats;// [TPT] - MFCK [addon] - New Tooltips [Rayita]
	bool bQl;
	bool bKl;


protected:
	CSplitterControl m_wndSplitter;
	uint8 m_uWnd2;
	bool downloadlistactive;
	
	// [TPT] - TBH Transfers Window Style
	void OnBnClickedChangeView();
	void OnBnClickedDownUploads();
		
	CButtonST m_uplBtn;
	TabControl m_dlTab;
	int	rightclickindex;
	int m_nDragIndex;
	int m_nDropIndex;
	int m_nLastCatTT;
	int	m_isetcatmenu;
	bool m_bIsDragging;
	CImageList* m_pDragImage;
	HICON icon_download;
	POINT m_pLastMousePoint;
	// [TPT] - TBH Transfers Window Style
	CButtonST	m_btnChangeView;
	CButtonST	m_btnDownUploads;
	CButtonST	m_btnDownloads;
	CButtonST	m_btnUploads;
	CButtonST	m_btnQueue;
	CButtonST	m_btnTransfers;
	CButtonST	m_btnClient;
	uint16 showlist;	
	void ShowList(uint16 list);
	//[TPT] - Switch Lists Icons
	CButtonST	m_btnULChangeView;
	CButtonST	m_btnULUploads;
	CButtonST	m_btnULQueue;
	CButtonST	m_btnULTransfers;
	CButtonST	m_btnULClients;	
	//[TPT] - Switch Lists Icons

	void ChangeDlIcon(int wndToChange);
	void ShowWnd2(uint8 uList);
	void SetWnd2(uint8 uWnd2);
	void DoResize(int delta);
	void UpdateSplitterRange();
	void SetInitLayout();
	void DoSplitResize(int delta);
	void SetAllIcons();
	void SetWnd2Icon();
	// [TPT] - MFCK [addon] - New Tooltips [Rayita]
	//void UpdateTabToolTips() {UpdateTabToolTips(-1);}
	//void UpdateTabToolTips(int tab);
	//CString GetTabStatistic(uint8 tab);
	// [TPT] - MFCK [addon] - New Tooltips [Rayita]
	int GetTabUnderMouse(CPoint* point);
	//int GetItemUnderMouse(CListCtrl* ctrl); // [TPT] - MFCK [addon] - New Tooltips [Rayita]
	CString GetCatTitle(int catid);
	int AddCategory(CString newtitle,CString newincoming,CString newcomment,CString newautocat,bool addTab=true);
	void EditCatTabLabel(int index,CString newlabel);
	void EditCatTabLabel(int index);

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	
	DECLARE_MESSAGE_MAP()
	afx_msg void OnHoverUploadList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHoverDownloadList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTcnSelchangeDltab(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRclickDltab(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTabMovement(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus); //<<-- [TPT] - enkeyDEV(th1) -notifier-	
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLvnKeydownDownloadlist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSysColorChange();
	afx_msg void OnDblclickDltab();
	afx_msg void OnBnClickedQueueRefreshButton();
// [TPT] - MFCK [addon] - New Tooltips [Rayita]
private:
	CMuleListCtrl* lists_list[6];	
	CImageList m_ImageList;
	CPPToolTip m_ttip;
	CPPToolTip m_tabtip;
	CPPToolTip m_othertips;
	CPPToolTip m_btttp;
	void UpdateTabToolTips();
	void UpdateToolTips();
	int m_iOldToolTipItem[DOWNLOAD_WND];
	int GetClientImage(CUpDownClient* client);
protected:
	afx_msg BOOL OnToolTipNotify(UINT id, NMHDR *pNMH, LRESULT *pResult);
public:
	CImageList* GetImageList() { return &m_ImageList; }
	void DrawClientImage(CDC *dc, POINT &point, CUpDownClient* client);
	void SetTTDelay();
// [TPT] - MFCK [addon] - New Tooltips [Rayita]
};
