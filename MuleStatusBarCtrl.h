#pragma once

enum EStatusBarPane
{
	SBarLog = 0,
	SBarUsers,
	SBarUpDown,
	SBarConnected,
	SBarChatMsg
};


// [TPT] - MFCK [addon] - New Tooltips [Rayita]
#	define SB_LOG			0
#	define SB_MSG			4
#	define SB_UP_SPEED		1
#	define SB_DN_SPEED		2
#	define SB_SERVER		3
// [TPT] - MFCK [addon] - New Tooltips [Rayita]

class CMuleStatusBarCtrl : public CStatusBarCtrl
{
	DECLARE_DYNAMIC(CMuleStatusBarCtrl)

public:
	CMuleStatusBarCtrl();
	virtual ~CMuleStatusBarCtrl();

	void Init(void);

protected:
	int GetPaneAtPosition(CPoint& point) const;
	// [TPT] - MFCK [addon] - New Tooltips [Rayita]
	//CString GetPaneToolTipText(EStatusBarPane iPane) const; 

	//virtual int OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
	// [TPT] - MFCK [addon] - New Tooltips [Rayita]

	DECLARE_MESSAGE_MAP()
	afx_msg void OnLButtonDblClk(UINT nFlags,CPoint point);
// [TPT] - MFCK [addon] - New Tooltips [Rayita]
public:
	HICON GetTipInfo(CString &info); // added by rayita
// [TPT] - MFCK [addon] - New Tooltips [Rayita]
};
