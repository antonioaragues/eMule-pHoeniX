//<<< eWombat [MYINFOWND]
// MyInfoWnd.cpp : Implementierungsdatei
//

#include "stdafx.h"
#include "emule.h"
#include "MyInfoWnd.h"
#include "emuledlg.h"
#include "sockets.h"
#include "TransferWnd.h"
#include "server.h"
#include "MuleToolbarCtrl.h"

// CMyInfoWnd

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CMyInfoWnd, CWnd)
CMyInfoWnd::CMyInfoWnd()
{
	m_bIsActive=false;
	m_bIsDragging=false;
}

CMyInfoWnd::~CMyInfoWnd()
{
}


BEGIN_MESSAGE_MAP(CMyInfoWnd, CWnd)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_WM_DESTROY()	
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN( )
END_MESSAGE_MAP()



// CMyInfoWnd-Meldungshandler
void CMyInfoWnd::OnClose()
{
	m_bIsActive=false;
	if (m_hIcon)
		DestroyIcon(m_hIcon);
    CWnd::OnClose();
}
void CMyInfoWnd::OnDestroy()
{
	m_bIsActive=false;
	if (m_hIcon)
		DestroyIcon(m_hIcon);
	CWnd::OnDestroy();
}

void CMyInfoWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	if (m_cList.GetSafeHwnd())
		{
		CRect rc;
		GetClientRect(&rc);
		m_cList.SetWindowPos(&wndTop,rc.left,rc.top,rc.Width(),rc.Height(),SWP_NOZORDER);
		}

}

void CMyInfoWnd::Show(void)
{
if (!m_bIsActive)
	{
	//Create Window
		// register window class:
	CString strClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, LoadCursor(NULL, IDC_ARROW),CBrush(::GetSysColor(COLOR_WINDOW)));

	CWnd* pParent = AfxGetApp()->GetMainWnd();
	CRect rc,rect;
	pParent->GetWindowRect(&rect);
	theApp.emuledlg->transferwnd->GetWindowRect(&rc);
	pParent->ClientToScreen(&rc);
	rc.bottom=rc.top+160;
	rc.right=rect.right-GetSystemMetrics(SM_CXFIXEDFRAME);
	rc.left=rc.right-348;
    if (!CreateEx(NULL  , strClass, GetResString(IDS_MYINFO),WS_POPUP |WS_BORDER | WS_VISIBLE
						,rc.left,rc.top,rc.Width(),rc.Height(), 
						pParent ? pParent->GetSafeHwnd() : NULL, NULL))
		return;
	m_hIcon=theApp.LoadIcon(_T("MyInfo"));
	SetIcon(m_hIcon,TRUE);
	SetIcon(m_hIcon,FALSE);
	GetClientRect(&rc);
	if (!m_cList.CreateEx(LVS_EX_FULLROWSELECT,WS_VISIBLE|WS_CHILD|WS_CLIPCHILDREN|WS_VSCROLL|WS_CLIPSIBLINGS|LVS_NOCOLUMNHEADER|LVS_REPORT,rc,(CWnd*)this,1001))
		{
		DestroyWindow();
		return;
		}
	m_cList.InsertColumn(0, _T(""), LVCFMT_LEFT, 100, -1); 
	m_cList.InsertColumn(1, _T(""), LVCFMT_LEFT, rc.Width()-100, 1); 
	m_cList.InsertItem(0,_T("?"));
	UpdateInfo();	
	m_bIsActive=true;
	SetTransparent();
	ShowWindow(SW_SHOW);
	}
else
	{
	DestroyWindow();
	}
}


void CMyInfoWnd::UpdateInfo(void)
{
	if (!::IsWindow(m_hWnd) || !::IsWindow(m_cList.m_hWnd))
		return;
	FillMyInfo(&m_cList,true);
	UpdatePos();
}

void CMyInfoWnd::UpdatePos(void)
{
	if (!::IsWindow(m_hWnd) || !::IsWindow(m_cList.m_hWnd))
		return;
	CRect rc,rect;
	m_cList.GetItemRect(0,&rc,LVIR_BOUNDS   );
	int height=4+(m_cList.GetItemCount()*rc.Height());
	theApp.emuledlg->toolbar->GetWindowRect(&rc);
	//theApp.emuledlg->ClientToScreen(&rc);
	rc.left=rc.right-348;
	SetWindowPos(&wndTop,rc.left,rc.bottom,rc.Width(),height,NULL);
}

bool CMyInfoWnd::SetTransparent(void)
{
	return SetLayeredWindow(this->GetSafeHwnd(),192);
}

void CMyInfoWnd::FillMyInfo(CListCtrl* m_pList,bool bKeys)
{
	if (!m_pList)
		return;
	CString buffer;

	m_pList->DeleteAllItems();  
	m_pList->InsertItem(0,GetResString(IDS_STATUS)+_T(":"));  
	if (theApp.serverconnect->IsConnected())  
		m_pList->SetItemText(0, 1, GetResString(IDS_CONNECTED ));  
	else    
		m_pList->SetItemText(0, 1, GetResString(IDS_DISCONNECTED ));     
	if (theApp.IsConnected())   
	{   
		m_pList->InsertItem(1,GetResString(IDS_IP) + _T(":") + GetResString(IDS_PORT) );  
		if (theApp.serverconnect->IsLowID())     
			buffer=GetResString(IDS_UNKNOWN);    
		else     
		{    
			uint32 myid=theApp.serverconnect->GetClientID();  
			uint8 d=myid/(256*256*256);myid-=d*(256*256*256);  
			uint8 c=myid/(256*256);myid-=c*256*256; 
			uint8 b=myid/(256);myid-=b*256;
			buffer.Format(_T("%i.%i.%i.%i:%i"),myid,b,c,d,thePrefs.GetPort());    
		}   
		m_pList->SetItemText(1,1,buffer);   
		buffer.Format(_T("%u"),theApp.serverconnect->GetClientID());
		m_pList->InsertItem(2,GetResString(IDS_ID)); 
		if (theApp.IsConnected())    
			m_pList->SetItemText(2, 1, buffer);  
		m_pList->InsertItem(3,_T(""));  
		if (theApp.serverconnect->IsLowID()) 
			m_pList->SetItemText(3, 1,GetResString(IDS_IDLOW)); 
		else    
			m_pList->SetItemText(3, 1,GetResString(IDS_IDHIGH)); 
	}   
	if (!bKeys)   
		return;   

	int iItem=m_pList->GetItemCount(); 
	buffer=EncodeBase16((const unsigned char*)thePrefs.GetUserHash(), 16);  
	if (!buffer.IsEmpty())  
	{    
		m_pList->InsertItem(iItem,_T("Hash:")); 
		m_pList->SetItemText(iItem,1,buffer);  
		iItem++;  
	}    
	iItem=m_pList->GetItemCount();   
	if (theApp.IsConnected())  
	{  
		buffer = theApp.serverconnect->GetCurrentServer()->GetListName(); 
		if(!buffer.IsEmpty())  
		{  
			m_pList->InsertItem(iItem,_T("Server:"));   
			m_pList->SetItemText(iItem,1,buffer); 
		}  
	}
}
//>>> eWombat [MYINFOWND]

void CMyInfoWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_bIsDragging = true;
}

void CMyInfoWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	if( !(nFlags & MK_LBUTTON) ) m_bIsDragging = false;

	if (m_bIsDragging){
		CPoint pt(point);           //get our current mouse coordinates
		ClientToScreen(&pt);        //convert to screen coordinates
		CRect rc;
		GetClientRect(&rc);
		m_cList.SetWindowPos(&wndTop,pt.x,pt.y,rc.Width(),rc.Height(),SWP_NOZORDER);
	}
}

void CMyInfoWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_bIsDragging = false;
}