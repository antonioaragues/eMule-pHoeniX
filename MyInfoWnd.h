#pragma once
#include "MuleListCtrl.h"

//<<< eWombat [MYINFOWND]
class CMyInfoWnd : public CWnd
{
	DECLARE_DYNAMIC(CMyInfoWnd)

public:
	CMyInfoWnd();
	virtual ~CMyInfoWnd();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void Show(void);
	bool IsActive() {return m_bIsActive;}

protected:
	bool			m_bIsActive;
	HICON			m_hIcon;
	CMuleListCtrl	m_cList;

public:
	afx_msg void OnDestroy();
	
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

	void UpdateInfo(void);
	void UpdatePos(void);
	void FillMyInfo(CListCtrl* m_pList,bool bKeys=false);	//Fill MyInfoList in ServerWnd and MyInfoWnd
protected:
	bool SetTransparent(void);
	bool m_bIsDragging;
};
//>>> eWOmbat [MYINFOWND]

