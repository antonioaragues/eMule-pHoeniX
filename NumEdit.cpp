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

#include "stdafx.h"
#include "NumEdit.h"
// [TPT] - emulEspaña: added by MoNKi
#include "emule.h"
#include "locale.h"
#include "numedit.h"
// [TPT] - end emulEspaña

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CNumEdit

IMPLEMENT_DYNAMIC(CNumEdit, CEdit)
CNumEdit::CNumEdit()
{
}

CNumEdit::~CNumEdit()
{
}

BEGIN_MESSAGE_MAP(CNumEdit, CEdit)
	ON_WM_CHAR()
	ON_MESSAGE(WM_PASTE,OnPaste)
	ON_WM_KILLFOCUS() // [TPT] - emulEspaña: added by MoNKi [MoNKi: -OnKillFocus support in CNumEdit-]
END_MESSAGE_MAP()

// CNumEdit message handlers

void CNumEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) {
	// [TPT] - MoNKi: -i18n support in CNumEdit-
	// Check new Char
	if(nChar ==  VK_BACK || (nChar >= '0' && nChar <= '9')){
		// Foward char
		CEdit::OnChar(nChar, nRepCnt, nFlags);
	}
	else {
		struct lconv *lconvStruct;		
		lconvStruct = localeconv();

		// modified by Announ: numeric keyboard '.' counts as decimal point
		if ( nChar == VK_DELETE )
		{
			_AFX_THREAD_STATE* pThreadState = _afxThreadState.GetData();
			nChar = pThreadState->m_lastSentMsg.wParam = (UINT)*lconvStruct->decimal_point;
		}
		// End Announ

		if ( char(nChar) == *lconvStruct->decimal_point )
		{
			// Check if those chars have been already used
			CString buffer;
			GetWindowText(buffer);
			// modified by Announ
			int nStart, nEnd;
			GetSel(nStart, nEnd);
			int nDecPointPos = buffer.Find(CString(lconvStruct->decimal_point));
			if ( nDecPointPos == -1 ||
				( nStart >= 0 && nDecPointPos >= nStart && nDecPointPos < nEnd ))
				// End Announ
			{
				// Foward char
				CEdit::OnChar(nChar, nRepCnt, nFlags);
			}
		}
	}
	// [TPT] - MoNKi: -i18n support in CNumEdit-
}

LONG CNumEdit::OnPaste(UINT wParam, LONG lParam)
{
	// [TPT] - MoNKi: -OnPaste support in CNumEdit-
	// Using code from Tom Archer:
	// http://www.codeproject.com/clipboard/archerclipboard1.asp
	if (OpenClipboard()){
		if (::IsClipboardFormatAvailable(CF_TEXT)
		|| ::IsClipboardFormatAvailable(CF_OEMTEXT)){
			// Retrieve the Clipboard data (specifying that 
			// we want ANSI text (via the CF_TEXT value).
			HANDLE hClipboardData = GetClipboardData(CF_TEXT);

			// Call GlobalLock so that to retrieve a pointer
			// to the data associated with the handle returned
			// from GetClipboardData.
			TCHAR *pchData = (TCHAR*)GlobalLock(hClipboardData);

			CString valueStr, curText, cbText;
			int startChar, endChar;
			struct lconv *lconvStruct;		

			lconvStruct = localeconv();

			//Checks the string and paste the result
			cbText = pchData;
			GetSel(startChar,endChar);
			GetWindowText(curText);

			valueStr = "";
			int i=0, decPos=0;
			TCHAR c;
			bool dec = false;
			while((i < cbText.GetLength()) &&
				(((c = cbText.GetAt(i)) >= '0' && c <= '9') || (c == lconvStruct->decimal_point[0] && !dec)))
			{
				valueStr += c;
				if(c == lconvStruct->decimal_point[0]){
					dec = true;
					decPos = i;
				}
				i++;
			}

			cbText = valueStr;
			if(!cbText.IsEmpty()){
				if(dec){
					int nDecPointPos = curText.Find(CString(lconvStruct->decimal_point));

					if (!(nDecPointPos == -1 ||
						(nDecPointPos >= startChar && nDecPointPos < endChar)))
					{
						cbText = cbText.Left(decPos);
					}
				}

				valueStr.Format(_T("%s%s%s"),curText.Left(startChar),cbText,curText.Right(curText.GetLength()-endChar));
			SetWindowText(valueStr);

				// Set cursor position
				startChar += cbText.GetLength();
				SetSel(startChar, startChar);
			}

			// Unlock the global memory.
			GlobalUnlock(hClipboardData);
		}
		// Finally, when finished I simply close the Clipboard
		// which has the effect of unlocking it so that other
		// applications can examine or modify its contents.
		CloseClipboard();
	}
	// [TPT] - MoNKi: -OnPaste support in CNumEdit-
	return 0L;
}

// [TPT] - emulEspaña: added by MoNKi [MoNKi: -OnKillFocus support in CNumEdit-]
void CNumEdit::OnKillFocus(CWnd* pNewWnd)
{
	CEdit::OnKillFocus(pNewWnd);

	CString curText;
	float fValue;
	int startChar, endChar;

	GetSel(startChar,endChar);
	GetWindowText(curText);
	fValue = _tstof(curText);
	curText.Format(_T("%.1f"), fValue);
	SetWindowText(curText);
	SetSel(startChar, endChar);
}
// [TPT] - End emulEspaña
