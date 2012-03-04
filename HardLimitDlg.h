//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
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
#include "types.h"

class CHardLimitDlg : public CDialog
{
	DECLARE_DYNAMIC(CHardLimitDlg)

public:
	CHardLimitDlg(); // standard constructor
	virtual ~CHardLimitDlg();
	enum { IDD = IDD_HARDLIMITDLG }; // Dialog Data

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()

private:
	bool m_RestoreDefault;
	void SetWithDefaultValues(void);

public:
	virtual BOOL OnInitDialog(void);
	void LoadSettings(void);
	afx_msg	void OnBnClickedTakeOver();
	afx_msg	void OnBnClickedSwitch();	
	afx_msg	void OnBnClickedMaxSourcesPerFileTakeOver();
	void Localize();
};