#pragma once
#include "DblScope.h"
#include "SnapDialog.h"
#include "sysinfo.h"

class CTBHMM : public CSnapDialog
{
	//DECLARE_DYNAMIC(CTBHMM)

public:
	CTBHMM(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTBHMM();
// Dialog Data
	enum { IDD = IDD_MINIMULE };
	CStatic	m_ctrlMMConnState;
	virtual		BOOL	OnCommand( WPARAM wParam, LPARAM lParam );
	void SetSpeedMeterValues(int iValue1, int iValue2)
	{
		m_ctrlSpeedMeter.AddValues(iValue1,iValue2);
	}
	void SetSpeedMeterRange(UINT nMax, UINT nMin)
	{
		m_ctrlSpeedMeter.SetRange(nMin, nMax);
	}
	void GetSpeedMeterRange(UINT& nMax, UINT& nMin)
	{
		m_ctrlSpeedMeter.GetRange(nMax, nMin);
	}
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnMenuButtonClicked();
	virtual BOOL OnInitDialog();
	CDblScope m_ctrlSpeedMeter;
	DECLARE_MESSAGE_MAP()
	HICON m_hCSConn;
	HICON m_hCSCing;
	HICON m_hCSDconn;
	uint32			m_nLastUpdate;
	CSysInfo sysinfo;

	// [TPT] - Improved minimule
	static UINT run(LPVOID p);
	void run();
	volatile BOOL running;
	// [TPT] - Improved minimule
	bool reset;
	void MMUpdate(void);

	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);// [TPT] - New Menu Style
public:
	void				DoMenu(CPoint doWhere);
	void				DoMenu(CPoint doWhere, UINT nFlags);
	void RunMiniMule(bool resetMiniMule = false);
	int smmin;
	int smmax;
private:
	// [TPT] - New Menu Styles
};
